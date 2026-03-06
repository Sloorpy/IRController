#include "Utils.hpp"

#include <esp_netif_sntp.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace Utils
{
    bool sync_time(WiFi& wifi)
    {
        while (true)
        {
            if (!wifi.is_connected())
            {
                return false;
            }
            
            setenv("TZ", "IST-2IDT,M3.4.4/26,M10.5.0", 1);
            tzset();

            esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
            esp_netif_sntp_init(&config);

            if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) == ESP_OK)
            {
                time_t now = time(nullptr);
                if (now > 1700000000)
                {
                    printf("[Time] Time synced successfully: %s", ctime(&now));
                    return true;
                }
            }

            printf("[Time] Time sync failed, retrying...\n");
            esp_netif_sntp_deinit();
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}
