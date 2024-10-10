/**
 * Title: Semester Project 3 Group 2 - Car code
 * Institution: University of Southern Denmark (SDU)
 * Campus: Sonderborg
 * File: SPROJ3G2_Car.c
 * Author: Bence Toth
 * Date: 10/10/2024
 * Course: BEng in Electronics
 * Semester: 3rd
 * Platform: RP2040
 * RF module: nRF24L01+
 * RF library: https://github.com/andyrids/pico-nrf24
 * Servo library: https://www.codeproject.com/Articles/5360397/Raspberry-Pi-Pico-library-for-working-with-servos
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
#include "servo.h"
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

    rc_servo Servo = rc_servo_init ( SERVO_PIN ) ;
    rc_servo_start ( &Servo , INIT_ANGLE ) ;

    nRF24_Setup ( &RF24 , &RF_Pins , &RF_Config , SPI_BAUDRATE , sizeof ( payload_t ) , RF24_RX , DYNPD_DISABLE , RF_ADDRESS , DATA_PIPE_0 ) ;

    while ( 1 ) {
        if ( RF24.is_packet ( &rx_pipe ) ) {
            RF24.read_packet ( &Payload , sizeof ( payload_t ) ) ;

            gpio_put ( PICO_DEFAULT_LED_PIN , Payload.direction ) ;
            rc_servo_set_angle ( &Servo , ( uint32_t ) Payload.servo_angle ) ;
        }
    }
    return 0 ;
}