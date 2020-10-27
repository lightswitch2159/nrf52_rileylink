# setup notes

initial dev using pca0040
nRF5_SDK_15.3.0_59ac345

# install arm armgcc
```
brew tap PX4/homebrew-px4
brew update
brew install gcc-arm-none-eabi
```

# Get NRF5 command line tools (nrfjprog)
https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools/Download

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
