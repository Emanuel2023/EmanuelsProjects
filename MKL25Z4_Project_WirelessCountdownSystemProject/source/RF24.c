/*
 * RF24.c
 *
 * Native C driver for the nRF24L01+ / nRF24L01+PA+LNA module
 * on the FRDM-KL25Z using MCUXpresso SDK.
 *
 * Hardware mapping is defined in RF24_config.h:
 *   PTA12             -> CE
 *   PTA14             -> CSN (GPIO-controlled)
 *   PTA15 / SPI0_SCK  -> SCK
 *   PTA16 / SPI0_SIN  <- MISO
 *   PTA17 / SPI0_SOUT -> MOSI
 *   IRQ               -> not connected initially
 *
 * This file is a native-C rewrite of the core RF24 radio logic. It does not
 * use Arduino, C++ classes, namespaces, constructors, dynamic allocation,
 * SPIClass, digitalWrite(), or millis().
 *
 * Required companion files:
 *   RF24.h
 *   RF24_config.h
 *   nRF24L01.h
 *   RF24_platform_kl25z.c
 *
 * License:
 * This derivative remains subject to GPL-2.0 because it is based on RF24.
 */

//#include "RF24.h"
//#include "RF24_config.h"
//#include "nRF24L01.h"
//
//#include <string.h>
//
///* -------------------------------------------------------------------------- */
///* Local constants                                                            */
///* -------------------------------------------------------------------------- */
//
//#define RF24_MAX_CHANNEL              125U
//#define RF24_DEFAULT_CHANNEL          76U
//#define RF24_DEFAULT_RETRY_DELAY      5U
//#define RF24_DEFAULT_RETRY_COUNT      15U
//#define RF24_DEFAULT_TX_TIMEOUT_MS    95U
//
//#define RF24_STATUS_IRQ_MASK \
//    ((uint8_t)(RF24_BIT(NRF24_MASK_MAX_RT) | \
//               RF24_BIT(NRF24_TX_DS)       | \
//               RF24_BIT(NRF24_RX_DR)))
//
//static const uint8_t s_child_pipe_enable[6] = {
//    NRF24_ERX_P0,
//    NRF24_ERX_P1,
//    NRF24_ERX_P2,
//    NRF24_ERX_P3,
//    NRF24_ERX_P4,
//    NRF24_ERX_P5
//};
//
//static const uint8_t s_child_pipe_address[6] = {
//    NRF24_RX_ADDR_P0,
//    NRF24_RX_ADDR_P1,
//    NRF24_RX_ADDR_P2,
//    NRF24_RX_ADDR_P3,
//    NRF24_RX_ADDR_P4,
//    NRF24_RX_ADDR_P5
//};
//
//static const uint8_t s_child_payload_size[6] = {
//    NRF24_RX_PW_P0,
//    NRF24_RX_PW_P1,
//    NRF24_RX_PW_P2,
//    NRF24_RX_PW_P3,
//    NRF24_RX_PW_P4,
//    NRF24_RX_PW_P5
//};
//
///* -------------------------------------------------------------------------- */
///* Low-level helpers                                                          */
///* -------------------------------------------------------------------------- */
//
//static void RF24_BeginTransaction(void)
//{
//    RF24_CSN_Low();
//}
//
//static void RF24_EndTransaction(void)
//{
//    RF24_CSN_High();
//}
//
//static bool RF24_TransferByte(uint8_t tx, uint8_t *rx)
//{
//    uint8_t received = 0xFFU;
//    bool ok = RF24_SPI_TransferByte(tx, &received);
//
//    if (rx != NULL) {
//        *rx = received;
//    }
//
//    return ok;
//}
//
//static bool RF24_Command(RF24_t *radio, uint8_t command)
//{
//    uint8_t status = 0xFFU;
//
//    if (radio == NULL) {
//        return false;
//    }
//
//    RF24_BeginTransaction();
//    bool ok = RF24_TransferByte(command, &status);
//    RF24_EndTransaction();
//
//    radio->status = status;
//    return ok;
//}
//
//static bool RF24_ReadRegisterBuffer(
//    RF24_t *radio,
//    uint8_t reg,
//    uint8_t *buffer,
//    uint8_t length)
//{
//    uint8_t status = 0xFFU;
//
//    if ((radio == NULL) || ((buffer == NULL) && (length > 0U))) {
//        return false;
//    }
//
//    RF24_BeginTransaction();
//
//    if (!RF24_TransferByte((uint8_t)(NRF24_R_REGISTER | (reg & NRF24_REGISTER_MASK)),
//                           &status)) {
//        RF24_EndTransaction();
//        return false;
//    }
//
//    radio->status = status;
//
//    for (uint8_t i = 0U; i < length; ++i) {
//        if (!RF24_TransferByte(NRF24_NOP, &buffer[i])) {
//            RF24_EndTransaction();
//            return false;
//        }
//    }
//
//    RF24_EndTransaction();
//    return true;
//}
//
//static bool RF24_ReadRegisterByte(
//    RF24_t *radio,
//    uint8_t reg,
//    uint8_t *value)
//{
//    return RF24_ReadRegisterBuffer(radio, reg, value, 1U);
//}
//
//static bool RF24_WriteRegisterBuffer(
//    RF24_t *radio,
//    uint8_t reg,
//    const uint8_t *buffer,
//    uint8_t length)
//{
//    uint8_t status = 0xFFU;
//    uint8_t ignored = 0U;
//
//    if ((radio == NULL) || ((buffer == NULL) && (length > 0U))) {
//        return false;
//    }
//
//    RF24_BeginTransaction();
//
//    if (!RF24_TransferByte(
//            (uint8_t)(NRF24_W_REGISTER | (reg & NRF24_REGISTER_MASK)),
//            &status)) {
//        RF24_EndTransaction();
//        return false;
//    }
//
//    radio->status = status;
//
//    for (uint8_t i = 0U; i < length; ++i) {
//        if (!RF24_TransferByte(buffer[i], &ignored)) {
//            RF24_EndTransaction();
//            return false;
//        }
//    }
//
//    RF24_EndTransaction();
//    return true;
//}
//
//static bool RF24_WriteRegisterByte(
//    RF24_t *radio,
//    uint8_t reg,
//    uint8_t value)
//{
//    return RF24_WriteRegisterBuffer(radio, reg, &value, 1U);
//}
//
//static bool RF24_WritePayloadInternal(
//    RF24_t *radio,
//    const void *buffer,
//    uint8_t data_length,
//    uint8_t command)
//{
//    const uint8_t *current = (const uint8_t *)buffer;
//    uint8_t status = 0xFFU;
//    uint8_t ignored = 0U;
//    uint8_t blank_length = 0U;
//
//    if ((radio == NULL) || ((buffer == NULL) && (data_length > 0U))) {
//        return false;
//    }
//
//    if (radio->dynamic_payloads_enabled) {
//        data_length = RF24_MIN(data_length, RF24_MAX_PAYLOAD_SIZE);
//    } else {
//        data_length = RF24_MIN(data_length, radio->payload_size);
//        blank_length = (uint8_t)(radio->payload_size - data_length);
//    }
//
//    RF24_BeginTransaction();
//
//    if (!RF24_TransferByte(command, &status)) {
//        RF24_EndTransaction();
//        return false;
//    }
//
//    radio->status = status;
//
//    for (uint8_t i = 0U; i < data_length; ++i) {
//        if (!RF24_TransferByte(current[i], &ignored)) {
//            RF24_EndTransaction();
//            return false;
//        }
//    }
//
//    for (uint8_t i = 0U; i < blank_length; ++i) {
//        if (!RF24_TransferByte(0U, &ignored)) {
//            RF24_EndTransaction();
//            return false;
//        }
//    }
//
//    RF24_EndTransaction();
//    return true;
//}
//
//static bool RF24_ReadPayloadInternal(
//    RF24_t *radio,
//    void *buffer,
//    uint8_t data_length)
//{
//    uint8_t *current = (uint8_t *)buffer;
//    uint8_t status = 0xFFU;
//    uint8_t ignored = 0U;
//    uint8_t blank_length = 0U;
//
//    if ((radio == NULL) || ((buffer == NULL) && (data_length > 0U))) {
//        return false;
//    }
//
//    if (radio->dynamic_payloads_enabled) {
//        data_length = RF24_MIN(data_length, RF24_MAX_PAYLOAD_SIZE);
//    } else {
//        data_length = RF24_MIN(data_length, radio->payload_size);
//        blank_length = (uint8_t)(radio->payload_size - data_length);
//    }
//
//    RF24_BeginTransaction();
//
//    if (!RF24_TransferByte(NRF24_R_RX_PAYLOAD, &status)) {
//        RF24_EndTransaction();
//        return false;
//    }
//
//    radio->status = status;
//
//    for (uint8_t i = 0U; i < data_length; ++i) {
//        if (!RF24_TransferByte(NRF24_NOP, &current[i])) {
//            RF24_EndTransaction();
//            return false;
//        }
//    }
//
//    for (uint8_t i = 0U; i < blank_length; ++i) {
//        if (!RF24_TransferByte(NRF24_NOP, &ignored)) {
//            RF24_EndTransaction();
//            return false;
//        }
//    }
//
//    RF24_EndTransaction();
//    return true;
//}
//
//static bool RF24_ToggleFeatures(RF24_t *radio)
//{
//    uint8_t status = 0xFFU;
//    uint8_t ignored = 0U;
//
//    if (radio == NULL) {
//        return false;
//    }
//
//    RF24_BeginTransaction();
//
//    if (!RF24_TransferByte(NRF24_ACTIVATE, &status)) {
//        RF24_EndTransaction();
//        return false;
//    }
//
//    radio->status = status;
//
//    if (!RF24_TransferByte(0x73U, &ignored)) {
//        RF24_EndTransaction();
//        return false;
//    }
//
//    RF24_EndTransaction();
//    return true;
//}
//
//static uint8_t RF24_GetDataRateRegisterValue(RF24_DataRate_t speed)
//{
//    switch (speed) {
//        case RF24_DATA_RATE_250KBPS:
//            return RF24_BIT(NRF24_RF_DR_LOW);
//
//        case RF24_DATA_RATE_2MBPS:
//            return RF24_BIT(NRF24_RF_DR_HIGH);
//
//        case RF24_DATA_RATE_1MBPS:
//        default:
//            return 0U;
//    }
//}
//
//static uint8_t RF24_GetPALevelRegisterValue(
//    RF24_PALevel_t level,
//    bool lna_enable)
//{
//    uint8_t value;
//
//    switch (level) {
//        case RF24_PA_MIN:
//            value = 0U;
//            break;
//
//        case RF24_PA_LOW:
//            value = RF24_BIT(NRF24_RF_PWR_LOW);
//            break;
//
//        case RF24_PA_HIGH:
//            value = RF24_BIT(NRF24_RF_PWR_HIGH);
//            break;
//
//        case RF24_PA_MAX:
//        default:
//            value = (uint8_t)(RF24_BIT(NRF24_RF_PWR_LOW) |
//                              RF24_BIT(NRF24_RF_PWR_HIGH));
//            break;
//    }
//
//    if (lna_enable) {
//        value |= RF24_BIT(NRF24_LNA_HCURR);
//    }
//
//    return value;
//}
//
//static bool RF24_WaitForTransmitResult(
//    RF24_t *radio,
//    uint32_t timeout_ms)
//{
//    uint32_t start;
//
//    if (radio == NULL) {
//        return false;
//    }
//
//    start = RF24_Millis();
//
//    while (true) {
//        uint8_t status = RF24_UpdateStatus(radio);
//
//        if ((status & RF24_STATUS_IRQ_MASK) != 0U) {
//            return true;
//        }
//
//        if ((RF24_Millis() - start) > timeout_ms) {
//#if RF24_FAILURE_HANDLING
//            radio->failure_detected = true;
//            radio->failure_recovery_attempts++;
//#endif
//            RF24_CE_Low();
//            (void)RF24_FlushRx(radio);
//            (void)RF24_FlushTx(radio);
//            return false;
//        }
//    }
//}
//
///* -------------------------------------------------------------------------- */
///* Public initialization                                                      */
///* -------------------------------------------------------------------------- */
//
//void RF24_ResetObject(RF24_t *radio)
//{
//    if (radio == NULL) {
//        return;
//    }
//
//    memset(radio, 0, sizeof(*radio));
//
//    radio->spi_speed_hz = RF24_SPI_BAUDRATE_HZ;
//    radio->payload_size = RF24_DEFAULT_PAYLOAD_SIZE;
//    radio->address_width = RF24_DEFAULT_ADDRESS_WIDTH;
//    radio->dynamic_payloads_enabled = false;
//    radio->ack_payloads_enabled = false;
//    radio->is_plus_variant = false;
//    radio->pipe0_is_receiving = false;
//    radio->tx_delay_us = 280U;
//    radio->cs_delay_us = 5U;
//}
//
//bool RF24_Init(RF24_t *radio)
//{
//    uint8_t feature_before = 0U;
//    uint8_t feature_after = 0U;
//    uint8_t expected_config;
//    uint8_t actual_config = 0U;
//
//    if (radio == NULL) {
//        return false;
//    }
//
//    RF24_ResetObject(radio);
//
//    if (!RF24_PlatformInit(radio->spi_speed_hz)) {
//        return false;
//    }
//
//    RF24_CE_Low();
//    RF24_CSN_High();
//
//    /* Allow radio power-on reset and register settling. */
//    RF24_DelayMs(5U);
//
//    if (!RF24_SetRetries(radio,
//                         RF24_DEFAULT_RETRY_DELAY,
//                         RF24_DEFAULT_RETRY_COUNT)) {
//        return false;
//    }
//
//    if (!RF24_SetDataRate(radio, RF24_DATA_RATE_1MBPS)) {
//        return false;
//    }
//
//    if (!RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature_before)) {
//        return false;
//    }
//
//    if (!RF24_ToggleFeatures(radio)) {
//        return false;
//    }
//
//    if (!RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature_after)) {
//        return false;
//    }
//
//    radio->is_plus_variant = (feature_before == feature_after);
//
//    if (feature_after != 0U) {
//        if (radio->is_plus_variant) {
//            if (!RF24_ToggleFeatures(radio)) {
//                return false;
//            }
//        }
//
//        if (!RF24_WriteRegisterByte(radio, NRF24_FEATURE, 0U)) {
//            return false;
//        }
//    }
//
//    radio->ack_payloads_enabled = false;
//    radio->dynamic_payloads_enabled = false;
//
//    if (!RF24_WriteRegisterByte(radio, NRF24_DYNPD, 0U) ||
//        !RF24_WriteRegisterByte(radio, NRF24_EN_AA, 0x3FU) ||
//        !RF24_WriteRegisterByte(radio, NRF24_EN_RXADDR, 0x03U) ||
//        !RF24_SetPayloadSize(radio, RF24_DEFAULT_PAYLOAD_SIZE) ||
//        !RF24_SetAddressWidth(radio, RF24_DEFAULT_ADDRESS_WIDTH) ||
//        !RF24_SetChannel(radio, RF24_DEFAULT_CHANNEL)) {
//        return false;
//    }
//
//    (void)RF24_ClearStatusFlags(radio, RF24_IRQ_ALL);
//    (void)RF24_FlushRx(radio);
//    (void)RF24_FlushTx(radio);
//
//    /*
//     * Disable physical IRQ output, use PTX mode, enable 16-bit CRC.
//     * PWR_UP is set by RF24_PowerUp().
//     */
//    expected_config =
//        (uint8_t)(RF24_BIT(NRF24_EN_CRC) |
//                  RF24_BIT(NRF24_CRCO) |
//                  RF24_BIT(NRF24_MASK_RX_DR) |
//                  RF24_BIT(NRF24_MASK_TX_DS) |
//                  RF24_BIT(NRF24_MASK_MAX_RT));
//
//    if (!RF24_WriteRegisterByte(radio, NRF24_CONFIG, expected_config)) {
//        return false;
//    }
//
//    radio->config_register = expected_config;
//
//    if (!RF24_PowerUp(radio)) {
//        return false;
//    }
//
//    if (!RF24_ReadRegisterByte(radio, NRF24_CONFIG, &actual_config)) {
//        return false;
//    }
//
//    expected_config |= RF24_BIT(NRF24_PWR_UP);
//
//    return actual_config == expected_config;
//}
//
//bool RF24_IsChipConnected(RF24_t *radio)
//{
//    uint8_t setup_aw = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(radio, NRF24_SETUP_AW, &setup_aw)) {
//        return false;
//    }
//
//    return setup_aw == (uint8_t)(radio->address_width - 2U);
//}
//
///* -------------------------------------------------------------------------- */
///* Basic register/status operations                                           */
///* -------------------------------------------------------------------------- */
//
//uint8_t RF24_UpdateStatus(RF24_t *radio)
//{
//    if (radio == NULL) {
//        return 0xFFU;
//    }
//
//    (void)RF24_Command(radio, NRF24_NOP);
//    return radio->status;
//}
//
//uint8_t RF24_GetStatus(const RF24_t *radio)
//{
//    return (radio != NULL) ? radio->status : 0xFFU;
//}
//
//uint8_t RF24_ClearStatusFlags(RF24_t *radio, uint8_t flags)
//{
//    if (radio == NULL) {
//        return 0xFFU;
//    }
//
//    (void)RF24_WriteRegisterByte(
//        radio,
//        NRF24_STATUS,
//        (uint8_t)(flags & RF24_IRQ_ALL));
//
//    return radio->status;
//}
//
//bool RF24_SetStatusFlags(RF24_t *radio, uint8_t flags)
//{
//    uint8_t config;
//
//    if (radio == NULL) {
//        return false;
//    }
//
//    config = (uint8_t)(
//        (radio->config_register & (uint8_t)~RF24_IRQ_ALL) |
//        ((uint8_t)~flags & RF24_IRQ_ALL));
//
//    if (!RF24_WriteRegisterByte(radio, NRF24_CONFIG, config)) {
//        return false;
//    }
//
//    radio->config_register = config;
//    return true;
//}
//
//bool RF24_FlushRx(RF24_t *radio)
//{
//    return RF24_Command(radio, NRF24_FLUSH_RX);
//}
//
//bool RF24_FlushTx(RF24_t *radio)
//{
//    return RF24_Command(radio, NRF24_FLUSH_TX);
//}
//
///* -------------------------------------------------------------------------- */
///* Configuration                                                              */
///* -------------------------------------------------------------------------- */
//
//bool RF24_SetChannel(RF24_t *radio, uint8_t channel)
//{
//    return RF24_WriteRegisterByte(
//        radio,
//        NRF24_RF_CH,
//        RF24_MIN(channel, RF24_MAX_CHANNEL));
//}
//
//uint8_t RF24_GetChannel(RF24_t *radio)
//{
//    uint8_t channel = 0U;
//    (void)RF24_ReadRegisterByte(radio, NRF24_RF_CH, &channel);
//    return channel;
//}
//
//bool RF24_SetPayloadSize(RF24_t *radio, uint8_t size)
//{
//    if (radio == NULL) {
//        return false;
//    }
//
//    size = RF24_MAX(1U, RF24_MIN(size, RF24_MAX_PAYLOAD_SIZE));
//    radio->payload_size = size;
//
//    for (uint8_t pipe = 0U; pipe < 6U; ++pipe) {
//        if (!RF24_WriteRegisterByte(
//                radio,
//                s_child_payload_size[pipe],
//                size)) {
//            return false;
//        }
//    }
//
//    return true;
//}
//
//uint8_t RF24_GetPayloadSize(const RF24_t *radio)
//{
//    return (radio != NULL) ? radio->payload_size : 0U;
//}
//
//bool RF24_SetAddressWidth(RF24_t *radio, uint8_t width)
//{
//    if (radio == NULL) {
//        return false;
//    }
//
//    width = RF24_MAX(3U, RF24_MIN(width, 5U));
//
//    if (!RF24_WriteRegisterByte(
//            radio,
//            NRF24_SETUP_AW,
//            (uint8_t)(width - 2U))) {
//        return false;
//    }
//
//    radio->address_width = width;
//    return true;
//}
//
//bool RF24_SetRetries(RF24_t *radio, uint8_t delay, uint8_t count)
//{
//    uint8_t value =
//        (uint8_t)(((delay & 0x0FU) << NRF24_ARD) |
//                  ((count & 0x0FU) << NRF24_ARC));
//
//    return RF24_WriteRegisterByte(radio, NRF24_SETUP_RETR, value);
//}
//
//bool RF24_SetDataRate(RF24_t *radio, RF24_DataRate_t speed)
//{
//    uint8_t setup = 0U;
//    uint8_t verify = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(radio, NRF24_RF_SETUP, &setup)) {
//        return false;
//    }
//
//    setup &= (uint8_t)~(
//        RF24_BIT(NRF24_RF_DR_LOW) |
//        RF24_BIT(NRF24_RF_DR_HIGH));
//
//    setup |= RF24_GetDataRateRegisterValue(speed);
//
//    if (!RF24_WriteRegisterByte(radio, NRF24_RF_SETUP, setup) ||
//        !RF24_ReadRegisterByte(radio, NRF24_RF_SETUP, &verify)) {
//        return false;
//    }
//
//    switch (speed) {
//        case RF24_DATA_RATE_250KBPS:
//            radio->tx_delay_us = 505U;
//            break;
//
//        case RF24_DATA_RATE_2MBPS:
//            radio->tx_delay_us = 240U;
//            break;
//
//        case RF24_DATA_RATE_1MBPS:
//        default:
//            radio->tx_delay_us = 280U;
//            break;
//    }
//
//    return verify == setup;
//}
//
//RF24_DataRate_t RF24_GetDataRate(RF24_t *radio)
//{
//    uint8_t setup = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(radio, NRF24_RF_SETUP, &setup)) {
//        return RF24_DATA_RATE_1MBPS;
//    }
//
//    if ((setup & RF24_BIT(NRF24_RF_DR_LOW)) != 0U) {
//        return RF24_DATA_RATE_250KBPS;
//    }
//
//    if ((setup & RF24_BIT(NRF24_RF_DR_HIGH)) != 0U) {
//        return RF24_DATA_RATE_2MBPS;
//    }
//
//    return RF24_DATA_RATE_1MBPS;
//}
//
//bool RF24_SetPALevel(
//    RF24_t *radio,
//    RF24_PALevel_t level,
//    bool lna_enable)
//{
//    uint8_t setup = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(radio, NRF24_RF_SETUP, &setup)) {
//        return false;
//    }
//
//    setup &= (uint8_t)~(
//        RF24_BIT(NRF24_RF_PWR_LOW) |
//        RF24_BIT(NRF24_RF_PWR_HIGH) |
//        RF24_BIT(NRF24_LNA_HCURR));
//
//    setup |= RF24_GetPALevelRegisterValue(level, lna_enable);
//
//    return RF24_WriteRegisterByte(radio, NRF24_RF_SETUP, setup);
//}
//
//RF24_PALevel_t RF24_GetPALevel(RF24_t *radio)
//{
//    uint8_t setup = 0U;
//    uint8_t level_bits;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(radio, NRF24_RF_SETUP, &setup)) {
//        return RF24_PA_MIN;
//    }
//
//    level_bits = (uint8_t)(
//        setup &
//        (RF24_BIT(NRF24_RF_PWR_LOW) |
//         RF24_BIT(NRF24_RF_PWR_HIGH)));
//
//    if (level_bits ==
//        (RF24_BIT(NRF24_RF_PWR_LOW) |
//         RF24_BIT(NRF24_RF_PWR_HIGH))) {
//        return RF24_PA_MAX;
//    }
//
//    if (level_bits == RF24_BIT(NRF24_RF_PWR_HIGH)) {
//        return RF24_PA_HIGH;
//    }
//
//    if (level_bits == RF24_BIT(NRF24_RF_PWR_LOW)) {
//        return RF24_PA_LOW;
//    }
//
//    return RF24_PA_MIN;
//}
//
//bool RF24_SetAutoAck(RF24_t *radio, bool enable)
//{
//    return RF24_WriteRegisterByte(
//        radio,
//        NRF24_EN_AA,
//        enable ? 0x3FU : 0U);
//}
//
//bool RF24_SetAutoAckPipe(
//    RF24_t *radio,
//    uint8_t pipe,
//    bool enable)
//{
//    uint8_t en_aa = 0U;
//
//    if ((radio == NULL) || (pipe > 5U) ||
//        !RF24_ReadRegisterByte(radio, NRF24_EN_AA, &en_aa)) {
//        return false;
//    }
//
//    if (enable) {
//        en_aa |= RF24_BIT(pipe);
//    } else {
//        en_aa &= (uint8_t)~RF24_BIT(pipe);
//    }
//
//    return RF24_WriteRegisterByte(radio, NRF24_EN_AA, en_aa);
//}
//
//bool RF24_EnableDynamicPayloads(RF24_t *radio)
//{
//    uint8_t feature = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature)) {
//        return false;
//    }
//
//    feature |= RF24_BIT(NRF24_EN_DPL);
//
//    if (!RF24_WriteRegisterByte(radio, NRF24_FEATURE, feature) ||
//        !RF24_WriteRegisterByte(radio, NRF24_DYNPD, 0x3FU)) {
//        return false;
//    }
//
//    radio->dynamic_payloads_enabled = true;
//    return true;
//}
//
//bool RF24_DisableDynamicPayloads(RF24_t *radio)
//{
//    uint8_t feature = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature)) {
//        return false;
//    }
//
//    feature &= (uint8_t)~RF24_BIT(NRF24_EN_DPL);
//
//    if (!RF24_WriteRegisterByte(radio, NRF24_FEATURE, feature) ||
//        !RF24_WriteRegisterByte(radio, NRF24_DYNPD, 0U)) {
//        return false;
//    }
//
//    radio->dynamic_payloads_enabled = false;
//    return true;
//}
//
//bool RF24_EnableAckPayload(RF24_t *radio)
//{
//    uint8_t feature = 0U;
//    uint8_t dynpd = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature) ||
//        !RF24_ReadRegisterByte(radio, NRF24_DYNPD, &dynpd)) {
//        return false;
//    }
//
//    feature |= (uint8_t)(
//        RF24_BIT(NRF24_EN_ACK_PAY) |
//        RF24_BIT(NRF24_EN_DPL));
//
//    dynpd |= (uint8_t)(
//        RF24_BIT(NRF24_DPL_P0) |
//        RF24_BIT(NRF24_DPL_P1));
//
//    if (!RF24_WriteRegisterByte(radio, NRF24_FEATURE, feature) ||
//        !RF24_WriteRegisterByte(radio, NRF24_DYNPD, dynpd)) {
//        return false;
//    }
//
//    radio->ack_payloads_enabled = true;
//    radio->dynamic_payloads_enabled = true;
//    return true;
//}
//
//bool RF24_DisableAckPayload(RF24_t *radio)
//{
//    uint8_t feature = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature)) {
//        return false;
//    }
//
//    feature &= (uint8_t)~RF24_BIT(NRF24_EN_ACK_PAY);
//
//    if (!RF24_WriteRegisterByte(radio, NRF24_FEATURE, feature)) {
//        return false;
//    }
//
//    radio->ack_payloads_enabled = false;
//    return true;
//}
//
//bool RF24_EnableDynamicAck(RF24_t *radio)
//{
//    uint8_t feature = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature)) {
//        return false;
//    }
//
//    feature |= RF24_BIT(NRF24_EN_DYN_ACK);
//
//    return RF24_WriteRegisterByte(radio, NRF24_FEATURE, feature);
//}
//
///* -------------------------------------------------------------------------- */
///* Addressing                                                                 */
///* -------------------------------------------------------------------------- */
//
//bool RF24_OpenWritingPipe(
//    RF24_t *radio,
//    const uint8_t *address)
//{
//    if ((radio == NULL) || (address == NULL)) {
//        return false;
//    }
//
//    memcpy(
//        radio->pipe0_writing_address,
//        address,
//        radio->address_width);
//
//    return RF24_WriteRegisterBuffer(
//               radio,
//               NRF24_RX_ADDR_P0,
//               address,
//               radio->address_width) &&
//           RF24_WriteRegisterBuffer(
//               radio,
//               NRF24_TX_ADDR,
//               address,
//               radio->address_width);
//}
//
//bool RF24_OpenReadingPipe(
//    RF24_t *radio,
//    uint8_t pipe,
//    const uint8_t *address)
//{
//    uint8_t en_rxaddr = 0U;
//
//    if ((radio == NULL) ||
//        (address == NULL) ||
//        (pipe > 5U)) {
//        return false;
//    }
//
//    if (pipe == 0U) {
//        memcpy(
//            radio->pipe0_reading_address,
//            address,
//            radio->address_width);
//        radio->pipe0_is_receiving = true;
//    }
//
//    if (pipe < 2U) {
//        if (!RF24_WriteRegisterBuffer(
//                radio,
//                s_child_pipe_address[pipe],
//                address,
//                radio->address_width)) {
//            return false;
//        }
//    } else {
//        if (!RF24_WriteRegisterByte(
//                radio,
//                s_child_pipe_address[pipe],
//                address[0])) {
//            return false;
//        }
//    }
//
//    if (!RF24_WriteRegisterByte(
//            radio,
//            s_child_payload_size[pipe],
//            radio->payload_size) ||
//        !RF24_ReadRegisterByte(
//            radio,
//            NRF24_EN_RXADDR,
//            &en_rxaddr)) {
//        return false;
//    }
//
//    en_rxaddr |= RF24_BIT(s_child_pipe_enable[pipe]);
//
//    return RF24_WriteRegisterByte(
//        radio,
//        NRF24_EN_RXADDR,
//        en_rxaddr);
//}
//
//bool RF24_CloseReadingPipe(RF24_t *radio, uint8_t pipe)
//{
//    uint8_t en_rxaddr = 0U;
//
//    if ((radio == NULL) ||
//        (pipe > 5U) ||
//        !RF24_ReadRegisterByte(
//            radio,
//            NRF24_EN_RXADDR,
//            &en_rxaddr)) {
//        return false;
//    }
//
//    en_rxaddr &= (uint8_t)~RF24_BIT(s_child_pipe_enable[pipe]);
//
//    if (pipe == 0U) {
//        radio->pipe0_is_receiving = false;
//    }
//
//    return RF24_WriteRegisterByte(
//        radio,
//        NRF24_EN_RXADDR,
//        en_rxaddr);
//}
//
///* -------------------------------------------------------------------------- */
///* Power and operating modes                                                  */
///* -------------------------------------------------------------------------- */
//
//bool RF24_PowerUp(RF24_t *radio)
//{
//    if (radio == NULL) {
//        return false;
//    }
//
//    if ((radio->config_register & RF24_BIT(NRF24_PWR_UP)) == 0U) {
//        radio->config_register |= RF24_BIT(NRF24_PWR_UP);
//
//        if (!RF24_WriteRegisterByte(
//                radio,
//                NRF24_CONFIG,
//                radio->config_register)) {
//            return false;
//        }
//
//        RF24_DelayUs(RF24_POWERUP_DELAY_US);
//    }
//
//    return true;
//}
//
//bool RF24_PowerDown(RF24_t *radio)
//{
//    if (radio == NULL) {
//        return false;
//    }
//
//    RF24_CE_Low();
//
//    radio->config_register &=
//        (uint8_t)~RF24_BIT(NRF24_PWR_UP);
//
//    return RF24_WriteRegisterByte(
//        radio,
//        NRF24_CONFIG,
//        radio->config_register);
//}
//
//bool RF24_StartListening(RF24_t *radio)
//{
//    if ((radio == NULL) ||
//        !RF24_PowerUp(radio)) {
//        return false;
//    }
//
//    radio->config_register |= RF24_BIT(NRF24_PRIM_RX);
//
//    if (!RF24_WriteRegisterByte(
//            radio,
//            NRF24_CONFIG,
//            radio->config_register) ||
//        !RF24_WriteRegisterByte(
//            radio,
//            NRF24_STATUS,
//            RF24_IRQ_ALL)) {
//        return false;
//    }
//
//    if (radio->pipe0_is_receiving) {
//        if (!RF24_WriteRegisterBuffer(
//                radio,
//                NRF24_RX_ADDR_P0,
//                radio->pipe0_reading_address,
//                radio->address_width)) {
//            return false;
//        }
//    } else {
//        if (!RF24_CloseReadingPipe(radio, 0U)) {
//            return false;
//        }
//    }
//
//    RF24_CE_High();
//    return true;
//}
//
//bool RF24_StopListening(RF24_t *radio)
//{
//    uint8_t en_rxaddr = 0U;
//
//    if (radio == NULL) {
//        return false;
//    }
//
//    RF24_CE_Low();
//    RF24_DelayUs(radio->tx_delay_us);
//
//    if (radio->ack_payloads_enabled) {
//        (void)RF24_FlushTx(radio);
//    }
//
//    radio->config_register &=
//        (uint8_t)~RF24_BIT(NRF24_PRIM_RX);
//
//    if (!RF24_WriteRegisterByte(
//            radio,
//            NRF24_CONFIG,
//            radio->config_register) ||
//        !RF24_WriteRegisterBuffer(
//            radio,
//            NRF24_RX_ADDR_P0,
//            radio->pipe0_writing_address,
//            radio->address_width) ||
//        !RF24_ReadRegisterByte(
//            radio,
//            NRF24_EN_RXADDR,
//            &en_rxaddr)) {
//        return false;
//    }
//
//    en_rxaddr |= RF24_BIT(NRF24_ERX_P0);
//
//    return RF24_WriteRegisterByte(
//        radio,
//        NRF24_EN_RXADDR,
//        en_rxaddr);
//}
//
///* -------------------------------------------------------------------------- */
///* TX/RX                                                                      */
///* -------------------------------------------------------------------------- */
//
//bool RF24_Available(RF24_t *radio)
//{
//    uint8_t status;
//
//    if (radio == NULL) {
//        return false;
//    }
//
//    status = RF24_UpdateStatus(radio);
//    return ((status >> NRF24_RX_P_NO) & 0x07U) < 6U;
//}
//
//bool RF24_AvailablePipe(
//    RF24_t *radio,
//    uint8_t *pipe_number)
//{
//    uint8_t status;
//    uint8_t pipe;
//
//    if (radio == NULL) {
//        return false;
//    }
//
//    status = RF24_UpdateStatus(radio);
//    pipe = (uint8_t)((status >> NRF24_RX_P_NO) & 0x07U);
//
//    if (pipe_number != NULL) {
//        *pipe_number = pipe;
//    }
//
//    return pipe < 6U;
//}
//
//bool RF24_Read(
//    RF24_t *radio,
//    void *buffer,
//    uint8_t length)
//{
//    if ((radio == NULL) ||
//        !RF24_ReadPayloadInternal(radio, buffer, length)) {
//        return false;
//    }
//
//    return RF24_WriteRegisterByte(
//        radio,
//        NRF24_STATUS,
//        RF24_BIT(NRF24_RX_DR));
//}
//
//uint8_t RF24_GetDynamicPayloadSize(RF24_t *radio)
//{
//    uint8_t width = 0U;
//    uint8_t status = 0xFFU;
//
//    if (radio == NULL) {
//        return 0U;
//    }
//
//    RF24_BeginTransaction();
//
//    if (!RF24_TransferByte(NRF24_R_RX_PL_WID, &status) ||
//        !RF24_TransferByte(NRF24_NOP, &width)) {
//        RF24_EndTransaction();
//        return 0U;
//    }
//
//    RF24_EndTransaction();
//    radio->status = status;
//
//    if (width > RF24_MAX_PAYLOAD_SIZE) {
//        (void)RF24_FlushRx(radio);
//        RF24_DelayMs(2U);
//        return 0U;
//    }
//
//    return width;
//}
//
//bool RF24_StartFastWrite(
//    RF24_t *radio,
//    const void *buffer,
//    uint8_t length,
//    bool no_ack,
//    bool start_tx)
//{
//    uint8_t command;
//
//    if (radio == NULL) {
//        return false;
//    }
//
//    command = no_ack
//        ? NRF24_W_TX_PAYLOAD_NO_ACK
//        : NRF24_W_TX_PAYLOAD;
//
//    if (!RF24_WritePayloadInternal(
//            radio,
//            buffer,
//            length,
//            command)) {
//        return false;
//    }
//
//    if (start_tx) {
//        RF24_CE_High();
//    }
//
//    return true;
//}
//
//bool RF24_Write(
//    RF24_t *radio,
//    const void *buffer,
//    uint8_t length)
//{
//    return RF24_WriteNoAck(radio, buffer, length, false);
//}
//
//bool RF24_WriteNoAck(
//    RF24_t *radio,
//    const void *buffer,
//    uint8_t length,
//    bool no_ack)
//{
//    bool wait_ok;
//
//    if (radio == NULL) {
//        return false;
//    }
//
//    if (!RF24_StartFastWrite(
//            radio,
//            buffer,
//            length,
//            no_ack,
//            true)) {
//        return false;
//    }
//
//    wait_ok = RF24_WaitForTransmitResult(
//        radio,
//        RF24_DEFAULT_TX_TIMEOUT_MS);
//
//    RF24_CE_Low();
//
//    if (!wait_ok) {
//        return false;
//    }
//
//    (void)RF24_ClearStatusFlags(radio, RF24_IRQ_ALL);
//
//    if ((radio->status & RF24_IRQ_TX_FAILED) != 0U) {
//        (void)RF24_FlushTx(radio);
//        return false;
//    }
//
//    return true;
//}
//
//bool RF24_WriteAckPayload(
//    RF24_t *radio,
//    uint8_t pipe,
//    const void *buffer,
//    uint8_t length)
//{
//    if ((radio == NULL) ||
//        (pipe > 5U) ||
//        !radio->ack_payloads_enabled) {
//        return false;
//    }
//
//    return RF24_WritePayloadInternal(
//        radio,
//        buffer,
//        length,
//        (uint8_t)(NRF24_W_ACK_PAYLOAD | (pipe & 0x07U)));
//}
//
//bool RF24_RxFifoFull(RF24_t *radio)
//{
//    uint8_t fifo = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(
//            radio,
//            NRF24_FIFO_STATUS,
//            &fifo)) {
//        return false;
//    }
//
//    return (fifo & RF24_BIT(NRF24_RX_FULL)) != 0U;
//}
//
//RF24_FifoState_t RF24_GetFifoState(
//    RF24_t *radio,
//    bool tx_fifo)
//{
//    uint8_t fifo = 0U;
//    bool full;
//    bool empty;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(
//            radio,
//            NRF24_FIFO_STATUS,
//            &fifo)) {
//        return RF24_FIFO_INVALID;
//    }
//
//    if (tx_fifo) {
//        full = (fifo & RF24_BIT(NRF24_FIFO_FULL)) != 0U;
//        empty = (fifo & RF24_BIT(NRF24_TX_EMPTY)) != 0U;
//    } else {
//        full = (fifo & RF24_BIT(NRF24_RX_FULL)) != 0U;
//        empty = (fifo & RF24_BIT(NRF24_RX_EMPTY)) != 0U;
//    }
//
//    if (full && empty) {
//        return RF24_FIFO_INVALID;
//    }
//
//    if (full) {
//        return RF24_FIFO_FULL;
//    }
//
//    if (empty) {
//        return RF24_FIFO_EMPTY;
//    }
//
//    return RF24_FIFO_OCCUPIED;
//}
//
///* -------------------------------------------------------------------------- */
///* Carrier detection and diagnostics                                          */
///* -------------------------------------------------------------------------- */
//
//bool RF24_TestCarrier(RF24_t *radio)
//{
//    uint8_t rpd = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(radio, NRF24_RPD, &rpd)) {
//        return false;
//    }
//
//    return (rpd & 0x01U) != 0U;
//}
//
//bool RF24_IsPlusVariant(const RF24_t *radio)
//{
//    return (radio != NULL) && radio->is_plus_variant;
//}
//
//uint8_t RF24_GetARC(RF24_t *radio)
//{
//    uint8_t observe = 0U;
//
//    if ((radio == NULL) ||
//        !RF24_ReadRegisterByte(
//            radio,
//            NRF24_OBSERVE_TX,
//            &observe)) {
//        return 0U;
//    }
//
//    return (uint8_t)(observe & 0x0FU);
//}



















