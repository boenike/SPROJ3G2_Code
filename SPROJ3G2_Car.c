/**
 * Title: Semester Project 3 Group 2 - Car code
 * Institution: University of Southern Denmark (SDU)
 * Campus: Sonderborg
 * File: SPROJ3G2_Car.c
 * Author: Bence Toth and Iliya Iliev
 * Date: 22/10/2024
 * Course: BEng in Electronics
 * Semester: 3rd
 * Platform: RP2040
 * RF module: nRF24L01+
 * RF library: https://github.com/andyrids/pico-nrf24
 */

// Include necessary libraries
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "nrf24_driver.h"
#include "functions.h"
#include "servo_control.h"
//#include "hardware/i2c.h"
//#include "hardware/timer.h"
//#include "hardware/uart.h"
//#include "hardware/irq.h"
//#include "pico/multicore.h"

uint8_t rx_pipe = 0 ;
payload_t Payload = { 1 , INIT_ANGLE } ;
nrf_client_t RF24 ;

int main ( void ) {

    stdio_init_all ( ) ;    // Initialize all present standard stdio types

    gpio_init ( PICO_DEFAULT_LED_PIN ) ;
    gpio_set_dir ( PICO_DEFAULT_LED_PIN , GPIO_OUT ) ;

    servo_init ( SERVO_PIN ) ;

    nRF24_Setup ( &RF24 , &RF_Pins , &RF_Config , SPI_BAUDRATE , sizeof ( payload_t ) , RF24_RX , DYNPD_DISABLE , RF_ADDRESS , DATA_PIPE_0 ) ;

    while ( 1 ) {
        if ( RF24.is_packet ( &rx_pipe ) ) {
            RF24.read_packet ( &Payload , sizeof ( payload_t ) ) ;

            gpio_put ( PICO_DEFAULT_LED_PIN , Payload.direction ) ;
            set_servo_angle ( Payload.servo_angle , SERVO_PIN ) ;
        }
    }
    return 0 ;
}