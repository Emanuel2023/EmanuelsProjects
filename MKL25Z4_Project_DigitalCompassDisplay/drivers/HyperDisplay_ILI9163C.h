/*
 * HyperDisplay_ILI9163C.h
 *
 * Pure-C port for FRDM-KL25Z / MCUXpresso SDK.
 * Original SparkFun HyperDisplay ILI9163C controller interface by Owen Lyke.
 *
 * This file preserves the ILI9163C controller API while replacing Arduino/C++
 * classes, inheritance, constructors, virtual functions, SPIClass, and
 * SPISettings with C structs, callbacks, and NXP SDK peripheral types.
 */

#ifndef HYPERDISPLAY_ILI9163C_H_
#define HYPERDISPLAY_ILI9163C_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "hyperdisplay.h"
#include "fsl_gpio.h"
#include "fsl_spi.h"

/* -------------------------------------------------------------------------- */
/* ILI9163C controller limits                                                 */
/* -------------------------------------------------------------------------- */

#define ILI9163C_MAX_X       132U
#define ILI9163C_MAX_Y       162U
#define ILI9163C_START_COL   0U
#define ILI9163C_STOP_COL    131U
#define ILI9163C_START_ROW   0U
#define ILI9163C_STOP_ROW    161U
#define ILI9163C_MAX_BPP     3U

/* -------------------------------------------------------------------------- */
/* SPI defaults from the original SparkFun implementation                    */
/* -------------------------------------------------------------------------- */

#define ILI9163C_SPI_DEFAULT_FREQ_HZ 24000000U
#define ILI9163C_SPI_MAX_FREQ_HZ     32000000U

/* Arduino SPI_MODE0 / MSBFIRST equivalents for the KL25Z SDK. */
#define ILI9163C_SPI_POLARITY   kSPI_ClockPolarityActiveHigh
#define ILI9163C_SPI_PHASE      kSPI_ClockPhaseFirstEdge
#define ILI9163C_SPI_DIRECTION  kSPI_MsbFirst

/* -------------------------------------------------------------------------- */
/* Status                                                                     */
/* -------------------------------------------------------------------------- */

typedef enum
{
    ILI9163C_STAT_Nominal = 0x00,
    ILI9163C_STAT_Error
} ILI9163C_STAT_t;

/* -------------------------------------------------------------------------- */
/* Controller commands                                                       */
/* -------------------------------------------------------------------------- */

typedef enum
{
    ILI9163C_CMD_NOP = 0x00,
    ILI9163C_CMD_SWRST,

    ILI9163C_CMD_RDIDI = 0x04,

    ILI9163C_CMD_RDSTS = 0x09,
    ILI9163C_CMD_RDPWR,
    ILI9163C_CMD_RDMADCTL,
    ILI9163C_CMD_RDPXFMT,
    ILI9163C_CMD_RDIM,
    ILI9163C_CMD_RDSM,
    ILI9163C_CMD_RDSM2,
    ILI9163C_CMD_SLPIN,
    ILI9163C_CMD_SLPOUT,
    ILI9163C_CMD_PTLON,
    ILI9163C_CMD_NMLON,

    ILI9163C_CMD_INVOFF = 0x20,
    ILI9163C_CMD_INVON,

    ILI9163C_CMD_GAMST = 0x26,

    ILI9163C_CMD_OFF = 0x28,
    ILI9163C_CMD_ON,
    ILI9163C_CMD_CASET,
    ILI9163C_CMD_RASET,
    ILI9163C_CMD_WRRAM,
    ILI9163C_CMD_WRCS,

    ILI9163C_CMD_PTLAREA = 0x30,

    ILI9163C_CMD_WRVSCRL = 0x33,
    ILI9163C_CMD_TELOFF,
    ILI9163C_CMD_TELON,
    ILI9163C_CMD_WRMADCTL,
    ILI9163C_CMD_WRVSSA,
    ILI9163C_CMD_IDLOFF,
    ILI9163C_CMD_IDLON,
    ILI9163C_CMD_WRPXFMT,

    ILI9163C_CMD_WRNMLFRCTL = 0xB1,
    ILI9163C_CMD_WRIDLFRCTL,
    ILI9163C_CMD_WRPTLFRCTL,
    ILI9163C_CMD_WRDICTL,
    ILI9163C_CMD_WRIBPS,
    ILI9163C_CMD_WRDF,
    ILI9163C_CMD_WRSDRVDIR,
    ILI9163C_CMD_WRGDRVDIR,

    ILI9163C_CMD_WRPWCTL1 = 0xC0,
    ILI9163C_CMD_WRPWCTL2,
    ILI9163C_CMD_WRPWCTL3,
    ILI9163C_CMD_WRPWCTL4,
    ILI9163C_CMD_WRPWCTL5,
    ILI9163C_CMD_WRVCOMCTL1,
    ILI9163C_CMD_WRVCOMCTL2,
    ILI9163C_CMD_WRVCMOFSTCTL,

    ILI9163C_CMD_WRID4 = 0xD3,

    ILI9163C_CMD_WRNVMFCTL1 = 0xD5,
    ILI9163C_CMD_WRNVMFCTL2,
    ILI9163C_CMD_WRNVMFCTL3,

    ILI9163C_CMD_RDID1 = 0xDA,
    ILI9163C_CMD_RDID2,
    ILI9163C_CMD_RDID3,

    ILI9163C_CMD_WRPGCS = 0xE0,
    ILI9163C_CMD_WRNGCS,

    ILI9163C_CMD_WRGAMRS = 0xF2
} ILI9163C_CMD_t;