#include "RF24.h"
#include "RF24_config.h"
#include "nRF24L01.h"
#include "fsl_debug_console.h"

#include <string.h>

/* -------------------------------------------------------------------------- */
/* Local constants                                                            */
/* -------------------------------------------------------------------------- */

#define RF24_MAX_CHANNEL              125U
#define RF24_DEFAULT_CHANNEL          76U
#define RF24_DEFAULT_RETRY_DELAY      5U
#define RF24_DEFAULT_RETRY_COUNT      15U
#define RF24_DEFAULT_TX_TIMEOUT_MS    95U

#define RF24_STATUS_IRQ_MASK \
    ((uint8_t)(RF24_BIT(NRF24_MASK_MAX_RT) | \
               RF24_BIT(NRF24_TX_DS)       | \
               RF24_BIT(NRF24_RX_DR)))

static const uint8_t s_child_pipe_enable[6] = {
    NRF24_ERX_P0,
    NRF24_ERX_P1,
    NRF24_ERX_P2,
    NRF24_ERX_P3,
    NRF24_ERX_P4,
    NRF24_ERX_P5
};

static const uint8_t s_child_pipe_address[6] = {
    NRF24_RX_ADDR_P0,
    NRF24_RX_ADDR_P1,
    NRF24_RX_ADDR_P2,
    NRF24_RX_ADDR_P3,
    NRF24_RX_ADDR_P4,
    NRF24_RX_ADDR_P5
};

