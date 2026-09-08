/*
 * HyperDisplay_KWH018ST01_4WSPI.c
 *
 * Pure-C port for FRDM-KL25Z / MKL25Z4 using the NXP MCUXpresso SDK.
 *
 * KWH018ST01 128x160 TFT
 * ILI9163C controller
 * 4-wire SPI interface
 */

#include "HyperDisplay_KWH018ST01_4WSPI.h"

#include <string.h>

#include "fsl_common.h"

/* -------------------------------------------------------------------------- */
/* Original library defaults                                                  */
/* -------------------------------------------------------------------------- */

char_info_t KWH018ST01_Default_CharInfo;
wind_info_t KWH018ST01_Default_Window;

/* -------------------------------------------------------------------------- */
/* Local helpers                                                              */
/* -------------------------------------------------------------------------- */

//static void KWH018ST01_delayMs(uint32_t milliseconds)
//{
//    if (milliseconds == 0U)
//    {
//        return;
//    }
//
//    SDK_DelayAtLeastUs(milliseconds * 1000U, SystemCoreClock);
//}

static void KWH018ST01_delayMs(uint32_t milliseconds)
{
    uint32_t reload;

    if (milliseconds == 0U)
    {
        return;
    }

    reload = (SystemCoreClock / 1000U) - 1U;

    SysTick->LOAD = reload;
    SysTick->VAL  = 0U;

    SysTick->CTRL =
        SysTick_CTRL_CLKSOURCE_Msk |
        SysTick_CTRL_ENABLE_Msk;

    while (milliseconds > 0U)
    {
        while ((SysTick->CTRL &
                SysTick_CTRL_COUNTFLAG_Msk) == 0U)
        {
        }

        milliseconds--;
    }

    SysTick->CTRL = 0U;
}

static ILI9163C_t *KWH018ST01_controller(KWH018ST01_4WSPI_t *display)
{
    if (display == NULL)
    {
        return NULL;
    }

    return &display->ili9163c4wspi.ili9163c;
}

static hyperdisplay_t *KWH018ST01_hyperdisplay(KWH018ST01_4WSPI_t *display)
{
    ILI9163C_t *ili = KWH018ST01_controller(display);

    if (ili == NULL)
    {
        return NULL;
    }

    return &ili->hyperdisplay;
}

/* -------------------------------------------------------------------------- */
/* Initialization                                                             */
/* -------------------------------------------------------------------------- */

void KWH018ST01_4WSPI_init(KWH018ST01_4WSPI_t *display)
{
    if (display == NULL)
    {
        return;
    }

    memset(display, 0, sizeof(*display));

    display->backlightWrite = NULL;
    display->backlightContext = NULL;
    display->backlightLevel = 0U;
}

ILI9163C_STAT_t KWH018ST01_4WSPI_begin(
    KWH018ST01_4WSPI_t *display,
    SPI_Type *spiBase,
    GPIO_Type *dcGpio,
    uint32_t dcPin,
    GPIO_Type *csGpio,
    uint32_t csPin,
    KWH018ST01_backlight_write_fn backlightWrite,
    void *backlightContext,
    uint32_t spiFreqHz)
{
    hyperdisplay_t *hd;
    uint8_t warmupByte = 0x00U;
    ILI9163C_STAT_t status;

    if ((display == NULL) ||
        (spiBase == NULL) ||
        (dcGpio == NULL) ||
        (csGpio == NULL))
    {
        return ILI9163C_STAT_Error;
    }

    /*
     * Configure the generic ILI9163C 4-wire transport.
     *
     * SPI pin muxing and SPI_MasterInit() are intentionally performed by the
     * KL25Z board/application layer before this function is called.
     */
    ILI9163C_4WSPI_init(
        &display->ili9163c4wspi,
        KWH018ST01_WIDTH,
        KWH018ST01_HEIGHT,
        spiBase,
        dcGpio,
        dcPin,
        csGpio,
        csPin);

    display->backlightWrite = backlightWrite;
    display->backlightContext = backlightContext;
    display->backlightLevel = 255U;

    if (spiFreqHz == 0U)
    {
        spiFreqHz = ILI9163C_SPI_DEFAULT_FREQ_HZ;
    }

    status = ILI9163C_4WSPI_setSPIFreq(
        &display->ili9163c4wspi,
        spiFreqHz);

    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    /* Default idle states: CS high, D/C high. */
    GPIO_WritePinOutput(csGpio, csPin, 1U);
    GPIO_WritePinOutput(dcGpio, dcPin, 1U);

    /* Original library starts with the backlight enabled. */
    if (display->backlightWrite != NULL)
    {
        display->backlightWrite(display->backlightContext, 255U);
    }

    hd = KWH018ST01_hyperdisplay(display);

    if (hd == NULL)
    {
        return ILI9163C_STAT_Error;
    }

    KWH018ST01_4WSPI_setWindowDefaults(display, hd->pCurrentWindow);

    /*
     * Preserve the original one-byte SPI warm-up transaction. CS remains high,
     * so the TFT does not interpret this byte as a command or data byte.
     */
    status = ILI9163C_4WSPI_transferSPIbuffer(
        &display->ili9163c4wspi,
        &warmupByte,
        1U);

    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    KWH018ST01_4WSPI_startup(display);

    status = KWH018ST01_4WSPI_defaultConfigure(display);

    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    return ILI9163C_STAT_Nominal;
}

