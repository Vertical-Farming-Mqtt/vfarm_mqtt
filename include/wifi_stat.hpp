#ifndef WIFI_STAT_HPP
#define WIFI_STAT_HPP

#include "esp_http_server.h"
#include "esp_netif.h"

namespace vfarm {
class WifiStatCheck {
public:
    const char* tag = "wifi_server";
    httpd_handle_t http_server_ = nullptr;
    esp_netif_t* ap_netif_ = nullptr;

    bool server_off_flag = false;
    WifiStatCheck(const char* ap_ssid = "ESP32_Config", const char* ap_pass = nullptr);
    ~WifiStatCheck();
    void start(const char* ap_ssid, const char* ap_pass);
    void stop();

private:
    static esp_err_t config_handler(httpd_req_t *req);
    static esp_err_t save_handler(httpd_req_t *req);
};
} // namespace vfarm

#endif