/**
 * Title: Semester Project 3 Group 2 - Controller code
 * Institution: University of Southern Denmark (SDU)
 * Campus: Sonderborg
 * File: SPROJ3G2_Controller.c
 * Authors: Bence Toth and Iliya Iliev
 * Date: 28/10/2024
 * Course: BEng in Electronics
 * Semester: 3rd
 * Platform: RP2040
 * RF module: nRF24L01+
 * OLED module: SSD1306 128x32 I2C
 * RF library:   https://github.com/andyrids/pico-nrf24
 * OLED library: https://github.com/daschr/pico-ssd1306
 */

// Include necessary libraries
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "nrf24_driver.h"
#include "functions.h"
#include "ssd1306.h"
//#include "hardware/timer.h"
//#include "hardware/uart.h"
//#include "hardware/irq.h"
//#include "pico/multicore.h"

uint8_t ADC_Pins [ ] = { 26 , 27 } ;
uint16_t Pot_X_Val ;

payload_t Payload = { .direction = 1 , .servo_angle = INIT_ANGLE } ;
echo_t Echo = { .echo = 0 } ;

//ssd1306_t OLED ;
nrf_client_t RF24 ;
fn_status_t success ;    // Result of packet transmission

const uint16_t THRESHOLD  = ADC_MAX / 2 ;
const uint16_t LOW_LIMIT  = THRESHOLD - TOLERANCE ;
const uint16_t HIGH_LIMIT = THRESHOLD + TOLERANCE ;

int main ( void ) {

    stdio_init_all ( ) ;

    uint8_t adc_setup = ADC_Setup ( ADC_Pins , sizeof ( ADC_Pins ) ) ;
    hard_assert ( adc_setup == 1 ) ;

    //OLED_Setup ( &OLED_Pins , &OLED ) ;

    nRF24_Setup ( &RF24 , &RF_Pins , &RF_Config , SPI_BAUDRATE , DYNPD_DISABLE ) ;
    nRF24_Comm_Dir_Setup ( &RF24 , RF24_TX , sizeof ( payload_t ) , sizeof ( echo_t ) , ( data_pipe_t ) payload_pipe , ( data_pipe_t ) echo_pipe , PAYLOAD_ADDRESS , ECHO_ADDRESS ) ;

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

        // send packet to receiver's payload address
        success = RF24.send_packet ( &Payload , sizeof ( Payload ) ) ;
    }
    return 0 ;
}