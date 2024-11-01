#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "nrf24_driver.h"
#include "ssd1306.h"

#define SERVO_PIN  8
#define ADC_REF_PIN 26
#define ADC_MAX_PIN 29
#define POT_X_PIN 26
#define POT_Y_PIN 27

#define INIT_ANGLE 90
#define MAX_ANGLE 180
#define MIN_ANGLE 0
#define ADC_MIN 0
#define ADC_MAX 4095
#define TOLERANCE 40

#define SPI_BAUDRATE 4000000            // 4 MHz SPI baudrate
#define RF_CHANNEL 50                   // ISM frequency band
#define INTERVAL_LIMIT 50               // Defines the maximum time interval between consecutive received data packets for the Car
#define MAX_RT_TRY 15                   // Defines the maximum Retransmission count of the Controller
#define RF_SETUP_OK 0b0000001111111111  // Checks if all RF setup functions returned the correct values

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDRESS 0x3C
#define I2C_BAUDRATE 400000     // 400 kHz I2C baudrate
#define FONT_SCALE 2

typedef struct {                // The payload structure for transmission
    uint8_t direction ;
    uint8_t servo_angle ;
} payload_t ;

typedef struct {
    uint8_t SDA ;
    uint8_t SCL ;
} oled_pins_t ;

extern oled_pins_t OLED_Pins ;

extern pin_manager_t RF_Pins ;
extern nrf_manager_t RF_Config ;

extern const uint8_t *PAYLOAD_ADDRESS ;
extern uint8_t payload_pipe ;

typedef enum { PTX = 0 , PRX = 1 } RF_Mode_t ;              // Determines whether the module is primary transmitter or primary receiver
typedef enum { ACK_OFF = 0 , ACK_ON = 1 } Auto_Ack_t ;      // Determines whether to enable or disable Auto-ACK

int32_t convertInterval ( int32_t x , int32_t in_min , int32_t in_max , int32_t out_min , int32_t out_max ) ;

uint8_t ADC_Setup ( uint8_t *input_pins , uint8_t input_length ) ;

uint16_t read_ADC ( uint8_t pin ) ;

uint16_t nRF24_Setup ( nrf_client_t *RF24_ptr , pin_manager_t *RF24_pins_ptr , nrf_manager_t *RF24_config_ptr ,
    Auto_Ack_t ack_mode , RF_Mode_t mode , size_t payload_size , data_pipe_t payload_pipe , const uint8_t *payload_address ) ;

void set_Payload_Data ( payload_t *payload , uint8_t pot_x_pin ) ;

uint8_t OLED_Setup ( oled_pins_t *oled_pins , ssd1306_t *oled_ptr ) ;

void draw_Initial_Texts ( ssd1306_t *oled_ptr ) ;

void update_Car_Status ( ssd1306_t *oled_ptr , uint8_t status ) ;

#ifdef __cplusplus
}
#endif