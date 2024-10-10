#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "nrf24_driver.h"

#define SERVO_PIN  8
#define ADC_REF_PIN 26
#define ADC_MAX_PIN 29
#define POT_X_PIN 26
#define POT_Y_PIN 27
#define INIT_ANGLE 90
#define MAX_ANGLE 180
#define MIN_ANGLE 0
#define ADC_MIN 0
#define ADC_MAX 4096
#define TOLERANCE 50
#define THRESHOLD ( ADC_MAX / 2 )
#define LOW_LIMIT ( THRESHOLD - TOLERANCE )
#define HIGH_LIMIT ( THRESHOLD + TOLERANCE )
#define SPI_BAUDRATE 4000000    // 4 MHz SPI baudrate
#define RF_CHANNEL 110          // 2.51 GHz ISM frequency band
#define RF_ADDRESS ( ( const uint8_t [ ] ) { 0x37 , 0x37 , 0x37 , 0x37 , 0x37 } )
#define FREEZE 20

extern pin_manager_t RF_Pins ;
extern nrf_manager_t RF_Config ;

typedef struct {                // The payload structure for transmission
    uint8_t direction ;
    uint8_t servo_angle ;
} payload_t ;

typedef enum { RF24_TX = 0 , RF24_RX = 1 } RF_Mode ;

int32_t convertInterval ( int32_t x , int32_t in_min , int32_t in_max , int32_t out_min , int32_t out_max ) ;

uint8_t ADC_Setup ( uint8_t *input_pins , uint8_t input_length ) ;

uint16_t read_ADC ( uint8_t pin ) ;

void nRF24_Setup ( nrf_client_t *RF24_ptr , pin_manager_t *RF24_pins_ptr ,
                    nrf_manager_t *RF24_config_ptr , uint32_t baudrate_SPI , size_t payload_size ,
                    RF_Mode mode , dyn_payloads_t dyn_mode , const uint8_t *address_buffer , data_pipe_t datapipe ) ;

#ifdef __cplusplus
}
#endif