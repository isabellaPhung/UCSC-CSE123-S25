#include <string.h>
#include <stdbool.h>
#include <sys/param.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "lwip/inet.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#ifdef CONFIG_HTTPS_SERVER
#include "esp_https_server.h"
#include "esp_tls.h"
#else
#include "esp_http_server.h"
#endif

#ifdef CONFIG_CAPTIVE_SERVER
#include "dns_server.h"
#endif

#include "wifi_setup.h"

#if CONFIG_ESP_STATION_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define H2E_IDENTIFIER ""
#elif CONFIG_ESP_STATION_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_STATION_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

#ifndef CONFIG_DEVICE_ID
#define CONFIG_DEVICE_ID "55"
#endif

static const char root_html[] =
  "<!DOCTYPE html>\n"
  "<html>\n"
  "  <head>\n"
  "    <style>\n"
  "      body {\n"
  "        background-color: #ffffff;\n"
  "      }\n"
  "    </style>\n"
  "    <title>Device Config Portal</title>\n"
  "  </head>\n"
  "  <body>\n"
  "    <h1>Device Config Portal</h1>\n"
  "    <p>Device ID: " CONFIG_DEVICE_ID "</p>\n"
  "    <p>Wi-Fi configuration</p>\n"
  "    <form action=\"/config\" method=\"post\" target=\"response\">\n"
  "      <label for=\"ssid\">SSID:</label>\n"
  "      <input type=\"text\" id=\"ssid\" name=\"ssid\"><br><br>\n"
  "      <label for=\"pswd\">Password:</label>\n"
  "      <input type=\"password\" id=\"pswd\" name=\"pswd\"><br><br>\n"
  "      <input type=\"submit\" value=\"Submit\">\n"
  "    </form>\n"
  "    <iframe name=\"response\" style=\"border:none;\"></iframe>\n"
  "  </body>\n"
  "</html>";

static char *TAG = "wifi_setup";

static httpd_handle_t webserver;

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

static bool connected;

#ifdef CONFIG_CAPTIVE_SERVER
    static dns_server_handle_t dns_server;
#endif

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define WIFI_SERVER_UP     BIT2
#define WIFI_RESP_SENT     BIT3

esp_err_t init_wifi_sta(const char ssid[32], const char pswd[64]) {
  // reset previous retry status
  s_retry_num = 0;
  xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

  // reset previous config
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));

  wifi_config_t wifi_config = {
    .sta = {
      .ssid = {0},
      .password = {0},
      .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
      .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
      .sae_h2e_identifier = H2E_IDENTIFIER,
    },
  };
  memcpy(wifi_config.sta.ssid, ssid, 32);
  memcpy(wifi_config.sta.password, pswd, 64);

  // set mode to station + access point
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

  esp_wifi_connect();

  ESP_LOGI(TAG, "wifi_init_sta finished.");

  // Wait for either a success or n retries failed
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
      WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

  esp_err_t ret = ESP_OK;
  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "Successfully connected to SSID:%s password:%s", ssid, pswd);
  } else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid, pswd);
    ret = ESP_FAIL;
  } else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
    ret = ESP_FAIL;
  }
  return ret;
}

static esp_err_t root_get_handler(httpd_req_t *req) {

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, root_html, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static const httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler
};

#define MAX_SSID_LEN 32
#define MAX_PSWD_LEN 64
#define BUF_LEN MAX_SSID_LEN + MAX_PSWD_LEN + 5 + 1 + 5

static esp_err_t login_post_handler(httpd_req_t *req) {
  xEventGroupClearBits(s_wifi_event_group, WIFI_RESP_SENT);
  int length = req->content_len;
  // max ssid+password is 32+64 = 96 chars
  // including "ssid=" "&" "pswd=" makes the buffer need 107
  char buf[BUF_LEN];

  httpd_req_recv(req, buf, MIN(length, BUF_LEN));
  ESP_LOGI(TAG, "Recieved buffer: %s", buf);

  char ssid[MAX_SSID_LEN] = {0};
  char pswd[MAX_PSWD_LEN] = {0};

  httpd_query_key_value(buf, "ssid", ssid, MAX_SSID_LEN);
  httpd_query_key_value(buf, "pswd", pswd, MAX_PSWD_LEN);
  char *garb = strchr(pswd, '\n');
  if (garb) {
    *garb = '\0';
    ESP_LOGI(TAG, "Deleted garbo");
  }

  ESP_LOGI(TAG, "SSID:%s Password:%s", ssid, pswd);

  esp_err_t res = init_wifi_sta(ssid, pswd);
  if (res != ESP_OK) {
    httpd_resp_send(req, "<h1>Not OK</h1>", HTTPD_RESP_USE_STRLEN);
  } else {
    httpd_resp_send(req, "<h1>OK</h1>", HTTPD_RESP_USE_STRLEN);
  }

  xEventGroupSetBits(s_wifi_event_group, WIFI_RESP_SENT);

  return ESP_OK;
}

static const httpd_uri_t login = {
  .uri = "/config",
  .method = HTTP_POST,
  .handler = login_post_handler,
  .user_ctx = NULL
};