static const uint8_t s_child_payload_size[6] = {
    NRF24_RX_PW_P0,
    NRF24_RX_PW_P1,
    NRF24_RX_PW_P2,
    NRF24_RX_PW_P3,
    NRF24_RX_PW_P4,
    NRF24_RX_PW_P5
};

/* -------------------------------------------------------------------------- */
/* Low-level helpers                                                          */
/* -------------------------------------------------------------------------- */

static void RF24_BeginTransaction(void)
{
    RF24_CSN_Low();
}

static void RF24_EndTransaction(void)
{
    RF24_CSN_High();
}

static bool RF24_TransferByte(uint8_t tx, uint8_t *rx)
{
    uint8_t received = 0xFFU;
    bool ok = RF24_SPI_TransferByte(tx, &received);

    if (rx != NULL) {
        *rx = received;
    }

    return ok;
}

static bool RF24_Command(RF24_t *radio, uint8_t command)
{
    uint8_t status = 0xFFU;

    if (radio == NULL) {
        return false;
    }

    RF24_BeginTransaction();
    bool ok = RF24_TransferByte(command, &status);
    RF24_EndTransaction();

    radio->status = status;
    return ok;
}

static bool RF24_ReadRegisterBuffer(
    RF24_t *radio,
    uint8_t reg,
    uint8_t *buffer,
    uint8_t length)
{
    uint8_t status = 0xFFU;

    if ((radio == NULL) || ((buffer == NULL) && (length > 0U))) {
        return false;
    }

    RF24_BeginTransaction();

    if (!RF24_TransferByte((uint8_t)(NRF24_R_REGISTER | (reg & NRF24_REGISTER_MASK)),
                           &status)) {
        RF24_EndTransaction();
        return false;
    }

    radio->status = status;

    for (uint8_t i = 0U; i < length; ++i) {
        if (!RF24_TransferByte(NRF24_NOP, &buffer[i])) {
            RF24_EndTransaction();
            return false;
        }
    }

    RF24_EndTransaction();
    return true;
}

