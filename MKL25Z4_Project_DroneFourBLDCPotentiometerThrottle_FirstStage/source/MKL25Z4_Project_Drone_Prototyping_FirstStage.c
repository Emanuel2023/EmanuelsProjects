#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"








#define TPM_MOD_VALUE      7500U   // 20 ms period (50 Hz)
#define MOTOR_MIN_PULSE    375U    // 1 ms pulse (Armed/Idle)
#define MOTOR_MAX_PULSE    750U    // 2 ms pulse (Full Throttle)
#define ARMING_DELAY       7000000U
#define ADC_MAX_VALUE      4095U
#define ADC_CHANNEL_PTB0   8U

void delay(volatile uint32_t cycles)
{
    while (cycles--)
    {
        __asm("nop");
    }
}

int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /*
     * 1. Enable clocks:
     *
     * PORTB: PTB0 potentiometer ADC input
     * PORTC: PTC2 PWM output
     * PORTD: PTD1 blue status LED
     * PORTE: PTE20, PTE21, PTE29 PWM outputs
     *
     * ADC0: potentiometer conversion
     * TPM0: PTC2 and PTE29
     * TPM1: PTE20 and PTE21
     */
    SIM->SCGC5 |=
        SIM_SCGC5_PORTB_MASK |
        SIM_SCGC5_PORTC_MASK |
        SIM_SCGC5_PORTD_MASK |
        SIM_SCGC5_PORTE_MASK;

    SIM->SCGC6 |=
        SIM_SCGC6_ADC0_MASK |
        SIM_SCGC6_TPM0_MASK |
        SIM_SCGC6_TPM1_MASK;

    /*
     * Select the 48 MHz MCGFLLCLK source for TPM0 and TPM1.
     */
    SIM->SOPT2 =
        (SIM->SOPT2 & ~SIM_SOPT2_TPMSRC_MASK) |
        SIM_SOPT2_TPMSRC(1);

    /*
     * 2. Pin setup
     */

    // PTB0 as ADC0_SE8 analog input
    PORTB->PCR[0] = PORT_PCR_MUX(0);

    // PTE20 as TPM1_CH0, ALT3
    PORTE->PCR[20] =
        (PORTE->PCR[20] & ~PORT_PCR_MUX_MASK) |
        PORT_PCR_MUX(3);

    // PTE21 as TPM1_CH1, ALT3
    PORTE->PCR[21] =
        (PORTE->PCR[21] & ~PORT_PCR_MUX_MASK) |
        PORT_PCR_MUX(3);

    // PTE29 as TPM0_CH2, ALT3
    PORTE->PCR[29] =
        (PORTE->PCR[29] & ~PORT_PCR_MUX_MASK) |
        PORT_PCR_MUX(3);

    // PTC2 as TPM0_CH1, ALT4
    PORTC->PCR[2] =
        (PORTC->PCR[2] & ~PORT_PCR_MUX_MASK) |
        PORT_PCR_MUX(4);

    // PTD1 blue LED as GPIO output
    PORTD->PCR[1] = PORT_PCR_MUX(1);
    PTD->PDDR |= (1UL << 1);

    /*
     * The KL25Z RGB LED is active-low.
     * Start with the blue LED off.
     */
    PTD->PSOR = (1UL << 1);

    /*
     * 3. ADC setup
     *
     * MODE = 1: 12-bit single-ended conversion
     * ADIV = 1: divide ADC input clock by 2
     */
    ADC0->CFG1 =
        ADC_CFG1_MODE(1) |
        ADC_CFG1_ADIV(1);

    /*
     * 4. Configure TPM0
     *
     * TPM0_CH1: PTC2
     * TPM0_CH2: PTE29
     */
    TPM0->SC = 0U;
    TPM0->CNT = 0U;
    TPM0->MOD = TPM_MOD_VALUE - 1U;

    TPM0->CONTROLS[1].CnSC =
        TPM_CnSC_MSB_MASK |
        TPM_CnSC_ELSB_MASK;

    TPM0->CONTROLS[2].CnSC =
        TPM_CnSC_MSB_MASK |
        TPM_CnSC_ELSB_MASK;

    /*
     * 5. Configure TPM1
     *
     * TPM1_CH0: PTE20
     * TPM1_CH1: PTE21
     */
    TPM1->SC = 0U;
    TPM1->CNT = 0U;
    TPM1->MOD = TPM_MOD_VALUE - 1U;

    TPM1->CONTROLS[0].CnSC =
        TPM_CnSC_MSB_MASK |
        TPM_CnSC_ELSB_MASK;

    TPM1->CONTROLS[1].CnSC =
        TPM_CnSC_MSB_MASK |
        TPM_CnSC_ELSB_MASK;

    /*
     * 6. ESC arming sequence
     *
     * Set every output to minimum throttle before starting
     * either timer.
     */
    PRINTF("Arming all ESCs... Ensure potentiometer is at ZERO.\r\n");

    // TPM0 outputs
    TPM0->CONTROLS[1].CnV = MOTOR_MIN_PULSE; // PTC2
    TPM0->CONTROLS[2].CnV = MOTOR_MIN_PULSE; // PTE29

    // TPM1 outputs
    TPM1->CONTROLS[0].CnV = MOTOR_MIN_PULSE; // PTE20
    TPM1->CONTROLS[1].CnV = MOTOR_MIN_PULSE; // PTE21

    /*
     * Start both timers:
     *
     * CMOD = 1: TPM increments from the selected clock
     * PS = 7: divide clock by 128
     */
    TPM0->SC =
        TPM_SC_CMOD(1) |
        TPM_SC_PS(0x07);

    TPM1->SC =
        TPM_SC_CMOD(1) |
        TPM_SC_PS(0x07);

    delay(ARMING_DELAY);

    PRINTF("All ESCs armed.\r\n");
    PRINTF("Potentiometer now controls all four outputs.\r\n");

    while (1)
    {
        /*
         * Toggle the blue LED to indicate that the program
         * is still reading the ADC and updating the PWM.
         */
        PTD->PTOR = (1UL << 1);

        /*
         * Start ADC conversion on ADC0 channel 8, PTB0.
         */
        ADC0->SC1[0] = ADC_CHANNEL_PTB0;

        while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK))
        {
            // Wait for conversion to finish
        }

        uint16_t adc_res = (uint16_t)ADC0->R[0];

        /*
         * Map ADC range 0-4095 into PWM range 375-750.
         *
         * The uint32_t cast prevents overflow during
         * multiplication.
         */
        uint32_t throttle =
            MOTOR_MIN_PULSE +
            (((uint32_t)adc_res *
              (MOTOR_MAX_PULSE - MOTOR_MIN_PULSE)) /
             ADC_MAX_VALUE);

        /*
         * Apply the same throttle command to all four ESCs.
         */

        // TPM0 outputs
        TPM0->CONTROLS[1].CnV = throttle; // PTC2
        TPM0->CONTROLS[2].CnV = throttle; // PTE29

        // TPM1 outputs
        TPM1->CONTROLS[0].CnV = throttle; // PTE20
        TPM1->CONTROLS[1].CnV = throttle; // PTE21

        PRINTF(
            "ADC: %u -> PWM: %u | "
            "PTE20, PTE21, PTE29, PTC2 updated\r\n",
            (unsigned int)adc_res,
            (unsigned int)throttle);

        /*
         * Small delay to reduce terminal flooding.
         */
        for (volatile uint32_t i = 0; i < 500000U; i++)
        {
            __asm("nop");
        }
    }
}










































































































































