#include "intermediary.h"
#include "mqtt_conn.h"

#include <stdio.h>
#include <string.h>

int SendTaskStatusChangeMessage(const char *uuid, CompletionStatus status)
{
    // ----------------------------- Construct Message ----------------------------------
    char msg[MAX_SEND_LENGTH];
    char status_msg[32];
    switch (status)
    {
    case INCOMPLETE:
        strcpy(status_msg, "incomplete");
        break;
    case COMPLETE:
        strcpy(status_msg, "complete");
        break;
    case MFD:
        strcpy(status_msg, "delete");
        break;
    default:
        strcpy(status_msg, "NULL");
        break;
    }

    snprintf(msg, MAX_SEND_LENGTH,
             "{\n"
             "  \"tasks\":[\n"
             "    {\n"
             "      \"id\":\"%s\",\n"
             "      \"status\":\"%s\",\n"
             "    }\n"
             "  ]\n"
             "}",
             uuid, status_msg);

    // ------------------------------ Publish Message -----------------------------------

    // TODO: Impletment MQTT publish function
    /*
    mqtt_init(...); // Something here
    mqtt_connect();
    mqtt_subscribe();

    mqtt_publish(msg, strlen(msg));

    size_t i = 0;
    do
    {
        mqtt_loop(5000);
        if (cb_data.expected == 0)
        {
            ESP_LOGI(TAG, "Did not get back a length message from the server, re-publishing after a delay");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            mqtt_publish(payload, payload_length);
        }
        else if (cb_data.expected != cb_data.cur_index)
        {
            ESP_LOGI(TAG, "Did not get the expected amount, listening for some more time");
        }
        else
        {
            break;
        }
        i++;
    } while (i < 5);

    mqtt_unsubscribe();
    mqtt_disconnect();

    if (i < 5)
        return 0;   // Success!
    */

    // ------------------------------ Buffer on Failure -----------------------------------
    return -1;
}
