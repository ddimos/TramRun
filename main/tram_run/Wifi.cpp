#include "tram_run/Wifi.hpp"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sys.h"

namespace
{
    static const char *TAG = "TR_WIFI";

    #define TR_ESP_WIFI_SSID      ""  // TODO add it to the config
    #define TR_ESP_WIFI_PASS      ""
    #define TR_ESP_MAXIMUM_RETRY  5

    static esp_netif_t* g_netif = nullptr;
    static esp_event_handler_instance_t g_instanceAnyId = nullptr;
    static esp_event_handler_instance_t g_instanceGotIp = nullptr;

    static unsigned g_retryNum = 0;
    static tr::wifi::OnWifiStateCallback g_callback{};

    static void event_handler(void* _arg, esp_event_base_t _eventBase, int32_t _eventId, void* _eventData)
    {
        if (_eventBase == WIFI_EVENT)
        {
            switch (_eventId)
            {
                case WIFI_EVENT_STA_START:
                {
                    ESP_LOGI(TAG, "WIFI_EVENT_STA_START: Station mode started, connecting to AP...");
                    esp_wifi_connect();
                    break;
                }
                case WIFI_EVENT_STA_CONNECTED:
                {
                    ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED: Connected to AP SSID:%s", TR_ESP_WIFI_SSID);
                    break;
                }
                case WIFI_EVENT_STA_DISCONNECTED:
                {
                    ESP_LOGW(TAG, "WIFI_EVENT_STA_DISCONNECTED: Lost connection.");
                    if (g_retryNum < TR_ESP_MAXIMUM_RETRY)
                    {
                        esp_wifi_connect();
                        g_retryNum++;
                        ESP_LOGI(TAG, "Retrying connection (%d/%d)...", g_retryNum, TR_ESP_MAXIMUM_RETRY);
                    }
                    else
                    {
                        ESP_LOGE(TAG, "Connection failed after %d retries.", TR_ESP_MAXIMUM_RETRY);
                        
                        g_callback(tr::wifi::State::NotAbleToConnect);
                    }
                    break;
                }
                default:
                {
                    ESP_LOGI(TAG, "Unhandled WIFI_EVENT: %ld", _eventId);
                    break;
                }
            }
        }
        else if (_eventBase == IP_EVENT)
        {
            switch(_eventId)
            {
                case IP_EVENT_STA_GOT_IP:
                {
                    ip_event_got_ip_t* event = (ip_event_got_ip_t*) _eventData;
                    ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP: Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
                    g_retryNum = 0;

                    g_callback(tr::wifi::State::Ready);
                    break;
                }
                default:
                {
                    ESP_LOGI(TAG, "Unhandled IP_EVENT: %ld", _eventId);
                    break;
                }
            }
        }
        else
        {
            ESP_LOGW(TAG, "Received event from unknown base: %s", _eventBase);
        }
    }

} // namespace

namespace tr::wifi
{
    void init(OnWifiStateCallback _callback)
    {
        g_callback = _callback;

        g_netif = esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        ESP_ERROR_CHECK(
            esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &g_instanceAnyId)
        );
        ESP_ERROR_CHECK(
            esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &g_instanceGotIp)
        );

        wifi_config_t wifiConfig;
        {
            wifi_sta_config_t staConfig = {
                .ssid = TR_ESP_WIFI_SSID,
                .password = TR_ESP_WIFI_PASS
            };

            staConfig.threshold.authmode = WIFI_AUTH_WPA2_PSK;
            wifiConfig.sta = staConfig;
        }

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));

        ESP_LOGI(TAG, "wifi init finished");
    }
    
    void deinit()
    {
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, g_instanceGotIp));
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, g_instanceAnyId));
        ESP_ERROR_CHECK(esp_wifi_deinit());
        esp_netif_destroy_default_wifi(g_netif);

        g_callback = {};
    }

    void start()
    {
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    void stop()
    {
        ESP_ERROR_CHECK(esp_wifi_stop());
    }

} // namespace tr::wifi
