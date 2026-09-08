/*
 * hyperdisplay.c
 *
 * Pure-C port of SparkFun HyperDisplay for FRDM-KL25Z / MCUXpresso.
 *
 * This file is intentionally hardware-independent. The later
 * HyperDisplay_ILI9163C / KWH018ST01_4WSPI port will connect the generic
 * drawing API to the KL25Z SPI/GPIO hardware.
 *
 * Planned TFT wiring:
 *   PTD1  -> SPI0_SCK  -> TFT SCLK
 *   PTD2  -> SPI0_SOUT -> TFT MOSI
 *   PTA17 -> GPIO      -> TFT LCDCS
 *   PTA16 -> GPIO      -> TFT D/C
 */

#include "hyperdisplay.h"

#include <stdlib.h>
#include <string.h>

#if HYPERDISPLAY_USE_RAY_TRACING
#error "Ray tracing is not used by the KL25Z compass project. Set HYPERDISPLAY_USE_RAY_TRACING to 0."
#endif

wind_info_t hyperdisplayDefaultWindow;
char_info_t hyperdisplayDefaultCharacter;

#if HYPERDISPLAY_USE_PRINT && HYPERDISPLAY_INCLUDE_DEFAULT_FONT
static hd_font_extent_t hyperdisplayDefaultXloc[
    HYPERDISPLAY_DEFAULT_FONT_WIDTH * HYPERDISPLAY_DEFAULT_FONT_HEIGHT];

static hd_font_extent_t hyperdisplayDefaultYloc[
    HYPERDISPLAY_DEFAULT_FONT_WIDTH * HYPERDISPLAY_DEFAULT_FONT_HEIGHT];
#endif

/* -------------------------------------------------------------------------- */
/* Local helpers                                                              */
/* -------------------------------------------------------------------------- */

static void hd_swap(hd_extent_t *a, hd_extent_t *b)
{
    hd_extent_t t;

    if ((a == NULL) || (b == NULL))
        return;

    t = *a;
    *a = *b;
    *b = t;
}

static hd_extent_t hd_abs_len(hd_extent_t a, hd_extent_t b)
{
    return (a >= b) ? ((a - b) + 1.0) : ((b - a) + 1.0);
}

static hd_hw_extent_t hd_abs_len_hw(hd_hw_extent_t a, hd_hw_extent_t b)
{
    return (a >= b) ? (hd_hw_extent_t)(a - b + 1U)
                    : (hd_hw_extent_t)(b - a + 1U);
}

static color_t hd_offset_color(hyperdisplay_t *display,
                               color_t base,
                               uint32_t offset)
{
    if (base == NULL)
        return NULL;

    if ((display == NULL) || (display->ops.getOffsetColor == NULL))
        return base;

    return display->ops.getOffsetColor(display, base, offset);
}

static color_t hd_resolve_color(hyperdisplay_t *display,
                                color_t data,
                                hd_colors_t *cycle,
                                hd_colors_t *offset)
{
    if (data != NULL)
        return data;

    if ((display == NULL) || (display->pCurrentWindow == NULL))
        return NULL;

    *cycle = display->pCurrentWindow->currentColorCycleLength;
    *offset = display->pCurrentWindow->currentColorOffset;

    return display->pCurrentWindow->currentSequenceData;
}

static uint16_t hd_line_core(hyperdisplay_t *display,
                             hd_extent_t x0,
                             hd_extent_t y0,
                             hd_extent_t x1,
                             hd_extent_t y1,
                             uint16_t width,
                             color_t data,
                             hd_colors_t colorCycleLength,
                             hd_colors_t startColorOffset,
                             bool reverseGradient)
{
    int32_t x = (int32_t)x0;
    int32_t y = (int32_t)y0;
    int32_t tx = (int32_t)x1;
    int32_t ty = (int32_t)y1;

    int32_t dx = abs(tx - x);
    int32_t sx = (x < tx) ? 1 : -1;
    int32_t dy = -abs(ty - y);
    int32_t sy = (y < ty) ? 1 : -1;
    int32_t err = dx + dy;

    uint32_t count = 0U;

    if ((display == NULL) || (width == 0U))
        return 0U;

    data = hd_resolve_color(display,
                            data,
                            &colorCycleLength,
                            &startColorOffset);

    if ((data == NULL) || (colorCycleLength == 0U))
        return 0U;

    startColorOffset =
        hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                       (uint16_t)startColorOffset,
                                       0);

    while (1)
    {
        uint16_t half = width / 2U;
        int32_t wx;
        int32_t wy;
        color_t current =
            hd_offset_color(display, data, startColorOffset);

        for (wy = -(int32_t)half; wy <= (int32_t)half; wy++)
        {
            for (wx = -(int32_t)half; wx <= (int32_t)half; wx++)
            {
                hyperdisplay_pixel(display,
                                   (hd_extent_t)(x + wx),
                                   (hd_extent_t)(y + wy),
                                   current,
                                   1U,
                                   0U);
            }
        }

        count++;

        if ((x == tx) && (y == ty))
            break;

        if (reverseGradient)
        {
            startColorOffset =
                hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                               (uint16_t)startColorOffset,
                                               -1);
        }
        else
        {
            startColorOffset =
                hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                               (uint16_t)startColorOffset,
                                               1);
        }

        {
            int32_t e2 = 2 * err;

            if (e2 >= dy)
            {
                err += dy;
                x += sx;
            }

            if (e2 <= dx)
            {
                err += dx;
                y += sy;
            }
        }
    }

    return (count > UINT16_MAX) ? UINT16_MAX : (uint16_t)count;
}

/* -------------------------------------------------------------------------- */
/* Initialization                                                             */
/* -------------------------------------------------------------------------- */

void hyperdisplay_init(hyperdisplay_t *display,
                       uint16_t xSize,
                       uint16_t ySize)
{
    if (display == NULL)
        return;

    memset(display, 0, sizeof(*display));

    display->xExt = xSize;
    display->yExt = ySize;
    display->pCurrentWindow = &hyperdisplayDefaultWindow;

    hyperdisplay_setWindowDefaults(display,
                                   display->pCurrentWindow);
}

/* -------------------------------------------------------------------------- */
/* Display callback dispatch                                                  */
/* -------------------------------------------------------------------------- */

void hyperdisplay_hwpixel(hyperdisplay_t *display,
                          hd_hw_extent_t x0,
                          hd_hw_extent_t y0,
                          color_t data,
                          hd_colors_t colorCycleLength,
                          hd_colors_t startColorOffset)
{
    if ((display == NULL) || (display->ops.hwpixel == NULL))
        return;

    display->ops.hwpixel(display,
                         x0,
                         y0,
                         data,
                         colorCycleLength,
                         startColorOffset);
}