// The setup routine runs once when you press reset:
/* --- Configuration Macros --- */
//int main(void) {
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    // 1. Power on everything
//    SIM->SCGC5 |= (SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK);
//    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;
//
//    // 2. Setup Pins
//    PORTB->PCR[1] = PORT_PCR_MUX(0); // PTB1 as Analog
//    PORTD->PCR[1] = PORT_PCR_MUX(1); // PTD1 (Blue LED) as GPIO
//    PTD->PDDR |= (1 << 1);           // Set Blue LED as Output
//
////    // 3. Setup ADC
//    ADC0->CFG1 = ADC_CFG1_MODE(1) | ADC_CFG1_ADIV(1);
////
//    PRINTF("Starting ADC Polling on PTB1...\r\n");
////
//    while (1) {
////        // Toggle Blue LED - if this stops blinking, the ADC is stuck
//        PTD->PTOR = (1 << 1);
////
//        ADC0->SC1[0] = 9U; // Trigger PTB1
////
////        // If it hangs here, the ADC clock isn't reaching the module
//        while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));
////
//        uint16_t res = (uint16_t)ADC0->R[0];
//        PRINTF("Raw Value: %d\r\n", res);
////
//        for (volatile int i = 0; i < 1000000; i++);
//    }
//}

















//#define TPM_MOD_VALUE      7500    // 20 ms period (50Hz)
//#define MOTOR_MIN_PULSE    375     // 1 ms pulse (Throttle OFF / ARMED)
//#define MOTOR_MAX_PULSE    750     // 2 ms pulse (Full Throttle)
//#define ARMING_DELAY       7000000 // Delay for ESC to recognize zero throttle
//
//void delay(volatile uint32_t cycles) {
//    while (cycles--) {
//        __asm("nop");
//    }
//}
//
//int main(void) {
//    /* Initialize board hardware */
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    /* 1. Enable Clock to Port D and TPM0 */
//    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
//    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
//    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // Select MCGFLLCLK (48MHz)
//
//    /* 2. Configure PTD0 as TPM0_CH0 (ALT4) */
//    PORTD->PCR[0] = (PORTD->PCR[0] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(4);
//
//    /* 3. Configure TPM0 for 50Hz Edge-Aligned PWM */
//    TPM0->SC = 0; // Disable timer for config
//    TPM0->MOD = TPM_MOD_VALUE - 1;
//    TPM0->CONTROLS[0].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
//
//    /* 4. ARMING SEQUENCE - CRITICAL FOR BLDC */
//    PRINTF("Initializing ESC... Hold for Arming Tones.\r\n");
//    TPM0->CONTROLS[0].CnV = MOTOR_MIN_PULSE; // Set to 1.0ms
//    TPM0->SC = TPM_SC_CMOD(1) | TPM_SC_PS(0x07); // Start timer (Prescaler 128)
//
//    delay(ARMING_DELAY); // Wait for the "Long Beep" from the ESC
//    PRINTF("ESC Armed. Use 'u' to increase, 'd' to decrease, 's' to stop.\r\n");
//
//    uint32_t current_throttle = MOTOR_MIN_PULSE;
//
//    while (1) {
//        char input = GETCHAR();
//
//        if (input == 'u' && current_throttle < MOTOR_MAX_PULSE) {
//            current_throttle += 10; // Increment speed
//        } else if (input == 'd' && current_throttle > MOTOR_MIN_PULSE) {
//            current_throttle -= 10; // Decrement speed
//        } else if (input == 's') {
//            current_throttle = MOTOR_MIN_PULSE; // Emergency Stop
//            PRINTF("!!! MOTOR STOPPED !!!\r\n");
//        }
//
//        TPM0->CONTROLS[0].CnV = current_throttle;
//        PRINTF("Current Throttle Value: %d\r\n", current_throttle);
//    }
//
//    return 0;
//}



















