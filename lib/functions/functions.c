#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "nrf24_driver.h"
#include "ssd1306.h"
#include "functions.h"

const uint8_t *PAYLOAD_ADDRESS = ( const uint8_t [ ] ) { 0x37 , 0x37 , 0x37 , 0x37 , 0x37 } ;    // Selected address
uint8_t payload_pipe = 0 ;  // Selected payload pipe

//GPIO pin numbers for the SSD1306 OLED module
oled_pins_t OLED_Pins = {
    .OLED_SDA = 8 ,
    .OLED_SCL = 9
} ;

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
    .dyn_payloads = DYNPD_ENABLE ,

    // data rate: RF_DR_250KBPS, RF_DR_1MBPS, RF_DR_2MBPS
    .data_rate = RF_DR_2MBPS ,

    // RF_PWR_NEG_18DBM, RF_PWR_NEG_12DBM, RF_PWR_NEG_6DBM, RF_PWR_0DBM
    .power = RF_PWR_0DBM ,

    // retransmission count: ARC_NONE...ARC_15RT
    .retr_count = ARC_1RT ,

    // retransmission delay: ARD_250US, ARD_500US, ARD_750US, ARD_1000US
    .retr_delay = ARD_500US 
} ;

int32_t convertInterval ( int32_t x , int32_t in_min , int32_t in_max , int32_t out_min , int32_t out_max ) {
  return ( x - in_min ) * ( out_max - out_min ) / ( in_max - in_min ) + out_min ;
}

uint8_t ADC_Setup ( uint8_t *input_pins , uint8_t input_length ) {
    uint8_t ctr , success = 0 ;
    adc_init ( ) ;

    // Make sure GPIO is high-impedance, no pullups etc...
    for ( ctr = 0 ; ctr < input_length ; ctr++ ) {
        if ( input_pins [ ctr ] >= ADC_REF_PIN && input_pins [ ctr ] <= ADC_MAX_PIN ) {
            adc_gpio_init ( input_pins [ ctr ] ) ;
            success++ ;
        }
    }
    return ( success == input_length ) ? 1 : 0 ;
}

uint16_t read_ADC ( uint8_t pin ) {
    adc_select_input ( pin - ADC_REF_PIN ) ;
    return adc_read ( ) ;
}

void nRF24_Setup ( nrf_client_t *RF24_ptr , pin_manager_t *RF24_pins_ptr , nrf_manager_t *RF24_config_ptr , uint32_t baudrate_SPI ,
            Auto_Ack_t ack_mode , RF_Mode_t mode , size_t payload_size , data_pipe_t payload_pipe , const uint8_t *payload_address )
{             
    nrf_driver_create_client ( RF24_ptr ) ;
    RF24_ptr->configure ( RF24_pins_ptr , baudrate_SPI ) ;      // Set up the correct pinout and set the SPI baudrate
    RF24_ptr->initialise ( RF24_config_ptr ) ;                  // Initialize the nRF24 module with the stated configuration
    RF24_ptr->rf_data_rate ( RF24_config_ptr->data_rate ) ;                   // Set the specified air transfer rate
    RF24_ptr->rf_power ( RF24_config_ptr->power ) ;                           // Set the specified antenna power
    RF24_ptr->payload_size ( payload_pipe , payload_size ) ;     // Set the size of the payload message

    switch ( mode ) {
        case RF24_RX :
            RF24_ptr->rx_destination ( payload_pipe , payload_address ) ;
            RF24_ptr->receiver_mode ( ) ;
            break ;

        case RF24_TX :
            RF24_ptr->tx_destination ( payload_address ) ;
            RF24_ptr->standby_mode ( ) ;
            break ;

        default : break ;
    }
    
    // Enable Auto-ACK with the specified Retransmission delay and count
    if ( ack_mode == ACK_ON ) { RF24_ptr->auto_retransmission ( RF24_config_ptr->retr_delay , RF24_config_ptr->retr_count ) ; }

    switch ( RF24_config_ptr->dyn_payloads ) {
        case DYNPD_ENABLE :
            RF24_ptr->dyn_payloads_enable ( ) ;
            break ;

        case DYNPD_DISABLE :
            RF24_ptr->dyn_payloads_disable ( ) ;
            break ;
        default : break ;
    }
}

void set_Payload_Data ( payload_t *payload , uint8_t pot_x_pin ) {
    uint16_t Pot_X_Val ;
    const uint16_t THRESHOLD  = ADC_MAX / 2 ;
    const uint16_t LOW_LIMIT  = THRESHOLD - TOLERANCE ;
    const uint16_t HIGH_LIMIT = THRESHOLD + TOLERANCE ;

    Pot_X_Val = read_ADC ( POT_X_PIN ) ;

    if ( Pot_X_Val >= HIGH_LIMIT || Pot_X_Val <= LOW_LIMIT ) {
        payload->servo_angle = ( uint8_t ) convertInterval ( ( int32_t ) Pot_X_Val , ADC_MIN , ADC_MAX , MIN_ANGLE , MAX_ANGLE ) ;
        payload->direction = ( Pot_X_Val > THRESHOLD ) ? 1 : 0 ;
    }

    else {
        payload->servo_angle = INIT_ANGLE ;
        payload->direction = 1 ;
    }
}

void OLED_Setup ( oled_pins_t *oled_pins , ssd1306_t *oled_ptr ) {
    i2c_inst_t *current_i2c_instance ;

    if ( oled_pins->OLED_SDA % 4 == 0 ) current_i2c_instance = i2c0 ;
    else current_i2c_instance = i2c1 ;

    i2c_init ( current_i2c_instance , I2C_BAUDRATE ) ;

    gpio_set_function ( oled_pins->OLED_SDA , GPIO_FUNC_I2C ) ;
    gpio_set_function ( oled_pins->OLED_SCL , GPIO_FUNC_I2C ) ;
    gpio_pull_up ( oled_pins->OLED_SDA ) ;
    gpio_pull_up ( oled_pins->OLED_SCL ) ;

    oled_ptr->external_vcc = false ;
    ssd1306_init ( oled_ptr , OLED_WIDTH , OLED_HEIGHT , OLED_ADDRESS , current_i2c_instance ) ;
    ssd1306_clear ( oled_ptr ) ;
    ssd1306_show ( oled_ptr ) ;
}

void draw_Initial_Texts ( ssd1306_t *oled_ptr ) {
    ssd1306_draw_string ( oled_ptr , 0 , 0 , FONT_SCALE , "Car:" ) ;
    ssd1306_draw_string ( oled_ptr , 0 , 23 , FONT_SCALE , "Bat:" ) ;
    //ssd1306_draw_string ( oled_ptr , 0 , 43 , FONT_SCALE , "Status:" ) ;
    ssd1306_show ( oled_ptr ) ;
}

void draw_Car_Status ( ssd1306_t *oled_ptr , uint8_t status ) {
    ssd1306_draw_string ( oled_ptr , 60 , 0 , FONT_SCALE , ( status ) ? "ON  " : "OFF " ) ;
    ssd1306_show ( oled_ptr ) ;
}