/* -------------------------------------------------------------------------- */
/* Recommended KWH018ST01 configuration                                       */
/* -------------------------------------------------------------------------- */

ILI9163C_STAT_t KWH018ST01_4WSPI_defaultConfigure(
    KWH018ST01_4WSPI_t *display)
{
    ILI9163C_t *ili;
    ILI9163C_STAT_t status;

    /*
     * The original source declares 16-byte arrays but supplies 15 explicit
     * values. C/C++ therefore zero-initializes byte 16. We make that final
     * zero explicit here so the behavior is unambiguous.
     */
    static const uint8_t pgam[16] =
    {
        0x36U, 0x29U, 0x12U, 0x22U,
        0x1CU, 0x15U, 0x42U, 0xB7U,
        0x2FU, 0x13U, 0x12U, 0x0AU,
        0x11U, 0x0BU, 0x06U, 0x00U
    };

    static const uint8_t ngam[16] =
    {
        0x09U, 0x16U, 0x2DU, 0x0DU,
        0x13U, 0x15U, 0x40U, 0x48U,
        0x53U, 0x0CU, 0x1DU, 0x25U,
        0x2EU, 0x34U, 0x39U, 0x00U
    };

    if (display == NULL)
    {
        return ILI9163C_STAT_Error;
    }

    ili = KWH018ST01_controller(display);

    if (ili == NULL)
    {
        return ILI9163C_STAT_Error;
    }

    status = ILI9163C_sleepOut(ili);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    KWH018ST01_delayMs(20U);

    status = ILI9163C_selectGammaCurve(ili, 0x04U);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setNormalFramerate(ili, 0x0CU, 0x14U);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setPowerControl1(ili, 0x0CU, 0x05U);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setPowerControl2(ili, 0x02U);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setPowerControl3(ili, 0x02U);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setVCOMControl1(ili, 0x20U, 0x55U);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setVCOMOffsetControl(ili, false, 0x40U);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setInterfacePixelFormat(ili, 0x06U);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setColumnAddress(
        ili,
        KWH018ST01_START_COL,
        KWH018ST01_STOP_COL);

    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setRowAddress(
        ili,
        KWH018ST01_START_ROW,
        KWH018ST01_STOP_ROW);

    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setMemoryAccessControl(
        ili,
        true,
        true,
        false,
        false,
        true,
        false);

    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setSrcDriverDir(ili, false);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setGamRSel(ili, true);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setPositiveGamCorr(ili, pgam);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    status = ILI9163C_setNegativeGamCorr(ili, ngam);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    KWH018ST01_delayMs(20U);

    status = ILI9163C_setPower(ili, true);
    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    return ILI9163C_STAT_Nominal;
}

/* -------------------------------------------------------------------------- */
/* Startup                                                                    */
/* -------------------------------------------------------------------------- */

void KWH018ST01_4WSPI_startup(KWH018ST01_4WSPI_t *display)
{
    ILI9163C_4WSPI_t *spi;

    if (display == NULL)
    {
        return;
    }

    spi = &display->ili9163c4wspi;

    /*
     * The SparkFun KWH018ST01 breakout does not require a reset pin in the
     * original begin() API. The generic ILI9163C transport still supports one
     * if a future board exposes it.
     */
    if (spi->hasReset && (spi->rstGpio != NULL))
    {
        GPIO_WritePinOutput(spi->rstGpio, spi->rstPin, 1U);
    }

    KWH018ST01_delayMs(10U);

    if (spi->hasReset && (spi->rstGpio != NULL))
    {
        GPIO_WritePinOutput(spi->rstGpio, spi->rstPin, 0U);
    }

    KWH018ST01_delayMs(10U);

    if (spi->hasReset && (spi->rstGpio != NULL))
    {
        GPIO_WritePinOutput(spi->rstGpio, spi->rstPin, 1U);
    }

    KWH018ST01_delayMs(120U);
}

