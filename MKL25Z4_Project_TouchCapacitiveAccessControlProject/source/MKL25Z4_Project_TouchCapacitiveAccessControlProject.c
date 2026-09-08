


#include "board.h"
#include "fsl_tsi_v4.h"
#include "fsl_debug_console.h"
#include "fsl_lptmr.h"
#include "fsl_i2c.h"
#include "clock_config.h"
#include "pin_mux.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*
 * Definitions
 */

/* Touch Sensor */
#define PAD_TSI_ELECTRODE_1_NAME    "E1"

#define LPTMR_SOURCE_CLOCK          CLOCK_GetFreq(kCLOCK_LpoClk)
#define LPTMR_USEC_COUNT            (300000U)

#define TOUCH_DELTA_VALUE           100U


/*
 * LED Control
 */

#define LED_INIT()                  \
    LED_RED_INIT(LOGIC_LED_OFF);    \
    LED_GREEN_INIT(LOGIC_LED_OFF)

#define LED_LOCK()                  \
    LED_RED_ON();                   \
    LED_GREEN_OFF()

#define LED_UNLOCK()                \
    LED_RED_OFF();                  \
    LED_GREEN_ON()


/*
 * PCA9685 Definitions
 */

#define PCA9685_I2C_ADDRESS         0x40U

#define PCA9685_MODE1               0x00U
#define PCA9685_MODE2               0x01U

#define PCA9685_SUBADR1             0x02U
#define PCA9685_SUBADR2             0x03U
#define PCA9685_SUBADR3             0x04U
#define PCA9685_ALLCALLADR          0x05U

#define PCA9685_LED0_ON_L           0x06U
#define PCA9685_LED0_ON_H           0x07U
#define PCA9685_LED0_OFF_L          0x08U
#define PCA9685_LED0_OFF_H          0x09U

#define PCA9685_PRESCALE            0xFEU

/* MODE1 bits */
#define MODE1_RESTART               0x80U
#define MODE1_AI                    0x20U
#define MODE1_SLEEP                 0x10U

/* MODE2 bits */
#define MODE2_OUTDRV                0x04U


/*
 * Servo Definitions
 */

/*
 * We are using five PCA9685 channels:
 *
 * Channel 0 -> Servo 1
 * Channel 1 -> Servo 2
 * Channel 2 -> Servo 3
 * Channel 3 -> Servo 4
 * Channel 4 -> Servo 5
 */

#define NUMBER_OF_SERVOS            5U

#define SERVO_1_CHANNEL             0U
#define SERVO_2_CHANNEL             1U
#define SERVO_3_CHANNEL             2U
#define SERVO_4_CHANNEL             3U
#define SERVO_5_CHANNEL             4U

/*
 * PCA9685 at 50 Hz:
 *
 * 205 counts ~= 1 ms
 * 410 counts ~= 2 ms
 *
 * Initially all five servos use the same lock/unlock positions.
 */
#define SERVO_LOCK_PULSE            205U
#define SERVO_UNLOCK_PULSE          410U

#define SERVO_FREQUENCY_HZ          50.0f


/*
 * Variables
 */

tsi_calibration_data_t buffer;

/*
 * false = unlocked
 * true  = locked
 */
volatile bool isLocked = false;

/*
 * Prevent continuous finger contact from causing repeated toggling.
 */
volatile bool wasTouched = false;

/*
 * Set by TSI interrupt.
 * Servos are moved later in main().
 */
volatile bool lockChangeRequested = false;


/*
 * PCA9685 Variables
 */

static I2C_Type *_i2c_base;
static uint8_t _addr;


/*
 * Delay
 */

static void delay_us(uint32_t us)
{
    uint32_t cycles;

    cycles = (SystemCoreClock / 1000000U) * us / 5U;

    while (cycles--)
    {
        __NOP();
    }
}


/*
 * PCA9685 I2C Write
 */

static status_t pca9685_memWrite(uint8_t reg,
                                 uint8_t *data,
                                 uint32_t len)
{
    i2c_master_transfer_t xfer = {0};

    xfer.slaveAddress   = _addr;
    xfer.direction      = kI2C_Write;
    xfer.subaddress     = reg;
    xfer.subaddressSize = 1U;
    xfer.data           = data;
    xfer.dataSize       = len;
    xfer.flags          = kI2C_TransferDefaultFlag;

    return I2C_MasterTransferBlocking(_i2c_base, &xfer);
}


