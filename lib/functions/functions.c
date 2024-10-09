#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "nrf24_driver.h"
#include "functions.h"

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

void nRF24_Setup ( nrf_client_t *RF24_ptr , pin_manager_t *RF24_pins_ptr ,
                    nrf_manager_t *RF24_config_ptr , uint32_t baudrate_SPI , size_t payload_size ,
                    RF_Mode mode , dyn_payloads_t dyn_mode , const uint8_t *address_buffer , data_pipe_t datapipe ) {
    nrf_driver_create_client ( RF24_ptr ) ;
    RF24_ptr->configure ( RF24_pins_ptr , baudrate_SPI ) ;      // Set up the correct pinout and set the SPI baudrate
    RF24_ptr->initialise ( RF24_config_ptr ) ;                  // Initialize the nRF24 module with the stated configuration
    RF24_ptr->payload_size ( DATA_PIPE_0 , payload_size ) ;     // Set the size of the payload
    
    switch ( dyn_mode ) {
        case DYNPD_ENABLE  : RF24_ptr->dyn_payloads_enable ( )  ; break ;
        case DYNPD_DISABLE : RF24_ptr->dyn_payloads_disable ( ) ; break ;
        default            : RF24_ptr->dyn_payloads_disable ( ) ; break ;
    }

    switch ( mode ) {
        case RF24_RX :  RF24_ptr->rx_destination ( datapipe , address_buffer ) ; RF24_ptr->receiver_mode ( ) ; break ;
        case RF24_TX :  RF24_ptr->tx_destination ( address_buffer )            ; RF24_ptr->standby_mode  ( ) ; break ;
        default      :  RF24_ptr->tx_destination ( address_buffer )            ; RF24_ptr->standby_mode  ( ) ; break ;
    }
}