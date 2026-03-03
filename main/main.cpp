#include <cstddef>
#include <stdio.h>
#include <sys/select.h>
#include <sys/unistd.h>

#include "Exception.hpp"
#include "IRCommand.hpp"
#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LEDTransmitter.hpp"
#include "IRReceiver.hpp"

#include <sys/time.h>

static constexpr gpio_num_t IR_TRANSFER_PIN = GPIO_NUM_13;

void turn_on_max(LEDTransmitter& led)
{
    led.send_multiple(LEDProtocol::POWER_ON, 30);

    static constexpr uint16_t MAX_BRIGHTNESS = 12;
    static constexpr uint16_t DELAY_BETWEEN_SIGNALS = 70;
    led.send_multiple(LEDProtocol::BRIGHT_UP, MAX_BRIGHTNESS, DELAY_BETWEEN_SIGNALS);
}

void blink(LEDTransmitter& led)
{
    static constexpr uint64_t LED_BREAK_TIME_US = 10000;

    led.send_multiple(LEDProtocol::WHITE_LIGHT, 2);
    usleep(LED_BREAK_TIME_US);
    
    led.send_multiple(LEDProtocol::RED_LIGHT, 2);
    usleep(LED_BREAK_TIME_US);
    
    led.send_multiple(LEDProtocol::GREEN_LIGHT, 2);
    usleep(LED_BREAK_TIME_US);

    led.send_multiple(LEDProtocol::YELLOW_LIGHT, 2);
    usleep(LED_BREAK_TIME_US);
}

extern "C" void app_main(void)
{
    try 
    {
        LEDTransmitter ir_transmitter(IR_TRANSFER_PIN);
                
        printf("IR Transmitter and Receiver initialized successfully\n");
        
        turn_on_max(ir_transmitter);

        while (true)
        {
            blink(ir_transmitter);
        }
    
    } 
    catch (EspException ex) {
        printf("ERROR: crashed with esp error code %d: %s\n", static_cast<uint16_t>(ex.get()), esp_err_to_name(ex.get()));
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
