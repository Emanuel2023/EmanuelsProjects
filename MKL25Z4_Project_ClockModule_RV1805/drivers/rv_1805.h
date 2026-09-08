/*
 * rv_1805.h
 *
 * Created on: Jul 29, 2025
 * Author: KC
 */

#ifndef RV_1805_H_
#define RV_1805_H_

// Include necessary headers if not already present (e.g., for I2C0 definition)
// #include "fsl_i2c.h" // This might be needed depending on your SDK setup for I2C0, I2C1 definitions.
// #include "fsl_common.h" // For basic types like bool, uint8_t, etc.

#define RV1805_ADDR 0x69

// I2C specific defines for RV1805 communication
// Assuming you are using I2C0 on MKL25Z4 and a standard 100 kHz baud rate.
// Adjust I2C0 to I2C1 if your pin configuration uses I2C1.
#define RV1805_I2C_BASE                 I2C0        // Define the I2C peripheral base address (e.g., I2C0)
#define RV1805_I2C_BAUD                 100000U     // Define the I2C bus speed (e.g., 100 kHz)

// Length of time array for registers
#define TIME_ARRAY_LENGTH 8

// Register addresses (example, add more as needed)
#define RV1805_ID0              0x00
#define RV1805_CONF_KEY         0x26
#define RV1805_CONF_WRT         0xA1
#define RV1805_CAP_RC           0x27
#define RV1805_CTRL1            0x01
#define RV1805_HOURS            0x02
#define RV1805_STATUS           0x03
#define RV1805_HUNDREDTHS       0x08
#define RV1805_SECONDS          0x09
#define RV1805_MINUTES          0x0A
#define RV1805_DATE             0x0B
#define RV1805_MONTHS           0x0C
#define RV1805_YEAR             0x0D
#define RV1805_DAY              0x0E

// Bit positions (examples)
#define CTRL1_12_24             1
#define HOURS_AM_PM             5

// Interrupt sources
#define INTERRUPT_BLIE          4
#define INTERRUPT_TIE           3
#define INTERRUPT_AIE           2
#define INTERRUPT_EIE           1

// Define any other necessary constants/macros here...

// RV1805 device context struct
typedef struct
{
    void* i2c_handle;   // Pointer to I2C peripheral handle (depends on SDK)
    uint8_t time[TIME_ARRAY_LENGTH]; // Time buffer for RTC registers
} RV1805_t;

// Initialization and configuration
bool RV1805_begin(RV1805_t* dev, void* i2c_handle);
bool RV1805_setToCompilerTime(RV1805_t* dev);
bool RV1805_updateTime(RV1805_t* dev);
char* RV1805_stringTime(RV1805_t* dev);

// Time setters and getters
bool RV1805_setTime(RV1805_t* dev, uint8_t hund, uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month, uint16_t year, uint8_t day);
uint8_t RV1805_getHours(RV1805_t* dev);
uint8_t RV1805_getMinutes(RV1805_t* dev);
uint8_t RV1805_getSeconds(RV1805_t* dev);

// Register read/write
uint8_t RV1805_readRegister(RV1805_t* dev, uint8_t reg);
bool RV1805_writeRegister(RV1805_t* dev, uint8_t reg, uint8_t val);
bool RV1805_readMultipleRegisters(RV1805_t* dev, uint8_t reg, uint8_t* buffer, uint8_t len);
bool RV1805_writeMultipleRegisters(RV1805_t* dev, uint8_t reg, uint8_t* data, uint8_t len);

//status_t rv1805_get_hours(I2C_Type *base, uint8_t *hour);
//status_t rv1805_get_minutes(I2C_Type *base, uint8_t *minute);
// Utility functions
uint8_t RV1805_BCDtoDEC(uint8_t val);
uint8_t RV1805_DECtoBCD(uint8_t val);

#endif /* RV_1805_H_ */
