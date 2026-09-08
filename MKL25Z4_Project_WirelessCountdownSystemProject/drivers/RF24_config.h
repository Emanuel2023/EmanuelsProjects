
 //Native C platform configuration for the nRF24L01+ PA/LNA module on the FRDM-KL25Z using MCUXpresso SDK and SPI0.
 //Wiring:
 //PTA12             -> CE
 //PTA14             -> CSN (GPIO-controlled)
 //PTA15 / SPI0_SCK  -> SCK
 //PTA16 / SPI0_SIN  <- MISO
 //PTA17 / SPI0_SOUT -> MOSI
 //IRQ               -> Not connected initially
 //Ground-station potentiometer:
 //PTB3 / ADC0_SE13  <- Potentiometer wiper
 //PTA14 supports SPI0_PCS0, but this driver intentionally uses it as a normal GPIO so CSN can remain LOW for an entire nRF24L01+ command.


#ifndef RF24_CONFIG_H_
#define RF24_CONFIG_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_spi.h"

#ifdef __cplusplus
extern "C" {
#endif


//Driver feature configuration


#define RF24_FAILURE_HANDLING 1
#define RF24_DEBUG            0
#define RF24_MINIMAL          1

//Conservative power-down-to-standby delay required by the radio.
#ifndef RF24_POWERUP_DELAY_US
#define RF24_POWERUP_DELAY_US 5000U
#endif

//Start slowly for bring-up. Increase only after reliable communication.
#ifndef RF24_SPI_BAUDRATE_HZ
#define RF24_SPI_BAUDRATE_HZ 1000000U
#endif

#define RF24_MAX_PAYLOAD_SIZE      32U
#define RF24_DEFAULT_PAYLOAD_SIZE  32U
#define RF24_DEFAULT_ADDRESS_WIDTH 5U
#define RF24_INVALID_PIN           UINT8_MAX

#define RF24_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define RF24_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define RF24_BIT(bit)  ((uint8_t)(1U << (bit)))


//KL25Z SPI0 configuration


#define RF24_SPI_BASE         SPI0
#define RF24_SPI_CLOCK_SOURCE kCLOCK_BusClk

#define RF24_SPI_CLOCK_POLARITY  kSPI_ClockPolarityActiveHigh
#define RF24_SPI_CLOCK_PHASE     kSPI_ClockPhaseFirstEdge
#define RF24_SPI_SHIFT_DIRECTION kSPI_MsbFirst


//Pin assignments


//CE: PTA12 as GPIO output.
#define RF24_CE_PORT PORTA
#define RF24_CE_GPIO GPIOA
#define RF24_CE_PIN  12U
#define RF24_CE_MUX  kPORT_MuxAsGpio

//CSN: PTA14 as GPIO output, not hardware PCS0.
#define RF24_CSN_PORT PORTD
#define RF24_CSN_GPIO GPIOD
#define RF24_CSN_PIN  0U
#define RF24_CSN_MUX  kPORT_MuxAsGpio

//SPI0 SCK: PTA15, ALT2.
#define RF24_SCK_PORT PORTD
#define RF24_SCK_PIN  1U
#define RF24_SCK_MUX  kPORT_MuxAlt2

//SPI0 SIN: PTA16, ALT2. Connect to radio MISO.
#define RF24_MISO_PORT PORTD
#define RF24_MISO_PIN  3U
#define RF24_MISO_MUX  kPORT_MuxAlt2

//SPI0 SOUT: PTA17, ALT2. Connect to radio MOSI.
#define RF24_MOSI_PORT PORTD
#define RF24_MOSI_PIN  2U
#define RF24_MOSI_MUX  kPORT_MuxAlt2


//Native C platform API
//We implement these functions in RF24_platform_kl25z.c
//Initialize PORTA pin muxing, CE, CSN, and SPI0.
 //Initial output states:
 //CE  = LOW
 //CSN = HIGH

bool RF24_PlatformInit(uint32_t baud_rate_hz);


 //Perform one full-duplex SPI0 byte transfer.
 //@param tx_byte Byte transmitted on PTA17 / SPI0_SOUT.
 //@param rx_byte Storage for byte received on PTA16 / SPI0_SIN.
 //@return true on successful transfer; false on SDK transfer failure.

bool RF24_SPI_TransferByte(uint8_t tx_byte, uint8_t *rx_byte);


 //Perform a full-duplex SPI0 buffer transfer while CSN is controlled separately by RF24_CSN_Low() and RF24_CSN_High().

bool RF24_SPI_Transfer(const uint8_t *tx_data,
                       uint8_t *rx_data,
                       size_t length);

//GPIO control for the radio.
void RF24_CE_High(void);
void RF24_CE_Low(void);
void RF24_CSN_High(void);
void RF24_CSN_Low(void);

//Timing services required by the C RF24 driver.
void RF24_DelayUs(uint32_t microseconds);
void RF24_DelayMs(uint32_t milliseconds);
uint32_t RF24_Millis(void);


//Ground-station ADC16 support
//PTB3 is used as ADC0_SE13 for the 10 kOhm potentiometer.


//Initialize ADC0 for 12-bit software-triggered conversions on PTB3.
 //Potentiometer wiring:
 //Outer terminal 1 -> 3.3 V
 //Center wiper     -> PTB3
 //Outer terminal 2 -> GND
 //@return true if ADC initialization succeeds.

bool RF24_GroundStationADC_Init(void);

//Perform one ADC conversion of the PTB3 potentiometer input.
 //@return 12-bit result in the range 0 through 4095.

uint16_t RF24_GroundStationADC_ReadPot(void);


//Optional direct GPIO helpers


//static inline void RF24_CE_Write(bool high)
//{
//    GPIO_PinWrite(RF24_CE_GPIO, RF24_CE_PIN, high ? 1U : 0U);
//}
//
//static inline void RF24_CSN_Write(bool high)
//{
//    GPIO_PinWrite(RF24_CSN_GPIO, RF24_CSN_PIN, high ? 1U : 0U);
//}

static inline void RF24_CE_Write(bool high)
{
    if (high) {
        RF24_CE_GPIO->PSOR = (1UL << RF24_CE_PIN);
    } else {
        RF24_CE_GPIO->PCOR = (1UL << RF24_CE_PIN);
    }
}

static inline void RF24_CSN_Write(bool high)
{
    if (high) {
        RF24_CSN_GPIO->PSOR = (1UL << RF24_CSN_PIN);
    } else {
        RF24_CSN_GPIO->PCOR = (1UL << RF24_CSN_PIN);
    }
}

#ifdef __cplusplus
}
#endif

#endif