//PTE20 Test

//#define TPM_MOD_VALUE      7500    // 20 ms period (50Hz)
//#define MOTOR_MIN_PULSE    375     // 1 ms pulse (Throttle OFF / ARMED)
//#define MOTOR_MAX_PULSE    750     // 2 ms pulse (Full Throttle)
//#define ARMING_DELAY       7000000 // Delay for ESC to recognize zero throttle
//
//void delay(volatile uint32_t cycles) {
//    while (cycles--) {
//        __asm("nop");
//    }
//}
//
//int main(void) {
//    /* Initialize board hardware */
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    /* 1. Enable Clock to Port E and TPM1 */
//    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
//    SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
//    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // Select MCGFLLCLK (48MHz)
//
//    /* 2. Configure PTE20 as TPM1_CH0 (ALT3) */
//    PORTE->PCR[20] =
//        (PORTE->PCR[20] & ~PORT_PCR_MUX_MASK) |
//        PORT_PCR_MUX(3);
//
//    /* 3. Configure TPM1 for 50Hz Edge-Aligned PWM */
//    TPM1->SC = 0; // Disable timer for config
//    TPM1->MOD = TPM_MOD_VALUE - 1;
//    TPM1->CONTROLS[0].CnSC =
//        TPM_CnSC_MSB_MASK |
//        TPM_CnSC_ELSB_MASK;
//
//    /* 4. ARMING SEQUENCE - CRITICAL FOR BLDC */
//    PRINTF("Initializing ESC... Hold for Arming Tones.\r\n");
//
//    TPM1->CONTROLS[0].CnV = MOTOR_MIN_PULSE; // Set to 1.0ms
//
//    TPM1->SC =
//        TPM_SC_CMOD(1) |
//        TPM_SC_PS(0x07); // Start timer (Prescaler 128)
//
//    delay(ARMING_DELAY); // Wait for the "Long Beep" from the ESC
//
//    PRINTF("ESC Armed. Use 'u' to increase, 'd' to decrease, 's' to stop.\r\n");
//
//    uint32_t current_throttle = MOTOR_MIN_PULSE;
//
//    while (1) {
//        char input = GETCHAR();
//
//        if (input == 'u' && current_throttle < MOTOR_MAX_PULSE) {
//            current_throttle += 10; // Increment speed
//        } else if (input == 'd' && current_throttle > MOTOR_MIN_PULSE) {
//            current_throttle -= 10; // Decrement speed
//        } else if (input == 's') {
//            current_throttle = MOTOR_MIN_PULSE; // Emergency Stop
//            PRINTF("!!! MOTOR STOPPED !!!\r\n");
//        }
//
//        TPM1->CONTROLS[0].CnV = current_throttle;
//
//        PRINTF("Current Throttle Value: %d\r\n",
//               current_throttle);
//    }
//
//    return 0;
//}


