void hyperdisplay_swpixel(hyperdisplay_t *display,
                          hd_extent_t x0,
                          hd_extent_t y0,
                          color_t data,
                          hd_colors_t colorCycleLength,
                          hd_colors_t startColorOffset)
{
    if ((display == NULL) || (display->ops.swpixel == NULL))
        return;

    display->ops.swpixel(display,
                         x0,
                         y0,
                         data,
                         colorCycleLength,
                         startColorOffset);
}

void hyperdisplay_hwxline(hyperdisplay_t *display,
                          hd_hw_extent_t x0,
                          hd_hw_extent_t y0,
                          hd_hw_extent_t len,
                          color_t data,
                          hd_colors_t colorCycleLength,
                          hd_colors_t startColorOffset,
                          bool goLeft)
{
    hd_hw_extent_t i;

    if (display == NULL)
        return;

    if (display->ops.hwxline != NULL)
    {
        display->ops.hwxline(display,
                             x0,
                             y0,
                             len,
                             data,
                             colorCycleLength,
                             startColorOffset,
                             goLeft);
        return;
    }

    if ((data == NULL) || (colorCycleLength == 0U))
        return;

    startColorOffset =
        hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                       (uint16_t)startColorOffset,
                                       0);

    for (i = 0U; i < len; i++)
    {
        color_t value =
            hd_offset_color(display, data, startColorOffset);

        hyperdisplay_hwpixel(display,
                             goLeft ? (hd_hw_extent_t)(x0 - i)
                                    : (hd_hw_extent_t)(x0 + i),
                             y0,
                             value,
                             1U,
                             0U);

        startColorOffset =
            hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                           (uint16_t)startColorOffset,
                                           1);
    }

    hyperdisplayXLineCallback(x0,
                              y0,
                              len,
                              data,
                              colorCycleLength,
                              startColorOffset,
                              goLeft);
}

void hyperdisplay_hwyline(hyperdisplay_t *display,
                          hd_hw_extent_t x0,
                          hd_hw_extent_t y0,
                          hd_hw_extent_t len,
                          color_t data,
                          hd_colors_t colorCycleLength,
                          hd_colors_t startColorOffset,
                          bool goUp)
{
    hd_hw_extent_t i;

    if (display == NULL)
        return;

    if (display->ops.hwyline != NULL)
    {
        display->ops.hwyline(display,
                             x0,
                             y0,
                             len,
                             data,
                             colorCycleLength,
                             startColorOffset,
                             goUp);
        return;
    }

    if ((data == NULL) || (colorCycleLength == 0U))
        return;

    startColorOffset =
        hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                       (uint16_t)startColorOffset,
                                       0);

    for (i = 0U; i < len; i++)
    {
        color_t value =
            hd_offset_color(display, data, startColorOffset);

        hyperdisplay_hwpixel(display,
                             x0,
                             goUp ? (hd_hw_extent_t)(y0 - i)
                                  : (hd_hw_extent_t)(y0 + i),
                             value,
                             1U,
                             0U);

        startColorOffset =
            hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                           (uint16_t)startColorOffset,
                                           1);
    }

    hyperdisplayYLineCallback(x0,
                              y0,
                              len,
                              data,
                              colorCycleLength,
                              startColorOffset,
                              goUp);
}

void hyperdisplay_hwrectangle(hyperdisplay_t *display,
                              hd_hw_extent_t x0,
                              hd_hw_extent_t y0,
                              hd_hw_extent_t x1,
                              hd_hw_extent_t y1,
                              bool filled,
                              color_t data,
                              hd_colors_t colorCycleLength,
                              hd_colors_t startColorOffset,
                              bool reverseGradient,
                              bool gradientVertical)
{
    hd_hw_extent_t xlen;
    hd_hw_extent_t ylen;

    (void)gradientVertical;

    if (display == NULL)
        return;

    if (display->ops.hwrectangle != NULL)
    {
        display->ops.hwrectangle(display,
                                 x0,
                                 y0,
                                 x1,
                                 y1,
                                 filled,
                                 data,
                                 colorCycleLength,
                                 startColorOffset,
                                 reverseGradient,
                                 gradientVertical);
        return;
    }

    if ((data == NULL) || (colorCycleLength == 0U))
        return;

    xlen = (hd_hw_extent_t)(x1 - x0 + 1U);
    ylen = (hd_hw_extent_t)(y1 - y0 + 1U);

    if (filled)
    {
        int32_t y;
        int32_t start = reverseGradient ? (int32_t)y1 : (int32_t)y0;
        int32_t stop  = reverseGradient ? (int32_t)y0 : (int32_t)y1;
        int32_t step  = reverseGradient ? -1 : 1;

        for (y = start;
             reverseGradient ? (y >= stop) : (y <= stop);
             y += step)
        {
            hyperdisplay_hwxline(display,
                                 reverseGradient ? x1 : x0,
                                 (hd_hw_extent_t)y,
                                 xlen,
                                 data,
                                 colorCycleLength,
                                 startColorOffset,
                                 reverseGradient);
        }
    }
    else
    {
        hyperdisplay_hwxline(display,
                             x0,
                             y0,
                             xlen,
                             data,
                             colorCycleLength,
                             startColorOffset,
                             false);

        if (y1 != y0)
        {
            hyperdisplay_hwxline(display,
                                 x0,
                                 y1,
                                 xlen,
                                 data,
                                 colorCycleLength,
                                 startColorOffset,
                                 false);
        }

        if (ylen > 2U)
        {
            hyperdisplay_hwyline(display,
                                 x0,
                                 (hd_hw_extent_t)(y0 + 1U),
                                 (hd_hw_extent_t)(ylen - 2U),
                                 data,
                                 colorCycleLength,
                                 startColorOffset,
                                 false);

            if (x1 != x0)
            {
                hyperdisplay_hwyline(display,
                                     x1,
                                     (hd_hw_extent_t)(y0 + 1U),
                                     (hd_hw_extent_t)(ylen - 2U),
                                     data,
                                     colorCycleLength,
                                     startColorOffset,
                                     false);
            }
        }
    }

    hyperdisplayRectangleCallback(x0,
                                  y0,
                                  x1,
                                  y1,
                                  data,
                                  filled,
                                  colorCycleLength,
                                  startColorOffset,
                                  gradientVertical,
                                  reverseGradient);
}

