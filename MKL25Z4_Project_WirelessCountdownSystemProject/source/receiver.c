#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_tpm.h"

#include "RF24.h"
#include "RF24_config.h"
#include "RF24_packets.h"

#include <stdbool.h>
#include <stdint.h>


/* ==========================================================
 * RF24 RADIO SETTINGS
 * ========================================================== */

static const uint8_t s_radio_address[5] = {
    'D', 'R', 'O', 'N', 'E'
};


/* ==========================================================
 * ONBOARD RGB LED
 * ========================================================== */

#define RECEIVER_LED_PORT        PORTB
#define RECEIVER_LED_PIN         18U
#define RECEIVER_LED_TPM         TPM2
#define RECEIVER_LED_CHANNEL     kTPM_Chnl_0
#define RECEIVER_LED_PWM_HZ      1000U

#define RECEIVER_TIMEOUT_MS      250U


/* ==========================================================
 * COUNTDOWN SETTINGS
 * ========================================================== */

/*
 * 12-bit ADC:
 *
 * 0 - 4095
 *
 * Countdown starts when the received ADC value
 * reaches 4000 or greater.
 */

#define COUNTDOWN_TRIGGER_ADC    4000U

/*
 * 10 seconds = 10,000 milliseconds
 */

#define COUNTDOWN_START_MS       10000U


/* ==========================================================
 * SEVEN-SEGMENT DISPLAY PINS
 * ========================================================== */

/*
 * Segment wiring:
 *
 * A  -> PTE20
 * B  -> PTE22
 * C  -> PTD5
 * D  -> PTA16
 * E  -> PTC17
 * F  -> PTE21
 * G  -> PTE23
 * DP -> PTA17
 */

#define SEG_A       20U     /* PTE20 */
#define SEG_B       22U     /* PTE22 */
#define SEG_C       5U      /* PTD5  */
#define SEG_D       16U     /* PTA16 */
#define SEG_E       17U     /* PTC17 */
#define SEG_F       21U     /* PTE21 */
#define SEG_G       23U     /* PTE23 */
#define SEG_DP      17U     /* PTA17 */


/*
 * Digit select pins
 */

#define DIGIT1      9U      /* PTC9  */
#define DIGIT2      8U      /* PTC8  */
#define DIGIT3      11U     /* PTC11 */
#define DIGIT4      10U     /* PTC10 */


/* ==========================================================
 * SEVEN-SEGMENT PATTERNS
 * ========================================================== */

/*
 * Bit order:
 *
 * bit 0 = A
 * bit 1 = B
 * bit 2 = C
 * bit 3 = D
 * bit 4 = E
 * bit 5 = F
 * bit 6 = G
 */

static const uint8_t digit_segments[10] =
{
    0b00111111,     /* 0 */
    0b00000110,     /* 1 */
    0b01011011,     /* 2 */
    0b01001111,     /* 3 */
    0b01100110,     /* 4 */
    0b01101101,     /* 5 */
    0b01111101,     /* 6 */
    0b00000111,     /* 7 */
    0b01111111,     /* 8 */
    0b01101111      /* 9 */
};


/* ==========================================================
 * FATAL ERROR
 * ========================================================== */

static void FatalError(const char *message)
{
    PRINTF("ERROR: %s\r\n", message);

    while (true)
    {
        __NOP();
    }
}


/* ==========================================================
 * ONBOARD LED INITIALIZATION
 * ========================================================== */

