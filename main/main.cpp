#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdio.h>
#include <string>

#include "Exception.hpp"
#include "IRCommand.hpp"
#include "driver/gpio.h"
#include "esp_err.h"
#include "LEDTransmitter.hpp"
#include "IRReceiver.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "WiFi.hpp"
#include "HTTPClient.hpp"
#include "WifiCreds.h"
#include "Utils.hpp"

static constexpr std::string_view LIVE_ALERT = "https://www.oref.org.il/warningMessages/alert/Alerts.json";
static constexpr std::string_view ALERT_HISTORY = "https://alerts-history.oref.org.il//Shared/Ajax/GetAlarmsHistory.aspx?lang=he&mode=1&city_0=%D7%A0%D7%AA%D7%A0%D7%99%D7%94%20-%20%D7%9E%D7%A2%D7%A8%D7%91";

enum class LiveAlertCategory : uint8_t
{
    AIR_RAID = 1,
    HOSTILE_UFO = 2,
    LIVE_HOSTILE_UFO = 6,
    
    HOSTILE_UFO_OVER = 10,
    ALERT_OVER = 13,
    WARNING = 14,
};

enum class AlertCategory : uint8_t
{
    AIR_RAID = 1,

};

bool find_city_in_data(const json& response, const std::string& city)
{
    static constexpr std::string_view CITIES = "data";

    if (!response.contains(CITIES))
    {
        return false;
    }

    if (response[CITIES].is_string() && response[CITIES].get<std::string>() == city)
    {
        return true;
    }

    if (!response["data"].is_array())
    {
        return false;
    }

    for (const auto& item : response["data"])
    {
        if (!item.is_string())
        {
            continue;
        }

        if (item.get<std::string>() == city)
        {
            return true;
        }
    }

    return false;
}

bool has_alert_live(HTTPClient& client, std::string city)
{
    json result = client.get_json(LIVE_ALERT);
    
    if (result.empty())
    {
        return false;
    }

    if (!result.contains("cat") || !result["cat"].is_string())
    {
        return false;
    }

    LiveAlertCategory category = static_cast<LiveAlertCategory>(std::stoi(result["cat"].get<std::string>()));
    if (category == LiveAlertCategory::ALERT_OVER || category == LiveAlertCategory::HOSTILE_UFO_OVER)
    {
        return false;
    }

    printf("%s", result.dump(4).c_str());
    return find_city_in_data(result, city);
}