static bool RF24_ReadRegisterByte(
    RF24_t *radio,
    uint8_t reg,
    uint8_t *value)
{
    return RF24_ReadRegisterBuffer(radio, reg, value, 1U);
}

static bool RF24_WriteRegisterBuffer(
    RF24_t *radio,
    uint8_t reg,
    const uint8_t *buffer,
    uint8_t length)
{
    uint8_t status = 0xFFU;
    uint8_t ignored = 0U;

    if ((radio == NULL) || ((buffer == NULL) && (length > 0U))) {
        return false;
    }

    RF24_BeginTransaction();

    if (!RF24_TransferByte(
            (uint8_t)(NRF24_W_REGISTER | (reg & NRF24_REGISTER_MASK)),
            &status)) {
        RF24_EndTransaction();
        return false;
    }

    radio->status = status;

    for (uint8_t i = 0U; i < length; ++i) {
        if (!RF24_TransferByte(buffer[i], &ignored)) {
            RF24_EndTransaction();
            return false;
        }
    }

    RF24_EndTransaction();
    return true;
}

static bool RF24_WriteRegisterByte(
    RF24_t *radio,
    uint8_t reg,
    uint8_t value)
{
    return RF24_WriteRegisterBuffer(radio, reg, &value, 1U);
}

static bool RF24_WritePayloadInternal(
    RF24_t *radio,
    const void *buffer,
    uint8_t data_length,
    uint8_t command)
{
    const uint8_t *current = (const uint8_t *)buffer;
    uint8_t status = 0xFFU;
    uint8_t ignored = 0U;
    uint8_t blank_length = 0U;

    if ((radio == NULL) || ((buffer == NULL) && (data_length > 0U))) {
        return false;
    }

    if (radio->dynamic_payloads_enabled) {
        data_length = RF24_MIN(data_length, RF24_MAX_PAYLOAD_SIZE);
    } else {
        data_length = RF24_MIN(data_length, radio->payload_size);
        blank_length = (uint8_t)(radio->payload_size - data_length);
    }

    RF24_BeginTransaction();

    if (!RF24_TransferByte(command, &status)) {
        RF24_EndTransaction();
        return false;
    }

    radio->status = status;

    for (uint8_t i = 0U; i < data_length; ++i) {
        if (!RF24_TransferByte(current[i], &ignored)) {
            RF24_EndTransaction();
            return false;
        }
    }

    for (uint8_t i = 0U; i < blank_length; ++i) {
        if (!RF24_TransferByte(0U, &ignored)) {
            RF24_EndTransaction();
            return false;
        }
    }

    RF24_EndTransaction();
    return true;
}

