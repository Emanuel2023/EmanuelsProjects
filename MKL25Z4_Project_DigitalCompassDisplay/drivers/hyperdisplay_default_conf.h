/*
 * hyperdisplay_default_conf.h
 *
 * Configuration for SparkFun HyperDisplay
 * Ported for the FRDM-KL25Z / MCUXpresso SDK
 *
 * This configuration keeps the general HyperDisplay drawing
 * and text functionality enabled while removing Arduino/C++
 * dependencies that are not currently part of the KL25Z port.
 */

#ifndef HYPERDISPLAY_DEFAULT_CONF_H_
#define HYPERDISPLAY_DEFAULT_CONF_H_


/*
 * DRAWING_LEVEL
 *
 * 0 - Primitive drawing functions:
 *     pixel
 *     xline
 *     yline
 *     rectangle
 *     fillFromArray
 *     fillWindow
 *
 * 1 - Level 0 plus:
 *     line
 *     polygon
 *     circle
 *
 * Higher values retain all drawing functionality enabled
 * by the original HyperDisplay configuration.
 */
#define HYPERDISPLAY_DRAWING_LEVEL 4


/*
 * USE_PRINT
 *
 * 0 - Disable text printing
 * 1 - Enable text printing
 *
 * Enabled so HyperDisplay retains text support.
 */
#define HYPERDISPLAY_USE_PRINT 1


/*
 * INCLUDE_DEFAULT_FONT
 *
 * 0 - Do not include the default 5x7 font
 * 1 - Include the default 5x7 font
 *
 * Enabled so the display can use HyperDisplay's default
 * text font.
 *
 * The font itself will be converted from the Arduino/AVR
 * representation to a normal KL25Z const data array.
 */
#define HYPERDISPLAY_INCLUDE_DEFAULT_FONT 1


/*
 * USE_MATH
 *
 * 0 - Disable HyperDisplay mathematical extensions
 * 1 - Enable HyperDisplay mathematical extensions
 *
 * The original configuration has this disabled, so we
 * preserve that configuration for the KL25Z port.
 */
#define HYPERDISPLAY_USE_MATH 0


/*
 * USE_RAY_TRACING
 *
 * 0 - Disable SparkFun 2D Ray Tracing
 * 1 - Enable SparkFun 2D Ray Tracing
 *
 * Disabled because SparkFun_2DRayTracing is a separate
 * C++ library and has not been ported to pure C for the
 * KL25Z.
 *
 * Core HyperDisplay operation does NOT require it.
 */
#define HYPERDISPLAY_USE_RAY_TRACING 0


#endif /* HYPERDISPLAY_DEFAULT_CONF_H_ */