/*
 * PCA9685 I2C Read
 */

static status_t pca9685_memRead(uint8_t reg,
                                uint8_t *data,
                                uint32_t len)
{
    i2c_master_transfer_t xfer = {0};

    xfer.slaveAddress   = _addr;
    xfer.direction      = kI2C_Read;
    xfer.subaddress     = reg;
    xfer.subaddressSize = 1U;
    xfer.data           = data;
    xfer.dataSize       = len;
    xfer.flags          = kI2C_TransferDefaultFlag;

    return I2C_MasterTransferBlocking(_i2c_base, &xfer);
}


/*
 * PCA9685 Write Register
 */

static status_t pca9685_write8(uint8_t reg, uint8_t value)
{
    return pca9685_memWrite(reg, &value, 1U);
}


/*
 * PCA9685 Read Register
 */

static status_t pca9685_read8(uint8_t reg, uint8_t *value)
{
    return pca9685_memRead(reg, value, 1U);
}


/*
 * PCA9685 Initialization
 */

static status_t pca9685_init(I2C_Type *base, uint8_t address)
{
    status_t status;

    uint8_t mode1 = 0x00U;
    uint8_t mode2 = MODE2_OUTDRV;

    _i2c_base = base;
    _addr = address;

    status = pca9685_memWrite(
        PCA9685_MODE1,
        &mode1,
        1U
    );

    if (status != kStatus_Success)
    {
        return status;
    }

    status = pca9685_memWrite(
        PCA9685_MODE2,
        &mode2,
        1U
    );

    return status;
}


/*
 * Set PCA9685 PWM Frequency
 */

static status_t pca9685_setPWMFreq(float freq)
{
    float prescaleval;

    uint8_t prescale;
    uint8_t oldmode;
    uint8_t sleep;

    status_t status;

    /*
     * PCA9685 nominal oscillator:
     * 25 MHz
     */
    prescaleval =
        (25000000.0f / (4096.0f * freq)) - 1.0f;

    prescale =
        (uint8_t)(prescaleval + 0.5f);


    status = pca9685_read8(
        PCA9685_MODE1,
        &oldmode
    );

    if (status != kStatus_Success)
    {
        return status;
    }


    /*
     * PCA9685 must sleep before changing PRESCALE.
     */
    sleep =
        (oldmode & 0x7FU) | MODE1_SLEEP;

    status = pca9685_write8(
        PCA9685_MODE1,
        sleep
    );

    if (status != kStatus_Success)
    {
        return status;
    }


    status = pca9685_write8(
        PCA9685_PRESCALE,
        prescale
    );

    if (status != kStatus_Success)
    {
        return status;
    }


    /*
     * Wake PCA9685.
     */
    status = pca9685_write8(
        PCA9685_MODE1,
        oldmode
    );

    if (status != kStatus_Success)
    {
        return status;
    }


    delay_us(500U);


    /*
     * Restart oscillator and enable Auto Increment.
     */
    status = pca9685_write8(
        PCA9685_MODE1,
        oldmode | MODE1_RESTART | MODE1_AI
    );

    return status;
}


/*
 * Set One PCA9685 PWM Channel
 */

static status_t pca9685_setPWM(uint8_t channel,
                               uint16_t on,
                               uint16_t off)
{
    uint8_t reg;

    uint8_t buf[4];


    if (channel > 15U)
    {
        return kStatus_InvalidArgument;
    }


    /*
     * Each PCA9685 channel uses 4 registers.
     */
    reg =
        PCA9685_LED0_ON_L +
        (4U * channel);


    buf[0] = (uint8_t)(on & 0xFFU);
    buf[1] = (uint8_t)((on >> 8U) & 0x0FU);

    buf[2] = (uint8_t)(off & 0xFFU);
    buf[3] = (uint8_t)((off >> 8U) & 0x0FU);


    return pca9685_memWrite(
        reg,
        buf,
        4U
    );
}


/*
 * Set All Five Servos
 */

