#!/bin/bash

export SDK_PATH=$(pwd)/esp8266-rtos-sdk-hilink
export BIN_PATH=$(pwd)/bin
echo -e "\033[32m----------------------"
echo "SDK_PATH: $SDK_PATH"
echo "BIN_PATH: $BIN_PATH"
echo -e "----------------------\033[00m"
if [ ! -d "bin" ]; then
  mkdir bin
fi

boot=new
app=1
spi_speed=40
spi_mode=QIO
spi_size_map=5

echo -e "\033[32m----------------------"
echo "boot mode    : $boot"
echo "generate bin : user1.bin"
echo "spi speed    : 40MHz"
echo "spi mode     : QIO"
echo "spi size     : 2048KB"
echo "spi ota map  : 1024 + 1024KB"
echo -e "----------------------\033[00m"

rm bin/* -rf
cd platforms
make
cd -
make clean
make BOOT=$boot APP=$app SPI_SPEED=$spi_speed SPI_MODE=$spi_mode SPI_SIZE_MAP=$spi_size_map
# copy bin
cp esp8266-rtos-sdk-hilink/bin/*.bin bin/
