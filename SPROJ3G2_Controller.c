/**
 * Title:           Semester Project 3 Group 2 - Controller PTX source code
 * Institution:     University of Southern Denmark (SDU)
 * Campus:          Sønderborg
 * File:            SPROJ3G2_Controller.c
 * Authors:         Bence Tóth
 * Date:            16/12/2024
 * Course:          BEng in Electronics
 * Semester:        3rd
 * Platform:        Raspberry Pi RP2040 C SDK
 * RF module:       nRF24L01 PA/LNA
 * OLED module:     SSD1306 128x32 I2C
 * RF library:      https://github.com/andyrids/pico-nrf24
 * OLED library:    https://github.com/daschr/pico-ssd1306
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
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "nrf24_driver.h"
#include "ssd1306.h"
#include "functions.h"

uint8_t ADC_Pins [ ] = { 26 , 27 } ;
uint8_t retr_lim_reached = 0 , RT_count = 0 ;
volatile uint8_t car_connected = 0 , charge_state = 0 , rec_val , charge_print = 0 ;

// Initial values - middlepoint for servo - forward mode and halt for the motor
payload_t Payload = { .servo_angle = INIT_ANGLE , .speed_and_direction = 0 } ;

ssd1306_t OLED ;
nrf_client_t RF24 ;
fn_status_t success ;    // Result of packet transmission

void UART_RX_ISR ( void ) {
    if ( uart_is_readable ( UART_ID ) ) {
        rec_val = uart_getc ( UART_ID ) ;
        charge_state = ( rec_val ) ? 1 : 0 ;
        charge_print = 1 ;
    }
}

int main ( void ) {

    hard_assert ( stdio_init_all ( ) ) ;

    hard_assert ( UART_Setup ( UART_ID , UART_BAUDRATE , UART_RX_PIN , UART_TX_PIN ) ) ;

    Setup_UART_RX_IRQ ( UART_ID , UART_RX_ISR ) ;

    hard_assert ( ADC_Setup ( ADC_Pins , sizeof ( ADC_Pins ) ) ) ;

    hard_assert ( OLED_Setup ( &OLED_Pins , &OLED ) ) ;

    uint16_t rf_setup = nRF24_Setup ( &RF24 , &RF_Pins , &RF_Config , ACK_ON ,
        PTX , sizeof ( payload_t ) , ( data_pipe_t ) payload_pipe , PAYLOAD_ADDRESS ) ;
        
    hard_assert ( rf_setup == RF_SETUP_OK ) ;

    while ( 1 ) {
        set_Payload_Data ( &Payload , POT_X_PIN , POT_Y_PIN ) ;

        // Send the packet to the Receiver's payload address
        success = RF24.send_packet ( &Payload , sizeof ( payload_t ) ) ;

        if ( charge_print ) {
            update_Car_Status ( &OLED , car_connected , charge_state ) ;
            charge_print = 0 ; }

        switch ( success ) {
            case NRF_MNGR_OK :
                if ( !car_connected ) {
                    car_connected = 1 ;
                    retr_lim_reached = 0 ;
                    update_Car_Status ( &OLED , car_connected , charge_state ) ;
                }
                break ;
            case ERROR :
                if ( !retr_lim_reached ) {
                    RT_count++ ;
                    if ( RT_count == MAX_RT_TRY ) {
                        retr_lim_reached = 1 ;
                        RT_count = 0 ;
                        car_connected = 0 ;
                        update_Car_Status ( &OLED , car_connected , charge_state ) ;
                    }
                }
                break ;
            default : break ;
        }
    }
    return 0 ;
}