/**
 * Title:           Semester Project 3 Group 2 - Functions header file
 * Institution:     University of Southern Denmark (SDU)
 * Campus:          Sønderborg
 * File:            functions.h
 * Authors:         Bence Tóth
 * Date:            23/01/2025
 * Course:          BEng in Electronics
 * Semester:        3rd
 * Platform:        Raspberry Pi RP2040 C SDK
 * RF module:       nRF24L01 PA/LNA
 * OLED module:     SSD1306 128x32 I2C
 * RF library:      https://github.com/andyrids/pico-nrf24
 * OLED library:    https://github.com/daschr/pico-ssd1306
 */

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "nrf24_driver.h"
#include "ssd1306.h"

// UART configs
#define UART_TX_PIN     16
#define UART_RX_PIN     17
#define UART_BAUDRATE 9600
#define DATA_BITS        8
#define STOP_BITS        1
#define UART_ID          uart0
#define PARITY           UART_PARITY_NONE

// Servo configs
#define SERVO_PIN   8
#define INIT_ANGLE 90     // in degrees
#define MAX_ANGLE 110     // in degrees
#define MIN_ANGLE  70     // in degrees

// Thumbstick configs
#define ADC_REF_PIN   26
#define ADC_MAX_PIN   29
#define POT_X_PIN     26
#define POT_Y_PIN     27
#define ADC_MIN        0
#define ADC_MAX     4095     // 12 bit ADC
#define TOLERANCE    300     // This value can be lowered, needs calibration

// Motor configs
#define SPEED_MAX_FORWARD   127
#define SPEED_MAX_BACKWARD -128
#define HALT                  0

// nRF24L01 configs
#define SPI_BAUDRATE              4000000  // 4 MHz SPI baudrate
#define RF_CHANNEL_LO                  50  // 2.45 GHz carrier frequency for the low-range RF modules
#define RF_CHANNEL_HI                 100  // 2.50 GHz carrier frequency for the high-range RF modules
#define INTERVAL_LIMIT                250  // Defines the maximum time interval between consecutive received data packets for the Car
#define MAX_RT_TRY                     50  // Defines the maximum Retransmission count of the Controller
#define RF_SETUP_OK    0b0000001111111111  // Checks if all RF setup functions returned the correct values

// SSD1306 OLED configs
#define OLED_WIDTH      128     // in pixels
#define OLED_HEIGHT      32     // in pixels
#define OLED_ADDRESS   0x3C     // Default I2C address for an SSD1306
#define I2C_BAUDRATE 400000     // 400 kHz I2C baudrate
#define FONT_SCALE        1     // The value of 1 is miniscule, 3 is too large...

// The payload structure for transmission
typedef struct {
    uint8_t servo_angle ;
    int8_t speed_and_direction ;
} payload_t ;

// OLED I2C Pins
typedef struct {
    uint8_t SDA ;
    uint8_t SCL ;
} oled_pins_t ;

extern oled_pins_t OLED_Pins ;

extern pin_manager_t RF_Pins ;

extern nrf_manager_t RF_Config ;

extern const uint8_t *PAYLOAD_ADDRESS ;

extern uint8_t payload_pipe ;

// Determines whether the module is primary transmitter or primary receiver
typedef enum { PTX = 0 , PRX = 1 } RF_Mode_t ;

// Determines whether to enable or disable Auto-ACK
typedef enum { ACK_OFF = 0 , ACK_ON = 1 } Auto_Ack_t ;

// Function declarations

int32_t convertInterval ( int32_t x , int32_t in_min , int32_t in_max , int32_t out_min , int32_t out_max ) ;

uint8_t ADC_Setup ( uint8_t *input_pins , uint8_t input_length ) ;

uint16_t read_ADC ( uint8_t pin ) ;

uint16_t nRF24_Setup ( nrf_client_t *RF24_ptr , pin_manager_t *RF24_pins_ptr , nrf_manager_t *RF24_config_ptr ,
    Auto_Ack_t ack_mode , RF_Mode_t mode , size_t payload_size , data_pipe_t payload_pipe , const uint8_t *payload_address ) ;

void set_Payload_Data ( payload_t *payload , uint8_t pot_x_pin , uint8_t pot_y_pin ) ;

uint8_t OLED_Setup ( oled_pins_t *oled_pins , ssd1306_t *oled_ptr ) ;

void draw_Initial_Texts ( ssd1306_t *oled_ptr ) ;

void update_Car_Status ( ssd1306_t *oled_ptr , uint8_t car_status , uint8_t charging_status ) ;

uint8_t UART_Setup ( uart_inst_t *chosen_uart_instance_id , uint chosen_baudrate , uint RX_pin , uint TX_pin ) ;

void Setup_UART_RX_IRQ ( uart_inst_t *chosen_uart_instance_id , irq_handler_t rx_isr ) ;

#ifdef __cplusplus
}
#endif