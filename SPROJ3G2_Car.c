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

#define FREEZE 100
#define SPI_BAUDRATE 4000000
#define RF_CHANNEL 110
#define THRESHOLD 2000
//#define ADDRESS_WIDTH 5

//const uint8_t address [ ADDRESS_WIDTH ] = { 0x37 , 0x37 , 0x37 , 0x37 , 0x37 } ;

int main ( ) {

    uint8_t pipe_number = 0 , led_state = 0 ;
    uint16_t payload ;
    // initialize all present standard stdio types
    stdio_init_all ( ) ;

    gpio_init ( PICO_DEFAULT_LED_PIN ) ;
    gpio_set_dir ( PICO_DEFAULT_LED_PIN , GPIO_OUT ) ;

    // GPIO pin numbers
    pin_manager_t RF_Pins = { 
        .sck = 2,   // Serial Clock
        .copi = 3,  // Master Out - Slave In
        .cipo = 4,  // Master In - Slave Out
        .csn = 5,   // Chip Select Not
        .ce = 6     // Chip Enable
    } ;

    nrf_manager_t RF_Config = {
        // RF Channel 
        .channel = RF_CHANNEL,

        // AW_3_BYTES, AW_4_BYTES, AW_5_BYTES
        .address_width = AW_5_BYTES,

        // dynamic payloads: DYNPD_ENABLE, DYNPD_DISABLE
        .dyn_payloads = DYNPD_DISABLE,

        // data rate: RF_DR_250KBPS, RF_DR_1MBPS, RF_DR_2MBPS
        .data_rate = RF_DR_250KBPS,

        // RF_PWR_NEG_18DBM, RF_PWR_NEG_12DBM, RF_PWR_NEG_6DBM, RF_PWR_0DBM
        .power = RF_PWR_NEG_18DBM,

        // retransmission count: ARC_NONE...ARC_15RT
        .retr_count = ARC_NONE,

        // retransmission delay: ARD_250US, ARD_500US, ARD_750US, ARD_1000US
        .retr_delay = ARD_250US 
    } ;

    nrf_client_t RF24 ;

    nrf_driver_create_client ( &RF24 ) ;            // Initialise the RF module

    RF24.configure ( &RF_Pins , SPI_BAUDRATE ) ;    // Configure GPIO pins and SPI
 
    RF24.initialise ( &RF_Config ) ;

    RF24.rx_destination ( DATA_PIPE_0 , ( const uint8_t [ ] ) { 0x37 , 0x37 , 0x37 , 0x37 , 0x37 } ) ;

    RF24.payload_size ( ALL_DATA_PIPES , sizeof ( payload ) ) ;

    RF24.dyn_payloads_disable ( ) ; // Dynamic payloads disabled

    RF24.receiver_mode ( ) ;        // RF module set to RX Mode

    while ( 1 ) {
        if ( RF24.is_packet ( &pipe_number ) ) {
            RF24.read_packet ( &payload , sizeof ( payload ) ) ;

            led_state = ( payload > THRESHOLD ) ? 1 : 0 ;
            gpio_put ( PICO_DEFAULT_LED_PIN , led_state ) ;
        }
    }
    return 0 ;
}