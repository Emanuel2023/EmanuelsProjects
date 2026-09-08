/*
 * HyperDisplay_ILI9163C.c
 *
 * Pure-C port for FRDM-KL25Z / MCUXpresso SDK.
 * Original SparkFun HyperDisplay ILI9163C implementation by Owen Lyke.
 *
 * Porting notes:
 *  - C++ methods/constructors are replaced by C functions.
 *  - Arduino SPIClass/SPISettings/digitalWrite are replaced with the
 *    MCUXpresso SDK SPI and GPIO APIs.
 *  - Chip select and D/C are software-controlled GPIO signals.
 *  - SPI pin mux and SPI_MasterInit() remain the responsibility of the
 *    board/application layer.
 */

#include "HyperDisplay_ILI9163C.h"
#include "fast_hsv2rgb.h"

#include <string.h>

/* -------------------------------------------------------------------------- */
/* Internal helpers                                                           */
/* -------------------------------------------------------------------------- */

static ILI9163C_t *ILI9163C_fromHyperdisplay(hyperdisplay_t *display)
{
    /*
     * ILI9163C_t begins with hyperdisplay_t, so both pointers have the same
     * address. This replaces the original C++ inheritance relationship.
     */
    return (ILI9163C_t *)display;
}

static ILI9163C_4WSPI_t *ILI9163C_4WSPI_fromBase(ILI9163C_t *base)
{
    /*
     * ILI9163C_4WSPI_t begins with ILI9163C_t, so both pointers have the same
     * address. This replaces the original C++ derived-class relationship.
     */
    return (ILI9163C_4WSPI_t *)base;
}

static ILI9163C_4WSPI_t *ILI9163C_4WSPI_fromHyperdisplay(hyperdisplay_t *display)
{
    return (ILI9163C_4WSPI_t *)display;
}

static ILI9163C_STAT_t ILI9163C_callWritePacket(
    ILI9163C_t *display,
    const ILI9163C_CMD_t *pcmd,
    const uint8_t *pdata,
    uint16_t dlen)
{
    if ((display == NULL) || (display->writePacket == NULL))
    {
        return ILI9163C_STAT_Error;
    }

    return display->writePacket(display, pcmd, pdata, dlen);
}

/* -------------------------------------------------------------------------- */
/* Initialization                                                             */
/* -------------------------------------------------------------------------- */

void ILI9163C_init(
    ILI9163C_t *display,
    uint16_t xSize,
    uint16_t ySize,
    ILI9163C_INTFC_t intfc,
    ILI9163C_write_packet_fn writePacket)
{
    if (display == NULL)
    {
        return;
    }

    hyperdisplay_init(&display->hyperdisplay, xSize, ySize);

    display->intfc = intfc;

    /*
     * The original constructor did not initialize _pxlfmt. Use 16-bit RGB565
     * as a safe default until setInterfacePixelFormat() selects another mode.
     */
    display->pxlfmt = ILI9163C_PXLFMT_16;
    display->writePacket = writePacket;

    /* Implement HyperDisplay's required display-specific callbacks. */
    display->hyperdisplay.ops.getOffsetColor = ILI9163C_getOffsetColor;
    display->hyperdisplay.ops.hwpixel = ILI9163C_hwpixel;
    display->hyperdisplay.ops.swpixel = ILI9163C_swpixel;
}

/* -------------------------------------------------------------------------- */
/* Color conversion                                                           */
/* -------------------------------------------------------------------------- */

ILI9163C_color_18_t ILI9163C_hsvTo18b(uint16_t h, uint8_t s, uint8_t v)
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    fast_hsv2rgb_8bit(h, s, v, &r, &g, &b);

    return ILI9163C_rgbTo18b(r, g, b);
}

ILI9163C_color_16_t ILI9163C_hsvTo16b(uint16_t h, uint8_t s, uint8_t v)
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    fast_hsv2rgb_8bit(h, s, v, &r, &g, &b);

    return ILI9163C_rgbTo16b(r, g, b);
}

ILI9163C_color_12_t ILI9163C_hsvTo12b(
    uint16_t h,
    uint8_t s,
    uint8_t v,
    uint8_t odd)
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    fast_hsv2rgb_8bit(h, s, v, &r, &g, &b);

    return ILI9163C_rgbTo12b(r, g, b, odd);
}

ILI9163C_color_18_t ILI9163C_rgbTo18b(uint8_t r, uint8_t g, uint8_t b)
{
    ILI9163C_color_18_t retval;

    retval.r = r;
    retval.g = g;
    retval.b = b;

    return retval;
}

ILI9163C_color_16_t ILI9163C_rgbTo16b(uint8_t r, uint8_t g, uint8_t b)
{
    ILI9163C_color_16_t retval;

    retval.rgh = (uint8_t)((r & 0xF8U) | (g >> 5U));
    retval.glb = (uint8_t)(((g & 0x1CU) << 3U) | (b >> 3U));

    return retval;
}