static bool ReceiverLED_Init(void)
{
    tpm_config_t tpm_config;

    tpm_chnl_pwm_signal_param_t pwm_param = {
        .chnlNumber = RECEIVER_LED_CHANNEL,
        .level = kTPM_LowTrue,
        .dutyCyclePercent = 0U
    };

    uint32_t tpm_source_clock_hz;


    CLOCK_EnableClock(kCLOCK_PortB);


    /*
     * PTB18 -> TPM2_CH0
     */

    PORT_SetPinMux(
        RECEIVER_LED_PORT,
        RECEIVER_LED_PIN,
        kPORT_MuxAlt3);


    CLOCK_SetTpmClock(1U);


    tpm_source_clock_hz =
        CLOCK_GetFreq(kCLOCK_PllFllSelClk);


    if (tpm_source_clock_hz == 0U)
    {
        return false;
    }


    TPM_GetDefaultConfig(&tpm_config);


    TPM_Init(
        RECEIVER_LED_TPM,
        &tpm_config);


    if (TPM_SetupPwm(
            RECEIVER_LED_TPM,
            &pwm_param,
            1U,
            kTPM_EdgeAlignedPwm,
            RECEIVER_LED_PWM_HZ,
            tpm_source_clock_hz)
        != kStatus_Success)
    {
        return false;
    }


    TPM_StartTimer(
        RECEIVER_LED_TPM,
        kTPM_SystemClock);


    return true;
}


/* ==========================================================
 * CONTROL LED FROM RECEIVED ADC
 * ========================================================== */

static void ReceiverLED_SetFromADC(uint16_t adc_value)
{
    uint32_t duty_cycle;


    if (adc_value > 4095U)
    {
        adc_value = 4095U;
    }


    duty_cycle =
        ((uint32_t)adc_value * 100U) / 4095U;


    TPM_UpdatePwmDutycycle(
        RECEIVER_LED_TPM,
        RECEIVER_LED_CHANNEL,
        kTPM_EdgeAlignedPwm,
        (uint8_t)duty_cycle);
}


/* ==========================================================
 * TURN ONBOARD LED OFF
 * ========================================================== */

static void ReceiverLED_Off(void)
{
    TPM_UpdatePwmDutycycle(
        RECEIVER_LED_TPM,
        RECEIVER_LED_CHANNEL,
        kTPM_EdgeAlignedPwm,
        0U);
}


/* ==========================================================
 * SEVEN-SEGMENT INITIALIZATION
 * ========================================================== */

static void SevenSegment_Init(void)
{
    /*
     * Enable GPIO port clocks.
     */

    CLOCK_EnableClock(kCLOCK_PortA);
    CLOCK_EnableClock(kCLOCK_PortC);
    CLOCK_EnableClock(kCLOCK_PortD);
    CLOCK_EnableClock(kCLOCK_PortE);


    /* ------------------------------------------------------
     * Segment pins
     * ------------------------------------------------------ */

    PORTE->PCR[SEG_A] =
        PORT_PCR_MUX(1);

    PORTE->PCR[SEG_B] =
        PORT_PCR_MUX(1);

    PORTD->PCR[SEG_C] =
        PORT_PCR_MUX(1);

    PORTA->PCR[SEG_D] =
        PORT_PCR_MUX(1);

    PORTC->PCR[SEG_E] =
        PORT_PCR_MUX(1);

    PORTE->PCR[SEG_F] =
        PORT_PCR_MUX(1);

    PORTE->PCR[SEG_G] =
        PORT_PCR_MUX(1);

    PORTA->PCR[SEG_DP] =
        PORT_PCR_MUX(1);


    /* ------------------------------------------------------
     * Digit-select pins
     * ------------------------------------------------------ */

    PORTC->PCR[DIGIT1] =
        PORT_PCR_MUX(1);

    PORTC->PCR[DIGIT2] =
        PORT_PCR_MUX(1);

    PORTC->PCR[DIGIT3] =
        PORT_PCR_MUX(1);

    PORTC->PCR[DIGIT4] =
        PORT_PCR_MUX(1);


    /* ------------------------------------------------------
     * Configure outputs
     * ------------------------------------------------------ */

    GPIOE->PDDR |=
        (1U << SEG_A) |
        (1U << SEG_B) |
        (1U << SEG_F) |
        (1U << SEG_G);


    GPIOD->PDDR |=
        (1U << SEG_C);


    GPIOA->PDDR |=
        (1U << SEG_D) |
        (1U << SEG_DP);


    GPIOC->PDDR |=
        (1U << SEG_E) |
        (1U << DIGIT1) |
        (1U << DIGIT2) |
        (1U << DIGIT3) |
        (1U << DIGIT4);


    /*
     * All digit cathodes OFF initially.
     *
     * HIGH = OFF
     * LOW  = ON
     */

    GPIOC->PSOR =
        (1U << DIGIT1) |
        (1U << DIGIT2) |
        (1U << DIGIT3) |
        (1U << DIGIT4);
}


