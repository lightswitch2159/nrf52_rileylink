# setup notes

initial dev using pca0040

# spi channel on ble113
<usart channel="0" mode="spi_master" alternate="1" polarity="negative" phase="0" endianness="lsb" baud="57200" endpoint="none" />

# Debug logs
JLinkExe -if SWD (then type `connect` and accept defaults)
JLinkRTTClient

# subg_rfspy notes

cc-tool (https://github.com/dashesy/cc-tool)
brew install autoconf
brew install automake
brew install libtool
brew install libusb
brew install pkg-config

# Run
cd pca10056/s140/armgcc
