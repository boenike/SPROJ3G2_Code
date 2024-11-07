/**
 * Title: Semester Project 3 Group 2 - Car code
 * Institution: University of Southern Denmark (SDU)
 * Campus: Sonderborg
 * File: SPROJ3G2_Car.c
 * Authors: Bence Toth and Iliya Iliev
 * Date: 07/11/2024
 * Course: BEng in Electronics
 * Semester: 3rd
 * Platform: RP2040
 * RF module: nRF24L01+
 * OLED module: SSD1306 128x32 I2C
 * RF library:   https://github.com/andyrids/pico-nrf24
 */

// Include necessary libraries
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "nrf24_driver.h"
#include "functions.h"
#include "servo_control.h"
#include "hardware/i2c.h"
//#include "ssd1306.h"
//#include "hardware/timer.h"
//#include "hardware/uart.h"
//#include "hardware/irq.h"
//#include "pico/multicore.h"

payload_t Payload = { .direction = 1 , .servo_angle = INIT_ANGLE } ;

uint8_t car_stopped = 0 ;
uint32_t currentTime , prevTime = 0 ;

nrf_client_t RF24 ;
fn_status_t success ;    // Result of packet transmission

int main ( void ) {

    hard_assert ( stdio_init_all ( ) ) ;    // Initialize all present standard stdio types

    gpio_init ( PICO_DEFAULT_LED_PIN ) ;
    gpio_set_dir ( PICO_DEFAULT_LED_PIN , GPIO_OUT ) ;
    gpio_put ( PICO_DEFAULT_LED_PIN , 0 ) ;

    servo_init ( SERVO_PIN ) ;

    uint16_t rf_setup = nRF24_Setup ( &RF24 , &RF_Pins , &RF_Config , ACK_OFF ,
        PRX , sizeof ( payload_t ) , ( data_pipe_t ) payload_pipe , PAYLOAD_ADDRESS ) ;
    hard_assert ( rf_setup == RF_SETUP_OK ) ;

    while ( 1 ) {

        currentTime = to_ms_since_boot ( get_absolute_time ( ) ) ;

        // If the time delay between two consecutive data packets is
        // greater than the predetermined value, set the car into initial conditions

        if ( ( currentTime - prevTime ) > INTERVAL_LIMIT && !car_stopped ) {
            car_stopped = 1 ;
            set_servo_angle(INIT_ANGLE , SERVO_PIN ) ;
            gpio_put ( PICO_DEFAULT_LED_PIN , 1 ) ;
        }

        if ( RF24.is_packet ( &payload_pipe ) ) {
            car_stopped = 0 ;
            success = RF24.read_packet ( &Payload , sizeof ( payload_t ) ) ;
            prevTime = to_ms_since_boot ( get_absolute_time ( ) ) ;
            gpio_put ( PICO_DEFAULT_LED_PIN , Payload.direction ) ;
            set_servo_angle ( Payload.servo_angle , SERVO_PIN ) ;
        }
    }
    return 0 ;
}