/* ==========================================================
 * DISABLE ALL DIGITS
 * ========================================================== */

static void SevenSegment_DisableDigits(void)
{
    GPIOC->PSOR =
        (1U << DIGIT1) |
        (1U << DIGIT2) |
        (1U << DIGIT3) |
        (1U << DIGIT4);
}


/* ==========================================================
 * CLEAR ALL SEGMENTS
 * ========================================================== */

static void SevenSegment_ClearSegments(void)
{
    /*
     * A, B, F, G
     */

    GPIOE->PCOR =
        (1U << SEG_A) |
        (1U << SEG_B) |
        (1U << SEG_F) |
        (1U << SEG_G);


    /*
     * C
     */

    GPIOD->PCOR =
        (1U << SEG_C);


    /*
     * D and decimal point
     */

    GPIOA->PCOR =
        (1U << SEG_D) |
        (1U << SEG_DP);


    /*
     * E
     */

    GPIOC->PCOR =
        (1U << SEG_E);
}


/* ==========================================================
 * SET SEGMENT PATTERN FOR ONE DIGIT
 * ========================================================== */

static void SevenSegment_SetDigit(
    uint8_t number,
    bool decimal_point)
{
    uint8_t pattern;


    if (number > 9U)
    {
        number = 0U;
    }


    SevenSegment_ClearSegments();


    pattern =
        digit_segments[number];


    /* A */

    if (pattern & (1U << 0))
    {
        GPIOE->PSOR =
            (1U << SEG_A);
    }


    /* B */

    if (pattern & (1U << 1))
    {
        GPIOE->PSOR =
            (1U << SEG_B);
    }


    /* C */

    if (pattern & (1U << 2))
    {
        GPIOD->PSOR =
            (1U << SEG_C);
    }


    /* D */

    if (pattern & (1U << 3))
    {
        GPIOA->PSOR =
            (1U << SEG_D);
    }


    /* E */

    if (pattern & (1U << 4))
    {
        GPIOC->PSOR =
            (1U << SEG_E);
    }


    /* F */

    if (pattern & (1U << 5))
    {
        GPIOE->PSOR =
            (1U << SEG_F);
    }


    /* G */

    if (pattern & (1U << 6))
    {
        GPIOE->PSOR =
            (1U << SEG_G);
    }


    /* Decimal point */

    if (decimal_point)
    {
        GPIOA->PSOR =
            (1U << SEG_DP);
    }
}


/* ==========================================================
 * ENABLE ONE DIGIT
 * ========================================================== */

static void SevenSegment_EnableDigit(uint8_t digit)
{
    switch (digit)
    {
        case 0U:

            GPIOC->PCOR =
                (1U << DIGIT1);

            break;


        case 1U:

            GPIOC->PCOR =
                (1U << DIGIT2);

            break;


        case 2U:

            GPIOC->PCOR =
                (1U << DIGIT3);

            break;


        case 3U:

            GPIOC->PCOR =
                (1U << DIGIT4);

            break;


        default:

            break;
    }
}


/* ==========================================================
 * REFRESH SEVEN-SEGMENT DISPLAY
 * ========================================================== */

/*
 * One digit is multiplexed approximately
 * every millisecond.
 *
 * RF24_Millis() is used instead of creating
 * another SysTick_Handler().
 */

