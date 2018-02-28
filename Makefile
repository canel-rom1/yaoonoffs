#CHIP        = esp8266
BOARD       = nodemcuv2
UPLOAD_PORT = /dev/ttyUSB0

BUILD_DIR = ./build-$(CHIP)-$(BOARD)
ESP_ROOT = /home/romain/programs/esp8266Make/Arduino
#ARDUINO_LIBS = /home/romain/arduino/libraries
CUSTOM_LIBS = /home/romain/programs/esp8266Make/lib

include /home/romain/programs/esp8266Make/makeEspArduino/makeEspArduino.mk