//PTE29 Test

//#define TPM_MOD_VALUE      7500    // 20 ms period (50Hz)
//#define MOTOR_MIN_PULSE    375     // 1 ms pulse (Throttle OFF / ARMED)
//#define MOTOR_MAX_PULSE    750     // 2 ms pulse (Full Throttle)
//#define ARMING_DELAY       7000000 // Delay for ESC to recognize zero throttle
//
//void delay(volatile uint32_t cycles) {
//    while (cycles--) {
//        __asm("nop");
//    }
//}
//
//int main(void) {
//    /* Initialize board hardware */
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    /* 1. Enable Clock to Port E and TPM0 */
//    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
//    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
//    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // Select MCGFLLCLK (48MHz)
//
//    /* 2. Configure PTE29 as TPM0_CH2 (ALT3) */
//    PORTE->PCR[29] =
//        (PORTE->PCR[29] & ~PORT_PCR_MUX_MASK) |
//        PORT_PCR_MUX(3);
//
//    /* 3. Configure TPM0 for 50Hz Edge-Aligned PWM */
//    TPM0->SC = 0; // Disable timer for config
//    TPM0->MOD = TPM_MOD_VALUE - 1;
//    TPM0->CONTROLS[2].CnSC =
//        TPM_CnSC_MSB_MASK |
//        TPM_CnSC_ELSB_MASK;
//
//    /* 4. ARMING SEQUENCE - CRITICAL FOR BLDC */
//    PRINTF("Initializing ESC... Hold for Arming Tones.\r\n");
//    TPM0->CONTROLS[2].CnV = MOTOR_MIN_PULSE; // Set to 1.0ms
//    TPM0->SC = TPM_SC_CMOD(1) | TPM_SC_PS(0x07); // Start timer (Prescaler 128)
//
//    delay(ARMING_DELAY); // Wait for the "Long Beep" from the ESC
//    PRINTF("ESC Armed. Use 'u' to increase, 'd' to decrease, 's' to stop.\r\n");
//
//    uint32_t current_throttle = MOTOR_MIN_PULSE;
//
//    while (1) {
//        char input = GETCHAR();
//
//        if (input == 'u' && current_throttle < MOTOR_MAX_PULSE) {
//            current_throttle += 10; // Increment speed
//        } else if (input == 'd' && current_throttle > MOTOR_MIN_PULSE) {
//            current_throttle -= 10; // Decrement speed
//        } else if (input == 's') {
//            current_throttle = MOTOR_MIN_PULSE; // Emergency Stop
//            PRINTF("!!! MOTOR STOPPED !!!\r\n");
//        }
//
//        TPM0->CONTROLS[2].CnV = current_throttle;
//        PRINTF("Current Throttle Value: %d\r\n", current_throttle);
//    }
//
//    return 0;
//}