ILI9163C_color_12_t ILI9163C_rgbTo12b(
    uint8_t r,
    uint8_t g,
    uint8_t b,
    uint8_t odd)
{
    ILI9163C_color_12_t retval;

    if (odd != 0U)
    {
        retval.b0 = (uint8_t)(r >> 4U);
        retval.b1 = (uint8_t)((g & 0xF0U) | (b >> 4U));
    }
    else
    {
        retval.b0 = (uint8_t)((r & 0xF0U) | (g >> 4U));
        retval.b1 = (uint8_t)(b & 0xF0U);
    }

    return retval;
}

/* -------------------------------------------------------------------------- */
/* HyperDisplay callbacks                                                     */
/* -------------------------------------------------------------------------- */

uint8_t ILI9163C_getBytesPerPixel(const ILI9163C_t *display)
{
    if (display == NULL)
    {
        return 0U;
    }

    switch (display->pxlfmt)
    {
        case ILI9163C_PXLFMT_18:
            return (uint8_t)(offsetof(ILI9163C_color_18_t, b) + 1U);

        case ILI9163C_PXLFMT_16:
            return (uint8_t)(offsetof(ILI9163C_color_16_t, glb) + 1U);

        case ILI9163C_PXLFMT_12:
            return (uint8_t)(offsetof(ILI9163C_color_12_t, b1) + 1U);

        default:
            return 0U;
    }
}

color_t ILI9163C_getOffsetColor(
    hyperdisplay_t *display,
    color_t base,
    uint32_t numPixels)
{
    ILI9163C_t *ili = ILI9163C_fromHyperdisplay(display);

    if ((ili == NULL) || (base == NULL))
    {
        return base;
    }

    switch (ili->pxlfmt)
    {
        case ILI9163C_PXLFMT_18:
            return (color_t)(((ILI9163C_color_18_t *)base) + numPixels);

        case ILI9163C_PXLFMT_16:
            return (color_t)(((ILI9163C_color_16_t *)base) + numPixels);

        case ILI9163C_PXLFMT_12:
            return (color_t)(((ILI9163C_color_12_t *)base) + numPixels);

        default:
            return base;
    }
}

void ILI9163C_hwpixel(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset)
{
    ILI9163C_t *ili = ILI9163C_fromHyperdisplay(display);
    color_t value;
    uint8_t len;

    if ((ili == NULL) || (data == NULL) || (colorCycleLength == 0U))
    {
        return;
    }

    startColorOffset =
        hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                       (uint16_t)startColorOffset,
                                       0);

    value = ILI9163C_getOffsetColor(display, data, startColorOffset);
    len = ILI9163C_getBytesPerPixel(ili);

    if ((value == NULL) || (len == 0U))
    {
        return;
    }

    if (ILI9163C_setColumnAddress(ili, (uint16_t)x0, (uint16_t)x0) !=
        ILI9163C_STAT_Nominal)
    {
        return;
    }

    if (ILI9163C_setRowAddress(ili, (uint16_t)y0, (uint16_t)y0) !=
        ILI9163C_STAT_Nominal)
    {
        return;
    }

    (void)ILI9163C_writeToRAM(ili, (const uint8_t *)value, len);
}

void ILI9163C_swpixel(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset)
{
    ILI9163C_t *ili = ILI9163C_fromHyperdisplay(display);
    wind_info_t *window;
    hd_pixels_t pixOffset;
    color_t value;
    color_t dest;
    uint8_t len;

    if ((ili == NULL) ||
        (data == NULL) ||
        (colorCycleLength == 0U))
    {
        return;
    }

    window = display->pCurrentWindow;

    if ((window == NULL) || (window->data == NULL))
    {
        return;
    }

    startColorOffset =
        hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                       (uint16_t)startColorOffset,
                                       0);

    value = ILI9163C_getOffsetColor(display, data, startColorOffset);

    pixOffset = hyperdisplay_wToPix(window,
                                    (hd_hw_extent_t)x0,
                                    (hd_hw_extent_t)y0);

    if (pixOffset >= window->numPixels)
    {
        return;
    }

    dest = ILI9163C_getOffsetColor(display, window->data, pixOffset);
    len = ILI9163C_getBytesPerPixel(ili);

    if ((value == NULL) || (dest == NULL) || (len == 0U))
    {
        return;
    }

    memcpy(dest, value, (size_t)len);
}

/* -------------------------------------------------------------------------- */
/* Basic controller commands                                                  */
/* -------------------------------------------------------------------------- */

ILI9163C_STAT_t ILI9163C_swReset(ILI9163C_t *display)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_SWRST;
    return ILI9163C_callWritePacket(display, &cmd, NULL, 0U);
}

ILI9163C_STAT_t ILI9163C_sleepIn(ILI9163C_t *display)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_SLPIN;
    return ILI9163C_callWritePacket(display, &cmd, NULL, 0U);
}

