/*
 * fast_hsv2rgb.h
 *
 * Pure-C version for FRDM-KL25Z / MCUXpresso
 *
 * Original:
 * Copyright (c) 2016 B. Stultiens
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 */

#ifndef FAST_HSV2RGB_H_
#define FAST_HSV2RGB_H_

#include <stdint.h>


/* ==========================================================================
 * HSV ranges
 * ========================================================================== */

#define HSV_HUE_SEXTANT     256U
#define HSV_HUE_STEPS       (6U * HSV_HUE_SEXTANT)

#define HSV_HUE_MIN         0U
#define HSV_HUE_MAX         (HSV_HUE_STEPS - 1U)

#define HSV_SAT_MIN         0U
#define HSV_SAT_MAX         255U

#define HSV_VAL_MIN         0U
#define HSV_VAL_MAX         255U


/* ==========================================================================
 * Optional configuration
 * ========================================================================== */

/*
 * Uncomment to force the hue sextant to remain within the valid
 * range of 0 through 5.
 */

/* #define HSV_USE_SEXTANT_TEST */


/*
 * AVR assembly optimization from the original library is intentionally
 * NOT used on the KL25Z.
 *
 * The MKL25Z4 uses an ARM Cortex-M0+, so AVR instructions such as:
 *
 *      mul
 *      movw
 *      clr
 *      sbrc
 *      adc
 *      lsr
 *      brne
 *
 * are not compatible.
 *
 * The portable C implementation will be used instead.
 */


/* ==========================================================================
 * Function prototypes
 * ========================================================================== */

/*
 * Convert HSV to 8-bit RGB.
 *
 * h:
 *      0 to HSV_HUE_MAX
 *
 * s:
 *      0 to 255
 *
 * v:
 *      0 to 255
 *
 * r, g, b:
 *      Output RGB values from 0 to 255
 */
void fast_hsv2rgb_8bit(
    uint16_t h,
    uint8_t s,
    uint8_t v,
    uint8_t *r,
    uint8_t *g,
    uint8_t *b);


/*
 * Convert HSV to RGB using the 32-bit intermediate implementation.
 */
void fast_hsv2rgb_32bit(
    uint16_t h,
    uint8_t s,
    uint8_t v,
    uint8_t *r,
    uint8_t *g,
    uint8_t *b);


/* ==========================================================================
 * Common implementation macros
 * ========================================================================== */

/*
 * If saturation is zero, the color is monochromatic.
 *
 * Therefore:
 *
 *      R = G = B = V
 */
#define HSV_MONOCHROMATIC_TEST(s, v, r, g, b) \
    do                                         \
    {                                          \
        if (!(s))                              \
        {                                      \
            *(r) = (v);                        \
            *(g) = (v);                        \
            *(b) = (v);                        \
            return;                            \
        }                                      \
    } while (0)


/*
 * Optionally constrain the calculated sextant.
 */
#ifdef HSV_USE_SEXTANT_TEST

#define HSV_SEXTANT_TEST(sextant) \
    do                            \
    {                             \
        if ((sextant) > 5U)       \
        {                         \
            (sextant) = 5U;       \
        }                         \
    } while (0)

#else

#define HSV_SEXTANT_TEST(sextant) \
    do                            \
    {                             \
        (void)(sextant);          \
    } while (0)

#endif


/* ==========================================================================
 * Pointer swapping
 * ========================================================================== */

/*
 * Swap two uint8_t pointers.
 */
#define HSV_SWAPPTR(a, b)            \
    do                               \
    {                                \
        uint8_t *tmp = (a);          \
        (a) = (b);                   \
        (b) = tmp;                   \
    } while (0)


/*
 * Rearrange the RGB output pointers according to the hue sextant.
 *
 * Original lookup:
 *
 * sextant     r g b
 * -----------------
 * 0           v u c
 * 1           d v c
 * 2           c v u
 * 3           c d v
 * 4           u c v
 * 5           v c d
 */
#define HSV_POINTER_SWAP(sextant, r, g, b) \
    do                                     \
    {                                      \
        if ((sextant) & 2U)                \
        {                                  \
            HSV_SWAPPTR((r), (b));         \
        }                                  \
                                           \
        if ((sextant) & 4U)                \
        {                                  \
            HSV_SWAPPTR((g), (b));         \
        }                                  \
                                           \
        if (!((sextant) & 6U))             \
        {                                  \
            if (!((sextant) & 1U))         \
            {                              \
                HSV_SWAPPTR((r), (g));     \
            }                              \
        }                                  \
        else                               \
        {                                  \
            if ((sextant) & 1U)            \
            {                              \
                HSV_SWAPPTR((r), (g));     \
            }                              \
        }                                  \
    } while (0)


#endif /* FAST_HSV2RGB_H_ */
