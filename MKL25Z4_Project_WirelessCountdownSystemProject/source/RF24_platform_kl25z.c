
//Shared native-C hardware abstraction layer for the RF24 driver on the
 //FRDM-KL25Z using MCUXpresso SDK.

// This same file is used by BOTH:
//   - the ground-station transmitter project
//   - the onboard receiver project

 //Radio wiring:
 //  PTA12             -> nRF24L01+ CE
 //  PTA14             -> nRF24L01+ CSN (GPIO-controlled)
 //  PTA15 / SPI0_SCK  -> nRF24L01+ SCK
 //  PTA16 / SPI0_SIN  <- nRF24L01+ MISO
 //  PTA17 / SPI0_SOUT -> nRF24L01+ MOSI
 //   IRQ               -> not connected initially

 //This version also exposes optional ground-station ADC16 helper functions
 //for a 10 kohm potentiometer connected to PTB3 / ADC0_SE13. The receiver's
 //onboard LED PWM control still belongs in receiver_main.c because this file
 //is shared by both boards.


#include "RF24_config.h"

#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_spi.h"
#include "fsl_adc16.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Local state


static volatile uint32_t s_rf24_milliseconds = 0U;
static bool s_rf24_platform_initialized = false;


// Ground-station potentiometer input:
//   PTB3 -> ADC0_SE13
 //Potentiometer wiring:
 //  one outer terminal   -> 3.3 V
 //  center/wiper         -> PTB3
 //   other outer terminal -> GND

#define RF24_POT_ADC_BASE            ADC0
#define RF24_POT_ADC_CHANNEL_GROUP   0U
#define RF24_POT_ADC_CHANNEL         13U
#define RF24_POT_PORT                PORTB
#define RF24_POT_PIN                 3U

static bool s_rf24_adc_initialized = false;




//SysTick time base



 //This handler provides the 1 ms time base used by RF24_Millis().


void SysTick_Handler(void)
{
    s_rf24_milliseconds++;
}


//Internal initialization helpers


static void RF24_InitPinMux(void)
{

     //Enable PORTA clock before configuring PTA12 and PTA14-PTA17.

    CLOCK_EnableClock(kCLOCK_PortA);
    CLOCK_EnableClock(kCLOCK_PortD);


     //CE and CSN are ordinary GPIO outputs.
     //PTA14 could be routed as SPI0_PCS0, but the RF24 driver intentionally
     //controls CSN manually so it remains LOW for a complete radio command.

    PORT_SetPinMux(RF24_CE_PORT, RF24_CE_PIN, RF24_CE_MUX);
    PORT_SetPinMux(RF24_CSN_PORT, RF24_CSN_PIN, RF24_CSN_MUX);

    /*
     * SPI0 routing:
     *   PTA15 = SPI0_SCK
     *   PTA16 = SPI0_SIN  (radio MISO)
     *   PTA17 = SPI0_SOUT (radio MOSI)
     */
    PORT_SetPinMux(RF24_SCK_PORT, RF24_SCK_PIN, RF24_SCK_MUX);
    PORT_SetPinMux(RF24_MISO_PORT, RF24_MISO_PIN, RF24_MISO_MUX);
    PORT_SetPinMux(RF24_MOSI_PORT, RF24_MOSI_PIN, RF24_MOSI_MUX);
}

static void RF24_InitControlGPIO(void)
{
    const gpio_pin_config_t ce_config = {
        kGPIO_DigitalOutput,
        0U
    };

    const gpio_pin_config_t csn_config = {
        kGPIO_DigitalOutput,
        1U
    };

    /*
     * Safe startup states:
     *   CE  LOW  -> radio does not actively transmit or receive
     *   CSN HIGH -> no SPI transaction selected
     */
    GPIO_PinInit(RF24_CE_GPIO, RF24_CE_PIN, &ce_config);
    GPIO_PinInit(RF24_CSN_GPIO, RF24_CSN_PIN, &csn_config);
}

