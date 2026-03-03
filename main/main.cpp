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

static constexpr gpio_num_t IR_TRANSFER_PIN = GPIO_NUM_33;
static constexpr gpio_num_t IR_RECV_PIN = GPIO_NUM_32;

extern "C" void app_main(void)
{
    try 
    {
        LEDTransmitter ir_transmitter(IR_TRANSFER_PIN);
        
        static constexpr bool ACCEPT_INVALID_SIGNAL = false;
        IRReceiver ir_receiver(IR_RECV_PIN, ACCEPT_INVALID_SIGNAL);
        IRReceiverIter& iter = ir_receiver.get_receiver();
        
        printf("IR Transmitter and Receiver initialized successfully\n");
        
        ir_transmitter.send_multiple(LEDProtocol::POWER_ON, 1);


        static constexpr uint64_t LED_BREAK_TIME_US = 500; // 0.0005 sec
        while (true)
        { 
            ir_transmitter.send_multiple(LEDProtocol::RED_LIGHT, 1);
            printf("%s\n", iter.receive().str().c_str());
            usleep(LED_BREAK_TIME_US);

            ir_transmitter.send_multiple(LEDProtocol::BLUE_LIGHT, 1);
            printf("%s\n", iter.receive().str().c_str());
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