/* -------------------------------------------------------------------------- */
/* Character information                                                      */
/* -------------------------------------------------------------------------- */

void KWH018ST01_4WSPI_getCharInfo(
    hyperdisplay_t *display,
    uint8_t val,
    char_info_t *pchar)
{
    if ((display == NULL) || (pchar == NULL))
    {
        return;
    }

    /*
     * The original KWH018ST01 .cpp never implemented getCharInfo(); it only
     * contained a commented example for another display. Our HyperDisplay C
     * port already provides the default 5x7-font fallback, so use that.
     *
     * This function is intentionally not installed as display->ops.getCharInfo,
     * avoiding recursion through hyperdisplay_getCharInfo().
     */
    hyperdisplay_getCharInfo(display, val, pchar);
}

/* -------------------------------------------------------------------------- */
/* Specialized drawing                                                        */
/* -------------------------------------------------------------------------- */

void KWH018ST01_4WSPI_clearDisplay(KWH018ST01_4WSPI_t *display)
{
    hyperdisplay_t *hd;
    wind_info_t *previousWindow;
    wind_info_t window;
    ILI9163C_color_18_t black;

    if (display == NULL)
    {
        return;
    }

    hd = KWH018ST01_hyperdisplay(display);

    if (hd == NULL)
    {
        return;
    }

    previousWindow = hd->pCurrentWindow;

    hd->pCurrentWindow = &window;

    KWH018ST01_4WSPI_setWindowDefaults(display, &window);

    black.r = 0U;
    black.g = 0U;
    black.b = 0U;

    hyperdisplay_fillWindow(
        hd,
        (color_t)&black,
        1U,
        0U);

    hd->pCurrentWindow = previousWindow;
}

/* -------------------------------------------------------------------------- */
/* Window defaults                                                            */
/* -------------------------------------------------------------------------- */

void KWH018ST01_4WSPI_setWindowDefaults(
    KWH018ST01_4WSPI_t *display,
    wind_info_t *pwindow)
{
    hyperdisplay_t *hd;

    if ((display == NULL) || (pwindow == NULL))
    {
        return;
    }

    hd = KWH018ST01_hyperdisplay(display);

    if (hd == NULL)
    {
        return;
    }

    memset(pwindow, 0, sizeof(*pwindow));

    pwindow->xMin = KWH018ST01_START_COL;
    pwindow->yMin = KWH018ST01_START_ROW;
    pwindow->xMax = KWH018ST01_STOP_COL;
    pwindow->yMax = KWH018ST01_STOP_ROW;

    pwindow->cursorX = 0;
    pwindow->cursorY = 0;

    pwindow->xReset = 0;
    pwindow->yReset = 0;

    pwindow->lastCharacter.data = NULL;
    pwindow->lastCharacter.xLoc = NULL;
    pwindow->lastCharacter.yLoc = NULL;
    pwindow->lastCharacter.xDim = 0U;
    pwindow->lastCharacter.yDim = 0U;
    pwindow->lastCharacter.numPixels = 0U;
    pwindow->lastCharacter.show = false;
    pwindow->lastCharacter.causesNewline = false;

    /* Start in direct-to-display mode. */
    pwindow->bufferMode = false;
    pwindow->data = NULL;
    pwindow->numPixels = 0U;
    pwindow->dynamic = false;

    hyperdisplay_setWindowColorSequence(
        hd,
        pwindow,
        NULL,
        1U,
        0U);
}

/* -------------------------------------------------------------------------- */
/* Backlight                                                                  */
/* -------------------------------------------------------------------------- */

void KWH018ST01_4WSPI_setBacklight(
    KWH018ST01_4WSPI_t *display,
    uint8_t level)
{
    if (display == NULL)
    {
        return;
    }

    display->backlightLevel = level;

    if (display->backlightWrite != NULL)
    {
        display->backlightWrite(
            display->backlightContext,
            level);
    }
}

