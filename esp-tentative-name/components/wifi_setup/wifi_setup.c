#include <string.h>
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

#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN CONFIG_ESP_MAX_STA_CONN
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#if CONFIG_ESP_STATION_EXAMPLE_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_STATION_EXAMPLE_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_STATION_EXAMPLE_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
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

static esp_netif_t *sta_netif = NULL;
static esp_netif_t *ap_netif = NULL;
static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;

extern const char root_start[] asm("_binary_root_html_start");
extern const char root_end[] asm("_binary_root_html_end");

static char *TAG = "wifi_setup";

static httpd_handle_t webserver;

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

#ifdef CONFIG_CAPTIVE_SERVER
    static dns_server_handle_t dns_server;
#endif

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define WIFI_SERVER_UP     BIT2
#define WIFI_RESP_SENT     BIT3

esp_err_t init_wifi_sta(const char ssid[32], const char pswd[64]){
  // reset previous retry status
  s_retry_num = 0;
  xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

  // reset previous config
  esp_netif_destroy_default_wifi(sta_netif);
  sta_netif = esp_netif_create_default_wifi_sta();

  // set mode to station + access point
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

  wifi_config_t wifi_config = {
    .sta = {
      .ssid = {0},
      .password = {0},
      .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
      .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
      .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
    },
  };
  memcpy(wifi_config.sta.ssid, ssid, 32);
  memcpy(wifi_config.sta.password, pswd, 64);
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

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

static esp_err_t root_get_handler(httpd_req_t *req)
{
    const uint32_t root_len = root_end - root_start;

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, root_start, root_len);

    return ESP_OK;
}

static const httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler
};

static esp_err_t login_post_handler(httpd_req_t *req) {
  int length = req->content_len;
  char buf[100];

  httpd_req_recv(req, buf, MIN(length, sizeof(buf)));

  char *start = buf;
  char *req_ampersand = strstr(buf, "&");
  char *req_password = strstr(buf, "pswd");
  char *end = buf + length;

  size_t ssid_len = req_ampersand - start - 5;
  size_t pswd_len = end - req_password - 5;

  // Set lengths for the SSID and Password based on wifi_config_t
  char ssid[32] = {0};
  char pswd[64] = {0};

  memcpy(ssid, start + 5, ssid_len);
  memcpy(pswd, req_password + 5, pswd_len);

  esp_err_t res = init_wifi_sta(ssid, pswd);
  if (res != ESP_OK){
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
static esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
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

static httpd_handle_t start_webserver(void)
{
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
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &login);
#ifdef CONFIG_CAPTIVE_SERVER
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
#endif
    }
    ESP_LOGI(TAG, "Completed https server setup");
    return server;
}

#ifdef CONFIG_CAPTIVE_SERVER
static void ap_dns_start(void){ 
  // Start the DNS server that will redirect all queries to the softAP IP
  dns_server_config_t dns_conf = DNS_SERVER_CONFIG_SINGLE("*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
  dns_server = start_dns_server(&dns_conf);
}
#endif

static void init_wifi_ap(void){
  wifi_config_t wifi_config = {
    .ap = {
      .ssid = EXAMPLE_ESP_WIFI_SSID,
      .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
      .password = EXAMPLE_ESP_WIFI_PASS,
      .max_connection = EXAMPLE_MAX_STA_CONN,
      .authmode = WIFI_AUTH_WPA_WPA2_PSK
    },
  };
  if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

  esp_netif_ip_info_t ip_info;
  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

  char ip_addr[16];
  inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
  ESP_LOGI(TAG, "Set up softAP with IP: %s", ip_addr);

  ESP_LOGI(TAG, "wifi_init_softap finished. SSID:'%s' password:'%s'",
      EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

static void wifi_event_handler(void *arg,
    esp_event_base_t event_base, int32_t event_id, void *event_data) {

  switch(event_id) {
    case WIFI_EVENT_AP_STACONNECTED:
      {
      wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
      ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
          MAC2STR(event->mac), event->aid);
      break;
      }

    case WIFI_EVENT_AP_STADISCONNECTED:
      {
      wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
      ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d, reason=%d",
          MAC2STR(event->mac), event->aid, event->reason);
      break;
      }

    case WIFI_EVENT_STA_START:
      esp_wifi_connect();
      break;

    case WIFI_EVENT_STA_DISCONNECTED:
      xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
      if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
        esp_wifi_connect();
        s_retry_num++;
        ESP_LOGI(TAG, "Retry connecting to the AP");
      } else {
        if (xEventGroupGetBits(s_wifi_event_group) ^ WIFI_SERVER_UP){
          // if wifi dies after a successful connection
          ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
          webserver = start_webserver();
#ifdef CONFIG_CAPTIVE_SERVER
          ap_dns_start(); 
#endif
          xEventGroupSetBits(s_wifi_event_group, WIFI_SERVER_UP);

          ESP_LOGI(TAG, "Wifi connection lost, restarting softAP");
        }
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
      }
      ESP_LOGI(TAG,"Disconnected from the AP");
      break;

    case IP_EVENT_STA_GOT_IP:
      {
      ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
      ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
      s_retry_num = 0;
      xEventGroupSync(s_wifi_event_group, 
          WIFI_CONNECTED_BIT, WIFI_RESP_SENT, portMAX_DELAY);
#ifdef CONFIG_CAPTIVE_SERVER
      stop_dns_server(dns_server);
#endif
#ifdef CONFIG_HTTPS_SERVER
      httpd_ssl_stop(webserver);
#else
      httpd_stop(webserver);
#endif
      xEventGroupClearBits(s_wifi_event_group, WIFI_SERVER_UP);
      ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
      break;
      }
  }
}

// Returns true if connected
bool is_wifi_connected(void){
  return WIFI_CONNECTED_BIT & xEventGroupGetBits(s_wifi_event_group);
}

/* The wifi setup
 * 1. initialize callbacks and configs/memory for ap/sta
 * 2. Set-up softAP and start the dns (optional) and http(s) servers
 * 3. Wait for form submission
 * 4. If the form entry is a valid ssid and password, establish connection with ap
 * 5. turn off softAP until connection to the ap is lost
 * 6. Turn off sta once connection is lost, restart softAP for the user to enter config again.
 */
void setup_wifi(void){
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

  ap_netif = esp_netif_create_default_wifi_ap();
  sta_netif = esp_netif_create_default_wifi_sta();

  init_wifi_ap();
  webserver = start_webserver();
#ifdef CONFIG_CAPTIVE_SERVER
  ap_dns_start();
#endif
  xEventGroupSetBits(s_wifi_event_group, WIFI_SERVER_UP);

  ESP_ERROR_CHECK(esp_wifi_start());
}
