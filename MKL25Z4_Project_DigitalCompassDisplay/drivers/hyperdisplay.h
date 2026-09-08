/*
 * hyperdisplay.h
 *
 * SparkFun HyperDisplay port for FRDM-KL25Z
 *
 * Original library:
 * SparkFun Master Display Library
 *
 * KL25Z C port goals:
 *  - Remove Arduino dependency
 *  - Remove C++ classes/templates/inheritance
 *  - Preserve HyperDisplay graphics structures and API concepts
 *  - Allow display-specific drivers to provide hardware callbacks
 *
 * TFT project hardware target (implemented later):
 *      PTD1  -> SPI0_SCK  -> TFT SCLK
 *      PTD2  -> SPI0_SOUT -> TFT MOSI
 *      PTA17 -> GPIO      -> TFT LCDCS
 *      PTA16 -> GPIO      -> TFT D/C
 */

#ifndef HYPERDISPLAY_H_
#define HYPERDISPLAY_H_

/* --------------------------------------------------------------------------
 * Standard C headers
 * -------------------------------------------------------------------------- */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


/* --------------------------------------------------------------------------
 * HyperDisplay configuration
 * -------------------------------------------------------------------------- */

#ifndef HYPERDISPLAY_HAVE_CUSTOM_CONFIG
#include "hyperdisplay_default_conf.h"
#endif


#if HYPERDISPLAY_USE_MATH
#include <math.h>
#endif


#if HYPERDISPLAY_USE_RAY_TRACING
#include "SparkFun_2DRayTracing.h"
#endif


#if HYPERDISPLAY_USE_PRINT

#if HYPERDISPLAY_INCLUDE_DEFAULT_FONT
#include "font5x7.h"

#define HYPERDISPLAY_DEFAULT_FONT_WIDTH   5U
#define HYPERDISPLAY_DEFAULT_FONT_HEIGHT  8U

#endif

#endif


/* --------------------------------------------------------------------------
 * C / C++ compatibility
 * -------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif


/* --------------------------------------------------------------------------
 * Basic HyperDisplay data types
 * -------------------------------------------------------------------------- */

typedef double   hd_extent_t;
typedef uint16_t hd_hw_extent_t;
typedef uint8_t  hd_font_extent_t;
typedef uint32_t hd_colors_t;
typedef hd_colors_t hd_pixels_t;


/*
 * Pixel/color data remains generic because different display drivers may use
 * different color formats.
 */
typedef void *color_t;


/* --------------------------------------------------------------------------
 * Axis / coordinate constants
 * -------------------------------------------------------------------------- */

#define hdX  false
#define hdY  true

#define hdW  true
#define hdH  false


/* --------------------------------------------------------------------------
 * Character information
 * -------------------------------------------------------------------------- */

typedef struct character_info
{
    /*
     * Deprecated in original library.
     * Fonts now normally use the first color in the window color sequence.
     */
    color_t data;

    hd_font_extent_t *xLoc;
    hd_font_extent_t *yLoc;

    hd_font_extent_t xDim;
    hd_font_extent_t yDim;

    hd_pixels_t numPixels;

    bool show;
    bool causesNewline;

} char_info_t;


/* --------------------------------------------------------------------------
 * Window information
 * -------------------------------------------------------------------------- */

typedef struct window_info
{
    /*
     * Window limits in hardware coordinates.
     */
    hd_hw_extent_t xMin;
    hd_hw_extent_t xMax;
    hd_hw_extent_t yMin;
    hd_hw_extent_t yMax;

    /*
     * Current text/drawing cursor in window coordinates.
     */
    hd_extent_t cursorX;
    hd_extent_t cursorY;

    /*
     * Cursor reset location.
     */
    hd_extent_t xReset;
    hd_extent_t yReset;

    /*
     * Information about the last printed character.
     */
    char_info_t lastCharacter;

    /*
     * Current/default color sequence.
     */
    color_t currentSequenceData;

    hd_colors_t currentColorCycleLength;
    hd_colors_t currentColorOffset;

    /*
     * false = direct drawing to TFT
     * true  = drawing into RAM buffer
     */
    bool bufferMode;

    /*
     * Optional pixel buffer.
     */
    color_t data;

    hd_pixels_t numPixels;

    /*
     * Indicates whether the library dynamically allocated the buffer.
     */
    bool dynamic;

} wind_info_t;