static void SevenSegment_Refresh(uint32_t time_ms)
{
    static uint8_t current_digit = 0U;

    static uint32_t last_refresh_time = 0U;

    uint32_t now;
    uint32_t hundredths;

    uint8_t digit1;
    uint8_t digit2;
    uint8_t digit3;
    uint8_t digit4;

    uint8_t number;

    bool decimal_point;


    now =
        RF24_Millis();


    /*
     * Refresh one digit every 1 ms.
     */

    if ((now - last_refresh_time) < 1U)
    {
        return;
    }


    last_refresh_time =
        now;


    /*
     * Do not allow a displayed value
     * greater than 10.00.
     */

    if (time_ms > COUNTDOWN_START_MS)
    {
        time_ms =
            COUNTDOWN_START_MS;
    }


    hundredths =
        time_ms / 10U;


    /*
     * Exactly 10 seconds:
     *
     * 10.00
     */

    if (time_ms == COUNTDOWN_START_MS)
    {
        digit1 = 1U;
        digit2 = 0U;
        digit3 = 0U;
        digit4 = 0U;
    }

    /*
     * 0.00 through 9.99
     */

    else
    {
        digit1 = 0U;

        digit2 =
            (uint8_t)
            ((hundredths / 100U) % 10U);

        digit3 =
            (uint8_t)
            ((hundredths / 10U) % 10U);

        digit4 =
            (uint8_t)
            (hundredths % 10U);
    }


    /*
     * Turn all digits off before changing
     * segment data to reduce ghosting.
     */

    SevenSegment_DisableDigits();


    decimal_point = false;


    switch (current_digit)
    {
        case 0U:

            number = digit1;

            break;


        case 1U:

            number = digit2;

            /*
             * Decimal point after digit 2.
             *
             * Example:
             *
             * 10.00
             */

            decimal_point = true;

            break;


        case 2U:

            number = digit3;

            break;


        case 3U:

            number = digit4;

            break;


        default:

            number = 0U;

            break;
    }


    SevenSegment_SetDigit(
        number,
        decimal_point);


    SevenSegment_EnableDigit(
        current_digit);


    current_digit++;


    if (current_digit >= 4U)
    {
        current_digit = 0U;
    }
}


/* ==========================================================
 * MAIN
 * ========================================================== */

