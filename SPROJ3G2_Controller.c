/**
 * Title: Semester Project 3 Group 2 - Controller code
 * Institution: University of Southern Denmark (SDU)
 * Campus: Sonderborg
 * File: SPROJ3G2_Controller.c
 * Author: Bence Toth
 * Date: 10/10/2024
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
#include "hardware/spi.h"
#include "nrf24_driver.h"
#include "functions.h"
//#include "hardware/i2c.h"
//#include "hardware/timer.h"
//#include "hardware/uart.h"
//#include "hardware/irq.h"
//#include "pico/multicore.h"

uint8_t ADC_Pins [ ] = { 26 , 27 } ;
uint16_t Pot_X_Val ;
payload_t Payload = { 1 , INIT_ANGLE } ;
nrf_client_t RF24 ;
fn_status_t success ;    // Result of packet transmission

int main ( void ) {

    stdio_init_all ( ) ;

    uint8_t adc_setup = ADC_Setup ( ADC_Pins , sizeof ( ADC_Pins ) ) ;
    hard_assert ( adc_setup == 1 ) ;

    nRF24_Setup ( &RF24 , &RF_Pins , &RF_Config , SPI_BAUDRATE , sizeof ( payload_t ) , RF24_TX , DYNPD_DISABLE , RF_ADDRESS , DATA_PIPE_0 ) ;

    while ( 1 ) {
        Pot_X_Val = read_ADC ( POT_X_PIN ) ;

        if ( Pot_X_Val >= HIGH_LIMIT || Pot_X_Val <= LOW_LIMIT ) {
            Payload.servo_angle = ( uint8_t ) convertInterval ( ( int32_t ) Pot_X_Val , ADC_MIN , ADC_MAX , MIN_ANGLE , MAX_ANGLE ) ;
            Payload.direction = ( Pot_X_Val > THRESHOLD ) ? 1 : 0 ;
        }

        else {
            Payload.servo_angle = INIT_ANGLE ;
            Payload.direction = 1 ;
        }

        printf ( "ADC: %u <-> Angle: %u\n" , Pot_X_Val , Payload.servo_angle ) ;
        // send packet to receiver's DATA_PIPE_0 address
        success = RF24.send_packet ( &Payload , sizeof ( Payload ) ) ;

        //sleep_ms ( FREEZE ) ;
    }

    return 0 ;
}