static status_t setAllServos(uint16_t pulse)
{
    uint8_t channel;

    status_t status;


    /*
     * Channels 0 through 4.
     */
    for (channel = 0U;
         channel < NUMBER_OF_SERVOS;
         channel++)
    {
        status = pca9685_setPWM(
            channel,
            0U,
            pulse
        );


        if (status != kStatus_Success)
        {
            return status;
        }
    }


    return kStatus_Success;
}


/*
 * Lock All Five Servos
 */

static status_t lockAllServos(void)
{
    return setAllServos(
        SERVO_LOCK_PULSE
    );
}


/*
 * Unlock All Five Servos
 */

static status_t unlockAllServos(void)
{
    return setAllServos(
        SERVO_UNLOCK_PULSE
    );
}


/*
 * TSI Interrupt Handler
 */

void TSI0_IRQHandler(void)
{
    uint16_t currentCount;

    uint16_t threshold;


    /*
     * Read the touch sensor.
     */
    currentCount =
        TSI_GetCounter(TSI0);


    /*
     * Touch threshold =
     * calibrated baseline + delta.
     */
    threshold =
        buffer.calibratedData[
            BOARD_TSI_ELECTRODE_1
        ]
        + TOUCH_DELTA_VALUE;


    /*
     * Make sure E1 is the measured electrode.
     */
    if (TSI_GetMeasuredChannelNumber(TSI0)
        == BOARD_TSI_ELECTRODE_1)
    {
        /*
         * Finger touching electrode.
         */
        if (currentCount > threshold)
        {
            /*
             * Only trigger once per physical touch.
             */
            if (!wasTouched)
            {
                wasTouched = true;


                /*
                 * Toggle lock state.
                 */
                isLocked = !isLocked;


                /*
                 * Tell main loop to move the servos.
                 */
                lockChangeRequested = true;
            }
        }

        /*
         * Finger released.
         */
        else
        {
            wasTouched = false;
        }
    }


    /*
     * Clear TSI flags.
     */
    TSI_ClearStatusFlags(
        TSI0,
        kTSI_EndOfScanFlag
    );

    TSI_ClearStatusFlags(
        TSI0,
        kTSI_OutOfRangeFlag
    );
}




