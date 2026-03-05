#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_base.hpp"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"
#include "wifi_c.hpp"
#include "wifi_stat.hpp"



namespace vfarm {

WifiStatCheck::WifiStatCheck(const char* ap_ssid, const char* ap_pass) {
    this->start(ap_ssid, ap_pass);
}

void WifiStatCheck::start(const char* ap_ssid, const char* ap_pass){
    // nvs_flash_init();
    // esp_netif_init();
    // esp_event_loop_create_default();
    // wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    // esp_wifi_init(&cfg);
    ap_netif_ = esp_netif_create_default_wifi_ap();
    esp_wifi_set_mode(WIFI_MODE_APSTA);  // AP + STA mode

    wifi_config_t ap = {};
    strncpy((char*)ap.ap.ssid, ap_ssid, 32);
    ap.ap.ssid_len = strlen(ap_ssid);
    ap.ap.max_connection = 2;
    if (ap_pass && strlen(ap_pass) >= 8) {
        strncpy((char*)ap.ap.password, ap_pass, 64);
        ap.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        ap.ap.authmode = WIFI_AUTH_OPEN;
    }
    esp_wifi_set_config(WIFI_IF_AP, &ap);
    // esp_wifi_start();


    // Start HTTP server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&http_server_, &config) == ESP_OK) {
        // REGISTER HANDLERS ↓↓↓
        httpd_uri_t config_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = config_handler,
            .user_ctx = this
        };
        httpd_uri_t save_uri = {
            .uri = "/save",
            .method = HTTP_POST,
            .handler = save_handler,
            .user_ctx = this
        };
        httpd_register_uri_handler(http_server_, &config_uri);
        httpd_register_uri_handler(http_server_, &save_uri);

    }

   
}


esp_err_t WifiStatCheck::config_handler(httpd_req_t *req) {
    const char* status = vfarm::MqttBase::mqtt_connected ? "Connected" : "Disconnected";
    char resp[512];
    snprintf(resp, sizeof(resp),
        // "<html><head><meta http-equiv='refresh' content='5'></head>"
        "<html><body><h2>Setup</h2>"
        "<p>MQTT Status: %s</p>"
        "<form method='post' action='/save'>"
        "SSID: <input name='ssid'><br>"
        "Pass: <input name='pass'><br>"
        "MQTT: <input name='mqtt'><br>"
        "<input type='submit'></form></body></html>",
        status);
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t WifiStatCheck::save_handler(httpd_req_t *req) {
    httpd_resp_send(req, "Saved!", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

WifiStatCheck::~WifiStatCheck() { 
    // ESP_LOGI(tag, "Server off flag: ", server_off_flag);
    // ESP_LOGI(tag, "http_server_: ", http_server_);
    if (http_server_) {
        httpd_stop(http_server_);
        http_server_ = nullptr;  // ← mark as stopped
        ESP_LOGI(tag, "Setup mode stopped");
    }
}

void WifiStatCheck::stop() {
    if (http_server_) {
        httpd_stop(http_server_);
        http_server_ = nullptr;
        }
    esp_wifi_set_mode(WIFI_MODE_STA);
    // this->server_off_flag = true;
    ESP_LOGI(tag, "Setup mode stopped");
}
}