void hyperdisplay_hwfillFromArray(hyperdisplay_t *display,
                                  hd_hw_extent_t x0,
                                  hd_hw_extent_t y0,
                                  hd_hw_extent_t x1,
                                  hd_hw_extent_t y1,
                                  color_t data,
                                  hd_pixels_t numPixels,
                                  bool Vh)
{
    hd_pixels_t offset = 0U;

    if (display == NULL)
        return;

    if (display->ops.hwfillFromArray != NULL)
    {
        display->ops.hwfillFromArray(display,
                                     x0,
                                     y0,
                                     x1,
                                     y1,
                                     data,
                                     numPixels,
                                     Vh);
        return;
    }

    if ((data == NULL) || (numPixels == 0U))
        return;

    if (Vh)
    {
        uint32_t x;

        for (x = x0; x <= x1; x++)
        {
            uint32_t y;

            for (y = y0; y <= y1; y++)
            {
                hyperdisplay_hwpixel(
                    display,
                    (hd_hw_extent_t)x,
                    (hd_hw_extent_t)y,
                    hd_offset_color(display, data, offset),
                    1U,
                    0U);

                offset = (offset + 1U) % numPixels;
            }
        }
    }
    else
    {
        uint32_t y;

        for (y = y0; y <= y1; y++)
        {
            uint32_t x;

            for (x = x0; x <= x1; x++)
            {
                hyperdisplay_hwpixel(
                    display,
                    (hd_hw_extent_t)x,
                    (hd_hw_extent_t)y,
                    hd_offset_color(display, data, offset),
                    1U,
                    0U);

                offset = (offset + 1U) % numPixels;
            }
        }
    }

    hyperdisplayFillFromArrayCallback(x0,
                                      y0,
                                      x1,
                                      y1,
                                      numPixels,
                                      data);
}

/* -------------------------------------------------------------------------- */
/* Software-buffer primitives                                                 */
/* -------------------------------------------------------------------------- */

hd_pixels_t hyperdisplay_wToPix(wind_info_t *wind,
                                hd_hw_extent_t x0,
                                hd_hw_extent_t y0)
{
    if (wind == NULL)
        return 0U;

    return (hd_pixels_t)x0 +
           ((hd_pixels_t)y0 *
            (hd_pixels_t)hd_abs_len_hw(wind->xMax, wind->xMin));
}

void hyperdisplay_swxline(hyperdisplay_t *display,
                          hd_extent_t x0,
                          hd_extent_t y0,
                          hd_extent_t len,
                          color_t data,
                          hd_colors_t colorCycleLength,
                          hd_colors_t startColorOffset,
                          bool goLeft)
{
    uint32_t i;

    if ((display == NULL) ||
        (data == NULL) ||
        (colorCycleLength == 0U) ||
        (len <= 0.0))
        return;

    for (i = 0U; i < (uint32_t)len; i++)
    {
        color_t value =
            hd_offset_color(display, data, startColorOffset);

        hyperdisplay_swpixel(display,
                             goLeft ? (x0 - i) : (x0 + i),
                             y0,
                             value,
                             1U,
                             0U);

        startColorOffset =
            hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                           (uint16_t)startColorOffset,
                                           1);
    }
}

void hyperdisplay_swyline(hyperdisplay_t *display,
                          hd_extent_t x0,
                          hd_extent_t y0,
                          hd_extent_t len,
                          color_t data,
                          hd_colors_t colorCycleLength,
                          hd_colors_t startColorOffset,
                          bool goUp)
{
    uint32_t i;

    if ((display == NULL) ||
        (data == NULL) ||
        (colorCycleLength == 0U) ||
        (len <= 0.0))
        return;

    for (i = 0U; i < (uint32_t)len; i++)
    {
        color_t value =
            hd_offset_color(display, data, startColorOffset);

        hyperdisplay_swpixel(display,
                             x0,
                             goUp ? (y0 - i) : (y0 + i),
                             value,
                             1U,
                             0U);

        startColorOffset =
            hyperdisplay_getNewColorOffset((uint16_t)colorCycleLength,
                                           (uint16_t)startColorOffset,
                                           1);
    }
}

void hyperdisplay_swrectangle(hyperdisplay_t *display,
                              hd_extent_t x0,
                              hd_extent_t y0,
                              hd_extent_t x1,
                              hd_extent_t y1,
                              bool filled,
                              color_t data,
                              hd_colors_t colorCycleLength,
                              hd_colors_t startColorOffset,
                              bool reverseGradient,
                              bool gradientVertical)
{
    hd_extent_t y;

    (void)gradientVertical;

    if ((display == NULL) ||
        (data == NULL) ||
        (colorCycleLength == 0U))
        return;

    if (x0 > x1)
        hd_swap(&x0, &x1);

    if (y0 > y1)
        hd_swap(&y0, &y1);

    if (filled)
    {
        for (y = y0; y <= y1; y += 1.0)
        {
            hyperdisplay_swxline(display,
                                 reverseGradient ? x1 : x0,
                                 y,
                                 hd_abs_len(x0, x1),
                                 data,
                                 colorCycleLength,
                                 startColorOffset,
                                 reverseGradient);
        }
    }
    else
    {
        hyperdisplay_swxline(display,
                             x0,
                             y0,
                             hd_abs_len(x0, x1),
                             data,
                             colorCycleLength,
                             startColorOffset,
                             false);

        if (y1 != y0)
        {
            hyperdisplay_swxline(display,
                                 x0,
                                 y1,
                                 hd_abs_len(x0, x1),
                                 data,
                                 colorCycleLength,
                                 startColorOffset,
                                 false);
        }

        if ((y1 - y0) > 1.0)
        {
            hyperdisplay_swyline(display,
                                 x0,
                                 y0 + 1.0,
                                 y1 - y0 - 1.0,
                                 data,
                                 colorCycleLength,
                                 startColorOffset,
                                 false);

            if (x1 != x0)
            {
                hyperdisplay_swyline(display,
                                     x1,
                                     y0 + 1.0,
                                     y1 - y0 - 1.0,
                                     data,
                                     colorCycleLength,
                                     startColorOffset,
                                     false);
            }
        }
    }
}