/* -------------------------------------------------------------------------- */
/* Interface and pixel formats                                                */
/* -------------------------------------------------------------------------- */

typedef enum
{
    ILI9163C_INTFC_4WSPI = 0x00,
    ILI9163C_INTFC_3WSPI,
    ILI9163C_INTFC_8080
} ILI9163C_INTFC_t;

typedef enum
{
    ILI9163C_PXLFMT_12 = 0x03,
    ILI9163C_PXLFMT_16 = 0x05,
    ILI9163C_PXLFMT_18 = 0x06
} ILI9163C_PXLFMT_t;

/* -------------------------------------------------------------------------- */
/* Color representations                                                      */
/* -------------------------------------------------------------------------- */

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ILI9163C_color_18_t;

typedef struct
{
    uint8_t rgh; /* Red + green high bits. */
    uint8_t glb; /* Green low + blue bits. */
} ILI9163C_color_16_t;

typedef struct
{
    uint8_t b0;
    uint8_t b1;
} ILI9163C_color_12_t;

/* -------------------------------------------------------------------------- */
/* ILI9163C controller object                                                 */
/* -------------------------------------------------------------------------- */

struct ILI9163C;
typedef struct ILI9163C ILI9163C_t;

/* Replacement for the original pure-virtual writePacket() method. */
typedef ILI9163C_STAT_t (*ILI9163C_write_packet_fn)(
    ILI9163C_t *display,
    const ILI9163C_CMD_t *pcmd,
    const uint8_t *pdata,
    uint16_t dlen);

struct ILI9163C
{
    /*
     * Must remain the first member. HyperDisplay callbacks receive a
     * hyperdisplay_t* and the implementation casts it back to ILI9163C_t*.
     */
    hyperdisplay_t hyperdisplay;

    ILI9163C_INTFC_t intfc;
    ILI9163C_PXLFMT_t pxlfmt;

    ILI9163C_write_packet_fn writePacket;
};

/* -------------------------------------------------------------------------- */
/* Controller initialization                                                  */
/* -------------------------------------------------------------------------- */

void ILI9163C_init(
    ILI9163C_t *display,
    uint16_t xSize,
    uint16_t ySize,
    ILI9163C_INTFC_t intfc,
    ILI9163C_write_packet_fn writePacket);

/* -------------------------------------------------------------------------- */
/* HyperDisplay callback implementations                                      */
/* -------------------------------------------------------------------------- */

color_t ILI9163C_getOffsetColor(
    hyperdisplay_t *display,
    color_t base,
    uint32_t numPixels);

void ILI9163C_hwpixel(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset);

void ILI9163C_swpixel(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset);

/* -------------------------------------------------------------------------- */
/* Color conversion helpers                                                   */
/* -------------------------------------------------------------------------- */