static bool RF24_InitSPI0(uint32_t baud_rate_hz)
{
    spi_master_config_t master_config;
    uint32_t source_clock_hz;

    CLOCK_EnableClock(kCLOCK_Spi0);

    SPI_MasterGetDefaultConfig(&master_config);

    master_config.enableMaster = true;
    master_config.enableStopInWaitMode = false;

    /*
     * nRF24L01+ SPI mode 0:
     *   CPOL = 0 -> clock idles LOW
     *   CPHA = 0 -> sample on first clock edge
     */
    master_config.polarity = RF24_SPI_CLOCK_POLARITY;
    master_config.phase = RF24_SPI_CLOCK_PHASE;
    master_config.direction = RF24_SPI_SHIFT_DIRECTION;
    master_config.baudRate_Bps = baud_rate_hz;

    source_clock_hz = CLOCK_GetFreq(RF24_SPI_CLOCK_SOURCE);

    if (source_clock_hz == 0U) {
        return false;
    }

    SPI_MasterInit(RF24_SPI_BASE, &master_config, source_clock_hz);
    return true;
}

static bool RF24_InitTimeBase(void)
{
    SystemCoreClockUpdate();

    if ((SystemCoreClock == 0U) ||
        (SysTick_Config(SystemCoreClock / 1000U) != 0U)) {
        return false;
    }

    s_rf24_milliseconds = 0U;
    return true;
}

/* -------------------------------------------------------------------------- */
/* Optional ground-station ADC16 support                                      */
/* -------------------------------------------------------------------------- */

bool RF24_GroundStationADC_Init(void)
{
    adc16_config_t adc_config;

    /*
     * PTB3 is ADC0_SE13. Analog operation requires the pin mux disabled.
     */
    CLOCK_EnableClock(kCLOCK_PortB);
    PORT_SetPinMux(RF24_POT_PORT, RF24_POT_PIN, kPORT_PinDisabledOrAnalog);

    CLOCK_EnableClock(kCLOCK_Adc0);

    ADC16_GetDefaultConfig(&adc_config);

    /*
     * Equivalent intent to the earlier direct-register test:
     * - 12-bit single-ended conversions
     * - ADC input clock divided by 2
     * - software-triggered conversions
     */
    adc_config.resolution = kADC16_ResolutionSE12Bit;
    adc_config.clockDivider = kADC16_ClockDivider2;
    adc_config.enableContinuousConversion = false;

    ADC16_Init(RF24_POT_ADC_BASE, &adc_config);
    ADC16_EnableHardwareTrigger(RF24_POT_ADC_BASE, false);

#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
    if (ADC16_DoAutoCalibration(RF24_POT_ADC_BASE) != kStatus_Success) {
        return false;
    }
#endif

    s_rf24_adc_initialized = true;
    return true;
}

uint16_t RF24_GroundStationADC_ReadPot(void)
{
    adc16_channel_config_t channel_config;

    if (!s_rf24_adc_initialized) {
        return 0U;
    }

    channel_config.channelNumber = RF24_POT_ADC_CHANNEL;
    channel_config.enableInterruptOnConversionCompleted = false;

#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
    channel_config.enableDifferentialConversion = false;
#endif

    ADC16_SetChannelConfig(
        RF24_POT_ADC_BASE,
        RF24_POT_ADC_CHANNEL_GROUP,
        &channel_config);

    while ((ADC16_GetChannelStatusFlags(
                RF24_POT_ADC_BASE,
                RF24_POT_ADC_CHANNEL_GROUP) &
            kADC16_ChannelConversionDoneFlag) == 0U) {
        /* Poll until conversion is complete. */
    }

    return ADC16_GetChannelConversionValue(
        RF24_POT_ADC_BASE,
        RF24_POT_ADC_CHANNEL_GROUP);

}


/* Public platform initialization                                             */


