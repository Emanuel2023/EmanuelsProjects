
 //Native C register map, bit positions, and SPI command definitions for the
 //nRF24L01 / nRF24L01+ transceiver.
 //This file is platform-independent. The KL25Z SPI0 pin mapping is defined in RF24_config.h.


#ifndef NRF24L01_H_
#define NRF24L01_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


//Register memory map


#define NRF24_CONFIG          ((uint8_t)0x00U)
#define NRF24_EN_AA           ((uint8_t)0x01U)
#define NRF24_EN_RXADDR       ((uint8_t)0x02U)
#define NRF24_SETUP_AW        ((uint8_t)0x03U)
#define NRF24_SETUP_RETR      ((uint8_t)0x04U)
#define NRF24_RF_CH           ((uint8_t)0x05U)
#define NRF24_RF_SETUP        ((uint8_t)0x06U)
#define NRF24_STATUS          ((uint8_t)0x07U)
#define NRF24_OBSERVE_TX      ((uint8_t)0x08U)
#define NRF24_CD              ((uint8_t)0x09U)
#define NRF24_RX_ADDR_P0      ((uint8_t)0x0AU)
#define NRF24_RX_ADDR_P1      ((uint8_t)0x0BU)
#define NRF24_RX_ADDR_P2      ((uint8_t)0x0CU)
#define NRF24_RX_ADDR_P3      ((uint8_t)0x0DU)
#define NRF24_RX_ADDR_P4      ((uint8_t)0x0EU)
#define NRF24_RX_ADDR_P5      ((uint8_t)0x0FU)
#define NRF24_TX_ADDR         ((uint8_t)0x10U)
#define NRF24_RX_PW_P0        ((uint8_t)0x11U)
#define NRF24_RX_PW_P1        ((uint8_t)0x12U)
#define NRF24_RX_PW_P2        ((uint8_t)0x13U)
#define NRF24_RX_PW_P3        ((uint8_t)0x14U)
#define NRF24_RX_PW_P4        ((uint8_t)0x15U)
#define NRF24_RX_PW_P5        ((uint8_t)0x16U)
#define NRF24_FIFO_STATUS     ((uint8_t)0x17U)
#define NRF24_DYNPD           ((uint8_t)0x1CU)
#define NRF24_FEATURE         ((uint8_t)0x1DU)

//nRF24L01+ name for register 0x09.
#define NRF24_RPD             ((uint8_t)0x09U)


//CONFIG register bit positions


#define NRF24_MASK_RX_DR      ((uint8_t)6U)
#define NRF24_MASK_TX_DS      ((uint8_t)5U)
#define NRF24_MASK_MAX_RT     ((uint8_t)4U)
#define NRF24_EN_CRC          ((uint8_t)3U)
#define NRF24_CRCO            ((uint8_t)2U)
#define NRF24_PWR_UP          ((uint8_t)1U)
#define NRF24_PRIM_RX         ((uint8_t)0U)


//EN_AA register bit positions


#define NRF24_ENAA_P5         ((uint8_t)5U)
#define NRF24_ENAA_P4         ((uint8_t)4U)
#define NRF24_ENAA_P3         ((uint8_t)3U)
#define NRF24_ENAA_P2         ((uint8_t)2U)
#define NRF24_ENAA_P1         ((uint8_t)1U)
#define NRF24_ENAA_P0         ((uint8_t)0U)


//EN_RXADDR register bit positions


#define NRF24_ERX_P5          ((uint8_t)5U)
#define NRF24_ERX_P4          ((uint8_t)4U)
#define NRF24_ERX_P3          ((uint8_t)3U)
#define NRF24_ERX_P2          ((uint8_t)2U)
#define NRF24_ERX_P1          ((uint8_t)1U)
#define NRF24_ERX_P0          ((uint8_t)0U)


//SETUP_AW and SETUP_RETR field positions

#define NRF24_AW              ((uint8_t)0U)
#define NRF24_ARD             ((uint8_t)4U)
#define NRF24_ARC             ((uint8_t)0U)


//RF_SETUP register bit positions