/* --------------------------------------------------------------------------
 * Dimension checking
 * -------------------------------------------------------------------------- */

typedef enum
{
    hyperdisplay_dim_ok = 0,
    hyperdisplay_dim_low,
    hyperdisplay_dim_high,
    hyperdisplay_dim_no_val

} hyperdisplay_dim_check_t;


/* --------------------------------------------------------------------------
 * Forward declaration
 * -------------------------------------------------------------------------- */

typedef struct hyperdisplay hyperdisplay_t;


/* --------------------------------------------------------------------------
 * Hardware abstraction callbacks
 *
 * These replace the C++ virtual functions from the original library.
 *
 * The later ILI9163C / KWH018ST01 driver will provide these functions.
 * -------------------------------------------------------------------------- */

typedef color_t (*hyperdisplay_get_offset_color_fn)(
    hyperdisplay_t *display,
    color_t base,
    uint32_t numPixels
);


typedef void (*hyperdisplay_hw_pixel_fn)(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


typedef void (*hyperdisplay_sw_pixel_fn)(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


#if HYPERDISPLAY_USE_PRINT

typedef void (*hyperdisplay_get_char_info_fn)(
    hyperdisplay_t *display,
    uint8_t character,
    char_info_t *pchar
);

#endif


/*
 * Optional accelerated display operations.
 *
 * If these callbacks are NULL, generic HyperDisplay implementations can be
 * used instead.
 */

typedef void (*hyperdisplay_hw_xline_fn)(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goLeft
);


typedef void (*hyperdisplay_hw_yline_fn)(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goUp
);


typedef void (*hyperdisplay_hw_rectangle_fn)(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t x1,
    hd_hw_extent_t y1,
    bool filled,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool reverseGradient,
    bool gradientVertical
);


typedef void (*hyperdisplay_hw_fill_array_fn)(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t x1,
    hd_hw_extent_t y1,
    color_t data,
    hd_pixels_t numPixels,
    bool Vh
);


/* --------------------------------------------------------------------------
 * HyperDisplay hardware operations table
 * -------------------------------------------------------------------------- */

typedef struct hyperdisplay_ops
{
    /*
     * Required display-specific functions.
     */
    hyperdisplay_get_offset_color_fn getOffsetColor;
    hyperdisplay_hw_pixel_fn         hwpixel;
    hyperdisplay_sw_pixel_fn         swpixel;

#if HYPERDISPLAY_USE_PRINT
    hyperdisplay_get_char_info_fn    getCharInfo;
#endif

    /*
     * Optional accelerated operations.
     */
    hyperdisplay_hw_xline_fn         hwxline;
    hyperdisplay_hw_yline_fn         hwyline;
    hyperdisplay_hw_rectangle_fn     hwrectangle;
    hyperdisplay_hw_fill_array_fn    hwfillFromArray;

} hyperdisplay_ops_t;


/* --------------------------------------------------------------------------
 * HyperDisplay object
 *
 * Replaces:
 *
 *      class hyperdisplay : public Print
 *
 * from the original Arduino/C++ library.
 * -------------------------------------------------------------------------- */

struct hyperdisplay
{
    /*
     * Width and height of the physical display.
     */
    hd_hw_extent_t xExt;
    hd_hw_extent_t yExt;

    /*
     * Active drawing window.
     */
    wind_info_t *pCurrentWindow;

    /*
     * Display-specific operations.
     */
    hyperdisplay_ops_t ops;
};


/* --------------------------------------------------------------------------
 * Global default objects
 * -------------------------------------------------------------------------- */

extern wind_info_t hyperdisplayDefaultWindow;
extern char_info_t hyperdisplayDefaultCharacter;


/* --------------------------------------------------------------------------
 * Initialization
 *
 * Replaces the original C++ constructor:
 *
 *      hyperdisplay(uint16_t xSize, uint16_t ySize);
 * -------------------------------------------------------------------------- */

void hyperdisplay_init(
    hyperdisplay_t *display,
    uint16_t xSize,
    uint16_t ySize
);


/* --------------------------------------------------------------------------
 * Utility functions
 * -------------------------------------------------------------------------- */

uint16_t hyperdisplay_getNewColorOffset(
    uint16_t colorCycleLength,
    uint16_t startColorOffset,
    int32_t numWritten
);


hyperdisplay_dim_check_t hyperdisplay_enforceHWLimits(
    hyperdisplay_t *display,
    hd_extent_t *windowvar,
    hd_hw_extent_t *hardwarevar,
    bool axisSelect
);


hyperdisplay_dim_check_t hyperdisplay_enforceSWLimits(
    hyperdisplay_t *display,
    hd_extent_t *windowvar,
    bool axisSelect
);


/* --------------------------------------------------------------------------
 * Hardware-level primitive functions
 * -------------------------------------------------------------------------- */

void hyperdisplay_hwpixel(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


void hyperdisplay_hwxline(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goLeft
);


void hyperdisplay_hwyline(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goUp
);


void hyperdisplay_hwrectangle(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t x1,
    hd_hw_extent_t y1,
    bool filled,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool reverseGradient,
    bool gradientVertical
);


void hyperdisplay_hwfillFromArray(
    hyperdisplay_t *display,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t x1,
    hd_hw_extent_t y1,
    color_t data,
    hd_pixels_t numPixels,
    bool Vh
);


/* --------------------------------------------------------------------------
 * Software-buffer drawing functions
 * -------------------------------------------------------------------------- */

hd_pixels_t hyperdisplay_wToPix(
    wind_info_t *wind,
    hd_hw_extent_t x0,
    hd_hw_extent_t y0
);


void hyperdisplay_swpixel(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


void hyperdisplay_swxline(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goLeft
);


void hyperdisplay_swyline(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goUp
);


void hyperdisplay_swrectangle(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t x1,
    hd_extent_t y1,
    bool filled,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool reverseGradient,
    bool gradientVertical
);


void hyperdisplay_swfillFromArray(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t x1,
    hd_extent_t y1,
    color_t data,
    hd_pixels_t numPixels,
    bool Vh
);


/* --------------------------------------------------------------------------
 * Window configuration
 * -------------------------------------------------------------------------- */

void hyperdisplay_setWindowDefaults(
    hyperdisplay_t *display,
    wind_info_t *pwindow
);


void hyperdisplay_setWindowColorSequence(
    hyperdisplay_t *display,
    wind_info_t *wind,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


void hyperdisplay_setCurrentWindowColorSequence(
    hyperdisplay_t *display,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


int hyperdisplay_setWindowMemory(
    hyperdisplay_t *display,
    wind_info_t *wind,
    color_t data,
    hd_pixels_t numPixels,
    uint8_t bpp,
    bool allowDynamic
);


int hyperdisplay_setCurrentWindowMemory(
    hyperdisplay_t *display,
    color_t data,
    hd_pixels_t numPixels,
    uint8_t bpp,
    bool allowDynamic
);


/* --------------------------------------------------------------------------
 * Buffer / direct drawing control
 * -------------------------------------------------------------------------- */

void hyperdisplay_buffer(
    hyperdisplay_t *display,
    wind_info_t *wind
);


void hyperdisplay_direct(
    hyperdisplay_t *display,
    wind_info_t *wind
);


void hyperdisplay_show(
    hyperdisplay_t *display,
    wind_info_t *wind
);


/* --------------------------------------------------------------------------
 * Primitive drawing API
 * -------------------------------------------------------------------------- */

void hyperdisplay_pixel(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


void hyperdisplay_xline(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goLeft
);


void hyperdisplay_yline(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goUp
);


void hyperdisplay_rectangle(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t x1,
    hd_extent_t y1,
    bool filled,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool reverseGradient,
    bool gradientVertical
);


void hyperdisplay_fillFromArray(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t x1,
    hd_extent_t y1,
    color_t data,
    hd_pixels_t numPixels,
    bool Vh
);


void hyperdisplay_fillWindow(
    hyperdisplay_t *display,
    color_t color,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


/* --------------------------------------------------------------------------
 * Drawing level 1
 * -------------------------------------------------------------------------- */

#if HYPERDISPLAY_DRAWING_LEVEL > 0

uint16_t hyperdisplay_line(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t x1,
    hd_extent_t y1,
    uint16_t width,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool reverseGradient
);


void hyperdisplay_polygon(
    hyperdisplay_t *display,
    hd_extent_t x[],
    hd_extent_t y[],
    uint8_t numSides,
    uint16_t width,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool reverseGradient
);


void hyperdisplay_circle(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t radius,
    bool filled,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool reverseGradient
);


/*
 * Internal line helpers.
 */

hd_extent_t hyperdisplay_lineHighNorm(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t x1,
    hd_extent_t y1,
    uint16_t width,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


hd_extent_t hyperdisplay_lineLowNorm(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t x1,
    hd_extent_t y1,
    uint16_t width,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


hd_extent_t hyperdisplay_lineHighReverse(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t x1,
    hd_extent_t y1,
    uint16_t width,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


hd_extent_t hyperdisplay_lineLowReverse(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t x1,
    hd_extent_t y1,
    uint16_t width,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset
);


void hyperdisplay_circle_Bresenham(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t radius,
    color_t color,
    bool fill
);


void hyperdisplay_circle_midpoint(
    hyperdisplay_t *display,
    hd_extent_t x0,
    hd_extent_t y0,
    hd_extent_t radius,
    color_t color,
    bool fill
);


void hyperdisplay_circle_eight(
    hyperdisplay_t *display,
    hd_extent_t xc,
    hd_extent_t yc,
    hd_extent_t dx,
    hd_extent_t dy,
    color_t color,
    bool fill
);

#endif /* HYPERDISPLAY_DRAWING_LEVEL > 0 */


/* --------------------------------------------------------------------------
 * Ray tracing
 * -------------------------------------------------------------------------- */

#if HYPERDISPLAY_USE_RAY_TRACING

void hyperdisplay_filledPolygon(
    hyperdisplay_t *display,
    sf2drt_polygon *poly,
    uint16_t width,
    bool filled,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool reverseGradient
);

#endif


/* --------------------------------------------------------------------------
 * Text / printing
 *
 * Arduino Print inheritance has been removed.
 * -------------------------------------------------------------------------- */

size_t hyperdisplay_write(
    hyperdisplay_t *display,
    uint8_t val
);


#if HYPERDISPLAY_USE_PRINT

void hyperdisplay_getCharInfo(
    hyperdisplay_t *display,
    uint8_t character,
    char_info_t *pchar
);

#endif


void hyperdisplay_setTextCursor(
    hyperdisplay_t *display,
    int32_t x0,
    int32_t y0,
    wind_info_t *window
);


void hyperdisplay_resetTextCursor(
    hyperdisplay_t *display,
    wind_info_t *window
);


/* --------------------------------------------------------------------------
 * Weak user callbacks
 * -------------------------------------------------------------------------- */

#if defined(__GNUC__)
#define HYPERDISPLAY_WEAK __attribute__((weak))
#else
#define HYPERDISPLAY_WEAK
#endif


void hyperdisplayXLineCallback(
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goLeft
) HYPERDISPLAY_WEAK;


void hyperdisplayYLineCallback(
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t len,
    color_t data,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool goUp
) HYPERDISPLAY_WEAK;


void hyperdisplayRectangleCallback(
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t x1,
    hd_hw_extent_t y1,
    color_t data,
    bool filled,
    hd_colors_t colorCycleLength,
    hd_colors_t startColorOffset,
    bool gradientVertical,
    bool reverseGradient
) HYPERDISPLAY_WEAK;


void hyperdisplayFillFromArrayCallback(
    hd_hw_extent_t x0,
    hd_hw_extent_t y0,
    hd_hw_extent_t x1,
    hd_hw_extent_t y1,
    hd_pixels_t numPixels,
    color_t data
) HYPERDISPLAY_WEAK;


/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* HYPERDISPLAY_H_ */