ILI9163C_color_18_t ILI9163C_hsvTo18b(uint16_t h, uint8_t s, uint8_t v);
ILI9163C_color_16_t ILI9163C_hsvTo16b(uint16_t h, uint8_t s, uint8_t v);
ILI9163C_color_12_t ILI9163C_hsvTo12b(uint16_t h, uint8_t s, uint8_t v, uint8_t odd);

ILI9163C_color_18_t ILI9163C_rgbTo18b(uint8_t r, uint8_t g, uint8_t b);
ILI9163C_color_16_t ILI9163C_rgbTo16b(uint8_t r, uint8_t g, uint8_t b);
ILI9163C_color_12_t ILI9163C_rgbTo12b(uint8_t r, uint8_t g, uint8_t b, uint8_t odd);

/* -------------------------------------------------------------------------- */
/* Utility                                                                    */
/* -------------------------------------------------------------------------- */

uint8_t ILI9163C_getBytesPerPixel(const ILI9163C_t *display);

/* -------------------------------------------------------------------------- */
/* Basic controller operations                                                */
/* -------------------------------------------------------------------------- */

ILI9163C_STAT_t ILI9163C_swReset(ILI9163C_t *display);
ILI9163C_STAT_t ILI9163C_sleepIn(ILI9163C_t *display);
ILI9163C_STAT_t ILI9163C_sleepOut(ILI9163C_t *display);
ILI9163C_STAT_t ILI9163C_partialModeOn(ILI9163C_t *display);
ILI9163C_STAT_t ILI9163C_normalDisplayModeOn(ILI9163C_t *display);
ILI9163C_STAT_t ILI9163C_setInversion(ILI9163C_t *display, bool on);
ILI9163C_STAT_t ILI9163C_setPower(ILI9163C_t *display, bool on);
ILI9163C_STAT_t ILI9163C_setColumnAddress(ILI9163C_t *display, uint16_t start, uint16_t end);
ILI9163C_STAT_t ILI9163C_setRowAddress(ILI9163C_t *display, uint16_t start, uint16_t end);
ILI9163C_STAT_t ILI9163C_writeToRAM(ILI9163C_t *display, const uint8_t *pdata, uint16_t numBytes);

/* -------------------------------------------------------------------------- */
/* Full display configuration                                                 */
/* -------------------------------------------------------------------------- */

ILI9163C_STAT_t ILI9163C_setMemoryAccessControl(
    ILI9163C_t *display,
    bool mx,
    bool my,
    bool mv,
    bool ml,
    bool bgr,
    bool mh);

ILI9163C_STAT_t ILI9163C_selectGammaCurve(ILI9163C_t *display, uint8_t bmNumber);
ILI9163C_STAT_t ILI9163C_setPartialArea(ILI9163C_t *display, uint16_t start, uint16_t end);
ILI9163C_STAT_t ILI9163C_setVerticalScrolling(ILI9163C_t *display, uint16_t tfa, uint16_t vsa, uint16_t bfa);
ILI9163C_STAT_t ILI9163C_setVerticalScrollingStartAddress(ILI9163C_t *display, uint16_t ssa);
ILI9163C_STAT_t ILI9163C_setIdleMode(ILI9163C_t *display, bool on);
ILI9163C_STAT_t ILI9163C_setInterfacePixelFormat(ILI9163C_t *display, uint8_t CTRLintfc);
ILI9163C_STAT_t ILI9163C_setTearingEffectLine(ILI9163C_t *display, bool on);
ILI9163C_STAT_t ILI9163C_setNormalFramerate(ILI9163C_t *display, uint8_t diva, uint8_t vpa);
ILI9163C_STAT_t ILI9163C_setIdleFramerate(ILI9163C_t *display, uint8_t divb, uint8_t vpb);
ILI9163C_STAT_t ILI9163C_setPartialFramerate(ILI9163C_t *display, uint8_t divc, uint8_t vpc);
ILI9163C_STAT_t ILI9163C_setPowerControl1(ILI9163C_t *display, uint8_t vrh, uint8_t vc);
ILI9163C_STAT_t ILI9163C_setPowerControl2(ILI9163C_t *display, uint8_t bt);
ILI9163C_STAT_t ILI9163C_setPowerControl3(ILI9163C_t *display, uint8_t apa);
ILI9163C_STAT_t ILI9163C_setPowerControl4(ILI9163C_t *display, uint8_t apb);
ILI9163C_STAT_t ILI9163C_setPowerControl5(ILI9163C_t *display, uint8_t apc);
ILI9163C_STAT_t ILI9163C_setVCOMControl1(ILI9163C_t *display, uint8_t vmh, uint8_t vml);
ILI9163C_STAT_t ILI9163C_setVCOMControl2(ILI9163C_t *display, uint8_t vma);
ILI9163C_STAT_t ILI9163C_setVCOMOffsetControl(ILI9163C_t *display, bool nVM, uint8_t vmf);
ILI9163C_STAT_t ILI9163C_setSrcDriverDir(ILI9163C_t *display, bool crl);
ILI9163C_STAT_t ILI9163C_setGateDriverDir(ILI9163C_t *display, bool ctb);
ILI9163C_STAT_t ILI9163C_setGamRSel(ILI9163C_t *display, bool gamrsel);
ILI9163C_STAT_t ILI9163C_setPositiveGamCorr(ILI9163C_t *display, const uint8_t *gam16byte);
ILI9163C_STAT_t ILI9163C_setNegativeGamCorr(ILI9163C_t *display, const uint8_t *gam16byte);