bool RF24_PlatformInit(uint32_t baud_rate_hz)
{
    if (s_rf24_platform_initialized) {
        return true;
    }

    if (baud_rate_hz == 0U) {
        return false;
    }

    RF24_InitPinMux();
    RF24_InitControlGPIO();

    if (!RF24_InitSPI0(baud_rate_hz)) {
        return false;
    }

    if (!RF24_InitTimeBase()) {
        SPI_Deinit(RF24_SPI_BASE);
        return false;
    }

    /*
     * Reassert safe radio states after all peripheral initialization.
     */
    RF24_CE_Low();
    RF24_CSN_High();

    s_rf24_platform_initialized = true;
    return true;
}


/* SPI transfer functions                                                     */


bool RF24_SPI_TransferByte(uint8_t tx_byte, uint8_t *rx_byte)
{
    spi_transfer_t transfer;
    uint8_t received = 0xFFU;
    status_t result;

    if (!s_rf24_platform_initialized) {
        return false;
    }

    transfer.txData = &tx_byte;
    transfer.rxData = &received;
    transfer.dataSize = 1U;

    /*
     * The KL25Z SPI driver's flags field is not used for this peripheral.
     * CSN is controlled manually through PTA14.
     */
    transfer.flags = 0U;

    result = SPI_MasterTransferBlocking(RF24_SPI_BASE, &transfer);

    if (result != kStatus_Success) {
        return false;
    }

    if (rx_byte != NULL) {
        *rx_byte = received;
    }

    return true;
}

bool RF24_SPI_Transfer(
    const uint8_t *tx_data,
    uint8_t *rx_data,
    size_t length)
{
    spi_transfer_t transfer;
    status_t result;

    if (!s_rf24_platform_initialized) {
        return false;
    }

    if ((length == 0U) ||
        ((tx_data == NULL) && (rx_data == NULL))) {
        return false;
    }

    transfer.txData = (uint8_t *)tx_data;
    transfer.rxData = rx_data;
    transfer.dataSize = length;
    transfer.flags = 0U;

    /*
     * The caller is responsible for:
     *   RF24_CSN_Low();
     *   RF24_SPI_Transfer(...);
     *   RF24_CSN_High();
     */
    result = SPI_MasterTransferBlocking(RF24_SPI_BASE, &transfer);

    return result == kStatus_Success;
}

/* -------------------------------------------------------------------------- */
/* CE and CSN control                                                         */
/* -------------------------------------------------------------------------- */

void RF24_CE_High(void)
{
    RF24_CE_Write(true);
}

void RF24_CE_Low(void)
{
    RF24_CE_Write(false);
}

void RF24_CSN_High(void)
{
    RF24_CSN_Write(true);
}

void RF24_CSN_Low(void)
{
    RF24_CSN_Write(false);
}

/* -------------------------------------------------------------------------- */
/* Timing                                                                     */
/* -------------------------------------------------------------------------- */

//void RF24_DelayUs(uint32_t microseconds)
//{
//    if (microseconds == 0U) {
//        return;
//    }
//
//    /*
//     * SDK_DelayAtLeastUs() uses the supplied core clock to guarantee at least
//     * the requested delay.
//     */
//    SDK_DelayAtLeastUs(microseconds, SystemCoreClock);
//}


void RF24_DelayUs(uint32_t microseconds)
{
    uint32_t cycles_per_us;
    volatile uint32_t cycles;

    if ((microseconds == 0U) || (SystemCoreClock == 0U)) {
        return;
    }

    /*
     * Approximate delay loop for Cortex-M0+.
     *
     * The loop intentionally runs slightly long rather than short because
     * the nRF24L01+ timing requirements specify minimum delays.
     */
    cycles_per_us = SystemCoreClock / 3000000U;

    if (cycles_per_us == 0U) {
        cycles_per_us = 1U;
    }

    cycles = cycles_per_us * microseconds;

    while (cycles > 0U) {
        __NOP();
        cycles--;
    }
}






void RF24_DelayMs(uint32_t milliseconds)
{
    while (milliseconds > 0U) {
        RF24_DelayUs(1000U);
        milliseconds--;
    }
}

uint32_t RF24_Millis(void)
{
    return s_rf24_milliseconds;
}