int main(void)
{
    RF24_t radio;


    RF24_ControlPacket_t packet = {
        .potentiometer = 0U,
        .sequence = 0U
    };


    uint32_t last_packet_time = 0U;

    uint16_t previous_sequence = 0U;


    /*
     * Countdown state.
     *
     * At power-up:
     *
     * countdown_active = false
     * trigger_armed     = true
     * display_time_ms   = 10000
     *
     * Therefore the display waits at 10.00.
     */

    bool countdown_active = false;

    bool trigger_armed = true;

    uint32_t countdown_start_time = 0U;

    uint32_t display_time_ms =
        COUNTDOWN_START_MS;


    /* ======================================================
     * BOARD INITIALIZATION
     * ====================================================== */

    BOARD_InitBootPins();

    BOARD_InitBootClocks();

    BOARD_InitDebugConsole();


    PRINTF(
        "\r\nKL25Z RF24 receiver + 10 second countdown\r\n");

    PRINTF(
        "Countdown trigger: ADC >= %u\r\n",
        COUNTDOWN_TRIGGER_ADC);


    /*
     * Initialize onboard LED.
     */

    if (!ReceiverLED_Init())
    {
        FatalError(
            "TPM2 LED PWM initialization failed");
    }


    /*
     * Initialize seven-segment display.
     */

    SevenSegment_Init();


    /*
     * Initialize nRF24L01+.
     */

    if (!RF24_Init(&radio))
    {
        FatalError(
            "RF24 initialization failed");
    }


    /*
     * Verify that the radio responds
     * over SPI0.
     */

    if (!RF24_IsChipConnected(&radio))
    {
        FatalError(
            "nRF24L01+ did not respond over SPI0");
    }


    /*
     * Receiver RF configuration.
     */

    if (!RF24_SetAddressWidth(
            &radio,
            5U) ||

        !RF24_SetChannel(
            &radio,
            76U) ||

        !RF24_SetDataRate(
            &radio,
            RF24_DATA_RATE_1MBPS) ||

        !RF24_SetPALevel(
            &radio,
            RF24_PA_LOW,
            true) ||

        !RF24_SetPayloadSize(
            &radio,
            (uint8_t)sizeof(packet)) ||

        !RF24_SetAutoAck(
            &radio,
            true) ||

        !RF24_SetRetries(
            &radio,
            5U,
            15U) ||

        !RF24_OpenReadingPipe(
            &radio,
            1U,
            s_radio_address) ||

        !RF24_StartListening(
            &radio))
    {
        FatalError(
            "receiver radio configuration failed");
    }


    PRINTF(
        "Radio connected. Listening for packets.\r\n");

    PRINTF(
        "Timer armed. Display = 10.00\r\n");


    last_packet_time =
        RF24_Millis();


    /* ======================================================
     * MAIN LOOP
     * ====================================================== */

    while (true)
    {
        uint32_t now;


        now =
            RF24_Millis();


        /* ==================================================
         * RECEIVE RF24 PACKET
         * ================================================== */

        if (RF24_Available(&radio))
        {
            if (RF24_Read(
                    &radio,
                    &packet,
                    (uint8_t)sizeof(packet)))
            {
                last_packet_time =
                    now;


                /*
                 * Original LED behavior remains.
                 */

                ReceiverLED_SetFromADC(
                    packet.potentiometer);


                /*
                 * Print received ADC information.
                 */

                PRINTF(
                    "SEQ=%u ADC=%u",
                    packet.sequence,
                    packet.potentiometer);


                /*
                 * Detect missed packet sequences.
                 */

                if ((uint16_t)
                    (previous_sequence + 1U)
                    != packet.sequence)
                {
                    PRINTF(" MISSED");
                }


                previous_sequence =
                    packet.sequence;


                /* ==========================================
                 * POTENTIOMETER BELOW 4000
                 * ========================================== */

                /*
                 * If the countdown is NOT currently running
                 * and the potentiometer is below 4000:
                 *
                 * 1. Re-arm the trigger.
                 * 2. Reset timer to 10 seconds.
                 * 3. Display 10.00.
                 *
                 * Therefore, after a completed countdown,
                 * turning the potentiometer back down resets
                 * the system for the next countdown.
                 */

                if (packet.potentiometer
                    < COUNTDOWN_TRIGGER_ADC)
                {
                    if (!countdown_active)
                    {
                        trigger_armed = true;

                        display_time_ms =
                            COUNTDOWN_START_MS;
                    }
                }


                /* ==========================================
                 * POTENTIOMETER >= 4000
                 * ========================================== */

                else
                {
                    /*
                     * Start the countdown only if:
                     *
                     * 1. The trigger is armed.
                     * 2. No countdown is already running.
                     */

                    if (trigger_armed &&
                        !countdown_active)
                    {
                        /*
                         * Immediately disarm the trigger.
                         *
                         * Holding the potentiometer above
                         * 4000 will therefore NOT repeatedly
                         * restart the countdown.
                         */

                        trigger_armed = false;


                        countdown_active = true;


                        countdown_start_time =
                            now;


                        display_time_ms =
                            COUNTDOWN_START_MS;


                        PRINTF(
                            " COUNTDOWN START");
                    }
                }


                PRINTF("\r\n");
            }
        }


        /* ==================================================
         * COUNTDOWN TIMER
         * ================================================== */

        if (countdown_active)
        {
            uint32_t elapsed_ms;


            elapsed_ms =
                now - countdown_start_time;


            /*
             * Countdown finished.
             */

            if (elapsed_ms >=
                COUNTDOWN_START_MS)
            {
                countdown_active =
                    false;


                /*
                 * Stay at 00.00 until the
                 * potentiometer is turned
                 * back below 4000.
                 */

                display_time_ms =
                    0U;


                PRINTF(
                    "Countdown complete: 00.00\r\n");
            }

            /*
             * Countdown is still running.
             */

            else
            {
                display_time_ms =
                    COUNTDOWN_START_MS -
                    elapsed_ms;
            }
        }


        /* ==================================================
         * SEVEN-SEGMENT MULTIPLEXING
         * ================================================== */

        SevenSegment_Refresh(
            display_time_ms);


        /* ==================================================
         * RADIO LOSS FAILSAFE
         * ================================================== */

        if ((now - last_packet_time)
            > RECEIVER_TIMEOUT_MS)
        {
            ReceiverLED_Off();
        }
    }
}