void hyperdisplay_swfillFromArray(hyperdisplay_t *display,
                                  hd_extent_t x0,
                                  hd_extent_t y0,
                                  hd_extent_t x1,
                                  hd_extent_t y1,
                                  color_t data,
                                  hd_pixels_t numPixels,
                                  bool Vh)
{
    hd_pixels_t offset = 0U;

    if ((display == NULL) ||
        (data == NULL) ||
        (numPixels == 0U))
        return;

    if (Vh)
    {
        int32_t x;

        for (x = (int32_t)x0; x <= (int32_t)x1; x++)
        {
            int32_t y;

            for (y = (int32_t)y0; y <= (int32_t)y1; y++)
            {
                hyperdisplay_swpixel(
                    display,
                    (hd_extent_t)x,
                    (hd_extent_t)y,
                    hd_offset_color(display, data, offset),
                    1U,
                    0U);

                offset = (offset + 1U) % numPixels;
            }
        }
    }
    else
    {
        int32_t y;

        for (y = (int32_t)y0; y <= (int32_t)y1; y++)
        {
            int32_t x;

            for (x = (int32_t)x0; x <= (int32_t)x1; x++)
            {
                hyperdisplay_swpixel(
                    display,
                    (hd_extent_t)x,
                    (hd_extent_t)y,
                    hd_offset_color(display, data, offset),
                    1U,
                    0U);

                offset = (offset + 1U) % numPixels;
            }
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Window-coordinate drawing API                                              */
/* -------------------------------------------------------------------------- */

void hyperdisplay_pixel(hyperdisplay_t *display,
                        hd_extent_t x0,
                        hd_extent_t y0,
                        color_t data,
                        hd_colors_t colorCycleLength,
                        hd_colors_t startColorOffset)
{
    wind_info_t *wind;

    if ((display == NULL) || (display->pCurrentWindow == NULL))
        return;

    wind = display->pCurrentWindow;

    data = hd_resolve_color(display,
                            data,
                            &colorCycleLength,
                            &startColorOffset);

    if ((data == NULL) || (colorCycleLength == 0U))
        return;

    if (wind->bufferMode)
    {
        if ((wind->data == NULL) ||
            (hyperdisplay_enforceSWLimits(display, &x0, hdX) !=
             hyperdisplay_dim_ok) ||
            (hyperdisplay_enforceSWLimits(display, &y0, hdY) !=
             hyperdisplay_dim_ok))
            return;

        if (hyperdisplay_wToPix(wind,
                                (hd_hw_extent_t)x0,
                                (hd_hw_extent_t)y0) >= wind->numPixels)
            return;

        hyperdisplay_swpixel(display,
                             x0,
                             y0,
                             data,
                             colorCycleLength,
                             startColorOffset);
    }
    else
    {
        hd_hw_extent_t xhw;
        hd_hw_extent_t yhw;

        if ((hyperdisplay_enforceHWLimits(display,
                                          &x0,
                                          &xhw,
                                          hdX) !=
             hyperdisplay_dim_ok) ||
            (hyperdisplay_enforceHWLimits(display,
                                          &y0,
                                          &yhw,
                                          hdY) !=
             hyperdisplay_dim_ok))
            return;

        hyperdisplay_hwpixel(display,
                             xhw,
                             yhw,
                             data,
                             colorCycleLength,
                             startColorOffset);
    }
}

void hyperdisplay_xline(hyperdisplay_t *display,
                        hd_extent_t x0,
                        hd_extent_t y0,
                        hd_extent_t len,
                        color_t data,
                        hd_colors_t colorCycleLength,
                        hd_colors_t startColorOffset,
                        bool goLeft)
{
    hd_extent_t x1;

    if ((display == NULL) || (len <= 0.0))
        return;

    x1 = goLeft ? (x0 - len + 1.0)
                : (x0 + len - 1.0);

    (void)hyperdisplay_line(display,
                            x0,
                            y0,
                            x1,
                            y0,
                            1U,
                            data,
                            colorCycleLength,
                            startColorOffset,
                            false);
}

void hyperdisplay_yline(hyperdisplay_t *display,
                        hd_extent_t x0,
                        hd_extent_t y0,
                        hd_extent_t len,
                        color_t data,
                        hd_colors_t colorCycleLength,
                        hd_colors_t startColorOffset,
                        bool goUp)
{
    hd_extent_t y1;

    if ((display == NULL) || (len <= 0.0))
        return;

    y1 = goUp ? (y0 - len + 1.0)
              : (y0 + len - 1.0);

    (void)hyperdisplay_line(display,
                            x0,
                            y0,
                            x0,
                            y1,
                            1U,
                            data,
                            colorCycleLength,
                            startColorOffset,
                            false);
}

void hyperdisplay_rectangle(hyperdisplay_t *display,
                            hd_extent_t x0,
                            hd_extent_t y0,
                            hd_extent_t x1,
                            hd_extent_t y1,
                            bool filled,
                            color_t data,
                            hd_colors_t colorCycleLength,
                            hd_colors_t startColorOffset,
                            bool reverseGradient,
                            bool gradientVertical)
{
    wind_info_t *wind;

    if ((display == NULL) || (display->pCurrentWindow == NULL))
        return;

    wind = display->pCurrentWindow;

    if (x0 > x1)
        hd_swap(&x0, &x1);

    if (y0 > y1)
        hd_swap(&y0, &y1);

    data = hd_resolve_color(display,
                            data,
                            &colorCycleLength,
                            &startColorOffset);

    if ((data == NULL) || (colorCycleLength == 0U))
        return;

    if (wind->bufferMode)
    {
        hyperdisplay_swrectangle(display,
                                 x0,
                                 y0,
                                 x1,
                                 y1,
                                 filled,
                                 data,
                                 colorCycleLength,
                                 startColorOffset,
                                 reverseGradient,
                                 gradientVertical);
    }
    else
    {
        hd_hw_extent_t x0hw;
        hd_hw_extent_t y0hw;
        hd_hw_extent_t x1hw;
        hd_hw_extent_t y1hw;

        hyperdisplay_dim_check_t cx0 =
            hyperdisplay_enforceHWLimits(display, &x0, &x0hw, hdX);
        hyperdisplay_dim_check_t cy0 =
            hyperdisplay_enforceHWLimits(display, &y0, &y0hw, hdY);
        hyperdisplay_dim_check_t cx1 =
            hyperdisplay_enforceHWLimits(display, &x1, &x1hw, hdX);
        hyperdisplay_dim_check_t cy1 =
            hyperdisplay_enforceHWLimits(display, &y1, &y1hw, hdY);

        if (((cx0 == hyperdisplay_dim_low) &&
             (cx1 == hyperdisplay_dim_low)) ||
            ((cx0 == hyperdisplay_dim_high) &&
             (cx1 == hyperdisplay_dim_high)) ||
            ((cy0 == hyperdisplay_dim_low) &&
             (cy1 == hyperdisplay_dim_low)) ||
            ((cy0 == hyperdisplay_dim_high) &&
             (cy1 == hyperdisplay_dim_high)))
            return;

        hyperdisplay_hwrectangle(display,
                                 x0hw,
                                 y0hw,
                                 x1hw,
                                 y1hw,
                                 filled,
                                 data,
                                 colorCycleLength,
                                 startColorOffset,
                                 reverseGradient,
                                 gradientVertical);
    }
}

void hyperdisplay_fillFromArray(hyperdisplay_t *display,
                                hd_extent_t x0,
                                hd_extent_t y0,
                                hd_extent_t x1,
                                hd_extent_t y1,
                                color_t data,
                                hd_pixels_t numPixels,
                                bool Vh)
{
    wind_info_t *wind;

    if ((display == NULL) ||
        (display->pCurrentWindow == NULL) ||
        (data == NULL) ||
        (numPixels == 0U))
        return;

    wind = display->pCurrentWindow;

    if (x0 > x1)
        hd_swap(&x0, &x1);

    if (y0 > y1)
        hd_swap(&y0, &y1);

    if (wind->bufferMode)
    {
        hyperdisplay_swfillFromArray(display,
                                     x0,
                                     y0,
                                     x1,
                                     y1,
                                     data,
                                     numPixels,
                                     Vh);
    }
    else
    {
        hd_hw_extent_t x0hw;
        hd_hw_extent_t y0hw;
        hd_hw_extent_t x1hw;
        hd_hw_extent_t y1hw;

        if ((hyperdisplay_enforceHWLimits(display,
                                          &x0,
                                          &x0hw,
                                          hdX) ==
             hyperdisplay_dim_no_val) ||
            (hyperdisplay_enforceHWLimits(display,
                                          &y0,
                                          &y0hw,
                                          hdY) ==
             hyperdisplay_dim_no_val) ||
            (hyperdisplay_enforceHWLimits(display,
                                          &x1,
                                          &x1hw,
                                          hdX) ==
             hyperdisplay_dim_no_val) ||
            (hyperdisplay_enforceHWLimits(display,
                                          &y1,
                                          &y1hw,
                                          hdY) ==
             hyperdisplay_dim_no_val))
            return;

        hyperdisplay_hwfillFromArray(display,
                                     x0hw,
                                     y0hw,
                                     x1hw,
                                     y1hw,
                                     data,
                                     numPixels,
                                     Vh);
    }
}

void hyperdisplay_fillWindow(hyperdisplay_t *display,
                             color_t color,
                             hd_colors_t colorCycleLength,
                             hd_colors_t startColorOffset)
{
    wind_info_t *wind;

    if ((display == NULL) || (display->pCurrentWindow == NULL))
        return;

    wind = display->pCurrentWindow;

    hyperdisplay_rectangle(display,
                           0.0,
                           0.0,
                           (hd_extent_t)(wind->xMax - wind->xMin),
                           (hd_extent_t)(wind->yMax - wind->yMin),
                           true,
                           color,
                           colorCycleLength,
                           startColorOffset,
                           false,
                           false);
}

/* -------------------------------------------------------------------------- */
/* Window / buffer configuration                                              */
/* -------------------------------------------------------------------------- */

void hyperdisplay_setWindowColorSequence(hyperdisplay_t *display,
                                         wind_info_t *wind,
                                         color_t data,
                                         hd_colors_t colorCycleLength,
                                         hd_colors_t startColorOffset)
{
    (void)display;

    if (wind == NULL)
        return;

    wind->currentSequenceData = data;
    wind->currentColorCycleLength = colorCycleLength;
    wind->currentColorOffset = startColorOffset;
}

void hyperdisplay_setCurrentWindowColorSequence(
    hyperdisplay_t *display,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset)
{
    if ((display == NULL) || (display->pCurrentWindow == NULL))
        return;

    hyperdisplay_setWindowColorSequence(display,
                                        display->pCurrentWindow,
                                        data,
                                        colorCycleLength,
                                        startColorOffset);
}

int hyperdisplay_setWindowMemory(hyperdisplay_t *display,
                                 wind_info_t *wind,
                                 color_t data,
                                 hd_pixels_t numPixels,
                                 uint8_t bpp,
                                 bool allowDynamic)
{
    (void)display;

    if (wind == NULL)
        return -1;

    if (wind->dynamic && (wind->data != NULL))
    {
        free(wind->data);
        wind->data = NULL;
        wind->dynamic = false;
        wind->numPixels = 0U;
    }

    if (data == NULL)
    {
        if ((!allowDynamic) || (numPixels == 0U) || (bpp == 0U))
            return -1;

        wind->data = malloc((size_t)numPixels * (size_t)bpp);

        if (wind->data == NULL)
            return -1;

        wind->dynamic = true;
    }
    else
    {
        wind->data = data;
        wind->dynamic = false;
    }

    wind->numPixels = numPixels;

    return 0;
}

int hyperdisplay_setCurrentWindowMemory(hyperdisplay_t *display,
                                        color_t data,
                                        hd_pixels_t numPixels,
                                        uint8_t bpp,
                                        bool allowDynamic)
{
    if ((display == NULL) || (display->pCurrentWindow == NULL))
        return -1;

    return hyperdisplay_setWindowMemory(display,
                                        display->pCurrentWindow,
                                        data,
                                        numPixels,
                                        bpp,
                                        allowDynamic);
}

void hyperdisplay_buffer(hyperdisplay_t *display,
                         wind_info_t *wind)
{
    if (display == NULL)
        return;

    if (wind == NULL)
        wind = display->pCurrentWindow;

    if (wind != NULL)
        wind->bufferMode = true;
}

void hyperdisplay_direct(hyperdisplay_t *display,
                         wind_info_t *wind)
{
    if (display == NULL)
        return;

    if (wind == NULL)
        wind = display->pCurrentWindow;

    if (wind != NULL)
        wind->bufferMode = false;
}

void hyperdisplay_show(hyperdisplay_t *display,
                       wind_info_t *wind)
{
    wind_info_t *previous;
    bool oldMode;

    if (display == NULL)
        return;

    if (wind == NULL)
        wind = display->pCurrentWindow;

    if ((wind == NULL) ||
        (wind->data == NULL) ||
        (wind->numPixels == 0U))
        return;

    previous = display->pCurrentWindow;
    oldMode = wind->bufferMode;

    display->pCurrentWindow = wind;
    wind->bufferMode = false;

    hyperdisplay_fillFromArray(display,
                               0.0,
                               0.0,
                               (hd_extent_t)(wind->xMax - wind->xMin),
                               (hd_extent_t)(wind->yMax - wind->yMin),
                               wind->data,
                               wind->numPixels,
                               false);

    wind->bufferMode = oldMode;
    display->pCurrentWindow = previous;
}

/* -------------------------------------------------------------------------- */
/* Text                                                                       */
/* -------------------------------------------------------------------------- */

#if HYPERDISPLAY_USE_PRINT

void hyperdisplay_getCharInfo(hyperdisplay_t *display,
                              uint8_t character,
                              char_info_t *info)
{
    if (info == NULL)
        return;

    if ((display != NULL) && (display->ops.getCharInfo != NULL))
    {
        display->ops.getCharInfo(display, character, info);
        return;
    }

#if HYPERDISPLAY_INCLUDE_DEFAULT_FONT
    {
        uint16_t offset;
        uint16_t n = 0U;
        uint8_t x;

        info->data = NULL;
        info->xLoc = hyperdisplayDefaultXloc;
        info->yLoc = hyperdisplayDefaultYloc;
        info->xDim = HYPERDISPLAY_DEFAULT_FONT_WIDTH;
        info->yDim = HYPERDISPLAY_DEFAULT_FONT_HEIGHT;
        info->numPixels = 0U;

        info->causesNewline =
            ((character == (uint8_t)'\r') ||
             (character == (uint8_t)'\n'));

        if ((character < (uint8_t)' ') ||
            (character > (uint8_t)'~'))
        {
            info->show = false;
            return;
        }

        info->show = true;

        /*
         * KL25Z flash is directly memory mapped.
         * No AVR pgm_read_byte() is required.
         */
        offset = (uint16_t)(6U + 5U * (uint16_t)character);

        for (x = 0U; x < HYPERDISPLAY_DEFAULT_FONT_WIDTH; x++)
        {
            uint8_t y;
            uint8_t value = font5x7[offset + x];

            for (y = 0U; y < HYPERDISPLAY_DEFAULT_FONT_HEIGHT; y++)
            {
                if ((value & (uint8_t)(1U << y)) != 0U)
                {
                    if (n <
                        (HYPERDISPLAY_DEFAULT_FONT_WIDTH *
                         HYPERDISPLAY_DEFAULT_FONT_HEIGHT))
                    {
                        info->xLoc[n] = x;
                        info->yLoc[n] = y;
                        n++;
                    }
                }
            }
        }

        info->numPixels = n;
    }
#else
    (void)display;
    (void)character;
    info->show = false;
    info->causesNewline = false;
    info->numPixels = 0U;
#endif
}

size_t hyperdisplay_write(hyperdisplay_t *display,
                          uint8_t val)
{
    wind_info_t *wind;
    uint32_t i;

    if ((display == NULL) || (display->pCurrentWindow == NULL))
        return 0U;

    wind = display->pCurrentWindow;

    hyperdisplay_getCharInfo(display,
                             val,
                             &hyperdisplayDefaultCharacter);

    if (hyperdisplayDefaultCharacter.causesNewline)
    {
        wind->cursorX = wind->xReset;
        wind->cursorY += hyperdisplayDefaultCharacter.yDim;
        return 1U;
    }

    if (((hd_extent_t)(wind->xMax - wind->xMin) -
         hyperdisplayDefaultCharacter.xDim) < wind->cursorX)
    {
        if (((hd_extent_t)(wind->yMax - wind->yMin) -
             hyperdisplayDefaultCharacter.yDim) < wind->cursorY)
            return 0U;

        wind->cursorX = wind->xReset;
        wind->cursorY += hyperdisplayDefaultCharacter.yDim;
    }

    if (!hyperdisplayDefaultCharacter.show)
        return 0U;

    for (i = 0U;
         i < hyperdisplayDefaultCharacter.numPixels;
         i++)
    {
        hyperdisplay_pixel(
            display,
            wind->cursorX + hyperdisplayDefaultCharacter.xLoc[i],
            wind->cursorY + hyperdisplayDefaultCharacter.yLoc[i],
            NULL,
            1U,
            0U);
    }

    wind->cursorX += hyperdisplayDefaultCharacter.xDim + 1U;
    wind->lastCharacter = hyperdisplayDefaultCharacter;

    return 1U;
}

#else

size_t hyperdisplay_write(hyperdisplay_t *display,
                          uint8_t val)
{
    (void)display;
    (void)val;
    return 0U;
}

#endif

void hyperdisplay_setTextCursor(hyperdisplay_t *display,
                                int32_t x0,
                                int32_t y0,
                                wind_info_t *window)
{
    if (display == NULL)
        return;

    if (window == NULL)
        window = display->pCurrentWindow;

    if (window == NULL)
        return;

    window->cursorX = x0;
    window->cursorY = y0;
}

void hyperdisplay_resetTextCursor(hyperdisplay_t *display,
                                  wind_info_t *window)
{
    if (display == NULL)
        return;

    if (window == NULL)
        window = display->pCurrentWindow;

    if (window == NULL)
        return;

    hyperdisplay_setTextCursor(display,
                               (int32_t)window->xReset,
                               (int32_t)window->yReset,
                               window);
}

/* -------------------------------------------------------------------------- */
/* Level 1 drawing                                                            */
/* -------------------------------------------------------------------------- */

#if HYPERDISPLAY_DRAWING_LEVEL > 0

uint16_t hyperdisplay_line(hyperdisplay_t *display,
                           hd_extent_t x0,
                           hd_extent_t y0,
                           hd_extent_t x1,
                           hd_extent_t y1,
                           uint16_t width,
                           color_t data,
                           hd_colors_t colorCycleLength,
                           hd_colors_t startColorOffset,
                           bool reverseGradient)
{
    return hd_line_core(display,
                        x0,
                        y0,
                        x1,
                        y1,
                        width,
                        data,
                        colorCycleLength,
                        startColorOffset,
                        reverseGradient);
}

void hyperdisplay_polygon(hyperdisplay_t *display,
                          hd_extent_t x[],
                          hd_extent_t y[],
                          uint8_t numSides,
                          uint16_t width,
                          color_t data,
                          hd_colors_t colorCycleLength,
                          hd_colors_t startColorOffset,
                          bool reverseGradient)
{
    uint8_t i;

    if ((display == NULL) ||
        (x == NULL) ||
        (y == NULL) ||
        (numSides < 2U))
        return;

    for (i = 0U; i < numSides; i++)
    {
        uint8_t next =
            (i == (uint8_t)(numSides - 1U)) ? 0U
                                            : (uint8_t)(i + 1U);

        (void)hyperdisplay_line(display,
                                x[i],
                                y[i],
                                x[next],
                                y[next],
                                width,
                                data,
                                colorCycleLength,
                                startColorOffset,
                                reverseGradient);
    }
}

void hyperdisplay_circle(hyperdisplay_t *display,
                         hd_extent_t x0,
                         hd_extent_t y0,
                         hd_extent_t radius,
                         bool filled,
                         color_t data,
                         hd_colors_t colorCycleLength,
                         hd_colors_t startColorOffset,
                         bool reverseGradient)
{
    color_t color;

    (void)reverseGradient;

    if ((display == NULL) || (radius < 0.0))
        return;

    data = hd_resolve_color(display,
                            data,
                            &colorCycleLength,
                            &startColorOffset);

    if ((data == NULL) || (colorCycleLength == 0U))
        return;

    color = hd_offset_color(display,
                            data,
                            startColorOffset);

    hyperdisplay_circle_midpoint(display,
                                 x0,
                                 y0,
                                 radius,
                                 color,
                                 filled);
}

/*
 * These four functions remain available because the original HyperDisplay API
 * exposed them internally through the C++ class.
 *
 * In the C port they share the same generic Bresenham implementation.
 */

hd_extent_t hyperdisplay_lineHighNorm(hyperdisplay_t *display,
                                      hd_extent_t x0,
                                      hd_extent_t y0,
                                      hd_extent_t x1,
                                      hd_extent_t y1,
                                      uint16_t width,
                                      color_t data,
                                      hd_colors_t colorCycleLength,
                                      hd_colors_t startColorOffset)
{
    return hd_line_core(display,
                        x0, y0, x1, y1,
                        width,
                        data,
                        colorCycleLength,
                        startColorOffset,
                        false);
}

hd_extent_t hyperdisplay_lineLowNorm(hyperdisplay_t *display,
                                     hd_extent_t x0,
                                     hd_extent_t y0,
                                     hd_extent_t x1,
                                     hd_extent_t y1,
                                     uint16_t width,
                                     color_t data,
                                     hd_colors_t colorCycleLength,
                                     hd_colors_t startColorOffset)
{
    return hd_line_core(display,
                        x0, y0, x1, y1,
                        width,
                        data,
                        colorCycleLength,
                        startColorOffset,
                        false);
}

hd_extent_t hyperdisplay_lineHighReverse(hyperdisplay_t *display,
                                         hd_extent_t x0,
                                         hd_extent_t y0,
                                         hd_extent_t x1,
                                         hd_extent_t y1,
                                         uint16_t width,
                                         color_t data,
                                         hd_colors_t colorCycleLength,
                                         hd_colors_t startColorOffset)
{
    return hd_line_core(display,
                        x0, y0, x1, y1,
                        width,
                        data,
                        colorCycleLength,
                        startColorOffset,
                        true);
}

hd_extent_t hyperdisplay_lineLowReverse(hyperdisplay_t *display,
                                        hd_extent_t x0,
                                        hd_extent_t y0,
                                        hd_extent_t x1,
                                        hd_extent_t y1,
                                        uint16_t width,
                                        color_t data,
                                        hd_colors_t colorCycleLength,
                                        hd_colors_t startColorOffset)
{
    return hd_line_core(display,
                        x0, y0, x1, y1,
                        width,
                        data,
                        colorCycleLength,
                        startColorOffset,
                        true);
}

void hyperdisplay_circle_Bresenham(hyperdisplay_t *display,
                                   hd_extent_t x0,
                                   hd_extent_t y0,
                                   hd_extent_t radius,
                                   color_t color,
                                   bool fill)
{
    hyperdisplay_circle_midpoint(display,
                                 x0,
                                 y0,
                                 radius,
                                 color,
                                 fill);
}

void hyperdisplay_circle_midpoint(hyperdisplay_t *display,
                                  hd_extent_t x0,
                                  hd_extent_t y0,
                                  hd_extent_t radius,
                                  color_t color,
                                  bool fill)
{
    int32_t x;
    int32_t y = 0;
    int32_t decision;

    if (display == NULL)
        return;

    x = (int32_t)radius;
    decision = 1 - x;

    while (x >= y)
    {
        hyperdisplay_circle_eight(display,
                                  x0,
                                  y0,
                                  x,
                                  y,
                                  color,
                                  fill);

        y++;

        if (decision <= 0)
        {
            decision += (2 * y) + 1;
        }
        else
        {
            x--;
            decision += (2 * (y - x)) + 1;
        }
    }
}

void hyperdisplay_circle_eight(hyperdisplay_t *display,
                               hd_extent_t x0,
                               hd_extent_t y0,
                               hd_extent_t dx,
                               hd_extent_t dy,
                               color_t color,
                               bool fill)
{
    if (display == NULL)
        return;

    if (fill)
    {
        (void)hyperdisplay_line(display,
                                x0 - dx,
                                y0 + dy,
                                x0 + dx,
                                y0 + dy,
                                1U,
                                color,
                                1U,
                                0U,
                                false);

        (void)hyperdisplay_line(display,
                                x0 - dx,
                                y0 - dy,
                                x0 + dx,
                                y0 - dy,
                                1U,
                                color,
                                1U,
                                0U,
                                false);

        (void)hyperdisplay_line(display,
                                x0 - dy,
                                y0 + dx,
                                x0 + dy,
                                y0 + dx,
                                1U,
                                color,
                                1U,
                                0U,
                                false);

        (void)hyperdisplay_line(display,
                                x0 - dy,
                                y0 - dx,
                                x0 + dy,
                                y0 - dx,
                                1U,
                                color,
                                1U,
                                0U,
                                false);
    }
    else
    {
        hyperdisplay_pixel(display, x0 + dx, y0 + dy, color, 1U, 0U);
        hyperdisplay_pixel(display, x0 - dx, y0 + dy, color, 1U, 0U);
        hyperdisplay_pixel(display, x0 + dx, y0 - dy, color, 1U, 0U);
        hyperdisplay_pixel(display, x0 - dx, y0 - dy, color, 1U, 0U);

        hyperdisplay_pixel(display, x0 + dy, y0 + dx, color, 1U, 0U);
        hyperdisplay_pixel(display, x0 - dy, y0 + dx, color, 1U, 0U);
        hyperdisplay_pixel(display, x0 + dy, y0 - dx, color, 1U, 0U);
        hyperdisplay_pixel(display, x0 - dy, y0 - dx, color, 1U, 0U);
    }
}

#endif /* HYPERDISPLAY_DRAWING_LEVEL > 0 */

/* -------------------------------------------------------------------------- */
/* Utility                                                                    */
/* -------------------------------------------------------------------------- */

uint16_t hyperdisplay_getNewColorOffset(uint16_t colorCycleLength,
                                        uint16_t startColorOffset,
                                        int32_t numWritten)
{
    if (colorCycleLength == 0U)
        return 0U;

    startColorOffset %= colorCycleLength;

    if (numWritten >= 0)
    {
        return (uint16_t)(((uint32_t)startColorOffset +
                           (uint32_t)numWritten) %
                          colorCycleLength);
    }
    else
    {
        uint32_t amount = (uint32_t)(-(int64_t)numWritten);
        uint16_t remainder =
            (uint16_t)(amount % colorCycleLength);

        if (startColorOffset >= remainder)
            return (uint16_t)(startColorOffset - remainder);

        return (uint16_t)(colorCycleLength -
                          (remainder - startColorOffset));
    }
}

hyperdisplay_dim_check_t hyperdisplay_enforceHWLimits(
    hyperdisplay_t *display,
    hd_extent_t *windowvar,
    hd_hw_extent_t *hardwarevar,
    bool axisSelect)
{
    wind_info_t *wind;
    hd_extent_t translated;
    hd_hw_extent_t minValue;
    hd_hw_extent_t maxValue;
    bool low = false;
    bool high = false;

    if ((display == NULL) ||
        (display->pCurrentWindow == NULL) ||
        (windowvar == NULL) ||
        (hardwarevar == NULL))
        return hyperdisplay_dim_no_val;

    wind = display->pCurrentWindow;

    if (axisSelect == hdY)
    {
        if (display->yExt == 0U)
            return hyperdisplay_dim_no_val;

        minValue = wind->yMin;
        maxValue = wind->yMax;
    }
    else
    {
        if (display->xExt == 0U)
            return hyperdisplay_dim_no_val;

        minValue = wind->xMin;
        maxValue = wind->xMax;
    }

    translated =
        *windowvar + (hd_extent_t)minValue;

    if (translated < (hd_extent_t)minValue)
    {
        low = true;
        *hardwarevar = minValue;
    }
    else if (translated > (hd_extent_t)maxValue)
    {
        high = true;
        *hardwarevar = maxValue;
    }
    else
    {
        *hardwarevar = (hd_hw_extent_t)translated;
    }

    if (high)
        return hyperdisplay_dim_high;

    if (low)
        return hyperdisplay_dim_low;

    return hyperdisplay_dim_ok;
}

hyperdisplay_dim_check_t hyperdisplay_enforceSWLimits(
    hyperdisplay_t *display,
    hd_extent_t *windowvar,
    bool axisSelect)
{
    wind_info_t *wind;
    hd_extent_t extent;
    bool low = false;
    bool high = false;

    if ((display == NULL) ||
        (display->pCurrentWindow == NULL) ||
        (windowvar == NULL))
        return hyperdisplay_dim_no_val;

    wind = display->pCurrentWindow;

    extent =
        (axisSelect == hdY)
            ? (hd_extent_t)hd_abs_len_hw(wind->yMax, wind->yMin)
            : (hd_extent_t)hd_abs_len_hw(wind->xMax, wind->xMin);

    if (*windowvar < 0.0)
    {
        *windowvar = 0.0;
        low = true;
    }

    if (*windowvar >= extent)
    {
        *windowvar = extent - 1.0;
        high = true;
    }

    if (high)
        return hyperdisplay_dim_high;

    if (low)
        return hyperdisplay_dim_low;

    return hyperdisplay_dim_ok;
}

void hyperdisplay_setWindowDefaults(hyperdisplay_t *display,
                                    wind_info_t *window)
{
    if ((display == NULL) || (window == NULL))
        return;

    memset(window, 0, sizeof(*window));

    window->xMin = 0U;
    window->yMin = 0U;

    /*
     * xExt/yExt are pixel counts, not maximum coordinate values.
     * For the 128x160 TFT:
     *   X = 0..127
     *   Y = 0..159
     */
    window->xMax =
        (display->xExt > 0U)
            ? (hd_hw_extent_t)(display->xExt - 1U)
            : 0U;

    window->yMax =
        (display->yExt > 0U)
            ? (hd_hw_extent_t)(display->yExt - 1U)
            : 0U;

    window->bufferMode = false;
    window->dynamic = false;

    hyperdisplay_setWindowColorSequence(display,
                                        window,
                                        NULL,
                                        0U,
                                        0U);
}

/* -------------------------------------------------------------------------- */
/* Weak callback defaults                                                     */
/* -------------------------------------------------------------------------- */

void hyperdisplayXLineCallback(hd_hw_extent_t x0,
                               hd_hw_extent_t y0,
                               hd_hw_extent_t len,
                               color_t data,
                               hd_colors_t colorCycleLength,
                               hd_colors_t startColorOffset,
                               bool goLeft)
{
    (void)x0; (void)y0; (void)len; (void)data;
    (void)colorCycleLength; (void)startColorOffset; (void)goLeft;
}

void hyperdisplayYLineCallback(hd_hw_extent_t x0,
                               hd_hw_extent_t y0,
                               hd_hw_extent_t len,
                               color_t data,
                               hd_colors_t colorCycleLength,
                               hd_colors_t startColorOffset,
                               bool goUp)
{
    (void)x0; (void)y0; (void)len; (void)data;
    (void)colorCycleLength; (void)startColorOffset; (void)goUp;
}

void hyperdisplayRectangleCallback(hd_hw_extent_t x0,
                                   hd_hw_extent_t y0,
                                   hd_hw_extent_t x1,
                                   hd_hw_extent_t y1,
                                   color_t data,
                                   bool filled,
                                   hd_colors_t colorCycleLength,
                                   hd_colors_t startColorOffset,
                                   bool gradientVertical,
                                   bool reverseGradient)
{
    (void)x0; (void)y0; (void)x1; (void)y1; (void)data;
    (void)filled; (void)colorCycleLength; (void)startColorOffset;
    (void)gradientVertical; (void)reverseGradient;
}

void hyperdisplayFillFromArrayCallback(hd_hw_extent_t x0,
                                       hd_hw_extent_t y0,
                                       hd_hw_extent_t x1,
                                       hd_hw_extent_t y1,
                                       hd_pixels_t numPixels,
                                       color_t data)
{
    (void)x0; (void)y0; (void)x1; (void)y1;
    (void)numPixels; (void)data;
}