#define NRF24_CONT_WAVE       ((uint8_t)7U)
#define NRF24_RF_DR_LOW       ((uint8_t)5U)
#define NRF24_PLL_LOCK        ((uint8_t)4U)
#define NRF24_RF_DR_HIGH      ((uint8_t)3U)
#define NRF24_RF_PWR_HIGH     ((uint8_t)2U)
#define NRF24_RF_PWR_LOW      ((uint8_t)1U)
#define NRF24_LNA_HCURR       ((uint8_t)0U)


//Legacy aliases retained for readability when comparing against older nRF24L01 code or datasheet terminology.

#define NRF24_RF_DR           NRF24_RF_DR_HIGH
#define NRF24_RF_PWR          ((uint8_t)6U)


//STATUS register bit and field positions


#define NRF24_RX_DR           ((uint8_t)6U)
#define NRF24_TX_DS           ((uint8_t)5U)
#define NRF24_MAX_RT          ((uint8_t)4U)
#define NRF24_RX_P_NO         ((uint8_t)1U)
#define NRF24_TX_FULL         ((uint8_t)0U)


//OBSERVE_TX field positions


#define NRF24_PLOS_CNT        ((uint8_t)4U)
#define NRF24_ARC_CNT         ((uint8_t)0U)


//FIFO_STATUS register bit positions


#define NRF24_TX_REUSE        ((uint8_t)6U)
#define NRF24_FIFO_FULL       ((uint8_t)5U)
#define NRF24_TX_EMPTY        ((uint8_t)4U)
#define NRF24_RX_FULL         ((uint8_t)1U)
#define NRF24_RX_EMPTY        ((uint8_t)0U)


//DYNPD register bit positions


#define NRF24_DPL_P5          ((uint8_t)5U)
#define NRF24_DPL_P4          ((uint8_t)4U)
#define NRF24_DPL_P3          ((uint8_t)3U)
#define NRF24_DPL_P2          ((uint8_t)2U)
#define NRF24_DPL_P1          ((uint8_t)1U)
#define NRF24_DPL_P0          ((uint8_t)0U)


//FEATURE register bit positions

#define NRF24_EN_DPL          ((uint8_t)2U)
#define NRF24_EN_ACK_PAY      ((uint8_t)1U)
#define NRF24_EN_DYN_ACK      ((uint8_t)0U)


//SPI instruction mnemonics


#define NRF24_R_REGISTER              ((uint8_t)0x00U)
#define NRF24_W_REGISTER              ((uint8_t)0x20U)
#define NRF24_REGISTER_MASK           ((uint8_t)0x1FU)
#define NRF24_ACTIVATE                ((uint8_t)0x50U)
#define NRF24_R_RX_PL_WID             ((uint8_t)0x60U)
#define NRF24_R_RX_PAYLOAD            ((uint8_t)0x61U)
#define NRF24_W_TX_PAYLOAD            ((uint8_t)0xA0U)
#define NRF24_W_ACK_PAYLOAD           ((uint8_t)0xA8U)
#define NRF24_W_TX_PAYLOAD_NO_ACK     ((uint8_t)0xB0U)
#define NRF24_FLUSH_TX                ((uint8_t)0xE1U)
#define NRF24_FLUSH_RX                ((uint8_t)0xE2U)
#define NRF24_REUSE_TX_PL             ((uint8_t)0xE3U)
#define NRF24_NOP                     ((uint8_t)0xFFU)


//Helper macros


//Build an nRF24L01 register-read SPI command.
#define NRF24_READ_REGISTER_COMMAND(reg) \
    ((uint8_t)(NRF24_R_REGISTER | ((uint8_t)(reg) & NRF24_REGISTER_MASK)))

//Build an nRF24L01 register-write SPI command.
#define NRF24_WRITE_REGISTER_COMMAND(reg) \
    ((uint8_t)(NRF24_W_REGISTER | ((uint8_t)(reg) & NRF24_REGISTER_MASK)))

//Build a W_ACK_PAYLOAD command for receive pipe 0 through 5.
#define NRF24_WRITE_ACK_PAYLOAD_COMMAND(pipe) \
    ((uint8_t)(NRF24_W_ACK_PAYLOAD | ((uint8_t)(pipe) & 0x07U)))

#ifdef __cplusplus
}
#endif

#endif