/* -------------------------------------------------------------------------- */
/* KL25Z 4-wire SPI transport                                                 */
/* -------------------------------------------------------------------------- */

typedef struct ILI9163C_4WSPI
{
    /* Must remain first so this object can be treated as ILI9163C_t*. */
    ILI9163C_t ili9163c;

    SPI_Type *spiBase;

    GPIO_Type *dcGpio;
    uint32_t dcPin;

    GPIO_Type *csGpio;
    uint32_t csPin;

    /* Optional reset support for displays that expose a reset pin. */
    GPIO_Type *rstGpio;
    uint32_t rstPin;
    bool hasReset;

    uint32_t spiFreqHz;
} ILI9163C_4WSPI_t;

/*
 * Initialize the generic KL25Z 4-wire SPI transport object.
 *
 * SPI SCK/MOSI pin muxing is performed by the board/pin-mux configuration,
 * not by this library. For the current KL25Z TFT wiring that means SPI0 uses
 * PTD1 for SCK and PTD2 for SOUT/MOSI, while LCDCS and D/C are GPIO outputs.
 */
void ILI9163C_4WSPI_init(
    ILI9163C_4WSPI_t *display,
    uint16_t xSize,
    uint16_t ySize,
    SPI_Type *spiBase,
    GPIO_Type *dcGpio,
    uint32_t dcPin,
    GPIO_Type *csGpio,
    uint32_t csPin);

void ILI9163C_4WSPI_setResetPin(
    ILI9163C_4WSPI_t *display,
    GPIO_Type *rstGpio,
    uint32_t rstPin);

ILI9163C_STAT_t ILI9163C_4WSPI_writePacket(
    ILI9163C_t *base,
    const ILI9163C_CMD_t *pcmd,
    const uint8_t *pdata,
    uint16_t dlen);

ILI9163C_STAT_t ILI9163C_4WSPI_selectDriver(ILI9163C_4WSPI_t *display);
ILI9163C_STAT_t ILI9163C_4WSPI_deselectDriver(ILI9163C_4WSPI_t *display);
ILI9163C_STAT_t ILI9163C_4WSPI_setSPIFreq(ILI9163C_4WSPI_t *display, uint32_t freq);

ILI9163C_STAT_t ILI9163C_4WSPI_transferSPIbuffer(
    ILI9163C_4WSPI_t *display,
    const uint8_t *pdata,
    size_t count);

/* Accelerated HyperDisplay operations implemented by the 4-wire SPI layer. */
void ILI9163C_4WSPI_hwxline(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goLeft);

void ILI9163C_4WSPI_hwyline(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goUp);

void ILI9163C_4WSPI_hwfillFromArray(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t x1,
    hd_hw_extent_t y1,
    color_t data,
    hd_pixels_t numPixels,
    bool Vh);

#endif /* HYPERDISPLAY_ILI9163C_H_ */
