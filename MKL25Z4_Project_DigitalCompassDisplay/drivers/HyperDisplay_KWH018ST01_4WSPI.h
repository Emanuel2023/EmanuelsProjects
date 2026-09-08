/*
 * HyperDisplay_KWH018ST01_4WSPI.h
 *
 * Pure-C port for FRDM-KL25Z / MKL25Z4
 *
 * KWH018ST01 128x160 TFT display
 * ILI9163C controller
 * 4-wire SPI interface
 */

#ifndef HYPERDISPLAY_KWH018ST01_4WSPI_H_
#define HYPERDISPLAY_KWH018ST01_4WSPI_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "HyperDisplay_ILI9163C.h"


/* ==========================================================================
 * Display dimensions
 * ========================================================================== */

#define KWH018ST01_WIDTH        128U
#define KWH018ST01_HEIGHT       160U

#define KWH018ST01_START_ROW    0U
#define KWH018ST01_START_COL    0U

#define KWH018ST01_STOP_ROW     159U
#define KWH018ST01_STOP_COL     127U


/* ==========================================================================
 * Backlight callback
 *
 * The original Arduino library stores a pin number and can control the
 * display backlight through Arduino functions.
 *
 * For the KL25Z port we keep the display library independent of a specific
 * GPIO / TPM PWM implementation by using a callback.
 *
 * level:
 *      0   = off
 *      255 = maximum brightness
 *
 * A KL25Z-specific implementation can later drive either:
 *      - a GPIO output
 *      - a TPM PWM channel
 * ========================================================================== */

typedef void (*KWH018ST01_backlight_write_fn)(
    void *context,
    uint8_t level);


/* ==========================================================================
 * KWH018ST01 object
 *
 * Replaces:
 *
 * class KWH018ST01_4WSPI : public ILI9163C_4WSPI
 *
 * In C, the ILI9163C 4-wire SPI object is embedded as the first-level
 * controller/transport object.
 * ========================================================================== */

typedef struct
{
    ILI9163C_4WSPI_t ili9163c4wspi;

    KWH018ST01_backlight_write_fn backlightWrite;
    void *backlightContext;

    uint8_t backlightLevel;

} KWH018ST01_4WSPI_t;


/* ==========================================================================
 * Initialization
 *
 * Replaces the C++ constructor:
 *
 *      KWH018ST01_4WSPI();
 * ========================================================================== */

void KWH018ST01_4WSPI_init(
    KWH018ST01_4WSPI_t *display);


/* ==========================================================================
 * Begin
 *
 * Replaces:
 *
 * begin(
 *     uint8_t dcPin,
 *     uint8_t csPin,
 *     uint8_t blPin,
 *     SPIClass &spiInterface = SPI,
 *     uint32_t spiFreq = ILI9163C_SPI_DEFAULT_FREQ
 * );
 *
 * The KL25Z version explicitly supplies the SPI peripheral and GPIO ports.
 *
 * NOTE:
 * SPI pin multiplexing itself is configured outside this driver.
 * ========================================================================== */

ILI9163C_STAT_t KWH018ST01_4WSPI_begin(
    KWH018ST01_4WSPI_t *display,

    SPI_Type *spiBase,

    GPIO_Type *dcGpio,
    uint32_t dcPin,

    GPIO_Type *csGpio,
    uint32_t csPin,

    KWH018ST01_backlight_write_fn backlightWrite,
    void *backlightContext,

    uint32_t spiFreqHz);


/* ==========================================================================
 * Display configuration
 * ========================================================================== */

/*
 * Apply the manufacturer / SparkFun recommended controller configuration.
 */
ILI9163C_STAT_t KWH018ST01_4WSPI_defaultConfigure(
    KWH018ST01_4WSPI_t *display);


/*
 * Perform the normal startup sequence for this display.
 */
void KWH018ST01_4WSPI_startup(
    KWH018ST01_4WSPI_t *display);


/* ==========================================================================
 * Font support
 *
 * This signature matches the HyperDisplay getCharInfo callback type from
 * our pure-C HyperDisplay port.
 * ========================================================================== */

void KWH018ST01_4WSPI_getCharInfo(
    hyperdisplay_t *display,
    uint8_t val,
    char_info_t *pchar);


/* ==========================================================================
 * Specialized drawing functions
 * ========================================================================== */

/*
 * Clear the entire TFT.
 */
void KWH018ST01_4WSPI_clearDisplay(
    KWH018ST01_4WSPI_t *display);


/* ==========================================================================
 * Window configuration
 * ========================================================================== */

/*
 * Configure the default drawing window for the 128x160 KWH018ST01 panel.
 */
void KWH018ST01_4WSPI_setWindowDefaults(
    KWH018ST01_4WSPI_t *display,
    wind_info_t *pwindow);


/* ==========================================================================
 * Backlight
 *
 * level:
 *      0   = off
 *      255 = maximum brightness
 * ========================================================================== */

void KWH018ST01_4WSPI_setBacklight(
    KWH018ST01_4WSPI_t *display,
    uint8_t level);


#endif /* HYPERDISPLAY_KWH018ST01_4WSPI_H_ */