//PTE21

//#define TPM_MOD_VALUE      7500    // 20 ms period (50Hz)
//#define MOTOR_MIN_PULSE    375     // 1 ms pulse (Throttle OFF / ARMED)
//#define MOTOR_MAX_PULSE    750     // 2 ms pulse (Full Throttle)
//#define ARMING_DELAY       7000000 // Delay for ESC to recognize zero throttle
//
//void delay(volatile uint32_t cycles) {
//    while (cycles--) {
//        __asm("nop");
//    }
//}
//
//int main(void) {
//    /* Initialize board hardware */
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    /* 1. Enable Clock to Port E and TPM1 */
//    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
//    SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
//    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // Select MCGFLLCLK (48MHz)
//
//    /* 2. Configure PTE21 as TPM1_CH1 (ALT3) */
//    PORTE->PCR[21] =
//        (PORTE->PCR[21] & ~PORT_PCR_MUX_MASK) |
//        PORT_PCR_MUX(3);
//
//    /* 3. Configure TPM1 for 50Hz Edge-Aligned PWM */
//    TPM1->SC = 0; // Disable timer for config
//    TPM1->MOD = TPM_MOD_VALUE - 1;
//    TPM1->CONTROLS[1].CnSC =
//        TPM_CnSC_MSB_MASK |
//        TPM_CnSC_ELSB_MASK;
//
//    /* 4. ARMING SEQUENCE - CRITICAL FOR BLDC */
//    PRINTF("Initializing ESC... Hold for Arming Tones.\r\n");
//
//    TPM1->CONTROLS[1].CnV = MOTOR_MIN_PULSE; // Set to 1.0ms
//
//    TPM1->SC =
//        TPM_SC_CMOD(1) |
//        TPM_SC_PS(0x07); // Start timer (Prescaler 128)
//
//    delay(ARMING_DELAY); // Wait for the "Long Beep" from the ESC
//
//    PRINTF("ESC Armed. Use 'u' to increase, 'd' to decrease, 's' to stop.\r\n");
//
//    uint32_t current_throttle = MOTOR_MIN_PULSE;
//
//    while (1) {
//        char input = GETCHAR();
//
//        if (input == 'u' && current_throttle < MOTOR_MAX_PULSE) {
//            current_throttle += 10; // Increment speed
//        } else if (input == 'd' && current_throttle > MOTOR_MIN_PULSE) {
//            current_throttle -= 10; // Decrement speed
//        } else if (input == 's') {
//            current_throttle = MOTOR_MIN_PULSE; // Emergency Stop
//            PRINTF("!!! MOTOR STOPPED !!!\r\n");
//        }
//
//        TPM1->CONTROLS[1].CnV = current_throttle;
//        PRINTF("Current Throttle Value: %d\r\n", current_throttle);
//    }
//
//    return 0;
//}







//PTC2 Test


