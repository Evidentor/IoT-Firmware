#include <stdio.h>
#include <string.h>
#include "wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "../../../../../../../../../esp/esp-idf/components/wifi_provisioning/src/wifi_provisioning_priv.h"
#include "rom/ets_sys.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_softap.h"

#define EXAMPLE_ESP_WIFI_SSID "myssid"
#define EXAMPLE_ESP_WIFI_CHANNEL 1
#define EXAMPLE_ESP_WIFI_PASS "mypassword"
#define EXAMPLE_MAX_STA_CONN 3
#define WIFI_MAX_RETRY_NUM 3

static const char *TAG = "wifi";
static int s_retry_num = 0;
wifi_config_t wifi_config;
bool wifi_connected = false;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d, reason=%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Started listening for DPP Authentication");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAX_RETRY_NUM) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        ESP_LOGI(TAG, "connect to the AP fail");
        wifi_connected = false;
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_prov_mgr_done();
        wifi_prov_mgr_stop_provisioning();
        ESP_LOGI(TAG, "Successfully connected to the AP ssid : %p ", wifi_config.sta.ssid);
        wifi_connected = true;
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("WIFI", "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_connected = true;
    }
}

void init_wifi(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize networking stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    // Initialize Wi-Fi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
// #ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
//             .authmode = WIFI_AUTH_WPA3_PSK,
//             .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
// #else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
//             .authmode = WIFI_AUTH_WPA2_PSK,
// #endif
            .pmf_cfg = {
                .required = true,
            },
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);

    // Init provisioning service
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };
    ESP_ERROR_CHECK( wifi_prov_mgr_init(config) );

    // Start provisioning service
    wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
    const char *pop = "abcd1234";
    ESP_ERROR_CHECK( wifi_prov_mgr_start_provisioning(security, pop, EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS) );

    // Wait for service to complete
    ESP_LOGI(TAG, "Waiting for provisioning to complete...");
    wifi_prov_mgr_wait();

    // Finally de-initialize the manager
    ESP_LOGI(TAG, "De-initializing the manager...");
    wifi_prov_mgr_deinit();

    // // Configure HTTP provisioning scheme (SoftAP + HTTP server)
    // wifi_prov_mgr_config_t config = {
    //     .scheme = wifi_prov_scheme_softap,
    //     .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    // };
    // ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
    //
    // bool provisioned = false;
    // ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));
    //
    // if (!provisioned) {
    //     const char *service_name = "my_device";
    //     const char *service_key = "password";  // AP password
    //
    //     wifi_prov_security_t security = WIFI_PROV_SECURITY_0;
    //     const char *pop = "abcd1234"; // Proof-of-possession (used in security)
    //
    //     ESP_LOGI(TAG, "Starting HTTP provisioning via SoftAP...");
    //     ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key));
    // } else {
    //     ESP_LOGI(TAG, "Device already provisioned, starting Wi-Fi...");
    //     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    //     ESP_ERROR_CHECK(esp_wifi_start());
    // }
}

bool is_connected(void) {
    return wifi_connected;
}
