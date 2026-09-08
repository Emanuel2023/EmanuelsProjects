
 //Shared packet definition for the KL25Z potentiometer-to-LED radio test.
 //Include this exact file in both the transmitter and receiver projects.


#ifndef RF24_PACKETS_H_
#define RF24_PACKETS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t potentiometer; // 12-bit ADC value: 0 through 4095
    uint16_t sequence;      // Increments once per transmitted packet
} RF24_ControlPacket_t;

#ifdef __cplusplus
}
#endif

#endif
