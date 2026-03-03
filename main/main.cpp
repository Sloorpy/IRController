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

static constexpr gpio_num_t IR_TRANSFER_PIN = GPIO_NUM_22;
static constexpr gpio_num_t IR_RECV_PIN = GPIO_NUM_23;

extern "C" void app_main(void)
{
    try 
    {
        LEDTransmitter ir_transmitter(IR_TRANSFER_PIN);
        //IRReceiver ir_receiver(IR_RECV_PIN);

        
        printf("IR Transmitter and Receiver initialized successfully\n");
        
        //IRReceiverIter& iter = ir_receiver.get_receiver();

        static constexpr uint64_t LED_BREAK_TIME_US = 500; // 0.0005 sec
        while (true)
        {
            ir_transmitter.send(LEDProtocol::RED_LIGHT);
            usleep(LED_BREAK_TIME_US);

            ir_transmitter.send(LEDProtocol::BLUE_LIGHT);
            usleep(LED_BREAK_TIME_US);
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