//#define TPM_MOD_VALUE      7500    // 20 ms period (50Hz)
//#define MOTOR_MIN_PULSE    375     // 1 ms pulse (Throttle OFF / ARMED)
//#define MOTOR_MAX_PULSE    750     // 2 ms pulse (Full Throttle)
//#define ARMING_DELAY       7000000 // Delay for ESC to recognize zero throttle
//
//void delay(volatile uint32_t cycles) {
//    while (cycles--) {
//        __asm("nop");
//    }
//}
//
//int main(void) {
//    /* Initialize board hardware */
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    /* 1. Enable Clock to Port C and TPM0 */
//    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
//    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
//    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // Select MCGFLLCLK (48MHz)
//
//    /* 2. Configure PTC2 as TPM0_CH1 (ALT4) */
//    PORTC->PCR[2] =
//        (PORTC->PCR[2] & ~PORT_PCR_MUX_MASK) |
//        PORT_PCR_MUX(4);
//
//    /* 3. Configure TPM0 for 50Hz Edge-Aligned PWM */
//    TPM0->SC = 0; // Disable timer for config
//    TPM0->MOD = TPM_MOD_VALUE - 1;
//    TPM0->CONTROLS[1].CnSC =
//        TPM_CnSC_MSB_MASK |
//        TPM_CnSC_ELSB_MASK;
//
//    /* 4. ARMING SEQUENCE - CRITICAL FOR BLDC */
//    PRINTF("Initializing ESC... Hold for Arming Tones.\r\n");
//    TPM0->CONTROLS[1].CnV = MOTOR_MIN_PULSE; // Set to 1.0ms
//    TPM0->SC = TPM_SC_CMOD(1) | TPM_SC_PS(0x07); // Start timer (Prescaler 128)
//
//    delay(ARMING_DELAY); // Wait for the "Long Beep" from the ESC
//    PRINTF("ESC Armed. Use 'u' to increase, 'd' to decrease, 's' to stop.\r\n");
//
//    uint32_t current_throttle = MOTOR_MIN_PULSE;
//
//    while (1) {
//        char input = GETCHAR();
//
//        if (input == 'u' && current_throttle < MOTOR_MAX_PULSE) {
//            current_throttle += 10; // Increment speed
//        } else if (input == 'd' && current_throttle > MOTOR_MIN_PULSE) {
//            current_throttle -= 10; // Decrement speed
//        } else if (input == 's') {
//            current_throttle = MOTOR_MIN_PULSE; // Emergency Stop
//            PRINTF("!!! MOTOR STOPPED !!!\r\n");
//        }
//
//        TPM0->CONTROLS[1].CnV = current_throttle;
//        PRINTF("Current Throttle Value: %d\r\n", current_throttle);
//    }
//
//    return 0;
//}


























































//PTB1 Pot Throttle FRIED



//#define TPM_MOD_VALUE      7500    // 20 ms period (50Hz)
//#define MOTOR_MIN_PULSE    375     // 1 ms pulse (Armed/Idle)
//#define MOTOR_MAX_PULSE    750     // 2 ms pulse (Full Throttle)
//#define ARMING_DELAY       7000000
//
//void delay(volatile uint32_t cycles) {
//    while (cycles--) { __asm("nop"); }
//}
//
//int main(void) {
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    // 1. Power on Port B (ADC), Port D (PWM & LED), and Timer
//    SIM->SCGC5 |= (SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK);
//    SIM->SCGC6 |= (SIM_SCGC6_ADC0_MASK | SIM_SCGC6_TPM0_MASK);
//    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // 48MHz clock source
//
//    // 2. Pin Setup
//    PORTB->PCR[1] = PORT_PCR_MUX(0); // PTB1 as Analog Input
//    PORTD->PCR[0] = PORT_PCR_MUX(4); // PTD0 as TPM0_CH0 (PWM Output)
//    PORTD->PCR[1] = PORT_PCR_MUX(1); // PTD1 (Blue LED) as GPIO
//    PTD->PDDR |= (1 << 1);
//
//    // 3. ADC Setup (12-bit mode)
//    ADC0->CFG1 = ADC_CFG1_MODE(1) | ADC_CFG1_ADIV(1);
//
//    // 4. TPM PWM Setup
//    TPM0->SC = 0;
//    TPM0->MOD = TPM_MOD_VALUE - 1;
//    TPM0->CONTROLS[0].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
//
//    // 5. Mandatory Arming Sequence
//    PRINTF("Arming ESC... Ensure potentiometer is at ZERO.\r\n");
//    TPM0->CONTROLS[0].CnV = MOTOR_MIN_PULSE;
//    TPM0->SC = TPM_SC_CMOD(1) | TPM_SC_PS(0x07); // Prescaler 128
//    delay(ARMING_DELAY);
//    PRINTF("ESC Armed! Potentiometer now controlling motor.\r\n");
//
//    while (1) {
//        // Toggle Blue LED to show the loop is alive
//        PTD->PTOR = (1 << 1);
//
//        // Start ADC conversion on Channel 9 (PTB1)
//        ADC0->SC1[0] = 9U;
//        while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));
//        uint16_t adc_res = (uint16_t)ADC0->R[0];
//
//        // Map 0-4095 to 375-750
//        // We use 375 as the range (750 - 375)
//        uint32_t throttle = MOTOR_MIN_PULSE + ((adc_res * 375) / 4095);
//
//        // Update PWM Register
//        TPM0->CONTROLS[0].CnV = throttle;
//
//        PRINTF("ADC: %d -> PWM CnV: %d\r\n", adc_res, throttle);
//
//        // Small delay to prevent terminal flooding
//        for (volatile int i = 0; i < 500000; i++);
//    }
//}