ILI9163C_STAT_t ILI9163C_sleepOut(ILI9163C_t *display)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_SLPOUT;
    return ILI9163C_callWritePacket(display, &cmd, NULL, 0U);
}

ILI9163C_STAT_t ILI9163C_partialModeOn(ILI9163C_t *display)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_PTLON;
    return ILI9163C_callWritePacket(display, &cmd, NULL, 0U);
}

ILI9163C_STAT_t ILI9163C_normalDisplayModeOn(ILI9163C_t *display)
{
    /*
     * The original .cpp sends PTLON here even though this function is named
     * normalDisplayModeOn(). Use the controller's NMLON command instead.
     */
    ILI9163C_CMD_t cmd = ILI9163C_CMD_NMLON;
    return ILI9163C_callWritePacket(display, &cmd, NULL, 0U);
}

ILI9163C_STAT_t ILI9163C_setInversion(ILI9163C_t *display, bool on)
{
    ILI9163C_CMD_t cmd = on ? ILI9163C_CMD_INVON : ILI9163C_CMD_INVOFF;
    return ILI9163C_callWritePacket(display, &cmd, NULL, 0U);
}

ILI9163C_STAT_t ILI9163C_setPower(ILI9163C_t *display, bool on)
{
    ILI9163C_CMD_t cmd = on ? ILI9163C_CMD_ON : ILI9163C_CMD_OFF;
    return ILI9163C_callWritePacket(display, &cmd, NULL, 0U);
}

ILI9163C_STAT_t ILI9163C_setColumnAddress(
    ILI9163C_t *display,
    uint16_t start,
    uint16_t end)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_CASET;
    uint8_t buff[4] =
    {
        (uint8_t)(start >> 8U),
        (uint8_t)(start & 0x00FFU),
        (uint8_t)(end >> 8U),
        (uint8_t)(end & 0x00FFU)
    };

    return ILI9163C_callWritePacket(display, &cmd, buff, sizeof(buff));
}

ILI9163C_STAT_t ILI9163C_setRowAddress(
    ILI9163C_t *display,
    uint16_t start,
    uint16_t end)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_RASET;
    uint8_t buff[4] =
    {
        (uint8_t)(start >> 8U),
        (uint8_t)(start & 0x00FFU),
        (uint8_t)(end >> 8U),
        (uint8_t)(end & 0x00FFU)
    };

    return ILI9163C_callWritePacket(display, &cmd, buff, sizeof(buff));
}

ILI9163C_STAT_t ILI9163C_writeToRAM(
    ILI9163C_t *display,
    const uint8_t *pdata,
    uint16_t numBytes)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRRAM;

    if ((pdata == NULL) && (numBytes != 0U))
    {
        return ILI9163C_STAT_Error;
    }

    return ILI9163C_callWritePacket(display, &cmd, pdata, numBytes);
}

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
    bool mh)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRMADCTL;
    uint8_t buff = 0U;

    if (my)  { buff |= 0x80U; }
    if (mx)  { buff |= 0x40U; }
    if (mv)  { buff |= 0x20U; }
    if (ml)  { buff |= 0x10U; }
    if (bgr) { buff |= 0x08U; }
    if (mh)  { buff |= 0x04U; }

    return ILI9163C_callWritePacket(display, &cmd, &buff, 1U);
}

ILI9163C_STAT_t ILI9163C_selectGammaCurve(
    ILI9163C_t *display,
    uint8_t bmNumber)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_GAMST;
    uint8_t buff = bmNumber;

    return ILI9163C_callWritePacket(display, &cmd, &buff, 1U);
}

ILI9163C_STAT_t ILI9163C_setPartialArea(
    ILI9163C_t *display,
    uint16_t start,
    uint16_t end)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_PTLAREA;
    uint8_t buff[4] =
    {
        (uint8_t)(start >> 8U),
        (uint8_t)(start & 0x00FFU),
        (uint8_t)(end >> 8U),
        (uint8_t)(end & 0x00FFU)
    };

    return ILI9163C_callWritePacket(display, &cmd, buff, sizeof(buff));
}

ILI9163C_STAT_t ILI9163C_setVerticalScrolling(
    ILI9163C_t *display,
    uint16_t tfa,
    uint16_t vsa,
    uint16_t bfa)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRVSCRL;
    uint8_t buff[6] =
    {
        (uint8_t)(tfa >> 8U),
        (uint8_t)(tfa & 0x00FFU),
        (uint8_t)(vsa >> 8U),
        (uint8_t)(vsa & 0x00FFU),
        (uint8_t)(bfa >> 8U),
        (uint8_t)(bfa & 0x00FFU)
    };

    return ILI9163C_callWritePacket(display, &cmd, buff, sizeof(buff));
}

