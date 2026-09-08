///*
// * KL25Z_TFT_DisplayTest.c
// *
// * FRDM-KL25Z port of SparkFun HyperDisplay Example1_DisplayTest
// * for the SparkFun 1.8in 128x160 KWH018ST01 / ILI9163C TFT.
// *
// * Fixed TFT wiring used by this project:
// *
// *   FRDM-KL25Z             SparkFun TFT
// *   --------------------------------------
// *   PTD1  SPI0_SCK   ----> SCLK
// *   PTD2  SPI0_SOUT  ----> MOSI
// *   PTA17 GPIO       ----> LCDCS
// *   PTA16 GPIO       ----> D/C
// *   3.3 V            ----> 3V3-5V5
// *   GND              ----> GND
// *
// * MISO is not required for the TFT-only write path.
// * PWM/backlight control is intentionally omitted from the first bring-up.
// */
//
//#include <stdint.h>
//#include <stdbool.h>
//
//#include "board.h"
//#include "pin_mux.h"
//#include "clock_config.h"
//
//#include "fsl_clock.h"
//#include "fsl_common.h"
//#include "fsl_debug_console.h"
//#include "fsl_gpio.h"
//#include "fsl_port.h"
//#include "fsl_spi.h"
//
//#include "HyperDisplay_KWH018ST01_4WSPI.h"
//#include "fast_hsv2rgb.h"
//
///* -------------------------------------------------------------------------- */
///* TFT hardware                                                               */
///* -------------------------------------------------------------------------- */
//
//#define TFT_SPI_BASE              SPI0
//
//#define TFT_SCK_PORT              PORTD
//#define TFT_SCK_PIN               1U
//
//#define TFT_MOSI_PORT             PORTD
//#define TFT_MOSI_PIN              2U
//
//#define TFT_DC_PORT               PORTA
//#define TFT_DC_GPIO               GPIOA
//#define TFT_DC_PIN                16U
//
//#define TFT_CS_PORT               PORTA
//#define TFT_CS_GPIO               GPIOA
//#define TFT_CS_PIN                17U
//
//#define TFT_SPI_BAUDRATE_HZ       1000000U
//
///* -------------------------------------------------------------------------- */
///* Globals                                                                    */
///* -------------------------------------------------------------------------- */
//
//static KWH018ST01_4WSPI_t g_tft;
//static ILI9163C_color_18_t g_defaultColor;
//
///* -------------------------------------------------------------------------- */
///* Local prototypes                                                           */
///* -------------------------------------------------------------------------- */
//
//static void TFT_InitPins(void);
//static void TFT_InitSPI(void);
//static void TFT_DelayMs(uint32_t milliseconds);
//static hyperdisplay_t *TFT_GetHyperDisplay(void);
//static void TFT_PrintString(hyperdisplay_t *hd, const char *text);
//
//static void TextTest(void);
//static void LineTest(void);
//static void RectangleTest(void);
//static void CircleTest(void);
//
///* -------------------------------------------------------------------------- */
///* KL25Z hardware initialization                                              */
///* -------------------------------------------------------------------------- */
//
//static void TFT_InitPins(void)
//{
//    CLOCK_EnableClock(kCLOCK_PortD);
//    CLOCK_EnableClock(kCLOCK_PortA);
//
//    /* PTD1 = SPI0_SCK, PTD2 = SPI0_SOUT/MOSI. */
//    PORT_SetPinMux(TFT_SCK_PORT, TFT_SCK_PIN, kPORT_MuxAlt2);
//    PORT_SetPinMux(TFT_MOSI_PORT, TFT_MOSI_PIN, kPORT_MuxAlt2);
//
//    /* LCDCS and D/C are GPIO outputs. */
//    PORT_SetPinMux(TFT_CS_PORT, TFT_CS_PIN, kPORT_MuxAsGpio);
//    PORT_SetPinMux(TFT_DC_PORT, TFT_DC_PIN, kPORT_MuxAsGpio);
//}
//
//static void TFT_InitSPI(void)
//{
//    spi_master_config_t masterConfig;
//    uint32_t sourceClockHz;
//
//    CLOCK_EnableClock(kCLOCK_Spi0);
//
//    SPI_MasterGetDefaultConfig(&masterConfig);
//
//    masterConfig.baudRate_Bps = TFT_SPI_BAUDRATE_HZ;
//
//    /* SPI Mode 0. */
//    masterConfig.polarity = kSPI_ClockPolarityActiveHigh;
//    masterConfig.phase = kSPI_ClockPhaseFirstEdge;
//
//    /* MSB first. */
//    masterConfig.direction = kSPI_MsbFirst;
//
//    /* CS is controlled manually using PTA17. */
//    masterConfig.outputMode = kSPI_SlaveSelectAsGpio;
//    masterConfig.pinMode = kSPI_PinModeNormal;
//
//    sourceClockHz = CLOCK_GetFreq(kCLOCK_BusClk);
//
//    SPI_MasterInit(
//        TFT_SPI_BASE,
//        &masterConfig,
//        sourceClockHz);
//}
//
////static void TFT_DelayMs(uint32_t milliseconds)
////{
////    if (milliseconds == 0U)
////    {
////        return;
////    }
////
////    SDK_DelayAtLeastUs(
////        milliseconds * 1000U,
////        SystemCoreClock);
////}
//
//
//static void TFT_DelayMs(uint32_t milliseconds)
//{
//    uint32_t reload;
//
//    if (milliseconds == 0U)
//    {
//        return;
//    }
//
//    reload = (SystemCoreClock / 1000U) - 1U;
//
//    SysTick->LOAD = reload;
//    SysTick->VAL  = 0U;
//
//    SysTick->CTRL =
//        SysTick_CTRL_CLKSOURCE_Msk |
//        SysTick_CTRL_ENABLE_Msk;
//
//    while (milliseconds > 0U)
//    {
//        while ((SysTick->CTRL &
//                SysTick_CTRL_COUNTFLAG_Msk) == 0U)
//        {
//        }
//
//        milliseconds--;
//    }
//
//    SysTick->CTRL = 0U;
//}
//
//
//static hyperdisplay_t *TFT_GetHyperDisplay(void)
//{
//    return &g_tft.ili9163c4wspi.ili9163c.hyperdisplay;
//}
//
///* -------------------------------------------------------------------------- */
///* Text helper                                                                */
///* -------------------------------------------------------------------------- */
//
//static void TFT_PrintString(
//    hyperdisplay_t *hd,
//    const char *text)
//{
//    if ((hd == NULL) || (text == NULL))
//    {
//        return;
//    }
//
//    while (*text != '\0')
//    {
//        (void)hyperdisplay_write(
//            hd,
//            (uint8_t)*text);
//
//        text++;
//    }
//}
//
///* -------------------------------------------------------------------------- */
///* Text test                                                                  */
///* -------------------------------------------------------------------------- */
//
//static void TextTest(void)
//{
//    hyperdisplay_t *hd = TFT_GetHyperDisplay();
//    uint16_t hue;
//
//    KWH018ST01_4WSPI_clearDisplay(&g_tft);
//
//    hyperdisplay_setCurrentWindowColorSequence(
//        hd,
//        (color_t)&g_defaultColor,
//        1U,
//        0U);
//
//    for (hue = HSV_HUE_MIN;
//         hue <= HSV_HUE_MAX;
//         hue = (uint16_t)(hue + 30U))
//    {
//        hyperdisplay_setTextCursor(
//            hd,
//            0,
//            0,
//            NULL);
//
//        g_defaultColor =
//            ILI9163C_hsvTo18b(
//                hue,
//                255U,
//                255U);
//
//        TFT_PrintString(
//            hd,
//            "Hello world!");
//
//        if ((uint16_t)(HSV_HUE_MAX - hue) < 30U)
//        {
//            break;
//        }
//    }
//}
//
///* -------------------------------------------------------------------------- */
///* Line test                                                                  */
///* -------------------------------------------------------------------------- */
//
//static void LineTest(void)
//{
//    hyperdisplay_t *hd = TFT_GetHyperDisplay();
//    ILI9163C_color_18_t color;
//    hd_hw_extent_t i;
//
//    KWH018ST01_4WSPI_clearDisplay(&g_tft);
//
//    /* White lines from top-left. */
//    color = ILI9163C_rgbTo18b(
//        255U,
//        255U,
//        255U);
//
//    for (i = 0U; i < hd->xExt; i++)
//    {
//        (void)hyperdisplay_line(
//            hd,
//            0.0,
//            0.0,
//            (hd_extent_t)i,
//            (hd_extent_t)(hd->yExt - 1U),
//            1U,
//            (color_t)&color,
//            1U,
//            0U,
//            false);
//    }
//
//    /* Red lines from bottom-left. */
//    color = ILI9163C_rgbTo18b(
//        255U,
//        0U,
//        0U);
//
//    for (i = 0U; i < hd->yExt; i++)
//    {
//        (void)hyperdisplay_line(
//            hd,
//            0.0,
//            (hd_extent_t)(hd->yExt - 1U),
//            (hd_extent_t)(hd->xExt - 1U),
//            (hd_extent_t)(hd->yExt - i - 1U),
//            1U,
//            (color_t)&color,
//            1U,
//            0U,
//            false);
//    }
//
//    /* Green lines from bottom-right. */
//    color = ILI9163C_rgbTo18b(
//        0U,
//        255U,
//        0U);
//
//    for (i = 0U; i < hd->xExt; i++)
//    {
//        (void)hyperdisplay_line(
//            hd,
//            (hd_extent_t)(hd->xExt - 1U),
//            (hd_extent_t)(hd->yExt - 1U),
//            (hd_extent_t)(hd->xExt - i - 1U),
//            0.0,
//            1U,
//            (color_t)&color,
//            1U,
//            0U,
//            false);
//    }
//
//    /* Blue lines from top-right. */
//    color = ILI9163C_rgbTo18b(
//        0U,
//        0U,
//        255U);
//
//    for (i = 0U; i < hd->yExt; i++)
//    {
//        (void)hyperdisplay_line(
//            hd,
//            (hd_extent_t)(hd->xExt - 1U),
//            0.0,
//            0.0,
//            (hd_extent_t)i,
//            1U,
//            (color_t)&color,
//            1U,
//            0U,
//            false);
//    }
//}
//
///* -------------------------------------------------------------------------- */
///* Rectangle test                                                             */
///* -------------------------------------------------------------------------- */
//
//static void RectangleTest(void)
//{
//    hyperdisplay_t *hd = TFT_GetHyperDisplay();
//
//    ILI9163C_color_18_t color;
//
//    hd_hw_extent_t i;
//    hd_hw_extent_t halfX;
//    hd_hw_extent_t halfY;
//
//    KWH018ST01_4WSPI_clearDisplay(&g_tft);
//
//    halfX = (hd_hw_extent_t)(hd->xExt / 2U);
//    halfY = (hd_hw_extent_t)(hd->yExt / 2U);
//
//    color = ILI9163C_rgbTo18b(
//        255U,
//        255U,
//        255U);
//
//    for (i = 0U; i < halfX; i++)
//    {
//        hyperdisplay_rectangle(
//            hd,
//            (hd_extent_t)(halfX - 1U - i),
//            (hd_extent_t)(halfY - 1U - i),
//            (hd_extent_t)(halfX + 1U + i),
//            (hd_extent_t)(halfY + 1U + i),
//            false,
//            (color_t)&color,
//            1U,
//            0U,
//            false,
//            false);
//
//        TFT_DelayMs(50U);
//    }
//
//    /* Fill entire display blue. */
//    color = ILI9163C_rgbTo18b(
//        0U,
//        0U,
//        255U);
//
//    hyperdisplay_rectangle(
//        hd,
//        0.0,
//        0.0,
//        (hd_extent_t)(hd->xExt - 1U),
//        (hd_extent_t)(hd->yExt - 1U),
//        true,
//        (color_t)&color,
//        1U,
//        0U,
//        false,
//        false);
//
//    /* Black rectangles. */
//    color = ILI9163C_rgbTo18b(
//        0U,
//        0U,
//        0U);
//
//    for (i = 0U; i < halfX; i++)
//    {
//        hyperdisplay_rectangle(
//            hd,
//            (hd_extent_t)(halfX - 1U - i),
//            (hd_extent_t)(halfY - 1U - i),
//            (hd_extent_t)(halfX + 1U + i),
//            (hd_extent_t)(halfY + 1U + i),
//            false,
//            (color_t)&color,
//            1U,
//            0U,
//            false,
//            false);
//
//        TFT_DelayMs(50U);
//    }
//}
//
///* -------------------------------------------------------------------------- */
///* Circle test                                                                */
///* -------------------------------------------------------------------------- */
//
//static void CircleTest(void)
//{
//    hyperdisplay_t *hd = TFT_GetHyperDisplay();
//
//    ILI9163C_color_18_t color;
//
//    hd_hw_extent_t i;
//    hd_hw_extent_t halfX;
//    hd_hw_extent_t halfY;
//    hd_hw_extent_t maxRadius;
//
//    halfX = (hd_hw_extent_t)(hd->xExt / 2U);
//    halfY = (hd_hw_extent_t)(hd->yExt / 2U);
//
//    maxRadius =
//        (hd_hw_extent_t)(halfX - 1U);
//
//    KWH018ST01_4WSPI_clearDisplay(&g_tft);
//
//    for (i = 0U; i < maxRadius; i++)
//    {
//        uint16_t hue;
//
//        hue =
//            (uint16_t)(
//                ((uint32_t)HSV_HUE_MAX *
//                 (uint32_t)i) /
//                (uint32_t)maxRadius);
//
//        color =
//            ILI9163C_hsvTo18b(
//                hue,
//                255U,
//                255U);
//
//        hyperdisplay_circle(
//            hd,
//            (hd_extent_t)(halfX - 1U),
//            (hd_extent_t)(halfY - 1U),
//            (hd_extent_t)(maxRadius - i),
//            true,
//            (color_t)&color,
//            1U,
//            0U,
//            false);
//
//        TFT_DelayMs(100U);
//    }
//}
//
///* -------------------------------------------------------------------------- */
///* main                                                                       */
///* -------------------------------------------------------------------------- */
//
//int main(void)
//{
//    ILI9163C_STAT_t status;
//
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    PRINTF(
//        "\r\nKL25Z + SparkFun 1.8in TFT Display Test\r\n");
//
//    TFT_InitPins();
//    TFT_InitSPI();
//
//    KWH018ST01_4WSPI_init(&g_tft);
//
//    status =
//        KWH018ST01_4WSPI_begin(
//            &g_tft,
//
//            TFT_SPI_BASE,
//
//            TFT_DC_GPIO,
//            TFT_DC_PIN,
//
//            TFT_CS_GPIO,
//            TFT_CS_PIN,
//
//            NULL,
//            NULL,
//
//            TFT_SPI_BAUDRATE_HZ);
//
//    if (status != ILI9163C_STAT_Nominal)
//    {
//        PRINTF(
//            "TFT initialization FAILED.\r\n");
//
//        while (1)
//        {
//        }
//    }
//
//    PRINTF(
//        "TFT initialization successful.\r\n");
//
//    TextTest();
//
//    TFT_DelayMs(1000U);
//
//    while (1)
//    {
//        PRINTF("Line test\r\n");
//
//        LineTest();
//
//        TFT_DelayMs(500U);
//
//        PRINTF("Rectangle test\r\n");
//
//        RectangleTest();
//
//        TFT_DelayMs(500U);
//
//        PRINTF("Circle test\r\n");
//
//        CircleTest();
//
//        TFT_DelayMs(500U);
//
//        PRINTF("Text test\r\n");
//
//        TextTest();
//
//        TFT_DelayMs(1000U);
//    }
//}

















/*
 * FRDM-KL25Z + SparkFun 1.8" 128x160 TFT
 * Static Compass Display Demo
 *
 * TFT wiring:
 *
 *   FRDM-KL25Z             SparkFun TFT
 *   --------------------------------------
 *   PTD1  SPI0_SCK   ----> SCLK
 *   PTD2  SPI0_SOUT  ----> MOSI
 *   PTA17 GPIO       ----> LCDCS
 *   PTA16 GPIO       ----> D/C
 *   3.3V             ----> 3V3-5V5
 *   GND              ----> GND
 */
//
//#include <stdint.h>
//#include <stdbool.h>
//
//#include "board.h"
//#include "pin_mux.h"
//#include "clock_config.h"
//
//#include "fsl_clock.h"
//#include "fsl_common.h"
//#include "fsl_debug_console.h"
//#include "fsl_gpio.h"
//#include "fsl_port.h"
//#include "fsl_spi.h"
//
//#include "HyperDisplay_KWH018ST01_4WSPI.h"
//
///* -------------------------------------------------------------------------- */
///* TFT hardware                                                               */
///* -------------------------------------------------------------------------- */
//
//#define TFT_SPI_BASE              SPI0
//
//#define TFT_SCK_PORT              PORTD
//#define TFT_SCK_PIN               1U
//
//#define TFT_MOSI_PORT             PORTD
//#define TFT_MOSI_PIN              2U
//
//#define TFT_DC_PORT               PORTA
//#define TFT_DC_GPIO               GPIOA
//#define TFT_DC_PIN                16U
//
//#define TFT_CS_PORT               PORTA
//#define TFT_CS_GPIO               GPIOA
//#define TFT_CS_PIN                17U
//
///* Keep the known-good bring-up speed for now. */
//#define TFT_SPI_BAUDRATE_HZ       1000000U
//
///* -------------------------------------------------------------------------- */
///* Compass geometry                                                           */
///* -------------------------------------------------------------------------- */
//
//#define COMPASS_CENTER_X          64
//#define COMPASS_CENTER_Y          58
//#define COMPASS_RADIUS            48
//
//typedef struct
//{
//    int16_t x1;
//    int16_t y1;
//    int16_t x2;
//    int16_t y2;
//} compass_tick_t;
//
///*
// * Pre-calculated tick locations every 15 degrees.
// *
// * This avoids needing sin() and cos() for the static compass face.
// */
//static const compass_tick_t g_compassTicks[] =
//{
//    {64, 19, 64, 10},     /*   0 */
//    {75, 16, 76, 12},     /*  15 */
//    {86, 21, 88, 16},     /*  30 */
//    {94, 28, 98, 24},     /*  45 */
//    {101,36,106,34},      /*  60 */
//    {106,47,110,46},      /*  75 */
//
//    {103,58,112,58},      /*  90 */
//    {106,69,110,70},      /* 105 */
//    {101,79,106,82},      /* 120 */
//    {94, 88, 98, 92},     /* 135 */
//    {86, 95, 88,100},     /* 150 */
//    {75,100, 76,104},     /* 165 */
//
//    {64, 97, 64,106},     /* 180 */
//    {53,100, 52,104},     /* 195 */
//    {42, 95, 40,100},     /* 210 */
//    {34, 88, 30, 92},     /* 225 */
//    {27, 80, 22, 82},     /* 240 */
//    {22, 69, 18, 70},     /* 255 */
//
//    {25, 58, 16, 58},     /* 270 */
//    {22, 47, 18, 46},     /* 285 */
//    {27, 36, 22, 34},     /* 300 */
//    {34, 28, 30, 24},     /* 315 */
//    {42, 21, 40, 16},     /* 330 */
//    {53, 16, 52, 12}      /* 345 */
//};
//
//#define COMPASS_NUM_TICKS \
//    (sizeof(g_compassTicks) / sizeof(g_compassTicks[0]))
//
///* -------------------------------------------------------------------------- */
///* Globals                                                                    */
///* -------------------------------------------------------------------------- */
//
//static KWH018ST01_4WSPI_t g_tft;
//
///* -------------------------------------------------------------------------- */
///* Function prototypes                                                        */
///* -------------------------------------------------------------------------- */
//
//static void TFT_InitPins(void);
//static void TFT_InitSPI(void);
//
//static hyperdisplay_t *TFT_GetHyperDisplay(void);
//
//static void TFT_PrintString(
//    hyperdisplay_t *hd,
//    int32_t x,
//    int32_t y,
//    const char *text);
//
//static void Compass_DrawFace(void);
//static void Compass_DrawNeedle143(void);
//static void Compass_DrawHeading(void);
//static void Compass_DrawDemo(void);
//
///* -------------------------------------------------------------------------- */
///* TFT pin initialization                                                     */
///* -------------------------------------------------------------------------- */
//
//static void TFT_InitPins(void)
//{
//    CLOCK_EnableClock(kCLOCK_PortD);
//    CLOCK_EnableClock(kCLOCK_PortA);
//
//    /*
//     * SPI0:
//     *
//     * PTD1 = SPI0_SCK
//     * PTD2 = SPI0_SOUT / MOSI
//     */
//    PORT_SetPinMux(
//        TFT_SCK_PORT,
//        TFT_SCK_PIN,
//        kPORT_MuxAlt2);
//
//    PORT_SetPinMux(
//        TFT_MOSI_PORT,
//        TFT_MOSI_PIN,
//        kPORT_MuxAlt2);
//
//    /*
//     * GPIO:
//     *
//     * PTA17 = TFT CS
//     * PTA16 = TFT D/C
//     */
//    PORT_SetPinMux(
//        TFT_CS_PORT,
//        TFT_CS_PIN,
//        kPORT_MuxAsGpio);
//
//    PORT_SetPinMux(
//        TFT_DC_PORT,
//        TFT_DC_PIN,
//        kPORT_MuxAsGpio);
//}
//
///* -------------------------------------------------------------------------- */
///* SPI initialization                                                         */
///* -------------------------------------------------------------------------- */
//
//static void TFT_InitSPI(void)
//{
//    spi_master_config_t masterConfig;
//    uint32_t sourceClockHz;
//
//    CLOCK_EnableClock(kCLOCK_Spi0);
//
//    SPI_MasterGetDefaultConfig(&masterConfig);
//
//    masterConfig.baudRate_Bps =
//        TFT_SPI_BAUDRATE_HZ;
//
//    /*
//     * ILI9163C:
//     *
//     * SPI Mode 0
//     * Clock idle LOW
//     * Data captured on first edge
//     */
//    masterConfig.polarity =
//        kSPI_ClockPolarityActiveHigh;
//
//    masterConfig.phase =
//        kSPI_ClockPhaseFirstEdge;
//
//    masterConfig.direction =
//        kSPI_MsbFirst;
//
//    /*
//     * We manually control TFT chip select
//     * using PTA17.
//     */
//    masterConfig.outputMode =
//        kSPI_SlaveSelectAsGpio;
//
//    masterConfig.pinMode =
//        kSPI_PinModeNormal;
//
//    sourceClockHz =
//        CLOCK_GetFreq(kCLOCK_BusClk);
//
//    SPI_MasterInit(
//        TFT_SPI_BASE,
//        &masterConfig,
//        sourceClockHz);
//}
//
///* -------------------------------------------------------------------------- */
///* HyperDisplay access                                                        */
///* -------------------------------------------------------------------------- */
//
//static hyperdisplay_t *TFT_GetHyperDisplay(void)
//{
//    return
//        &g_tft
//            .ili9163c4wspi
//            .ili9163c
//            .hyperdisplay;
//}
//
///* -------------------------------------------------------------------------- */
///* 5x7 text helper                                                            */
///* -------------------------------------------------------------------------- */
//
//static void TFT_PrintString(
//    hyperdisplay_t *hd,
//    int32_t x,
//    int32_t y,
//    const char *text)
//{
//    if ((hd == NULL) || (text == NULL))
//    {
//        return;
//    }
//
//    hyperdisplay_setTextCursor(
//        hd,
//        x,
//        y,
//        NULL);
//
//    while (*text != '\0')
//    {
//        (void)hyperdisplay_write(
//            hd,
//            (uint8_t)*text);
//
//        text++;
//    }
//}
//
///* -------------------------------------------------------------------------- */
///* Draw compass face                                                          */
///* -------------------------------------------------------------------------- */
//
//static void Compass_DrawFace(void)
//{
//    hyperdisplay_t *hd;
//
//    ILI9163C_color_18_t green;
//    uint32_t i;
//
//    hd = TFT_GetHyperDisplay();
//
//    /*
//     * Bright green similar to the design
//     * we selected.
//     */
//    green =
//        ILI9163C_rgbTo18b(
//            80U,
//            255U,
//            40U);
//
//    /*
//     * Tell HyperDisplay that our text color
//     * is green.
//     */
//    hyperdisplay_setCurrentWindowColorSequence(
//        hd,
//        (color_t)&green,
//        1U,
//        0U);
//
//    /* ------------------------------------------------------ */
//    /* Outer compass circle                                   */
//    /* ------------------------------------------------------ */
//
//    hyperdisplay_circle(
//        hd,
//
//        COMPASS_CENTER_X,
//        COMPASS_CENTER_Y,
//
//        COMPASS_RADIUS,
//
//        false,
//
//        (color_t)&green,
//        1U,
//        0U,
//
//        false);
//
//    /* ------------------------------------------------------ */
//    /* Compass tick marks                                     */
//    /* ------------------------------------------------------ */
//
//    for (i = 0U;
//         i < COMPASS_NUM_TICKS;
//         i++)
//    {
//        (void)hyperdisplay_line(
//            hd,
//
//            g_compassTicks[i].x1,
//            g_compassTicks[i].y1,
//
//            g_compassTicks[i].x2,
//            g_compassTicks[i].y2,
//
//            1U,
//
//            (color_t)&green,
//            1U,
//            0U,
//
//            false);
//    }
//
//    /* ------------------------------------------------------ */
//    /* Cardinal direction labels                              */
//    /* ------------------------------------------------------ */
//
//    /*
//     * Coordinates are deliberately placed
//     * inside the circle because we only have
//     * the 5x7 font right now.
//     */
//
//    TFT_PrintString(
//        hd,
//        61,
//        22,
//        "N");
//
//    TFT_PrintString(
//        hd,
//        98,
//        55,
//        "E");
//
//    TFT_PrintString(
//        hd,
//        61,
//        87,
//        "S");
//
//    TFT_PrintString(
//        hd,
//        25,
//        55,
//        "W");
//}
//
///* -------------------------------------------------------------------------- */
///* Draw example 143 degree needle                                             */
///* -------------------------------------------------------------------------- */
//
//static void Compass_DrawNeedle143(void)
//{
//    hyperdisplay_t *hd;
//
//    ILI9163C_color_18_t red;
//    ILI9163C_color_18_t white;
//    ILI9163C_color_18_t black;
//
//    hd = TFT_GetHyperDisplay();
//
//    red =
//        ILI9163C_rgbTo18b(
//            255U,
//            30U,
//            20U);
//
//    white =
//        ILI9163C_rgbTo18b(
//            255U,
//            255U,
//            255U);
//
//    black =
//        ILI9163C_rgbTo18b(
//            0U,
//            0U,
//            0U);
//
//    /*
//     * Heading = 143 degrees.
//     *
//     * For our screen coordinate system:
//     *
//     * center = (64,58)
//     *
//     * 143 degree endpoint ≈ (84,85)
//     *
//     * opposite endpoint ≈ (44,31)
//     */
//
//    /* ------------------------------------------------------ */
//    /* White rear half of needle                              */
//    /* ------------------------------------------------------ */
//
//    (void)hyperdisplay_line(
//        hd,
//
//        COMPASS_CENTER_X,
//        COMPASS_CENTER_Y,
//
//        44,
//        31,
//
//        3U,
//
//        (color_t)&white,
//        1U,
//        0U,
//
//        false);
//
//    /* ------------------------------------------------------ */
//    /* Red heading half                                      */
//    /* ------------------------------------------------------ */
//
//    (void)hyperdisplay_line(
//        hd,
//
//        COMPASS_CENTER_X,
//        COMPASS_CENTER_Y,
//
//        84,
//        85,
//
//        3U,
//
//        (color_t)&red,
//        1U,
//        0U,
//
//        false);
//
//    /* ------------------------------------------------------ */
//    /* Center hub                                            */
//    /* ------------------------------------------------------ */
//
//    hyperdisplay_circle(
//        hd,
//
//        COMPASS_CENTER_X,
//        COMPASS_CENTER_Y,
//
//        4,
//
//        true,
//
//        (color_t)&white,
//        1U,
//        0U,
//
//        false);
//
//    /*
//     * Small black dot inside the white hub.
//     */
//    hyperdisplay_circle(
//        hd,
//
//        COMPASS_CENTER_X,
//        COMPASS_CENTER_Y,
//
//        2,
//
//        true,
//
//        (color_t)&black,
//        1U,
//        0U,
//
//        false);
//}
//
///* -------------------------------------------------------------------------- */
///* Draw heading information                                                   */
///* -------------------------------------------------------------------------- */
//
//static void Compass_DrawHeading(void)
//{
//    hyperdisplay_t *hd;
//
//    ILI9163C_color_18_t green;
//
//    hd = TFT_GetHyperDisplay();
//
//    green =
//        ILI9163C_rgbTo18b(
//            80U,
//            255U,
//            40U);
//
//    hyperdisplay_setCurrentWindowColorSequence(
//        hd,
//        (color_t)&green,
//        1U,
//        0U);
//
//    /*
//     * Existing 5x7 font.
//     *
//     * "143" is approximately 18 pixels wide,
//     * so starting near x=54 centers it.
//     */
//
//    TFT_PrintString(
//        hd,
//        54,
//        119,
//        "143");
//
//    /*
//     * Draw the degree symbol ourselves since
//     * we do not need another font yet.
//     */
//    hyperdisplay_circle(
//        hd,
//
//        75,
//        120,
//
//        2,
//
//        false,
//
//        (color_t)&green,
//        1U,
//        0U,
//
//        false);
//
//    /*
//     * Direction text.
//     */
//    TFT_PrintString(
//        hd,
//        58,
//        132,
//        "SE");
//}
//
///* -------------------------------------------------------------------------- */
///* Complete compass screen                                                    */
///* -------------------------------------------------------------------------- */
//
//static void Compass_DrawDemo(void)
//{
//    /*
//     * clearDisplay() gives us the black
//     * background.
//     */
//    KWH018ST01_4WSPI_clearDisplay(
//        &g_tft);
//
//    Compass_DrawFace();
//
//    Compass_DrawNeedle143();
//
//    Compass_DrawHeading();
//}
//
///* -------------------------------------------------------------------------- */
///* main                                                                       */
///* -------------------------------------------------------------------------- */
//
//int main(void)
//{
//    ILI9163C_STAT_t status;
//
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    PRINTF(
//        "\r\n"
//        "KL25Z TFT Compass Display\r\n");
//
//    /* ------------------------------------------------------ */
//    /* Initialize TFT hardware                                */
//    /* ------------------------------------------------------ */
//
//    TFT_InitPins();
//
//    TFT_InitSPI();
//
//    KWH018ST01_4WSPI_init(
//        &g_tft);
//
//    status =
//        KWH018ST01_4WSPI_begin(
//            &g_tft,
//
//            TFT_SPI_BASE,
//
//            TFT_DC_GPIO,
//            TFT_DC_PIN,
//
//            TFT_CS_GPIO,
//            TFT_CS_PIN,
//
//            NULL,
//            NULL,
//
//            TFT_SPI_BAUDRATE_HZ);
//
//    if (status != ILI9163C_STAT_Nominal)
//    {
//        PRINTF(
//            "TFT initialization FAILED.\r\n");
//
//        while (1)
//        {
//        }
//    }
//
//    PRINTF(
//        "TFT initialization successful.\r\n");
//
//    /* ------------------------------------------------------ */
//    /* Draw compass                                           */
//    /* ------------------------------------------------------ */
//
//    Compass_DrawDemo();
//
//    PRINTF(
//        "Compass display drawn.\r\n");
//
//    /* ------------------------------------------------------ */
//    /* Nothing needs to move yet                              */
//    /* ------------------------------------------------------ */
//
//    while (1)
//    {
//        /*
//         * Static compass demo.
//         *
//         * Magnetometer updating will be
//         * added here later.
//         */
//    }
//}























/*
 * FRDM-KL25Z + MMC5983MA + SparkFun 1.8" TFT Compass
 *
 * Combined from:
 *   1) Known-good MMC5983MA four-point compass application
 *   2) Known-good SparkFun HyperDisplay TFT compass display
 *
 * TFT:
 *   PTD1  = SPI0_SCK  -> TFT SCLK
 *   PTD2  = SPI0_SOUT -> TFT MOSI
 *   PTA17 = GPIO      -> TFT LCDCS
 *   PTA16 = GPIO      -> TFT D/C
 *
 * MMC5983MA:
 *   PTC8  = I2C0_SCL
 *   PTC9  = I2C0_SDA
 *   Address = 0x30
 *
 * The low-level compass.c / compass.h driver is not modified.
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"

#include "fsl_clock.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_i2c.h"
#include "fsl_port.h"
#include "fsl_spi.h"

#include "compass.h"
#include "HyperDisplay_KWH018ST01_4WSPI.h"

/* -------------------------------------------------------------------------- */
/* Compass I2C configuration                                                  */
/* -------------------------------------------------------------------------- */

#define COMPASS_I2C_BASE          I2C0
#define COMPASS_I2C_BAUDRATE      (100000U)

#define COMPASS_SCL_PORT          PORTC
#define COMPASS_SCL_PIN           8U

#define COMPASS_SDA_PORT          PORTC
#define COMPASS_SDA_PIN           9U

#define RAD_TO_DEG                (57.2957795f)

/* -------------------------------------------------------------------------- */
/* Four-point 2D environmental calibration constants                         */
/* -------------------------------------------------------------------------- */

#define COMPASS_X_BIAS_G          (-0.0255f)
#define COMPASS_Y_BIAS_G          ( 0.1375f)

#define COMPASS_X_RADIUS_G        ( 0.2365f)
#define COMPASS_Y_RADIUS_G        ( 0.2475f)

/* -------------------------------------------------------------------------- */
/* TFT hardware                                                               */
/* -------------------------------------------------------------------------- */

#define TFT_SPI_BASE              SPI0

#define TFT_SCK_PORT              PORTD
#define TFT_SCK_PIN               1U

#define TFT_MOSI_PORT             PORTD
#define TFT_MOSI_PIN              2U

#define TFT_DC_PORT               PORTA
#define TFT_DC_GPIO               GPIOA
#define TFT_DC_PIN                16U

#define TFT_CS_PORT               PORTA
#define TFT_CS_GPIO               GPIOA
#define TFT_CS_PIN                17U

/*
 * Keep the known-good conservative SPI speed.
 * Increase only after the integrated compass is stable.
 */
#define TFT_SPI_BAUDRATE_HZ       1000000U

/* -------------------------------------------------------------------------- */
/* Compass display geometry                                                   */
/* -------------------------------------------------------------------------- */

#define COMPASS_CENTER_X          64
#define COMPASS_CENTER_Y          58
#define COMPASS_RADIUS            48

/*
 * Keep the moving needle inside the green tick marks and cardinal letters.
 * This lets us erase only the old needle instead of redrawing the whole TFT.
 */
#define COMPASS_NEEDLE_FRONT      28
#define COMPASS_NEEDLE_REAR       21
#define COMPASS_NEEDLE_WIDTH      3U

typedef struct
{
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
} compass_tick_t;

/*
 * Tick marks every 15 degrees.
 */
static const compass_tick_t g_compassTicks[] =
{
    {64, 19, 64, 10},     /*   0 */
    {75, 16, 76, 12},     /*  15 */
    {86, 21, 88, 16},     /*  30 */
    {94, 28, 98, 24},     /*  45 */
    {101, 36, 106, 34},   /*  60 */
    {106, 47, 110, 46},   /*  75 */

    {103, 58, 112, 58},   /*  90 */
    {106, 69, 110, 70},   /* 105 */
    {101, 79, 106, 82},   /* 120 */
    {94, 88, 98, 92},     /* 135 */
    {86, 95, 88, 100},    /* 150 */
    {75, 100, 76, 104},   /* 165 */

    {64, 97, 64, 106},    /* 180 */
    {53, 100, 52, 104},   /* 195 */
    {42, 95, 40, 100},    /* 210 */
    {34, 88, 30, 92},     /* 225 */
    {27, 80, 22, 82},     /* 240 */
    {22, 69, 18, 70},     /* 255 */

    {25, 58, 16, 58},     /* 270 */
    {22, 47, 18, 46},     /* 285 */
    {27, 36, 22, 34},     /* 300 */
    {34, 28, 30, 24},     /* 315 */
    {42, 21, 40, 16},     /* 330 */
    {53, 16, 52, 12}      /* 345 */
};

#define COMPASS_NUM_TICKS \
    (sizeof(g_compassTicks) / sizeof(g_compassTicks[0]))

/* -------------------------------------------------------------------------- */
/* Globals                                                                    */
/* -------------------------------------------------------------------------- */

static KWH018ST01_4WSPI_t g_tft;

static ILI9163C_color_18_t g_colorGreen;
static ILI9163C_color_18_t g_colorRed;
static ILI9163C_color_18_t g_colorWhite;
static ILI9163C_color_18_t g_colorBlack;

static bool g_previousNeedleValid = false;
static int16_t g_previousNeedleDx = 0;
static int16_t g_previousNeedleDy = 0;

/* -------------------------------------------------------------------------- */
/* Local delay for older KL25Z SDK                                            */
/* -------------------------------------------------------------------------- */

static void APP_DelayUs(uint32_t delay_us)
{
    uint32_t core_clock_hz;
    uint64_t ticks_remaining;

    if (delay_us == 0U)
    {
        return;
    }

    core_clock_hz = CLOCK_GetFreq(kCLOCK_CoreSysClk);

    ticks_remaining =
        ((uint64_t)core_clock_hz * (uint64_t)delay_us) / 1000000ULL;

    while (ticks_remaining != 0ULL)
    {
        uint32_t chunk;

        if (ticks_remaining > 0x01000000ULL)
        {
            chunk = 0x01000000UL;
        }
        else
        {
            chunk = (uint32_t)ticks_remaining;
        }

        if (chunk == 0U)
        {
            chunk = 1U;
        }

        SysTick->CTRL = 0U;
        SysTick->LOAD = chunk - 1U;
        SysTick->VAL  = 0U;

        SysTick->CTRL =
            SysTick_CTRL_CLKSOURCE_Msk |
            SysTick_CTRL_ENABLE_Msk;

        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U)
        {
        }

        SysTick->CTRL = 0U;

        if (ticks_remaining >= (uint64_t)chunk)
        {
            ticks_remaining -= (uint64_t)chunk;
        }
        else
        {
            ticks_remaining = 0ULL;
        }
    }
}

/* -------------------------------------------------------------------------- */
/* I2C0 pin mux                                                               */
/* -------------------------------------------------------------------------- */

static void APP_InitI2CPins(void)
{
    CLOCK_EnableClock(kCLOCK_PortC);

    /*
     * KL25Z:
     *   PTC8 ALT2 = I2C0_SCL
     *   PTC9 ALT2 = I2C0_SDA
     */
    PORT_SetPinMux(
        COMPASS_SCL_PORT,
        COMPASS_SCL_PIN,
        kPORT_MuxAlt2);

    PORT_SetPinMux(
        COMPASS_SDA_PORT,
        COMPASS_SDA_PIN,
        kPORT_MuxAlt2);
}

/* -------------------------------------------------------------------------- */
/* Initialize I2C0                                                            */
/* -------------------------------------------------------------------------- */

static void APP_InitI2C0(void)
{
    i2c_master_config_t masterConfig;

    I2C_MasterGetDefaultConfig(&masterConfig);

    masterConfig.baudRate_Bps = COMPASS_I2C_BAUDRATE;

    I2C_MasterInit(
        COMPASS_I2C_BASE,
        &masterConfig,
        CLOCK_GetFreq(kCLOCK_BusClk));
}

/* -------------------------------------------------------------------------- */
/* TFT pin mux                                                                */
/* -------------------------------------------------------------------------- */

static void TFT_InitPins(void)
{
    CLOCK_EnableClock(kCLOCK_PortD);
    CLOCK_EnableClock(kCLOCK_PortA);

    /* PTD1 = SPI0_SCK. */
    PORT_SetPinMux(
        TFT_SCK_PORT,
        TFT_SCK_PIN,
        kPORT_MuxAlt2);

    /* PTD2 = SPI0_SOUT / MOSI. */
    PORT_SetPinMux(
        TFT_MOSI_PORT,
        TFT_MOSI_PIN,
        kPORT_MuxAlt2);

    /* PTA17 = TFT CS GPIO. */
    PORT_SetPinMux(
        TFT_CS_PORT,
        TFT_CS_PIN,
        kPORT_MuxAsGpio);

    /* PTA16 = TFT D/C GPIO. */
    PORT_SetPinMux(
        TFT_DC_PORT,
        TFT_DC_PIN,
        kPORT_MuxAsGpio);
}

/* -------------------------------------------------------------------------- */
/* SPI0 initialization                                                        */
/* -------------------------------------------------------------------------- */

static void TFT_InitSPI(void)
{
    spi_master_config_t masterConfig;
    uint32_t sourceClockHz;

    CLOCK_EnableClock(kCLOCK_Spi0);

    SPI_MasterGetDefaultConfig(&masterConfig);

    masterConfig.baudRate_Bps =
        TFT_SPI_BAUDRATE_HZ;

    /* SPI Mode 0. */
    masterConfig.polarity =
        kSPI_ClockPolarityActiveHigh;

    masterConfig.phase =
        kSPI_ClockPhaseFirstEdge;

    masterConfig.direction =
        kSPI_MsbFirst;

    /*
     * TFT CS is manually controlled by the converted ILI9163C driver.
     */
    masterConfig.outputMode =
        kSPI_SlaveSelectAsGpio;

    masterConfig.pinMode =
        kSPI_PinModeNormal;

    sourceClockHz =
        CLOCK_GetFreq(kCLOCK_BusClk);

    SPI_MasterInit(
        TFT_SPI_BASE,
        &masterConfig,
        sourceClockHz);
}

/* -------------------------------------------------------------------------- */
/* HyperDisplay helpers                                                       */
/* -------------------------------------------------------------------------- */

static hyperdisplay_t *TFT_GetHyperDisplay(void)
{
    return &g_tft.ili9163c4wspi.ili9163c.hyperdisplay;
}

static void TFT_SetTextColor(ILI9163C_color_18_t *color)
{
    hyperdisplay_t *hd = TFT_GetHyperDisplay();

    hyperdisplay_setCurrentWindowColorSequence(
        hd,
        (color_t)color,
        1U,
        0U);
}

static void TFT_PrintString(
    int32_t x,
    int32_t y,
    const char *text)
{
    hyperdisplay_t *hd = TFT_GetHyperDisplay();

    if (text == NULL)
    {
        return;
    }

    hyperdisplay_setTextCursor(
        hd,
        x,
        y,
        NULL);

    while (*text != '\0')
    {
        (void)hyperdisplay_write(
            hd,
            (uint8_t)*text);

        text++;
    }
}

/* -------------------------------------------------------------------------- */
/* Integer-only terminal helpers                                              */
/* -------------------------------------------------------------------------- */

static char APP_GetFloatSign(float value)
{
    if (value < 0.0f)
    {
        return '-';
    }

    return '+';
}

static int32_t APP_FloatMagnitudeToMilliUnits(float value)
{
    float magnitude;

    magnitude = value;

    if (magnitude < 0.0f)
    {
        magnitude = -magnitude;
    }

    return (int32_t)(magnitude * 1000.0f + 0.5f);
}

static int32_t APP_NormalizedToThousandths(float value)
{
    float magnitude;

    magnitude = value;

    if (magnitude < 0.0f)
    {
        magnitude = -magnitude;
    }

    return (int32_t)(magnitude * 1000.0f + 0.5f);
}

static int APP_HeadingToTenths(float heading_deg)
{
    int heading_tenths;

    heading_tenths =
        (int)(heading_deg * 10.0f + 0.5f);

    if (heading_tenths >= 3600)
    {
        heading_tenths -= 3600;
    }

    if (heading_tenths < 0)
    {
        heading_tenths += 3600;
    }

    return heading_tenths;
}

/* -------------------------------------------------------------------------- */
/* Compass display: static green face                                         */
/* -------------------------------------------------------------------------- */

static void Compass_DrawFace(void)
{
    hyperdisplay_t *hd = TFT_GetHyperDisplay();
    uint32_t i;

    /*
     * Black background.
     */
    KWH018ST01_4WSPI_clearDisplay(&g_tft);

    /*
     * Outer green circle.
     */
    hyperdisplay_circle(
        hd,
        COMPASS_CENTER_X,
        COMPASS_CENTER_Y,
        COMPASS_RADIUS,
        false,
        (color_t)&g_colorGreen,
        1U,
        0U,
        false);

    /*
     * Tick marks.
     */
    for (i = 0U; i < COMPASS_NUM_TICKS; i++)
    {
        (void)hyperdisplay_line(
            hd,
            g_compassTicks[i].x1,
            g_compassTicks[i].y1,
            g_compassTicks[i].x2,
            g_compassTicks[i].y2,
            1U,
            (color_t)&g_colorGreen,
            1U,
            0U,
            false);
    }

    /*
     * 5x7 cardinal letters.
     */
    TFT_SetTextColor(&g_colorGreen);

    TFT_PrintString(61, 22, "N");
    TFT_PrintString(98, 55, "E");
    TFT_PrintString(61, 87, "S");
    TFT_PrintString(25, 55, "W");
}

/* -------------------------------------------------------------------------- */
/* Compass display: clear only the lower heading area                         */
/* -------------------------------------------------------------------------- */

static void Compass_ClearHeadingArea(void)
{
    hyperdisplay_t *hd = TFT_GetHyperDisplay();

    hyperdisplay_rectangle(
        hd,
        37.0,
        112.0,
        91.0,
        151.0,
        true,
        (color_t)&g_colorBlack,
        1U,
        0U,
        false,
        false);
}

/* -------------------------------------------------------------------------- */
/* Convert heading tenths into fixed 5x7 text: "143.8"                        */
/* -------------------------------------------------------------------------- */

static void Compass_FormatHeading(
    int heading_tenths,
    char text[6])
{
    int degrees;
    int tenths;

    if (heading_tenths < 0)
    {
        heading_tenths = 0;
    }

    if (heading_tenths >= 3600)
    {
        heading_tenths %= 3600;
    }

    degrees = heading_tenths / 10;
    tenths  = heading_tenths % 10;

    text[0] = (char)('0' + ((degrees / 100) % 10));
    text[1] = (char)('0' + ((degrees / 10) % 10));
    text[2] = (char)('0' + (degrees % 10));
    text[3] = '.';
    text[4] = (char)('0' + tenths);
    text[5] = '\0';
}

/* -------------------------------------------------------------------------- */
/* Determine N / NE / E / SE / S / SW / W / NW                              */
/* -------------------------------------------------------------------------- */

static const char *Compass_GetDirection(
    int heading_tenths)
{
    if ((heading_tenths >= 3375) ||
        (heading_tenths < 225))
    {
        return "N";
    }

    if (heading_tenths < 675)
    {
        return "NE";
    }

    if (heading_tenths < 1125)
    {
        return "E";
    }

    if (heading_tenths < 1575)
    {
        return "SE";
    }

    if (heading_tenths < 2025)
    {
        return "S";
    }

    if (heading_tenths < 2475)
    {
        return "SW";
    }

    if (heading_tenths < 2925)
    {
        return "W";
    }

    return "NW";
}

/* -------------------------------------------------------------------------- */
/* Needle vector from calibrated X/Y                                          */
/*                                                                            */
/* Your heading equation is:                                                  */
/*                                                                            */
/*     heading = atan2(Ycorrected, Xcorrected)                                */
/*                                                                            */
/* Therefore:                                                                 */
/*     heading 0 degrees   -> +X -> screen UP                                 */
/*     heading 90 degrees  -> +Y -> screen RIGHT                              */
/*                                                                            */
/* We use the already-calibrated X/Y vector directly instead of calling       */
/* sin() and cos(). A small magnitude approximation keeps needle length       */
/* nearly constant while preserving the angle.                               */
/* -------------------------------------------------------------------------- */

static void Compass_CalculateNeedleVector(
    float x_corrected,
    float y_corrected,
    int16_t *dx,
    int16_t *dy)
{
    float ax;
    float ay;
    float max_axis;
    float min_axis;
    float magnitude_approx;
    float scale;
    float dx_float;
    float dy_float;

    if ((dx == NULL) || (dy == NULL))
    {
        return;
    }

    ax = x_corrected;
    ay = y_corrected;

    if (ax < 0.0f)
    {
        ax = -ax;
    }

    if (ay < 0.0f)
    {
        ay = -ay;
    }

    if (ax > ay)
    {
        max_axis = ax;
        min_axis = ay;
    }
    else
    {
        max_axis = ay;
        min_axis = ax;
    }

    /*
     * Approximation to sqrt(x^2 + y^2):
     * max + 0.375 * min
     *
     * This avoids adding sinf/cosf/sqrtf to the display update.
     */
    magnitude_approx =
        max_axis + (0.375f * min_axis);

    if (magnitude_approx < 0.001f)
    {
        *dx = 0;
        *dy = -COMPASS_NEEDLE_FRONT;
        return;
    }

    scale =
        (float)COMPASS_NEEDLE_FRONT /
        magnitude_approx;

    /*
     * Screen coordinates:
     *   +X screen = right
     *   +Y screen = down
     *
     * Compass:
     *   +X magnetic = North/up
     *   +Y magnetic = East/right
     */
    dx_float = y_corrected * scale;
    dy_float = -x_corrected * scale;

    if (dx_float >= 0.0f)
    {
        *dx = (int16_t)(dx_float + 0.5f);
    }
    else
    {
        *dx = (int16_t)(dx_float - 0.5f);
    }

    if (dy_float >= 0.0f)
    {
        *dy = (int16_t)(dy_float + 0.5f);
    }
    else
    {
        *dy = (int16_t)(dy_float - 0.5f);
    }
}

/* -------------------------------------------------------------------------- */
/* Draw one two-color compass needle                                          */
/* -------------------------------------------------------------------------- */

static void Compass_DrawNeedleVector(
    int16_t dx,
    int16_t dy,
    ILI9163C_color_18_t *frontColor,
    ILI9163C_color_18_t *rearColor,
    bool drawHub)
{
    hyperdisplay_t *hd = TFT_GetHyperDisplay();

    int16_t rearDx;
    int16_t rearDy;

    rearDx =
        (int16_t)(-((int32_t)dx * COMPASS_NEEDLE_REAR) /
                  COMPASS_NEEDLE_FRONT);

    rearDy =
        (int16_t)(-((int32_t)dy * COMPASS_NEEDLE_REAR) /
                  COMPASS_NEEDLE_FRONT);

    /*
     * Rear half.
     */
    (void)hyperdisplay_line(
        hd,
        COMPASS_CENTER_X,
        COMPASS_CENTER_Y,
        COMPASS_CENTER_X + rearDx,
        COMPASS_CENTER_Y + rearDy,
        COMPASS_NEEDLE_WIDTH,
        (color_t)rearColor,
        1U,
        0U,
        false);

    /*
     * Front / heading half.
     */
    (void)hyperdisplay_line(
        hd,
        COMPASS_CENTER_X,
        COMPASS_CENTER_Y,
        COMPASS_CENTER_X + dx,
        COMPASS_CENTER_Y + dy,
        COMPASS_NEEDLE_WIDTH,
        (color_t)frontColor,
        1U,
        0U,
        false);

    if (drawHub)
    {
        /*
         * White center hub.
         */
        hyperdisplay_circle(
            hd,
            COMPASS_CENTER_X,
            COMPASS_CENTER_Y,
            3,
            true,
            (color_t)&g_colorWhite,
            1U,
            0U,
            false);

        /*
         * Black center dot.
         */
        hyperdisplay_circle(
            hd,
            COMPASS_CENTER_X,
            COMPASS_CENTER_Y,
            1,
            true,
            (color_t)&g_colorBlack,
            1U,
            0U,
            false);
    }
}

/* -------------------------------------------------------------------------- */
/* Erase previous moving needle                                               */
/* -------------------------------------------------------------------------- */

static void Compass_ErasePreviousNeedle(void)
{
    if (!g_previousNeedleValid)
    {
        return;
    }

    Compass_DrawNeedleVector(
        g_previousNeedleDx,
        g_previousNeedleDy,
        &g_colorBlack,
        &g_colorBlack,
        false);
}

/* -------------------------------------------------------------------------- */
/* Update heading text                                                        */
/* -------------------------------------------------------------------------- */

static void Compass_DrawHeadingText(
    int heading_tenths)
{
    hyperdisplay_t *hd = TFT_GetHyperDisplay();
    char headingText[6];
    const char *direction;

    Compass_ClearHeadingArea();

    Compass_FormatHeading(
        heading_tenths,
        headingText);

    direction =
        Compass_GetDirection(
            heading_tenths);

    TFT_SetTextColor(&g_colorGreen);

    /*
     * Fixed-width heading "000.0" through "359.9".
     */
    TFT_PrintString(
        49,
        118,
        headingText);

    /*
     * Small drawn degree symbol.
     */
    hyperdisplay_circle(
        hd,
        82,
        119,
        2,
        false,
        (color_t)&g_colorGreen,
        1U,
        0U,
        false);

    /*
     * Cardinal/intercardinal direction.
     * Two characters fit comfortably with the existing 5x7 font.
     */
    if (direction[1] == '\0')
    {
        TFT_PrintString(
            61,
            134,
            direction);
    }
    else
    {
        TFT_PrintString(
            58,
            134,
            direction);
    }
}

/* -------------------------------------------------------------------------- */
/* Update live compass display                                                */
/* -------------------------------------------------------------------------- */

static void Compass_UpdateDisplay(
    float x_corrected,
    float y_corrected,
    int heading_tenths)
{
    int16_t dx;
    int16_t dy;

    Compass_CalculateNeedleVector(
        x_corrected,
        y_corrected,
        &dx,
        &dy);

    /*
     * Remove only the previous needle.
     * The needle is intentionally short enough that this does not touch
     * the green outer scale or N/E/S/W labels.
     */
    Compass_ErasePreviousNeedle();

    /*
     * Draw new red/white needle.
     */
    Compass_DrawNeedleVector(
        dx,
        dy,
        &g_colorRed,
        &g_colorWhite,
        true);

    g_previousNeedleDx = dx;
    g_previousNeedleDy = dy;
    g_previousNeedleValid = true;

    /*
     * Update only the lower numerical readout.
     */
    Compass_DrawHeadingText(
        heading_tenths);
}

/* -------------------------------------------------------------------------- */
/* Standalone status messages for battery-powered testing                     */
/* -------------------------------------------------------------------------- */

static void Compass_ShowStatus(
    const char *text,
    bool isError)
{
    Compass_ClearHeadingArea();

    if (isError)
    {
        TFT_SetTextColor(&g_colorRed);
    }
    else
    {
        TFT_SetTextColor(&g_colorGreen);
    }

    /*
     * Status text begins at x=43 so strings such as "CALIB" or "MAG ERR"
     * fit in the lower area using the 5x7 font.
     */
    TFT_PrintString(
        43,
        128,
        text);
}

/* -------------------------------------------------------------------------- */
/* main                                                                       */
/* -------------------------------------------------------------------------- */

int main(void)
{
    compass_t compass;
    compass_data_t data;

    ILI9163C_STAT_t tftStatus;

    float x_corrected;
    float y_corrected;
    float heading_deg;

    int heading_tenths;

    char x_sign;
    char y_sign;
    char z_sign;
    char xc_sign;
    char yc_sign;

    int32_t x_comp_mag_mg;
    int32_t y_comp_mag_mg;
    int32_t z_comp_mag_mg;

    int32_t x_corrected_thousandths;
    int32_t y_corrected_thousandths;

    int32_t x_bridge_offset_mg;
    int32_t y_bridge_offset_mg;
    int32_t z_bridge_offset_mg;

    /* ---------------------------------------------------------------------- */
    /* KL25Z board initialization                                             */
    /* ---------------------------------------------------------------------- */

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /*
     * Explicitly configure both peripheral pin groups after BOARD_InitBootPins
     * so this merged project does not depend on the old project's pin_mux.c.
     */
    TFT_InitPins();
    APP_InitI2CPins();

    TFT_InitSPI();
    APP_InitI2C0();

    PRINTF("\r\n");
    PRINTF("====================================================\r\n");
    PRINTF(" FRDM-KL25Z MMC5983MA + TFT Compass\r\n");
    PRINTF("====================================================\r\n");
    PRINTF("TFT: PTD1=SCK PTD2=MOSI PTA17=CS PTA16=D/C\r\n");
    PRINTF("I2C0: PTC8=SCL, PTC9=SDA\r\n");
    PRINTF("MMC5983MA address: 0x30\r\n");
    PRINTF("\r\n");

    /* ---------------------------------------------------------------------- */
    /* Initialize TFT                                                        */
    /* ---------------------------------------------------------------------- */

    KWH018ST01_4WSPI_init(
        &g_tft);

    tftStatus =
        KWH018ST01_4WSPI_begin(
            &g_tft,

            TFT_SPI_BASE,

            TFT_DC_GPIO,
            TFT_DC_PIN,

            TFT_CS_GPIO,
            TFT_CS_PIN,

            NULL,
            NULL,

            TFT_SPI_BAUDRATE_HZ);

    if (tftStatus != ILI9163C_STAT_Nominal)
    {
        PRINTF("TFT initialization FAILED.\r\n");

        while (1)
        {
        }
    }

    /*
     * Create persistent colors.
     */
    g_colorGreen =
        ILI9163C_rgbTo18b(
            80U,
            255U,
            40U);

    g_colorRed =
        ILI9163C_rgbTo18b(
            255U,
            30U,
            20U);

    g_colorWhite =
        ILI9163C_rgbTo18b(
            255U,
            255U,
            255U);

    g_colorBlack =
        ILI9163C_rgbTo18b(
            0U,
            0U,
            0U);

    PRINTF("TFT initialization OK\r\n");

    /*
     * Draw the static green face once.
     */
    Compass_DrawFace();

    /*
     * When running from VIN with no laptop, this gives visible feedback that
     * the firmware has booted and is calibrating the magnetometer.
     */
    Compass_ShowStatus(
        "CALIB",
        false);

    /* ---------------------------------------------------------------------- */
    /* Initialize MMC5983MA                                                   */
    /* ---------------------------------------------------------------------- */

    COMPASS_Init(
        &compass,
        COMPASS_I2C_BASE);

    PRINTF("Starting MMC5983MA...\r\n");

    if (!COMPASS_Begin(&compass))
    {
        PRINTF("COMPASS_Begin FAILED\r\n");
        PRINTF(
            "Last I2C status = %d\r\n",
            (int)COMPASS_LastI2CStatus(&compass));

        Compass_ShowStatus(
            "MAG ERR",
            true);

        while (1)
        {
        }
    }

    PRINTF("COMPASS_Begin OK\r\n");
    PRINTF(
        "Product ID = 0x%02X\r\n",
        compass.product_id);

    if (compass.product_id !=
        COMPASS_PRODUCT_ID_EXPECTED)
    {
        PRINTF("Unexpected Product ID\r\n");

        Compass_ShowStatus(
            "ID ERR",
            true);

        while (1)
        {
        }
    }

    /* ---------------------------------------------------------------------- */
    /* Magnetic measurement bandwidth                                        */
    /* ---------------------------------------------------------------------- */

    if (!COMPASS_SetBandwidth(
            &compass,
            kCompassBandwidth100Hz))
    {
        PRINTF("Bandwidth configuration FAILED\r\n");

        Compass_ShowStatus(
            "BW ERR",
            true);

        while (1)
        {
        }
    }

    PRINTF("Bandwidth: 100 Hz setting\r\n");

    /* ---------------------------------------------------------------------- */
    /* SET/RESET bridge-offset calibration                                    */
    /* ---------------------------------------------------------------------- */

    PRINTF("\r\n");
    PRINTF("Running SET/RESET bridge-offset calibration...\r\n");
    PRINTF("Keep the sensor stationary during this step.\r\n");

    if (!COMPASS_CalibrateBridgeOffset(&compass))
    {
        PRINTF("Bridge-offset calibration FAILED\r\n");
        PRINTF(
            "Last I2C status = %d\r\n",
            (int)COMPASS_LastI2CStatus(&compass));

        Compass_ShowStatus(
            "CAL ERR",
            true);

        while (1)
        {
        }
    }

    x_bridge_offset_mg =
        (int32_t)(
            compass.bridge_offset_gauss[0] *
            1000.0f);

    y_bridge_offset_mg =
        (int32_t)(
            compass.bridge_offset_gauss[1] *
            1000.0f);

    z_bridge_offset_mg =
        (int32_t)(
            compass.bridge_offset_gauss[2] *
            1000.0f);

    PRINTF("Bridge-offset calibration OK\r\n");
    PRINTF("Bridge offsets:\r\n");
    PRINTF(
        "  X = %d mG\r\n",
        (int)x_bridge_offset_mg);
    PRINTF(
        "  Y = %d mG\r\n",
        (int)y_bridge_offset_mg);
    PRINTF(
        "  Z = %d mG\r\n",
        (int)z_bridge_offset_mg);

    PRINTF("\r\n");
    PRINTF("Four-point 2D calibration loaded:\r\n");
    PRINTF("  X bias   = -25.5 mG\r\n");
    PRINTF("  Y bias   = +137.5 mG\r\n");
    PRINTF("  X radius = 236.5 mG\r\n");
    PRINTF("  Y radius = 247.5 mG\r\n");
    PRINTF("\r\n");
    PRINTF("Starting calibrated heading measurements...\r\n");
    PRINTF("\r\n");

    /*
     * Remove CALIB status before first live update.
     */
    Compass_ClearHeadingArea();

    /* ---------------------------------------------------------------------- */
    /* Main measurement / display loop                                        */
    /* ---------------------------------------------------------------------- */

    while (1)
    {
        if (!COMPASS_Measure(
                &compass,
                &data))
        {
            PRINTF(
                "Measurement FAILED. I2C status=%d\r\n",
                (int)COMPASS_LastI2CStatus(&compass));

            Compass_ShowStatus(
                "I2C ERR",
                true);

            APP_DelayUs(
                100000U);

            continue;
        }

        /*
         * FOUR-POINT 2D CALIBRATION
         *
         * compass.c has already removed the internal SET/RESET bridge offset.
         * Now remove environmental X/Y bias and normalize the two radii.
         */
        x_corrected =
            (data.x_gauss - COMPASS_X_BIAS_G) /
            COMPASS_X_RADIUS_G;

        y_corrected =
            (data.y_gauss - COMPASS_Y_BIAS_G) /
            COMPASS_Y_RADIUS_G;

        /*
         * Same known-good heading equation from the compass-only project.
         */
        heading_deg =
            atan2f(
                y_corrected,
                x_corrected) *
            RAD_TO_DEG;

        if (heading_deg < 0.0f)
        {
            heading_deg += 360.0f;
        }

        if (heading_deg >= 360.0f)
        {
            heading_deg -= 360.0f;
        }

        heading_tenths =
            APP_HeadingToTenths(
                heading_deg);

        /*
         * Update the TFT from the exact same calibrated X/Y vector used by
         * the heading equation.
         */
        Compass_UpdateDisplay(
            x_corrected,
            y_corrected,
            heading_tenths);

        /* ------------------------------------------------------------------ */
        /* Preserve the useful compass terminal diagnostics                   */
        /* ------------------------------------------------------------------ */

        x_sign =
            APP_GetFloatSign(
                data.x_gauss);

        y_sign =
            APP_GetFloatSign(
                data.y_gauss);

        z_sign =
            APP_GetFloatSign(
                data.z_gauss);

        x_comp_mag_mg =
            APP_FloatMagnitudeToMilliUnits(
                data.x_gauss);

        y_comp_mag_mg =
            APP_FloatMagnitudeToMilliUnits(
                data.y_gauss);

        z_comp_mag_mg =
            APP_FloatMagnitudeToMilliUnits(
                data.z_gauss);

        xc_sign =
            APP_GetFloatSign(
                x_corrected);

        yc_sign =
            APP_GetFloatSign(
                y_corrected);

        x_corrected_thousandths =
            APP_NormalizedToThousandths(
                x_corrected);

        y_corrected_thousandths =
            APP_NormalizedToThousandths(
                y_corrected);

        PRINTF(
            "Comp X=%c%d mG Y=%c%d mG Z=%c%d mG | "
            "Cal X=%c%d Y=%c%d | "
            "Heading=%d.%d deg\r\n",

            x_sign,
            (int)x_comp_mag_mg,

            y_sign,
            (int)y_comp_mag_mg,

            z_sign,
            (int)z_comp_mag_mg,

            xc_sign,
            (int)x_corrected_thousandths,

            yc_sign,
            (int)y_corrected_thousandths,

            heading_tenths / 10,
            heading_tenths % 10);

        /*
         * 10 Hz display / measurement update.
         */
        APP_DelayUs(
            100000U);
    }
}