//PTB0 Throttle Pot Success

//#define TPM_MOD_VALUE      7500    // 20 ms period (50Hz)
//#define MOTOR_MIN_PULSE    375     // 1 ms pulse (Armed/Idle)
//#define MOTOR_MAX_PULSE    750     // 2 ms pulse (Full Throttle)
//#define ARMING_DELAY       7000000
//
//void delay(volatile uint32_t cycles) {
//    while (cycles--) { __asm("nop"); }
//}
//
//int main(void) {
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitDebugConsole();
//
//    // 1. Power on Port B (ADC), Port D (PWM & LED), and Timer
//    SIM->SCGC5 |= (SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK);
//    SIM->SCGC6 |= (SIM_SCGC6_ADC0_MASK | SIM_SCGC6_TPM0_MASK);
//    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // 48MHz clock source
//
//    // 2. Pin Setup
//    PORTB->PCR[0] = PORT_PCR_MUX(0); // PTB0 as Analog Input
//    PORTD->PCR[0] = PORT_PCR_MUX(4); // PTD0 as TPM0_CH0 (PWM Output)
//    PORTD->PCR[1] = PORT_PCR_MUX(1); // PTD1 (Blue LED) as GPIO
//    PTD->PDDR |= (1 << 1);
//
//    // 3. ADC Setup (12-bit mode)
//    ADC0->CFG1 = ADC_CFG1_MODE(1) | ADC_CFG1_ADIV(1);
//
//    // 4. TPM PWM Setup
//    TPM0->SC = 0;
//    TPM0->MOD = TPM_MOD_VALUE - 1;
//    TPM0->CONTROLS[0].CnSC =
//        TPM_CnSC_MSB_MASK |
//        TPM_CnSC_ELSB_MASK;
//
//    // 5. Mandatory Arming Sequence
//    PRINTF("Arming ESC... Ensure potentiometer is at ZERO.\r\n");
//    TPM0->CONTROLS[0].CnV = MOTOR_MIN_PULSE;
//    TPM0->SC = TPM_SC_CMOD(1) | TPM_SC_PS(0x07); // Prescaler 128
//    delay(ARMING_DELAY);
//
//    PRINTF("ESC Armed! Potentiometer now controlling motor.\r\n");
//
//    while (1) {
//        // Toggle Blue LED to show the loop is alive
//        PTD->PTOR = (1 << 1);
//
//        // Start ADC conversion on Channel 8 (PTB0)
//        ADC0->SC1[0] = 8U;
//
//        while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));
//
//        uint16_t adc_res = (uint16_t)ADC0->R[0];
//
//        // Map ADC range 0-4095 to PWM range 375-750
//        uint32_t throttle =
//            MOTOR_MIN_PULSE +
//            (((uint32_t)adc_res *
//              (MOTOR_MAX_PULSE - MOTOR_MIN_PULSE)) / 4095U);
//
//        // Update PWM register
//        TPM0->CONTROLS[0].CnV = throttle;
//
//        PRINTF("ADC: %u -> PWM CnV: %u\r\n",
//               (unsigned int)adc_res,
//               (unsigned int)throttle);
//
//        // Small delay to prevent terminal flooding
//        for (volatile int i = 0; i < 500000; i++);
//    }
//}
//