ILI9163C_STAT_t ILI9163C_setVerticalScrollingStartAddress(
    ILI9163C_t *display,
    uint16_t ssa)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRVSSA;
    uint8_t buff[2] =
    {
        (uint8_t)(ssa >> 8U),
        (uint8_t)(ssa & 0x00FFU)
    };

    return ILI9163C_callWritePacket(display, &cmd, buff, sizeof(buff));
}

ILI9163C_STAT_t ILI9163C_setIdleMode(ILI9163C_t *display, bool on)
{
    ILI9163C_CMD_t cmd = on ? ILI9163C_CMD_IDLON : ILI9163C_CMD_IDLOFF;
    return ILI9163C_callWritePacket(display, &cmd, NULL, 0U);
}

ILI9163C_STAT_t ILI9163C_setInterfacePixelFormat(
    ILI9163C_t *display,
    uint8_t CTRLintfc)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRPXFMT;
    uint8_t buff;
    ILI9163C_STAT_t status;

    if (display == NULL)
    {
        return ILI9163C_STAT_Error;
    }

    buff = (uint8_t)(CTRLintfc & 0x07U);

    status = ILI9163C_callWritePacket(display, &cmd, &buff, 1U);

    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    if (buff == (uint8_t)ILI9163C_PXLFMT_12)
    {
        display->pxlfmt = ILI9163C_PXLFMT_12;
    }
    else if (buff == (uint8_t)ILI9163C_PXLFMT_16)
    {
        display->pxlfmt = ILI9163C_PXLFMT_16;
    }
    else if (buff == (uint8_t)ILI9163C_PXLFMT_18)
    {
        display->pxlfmt = ILI9163C_PXLFMT_18;
    }

    return ILI9163C_STAT_Nominal;
}

ILI9163C_STAT_t ILI9163C_setTearingEffectLine(
    ILI9163C_t *display,
    bool on)
{
    ILI9163C_CMD_t cmd = on ? ILI9163C_CMD_TELON : ILI9163C_CMD_TELOFF;
    return ILI9163C_callWritePacket(display, &cmd, NULL, 0U);
}

ILI9163C_STAT_t ILI9163C_setNormalFramerate(
    ILI9163C_t *display,
    uint8_t diva,
    uint8_t vpa)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRNMLFRCTL;
    uint8_t buff[2] = {diva, vpa};
    return ILI9163C_callWritePacket(display, &cmd, buff, sizeof(buff));
}

ILI9163C_STAT_t ILI9163C_setIdleFramerate(
    ILI9163C_t *display,
    uint8_t divb,
    uint8_t vpb)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRIDLFRCTL;
    uint8_t buff[2] = {divb, vpb};
    return ILI9163C_callWritePacket(display, &cmd, buff, sizeof(buff));
}

ILI9163C_STAT_t ILI9163C_setPartialFramerate(
    ILI9163C_t *display,
    uint8_t divc,
    uint8_t vpc)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRPTLFRCTL;
    uint8_t buff[2] = {divc, vpc};
    return ILI9163C_callWritePacket(display, &cmd, buff, sizeof(buff));
}

ILI9163C_STAT_t ILI9163C_setPowerControl1(
    ILI9163C_t *display,
    uint8_t vrh,
    uint8_t vc)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRPWCTL1;
    uint8_t buff[2] =
    {
        (uint8_t)(vrh & 0x1FU),
        (uint8_t)(vc & 0x07U)
    };

    return ILI9163C_callWritePacket(display, &cmd, buff, sizeof(buff));
}

ILI9163C_STAT_t ILI9163C_setPowerControl2(ILI9163C_t *display, uint8_t bt)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRPWCTL2;
    uint8_t buff = (uint8_t)(bt & 0x07U);
    return ILI9163C_callWritePacket(display, &cmd, &buff, 1U);
}

ILI9163C_STAT_t ILI9163C_setPowerControl3(ILI9163C_t *display, uint8_t apa)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRPWCTL3;
    uint8_t buff = (uint8_t)(apa & 0x07U);
    return ILI9163C_callWritePacket(display, &cmd, &buff, 1U);
}

ILI9163C_STAT_t ILI9163C_setPowerControl4(ILI9163C_t *display, uint8_t apb)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRPWCTL4;
    uint8_t buff = (uint8_t)(apb & 0x07U);
    return ILI9163C_callWritePacket(display, &cmd, &buff, 1U);
}

ILI9163C_STAT_t ILI9163C_setPowerControl5(ILI9163C_t *display, uint8_t apc)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRPWCTL5;
    uint8_t buff = (uint8_t)(apc & 0x07U);
    return ILI9163C_callWritePacket(display, &cmd, &buff, 1U);
}

ILI9163C_STAT_t ILI9163C_setVCOMControl1(
    ILI9163C_t *display,
    uint8_t vmh,
    uint8_t vml)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRVCOMCTL1;
    uint8_t buff[2] =
    {
        (uint8_t)(vmh & 0x7FU),
        (uint8_t)(vml & 0x7FU)
    };

    return ILI9163C_callWritePacket(display, &cmd, buff, sizeof(buff));
}

