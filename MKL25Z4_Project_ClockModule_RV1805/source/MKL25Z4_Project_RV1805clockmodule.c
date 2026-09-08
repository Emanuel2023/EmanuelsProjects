/*
 * Copyright 2016-2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    MKL25Z4_Project_RV1805clockmodule.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include <time.h>
#include "rv_1805.h"
#include "fsl_gpio.h"

#define RV1805_ADDR                     (0x69U) // 7-bit I2C address

//===========================
// Part number
//===========================
#define RV1805_PART_NUMBER_UPPER        0x18

//===========================
// CONFKEY values
//===========================
#define RV1805_CONF_RST                 0x3C
#define RV1805_CONF_OSC                 0xA1
#define RV1805_CONF_WRT                 0x9D

//===========================
// CTRL1 bits / fields
//===========================
#define CTRL1_STOP      7
#define CTRL1_12_24     6
#define CTRL1_PSWB      5
#define CTRL1_RSTP      3
// Note: original header has "CTRL1_ARST 1 << 2". Keep the semantics:
#define CTRL1_ARST                      (1U << 2)

//===========================
// Hours register bits
//===========================
#define HOURS_AM_PM                     5

//===========================
// Trickle charge fields
//===========================
#define TRICKLE_CHARGER_TCS_OFFSET      4
#define TRICKLE_CHARGER_DIODE_OFFSET    2
#define TRICKLE_CHARGER_ROUT_OFFSET     0
#define TRICKLE_ENABLE                  0b1010
#define TRICKLE_DISABLE                 0b0000
#define DIODE_DISABLE                   0b00
#define DIODE_0_3V                      0b01
#define DIODE_0_6V                      0b10
#define ROUT_DISABLE                    0b00
#define ROUT_3K                         0b01
#define ROUT_6K                         0b10
#define ROUT_11K                        0b11

//===========================
// Interrupt Enable bits
//===========================
#define INTERRUPT_BLIE  4
#define INTERRUPT_TIE   3
#define INTERRUPT_AIE   2
#define INTERRUPT_EIE   1

//===========================
// PSW Pin Function Selection
//===========================
#define PSW_ON          false
#define PSW_OFF         true
#define PSW_UNLOCK      false
#define PSW_LOCK        true
#define PSWS_OFFSET     2
#define PSWS_INV_IRQ    0b000
#define PSWS_SQW        0b001
#define PSWS_INV_AIRQ   0b011
#define PSWS_TIRQ       0b100
#define PSWS_INV_TIRQ   0b101
#define PSWS_SLEEP      0b110
#define PSWS_STATIC     0b111

//===========================
// Countdown Timer Control
//===========================
#define COUNTDOWN_SECONDS       0b10
#define COUNTDOWN_MINUTES       0b11
#define CTDWN_TMR_TE_OFFSET     7
#define CTDWN_TMR_TM_OFFSET     6
#define CTDWN_TMR_TRPT_OFFSET   5

//===========================
// Status Bits
//===========================
#define STATUS_CB   7
#define STATUS_BAT  6
#define STATUS_WDF  5
#define STATUS_BLF  4
#define STATUS_TF   3
#define STATUS_AF   2
#define STATUS_EVF  1

//===========================
// Reference Voltage
//===========================
#define TWO_FIVE                     0x70
#define TWO_ONE                      0xB0
#define ONE_EIGHT                    0xD0
#define ONE_FOUR                     0xF0

//===========================
// Register Map
//===========================
#define RV1805_HUNDREDTHS               0x00
#define RV1805_SECONDS                  0x01
#define RV1805_MINUTES                  0x02
#define RV1805_HOURS                    0x03
#define RV1805_DATE                     0x04
#define RV1805_MONTHS                   0x05
#define RV1805_YEARS                    0x06
#define RV1805_WEEKDAYS                 0x07
#define RV1805_HUNDREDTHS_ALM           0x08
#define RV1805_SECONDS_ALM              0x09
#define RV1805_MINUTES_ALM              0x0A
#define RV1805_HOURS_ALM                0x0B
#define RV1805_DATE_ALM                 0x0C
#define RV1805_MONTHS_ALM               0x0D
#define RV1805_WEEKDAYS_ALM             0x0E
#define RV1805_STATUS                   0x0F
#define RV1805_CTRL1                    0x10
#define RV1805_CTRL2                    0x11
#define RV1805_INT_MASK                 0x12
#define RV1805_SQW                      0x13
#define RV1805_CAL_XT                   0x14
#define RV1805_CAL_RC_UP                0x15
#define RV1805_CAL_RC_LO                0x16
#define RV1805_SLP_CTRL                 0x17
#define RV1805_CTDWN_TMR_CTRL           0x18
#define RV1805_CTDWN_TMR                0x19
#define RV1805_TMR_INITIAL              0x1A
#define RV1805_WATCHDOG_TMR             0x1B
#define RV1805_OSC_CTRL                 0x1C
#define RV1805_OSC_STATUS               0x1D
#define RV1805_CONF_KEY                 0x1F
#define RV1805_TRICKLE_CHRG             0x20
#define RV1805_BREF_CTRL                0x21
#define RV1805_CAP_RC                   0x26
#define RV1805_IOBATMODE                0x27
#define RV1805_ID0                      0x28
#define RV1805_ANLG_STAT                0x2F
#define RV1805_OUT_CTRL                 0x30
#define RV1805_RAM_EXT                  0x3F

//===========================
// Time array ordering
//===========================
#define TIME_ARRAY_LENGTH 8
enum time_order {
    TIME_HUNDREDTHS = 0,
    TIME_SECONDS,     // 1
    TIME_MINUTES,     // 2
    TIME_HOURS,       // 3
    TIME_DATE,        // 4
    TIME_MONTH,       // 5
    TIME_YEAR,        // 6
    TIME_DAY          // 7
};

//===========================
// __DATE__ / __TIME__ parsing
//===========================
// <MONTH>
#define BUILD_MONTH_JAN (((__DATE__[0] == 'J') && (__DATE__[1] == 'a')) ? 1 : 0)
#define BUILD_MONTH_FEB ((__DATE__[0] == 'F') ? 2 : 0)
#define BUILD_MONTH_MAR (((__DATE__[0] == 'M') && (__DATE__[1] == 'a') && (__DATE__[2] == 'r')) ? 3 : 0)
#define BUILD_MONTH_APR (((__DATE__[0] == 'A') && (__DATE__[1] == 'p')) ? 4 : 0)
#define BUILD_MONTH_MAY (((__DATE__[0] == 'M') && (__DATE__[1] == 'a') && (__DATE__[2] == 'y')) ? 5 : 0)
#define BUILD_MONTH_JUN (((__DATE__[0] == 'J') && (__DATE__[1] == 'u') && (__DATE__[2] == 'n')) ? 6 : 0)
#define BUILD_MONTH_JUL (((__DATE__[0] == 'J') && (__DATE__[1] == 'u') && (__DATE__[2] == 'l')) ? 7 : 0)
#define BUILD_MONTH_AUG (((__DATE__[0] == 'A') && (__DATE__[1] == 'u')) ? 8 : 0)
#define BUILD_MONTH_SEP ((__DATE__[0] == 'S') ? 9 : 0)
#define BUILD_MONTH_OCT ((__DATE__[0] == 'O') ? 10 : 0)
#define BUILD_MONTH_NOV ((__DATE__[0] == 'N') ? 11 : 0)
#define BUILD_MONTH_DEC ((__DATE__[0] == 'D') ? 12 : 0)
#define BUILD_MONTH (BUILD_MONTH_JAN | BUILD_MONTH_FEB | BUILD_MONTH_MAR | \
                     BUILD_MONTH_APR | BUILD_MONTH_MAY | BUILD_MONTH_JUN | \
                     BUILD_MONTH_JUL | BUILD_MONTH_AUG | BUILD_MONTH_SEP | \
                     BUILD_MONTH_OCT | BUILD_MONTH_NOV | BUILD_MONTH_DEC)
// <DATE>
#define BUILD_DATE_0 (((__DATE__[4] == ' ') ? 0 : (__DATE__[4] - 0x30)))
#define BUILD_DATE_1 (__DATE__[5] - 0x30)
#define BUILD_DATE   ((BUILD_DATE_0 * 10) + BUILD_DATE_1)
// <YEAR>
#define BUILD_YEAR (((__DATE__[7] - 0x30) * 1000) + ((__DATE__[8] - 0x30) * 100) + \
                    ((__DATE__[9] - 0x30) * 10)  + ((__DATE__[10] - 0x30) * 1))
// <TIME>
#define BUILD_HOUR_0 (((__TIME__[0] == ' ') ? 0 : (__TIME__[0] - 0x30)))
#define BUILD_HOUR_1 (__TIME__[1] - 0x30)
#define BUILD_HOUR   ((BUILD_HOUR_0 * 10) + BUILD_HOUR_1)
#define BUILD_MINUTE_0 (((__TIME__[3] == ' ') ? 0 : (__TIME__[3] - 0x30)))
#define BUILD_MINUTE_1 (__TIME__[4] - 0x30)
#define BUILD_MINUTE   ((BUILD_MINUTE_0 * 10) + BUILD_MINUTE_1)
#define BUILD_SECOND_0 (((__TIME__[6] == ' ') ? 0 : (__TIME__[6] - 0x30)))
#define BUILD_SECOND_1 (__TIME__[7] - 0x30)
#define BUILD_SECOND   ((BUILD_SECOND_0 * 10) + BUILD_SECOND_1)

#define SEG_A     1  // PTD1
#define SEG_B     2  // PTD2
#define SEG_C     5  // PTD5
#define SEG_D     16 // PTA16
#define SEG_E     17 // PTC17
#define SEG_F     3  // PTD3
#define SEG_G     0  // PTD0
#define SEG_DP    17 // PTA17

// Define GPIO pins for digit select
#define DIGIT1    9   // PTC9
#define DIGIT2    8   // PTC8
#define DIGIT3    11  // PTC11
#define DIGIT4    10  // PTC10



//===========================
// Device context
//===========================
typedef struct {
    I2C_Type *i2c;                     // I2C base for KL25Z (e.g., I2C0)
    uint8_t  time_buf[TIME_ARRAY_LENGTH];
} rv1805_t;

//===========================
// Forward declarations (public-like API)
//===========================

// Initialization / status
bool rv1805_begin(rv1805_t *dev, I2C_Type *i2c_base);
void rv1805_reset(rv1805_t *dev);
uint8_t rv1805_status(rv1805_t *dev);

// Time handling
bool rv1805_update_time(rv1805_t *dev);
bool rv1805_set_time_full(rv1805_t *dev, uint8_t hund, uint8_t sec, uint8_t min, uint8_t hour,
                          uint8_t date, uint8_t month, uint16_t year, uint8_t day);
bool rv1805_set_time_array(rv1805_t *dev, uint8_t *time, uint8_t len);

// Individual setters
bool rv1805_set_hundredths(rv1805_t *dev, uint8_t value);
bool rv1805_set_seconds(rv1805_t *dev, uint8_t value);
bool rv1805_set_minutes(rv1805_t *dev, uint8_t value);
bool rv1805_set_hours(rv1805_t *dev, uint8_t value);
bool rv1805_set_date(rv1805_t *dev, uint8_t value);
bool rv1805_set_month(rv1805_t *dev, uint8_t value);
bool rv1805_set_year(rv1805_t *dev, uint8_t value);
bool rv1805_set_weekday(rv1805_t *dev, uint8_t value);

// Getters
uint8_t rv1805_get_hundredths(rv1805_t *dev);
uint8_t rv1805_get_seconds(rv1805_t *dev);
uint8_t rv1805_get_minutes(rv1805_t *dev);
uint8_t rv1805_get_hours(rv1805_t *dev);
uint8_t rv1805_get_weekday(rv1805_t *dev);
uint8_t rv1805_get_date(rv1805_t *dev);
uint8_t rv1805_get_month(rv1805_t *dev);
uint8_t rv1805_get_year(rv1805_t *dev);

// Strings / epoch
char *rv1805_string_date_usa(rv1805_t *dev);
char *rv1805_string_date(rv1805_t *dev);
char *rv1805_string_time(rv1805_t *dev);
char *rv1805_string_timestamp(rv1805_t *dev);
uint32_t rv1805_get_epoch(rv1805_t *dev);

// 12/24 hour helpers
bool rv1805_is_12hour(rv1805_t *dev);
bool rv1805_is_pm(rv1805_t *dev);
void rv1805_set_12hour(rv1805_t *dev);
void rv1805_set_24hour(rv1805_t *dev);

// Alarm
bool rv1805_set_alarm_ymdhms(rv1805_t *dev, uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month);
bool rv1805_set_alarm_array(rv1805_t *dev, uint8_t *alarm_time, uint8_t len);
void rv1805_set_alarm_mode(rv1805_t *dev, uint8_t mode);

// Interrupts
void rv1805_enable_interrupt(rv1805_t *dev, uint8_t source);
void rv1805_disable_interrupt(rv1805_t *dev, uint8_t source);
void rv1805_clear_interrupts(rv1805_t *dev);

// Countdown timer
void rv1805_set_countdown_timer(rv1805_t *dev, uint8_t duration, uint8_t unit, bool repeat, bool pulse);

// Power / trickle / low power
void rv1805_enable_sleep(rv1805_t *dev);
void rv1805_set_power_switch_function(rv1805_t *dev, uint8_t function);
void rv1805_set_power_switch_lock(rv1805_t *dev, bool lock);
void rv1805_set_static_power_switch_output(rv1805_t *dev, bool psw);
void rv1805_enable_trickle_charge(rv1805_t *dev, uint8_t diode, uint8_t rOut);
void rv1805_disable_trickle_charge(rv1805_t *dev);
void rv1805_enable_low_power(rv1805_t *dev);

// Battery / reference voltage
void rv1805_enable_battery_interrupt(rv1805_t *dev, uint8_t voltage, bool edgeTrigger);
bool rv1805_check_battery(rv1805_t *dev, uint8_t voltage);
void rv1805_set_reference_voltage(rv1805_t *dev, uint8_t voltage);
void rv1805_set_edge_trigger(rv1805_t *dev, bool edgeTrigger);

// Compiler time
bool rv1805_set_to_compiler_time(rv1805_t *dev);

// BCD helpers
static inline uint8_t rv1805_BCD_to_DEC(uint8_t v) { return (uint8_t)((v / 0x10u) * 10u + (v % 0x10u)); }
static inline uint8_t rv1805_DEC_to_BCD(uint8_t v) { return (uint8_t)(((v / 10u) * 0x10u) + (v % 10u)); }

//===========================
// Low-level I2C helpers (MCUXpresso SDK)
//===========================
static bool rv1805_write_reg(rv1805_t *dev, uint8_t reg, uint8_t val);
static bool rv1805_write_regs(rv1805_t *dev, uint8_t reg, const uint8_t *data, size_t len);
static bool rv1805_read_reg(rv1805_t *dev, uint8_t reg, uint8_t *val);
static bool rv1805_read_regs(rv1805_t *dev, uint8_t reg, uint8_t *data, size_t len);

//===========================
// Implementation
//===========================

bool rv1805_begin(rv1805_t *dev, I2C_Type *i2c_base)
{
    if (!dev || !i2c_base) return false;
    memset(dev, 0, sizeof(*dev));
    dev->i2c = i2c_base;

    uint8_t id0 = 0xFF;
    if (!rv1805_read_reg(dev, RV1805_ID0, &id0)) return false;
    if (id0 != RV1805_PART_NUMBER_UPPER) return false;

    // Enable write to CAPRC, etc.
    if (!rv1805_write_reg(dev, RV1805_CONF_KEY, RV1805_CONF_WRT)) return false;
    if (!rv1805_write_reg(dev, RV1805_CAP_RC, 0xA0)) return false;

    // Default charger + low power
    rv1805_enable_trickle_charge(dev, DIODE_0_3V, ROUT_3K);
    rv1805_enable_low_power(dev);

    // Auto-reset interrupt flags on status read
    uint8_t ctrl1 = 0;
    rv1805_read_reg(dev, RV1805_CTRL1, &ctrl1);
    ctrl1 |= CTRL1_ARST;
    rv1805_write_reg(dev, RV1805_CTRL1, ctrl1);

    // Default to 12-hour mode (matches original)
    rv1805_set_12hour(dev);

    return true;
}

void rv1805_reset(rv1805_t *dev)
{
    if (!dev) return;
    rv1805_write_reg(dev, RV1805_CONF_KEY, RV1805_CONF_RST);
}

uint8_t rv1805_status(rv1805_t *dev)
{
    uint8_t s = 0xFF;
    if (dev) rv1805_read_reg(dev, RV1805_STATUS, &s);
    return s;
}

bool rv1805_is_12hour(rv1805_t *dev)
{
    if (!dev) return false;
    uint8_t c = 0;
    if (!rv1805_read_reg(dev, RV1805_CTRL1, &c)) return false;
    return (c & (1u << CTRL1_12_24)) != 0u;
}

bool rv1805_is_pm(rv1805_t *dev)
{
    if (!dev) return false;
    uint8_t h = 0;
    if (!rv1805_read_reg(dev, RV1805_HOURS, &h)) return false;
    return (rv1805_is_12hour(dev) && ((h & (1u << HOURS_AM_PM)) != 0u));
}

void rv1805_set_12hour(rv1805_t *dev)
{
    if (!dev) return;
    if (!rv1805_is_12hour(dev)) {
        // read current 24h hour
        uint8_t hr_bcd = 0;
        if (!rv1805_read_reg(dev, RV1805_HOURS, &hr_bcd)) return;
        uint8_t h = rv1805_BCD_to_DEC(hr_bcd);

        // set 12/24 bit
        uint8_t c1 = 0;
        rv1805_read_reg(dev, RV1805_CTRL1, &c1);
        c1 |= (1u << CTRL1_12_24);
        rv1805_write_reg(dev, RV1805_CTRL1, c1);

        bool pm = false;
        if (h == 0) h = 12;
        else if (h == 12) pm = true;
        else if (h > 12) { h -= 12; pm = true; }

        uint8_t new_hr = rv1805_DEC_to_BCD(h);
        if (pm) new_hr |= (1u << HOURS_AM_PM);
        rv1805_write_reg(dev, RV1805_HOURS, new_hr);
    }
}

void rv1805_set_24hour(rv1805_t *dev)
{
    if (!dev) return;
    if (rv1805_is_12hour(dev)) {
        uint8_t hr_bcd = 0;
        if (!rv1805_read_reg(dev, RV1805_HOURS, &hr_bcd)) return;

        bool pm = (hr_bcd & (1u << HOURS_AM_PM)) != 0u;
        hr_bcd &= ~(1u << HOURS_AM_PM);

        uint8_t c1 = 0;
        rv1805_read_reg(dev, RV1805_CTRL1, &c1);
        c1 &= ~(1u << CTRL1_12_24);
        rv1805_write_reg(dev, RV1805_CTRL1, c1);

        uint8_t h = rv1805_BCD_to_DEC(hr_bcd);
        if (pm) h += 12;
        if (h == 12) h = 0;
        if (h == 24) h = 12;

        rv1805_write_reg(dev, RV1805_HOURS, rv1805_DEC_to_BCD(h));
    }
}
bool rv1805_update_time(rv1805_t *dev)
{
    if (!dev) return false;
    if (!rv1805_read_regs(dev, RV1805_HUNDREDTHS, dev->time_buf, TIME_ARRAY_LENGTH)) return false;

    // If in 12h, remove AM/PM bit for the numeric hours in buffer (as original code did)
    if (rv1805_is_12hour(dev)) {
        dev->time_buf[TIME_HOURS] &= ~(1u << HOURS_AM_PM);
    }
    return true;
}

bool rv1805_set_time_array(rv1805_t *dev, uint8_t *time, uint8_t len)
{
    if (!dev || !time || (len != TIME_ARRAY_LENGTH)) return false;
    return rv1805_write_regs(dev, RV1805_HUNDREDTHS, time, len);
}

bool rv1805_set_time_full(rv1805_t *dev, uint8_t hund, uint8_t sec, uint8_t min, uint8_t hour,
                          uint8_t date, uint8_t month, uint16_t year, uint8_t day)
{
    if (!dev) return false;

    dev->time_buf[TIME_HUNDREDTHS] = rv1805_DEC_to_BCD(hund);
    dev->time_buf[TIME_SECONDS]    = rv1805_DEC_to_BCD(sec);
    dev->time_buf[TIME_MINUTES]    = rv1805_DEC_to_BCD(min);
    dev->time_buf[TIME_HOURS]      = rv1805_DEC_to_BCD(hour);
    dev->time_buf[TIME_DATE]       = rv1805_DEC_to_BCD(date);
    dev->time_buf[TIME_MONTH]      = rv1805_DEC_to_BCD(month);
    dev->time_buf[TIME_YEAR]       = rv1805_DEC_to_BCD((uint8_t)(year - 2000));
    dev->time_buf[TIME_DAY]        = rv1805_DEC_to_BCD(day);

    bool ok;
    if (rv1805_is_12hour(dev)) {
        rv1805_set_24hour(dev);
        ok = rv1805_set_time_array(dev, dev->time_buf, TIME_ARRAY_LENGTH);
        rv1805_set_12hour(dev);
    } else {
        ok = rv1805_set_time_array(dev, dev->time_buf, TIME_ARRAY_LENGTH);
    }
    return ok;
}

// Individual setters => update whole array (to match original behavior)
bool rv1805_set_hundredths(rv1805_t *dev, uint8_t value) { dev->time_buf[TIME_HUNDREDTHS] = rv1805_DEC_to_BCD(value); return rv1805_set_time_array(dev, dev->time_buf, TIME_ARRAY_LENGTH); }
bool rv1805_set_seconds   (rv1805_t *dev, uint8_t value) { dev->time_buf[TIME_SECONDS]    = rv1805_DEC_to_BCD(value); return rv1805_set_time_array(dev, dev->time_buf, TIME_ARRAY_LENGTH); }
bool rv1805_set_minutes   (rv1805_t *dev, uint8_t value) { dev->time_buf[TIME_MINUTES]    = rv1805_DEC_to_BCD(value); return rv1805_set_time_array(dev, dev->time_buf, TIME_ARRAY_LENGTH); }
bool rv1805_set_hours     (rv1805_t *dev, uint8_t value) { dev->time_buf[TIME_HOURS]      = rv1805_DEC_to_BCD(value); return rv1805_set_time_array(dev, dev->time_buf, TIME_ARRAY_LENGTH); }
bool rv1805_set_date      (rv1805_t *dev, uint8_t value) { dev->time_buf[TIME_DATE]       = rv1805_DEC_to_BCD(value); return rv1805_set_time_array(dev, dev->time_buf, TIME_ARRAY_LENGTH); }
bool rv1805_set_month     (rv1805_t *dev, uint8_t value) { dev->time_buf[TIME_MONTH]      = rv1805_DEC_to_BCD(value); return rv1805_set_time_array(dev, dev->time_buf, TIME_ARRAY_LENGTH); }
bool rv1805_set_year      (rv1805_t *dev, uint8_t value) { dev->time_buf[TIME_YEAR]       = rv1805_DEC_to_BCD(value); return rv1805_set_time_array(dev, dev->time_buf, TIME_ARRAY_LENGTH); }
bool rv1805_set_weekday   (rv1805_t *dev, uint8_t value) { dev->time_buf[TIME_DAY]        = rv1805_DEC_to_BCD(value); return rv1805_set_time_array(dev, dev->time_buf, TIME_ARRAY_LENGTH); }

// Getters from cached buffer (after update_time)
uint8_t rv1805_get_hundredths(rv1805_t *dev) { return rv1805_BCD_to_DEC(dev->time_buf[TIME_HUNDREDTHS]); }
uint8_t rv1805_get_seconds   (rv1805_t *dev) { return rv1805_BCD_to_DEC(dev->time_buf[TIME_SECONDS]); }
uint8_t rv1805_get_minutes   (rv1805_t *dev) { return rv1805_BCD_to_DEC(dev->time_buf[TIME_MINUTES]); }
uint8_t rv1805_get_hours     (rv1805_t *dev) { return rv1805_BCD_to_DEC(dev->time_buf[TIME_HOURS]); }
uint8_t rv1805_get_weekday   (rv1805_t *dev) { return rv1805_BCD_to_DEC(dev->time_buf[TIME_DAY]); }
uint8_t rv1805_get_date      (rv1805_t *dev) { return rv1805_BCD_to_DEC(dev->time_buf[TIME_DATE]); }
uint8_t rv1805_get_month     (rv1805_t *dev) { return rv1805_BCD_to_DEC(dev->time_buf[TIME_MONTH]); }
uint8_t rv1805_get_year      (rv1805_t *dev) { return rv1805_BCD_to_DEC(dev->time_buf[TIME_YEAR]); }

// String helpers (static buffers)
char *rv1805_string_date_usa(rv1805_t *dev)
{
    static char buf[11];
    sprintf(buf, "%02u/%02u/20%02u",
            rv1805_get_month(dev), rv1805_get_date(dev), rv1805_get_year(dev));
    return buf;
}
char *rv1805_string_date(rv1805_t *dev)
{
    static char buf[11];
    sprintf(buf, "%02u/%02u/20%02u",
            rv1805_get_date(dev), rv1805_get_month(dev), rv1805_get_year(dev));
    return buf;
}
char *rv1805_string_time(rv1805_t *dev)
{
    static char buf[11];
    if (rv1805_is_12hour(dev)) {
        char half = rv1805_is_pm(dev) ? 'P' : 'A';
        sprintf(buf, "%02u:%02u:%02u%cM",
                rv1805_get_hours(dev), rv1805_get_minutes(dev), rv1805_get_seconds(dev), half);
    } else {
        sprintf(buf, "%02u:%02u:%02u",
                rv1805_get_hours(dev), rv1805_get_minutes(dev), rv1805_get_seconds(dev));
    }
    return buf;
}
char *rv1805_string_timestamp(rv1805_t *dev)
{
    static char buf[23];
    sprintf(buf, "20%02u-%02u-%02uT%02u:%02u:%02u:%02u",
            rv1805_get_year(dev),
            rv1805_get_month(dev),
            rv1805_get_date(dev),
            rv1805_get_hours(dev),
            rv1805_get_minutes(dev),
            rv1805_get_seconds(dev),
            rv1805_get_hundredths(dev));
    return buf;
}

uint32_t rv1805_get_epoch(rv1805_t *dev)
{
    struct tm tmv;
    memset(&tmv, 0, sizeof(tmv));
    tmv.tm_isdst = -1;
    tmv.tm_year  = rv1805_get_year(dev) + 100;           // years since 1900
    tmv.tm_mon   = rv1805_get_month(dev) - 1;            // 0-11
    tmv.tm_mday  = rv1805_get_date(dev);
    tmv.tm_hour  = rv1805_get_hours(dev);
    tmv.tm_min   = rv1805_get_minutes(dev);
    tmv.tm_sec   = rv1805_get_seconds(dev);
    return (uint32_t)mktime(&tmv);
}

bool rv1805_set_to_compiler_time(rv1805_t *dev)
{
    if (!dev) return false;

    dev->time_buf[TIME_SECONDS] = rv1805_DEC_to_BCD(BUILD_SECOND);
    dev->time_buf[TIME_MINUTES] = rv1805_DEC_to_BCD(BUILD_MINUTE);
    dev->time_buf[TIME_HOURS]   = rv1805_DEC_to_BCD(BUILD_HOUR);

    // If 12-hour mode, convert hours and set AM/PM bit
    if (rv1805_is_12hour(dev)) {
        uint8_t hour = BUILD_HOUR;
        bool pm = false;
        if (hour == 0) hour = 12;
        else if (hour == 12) pm = true;
        else if (hour > 12) { hour -= 12; pm = true; }
        dev->time_buf[TIME_HOURS] = rv1805_DEC_to_BCD(hour);
        if (pm) dev->time_buf[TIME_HOURS] |= (1u << HOURS_AM_PM);
    }

    dev->time_buf[TIME_MONTH] = rv1805_DEC_to_BCD(BUILD_MONTH);
    dev->time_buf[TIME_DATE]  = rv1805_DEC_to_BCD(BUILD_DATE);
    dev->time_buf[TIME_YEAR]  = rv1805_DEC_to_BCD((uint8_t)(BUILD_YEAR - 2000));

    // Weekday calc (0=Sunday .. 6=Saturday), original adds +1 (1..7)
    uint16_t d = BUILD_DATE;
    uint16_t m = BUILD_MONTH;
    uint16_t y = BUILD_YEAR;
    uint16_t weekday = (d += m < 3 ? y-- : y - 2, 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7 + 1;
    dev->time_buf[TIME_DAY] = rv1805_DEC_to_BCD((uint8_t)weekday);

    return rv1805_set_time_array(dev, dev->time_buf, TIME_ARRAY_LENGTH);
}

// Alarm
bool rv1805_set_alarm_ymdhms(rv1805_t *dev, uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month)
{
    if (!dev) return false;
    uint8_t alarm[TIME_ARRAY_LENGTH];
    alarm[TIME_HUNDREDTHS] = rv1805_DEC_to_BCD(0);
    alarm[TIME_SECONDS]    = rv1805_DEC_to_BCD(sec);
    alarm[TIME_MINUTES]    = rv1805_DEC_to_BCD(min);
    alarm[TIME_HOURS]      = rv1805_DEC_to_BCD(hour);
    alarm[TIME_DATE]       = rv1805_DEC_to_BCD(date);
    alarm[TIME_MONTH]      = rv1805_DEC_to_BCD(month);
    alarm[TIME_YEAR]       = rv1805_DEC_to_BCD(0);
    alarm[TIME_DAY]        = rv1805_DEC_to_BCD(0);
    return rv1805_set_alarm_array(dev, alarm, TIME_ARRAY_LENGTH);
}
bool rv1805_set_alarm_array(rv1805_t *dev, uint8_t *alarm_time, uint8_t len)
{
    if (!dev || !alarm_time || len != TIME_ARRAY_LENGTH) return false;
    return rv1805_write_regs(dev, RV1805_HUNDREDTHS_ALM, alarm_time, TIME_ARRAY_LENGTH);
}

void rv1805_set_alarm_mode(rv1805_t *dev, uint8_t mode)
{
    if (!dev) return;
    if (mode > 0b111) mode = 0b111;
    uint8_t v = 0;
    rv1805_read_reg(dev, RV1805_CTDWN_TMR_CTRL, &v);
    v &= 0b11100011;           // clear ARPT bits
    v |= (uint8_t)(mode << 2); // set ARPT (alarm repeat/match fields)
    rv1805_write_reg(dev, RV1805_CTDWN_TMR_CTRL, v);
}

// Interrupts
void rv1805_enable_interrupt(rv1805_t *dev, uint8_t source)
{
    if (!dev) return;
    uint8_t v = 0;
    rv1805_read_reg(dev, RV1805_INT_MASK, &v);
    v |= (uint8_t)(1u << source);
    rv1805_write_reg(dev, RV1805_INT_MASK, v);
}
void rv1805_disable_interrupt(rv1805_t *dev, uint8_t source)
{
    if (!dev) return;
    uint8_t v = 0;
    rv1805_read_reg(dev, RV1805_INT_MASK, &v);
    v &= (uint8_t)~(1u << source);
    rv1805_write_reg(dev, RV1805_INT_MASK, v);
}
void rv1805_clear_interrupts(rv1805_t *dev)
{
    (void)rv1805_status(dev); // reading STATUS clears flags (with ARST set)
}

// Countdown timer
void rv1805_set_countdown_timer(rv1805_t *dev, uint8_t duration, uint8_t unit, bool repeat, bool pulse)
{
    if (!dev) return;
    if ((duration == 0) || (unit > 0b11)) return;

    rv1805_write_reg(dev, RV1805_CTDWN_TMR, (uint8_t)(duration - 1));
    rv1805_write_reg(dev, RV1805_TMR_INITIAL, (uint8_t)(duration - 1));

    uint8_t v = 0;
    rv1805_read_reg(dev, RV1805_CTDWN_TMR_CTRL, &v);
    v &= 0b00011100; // preserve ARPT bits only (upper fields cleared)
    v |= unit;
    v |= (uint8_t)((!pulse) << CTDWN_TMR_TM_OFFSET);
    v |= (uint8_t)((repeat) << CTDWN_TMR_TRPT_OFFSET);
    v |= (uint8_t)(1u << CTDWN_TMR_TE_OFFSET);
    rv1805_write_reg(dev, RV1805_CTDWN_TMR_CTRL, v);
}

// Power / trickle / low power
void rv1805_enable_sleep(rv1805_t *dev)
{
    if (!dev) return;
    uint8_t v = 0;
    rv1805_read_reg(dev, RV1805_SLP_CTRL, &v);
    v |= (1u << 7);
    rv1805_write_reg(dev, RV1805_SLP_CTRL, v);
}
void rv1805_set_power_switch_function(rv1805_t *dev, uint8_t function)
{
    if (!dev) return;
    uint8_t v = 0;
    rv1805_read_reg(dev, RV1805_CTRL2, &v);
    v &= 0b11000011;                     // clear PSWS bits
    v |= (uint8_t)(function << PSWS_OFFSET);
    rv1805_write_reg(dev, RV1805_CTRL2, v);
}
void rv1805_set_power_switch_lock(rv1805_t *dev, bool lock)
{
    if (!dev) return;
    uint8_t v = 0;
    rv1805_read_reg(dev, RV1805_OSC_STATUS, &v);
    v &= (uint8_t)~(1u << 5);
    v |= (uint8_t)((lock ? 1u : 0u) << 5);
    rv1805_write_reg(dev, RV1805_OSC_STATUS, v);
}
void rv1805_set_static_power_switch_output(rv1805_t *dev, bool psw)
{
    if (!dev) return;
    uint8_t v = 0;
    rv1805_read_reg(dev, RV1805_CTRL1, &v);
    v &= (uint8_t)~(1u << CTRL1_PSWB);
    v |= (uint8_t)((psw ? 1u : 0u) << CTRL1_PSWB);
    rv1805_write_reg(dev, RV1805_CTRL1, v);
}
void rv1805_enable_trickle_charge(rv1805_t *dev, uint8_t diode, uint8_t rOut)
{
    if (!dev) return;
    rv1805_write_reg(dev, RV1805_CONF_KEY, RV1805_CONF_WRT);
    uint8_t v = 0;
    v |= (uint8_t)(TRICKLE_ENABLE << TRICKLE_CHARGER_TCS_OFFSET);
    v |= (uint8_t)(diode          << TRICKLE_CHARGER_DIODE_OFFSET);
    v |= (uint8_t)(rOut           << TRICKLE_CHARGER_ROUT_OFFSET);
    rv1805_write_reg(dev, RV1805_TRICKLE_CHRG, v);
}
void rv1805_disable_trickle_charge(rv1805_t *dev)
{
    if (!dev) return;
    rv1805_write_reg(dev, RV1805_CONF_KEY, RV1805_CONF_WRT);
    rv1805_write_reg(dev, RV1805_TRICKLE_CHRG, (uint8_t)(TRICKLE_DISABLE << TRICKLE_CHARGER_TCS_OFFSET));
}
void rv1805_enable_low_power(rv1805_t *dev)
{
    if (!dev) return;

    rv1805_write_reg(dev, RV1805_CONF_KEY, RV1805_CONF_WRT);
    rv1805_write_reg(dev, RV1805_IOBATMODE, 0x00); // Disable I2C on backup power

    rv1805_write_reg(dev, RV1805_CONF_KEY, RV1805_CONF_WRT);
    rv1805_write_reg(dev, RV1805_OUT_CTRL, 0x30);  // Disable WDI, etc.

    rv1805_write_reg(dev, RV1805_CONF_KEY, RV1805_CONF_OSC);
    rv1805_write_reg(dev, RV1805_OSC_CTRL, 0b11111100); // OSEL=1, ACAL=11, BOS=1, FOS=1, IOPW=1, OFIE=0, ACIE=0

    uint8_t c1 = 0;
    rv1805_read_reg(dev, RV1805_CTRL1, &c1);
    c1 |= (1u << CTRL1_RSTP);              // set reset pin high
    rv1805_write_reg(dev, RV1805_CTRL1, c1);

    rv1805_set_power_switch_lock(dev, PSW_UNLOCK);
    rv1805_set_static_power_switch_output(dev, PSW_OFF);

    // RC oscillator usage / auto calibration comments preserved from original
}

// Battery / reference voltage
void rv1805_enable_battery_interrupt(rv1805_t *dev, uint8_t voltage, bool edgeTrigger)
{
    if (!dev) return;
    rv1805_set_edge_trigger(dev, edgeTrigger);
    rv1805_enable_interrupt(dev, INTERRUPT_BLIE);
    rv1805_set_reference_voltage(dev, voltage);
}

bool rv1805_check_battery(rv1805_t *dev, uint8_t voltage)
{
    if (!dev) return false;
    rv1805_set_reference_voltage(dev, voltage);
    uint8_t s = 0;
    rv1805_read_reg(dev, RV1805_ANLG_STAT, &s);
    return (s >= 0x80);
}

void rv1805_set_reference_voltage(rv1805_t *dev, uint8_t voltage)
{
    if (!dev) return;
    if (voltage > 3) voltage = 3;
    uint8_t v = TWO_FIVE;
    switch (voltage) {
        case 0: v = TWO_FIVE;  break;
        case 1: v = TWO_ONE;   break;
        case 2: v = ONE_EIGHT; break;
        case 3: v = ONE_FOUR;  break;
        default: break;
    }
    rv1805_write_reg(dev, RV1805_CONF_KEY, RV1805_CONF_WRT);
    rv1805_write_reg(dev, RV1805_BREF_CTRL, v);
}

void rv1805_set_edge_trigger(rv1805_t *dev, bool edgeTrigger)
{
    if (!dev) return;
    uint8_t v = 0;
    rv1805_read_reg(dev, RV1805_RAM_EXT, &v);
    v &= (uint8_t)~(1u << 6); // Clear BPOL
    v |= (uint8_t)((edgeTrigger ? 1u : 0u) << 6);
    rv1805_write_reg(dev, RV1805_RAM_EXT, v);
}

//===========================
// Low-level I2C helpers (blocking) using NXP SDK
//===========================
static bool rv1805_write_reg(rv1805_t *dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    i2c_master_transfer_t xfer = {
        .flags = kI2C_TransferDefaultFlag,
        .slaveAddress = RV1805_ADDR,
        .direction = kI2C_Write,
        .subaddress = 0,
        .subaddressSize = 0,
        .data = buf,
        .dataSize = sizeof(buf)
    };
    return (I2C_MasterTransferBlocking(dev->i2c, &xfer) == kStatus_Success);
}

static bool rv1805_write_regs(rv1805_t *dev, uint8_t reg, const uint8_t *data, size_t len)
{
    uint8_t tmp[1 + 32];
    if (len > 32) return false; // adjust as needed; split if required
    tmp[0] = reg;
    memcpy(&tmp[1], data, len);

    i2c_master_transfer_t xfer = {
        .flags = kI2C_TransferDefaultFlag,
        .slaveAddress = RV1805_ADDR,
        .direction = kI2C_Write,
        .subaddress = 0,
        .subaddressSize = 0,
        .data = tmp,
        .dataSize = (uint32_t)(1 + len)
    };
    return (I2C_MasterTransferBlocking(dev->i2c, &xfer) == kStatus_Success);
}
static bool rv1805_read_reg(rv1805_t *dev, uint8_t reg, uint8_t *val)
{
    i2c_master_transfer_t xfer = {
        .flags = kI2C_TransferDefaultFlag,
        .slaveAddress = RV1805_ADDR,
        .direction = kI2C_Write,
        .subaddress = 0,
        .subaddressSize = 0,
        .data = &reg,
        .dataSize = 1
    };
    if (I2C_MasterTransferBlocking(dev->i2c, &xfer) != kStatus_Success) return false;

    i2c_master_transfer_t rx = {
        .flags = kI2C_TransferDefaultFlag,
        .slaveAddress = RV1805_ADDR,
        .direction = kI2C_Read,
        .subaddress = 0,
        .subaddressSize = 0,
        .data = val,
        .dataSize = 1
    };
    return (I2C_MasterTransferBlocking(dev->i2c, &rx) == kStatus_Success);
}

static bool rv1805_read_regs(rv1805_t *dev, uint8_t reg, uint8_t *data, size_t len)
{
    i2c_master_transfer_t w = {
        .flags = kI2C_TransferDefaultFlag,
        .slaveAddress = RV1805_ADDR,
        .direction = kI2C_Write,
        .subaddress = 0,
        .subaddressSize = 0,
        .data = &reg,
        .dataSize = 1
    };
    if (I2C_MasterTransferBlocking(dev->i2c, &w) != kStatus_Success) return false;

    i2c_master_transfer_t r = {
        .flags = kI2C_TransferDefaultFlag,
        .slaveAddress = RV1805_ADDR,
        .direction = kI2C_Read,
        .subaddress = 0,
        .subaddressSize = 0,
        .data = data,
        .dataSize = (uint32_t)len
    };
    return (I2C_MasterTransferBlocking(dev->i2c, &r) == kStatus_Success);
}

static void delay_ms(uint32_t ms)
{
    // Assuming a 48MHz core clock. Adjust as needed.
    volatile uint32_t cycles = ms * 48000U;
    while (cycles--) { __NOP(); }
}



//void init_gpio(void);
//void enable_digit(uint8_t digit);
//
//void init_gpio(void) {
//    // Enable clocks
//    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK;
//
//    // Segment pins
//    PORTD->PCR[SEG_A] = PORT_PCR_MUX(1); // PTD1
//    PORTD->PCR[SEG_B] = PORT_PCR_MUX(1); // PTD2
//    PORTD->PCR[SEG_C] = PORT_PCR_MUX(1); // PTD5
//    PORTD->PCR[SEG_F] = PORT_PCR_MUX(1); // PTD3
//    PORTD->PCR[SEG_G] = PORT_PCR_MUX(1); // PTD0
//    PORTA->PCR[SEG_D] = PORT_PCR_MUX(1); // PTA16
//    PORTC->PCR[SEG_E] = PORT_PCR_MUX(1); // PTC17
//    PORTA->PCR[SEG_DP] = PORT_PCR_MUX(1); // PTA17
//
//    // Digit select pins
//    PORTC->PCR[DIGIT1] = PORT_PCR_MUX(1); // PTC9
//    PORTC->PCR[DIGIT2] = PORT_PCR_MUX(1); // PTC8
//    PORTC->PCR[DIGIT3] = PORT_PCR_MUX(1); // PTC11
//    PORTC->PCR[DIGIT4] = PORT_PCR_MUX(1); // PTC10
//
//    // Set all as output
//    GPIOD->PDDR |= (1 << SEG_A) | (1 << SEG_B) | (1 << SEG_C) | (1 << SEG_F) | (1 << SEG_G);
//    GPIOA->PDDR |= (1 << SEG_D) | (1 << SEG_DP);
//    GPIOC->PDDR |= (1 << SEG_E) | (1 << DIGIT1) | (1 << DIGIT2) | (1 << DIGIT3) | (1 << DIGIT4);
//}
//const uint8_t digit_segments[10] = {
//    0b00111111, // 0 = A B C D E F
//    0b00000110, // 1 = B C
//    0b01011011, // 2 = A B G E D
//    0b01001111, // 3 = A B C D G
//    0b01100110, // 4 = F G B C
//    0b01101101, // 5 = A F G C D
//    0b01111101, // 6 = A F G C D E
//    0b00000111, // 7 = A B C
//    0b01111111, // 8 = All segments
//    0b01101111  // 9 = A B C D F G
//};
//
//void display_digit(uint8_t num) {
//    // Clear all segments first
//    GPIOD->PCOR = (1 << SEG_A) | (1 << SEG_B) | (1 << SEG_C) | (1 << SEG_F) | (1 << SEG_G);
//    GPIOA->PCOR = (1 << SEG_D) | (1 << SEG_DP);
//    GPIOC->PCOR = (1 << SEG_E);
//
//    uint8_t seg = digit_segments[num % 10];  // safeguard if num > 9
//
//    if (seg & (1 << 0)) GPIOD->PSOR |= (1 << SEG_A);
//    if (seg & (1 << 1)) GPIOD->PSOR |= (1 << SEG_B);
//    if (seg & (1 << 2)) GPIOD->PSOR |= (1 << SEG_C);
//    if (seg & (1 << 3)) GPIOA->PSOR |= (1 << SEG_D);
//    if (seg & (1 << 4)) GPIOC->PSOR |= (1 << SEG_E);
//    if (seg & (1 << 5)) GPIOD->PSOR |= (1 << SEG_F);
//    if (seg & (1 << 6)) GPIOD->PSOR |= (1 << SEG_G);
//}
//
//void enable_digit(uint8_t digit) {
//    // Turn off all digits first
//    GPIOC->PSOR = (1 << DIGIT1) | (1 << DIGIT2) | (1 << DIGIT3) | (1 << DIGIT4);
//
//    // Turn on the selected digit
//    switch (digit) {
//        case 0: GPIOC->PCOR = (1 << DIGIT1); break;
//        case 1: GPIOC->PCOR = (1 << DIGIT2); break;
//        case 2: GPIOC->PCOR = (1 << DIGIT3); break;
//        case 3: GPIOC->PCOR = (1 << DIGIT4); break;
//    }
//}
//
//
//int main(void) {
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    init_gpio();
//
//    // Initialize I2C, RV1805 device
//    rv1805_t rv1805_dev;
//    if (!rv1805_begin(&rv1805_dev, RV1805_I2C_BASE)) {
//        PRINTF("RV1805 initialization failed!\r\n");
//        while (1);  // Halt
//    }
//
//    // Optionally set time here (e.g., to compiler time)
//    rv1805_set_to_compiler_time(&rv1805_dev);
//
//    while (1) {
//        // Update time from RTC
//        if (rv1805_update_time(&rv1805_dev)) {
//            // Extract digits for HH:MM display
//            uint8_t digits[4];
//            digits[0] = rv1805_get_hours(&rv1805_dev) / 10;
//            digits[1] = rv1805_get_hours(&rv1805_dev) % 10;
//            digits[2] = rv1805_get_minutes(&rv1805_dev) / 10;
//            digits[3] = rv1805_get_minutes(&rv1805_dev) % 10;
//
//            // Multiplex display for a short period (e.g., 5ms each digit)
//            for (int i = 0; i < 4; i++) {
//                display_digit(digits[i]);
//                enable_digit(i);
//                delay_ms(1);
//            }
//        } else {
//            PRINTF("Failed to update time from RV1805.\r\n");
//            delay_ms(20);  // Wait a bit before retry
//        }
//    }
//
//    return 0;
//}


//
//const uint8_t digit_segments[10] = {
//    0b00111111, // 0 = A B C D E F
//    0b00000110, // 1 = B C
//    0b01011011, // 2 = A B G E D
//    0b01001111, // 3 = A B C D G
//    0b01100110, // 4 = F G B C
//    0b01101101, // 5 = A F G C D
//    0b01111101, // 6 = A F G C D E
//    0b00000111, // 7 = A B C
//    0b01111111, // 8 = All
//    0b01101111  // 9 = A B C D F G
//};
//
//void display_digit(uint8_t num) {
//    // Clear all segments
//    GPIOD->PCOR = (1 << SEG_A) | (1 << SEG_B) | (1 << SEG_C) |
//                  (1 << SEG_F) | (1 << SEG_G);
//    GPIOA->PCOR = (1 << SEG_D) | (1 << SEG_DP);
//    GPIOC->PCOR = (1 << SEG_E);
//
//    uint8_t seg = digit_segments[num % 10];  // safety
//
//    if (seg & (1 << 0)) GPIOD->PSOR |= (1 << SEG_A);
//    if (seg & (1 << 1)) GPIOD->PSOR |= (1 << SEG_B);
//    if (seg & (1 << 2)) GPIOD->PSOR |= (1 << SEG_C);
//    if (seg & (1 << 3)) GPIOA->PSOR |= (1 << SEG_D);
//    if (seg & (1 << 4)) GPIOC->PSOR |= (1 << SEG_E);
//    if (seg & (1 << 5)) GPIOD->PSOR |= (1 << SEG_F);
//    if (seg & (1 << 6)) GPIOD->PSOR |= (1 << SEG_G);
//}
//void enable_digit(uint8_t digit) {
//    // Turn off all digits first
//    GPIOC->PSOR = (1 << DIGIT1) | (1 << DIGIT2) | (1 << DIGIT3) | (1 << DIGIT4);
//
//    // Enable only the selected digit (active low or high depending on your hardware)
//    switch(digit) {
//        case 0: GPIOC->PCOR = (1 << DIGIT1); break;
//        case 1: GPIOC->PCOR = (1 << DIGIT2); break;
//        case 2: GPIOC->PCOR = (1 << DIGIT3); break;
//        case 3: GPIOC->PCOR = (1 << DIGIT4); break;
//        default: break;
//    }
//}
//
//int main(void) {
//    init_gpio();
//    RV1805_t rtc;
//    RV1805_begin(&rtc, I2C0);
//
//    uint8_t digits[4] = {0, 0, 0, 0};
//    uint32_t counter = 0;
//
//    while (1) {
//        // Refresh display via multiplexing
//        for (int i = 0; i < 4; i++) {
//            display_digit(digits[i]);
//            enable_digit(i);
//            delay_ms(1);
//        }
//
//        // Update time every ~1000 iterations (~1s)
//        if (++counter >= 1000) {
//            counter = 0;
//            RV1805_updateTime(&rtc);
//            uint8_t hours = RV1805_getHours(&rtc);
//            uint8_t mins = RV1805_getMinutes(&rtc);
//
//            // Convert to individual digits
//            digits[0] = hours / 10;
//            digits[1] = hours % 10;
//            digits[2] = mins / 10;
//            digits[3] = mins % 10;
//        }
//    }
//}










//int main(void)
//{
//    // -------------------------
//    // Board / clocks / pins
//    // -------------------------
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole(); // enables PRINTF()
//
//    PRINTF("\r\nKL25Z + RV1805 demo\r\n");
//
//    // -------------------------
//    // I2C init
//    // -------------------------
//    i2c_master_config_t i2cConfig;
//    I2C_MasterGetDefaultConfig(&i2cConfig);
//    // Assuming RV1805_I2C_BAUD is defined elsewhere, e.g., in board.h or rv_1805.h
//    // For example: #define RV1805_I2C_BAUD 100000U
//    i2cConfig.baudRate_Bps = RV1805_I2C_BAUD;
//
//    // KL25Z bus clock drives I2C0
//    uint32_t i2cClockHz = CLOCK_GetFreq(kCLOCK_BusClk);
//    I2C_MasterInit(RV1805_I2C_BASE, &i2cConfig, i2cClockHz);
//
//    // -------------------------
//    // RV1805 init
//    // -------------------------
//    rv1805_t rv1805_dev; // Declare the RV1805 device context
//
//    // Try to begin communication with the RV1805
//    if (!rv1805_begin(&rv1805_dev, RV1805_I2C_BASE))
//    {
//        PRINTF("RV1805 initialization failed! Please check wiring and I2C address.\r\n");
//        while (1) { /* Halt on error */ }
//    }
//    PRINTF("RV1805 initialized successfully.\r\n");
//
//    // Set the RTC time to the compiler's build time
//    if (!rv1805_set_to_compiler_time(&rv1805_dev))
//    {
//        PRINTF("Failed to set RV1805 time to compiler time.\r\n");
//    }
//    else
//    {
//        PRINTF("RV1805 time set to compiler time.\r\n");
//    }
//
//    // Main loop to continuously read and print the time
//    while (1)
//    {
//        if (rv1805_update_time(&rv1805_dev))
//        {
//            PRINTF("Current Time: %s %s (Weekday: %u, Epoch: %lu)\r\n",
//                   rv1805_string_date_usa(&rv1805_dev),
//                   rv1805_string_time(&rv1805_dev),
//                   rv1805_get_weekday(&rv1805_dev),
//                   rv1805_get_epoch(&rv1805_dev));
//        }
//        else
//        {
//            PRINTF("Failed to read time from RV1805.\r\n");
//        }
//
//        delay_ms(1000); // Delay for 1 second before reading again
//    }
//
//    return 0;
//
//}











