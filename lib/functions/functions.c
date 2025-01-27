/**
 * Title:           Semester Project 3 Group 2 - Functions source code
 * Institution:     University of Southern Denmark (SDU)
 * Campus:          Sønderborg
 * File:            functions.c
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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "nrf24_driver.h"
#include "ssd1306.h"
#include "functions.h"

#define RF_HI   // Choose between RF_HI or RF_LO -> depending on whether you want to use the PA+LNA or the standard nRF24L01 models

// Address of Data Pipe 0 - based on the datasheet of nRF24L01
const uint8_t *PAYLOAD_ADDRESS = ( const uint8_t [ ] ) { 0xE7 , 0xD3 , 0xF0 , 0x35 , 0x77 } ;
uint8_t payload_pipe = 0 ;  // Selected payload pipe

//GPIO pin numbers for I2C - SSD1306 OLED display module
oled_pins_t OLED_Pins = {
    .SDA = 8 ,  // Serial Data
    .SCL = 9    // Serial Clock
} ;

// GPIO pin numbers for SPI - nRF24L01 RF module
pin_manager_t RF_Pins = { 
    .sck  = 2 ,  // Serial Clock
    .copi = 3 ,  // Master Out - Slave In
    .cipo = 4 ,  // Master In - Slave Out
    .csn  = 5 ,  // Chip Select Not
    .ce   = 6    // Chip Enable
} ;

// RF module configurations
nrf_manager_t RF_Config = {
    // RF Channel in the 2.4GHz ISM band
    // Setting different channels based on which specific RF model is used (nRF24L01 or nRF24L01+PA/LNA)

    #ifdef RF_HI
    .channel = RF_CHANNEL_HI ,
    #endif

    #ifdef RF_LO
    .channel = RF_CHANNEL_LO ,
    #endif

    // Address Width: AW_3_BYTES, AW_4_BYTES, AW_5_BYTES
    .address_width = AW_5_BYTES ,

    // Dynamic Payloads: DYNPD_ENABLE, DYNPD_DISABLE
    .dyn_payloads = DYNPD_DISABLE ,

    // Air Data Rate: RF_DR_250KBPS, RF_DR_1MBPS, RF_DR_2MBPS
    .data_rate = RF_DR_1MBPS ,

    // Output Power: RF_PWR_NEG_18DBM, RF_PWR_NEG_12DBM, RF_PWR_NEG_6DBM, RF_PWR_0DBM
    .power = RF_PWR_NEG_12DBM ,

    // Retransmission Count: ARC_NONE...ARC_15RT
    .retr_count = ARC_10RT ,

    // Retransmission Delay: ARD_250US, ARD_500US, ARD_750US, ARD_1000US
    .retr_delay = ARD_750US
} ;

// Function to convert a value from one range of values to another
int32_t convertInterval ( int32_t x , int32_t in_min , int32_t in_max , int32_t out_min , int32_t out_max ) {
  return ( x - in_min ) * ( out_max - out_min ) / ( in_max - in_min ) + out_min ;
}

// Function to set up the required ADC pins whilst checking them for validity
uint8_t ADC_Setup ( uint8_t *input_pins , uint8_t input_length ) {
    uint8_t success = 0 ;
    adc_init ( ) ;

    // Make sure GPIO is high-impedance, no pullups etc...
    for ( uint8_t ctr = 0 ; ctr < input_length ; ctr++ ) {
        if ( input_pins [ ctr ] >= ADC_REF_PIN && input_pins [ ctr ] <= ADC_MAX_PIN ) {
            adc_gpio_init ( input_pins [ ctr ] ) ;
            success++ ;
        }
    }

    return ( success == input_length ) ? 1 : 0 ;
}

// Function to read the value of the desired ADC pin
uint16_t read_ADC ( uint8_t pin ) {
    adc_select_input ( pin - ADC_REF_PIN ) ;
    return adc_read ( ) ;
}

uint16_t nRF24_Setup ( nrf_client_t *RF24_ptr , pin_manager_t *RF24_pins_ptr , nrf_manager_t *RF24_config_ptr ,
    Auto_Ack_t ack_mode , RF_Mode_t mode , size_t payload_size , data_pipe_t payload_pipe , const uint8_t *payload_address )
{
    uint8_t requirement = 9 + ( uint8_t ) ack_mode ;
    uint16_t ret = 0 ;
    fn_status_t state ;
    retr_count_t ARC_count ;

    state = nrf_driver_create_client ( RF24_ptr ) ;
    if ( state == NRF_MNGR_OK ) { ret |= 1 << 0 ; }

    state = RF24_ptr->configure ( RF24_pins_ptr , SPI_BAUDRATE ) ;      // Set up the correct pinout and set the SPI baudrate
    if ( state == PIN_MNGR_OK ) { ret |= 1 << 1 ; }

    state = RF24_ptr->initialise ( RF24_config_ptr ) ;                  // Initialize the nRF24 module with the stated configuration
    if ( state == SPI_MNGR_OK ) { ret |= 1 << 2 ; }

    state = RF24_ptr->rf_data_rate ( RF24_config_ptr->data_rate ) ;     // Set the specified air transfer rate
    if ( state == SPI_MNGR_OK ) { ret |= 1 << 3 ; }

    state = RF24_ptr->rf_power ( RF24_config_ptr->power ) ;             // Set the specified antenna power
    if ( state == SPI_MNGR_OK ) { ret |= 1 << 4 ; }

    state = RF24_ptr->payload_size ( payload_pipe , payload_size ) ;    // Set the size of the payload message
    if ( state == SPI_MNGR_OK ) { ret |= 1 << 5 ; }

    switch ( mode ) {
        case PRX :
            state = RF24_ptr->rx_destination ( payload_pipe , payload_address ) ;
            if ( state == SPI_MNGR_OK ) { ret |= 1 << 6 ; }
            state = RF24_ptr->receiver_mode ( ) ;
            if ( state == SPI_MNGR_OK ) { ret |= 1 << 7 ; }
            break ;
        case PTX :
            state = RF24_ptr->tx_destination ( payload_address ) ;
            if ( state == SPI_MNGR_OK ) { ret |= 1 << 6 ; }
            state = RF24_ptr->standby_mode ( ) ;
            if ( state == NRF_MNGR_OK ) { ret |= 1 << 7 ; }
            break ;
        default : break ;
    }

    switch ( RF24_config_ptr->dyn_payloads ) {
        case DYNPD_ENABLE :
            state = RF24_ptr->dyn_payloads_enable ( ) ;
            if ( state == SPI_MNGR_OK ) { ret |= 1 << 8 ; }
            break ;

        case DYNPD_DISABLE :
            state = RF24_ptr->dyn_payloads_disable ( ) ;
            if ( state == NRF_MNGR_OK ) { ret |= 1 << 8 ; }
            break ;
        default : break ;
    }

    switch ( ack_mode ) {
        case ACK_OFF :
            ARC_count = ARC_NONE ;
            break ;
        case ACK_ON :
            ARC_count = RF24_config_ptr->retr_count ;
            break ;
        default :
            ARC_count = ARC_NONE ;
            break ;
    }

    // Enable Auto-ACK with the specified Retransmission delay and count
    state = RF24_ptr->auto_retransmission ( RF24_config_ptr->retr_delay , ARC_count ) ;
    if ( state == SPI_MNGR_OK ) { ret |= 1 << 9 ; }

    return ret ;
}

void set_Payload_Data ( payload_t *payload , uint8_t pot_x_pin , uint8_t pot_y_pin ) {
    uint16_t Pot_X_Val , Pot_Y_Val ;
    const uint16_t THRESHOLD  = ADC_MAX / 2 ;
    const uint16_t LOW_LIMIT_THRESH  = THRESHOLD - TOLERANCE ;
    const uint16_t HIGH_LIMIT_THRESH = THRESHOLD + TOLERANCE ;

    Pot_X_Val = read_ADC ( pot_x_pin ) ;
    Pot_Y_Val = read_ADC ( pot_y_pin ) ;

    if ( Pot_X_Val > HIGH_LIMIT_THRESH ) {
        payload->servo_angle = ( uint8_t ) convertInterval ( ( int32_t ) Pot_X_Val , ( int32_t ) HIGH_LIMIT_THRESH , ADC_MAX , INIT_ANGLE , MAX_ANGLE ) ;
    }

    else if ( Pot_X_Val < LOW_LIMIT_THRESH ) {
        payload->servo_angle = ( uint8_t ) convertInterval ( ( int32_t ) Pot_X_Val , ADC_MIN , ( int32_t ) LOW_LIMIT_THRESH , MIN_ANGLE , INIT_ANGLE ) ;
    }

    else if ( Pot_X_Val <= HIGH_LIMIT_THRESH && Pot_X_Val >= LOW_LIMIT_THRESH ) {  // Left thumbstick in center - set default values
        payload->servo_angle = INIT_ANGLE ;
    }

    if ( Pot_Y_Val > HIGH_LIMIT_THRESH ) {      // Forward
        payload->speed_and_direction = ( int8_t ) convertInterval ( ( int32_t ) Pot_Y_Val , ( int32_t ) HIGH_LIMIT_THRESH , ADC_MAX , HALT , SPEED_MAX_FORWARD ) ;
    }

    else if ( Pot_Y_Val < LOW_LIMIT_THRESH ) {  // Backward
        payload->speed_and_direction = ( int8_t ) convertInterval ( ( int32_t ) Pot_Y_Val , ADC_MIN , ( int32_t ) LOW_LIMIT_THRESH , SPEED_MAX_BACKWARD , HALT ) ;
    }

    else if ( Pot_Y_Val <= HIGH_LIMIT_THRESH && Pot_Y_Val >= LOW_LIMIT_THRESH ) { 
        // Right thumbstick in center - set default values - stop and forward direction
        payload->speed_and_direction = HALT ;
    }
}

uint8_t OLED_Setup ( oled_pins_t *oled_pins , ssd1306_t *oled_ptr ) {
    uint8_t setup_success = 0 ;
    i2c_inst_t *current_i2c_instance ;

    current_i2c_instance = ( oled_pins->SDA % 4 == 0 ) ? i2c0 : i2c1 ;

    uint __unused rate = i2c_init ( current_i2c_instance , I2C_BAUDRATE ) ;

    gpio_set_function ( oled_pins->SDA , GPIO_FUNC_I2C ) ;
    gpio_set_function ( oled_pins->SCL , GPIO_FUNC_I2C ) ;
    gpio_pull_up ( oled_pins->SDA ) ;
    gpio_pull_up ( oled_pins->SCL ) ;

    oled_ptr->external_vcc = false ;
    setup_success = ( uint8_t ) ssd1306_init ( oled_ptr , OLED_WIDTH , OLED_HEIGHT , OLED_ADDRESS , current_i2c_instance ) ;
    ssd1306_poweron ( oled_ptr ) ;
    ssd1306_clear ( oled_ptr ) ;
    ssd1306_show ( oled_ptr ) ;
    
    return setup_success ;
}

void update_Car_Status ( ssd1306_t *oled_ptr , uint8_t car_status , uint8_t charging_status ) {

    ssd1306_clear ( oled_ptr ) ;
    ssd1306_draw_string ( oled_ptr , 0 ,  0 , FONT_SCALE , "CAR:" ) ;
    ssd1306_draw_string ( oled_ptr , 0 , 28 , FONT_SCALE , "BAT:" ) ;

    ssd1306_draw_string ( oled_ptr , 50 , 0 , FONT_SCALE , ( car_status ) ? "ON" : "OFF" ) ;
    ssd1306_draw_string ( oled_ptr , 45 , 28 , FONT_SCALE , ( charging_status ) ? "CHRG" : "DISCHRG" ) ;

    ssd1306_show ( oled_ptr ) ;
}

uint8_t UART_Setup ( uart_inst_t *chosen_uart_instance_id , uint chosen_baudrate , uint RX_pin , uint TX_pin ) {

    // Set up the UART with the selected baudrate
    uint actual_baudrate = uart_init ( chosen_uart_instance_id , chosen_baudrate ) ;

    // Set the TX and RX pins by using the function select on the GPIO
    gpio_set_function ( TX_pin , UART_FUNCSEL_NUM ( chosen_uart_instance_id , TX_pin ) ) ;
    gpio_set_function ( RX_pin , UART_FUNCSEL_NUM ( chosen_uart_instance_id, RX_pin ) ) ;

    // Turn off UART flow control CTS/RTS
    uart_set_hw_flow ( chosen_uart_instance_id , false , false ) ;

    // Set the desired data format
    uart_set_format ( chosen_uart_instance_id , DATA_BITS , STOP_BITS , PARITY ) ;

    // Turn on FIFO on specified UART port
    uart_set_fifo_enabled ( chosen_uart_instance_id , true ) ;

    return ( actual_baudrate ) ? 1 : 0 ;
}

void Setup_UART_RX_IRQ ( uart_inst_t *chosen_uart_instance_id , irq_handler_t rx_isr ) {
    // Select correct interrupt for the UART port used
    uint UART_IRQ = ( chosen_uart_instance_id == uart0 ) ? UART0_IRQ : UART1_IRQ ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler ( UART_IRQ , rx_isr ) ;
    irq_set_enabled ( UART_IRQ , true ) ;

    // Enable the UART port to send interrupts - RX only
    uart_set_irq_enables ( chosen_uart_instance_id , true , false ) ;
}