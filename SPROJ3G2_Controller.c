/**
 * Title: Semester Project 3 Group 2 - Controller code
 * Institution: University of Southern Denmark (SDU)
 * Campus: Sonderborg
 * File: SPROJ3G2_Controller.c
 * Author: Bence Toth
 * Date: 09/10/2024
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

#define POT_X 26
#define POT_X_INPUT 0
#define FREEZE 20
#define INIT_ANGLE 90
#define MAX_ANGLE 180
#define MIN_ANGLE 0
#define ADC_MIN 0
#define ADC_MAX 4096
#define TOLERANCE 50
#define THRESHOLD ( ADC_MAX / 2 )
#define LOW_LIMIT ( THRESHOLD - TOLERANCE )
#define HIGH_LIMIT ( THRESHOLD + TOLERANCE )
#define SPI_BAUDRATE 4000000    // 4 MHz SPI baudrate
#define RF_CHANNEL 110          // 2.51 GHz ISM frequency band

typedef struct {
    uint8_t direction ;
    uint8_t servo_angle ;
} payload_t ;

int32_t convertInterval ( int32_t x , int32_t in_min , int32_t in_max , int32_t out_min , int32_t out_max ) {
  return ( x - in_min ) * ( out_max - out_min ) / ( in_max - in_min ) + out_min ;
}

void ADC_Setup ( uint8_t pin , uint8_t input ) {
    adc_init ( ) ;
    adc_gpio_init ( pin ) ;             // Make sure GPIO is high-impedance, no pullups etc...
    adc_select_input ( input ) ;        // Select ADC input
}

int main ( void ) {

    //const uint8_t address [ ] = { 0x37 , 0x37 , 0x37 , 0x37 , 0x37 } ;
    uint16_t adc_val ;
    payload_t Payload = { 1 , INIT_ANGLE } ;
    
    stdio_init_all ( ) ;
    ADC_Setup ( POT_X , POT_X_INPUT ) ;

    // GPIO pin numbers for nRF24L01
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

    RF24.payload_size ( DATA_PIPE_0 , sizeof ( Payload ) ) ;

    RF24.dyn_payloads_disable ( ) ; // Dynamic payloads disabled

    RF24.standby_mode ( ) ;     // TX mode

    fn_status_t success ;    // Result of packet transmission

    // send to receiver's selected pipe address
    RF24.tx_destination ( ( const uint8_t [ ] ) { 0x37 , 0x37 , 0x37 , 0x37 , 0x37 } ) ;

    while ( 1 ) {
        adc_val = adc_read ( ) ;

        if ( adc_val >= HIGH_LIMIT || adc_val <= LOW_LIMIT ) {
            Payload.servo_angle = ( uint8_t ) convertInterval ( ( int32_t ) adc_val , ADC_MIN , ADC_MAX , MIN_ANGLE , MAX_ANGLE ) ;
            Payload.direction = ( adc_val > THRESHOLD ) ? 1 : 0 ;
        }

        else {
            Payload.servo_angle = INIT_ANGLE ;
            Payload.direction = 1 ;
        }

        //printf ( "ADC: %d\n" , adc_val ) ;
        // send packet to receiver's DATA_PIPE_0 address
        success = RF24.send_packet ( &Payload , sizeof ( Payload ) ) ;

        //sleep_ms ( FREEZE ) ;
    }
    return 0 ;
}