int main(void)
{
    volatile uint32_t i = 0U;

    tsi_config_t tsiConfig_normal = {0};

    lptmr_config_t lptmrConfig;

    i2c_master_config_t i2cConfig;

    status_t status;


    memset(
        (void *)&lptmrConfig,
        0,
        sizeof(lptmrConfig)
    );


    /*
     * Board Initialization
     */

    BOARD_InitPins();

    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();


    /*
     * Initialize onboard LEDs.
     */
    LED_INIT();


    /*
     * Configure I2C1
     *
     * PTC1 = I2C1_SCL
     * PTC2 = I2C1_SDA
     */

    SIM->SCGC5 |=
        SIM_SCGC5_PORTC_MASK;

    SIM->SCGC4 |=
        SIM_SCGC4_I2C1_MASK;


    /*
     * PTC1 -> ALT2 -> I2C1_SCL
     */
    PORTC->PCR[1] =
        (PORTC->PCR[1] & ~PORT_PCR_MUX_MASK)
        | PORT_PCR_MUX(2);


    /*
     * PTC2 -> ALT2 -> I2C1_SDA
     */
    PORTC->PCR[2] =
        (PORTC->PCR[2] & ~PORT_PCR_MUX_MASK)
        | PORT_PCR_MUX(2);


    /*
     * Initialize I2C1
     */

    I2C_MasterGetDefaultConfig(
        &i2cConfig
    );


    i2cConfig.baudRate_Bps =
        100000U;


    I2C_MasterInit(
        I2C1,
        &i2cConfig,
        CLOCK_GetFreq(kCLOCK_BusClk)
    );


    /*
     * Initialize PCA9685
     */

    status = pca9685_init(
        I2C1,
        PCA9685_I2C_ADDRESS
    );


    if (status != kStatus_Success)
    {
        PRINTF(
            "ERROR: PCA9685 initialization failed.\r\n"
        );
    }
    else
    {
        PRINTF(
            "PCA9685 initialized at address 0x40.\r\n"
        );
    }


    /*
     * Configure PCA9685 for Servo PWM
     */

    status = pca9685_setPWMFreq(
        SERVO_FREQUENCY_HZ
    );


    if (status != kStatus_Success)
    {
        PRINTF(
            "ERROR: PCA9685 frequency configuration failed.\r\n"
        );
    }
    else
    {
        PRINTF(
            "PCA9685 configured for 50 Hz.\r\n"
        );
    }


    /*
     * Initial Door State
     *
     * Program starts unlocked.
     */

    isLocked = false;

    LED_UNLOCK();


    status =
        unlockAllServos();


    if (status == kStatus_Success)
    {
        PRINTF(
            "Door initially Unlocked - all 5 servos moved.\r\n"
        );
    }
    else
    {
        PRINTF(
            "ERROR: Initial servo positioning failed.\r\n"
        );
    }


    /*
     * Configure LPTMR
     */

    LPTMR_GetDefaultConfig(
        &lptmrConfig
    );


    /*
     * Configure TSI
     */

    TSI_GetNormalModeDefaultConfig(
        &tsiConfig_normal
    );


    LPTMR_Init(
        LPTMR0,
        &lptmrConfig
    );


    TSI_Init(
        TSI0,
        &tsiConfig_normal
    );


    /*
     * Touch sensor sampling interval.
     */
    LPTMR_SetTimerPeriod(
        LPTMR0,
        USEC_TO_COUNT(
            LPTMR_USEC_COUNT,
            LPTMR_SOURCE_CLOCK
        )
    );


    NVIC_EnableIRQ(
        TSI0_IRQn
    );


    TSI_EnableModule(
        TSI0,
        true
    );


    PRINTF(
        "\r\nKL25Z + PCA9685 Five Servo Door Lock\r\n"
    );


    /*
     * TSI Calibration
     */

    memset(
        (void *)&buffer,
        0,
        sizeof(buffer)
    );


    TSI_Calibrate(
        TSI0,
        &buffer
    );


    for (i = 0U;
         i < FSL_FEATURE_TSI_CHANNEL_COUNT;
         i++)
    {
        PRINTF(
            "Calibrated counter for channel %d: %d\r\n",
            i,
            buffer.calibratedData[i]
        );
    }


    /***************************
     * Hardware Trigger TSI Scan
     ***************************/

    PRINTF(
        "\r\nTouch pad %s to lock/unlock all five servos.\r\n",
        PAD_TSI_ELECTRODE_1_NAME
    );


    TSI_EnableModule(
        TSI0,
        false
    );


    TSI_EnableHardwareTriggerScan(
        TSI0,
        true
    );


    TSI_EnableInterrupts(
        TSI0,
        kTSI_GlobalInterruptEnable
    );


    TSI_EnableInterrupts(
        TSI0,
        kTSI_EndOfScanInterruptEnable
    );


    TSI_ClearStatusFlags(
        TSI0,
        kTSI_EndOfScanFlag
    );


    TSI_SetMeasuredChannelNumber(
        TSI0,
        BOARD_TSI_ELECTRODE_1
    );


    TSI_EnableModule(
        TSI0,
        true
    );


    /*
     * Start hardware-triggered TSI measurements.
     */
    LPTMR_StartTimer(
        LPTMR0
    );


    /************
     * Main Loop
     ************/

    while (1)
    {
        /*
         * New touch event detected.
         */
        if (lockChangeRequested)
        {
            lockChangeRequested = false;


            /*******
             * LOCK
             *******/

            if (isLocked)
            {
                LED_LOCK();


                /*
                 * Move PCA9685 channels
                 * 0, 1, 2, 3 and 4
                 * to the LOCK position.
                 */
                status =
                    lockAllServos();


                if (status == kStatus_Success)
                {
                    PRINTF(
                        "Door Locked - all 5 servos moved.\r\n"
                    );
                }
                else
                {
                    PRINTF(
                        "ERROR: Lock servo command failed.\r\n"
                    );
                }
            }


            /*********
             * UNLOCK
             *********/

            else
            {
                LED_UNLOCK();


                /*
                 * Move PCA9685 channels
                 * 0, 1, 2, 3 and 4
                 * to the UNLOCK position.
                 */
                status =
                    unlockAllServos();


                if (status == kStatus_Success)
                {
                    PRINTF(
                        "Door Unlocked - all 5 servos moved.\r\n"
                    );
                }
                else
                {
                    PRINTF(
                        "ERROR: Unlock servo command failed.\r\n"
                    );
                }
            }
        }
    }
}



















