bool has_alert_history(HTTPClient& client, const std::string& city, const uint16_t time_diff_sec)
{
    json latest_alert = client.get_json_first(ALERT_HISTORY);

    if (latest_alert.empty())
    {
        return false;
    }
    
    if (!find_city_in_data(latest_alert, city))
    {
        return false;
    }

    if (!latest_alert.contains("category") || !latest_alert["category"].is_number_integer())
    {
        return false;
    }

    LiveAlertCategory category = static_cast<LiveAlertCategory>(latest_alert["category"].get<uint32_t>());
    if (category == LiveAlertCategory::ALERT_OVER || category == LiveAlertCategory::HOSTILE_UFO_OVER)
    {
        return false;
    }

    static constexpr std::string_view ALERT_DATE_FIELD = "alertDate";

    if (!latest_alert.contains(ALERT_DATE_FIELD) || !latest_alert[ALERT_DATE_FIELD].is_string())
    {
        return false;
    }

    std::string alert_date_str = latest_alert[ALERT_DATE_FIELD].get<std::string>();

    int year, month, day, hour, minute, second;
    if (sscanf(alert_date_str.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6)
    {
        return false;
    }

    struct tm alert_tm = {};
    alert_tm.tm_year = year - 1900;
    alert_tm.tm_mon = month - 1;
    alert_tm.tm_mday = day;
    alert_tm.tm_hour = hour;
    alert_tm.tm_min = minute;
    alert_tm.tm_sec = second;
    alert_tm.tm_isdst = -1;

    time_t alert_time = mktime(&alert_tm);
    time_t now = time(nullptr);

    double diff = difftime(now, alert_time);

    return diff >= 0 && diff <= time_diff_sec;
}

bool check_alert(HTTPClient& client, const std::string& city)
{
    if (has_alert_live(client, city))
    {
        return true;
    }

    static constexpr uint32_t LAST_ALERT_TIME_DIFF_SEC = 3 * 60;
    return has_alert_history(client, city, LAST_ALERT_TIME_DIFF_SEC);
}

void blink(LEDTransmitter& led)
{
    static constexpr uint64_t LED_BREAK_TIME_US = 35000;

    led.send_multiple(LEDProtocol::RED_LIGHT);
    usleep(LED_BREAK_TIME_US);
    
    led.send_multiple(LEDProtocol::WHITE_LIGHT);
    usleep(LED_BREAK_TIME_US);

    led.send_multiple(LEDProtocol::GREEN_LIGHT);
    usleep(LED_BREAK_TIME_US);
}

void turn_on_max(LEDTransmitter& led)
{
    led.send_multiple(LEDProtocol::POWER_ON, 10);
    
    static constexpr uint16_t MAX_BRIGHTNESS = 15;
    led.send_multiple(LEDProtocol::BRIGHT_UP, MAX_BRIGHTNESS);
}

void turn_off(LEDTransmitter& led)
{
    static constexpr uint16_t DELAY_BETWEEN_SIGNALS_MS = 100;
    led.send_multiple(LEDProtocol::POWER_OFF, 50, DELAY_BETWEEN_SIGNALS_MS);
}

void start_alarm(LEDTransmitter& led, const uint32_t alarm_duration_sec)
{
    turn_on_max(led);
    
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    while (true)
    {
        blink(led);
        
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        static constexpr uint64_t NS_PER_SEC = 1000000000ULL;
        const uint64_t alarm_duration_ns = alarm_duration_sec * NS_PER_SEC;

        const uint64_t elapsed_ns = (current_time.tv_sec - start_time.tv_sec) * NS_PER_SEC + 
                                (current_time.tv_nsec - start_time.tv_nsec);
        
        if (elapsed_ns >= alarm_duration_ns)
        {
            break;
        }
    }
}

void start_notification(LEDTransmitter& led)
{
    led.send_multiple(LEDProtocol::POWER_ON);
    led.send_multiple(LEDProtocol::WHITE_LIGHT);
    
    static constexpr uint16_t NOTIFICATION_BLINKS = 4;
    static constexpr uint32_t DELAY_BETWEEN_SIGNALS_US = 500000; 

    for (uint16_t i = 0; i < NOTIFICATION_BLINKS; ++i)
    {
        led.send_multiple(LEDProtocol::POWER_ON);
        usleep(DELAY_BETWEEN_SIGNALS_US);
        
        led.send_multiple(LEDProtocol::POWER_OFF);
        usleep(DELAY_BETWEEN_SIGNALS_US);
    }
}   

extern "C" void app_main(void)
{
    static constexpr std::string_view CITY = "נתניה - מערב";
    static constexpr gpio_num_t IR_TRANSFER_PIN = GPIO_NUM_13;
    static constexpr uint32_t ALARM_DURATION_SEC = 90;

    
    while (true)
    {
        try 
        {
            LEDTransmitter ir_transmitter(IR_TRANSFER_PIN);
            WiFi wifi(SSID, PASSWORD);
            
            static constexpr uint32_t WAIT_FOR_CONNECTION = 0;
            if (!wifi.connect_and_sync(WAIT_FOR_CONNECTION))
            {
                printf("Failed to connect to WiFi or sync time\n");
                return;
            }

            printf("Connected to WiFi and synced time!\n");

            // Disabled it because when program crashes and reboots this will run again (maybe im asleep)
            //start_notification(ir_transmitter);
            //turn_off(ir_transmitter);
            
            HTTPClient client;
            
            static constexpr uint32_t INITIAL_DELAY_SEC = 1;
            while (true)
            {
                bool has_alert = check_alert(client, std::string(CITY));
                if (!has_alert)
                {
                    printf("City NOT found: %s\n", CITY.data());
                    sleep(INITIAL_DELAY_SEC);
                    continue;
                }

                printf("City FOUND: %s\n", CITY.data());
                
                start_alarm(ir_transmitter, ALARM_DURATION_SEC);

                turn_off(ir_transmitter);

                sleep(INITIAL_DELAY_SEC);
            }
        }
        catch (esp_err_t err) {
            printf("ERROR: crashed with esp error code %d: %s\n", err, esp_err_to_name(err));
        }
        catch (IRException ex) {
            printf("ERROR: crashed with IR error code %d\n", static_cast<uint16_t>(ex.get()));
        }
        catch (Exception ex) {
            printf("ERROR: crashed with code %d\n", static_cast<uint16_t>(ex.get()));
        }
        catch (...)
        {
            printf("ERROR: crashed with unknown error\n");
        }

    }
}