#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
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

#include "buzzer/buzzer.h"
#include "melodies/erika_melody.h"

extern "C" void app_main(void)
{
    while (true)
    {
        try 
        {
            buzzer_t* buzzer = buzzer_init(LEDC_CHANNEL_0, LEDC_TIMER_0, GPIO_NUM_12);

            buzzer_play_melody(buzzer, &erika_melody, 50);
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