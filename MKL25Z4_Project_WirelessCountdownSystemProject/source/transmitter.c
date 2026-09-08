#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

#include "RF24.h"
#include "RF24_config.h"
#include "RF24_packets.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Both radios must use the same address, channel, data rate, address width,
 * and payload size.
 */
static const uint8_t s_radio_address[5] = {
    'D', 'R', 'O', 'N', 'E'
};

static void FatalError(const char *message)
{
    PRINTF("ERROR: %s\r\n", message);

    while (true) {
        /*
         * Stop here so a debugger can inspect the failure.
         */
        __NOP();
    }
}

int main(void)
{
    RF24_t radio;

    RF24_ControlPacket_t packet = {
        .potentiometer = 0U,
        .sequence = 0U
    };

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\nKL25Z RF24 ground-station transmitter\r\n");
    PRINTF("Potentiometer input: PTB3 / ADC0_SE13\r\n");

    /*
     * RF24_Init() initializes SPI0, CE/CSN, timing, and the radio.
     */
    if (!RF24_Init(&radio)) {
        FatalError("RF24 initialization failed");
    }

    if (!RF24_IsChipConnected(&radio)) {
        FatalError("nRF24L01+ did not respond over SPI0");
    }

    /*
     * Initialize the PTB3 potentiometer ADC after the radio platform.
     */
    if (!RF24_GroundStationADC_Init()) {
        FatalError("ADC16 initialization failed");
    }

    /*
     * Explicitly configure matching radio parameters on both boards.
     * Start with a low PA setting because the PA/LNA module draws more current
     * than a basic nRF24L01+ module.
     */
    if (!RF24_SetAddressWidth(&radio, 5U) ||
        !RF24_SetChannel(&radio, 76U) ||
        !RF24_SetDataRate(&radio, RF24_DATA_RATE_1MBPS) ||
        !RF24_SetPALevel(&radio, RF24_PA_LOW, true) ||
        !RF24_SetPayloadSize(&radio, (uint8_t)sizeof(packet)) ||
        !RF24_SetAutoAck(&radio, true) ||
        !RF24_SetRetries(&radio, 5U, 15U) ||
        !RF24_OpenWritingPipe(&radio, s_radio_address) ||
        !RF24_StopListening(&radio)) {

        FatalError("transmitter radio configuration failed");
    }

    PRINTF("Radio connected. Beginning transmissions.\r\n");

    while (true) {
        bool sent;

        packet.potentiometer = RF24_GroundStationADC_ReadPot();
        packet.sequence++;

        sent = RF24_Write(
            &radio,
            &packet,
            (uint8_t)sizeof(packet));

        PRINTF(
            "SEQ=%u ADC=%u TX=%s\r\n",
            packet.sequence,
            packet.potentiometer,
            sent ? "OK" : "FAILED");

        /*
         * A 20 ms update interval gives a 50 Hz control packet rate.
         */
        RF24_DelayMs(20U);
    }
}