int main(void)
{
    // -------------------------
    // Board / clocks / pins
    // -------------------------
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole(); // enables PRINTF()

    PRINTF("\r\nKL25Z + RV1805 demo\r\n");

    // -------------------------
    // I2C init
    // -------------------------
    i2c_master_config_t i2cConfig;
    I2C_MasterGetDefaultConfig(&i2cConfig);
    // Assuming RV1805_I2C_BAUD is defined elsewhere, e.g., in board.h or rv_1805.h
    // For example: #define RV1805_I2C_BAUD 100000U
    i2cConfig.baudRate_Bps = RV1805_I2C_BAUD;

    // KL25Z bus clock drives I2C0
    uint32_t i2cClockHz = CLOCK_GetFreq(kCLOCK_BusClk);
    I2C_MasterInit(RV1805_I2C_BASE, &i2cConfig, i2cClockHz);

    // -------------------------
    // RV1805 init
    // -------------------------
    rv1805_t rv1805_dev; // Declare the RV1805 device context

    // Try to begin communication with the RV1805
    if (!rv1805_begin(&rv1805_dev, RV1805_I2C_BASE))
    {
        PRINTF("RV1805 initialization failed! Please check wiring and I2C address.\r\n");
        while (1) { /* Halt on error */ }
    }
    PRINTF("RV1805 initialized successfully.\r\n");

    // Set the RTC time to the compiler's build time
    if (!rv1805_set_to_compiler_time(&rv1805_dev))
    {
        PRINTF("Failed to set RV1805 time to compiler time.\r\n");
    }
    else
    {
        PRINTF("RV1805 time set to compiler time.\r\n");
    }

    // Main loop to continuously read and print the time
    while (1)
    {
        if (rv1805_update_time(&rv1805_dev))
        {
            PRINTF("Current Time: %s %s (Weekday: %u, Epoch: %lu)\r\n",
                   rv1805_string_date_usa(&rv1805_dev),
                   rv1805_string_time(&rv1805_dev),
                   rv1805_get_weekday(&rv1805_dev),
                   rv1805_get_epoch(&rv1805_dev));
        }
        else
        {
            PRINTF("Failed to read time from RV1805.\r\n");
        }

        delay_ms(1000); // Delay for 1 second before reading again
    }

    return 0;
}

