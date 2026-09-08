
  //RF24.h

  //Native C public interface for the nRF24L01+ / nRF24L01+PA+LNA driver
  //targeting the FRDM-KL25Z with MCUXpresso SDK.

  //Hardware mapping is defined in RF24_config.h:
    //PTA12             -> CE
    //PTA14             -> CSN (GPIO-controlled)
    //PTA15 / SPI0_SCK  -> SCK
    //PTA16 / SPI0_SIN  <- MISO
    //PTA17 / SPI0_SOUT -> MOSI
    //IRQ               -> not connected initially

  //This header replaces the original C++ RF24 class with:
    //- C enumerations
    //- an RF24_t state structure
    //- normal C function prototypes

  //This derivative remains subject to GPL-2.0 because it is based on RF24.


#ifndef RF24_H_
#define RF24_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "RF24_config.h"
#include "nRF24L01.h"

#ifdef __cplusplus
extern "C" {
#endif


// Radio configuration enumerations


  //Radio power-amplifier level.

  //For an nRF24L01+, these correspond approximately to:
  //RF24_PA_MIN  = -18 dBm
  //RF24_PA_LOW  = -12 dBm
  //RF24_PA_HIGH =  -6 dBm
  //RF24_PA_MAX  =   0 dBm
  //A PA/LNA module may produce a higher final RF output because of its external amplifier.

typedef enum
{
    RF24_PA_MIN = 0,
    RF24_PA_LOW,
    RF24_PA_HIGH,
    RF24_PA_MAX,
    RF24_PA_ERROR
} RF24_PALevel_t;

//Supported over-the-air data rates.
typedef enum
{
    RF24_DATA_RATE_1MBPS = 0,
    RF24_DATA_RATE_2MBPS,
    RF24_DATA_RATE_250KBPS
} RF24_DataRate_t;

//CRC configuration
typedef enum
{
    RF24_CRC_DISABLED = 0,
    RF24_CRC_8,
    RF24_CRC_16
} RF24_CRCLength_t;

//State of either the RX or TX FIFO.
typedef enum
{
    RF24_FIFO_OCCUPIED = 0,
    RF24_FIFO_EMPTY,
    RF24_FIFO_FULL,
    RF24_FIFO_INVALID
} RF24_FifoState_t;


// STATUS/IRQ masks


#define RF24_IRQ_NONE       0U
#define RF24_IRQ_TX_FAILED  RF24_BIT(NRF24_MASK_MAX_RT)
#define RF24_IRQ_TX_SENT    RF24_BIT(NRF24_TX_DS)
#define RF24_IRQ_RX_READY   RF24_BIT(NRF24_RX_DR)

#define RF24_IRQ_ALL \
    ((uint8_t)(RF24_IRQ_TX_FAILED | RF24_IRQ_TX_SENT | RF24_IRQ_RX_READY))

// Backward-readable aliases matching the original RF24 terminology.
#define RF24_TX_DF RF24_IRQ_TX_FAILED
#define RF24_TX_DS RF24_IRQ_TX_SENT
#define RF24_RX_DR RF24_IRQ_RX_READY


// Driver state



 //RF24 driver instance.

 //The original C++ class stored these values as private/protected members.
 //In native C, one RF24_t object is supplied to every driver function.

 //Do not modify members directly after initialization unless a function's
 //documentation explicitly permits it.

typedef struct
{
    // Last STATUS byte returned by an SPI transaction.
    uint8_t status;

    // Cached CONFIG register value.
    uint8_t config_register;

    // Static payload size in bytes, from 1 through 32.
    uint8_t payload_size;

    // Configured address width in bytes, from 3 through 5.
    uint8_t address_width;

    //Last user receive address assigned to pipe 0.
    uint8_t pipe0_reading_address[5];

    // Current writing address cached for auto-acknowledgment operation.
    uint8_t pipe0_writing_address[5];

    // Requested SPI0 baud rate in hertz.
    uint32_t spi_speed_hz;

    // RX-to-TX settling delay selected from the configured data rate.
    uint32_t tx_delay_us;

    // Optional delay after CSN transitions.
    uint32_t cs_delay_us;

    // True when dynamic payload lengths are enabled.
    bool dynamic_payloads_enabled;

    // True when acknowledgment payloads are enabled.
    bool ack_payloads_enabled;

    // True when the transceiver behaves as an nRF24L01+ variant.
    bool is_plus_variant;

    // True when pipe 0 has a user-configured receive address.
    bool pipe0_is_receiving;

#if RF24_FAILURE_HANDLING
    // Set when the driver detects a radio/SPI timeout.
    bool failure_detected;

    // Number of attempted timeout recoveries.
    uint16_t failure_recovery_attempts;
#endif
} RF24_t;


// Initialization and connection


// Reset an RF24_t object to the driver's default software state.
void RF24_ResetObject(RF24_t *radio);


  //Initialize the KL25Z platform layer, SPI0, GPIO pins, and radio registers.
  //@return true when initialization and CONFIG-register verification succeed.

bool RF24_Init(RF24_t *radio);

// Check whether SETUP_AW contains the expected address-width value.
bool RF24_IsChipConnected(RF24_t *radio);


// Status and FIFO commands


// Fetch a fresh STATUS byte by transmitting the NOP command.
uint8_t RF24_UpdateStatus(RF24_t *radio);

// Return the STATUS byte cached by the most recent SPI transaction.
uint8_t RF24_GetStatus(const RF24_t *radio);

//Clear selected RX_DR, TX_DS, and MAX_RT status flags.
uint8_t RF24_ClearStatusFlags(RF24_t *radio, uint8_t flags);


  //Select which events are reflected on the physical IRQ output.

  //The current project does not connect IRQ, but the register feature remains
  //available.

bool RF24_SetStatusFlags(RF24_t *radio, uint8_t flags);

// Flush all pending RX payloads.
bool RF24_FlushRx(RF24_t *radio);

// Flush all pending TX payloads.
bool RF24_FlushTx(RF24_t *radio);


// Radio configuration


// Set RF channel 0 through 125. Values above 125 are clamped.
bool RF24_SetChannel(RF24_t *radio, uint8_t channel);

// Read the currently configured RF channel.
uint8_t RF24_GetChannel(RF24_t *radio);

// Set static payload size from 1 through 32 bytes.
bool RF24_SetPayloadSize(RF24_t *radio, uint8_t size);

// Return the cached static payload size.
uint8_t RF24_GetPayloadSize(const RF24_t *radio);

// Set radio address width to 3, 4, or 5 bytes.
bool RF24_SetAddressWidth(RF24_t *radio, uint8_t width);


  //Configure automatic retransmission.

  //Delay and count are 4-bit values. The delay represents 250 us increments beginning at 250 us.

bool RF24_SetRetries(RF24_t *radio, uint8_t delay, uint8_t count);

// Configure 250 kbps, 1 Mbps, or 2 Mbps operation.
bool RF24_SetDataRate(RF24_t *radio, RF24_DataRate_t speed);

// Read the currently configured data rate.
RF24_DataRate_t RF24_GetDataRate(RF24_t *radio);

// Configure radio PA level and the RF_SETUP LNA bit.
bool RF24_SetPALevel(
    RF24_t *radio,
    RF24_PALevel_t level,
    bool lna_enable);

// Read the configured PA level.
RF24_PALevel_t RF24_GetPALevel(RF24_t *radio);

// Enable or disable automatic acknowledgment on all six pipes.
bool RF24_SetAutoAck(RF24_t *radio, bool enable);

// Enable or disable automatic acknowledgment on one pipe.
bool RF24_SetAutoAckPipe(
    RF24_t *radio,
    uint8_t pipe,
    bool enable);

// Enable dynamic payload lengths on all six pipes.
bool RF24_EnableDynamicPayloads(RF24_t *radio);

// Disable dynamic payload lengths.
bool RF24_DisableDynamicPayloads(RF24_t *radio);

// Enable acknowledgment payloads and required dynamic-payload features.
bool RF24_EnableAckPayload(RF24_t *radio);

// Disable acknowledgment payloads.
bool RF24_DisableAckPayload(RF24_t *radio);

// Enable the per-packet no-acknowledgment SPI command.
bool RF24_EnableDynamicAck(RF24_t *radio);


// Addressing



 //Configure the TX address and pipe-0 address used for auto acknowledgment.
 // The supplied array must contain at least radio->address_width bytes.

bool RF24_OpenWritingPipe(
    RF24_t *radio,
    const uint8_t *address);


  //Open receive pipe 0 through 5.
 //Pipes 0 and 1 use the complete configured address width. Pipes 2 through 5 use address[0] and inherit the remaining bytes from pipe 1.

bool RF24_OpenReadingPipe(
    RF24_t *radio,
    uint8_t pipe,
    const uint8_t *address);

// Disable receive pipe 0 through 5.
bool RF24_CloseReadingPipe(RF24_t *radio, uint8_t pipe);

//Power and operating modes


//Set PWR_UP and wait for the radio's standby transition.
bool RF24_PowerUp(RF24_t *radio);

//Lower CE and clear PWR_UP.
bool RF24_PowerDown(RF24_t *radio);

//Enter primary receive mode and raise CE.
bool RF24_StartListening(RF24_t *radio);

//Lower CE and return to primary transmit mode.
bool RF24_StopListening(RF24_t *radio);


//Receive operations


//Return true when the RX FIFO contains a payload.
bool RF24_Available(RF24_t *radio);


 //Return true when a payload is available and optionally return its pipe.

bool RF24_AvailablePipe(
    RF24_t *radio,
    uint8_t *pipe_number);

//Read a payload and clear RX_DR.
bool RF24_Read(
    RF24_t *radio,
    void *buffer,
    uint8_t length);


 //Read the next dynamic payload width.
 //Invalid widths greater than 32 cause the RX FIFO to be flushed.

uint8_t RF24_GetDynamicPayloadSize(RF24_t *radio);


//Transmit operations



 //Load a payload into the TX FIFO and optionally raise CE.
 //no_ack requires RF24_EnableDynamicAck() to have been called.

bool RF24_StartFastWrite(
    RF24_t *radio,
    const void *buffer,
    uint8_t length,
    bool no_ack,
    bool start_tx);

//Transmit one payload using normal acknowledgment behavior.
bool RF24_Write(
    RF24_t *radio,
    const void *buffer,
    uint8_t length);

//Transmit one payload and optionally use W_TX_PAYLOAD_NO_ACK.

bool RF24_WriteNoAck(
    RF24_t *radio,
    const void *buffer,
    uint8_t length,
    bool no_ack);

//Load an acknowledgment payload for receive pipe 0 through 5.
bool RF24_WriteAckPayload(
    RF24_t *radio,
    uint8_t pipe,
    const void *buffer,
    uint8_t length);


//FIFO and RF diagnostics


//Return true when all three RX FIFO levels are occupied.
bool RF24_RxFifoFull(RF24_t *radio);


 //Return the state of the TX FIFO when tx_fifo is true, otherwise RX FIFO.

RF24_FifoState_t RF24_GetFifoState(
    RF24_t *radio,
    bool tx_fifo);

//Read the received-power detector/carrier-detect state.
bool RF24_TestCarrier(RF24_t *radio);

//Return whether initialization identified a plus-compatible variant.
bool RF24_IsPlusVariant(const RF24_t *radio);

//Return retransmission count for the most recent transmitted payload.
uint8_t RF24_GetARC(RF24_t *radio);

#ifdef __cplusplus
}
#endif

#endif