#ifdef CONFIG_CAPTIVE_SERVER
// HTTP Error (404) Handler - Redirects all requests to the root page
static esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err) {
  // Set status
  httpd_resp_set_status(req, "302 Temporary Redirect");
  // Redirect to the "/" root directory
  httpd_resp_set_hdr(req, "Location", "/");
  // iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
  httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

  ESP_LOGI(TAG, "Redirecting to root");
  return ESP_OK;
}
#endif

static void start_server(void) {
  httpd_handle_t server = NULL;

#ifdef CONFIG_HTTPS_SERVER
  httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();

  extern const unsigned char servercert_start[] asm("_binary_servercert_pem_start");
  extern const unsigned char servercert_end[]   asm("_binary_servercert_pem_end");
  conf.servercert = servercert_start;
  conf.servercert_len = servercert_end - servercert_start;

  extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
  extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
  conf.prvtkey_pem = prvtkey_pem_start;
  conf.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

  esp_err_t ret = httpd_ssl_start(&server, &conf);
#else
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_open_sockets = 4;
  config.lru_purge_enable = true;

  esp_err_t ret = httpd_start(&server, &config);
#endif

  if (ret == ESP_OK) {
    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &login);
#ifdef CONFIG_CAPTIVE_SERVER
    httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
#endif
    webserver = server;
  }
  ESP_LOGI(TAG, "Completed https server setup");

#ifdef CONFIG_CAPTIVE_SERVER
  // Start the DNS server that will redirect all queries to the softAP IP
  dns_server_config_t dns_conf = DNS_SERVER_CONFIG_SINGLE("*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
  dns_server = start_dns_server(&dns_conf);
  ESP_LOGI(TAG, "Completed dsn server setup");
#endif
}

static void close_server(void) {
#ifdef CONFIG_CAPTIVE_SERVER
  stop_dns_server(dns_server);
#endif
#ifdef CONFIG_HTTPS_SERVER
  httpd_ssl_stop(webserver);
#else
  httpd_stop(webserver);
#endif
}

static void init_wifi_ap(void) {
  wifi_config_t wifi_config = {
    .ap = {
      .ssid = CONFIG_ESP_WIFI_SSID,
      .ssid_len = strlen(CONFIG_ESP_WIFI_SSID),
      .password = CONFIG_ESP_WIFI_PASSWORD,
      .max_connection = CONFIG_ESP_MAX_STA_CONN,
      .authmode = WIFI_AUTH_WPA_WPA2_PSK
    },
  };
  if (strlen(CONFIG_ESP_WIFI_PASSWORD) == 0) {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
}

static void wifi_event_handler(void *arg,
    esp_event_base_t event_base, int32_t event_id, void *event_data) {
  if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
    connected = false;
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI(TAG, "Retry connecting to the AP");
    } else {
      if (xEventGroupGetBits(s_wifi_event_group) ^ WIFI_SERVER_UP) {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        start_server();
        xEventGroupSetBits(s_wifi_event_group, WIFI_SERVER_UP);
        ESP_LOGI(TAG, "Wifi connection lost, restarting server");
      }
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    ESP_LOGI(TAG,"Disconnected from the AP");
  } else if (event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    connected = true;
    if (xEventGroupGetBits(s_wifi_event_group) & WIFI_SERVER_UP){
      xEventGroupSync(s_wifi_event_group,
          WIFI_CONNECTED_BIT, WIFI_RESP_SENT, portMAX_DELAY);
      close_server();
      xEventGroupClearBits(s_wifi_event_group, WIFI_SERVER_UP);
      ESP_LOGI(TAG, "Wifi connection successful, ended server");
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  }
}

bool check_nvs_sta_config(void) {
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));

  wifi_config_t config;
  ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &config));
  if (strlen((char *)config.sta.ssid) == 0) {
    return false;
  }
  ESP_LOGI(TAG, "Connecting to SSID:%s with PWD: %s", config.sta.ssid, config.sta.password);
  return true;
}

bool is_wifi_connected(void){
  return connected;
}

/* The wifi setup
 * 1. initialize callbacks and configs/memory for ap/sta
 * 2. Set-up softAP and start the dns (optional) and http(s) servers
 * 3. Wait for form submission
 * 4. If the form entry is a valid ssid and password, establish connection with ap
 * 5. turn off softAP until connection to the ap is lost
 * 6. Turn off sta once connection is lost, restart softAP for the user to enter config again.
 */
void setup_wifi(void) {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  s_wifi_event_group = xEventGroupCreate();
  connected = false;

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

  esp_netif_create_default_wifi_sta();
  esp_netif_create_default_wifi_ap();

  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  init_wifi_ap();

  if (!check_nvs_sta_config()) {
    ESP_LOGI(TAG, "No Wifi in nvs");
    ESP_LOGI(TAG, "Starting server");
    start_server();
    xEventGroupSetBits(s_wifi_event_group, WIFI_SERVER_UP);
  } else {
    ESP_LOGI(TAG, "NVS contains wifi config");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    esp_wifi_connect();
  }
}