static bool RF24_ReadPayloadInternal(
    RF24_t *radio,
    void *buffer,
    uint8_t data_length)
{
    uint8_t *current = (uint8_t *)buffer;
    uint8_t status = 0xFFU;
    uint8_t ignored = 0U;
    uint8_t blank_length = 0U;

    if ((radio == NULL) || ((buffer == NULL) && (data_length > 0U))) {
        return false;
    }

    if (radio->dynamic_payloads_enabled) {
        data_length = RF24_MIN(data_length, RF24_MAX_PAYLOAD_SIZE);
    } else {
        data_length = RF24_MIN(data_length, radio->payload_size);
        blank_length = (uint8_t)(radio->payload_size - data_length);
    }

    RF24_BeginTransaction();

    if (!RF24_TransferByte(NRF24_R_RX_PAYLOAD, &status)) {
        RF24_EndTransaction();
        return false;
    }

    radio->status = status;

    for (uint8_t i = 0U; i < data_length; ++i) {
        if (!RF24_TransferByte(NRF24_NOP, &current[i])) {
            RF24_EndTransaction();
            return false;
        }
    }

    for (uint8_t i = 0U; i < blank_length; ++i) {
        if (!RF24_TransferByte(NRF24_NOP, &ignored)) {
            RF24_EndTransaction();
            return false;
        }
    }

    RF24_EndTransaction();
    return true;
}

static bool RF24_ToggleFeatures(RF24_t *radio)
{
    uint8_t status = 0xFFU;
    uint8_t ignored = 0U;

    if (radio == NULL) {
        return false;
    }

    RF24_BeginTransaction();

    if (!RF24_TransferByte(NRF24_ACTIVATE, &status)) {
        RF24_EndTransaction();
        return false;
    }

    radio->status = status;

    if (!RF24_TransferByte(0x73U, &ignored)) {
        RF24_EndTransaction();
        return false;
    }

    RF24_EndTransaction();
    return true;
}

static uint8_t RF24_GetDataRateRegisterValue(RF24_DataRate_t speed)
{
    switch (speed) {
        case RF24_DATA_RATE_250KBPS:
            return RF24_BIT(NRF24_RF_DR_LOW);

        case RF24_DATA_RATE_2MBPS:
            return RF24_BIT(NRF24_RF_DR_HIGH);

        case RF24_DATA_RATE_1MBPS:
        default:
            return 0U;
    }
}

static uint8_t RF24_GetPALevelRegisterValue(
    RF24_PALevel_t level,
    bool lna_enable)
{
    uint8_t value;

    switch (level) {
        case RF24_PA_MIN:
            value = 0U;
            break;

        case RF24_PA_LOW:
            value = RF24_BIT(NRF24_RF_PWR_LOW);
            break;

        case RF24_PA_HIGH:
            value = RF24_BIT(NRF24_RF_PWR_HIGH);
            break;

        case RF24_PA_MAX:
        default:
            value = (uint8_t)(RF24_BIT(NRF24_RF_PWR_LOW) |
                              RF24_BIT(NRF24_RF_PWR_HIGH));
            break;
    }

    if (lna_enable) {
        value |= RF24_BIT(NRF24_LNA_HCURR);
    }

    return value;
}