ILI9163C_STAT_t ILI9163C_setVCOMControl2(
    ILI9163C_t *display,
    uint8_t vma)
{
    /*
     * The original SparkFun .cpp declares this API in the header but leaves
     * the implementation commented out. Keep it explicitly unsupported until
     * the required command/data definition is verified.
     */
    (void)display;
    (void)vma;

    return ILI9163C_STAT_Error;
}

ILI9163C_STAT_t ILI9163C_setVCOMOffsetControl(
    ILI9163C_t *display,
    bool nVM,
    uint8_t vmf)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRVCMOFSTCTL;
    uint8_t buff = (uint8_t)(vmf & 0x7FU);

    if (nVM)
    {
        buff |= 0x80U;
    }

    return ILI9163C_callWritePacket(display, &cmd, &buff, 1U);
}

ILI9163C_STAT_t ILI9163C_setSrcDriverDir(ILI9163C_t *display, bool crl)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRSDRVDIR;
    uint8_t buff = crl ? 0x01U : 0x00U;
    return ILI9163C_callWritePacket(display, &cmd, &buff, 1U);
}

ILI9163C_STAT_t ILI9163C_setGateDriverDir(ILI9163C_t *display, bool ctb)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRGDRVDIR;
    uint8_t buff = ctb ? 0x01U : 0x00U;
    return ILI9163C_callWritePacket(display, &cmd, &buff, 1U);
}

ILI9163C_STAT_t ILI9163C_setGamRSel(ILI9163C_t *display, bool gamrsel)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRGAMRS;
    uint8_t buff = gamrsel ? 0x01U : 0x00U;
    return ILI9163C_callWritePacket(display, &cmd, &buff, 1U);
}

ILI9163C_STAT_t ILI9163C_setPositiveGamCorr(
    ILI9163C_t *display,
    const uint8_t *gam16byte)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRPGCS;

    if (gam16byte == NULL)
    {
        return ILI9163C_STAT_Error;
    }

    return ILI9163C_callWritePacket(display, &cmd, gam16byte, 16U);
}

ILI9163C_STAT_t ILI9163C_setNegativeGamCorr(
    ILI9163C_t *display,
    const uint8_t *gam16byte)
{
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRNGCS;

    if (gam16byte == NULL)
    {
        return ILI9163C_STAT_Error;
    }

    return ILI9163C_callWritePacket(display, &cmd, gam16byte, 16U);
}

/* -------------------------------------------------------------------------- */
/* KL25Z 4-wire SPI transport                                                 */
/* -------------------------------------------------------------------------- */

void ILI9163C_4WSPI_init(
    ILI9163C_4WSPI_t *display,
    uint16_t xSize,
    uint16_t ySize,
    SPI_Type *spiBase,
    GPIO_Type *dcGpio,
    uint32_t dcPin,
    GPIO_Type *csGpio,
    uint32_t csPin)
{
    gpio_pin_config_t outputHigh =
    {
        kGPIO_DigitalOutput,
        1U
    };

    if (display == NULL)
    {
        return;
    }

    memset(display, 0, sizeof(*display));

    display->spiBase = spiBase;
    display->dcGpio = dcGpio;
    display->dcPin = dcPin;
    display->csGpio = csGpio;
    display->csPin = csPin;
    display->spiFreqHz = ILI9163C_SPI_MAX_FREQ_HZ;
    display->hasReset = false;

    if (display->dcGpio != NULL)
    {
        GPIO_PinInit(display->dcGpio, display->dcPin, &outputHigh);
    }

    if (display->csGpio != NULL)
    {
        GPIO_PinInit(display->csGpio, display->csPin, &outputHigh);
    }

    ILI9163C_init(&display->ili9163c,
                  xSize,
                  ySize,
                  ILI9163C_INTFC_4WSPI,
                  ILI9163C_4WSPI_writePacket);

    /* Install the 4-wire SPI optimized drawing callbacks. */
    display->ili9163c.hyperdisplay.ops.hwxline = ILI9163C_4WSPI_hwxline;
    display->ili9163c.hyperdisplay.ops.hwyline = ILI9163C_4WSPI_hwyline;
    display->ili9163c.hyperdisplay.ops.hwfillFromArray =
        ILI9163C_4WSPI_hwfillFromArray;
}

void ILI9163C_4WSPI_setResetPin(
    ILI9163C_4WSPI_t *display,
    GPIO_Type *rstGpio,
    uint32_t rstPin)
{
    gpio_pin_config_t outputHigh =
    {
        kGPIO_DigitalOutput,
        1U
    };

    if (display == NULL)
    {
        return;
    }

    display->rstGpio = rstGpio;
    display->rstPin = rstPin;
    display->hasReset = (rstGpio != NULL);

    if (display->hasReset)
    {
        GPIO_PinInit(display->rstGpio, display->rstPin, &outputHigh);
    }
}

