/*
 * Copyright 2016-2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    MKL25Z4_Project_TMP102_Sensor.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
#include "tmp102.h"
#include "fsl_i2c.h"
#include <math.h>

#define ISNAN(x) ((x) != (x))

void delay_us(uint32_t us) {
    uint32_t cycles = (SystemCoreClock / 1000000) * us / 5;
    while (cycles--) __NOP();
}

static I2C_Type *i2c_base = NULL;
static uint8_t tmp102_address = 0x48; // Default address

// Internal function to write to a TMP102 register
static status_t TMP102_WriteRegister(uint8_t reg, uint8_t *data, size_t len) {
    i2c_master_transfer_t xfer = {
        .slaveAddress = tmp102_address,
        .direction = kI2C_Write,
        .subaddress = reg,
        .subaddressSize = 1,
        .data = data,
        .dataSize = len,
        .flags = kI2C_TransferDefaultFlag
    };
    return I2C_MasterTransferBlocking(i2c_base, &xfer);
}

// Internal function to read from a TMP102 register
static status_t TMP102_ReadRegister(uint8_t reg, uint8_t *data, size_t len) {
    i2c_master_transfer_t xfer = {
        .slaveAddress = tmp102_address,
        .direction = kI2C_Read,
        .subaddress = reg,
        .subaddressSize = 1,
        .data = data,
        .dataSize = len,
        .flags = kI2C_TransferDefaultFlag
    };
    return I2C_MasterTransferBlocking(i2c_base, &xfer);
}

void TMP102_Init(I2C_Type *base, uint8_t address) {
    i2c_base = base;
    tmp102_address = address;
}

float TMP102_ReadTempC(void) {
    uint8_t rawData[2] = {0};
    if (TMP102_ReadRegister(0x00, rawData, 2) != kStatus_Success)
        return NAN;

    int16_t tempRaw = ((rawData[0] << 4) | (rawData[1] >> 4));
    // Handle negative temperature (12-bit two's complement)
    if (tempRaw & 0x800) {
        tempRaw |= 0xF000;
    }
    return tempRaw * 0.0625f;
}

float TMP102_ReadTempF(void) {
    float c = TMP102_ReadTempC();
    return ISNAN(c) ? NAN : (c * 9.0f / 5.0f + 32.0f);
}

bool TMP102_IsAlertActive(void) {
    uint8_t config[2] = {0};
    if (TMP102_ReadRegister(0x01, config, 2) != kStatus_Success)
        return false;
    return (config[1] & 0x20) >> 5;
}

void TMP102_SetConversionRate(uint8_t rate) {
    uint8_t config[2];
    if (TMP102_ReadRegister(0x01, config, 2) != kStatus_Success)
        return;

    config[1] &= 0x3F;
    config[1] |= (rate & 0x03) << 6;
    TMP102_WriteRegister(0x01, config, 2);
}

void TMP102_SetExtendedMode(bool enable) {
    uint8_t config[2];
    if (TMP102_ReadRegister(0x01, config, 2) != kStatus_Success)
        return;

    config[1] &= ~(1 << 4);
    config[1] |= (enable ? 1 : 0) << 4;
    TMP102_WriteRegister(0x01, config, 2);
}

int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    i2c_master_config_t config;
    I2C_MasterGetDefaultConfig(&config);
    I2C_MasterInit(I2C1, &config, CLOCK_GetFreq(I2C1_CLK_SRC));

    // NOTE: PTC1 = I2C1_SCL, PTC2 = I2C1_SDA on KL25Z
    TMP102_Init(I2C1, 0x48);

    while (1) {
        uint8_t rawData[2] = {0};
        status_t status = TMP102_ReadRegister(0x00, rawData, 2);
        PRINTF("I2C Status: %d, Raw: 0x%02X 0x%02X\r\n", status, rawData[0], rawData[1]);

        if (status != kStatus_Success) {
            PRINTF("Failed to read TMP102 sensor!\r\n");
        } else {
            int16_t tempRaw = ((rawData[0] << 4) | (rawData[1] >> 4));
            if (tempRaw & 0x800) {
                tempRaw |= 0xF000;
            }
            float tempC = tempRaw * 0.0625f;
            float tempF = tempC * 9.0f / 5.0f + 32.0f;
            //PRINTF("Temperature: %.2f C / %.2f F\r\n", tempC, tempF);
            PRINTF("TempC=%d, TempF=%d\r\n", (int)(tempC * 100), (int)(tempF * 100));
        }

        delay_us(1000000); // 1 second delay
    }
}
