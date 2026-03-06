idf_component_register(SRCS "IRCommand.cpp" "main.cpp" "RMTChannel.cpp" "IRTransmitter.cpp" "IRReceiverIter.cpp" "IRReceiver.cpp" "LEDTransmitter.cpp" "WiFi.cpp" "HTTPClient.cpp" "Utils.cpp"
                    INCLUDE_DIRS "." "../managed_components/dieortin__esp32-buzzer/include"
                    REQUIRES dieortin__esp32-buzzer esp_wifi esp_netif esp_http_client driver nvs_flash esp_timer esp_event)
