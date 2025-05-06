#include "sender.h"
#include "freertos/FreeRTOS.h"
#include "mqtt_conn.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define MAX_RETRIES 5
#define MQTT_LOOP_TIMEOUT_MS 5000

#define MAX_SEND_LENGTH 128 + UUID_LENGTH // Maximum length of JSON information that can be sent to the server at once

esp_err_t SendTaskStatus(const char *uuid, TASK_STATUS status, struct callback_data_t *cb_data)
{
    static const char *TAG = "Sender::SendTaskStatus";

    // ----------------- Construct JSON Payload from directory ---------------------
    char msg[MAX_SEND_LENGTH];

    // TODO: Replace this with a read from disk to cJSON
    snprintf(msg, MAX_SEND_LENGTH,
             "{\n"
             "\"action\": update,"
             "\"tasks\":[\n"
             "    {\n"
             "      \"id\":\"%s\",\n"
             "      \"completion\": %d,\n"
             "    }\n"
             "  ]\n"
             "}",
             uuid, (int)status);

    cb_data->update_ack = 0;

    // ----------------------------- Initialize MQTT ------------------------------------
    if (mqtt_connect() != 0)
    {
        ESP_LOGE(TAG, "Failed to connect to MQTT broker");
        return ESP_FAIL;
    }

    mqtt_subscribe();

    // ------------------------------ Publish Message -----------------------------------
    mqtt_publish(msg, strlen(msg));

    size_t i = 0;
    for (; i < MAX_RETRIES; i++)
    {
        mqtt_loop(MQTT_LOOP_TIMEOUT_MS);
        if (cb_data->update_ack == 0)
        {
            ESP_LOGI(TAG, "Did not get back a length message from the server, re-publishing after a delay");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            mqtt_publish(msg, strlen(msg));
        }
        else
        {
            ESP_LOGI(TAG, "Message acknowledged by server");
            break;
        }
        i++;
    }

    mqtt_unsubscribe();
    mqtt_disconnect();

    if (i < 5)
    {
        return ESP_OK; // Success!
    }

    // ------------------------------ Buffer on Failure -----------------------------------
    ESP_LOGW(TAG, "Failed to connect to server! Caching request...");

    // Set path name to UUID to be changed, this both makes the file contents apparent,
    // but also results put in the new requests file will be overwritten.
    char path[UUID_LENGTH + 25];
    snprintf(path, sizeof(path), MOUNT_POINT "/requests/%s.json", uuid); // Ensure full path

    FILE *file = fopen(path, "w");
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file %s! Error: %s", path, strerror(errno));
        return ESP_ERR_INVALID_ARG;
    }

    size_t written = fwrite(msg, 1, MAX_SEND_LENGTH, file);
    if (written != MAX_SEND_LENGTH)
    {
        ESP_LOGE(TAG, "Failed to write full message to file! Error: %s", strerror(errno));
        fclose(file);
        return ESP_ERR_INVALID_ARG;
    }

    fclose(file);
    return ESP_OK;
}