ILI9163C_STAT_t ILI9163C_4WSPI_selectDriver(ILI9163C_4WSPI_t *display)
{
    if ((display == NULL) || (display->csGpio == NULL))
    {
        return ILI9163C_STAT_Error;
    }

    GPIO_WritePinOutput(display->csGpio, display->csPin, 0U);
    return ILI9163C_STAT_Nominal;
}

ILI9163C_STAT_t ILI9163C_4WSPI_deselectDriver(ILI9163C_4WSPI_t *display)
{
    if ((display == NULL) || (display->csGpio == NULL))
    {
        return ILI9163C_STAT_Error;
    }

    GPIO_WritePinOutput(display->csGpio, display->csPin, 1U);
    return ILI9163C_STAT_Nominal;
}

ILI9163C_STAT_t ILI9163C_4WSPI_transferSPIbuffer(
    ILI9163C_4WSPI_t *display,
    const uint8_t *pdata,
    size_t count)
{
    spi_transfer_t transfer;
    status_t status;

    if ((display == NULL) ||
        (display->spiBase == NULL) ||
        ((pdata == NULL) && (count != 0U)))
    {
        return ILI9163C_STAT_Error;
    }

    if (count == 0U)
    {
        return ILI9163C_STAT_Nominal;
    }

    memset(&transfer, 0, sizeof(transfer));

    /* fsl_spi.h uses a non-const TX pointer even though TX data is not modified. */
    transfer.txData = (uint8_t *)(uintptr_t)pdata;
    transfer.rxData = NULL;
    transfer.dataSize = count;
    //transfer.configFlags = 0U;

    status = SPI_MasterTransferBlocking(display->spiBase, &transfer);

    return (status == kStatus_Success) ?
           ILI9163C_STAT_Nominal :
           ILI9163C_STAT_Error;
}

ILI9163C_STAT_t ILI9163C_4WSPI_writePacket(
    ILI9163C_t *base,
    const ILI9163C_CMD_t *pcmd,
    const uint8_t *pdata,
    uint16_t dlen)
{
    ILI9163C_4WSPI_t *display = ILI9163C_4WSPI_fromBase(base);
    ILI9163C_STAT_t status;

    if ((display == NULL) ||
        (display->spiBase == NULL) ||
        (display->dcGpio == NULL) ||
        (display->csGpio == NULL))
    {
        return ILI9163C_STAT_Error;
    }

    if ((pdata == NULL) && (dlen != 0U))
    {
        return ILI9163C_STAT_Error;
    }

    status = ILI9163C_4WSPI_selectDriver(display);

    if (status != ILI9163C_STAT_Nominal)
    {
        return status;
    }

    if (pcmd != NULL)
    {
        uint8_t command = (uint8_t)(*pcmd);

        GPIO_WritePinOutput(display->dcGpio, display->dcPin, 0U);

        status = ILI9163C_4WSPI_transferSPIbuffer(display, &command, 1U);

        if (status != ILI9163C_STAT_Nominal)
        {
            (void)ILI9163C_4WSPI_deselectDriver(display);
            return status;
        }
    }

    if ((pdata != NULL) && (dlen != 0U))
    {
        GPIO_WritePinOutput
		(display->dcGpio, display->dcPin, 1U);

        status = ILI9163C_4WSPI_transferSPIbuffer(display,
                                                 pdata,
                                                 (size_t)dlen);

        if (status != ILI9163C_STAT_Nominal)
        {
            (void)ILI9163C_4WSPI_deselectDriver(display);
            return status;
        }
    }

    return ILI9163C_4WSPI_deselectDriver(display);
}

ILI9163C_STAT_t ILI9163C_4WSPI_setSPIFreq(
    ILI9163C_4WSPI_t *display,
    uint32_t freq)
{
    if ((display == NULL) ||
        (freq == 0U) ||
        (freq > ILI9163C_SPI_MAX_FREQ_HZ))
    {
        return ILI9163C_STAT_Error;
    }

    /*
     * Arduino's SPISettings object applied the requested baud rate at each
     * beginTransaction(). The KL25Z SDK needs the SPI source-clock frequency
     * to change the hardware baud rate. This generic controller object does
     * not own that board-specific clock information, so preserve the requested
     * value here. The board/application SPI initialization should apply it via
     * SPI_MasterInit() or SPI_MasterSetBaudRate().
     */
    display->spiFreqHz = freq;

    return ILI9163C_STAT_Nominal;
}

/* -------------------------------------------------------------------------- */
/* Accelerated 4-wire SPI drawing                                             */
/* -------------------------------------------------------------------------- */