static bool RF24_WaitForTransmitResult(
    RF24_t *radio,
    uint32_t timeout_ms)
{
    uint32_t start;

    if (radio == NULL) {
        return false;
    }

    start = RF24_Millis();

    while (true) {
        uint8_t status = RF24_UpdateStatus(radio);

        if ((status & RF24_STATUS_IRQ_MASK) != 0U) {
            return true;
        }

        if ((RF24_Millis() - start) > timeout_ms) {
#if RF24_FAILURE_HANDLING
            radio->failure_detected = true;
            radio->failure_recovery_attempts++;
#endif
            RF24_CE_Low();
            (void)RF24_FlushRx(radio);
            (void)RF24_FlushTx(radio);
            return false;
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Public initialization                                                      */
/* -------------------------------------------------------------------------- */

void RF24_ResetObject(RF24_t *radio)
{
    if (radio == NULL) {
        return;
    }

    memset(radio, 0, sizeof(*radio));

    radio->spi_speed_hz = RF24_SPI_BAUDRATE_HZ;
    radio->payload_size = RF24_DEFAULT_PAYLOAD_SIZE;
    radio->address_width = RF24_DEFAULT_ADDRESS_WIDTH;
    radio->dynamic_payloads_enabled = false;
    radio->ack_payloads_enabled = false;
    radio->is_plus_variant = false;
    radio->pipe0_is_receiving = false;
    radio->tx_delay_us = 280U;
    radio->cs_delay_us = 5U;
}

bool RF24_Init(RF24_t *radio)
{
    uint8_t feature_before = 0U;
    uint8_t feature_after = 0U;
    uint8_t expected_config;
    uint8_t actual_config = 0U;

    if (radio == NULL) {
        PRINTF("RF24_Init FAIL: radio pointer is NULL\r\n");
        return false;
    }

    PRINTF("RF24_Init: ResetObject\r\n");
    RF24_ResetObject(radio);

    PRINTF("RF24_Init: PlatformInit\r\n");
    if (!RF24_PlatformInit(radio->spi_speed_hz)) {
        PRINTF("RF24_Init FAIL: PlatformInit\r\n");
        return false;
    }

    PRINTF("RF24_Init: CE LOW / CSN HIGH\r\n");
    RF24_CE_Low();
    RF24_CSN_High();
    RF24_DelayMs(5U);

    PRINTF("RF24_Init: SetRetries\r\n");
    if (!RF24_SetRetries(radio, RF24_DEFAULT_RETRY_DELAY, RF24_DEFAULT_RETRY_COUNT)) {
        PRINTF("RF24_Init FAIL: SetRetries\r\n");
        return false;
    }

    PRINTF("RF24_Init: SetDataRate\r\n");
    if (!RF24_SetDataRate(radio, RF24_DATA_RATE_1MBPS)) {
        PRINTF("RF24_Init FAIL: SetDataRate\r\n");
        return false;
    }

    PRINTF("RF24_Init: Read FEATURE before toggle\r\n");
    if (!RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature_before)) {
        PRINTF("RF24_Init FAIL: Read FEATURE before\r\n");
        return false;
    }
    PRINTF("FEATURE before = 0x%02X\r\n", feature_before);

    PRINTF("RF24_Init: ToggleFeatures\r\n");
    if (!RF24_ToggleFeatures(radio)) {
        PRINTF("RF24_Init FAIL: ToggleFeatures\r\n");
        return false;
    }

    PRINTF("RF24_Init: Read FEATURE after toggle\r\n");
    if (!RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature_after)) {
        PRINTF("RF24_Init FAIL: Read FEATURE after\r\n");
        return false;
    }
    PRINTF("FEATURE after = 0x%02X\r\n", feature_after);

    radio->is_plus_variant = (feature_before == feature_after);

    if (feature_after != 0U) {
        if (radio->is_plus_variant) {
            PRINTF("RF24_Init: ToggleFeatures second time\r\n");
            if (!RF24_ToggleFeatures(radio)) {
                PRINTF("RF24_Init FAIL: second ToggleFeatures\r\n");
                return false;
            }
        }

        PRINTF("RF24_Init: Clear FEATURE\r\n");
        if (!RF24_WriteRegisterByte(radio, NRF24_FEATURE, 0U)) {
            PRINTF("RF24_Init FAIL: clear FEATURE\r\n");
            return false;
        }
    }

    radio->ack_payloads_enabled = false;
    radio->dynamic_payloads_enabled = false;

    PRINTF("RF24_Init: Configure default registers\r\n");
    if (!RF24_WriteRegisterByte(radio, NRF24_DYNPD, 0U)) {
        PRINTF("RF24_Init FAIL: DYNPD\r\n");
        return false;
    }
    if (!RF24_WriteRegisterByte(radio, NRF24_EN_AA, 0x3FU)) {
        PRINTF("RF24_Init FAIL: EN_AA\r\n");
        return false;
    }
    if (!RF24_WriteRegisterByte(radio, NRF24_EN_RXADDR, 0x03U)) {
        PRINTF("RF24_Init FAIL: EN_RXADDR\r\n");
        return false;
    }

    PRINTF("RF24_Init: SetPayloadSize\r\n");
    if (!RF24_SetPayloadSize(radio, RF24_DEFAULT_PAYLOAD_SIZE)) {
        PRINTF("RF24_Init FAIL: SetPayloadSize\r\n");
        return false;
    }

    PRINTF("RF24_Init: SetAddressWidth\r\n");
    if (!RF24_SetAddressWidth(radio, RF24_DEFAULT_ADDRESS_WIDTH)) {
        PRINTF("RF24_Init FAIL: SetAddressWidth\r\n");
        return false;
    }

    PRINTF("RF24_Init: SetChannel\r\n");
    if (!RF24_SetChannel(radio, RF24_DEFAULT_CHANNEL)) {
        PRINTF("RF24_Init FAIL: SetChannel\r\n");
        return false;
    }

    PRINTF("RF24_Init: Clear status / flush FIFOs\r\n");
    (void)RF24_ClearStatusFlags(radio, RF24_IRQ_ALL);
    (void)RF24_FlushRx(radio);
    (void)RF24_FlushTx(radio);

    expected_config = (uint8_t)(
        RF24_BIT(NRF24_EN_CRC) |
        RF24_BIT(NRF24_CRCO) |
        RF24_BIT(NRF24_MASK_RX_DR) |
        RF24_BIT(NRF24_MASK_TX_DS) |
        RF24_BIT(NRF24_MASK_MAX_RT));

    PRINTF("RF24_Init: Write CONFIG = 0x%02X\r\n", expected_config);
    if (!RF24_WriteRegisterByte(radio, NRF24_CONFIG, expected_config)) {
        PRINTF("RF24_Init FAIL: write CONFIG\r\n");
        return false;
    }

    radio->config_register = expected_config;

    PRINTF("RF24_Init: PowerUp\r\n");
    if (!RF24_PowerUp(radio)) {
        PRINTF("RF24_Init FAIL: PowerUp\r\n");
        return false;
    }

    PRINTF("RF24_Init: Read CONFIG back\r\n");
    if (!RF24_ReadRegisterByte(radio, NRF24_CONFIG, &actual_config)) {
        PRINTF("RF24_Init FAIL: read CONFIG\r\n");
        return false;
    }

    expected_config |= RF24_BIT(NRF24_PWR_UP);

    PRINTF("CONFIG expected = 0x%02X\r\n", expected_config);
    PRINTF("CONFIG actual   = 0x%02X\r\n", actual_config);

    if (actual_config != expected_config) {
        PRINTF("RF24_Init FAIL: CONFIG mismatch\r\n");
        return false;
    }

    PRINTF("RF24_Init SUCCESS\r\n");
    return true;
}

bool RF24_IsChipConnected(RF24_t *radio)
{
    uint8_t setup_aw = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(radio, NRF24_SETUP_AW, &setup_aw)) {
        return false;
    }

    return setup_aw == (uint8_t)(radio->address_width - 2U);
}

/* -------------------------------------------------------------------------- */
/* Basic register/status operations                                           */
/* -------------------------------------------------------------------------- */

uint8_t RF24_UpdateStatus(RF24_t *radio)
{
    if (radio == NULL) {
        return 0xFFU;
    }

    (void)RF24_Command(radio, NRF24_NOP);
    return radio->status;
}

uint8_t RF24_GetStatus(const RF24_t *radio)
{
    return (radio != NULL) ? radio->status : 0xFFU;
}

uint8_t RF24_ClearStatusFlags(RF24_t *radio, uint8_t flags)
{
    if (radio == NULL) {
        return 0xFFU;
    }

    (void)RF24_WriteRegisterByte(
        radio,
        NRF24_STATUS,
        (uint8_t)(flags & RF24_IRQ_ALL));

    return radio->status;
}

bool RF24_SetStatusFlags(RF24_t *radio, uint8_t flags)
{
    uint8_t config;

    if (radio == NULL) {
        return false;
    }

    config = (uint8_t)(
        (radio->config_register & (uint8_t)~RF24_IRQ_ALL) |
        ((uint8_t)~flags & RF24_IRQ_ALL));

    if (!RF24_WriteRegisterByte(radio, NRF24_CONFIG, config)) {
        return false;
    }

    radio->config_register = config;
    return true;
}

bool RF24_FlushRx(RF24_t *radio)
{
    return RF24_Command(radio, NRF24_FLUSH_RX);
}

bool RF24_FlushTx(RF24_t *radio)
{
    return RF24_Command(radio, NRF24_FLUSH_TX);
}

/* -------------------------------------------------------------------------- */
/* Configuration                                                              */
/* -------------------------------------------------------------------------- */

