#include <cstddef>
#include <cstring>
#include <stdio.h>

#include "Exception.hpp"
#include "IRCommand.hpp"
#include "driver/gpio.h"
#include "esp_err.h"
#include "LEDTransmitter.hpp"
#include "IRReceiver.hpp"

#include "WiFi.hpp"
#include "HTTPClient.hpp"
#include "WifiCreds.h"


extern "C" void app_main(void)
{
    try 
    {
        WiFi wifi(SSID, PASSWORD);
        printf("Connected to WiFi!\n");

        HTTPClient http;
        std::string response = http.get("https://example.com");
        printf("%s\n", response.c_str());
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
        printf("ERROR: crashed with unknown error");
    }
}