void ILI9163C_4WSPI_hwxline(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goLeft)
{
    ILI9163C_4WSPI_t *spi = ILI9163C_4WSPI_fromHyperdisplay(display);
    ILI9163C_t *ili;
    color_t value;
    uint8_t bpp;
    hd_hw_extent_t x1;

    if ((spi == NULL) ||
        (data == NULL) ||
        (len < 1U) ||
        (colorCycleLength == 0U))
    {
        return;
    }

    ili = &spi->ili9163c;

    startColorOffset =
        hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                       (uint16_t)startColorOffset,
                                       0);

    value = ILI9163C_getOffsetColor(display, data, startColorOffset);
    bpp = ILI9163C_getBytesPerPixel(ili);

    if ((value == NULL) || (bpp == 0U))
    {
        return;
    }

    if (goLeft)
    {
        (void)ILI9163C_setMemoryAccessControl(ili,
                                              false,
                                              true,
                                              false,
                                              false,
                                              true,
                                              false);
        x0 = (hd_hw_extent_t)((display->xExt - 1U) - x0);
    }

    x1 = (hd_hw_extent_t)(x0 + (len - 1U));

    if ((ILI9163C_setColumnAddress(ili, x0, x1) != ILI9163C_STAT_Nominal) ||
        (ILI9163C_setRowAddress(ili, y0, y0) != ILI9163C_STAT_Nominal))
    {
        goto restore_orientation;
    }

    {
        ILI9163C_CMD_t cmd = ILI9163C_CMD_WRRAM;

        if (ILI9163C_4WSPI_writePacket(ili, &cmd, NULL, 0U) !=
            ILI9163C_STAT_Nominal)
        {
            goto restore_orientation;
        }
    }

    if (ILI9163C_4WSPI_selectDriver(spi) != ILI9163C_STAT_Nominal)
    {
        goto restore_orientation;
    }

    GPIO_WritePinOutput(spi->dcGpio, spi->dcPin, 1U);

    if (colorCycleLength == 1U)
    {
        uint8_t speedupArray[ILI9163C_MAX_X * ILI9163C_MAX_BPP];
        uint16_t i;
        uint8_t j;

        for (i = 0U; i < len; i++)
        {
            for (j = 0U; j < bpp; j++)
            {
                speedupArray[j + ((size_t)i * bpp)] =
                    *((const uint8_t *)data + j);
            }
        }

        (void)ILI9163C_4WSPI_transferSPIbuffer(spi,
                                              speedupArray,
                                              (size_t)len * bpp);
    }
    else
    {
        hd_hw_extent_t remaining = len;

        while (remaining != 0U)
        {
            hd_colors_t pixelsAvailable = colorCycleLength - startColorOffset;
            hd_hw_extent_t pixelsToDraw;

            value = ILI9163C_getOffsetColor(display,
                                            data,
                                            startColorOffset);

            pixelsToDraw =
                (pixelsAvailable >= remaining) ?
                remaining :
                (hd_hw_extent_t)pixelsAvailable;

            if (pixelsToDraw == 0U)
            {
                break;
            }

            if (ILI9163C_4WSPI_transferSPIbuffer(
                    spi,
                    (const uint8_t *)value,
                    (size_t)bpp * pixelsToDraw) != ILI9163C_STAT_Nominal)
            {
                break;
            }

            remaining = (hd_hw_extent_t)(remaining - pixelsToDraw);

            startColorOffset =
                hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                               (uint16_t)startColorOffset,
                                               (int32_t)pixelsToDraw);
        }
    }

    (void)ILI9163C_4WSPI_deselectDriver(spi);

restore_orientation:
    if (goLeft)
    {
        (void)ILI9163C_setMemoryAccessControl(ili,
                                              true,
                                              true,
                                              false,
                                              false,
                                              true,
                                              false);
    }
}

