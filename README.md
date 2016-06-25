# RFM70 RF module based projects for AVR
These are projects based on RFM70 radio modules. Basic Tx and Rx examples show Transmit and Receive functionality. This template can be adapted for any project. There is a configuration header file (rfm70_config.h), which should be modified for the project. Similarly modify Makefile.


The examples are targeted for AVR ATmega8 MCU, but can be run on any AVR having an SPI interface and one external interrupt pin. The library uses INT1 as interrupt


NOTE:
For compiling these projects, library for RFM70 from 'common_libs' repository is needed. 
