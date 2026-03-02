#include <stdio.h>
#include <sys/unistd.h>
#include <algorithm>

#include "Exception.hpp"
#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "IRTransmitter.hpp"
#include "IRReceiver.hpp"

static constexpr gpio_num_t IR_TRANSFER_PIN = GPIO_NUM_22;
static constexpr gpio_num_t IR_RECV_PIN = GPIO_NUM_23;


extern "C" void app_main(void)
{
    try 
    {
        IRTransmitter ir_transmitter(IR_TRANSFER_PIN);
        IRReceiver ir_receiver(IR_RECV_PIN);

        
        printf("IR Transmitter and Receiver initialized successfully\n");
        
        IRReceiverIter& iter = ir_receiver.get_receiver();

        while (true)
        {
            IRCommand cmd = iter.receive();

            printf("%s\n", cmd.str().c_str());
        }
    
    } 
    catch (Exception ex) {
        printf("ERROR: crashed with code %d\n", static_cast<uint16_t>(ex.get()));
    }
    catch (...)
    {
        printf("ERROR: crashed with unknown error");
    }
}
