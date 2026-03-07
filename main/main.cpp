#include <cstddef>
#include <stdio.h>
#include <sys/select.h>
#include <sys/unistd.h>

#include "Exception.hpp"
#include "IRCommand.hpp"
#include "IRReceiverIter.hpp"
#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LEDTransmitter.hpp"
#include "IRReceiver.hpp"

#include <sys/time.h>

static constexpr gpio_num_t IR_RECV_PIN = GPIO_NUM_22;
static constexpr gpio_num_t IR_SEND_PIN = GPIO_NUM_13;

void receive_loop(IRReceiverIter& iter)
{   
    printf("Receiving is up\n");
    while (true)
    {
        IRCommand cmd = iter.receive();
        printf("Received command: %s\n", cmd.str().c_str());
    }
}

void send_loop(LEDTransmitter& transmitter)
{
    printf("Transmitting is up\n");
    while (true)
    {
        transmitter.send(LEDProtocol::POWER_OFF);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}


extern "C" void app_main(void)
{
    try 
    {
        IRReceiver recv(IR_RECV_PIN);
        LEDTransmitter transmitter(IR_SEND_PIN);
        printf("IR Transmitter and Receiver initialized successfully\n");
        
        send_loop(transmitter);
        //receive_loop(recv.get_receiver());
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