void ILI9163C_4WSPI_hwyline(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goUp)
{
    ILI9163C_4WSPI_t *spi = ILI9163C_4WSPI_fromHyperdisplay(display);
    ILI9163C_t *ili;
    color_t value;
    uint8_t bpp;
    hd_hw_extent_t y1;

    if ((spi == NULL) ||
        (data == NULL) ||
        (len < 1U) ||
        (colorCycleLength == 0U))
    {
        return;
    }

    ili = &spi->ili9163c;

    startColorOffset =
        hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                       (uint16_t)startColorOffset,
                                       0);

    value = ILI9163C_getOffsetColor(display, data, startColorOffset);
    bpp = ILI9163C_getBytesPerPixel(ili);

    if ((value == NULL) || (bpp == 0U))
    {
        return;
    }

    if (goUp)
    {
        (void)ILI9163C_setMemoryAccessControl(ili,
                                              true,
                                              false,
                                              false,
                                              false,
                                              true,
                                              false);
        y0 = (hd_hw_extent_t)((display->yExt - 1U) - y0);
    }

    y1 = (hd_hw_extent_t)(y0 + (len - 1U));

    if ((ILI9163C_setColumnAddress(ili, x0, x0) != ILI9163C_STAT_Nominal) ||
        (ILI9163C_setRowAddress(ili, y0, y1) != ILI9163C_STAT_Nominal))
    {
        goto restore_orientation;
    }

    {
        ILI9163C_CMD_t cmd = ILI9163C_CMD_WRRAM;

        if (ILI9163C_4WSPI_writePacket(ili, &cmd, NULL, 0U) !=
            ILI9163C_STAT_Nominal)
        {
            goto restore_orientation;
        }
    }

    if (ILI9163C_4WSPI_selectDriver(spi) != ILI9163C_STAT_Nominal)
    {
        goto restore_orientation;
    }

    GPIO_WritePinOutput(spi->dcGpio, spi->dcPin, 1U);

    if (colorCycleLength == 1U)
    {
        uint8_t speedupArray[ILI9163C_MAX_Y * ILI9163C_MAX_BPP];
        uint16_t i;
        uint8_t j;

        for (i = 0U; i < len; i++)
        {
            for (j = 0U; j < bpp; j++)
            {
                speedupArray[j + ((size_t)i * bpp)] =
                    *((const uint8_t *)data + j);
            }
        }

        (void)ILI9163C_4WSPI_transferSPIbuffer(spi,
                                              speedupArray,
                                              (size_t)len * bpp);
    }
    else
    {
        hd_hw_extent_t remaining = len;

        while (remaining != 0U)
        {
            hd_colors_t pixelsAvailable = colorCycleLength - startColorOffset;
            hd_hw_extent_t pixelsToDraw;

            value = ILI9163C_getOffsetColor(display,
                                            data,
                                            startColorOffset);

            pixelsToDraw =
                (pixelsAvailable >= remaining) ?
                remaining :
                (hd_hw_extent_t)pixelsAvailable;

            if (pixelsToDraw == 0U)
            {
                break;
            }

            if (ILI9163C_4WSPI_transferSPIbuffer(
                    spi,
                    (const uint8_t *)value,
                    (size_t)bpp * pixelsToDraw) != ILI9163C_STAT_Nominal)
            {
                break;
            }

            remaining = (hd_hw_extent_t)(remaining - pixelsToDraw);

            startColorOffset =
                hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                               (uint16_t)startColorOffset,
                                               (int32_t)pixelsToDraw);
        }
    }

    (void)ILI9163C_4WSPI_deselectDriver(spi);

restore_orientation:
    if (goUp)
    {
        (void)ILI9163C_setMemoryAccessControl(ili,
                                              true,
                                              true,
                                              false,
                                              false,
                                              true,
                                              false);
    }
}

void ILI9163C_4WSPI_hwfillFromArray(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t x1,
    hd_hw_extent_t y1,
    color_t data,
    hd_pixels_t numPixels,
    bool Vh)
{
    ILI9163C_4WSPI_t *spi = ILI9163C_4WSPI_fromHyperdisplay(display);
    ILI9163C_t *ili;
    uint8_t bpp;
    ILI9163C_CMD_t cmd = ILI9163C_CMD_WRRAM;

    if ((spi == NULL) || (numPixels == 0U) || (data == NULL))
    {
        return;
    }

    ili = &spi->ili9163c;
    bpp = ILI9163C_getBytesPerPixel(ili);

    if (bpp == 0U)
    {
        return;
    }

    if (Vh)
    {
        if (ILI9163C_setMemoryAccessControl(ili,
                                            true,
                                            true,
                                            true,
                                            false,
                                            true,
                                            false) != ILI9163C_STAT_Nominal)
        {
            return;
        }

        if ((ILI9163C_setColumnAddress(ili, y0, y1) != ILI9163C_STAT_Nominal) ||
            (ILI9163C_setRowAddress(ili, x0, x1) != ILI9163C_STAT_Nominal))
        {
            goto restore_orientation;
        }
    }
    else
    {
        if ((ILI9163C_setColumnAddress(ili, x0, x1) != ILI9163C_STAT_Nominal) ||
            (ILI9163C_setRowAddress(ili, y0, y1) != ILI9163C_STAT_Nominal))
        {
            return;
        }
    }

    if (ILI9163C_4WSPI_writePacket(ili, &cmd, NULL, 0U) !=
        ILI9163C_STAT_Nominal)
    {
        goto restore_orientation;
    }

    if (ILI9163C_4WSPI_selectDriver(spi) != ILI9163C_STAT_Nominal)
    {
        goto restore_orientation;
    }

    GPIO_WritePinOutput(spi->dcGpio, spi->dcPin, 1U);

    (void)ILI9163C_4WSPI_transferSPIbuffer(spi,
                                          (const uint8_t *)data,
                                          (size_t)bpp * numPixels);

    (void)ILI9163C_4WSPI_deselectDriver(spi);

restore_orientation:
    if (Vh)
    {
        (void)ILI9163C_setMemoryAccessControl(ili,
                                              true,
                                              true,
                                              false,
                                              false,
                                              true,
                                              false);
    }
}
