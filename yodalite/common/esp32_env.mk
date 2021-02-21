        export CFLAGS += -mlongcalls \
 			   -I$(PRJ_DIR)/../vendor/esp32/esp-adf/esp-idf/components/mbedtls/mbedtls/include \
               -I$(PRJ_DIR)/../vendor/esp32/esp-adf/esp-idf/components/lwip/port/esp32/include \
	 	       -I$(PRJ_DIR)/lib/FreeRTOS/portable/GCC/ESP32/include                            \
               -I$(PRJ_DIR)/include/lib                                                        