bool RF24_SetChannel(RF24_t *radio, uint8_t channel)
{
    return RF24_WriteRegisterByte(
        radio,
        NRF24_RF_CH,
        RF24_MIN(channel, RF24_MAX_CHANNEL));
}

uint8_t RF24_GetChannel(RF24_t *radio)
{
    uint8_t channel = 0U;
    (void)RF24_ReadRegisterByte(radio, NRF24_RF_CH, &channel);
    return channel;
}

bool RF24_SetPayloadSize(RF24_t *radio, uint8_t size)
{
    if (radio == NULL) {
        return false;
    }

    size = RF24_MAX(1U, RF24_MIN(size, RF24_MAX_PAYLOAD_SIZE));
    radio->payload_size = size;

    for (uint8_t pipe = 0U; pipe < 6U; ++pipe) {
        if (!RF24_WriteRegisterByte(
                radio,
                s_child_payload_size[pipe],
                size)) {
            return false;
        }
    }

    return true;
}

uint8_t RF24_GetPayloadSize(const RF24_t *radio)
{
    return (radio != NULL) ? radio->payload_size : 0U;
}

bool RF24_SetAddressWidth(RF24_t *radio, uint8_t width)
{
    if (radio == NULL) {
        return false;
    }

    width = RF24_MAX(3U, RF24_MIN(width, 5U));

    if (!RF24_WriteRegisterByte(
            radio,
            NRF24_SETUP_AW,
            (uint8_t)(width - 2U))) {
        return false;
    }

    radio->address_width = width;
    return true;
}

bool RF24_SetRetries(RF24_t *radio, uint8_t delay, uint8_t count)
{
    uint8_t value =
        (uint8_t)(((delay & 0x0FU) << NRF24_ARD) |
                  ((count & 0x0FU) << NRF24_ARC));

    return RF24_WriteRegisterByte(radio, NRF24_SETUP_RETR, value);
}

bool RF24_SetDataRate(RF24_t *radio, RF24_DataRate_t speed)
{
    uint8_t setup = 0U;
    uint8_t verify = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(radio, NRF24_RF_SETUP, &setup)) {
        return false;
    }

    setup &= (uint8_t)~(
        RF24_BIT(NRF24_RF_DR_LOW) |
        RF24_BIT(NRF24_RF_DR_HIGH));

    setup |= RF24_GetDataRateRegisterValue(speed);

    if (!RF24_WriteRegisterByte(radio, NRF24_RF_SETUP, setup) ||
        !RF24_ReadRegisterByte(radio, NRF24_RF_SETUP, &verify)) {
        return false;
    }

    switch (speed) {
        case RF24_DATA_RATE_250KBPS:
            radio->tx_delay_us = 505U;
            break;

        case RF24_DATA_RATE_2MBPS:
            radio->tx_delay_us = 240U;
            break;

        case RF24_DATA_RATE_1MBPS:
        default:
            radio->tx_delay_us = 280U;
            break;
    }

    return verify == setup;
}

RF24_DataRate_t RF24_GetDataRate(RF24_t *radio)
{
    uint8_t setup = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(radio, NRF24_RF_SETUP, &setup)) {
        return RF24_DATA_RATE_1MBPS;
    }

    if ((setup & RF24_BIT(NRF24_RF_DR_LOW)) != 0U) {
        return RF24_DATA_RATE_250KBPS;
    }

    if ((setup & RF24_BIT(NRF24_RF_DR_HIGH)) != 0U) {
        return RF24_DATA_RATE_2MBPS;
    }

    return RF24_DATA_RATE_1MBPS;
}

bool RF24_SetPALevel(
    RF24_t *radio,
    RF24_PALevel_t level,
    bool lna_enable)
{
    uint8_t setup = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(radio, NRF24_RF_SETUP, &setup)) {
        return false;
    }

    setup &= (uint8_t)~(
        RF24_BIT(NRF24_RF_PWR_LOW) |
        RF24_BIT(NRF24_RF_PWR_HIGH) |
        RF24_BIT(NRF24_LNA_HCURR));

    setup |= RF24_GetPALevelRegisterValue(level, lna_enable);

    return RF24_WriteRegisterByte(radio, NRF24_RF_SETUP, setup);
}

RF24_PALevel_t RF24_GetPALevel(RF24_t *radio)
{
    uint8_t setup = 0U;
    uint8_t level_bits;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(radio, NRF24_RF_SETUP, &setup)) {
        return RF24_PA_MIN;
    }

    level_bits = (uint8_t)(
        setup &
        (RF24_BIT(NRF24_RF_PWR_LOW) |
         RF24_BIT(NRF24_RF_PWR_HIGH)));

    if (level_bits ==
        (RF24_BIT(NRF24_RF_PWR_LOW) |
         RF24_BIT(NRF24_RF_PWR_HIGH))) {
        return RF24_PA_MAX;
    }

    if (level_bits == RF24_BIT(NRF24_RF_PWR_HIGH)) {
        return RF24_PA_HIGH;
    }

    if (level_bits == RF24_BIT(NRF24_RF_PWR_LOW)) {
        return RF24_PA_LOW;
    }

    return RF24_PA_MIN;
}

bool RF24_SetAutoAck(RF24_t *radio, bool enable)
{
    return RF24_WriteRegisterByte(
        radio,
        NRF24_EN_AA,
        enable ? 0x3FU : 0U);
}

bool RF24_SetAutoAckPipe(
    RF24_t *radio,
    uint8_t pipe,
    bool enable)
{
    uint8_t en_aa = 0U;

    if ((radio == NULL) || (pipe > 5U) ||
        !RF24_ReadRegisterByte(radio, NRF24_EN_AA, &en_aa)) {
        return false;
    }

    if (enable) {
        en_aa |= RF24_BIT(pipe);
    } else {
        en_aa &= (uint8_t)~RF24_BIT(pipe);
    }

    return RF24_WriteRegisterByte(radio, NRF24_EN_AA, en_aa);
}

bool RF24_EnableDynamicPayloads(RF24_t *radio)
{
    uint8_t feature = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature)) {
        return false;
    }

    feature |= RF24_BIT(NRF24_EN_DPL);

    if (!RF24_WriteRegisterByte(radio, NRF24_FEATURE, feature) ||
        !RF24_WriteRegisterByte(radio, NRF24_DYNPD, 0x3FU)) {
        return false;
    }

    radio->dynamic_payloads_enabled = true;
    return true;
}

bool RF24_DisableDynamicPayloads(RF24_t *radio)
{
    uint8_t feature = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature)) {
        return false;
    }

    feature &= (uint8_t)~RF24_BIT(NRF24_EN_DPL);

    if (!RF24_WriteRegisterByte(radio, NRF24_FEATURE, feature) ||
        !RF24_WriteRegisterByte(radio, NRF24_DYNPD, 0U)) {
        return false;
    }

    radio->dynamic_payloads_enabled = false;
    return true;
}

bool RF24_EnableAckPayload(RF24_t *radio)
{
    uint8_t feature = 0U;
    uint8_t dynpd = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature) ||
        !RF24_ReadRegisterByte(radio, NRF24_DYNPD, &dynpd)) {
        return false;
    }

    feature |= (uint8_t)(
        RF24_BIT(NRF24_EN_ACK_PAY) |
        RF24_BIT(NRF24_EN_DPL));

    dynpd |= (uint8_t)(
        RF24_BIT(NRF24_DPL_P0) |
        RF24_BIT(NRF24_DPL_P1));

    if (!RF24_WriteRegisterByte(radio, NRF24_FEATURE, feature) ||
        !RF24_WriteRegisterByte(radio, NRF24_DYNPD, dynpd)) {
        return false;
    }

    radio->ack_payloads_enabled = true;
    radio->dynamic_payloads_enabled = true;
    return true;
}

bool RF24_DisableAckPayload(RF24_t *radio)
{
    uint8_t feature = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature)) {
        return false;
    }

    feature &= (uint8_t)~RF24_BIT(NRF24_EN_ACK_PAY);

    if (!RF24_WriteRegisterByte(radio, NRF24_FEATURE, feature)) {
        return false;
    }

    radio->ack_payloads_enabled = false;
    return true;
}

bool RF24_EnableDynamicAck(RF24_t *radio)
{
    uint8_t feature = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(radio, NRF24_FEATURE, &feature)) {
        return false;
    }

    feature |= RF24_BIT(NRF24_EN_DYN_ACK);

    return RF24_WriteRegisterByte(radio, NRF24_FEATURE, feature);
}

/* -------------------------------------------------------------------------- */
/* Addressing                                                                 */
/* -------------------------------------------------------------------------- */

bool RF24_OpenWritingPipe(
    RF24_t *radio,
    const uint8_t *address)
{
    if ((radio == NULL) || (address == NULL)) {
        return false;
    }

    memcpy(
        radio->pipe0_writing_address,
        address,
        radio->address_width);

    return RF24_WriteRegisterBuffer(
               radio,
               NRF24_RX_ADDR_P0,
               address,
               radio->address_width) &&
           RF24_WriteRegisterBuffer(
               radio,
               NRF24_TX_ADDR,
               address,
               radio->address_width);
}

bool RF24_OpenReadingPipe(
    RF24_t *radio,
    uint8_t pipe,
    const uint8_t *address)
{
    uint8_t en_rxaddr = 0U;

    if ((radio == NULL) ||
        (address == NULL) ||
        (pipe > 5U)) {
        return false;
    }

    if (pipe == 0U) {
        memcpy(
            radio->pipe0_reading_address,
            address,
            radio->address_width);
        radio->pipe0_is_receiving = true;
    }

    if (pipe < 2U) {
        if (!RF24_WriteRegisterBuffer(
                radio,
                s_child_pipe_address[pipe],
                address,
                radio->address_width)) {
            return false;
        }
    } else {
        if (!RF24_WriteRegisterByte(
                radio,
                s_child_pipe_address[pipe],
                address[0])) {
            return false;
        }
    }

    if (!RF24_WriteRegisterByte(
            radio,
            s_child_payload_size[pipe],
            radio->payload_size) ||
        !RF24_ReadRegisterByte(
            radio,
            NRF24_EN_RXADDR,
            &en_rxaddr)) {
        return false;
    }

    en_rxaddr |= RF24_BIT(s_child_pipe_enable[pipe]);

    return RF24_WriteRegisterByte(
        radio,
        NRF24_EN_RXADDR,
        en_rxaddr);
}

