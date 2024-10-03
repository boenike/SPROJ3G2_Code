/**
 * Title: Semester Project 3 Group 2 - Controller code
 * Institution: University of Southern Denmark (SDU)
 * Campus: Sonderborg
 * File: SPROJ3G2_Controller.c
 * Author: Bence Toth
 * Date: 03/10/2024
 * Course: BEng in Electronics
 * Semester: 3rd
 * Platform: RP2040
 * RF module: nRF24L01+
 * RF Library: https://github.com/andyrids/pico-nrf24
 */

// Include necessary libraries
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "nrf24l01/nrf24_driver.h"
//#include "hardware/spi.h"
//#include "hardware/i2c.h"
//#include "hardware/timer.h"
//#include "hardware/uart.h"
//#include "hardware/irq.h"
//#include "pico/multicore.h"

#define ADC_PIN 26
#define ADC_INPUT 0
#define FREEZE 100
#define SPI_BAUDRATE 4000000    // 4 MHz
#define RF_CHANNEL 110          // 2.51 GHz

void ADC_Setup ( uint8_t pin , uint8_t input ) {
    adc_init ( ) ;
    adc_gpio_init ( pin ) ;             // Make sure GPIO is high-impedance, no pullups etc...
    adc_select_input ( input ) ;        // Select ADC input
}

int main ( ) {

    //const uint8_t address [ ] = { 0x37 , 0x37 , 0x37 , 0x37 , 0x37 } ;
    uint16_t payload ;
    
    stdio_init_all ( ) ;
    ADC_Setup ( ADC_PIN , ADC_INPUT ) ;

    // GPIO pin numbers
    pin_manager_t RF_Pins = { 
        .sck = 2,   // Serial Clock
        .copi = 3,  // Master Out - Slave In
        .cipo = 4,  // Master In - Slave Out
        .csn = 5,   // Chip Select Not
        .ce = 6     // Chip Enable
    } ;

    nrf_manager_t RF_Config = {
        // RF Channel 
        .channel = RF_CHANNEL ,

        // AW_3_BYTES, AW_4_BYTES, AW_5_BYTES
        .address_width = AW_5_BYTES ,

        // dynamic payloads: DYNPD_ENABLE, DYNPD_DISABLE
        .dyn_payloads = DYNPD_DISABLE ,

        // data rate: RF_DR_250KBPS, RF_DR_1MBPS, RF_DR_2MBPS
        .data_rate = RF_DR_2MBPS ,

        // RF_PWR_NEG_18DBM, RF_PWR_NEG_12DBM, RF_PWR_NEG_6DBM, RF_PWR_0DBM
        .power = RF_PWR_0DBM ,

        // retransmission count: ARC_NONE...ARC_15RT
        .retr_count = ARC_NONE ,

        // retransmission delay: ARD_250US, ARD_500US, ARD_750US, ARD_1000US
        .retr_delay = ARD_250US 
    } ;

    nrf_client_t RF24 ;

    nrf_driver_create_client ( &RF24 ) ;

    RF24.configure ( &RF_Pins , SPI_BAUDRATE ) ;

    RF24.initialise ( &RF_Config ) ;

    RF24.payload_size ( DATA_PIPE_0 , sizeof ( payload ) ) ;

    RF24.dyn_payloads_disable ( ) ; // Dynamic payloads disabled

    RF24.standby_mode ( ) ;     // TX mode

    fn_status_t success = 0;    // Result of packet transmission

    while ( 1 ) {
        payload = adc_read ( ) ;
        
        // send to receiver's selected pipe address
        RF24.tx_destination ( ( const uint8_t [ ] ) { 0x37 , 0x37 , 0x37 , 0x37 , 0x37 } ) ;

        // send packet to receiver's DATA_PIPE_0 address
        success = RF24.send_packet ( &payload , sizeof ( payload ) ) ;
        //printf ( "RF PACKET SENT\n" ) ;

        sleep_ms ( FREEZE ) ;
    }
    return 0 ;
}