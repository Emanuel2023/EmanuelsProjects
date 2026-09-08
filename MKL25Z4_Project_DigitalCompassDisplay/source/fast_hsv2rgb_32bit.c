/*
 * fast_hsv2rgb_32bit.c
 *
 * Pure-C port for FRDM-KL25Z / MKL25Z4
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

#include "fast_hsv2rgb.h"


void fast_hsv2rgb_32bit(
    uint16_t h,
    uint8_t s,
    uint8_t v,
    uint8_t *r,
    uint8_t *g,
    uint8_t *b)
{
    uint8_t sextant;
    uint8_t h_fraction;

    uint16_t ww;
    uint32_t d;


    /*
     * If saturation is zero, the result is grayscale:
     *
     * R = G = B = V
     */
    HSV_MONOCHROMATIC_TEST(s, v, r, g, b);


    /*
     * Hue is divided into six sextants.
     *
     * Each sextant contains 256 hue values.
     */
    sextant = (uint8_t)(h >> 8);


    /*
     * Optional bounds check.
     *
     * This macro does nothing unless HSV_USE_SEXTANT_TEST
     * is enabled in fast_hsv2rgb.h.
     */
    HSV_SEXTANT_TEST(sextant);


    /*
     * Rearrange RGB pointers depending on the hue sextant.
     */
    HSV_POINTER_SWAP(sextant, r, g, b);


    /*
     * Highest RGB component.
     */
    *g = v;


    /*
     * ------------------------------------------------------------
     * Bottom RGB level
     *
     * v * (1.0 - s)
     *
     * Integer approximation:
     *
     * (v * (255 - s) + correction) / 256
     * ------------------------------------------------------------
     */

    ww =
        (uint16_t)v *
        (uint16_t)(255U - s);


    /*
     * Error correction.
     */
    ww += 1U;

    ww += (ww >> 8);


    *b = (uint8_t)(ww >> 8);


    /*
     * Position within the active hue sextant.
     *
     * Range:
     * 0 through 255
     */
    h_fraction =
        (uint8_t)(h & 0x00FFU);


    if ((sextant & 0x01U) == 0U)
    {
        /*
         * --------------------------------------------------------
         * Rising slope
         *
         * d =
         * v * ((255 << 8) -
         *      (s * (256 - h_fraction)))
         * --------------------------------------------------------
         */

        d =
            (uint32_t)v *
            (
                ((uint32_t)255U << 8) -
                (
                    (uint32_t)s *
                    (uint32_t)(256U - h_fraction)
                )
            );


        /*
         * Error corrections.
         */
        d += (d >> 8);

        d += (uint32_t)v;


        /*
         * Convert result back to 8-bit RGB.
         */
        *r =
            (uint8_t)(d >> 16);
    }
    else
    {
        /*
         * --------------------------------------------------------
         * Falling slope
         *
         * d =
         * v * ((255 << 8) -
         *      (s * h_fraction))
         * --------------------------------------------------------
         */

        d =
            (uint32_t)v *
            (
                ((uint32_t)255U << 8) -
                (
                    (uint32_t)s *
                    (uint32_t)h_fraction
                )
            );


        /*
         * Error corrections.
         */
        d += (d >> 8);

        d += (uint32_t)v;


        /*
         * Convert result back to 8-bit RGB.
         */
        *r =
            (uint8_t)(d >> 16);
    }
}