bool RF24_CloseReadingPipe(RF24_t *radio, uint8_t pipe)
{
    uint8_t en_rxaddr = 0U;

    if ((radio == NULL) ||
        (pipe > 5U) ||
        !RF24_ReadRegisterByte(
            radio,
            NRF24_EN_RXADDR,
            &en_rxaddr)) {
        return false;
    }

    en_rxaddr &= (uint8_t)~RF24_BIT(s_child_pipe_enable[pipe]);

    if (pipe == 0U) {
        radio->pipe0_is_receiving = false;
    }

    return RF24_WriteRegisterByte(
        radio,
        NRF24_EN_RXADDR,
        en_rxaddr);
}

/* -------------------------------------------------------------------------- */
/* Power and operating modes                                                  */
/* -------------------------------------------------------------------------- */

bool RF24_PowerUp(RF24_t *radio)
{
    if (radio == NULL) {
        return false;
    }

    if ((radio->config_register & RF24_BIT(NRF24_PWR_UP)) == 0U) {
        radio->config_register |= RF24_BIT(NRF24_PWR_UP);

        if (!RF24_WriteRegisterByte(
                radio,
                NRF24_CONFIG,
                radio->config_register)) {
            return false;
        }

        RF24_DelayUs(RF24_POWERUP_DELAY_US);
    }

    return true;
}

bool RF24_PowerDown(RF24_t *radio)
{
    if (radio == NULL) {
        return false;
    }

    RF24_CE_Low();

    radio->config_register &=
        (uint8_t)~RF24_BIT(NRF24_PWR_UP);

    return RF24_WriteRegisterByte(
        radio,
        NRF24_CONFIG,
        radio->config_register);
}

bool RF24_StartListening(RF24_t *radio)
{
    if ((radio == NULL) ||
        !RF24_PowerUp(radio)) {
        return false;
    }

    radio->config_register |= RF24_BIT(NRF24_PRIM_RX);

    if (!RF24_WriteRegisterByte(
            radio,
            NRF24_CONFIG,
            radio->config_register) ||
        !RF24_WriteRegisterByte(
            radio,
            NRF24_STATUS,
            RF24_IRQ_ALL)) {
        return false;
    }

    if (radio->pipe0_is_receiving) {
        if (!RF24_WriteRegisterBuffer(
                radio,
                NRF24_RX_ADDR_P0,
                radio->pipe0_reading_address,
                radio->address_width)) {
            return false;
        }
    } else {
        if (!RF24_CloseReadingPipe(radio, 0U)) {
            return false;
        }
    }

    RF24_CE_High();
    return true;
}

bool RF24_StopListening(RF24_t *radio)
{
    uint8_t en_rxaddr = 0U;

    if (radio == NULL) {
        return false;
    }

    RF24_CE_Low();
    RF24_DelayUs(radio->tx_delay_us);

    if (radio->ack_payloads_enabled) {
        (void)RF24_FlushTx(radio);
    }

    radio->config_register &=
        (uint8_t)~RF24_BIT(NRF24_PRIM_RX);

    if (!RF24_WriteRegisterByte(
            radio,
            NRF24_CONFIG,
            radio->config_register) ||
        !RF24_WriteRegisterBuffer(
            radio,
            NRF24_RX_ADDR_P0,
            radio->pipe0_writing_address,
            radio->address_width) ||
        !RF24_ReadRegisterByte(
            radio,
            NRF24_EN_RXADDR,
            &en_rxaddr)) {
        return false;
    }

    en_rxaddr |= RF24_BIT(NRF24_ERX_P0);

    return RF24_WriteRegisterByte(
        radio,
        NRF24_EN_RXADDR,
        en_rxaddr);
}

/* -------------------------------------------------------------------------- */
/* TX/RX                                                                      */
/* -------------------------------------------------------------------------- */

bool RF24_Available(RF24_t *radio)
{
    uint8_t status;

    if (radio == NULL) {
        return false;
    }

    status = RF24_UpdateStatus(radio);
    return ((status >> NRF24_RX_P_NO) & 0x07U) < 6U;
}

bool RF24_AvailablePipe(
    RF24_t *radio,
    uint8_t *pipe_number)
{
    uint8_t status;
    uint8_t pipe;

    if (radio == NULL) {
        return false;
    }

    status = RF24_UpdateStatus(radio);
    pipe = (uint8_t)((status >> NRF24_RX_P_NO) & 0x07U);

    if (pipe_number != NULL) {
        *pipe_number = pipe;
    }

    return pipe < 6U;
}

bool RF24_Read(
    RF24_t *radio,
    void *buffer,
    uint8_t length)
{
    if ((radio == NULL) ||
        !RF24_ReadPayloadInternal(radio, buffer, length)) {
        return false;
    }

    return RF24_WriteRegisterByte(
        radio,
        NRF24_STATUS,
        RF24_BIT(NRF24_RX_DR));
}

uint8_t RF24_GetDynamicPayloadSize(RF24_t *radio)
{
    uint8_t width = 0U;
    uint8_t status = 0xFFU;

    if (radio == NULL) {
        return 0U;
    }

    RF24_BeginTransaction();

    if (!RF24_TransferByte(NRF24_R_RX_PL_WID, &status) ||
        !RF24_TransferByte(NRF24_NOP, &width)) {
        RF24_EndTransaction();
        return 0U;
    }

    RF24_EndTransaction();
    radio->status = status;

    if (width > RF24_MAX_PAYLOAD_SIZE) {
        (void)RF24_FlushRx(radio);
        RF24_DelayMs(2U);
        return 0U;
    }

    return width;
}

bool RF24_StartFastWrite(
    RF24_t *radio,
    const void *buffer,
    uint8_t length,
    bool no_ack,
    bool start_tx)
{
    uint8_t command;

    if (radio == NULL) {
        return false;
    }

    command = no_ack
        ? NRF24_W_TX_PAYLOAD_NO_ACK
        : NRF24_W_TX_PAYLOAD;

    if (!RF24_WritePayloadInternal(
            radio,
            buffer,
            length,
            command)) {
        return false;
    }

    if (start_tx) {
        RF24_CE_High();
    }

    return true;
}

bool RF24_Write(
    RF24_t *radio,
    const void *buffer,
    uint8_t length)
{
    return RF24_WriteNoAck(radio, buffer, length, false);
}

bool RF24_WriteNoAck(
    RF24_t *radio,
    const void *buffer,
    uint8_t length,
    bool no_ack)
{
    bool wait_ok;

    if (radio == NULL) {
        return false;
    }

    if (!RF24_StartFastWrite(
            radio,
            buffer,
            length,
            no_ack,
            true)) {
        return false;
    }

    wait_ok = RF24_WaitForTransmitResult(
        radio,
        RF24_DEFAULT_TX_TIMEOUT_MS);

    RF24_CE_Low();

    if (!wait_ok) {
        return false;
    }

    (void)RF24_ClearStatusFlags(radio, RF24_IRQ_ALL);

    if ((radio->status & RF24_IRQ_TX_FAILED) != 0U) {
        (void)RF24_FlushTx(radio);
        return false;
    }

    return true;
}

bool RF24_WriteAckPayload(
    RF24_t *radio,
    uint8_t pipe,
    const void *buffer,
    uint8_t length)
{
    if ((radio == NULL) ||
        (pipe > 5U) ||
        !radio->ack_payloads_enabled) {
        return false;
    }

    return RF24_WritePayloadInternal(
        radio,
        buffer,
        length,
        (uint8_t)(NRF24_W_ACK_PAYLOAD | (pipe & 0x07U)));
}

bool RF24_RxFifoFull(RF24_t *radio)
{
    uint8_t fifo = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(
            radio,
            NRF24_FIFO_STATUS,
            &fifo)) {
        return false;
    }

    return (fifo & RF24_BIT(NRF24_RX_FULL)) != 0U;
}

RF24_FifoState_t RF24_GetFifoState(
    RF24_t *radio,
    bool tx_fifo)
{
    uint8_t fifo = 0U;
    bool full;
    bool empty;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(
            radio,
            NRF24_FIFO_STATUS,
            &fifo)) {
        return RF24_FIFO_INVALID;
    }

    if (tx_fifo) {
        full = (fifo & RF24_BIT(NRF24_FIFO_FULL)) != 0U;
        empty = (fifo & RF24_BIT(NRF24_TX_EMPTY)) != 0U;
    } else {
        full = (fifo & RF24_BIT(NRF24_RX_FULL)) != 0U;
        empty = (fifo & RF24_BIT(NRF24_RX_EMPTY)) != 0U;
    }

    if (full && empty) {
        return RF24_FIFO_INVALID;
    }

    if (full) {
        return RF24_FIFO_FULL;
    }

    if (empty) {
        return RF24_FIFO_EMPTY;
    }

    return RF24_FIFO_OCCUPIED;
}

/* -------------------------------------------------------------------------- */
/* Carrier detection and diagnostics                                          */
/* -------------------------------------------------------------------------- */

bool RF24_TestCarrier(RF24_t *radio)
{
    uint8_t rpd = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(radio, NRF24_RPD, &rpd)) {
        return false;
    }

    return (rpd & 0x01U) != 0U;
}

bool RF24_IsPlusVariant(const RF24_t *radio)
{
    return (radio != NULL) && radio->is_plus_variant;
}

uint8_t RF24_GetARC(RF24_t *radio)
{
    uint8_t observe = 0U;

    if ((radio == NULL) ||
        !RF24_ReadRegisterByte(
            radio,
            NRF24_OBSERVE_TX,
            &observe)) {
        return 0U;
    }

    return (uint8_t)(observe & 0x0FU);
}
