/**
 * @file        bm_usb_def.h
 * @brief       Busmust USB device data type definitions.
 * @author      busmust
 * @version     1.13.0.35
 * @copyright   Copyright 2020 by Busmust Tech Co.,Ltd <br>
 *              All rights reserved. Property of Busmust Tech Co.,Ltd.<br>
 *              Restricted rights to use, duplicate or disclose of this code are granted through contract.
 */
#ifndef __BM_USB_DEF_H__
#define __BM_USB_DEF_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def   BM_DATA_HEADER_SIZE
 * @brief Size (in bytes) of BM Data header, which contains type, routing, length and timestamp.
 */
#define BM_DATA_HEADER_SIZE (8U)

#define BM_DATA_TAIL_SIZE (16U)

/**
 * @def   BM_DATA_PAYLOAD_MAX_SIZE
 * @brief Size (in bytes) of BM Data payload, which contains a concrete message in CANFD|LIN|FLEXRAY|... type.
 */
#define BM_DATA_PAYLOAD_MAX_SIZE (72U + BM_DATA_TAIL_SIZE)

/**
 * @def   BM_JUMBO_DATA_PAYLOAD_MAX_SIZE
 * @brief Size (in bytes) of BM Jumbo Data payload, which contains a concrete message in ETH|CANTP|MODBUS... type.
 */
#define BM_JUMBO_DATA_PAYLOAD_MAX_SIZE (9*1024U + BM_DATA_TAIL_SIZE)

/**
 * @def   BM_DATA_PAYLOAD_MAX_VALID_LENGTH
 * @brief Report an error if greater.
 */
#define BM_DATA_PAYLOAD_MAX_VALID_LENGTH (1024U)

/**
 * @def   BM_DATA_MAX_SIZE
 * @brief Size (in bytes) of BM Data, which contains a header and payload.
 */
#define BM_DATA_MAX_SIZE (BM_DATA_HEADER_SIZE + BM_DATA_PAYLOAD_MAX_SIZE)

/**
 * @def   BM_JUMBO_DATA_MAX_SIZE
 * @brief Size (in bytes) of BM Jumbo Data, which contains a header and payload.
 */
#define BM_JUMBO_DATA_MAX_SIZE (BM_DATA_HEADER_SIZE + BM_JUMBO_DATA_PAYLOAD_MAX_SIZE)

/**
* @def   BM_REMOTE_IO_UDP_SOCKET_PORT
* @brief Remote IO UDP socket port.
* @note  This is a simple READONLY indicator for customers, DO NOT try to change this macro without re-compiling the BMAPI library.
*/
#define BM_REMOTE_IO_UDP_SOCKET_PORT 2502

/**
* @def   BM_REMOTE_CTRL_UDP_SOCKET_PORT
* @brief Remote CTRL UDP socket port.
* @note  This is a simple READONLY indicator for customers, DO NOT try to change this macro without re-compiling the BMAPI library.
*/
#define BM_REMOTE_CTRL_UDP_SOCKET_PORT 3502

/**
 * @enum  BM_LogLevelTypeDef
 * @brief Busmust library log level, see BM_SetLogLevel() for details.
 */
typedef enum
{
    BM_LOG_NONE = 0,             /**< Show nothing on debug console                                                                                 */
    BM_LOG_ERR,                  /**< Show only ERR level messages on debug console, note this is the default level for release versions            */
    BM_LOG_WRN,                  /**< Show ERR and WRN level messages on debug console                                                              */
    BM_LOG_INF,                  /**< Show ERR|WRN|INF level messages on debug console                                                              */
    BM_LOG_DBG                   /**< Show all messages on debug console, including debug messages, note this is NOT available for release versions */
} BM_LogLevelTypeDef;

/**
 * @enum  BM_CapabilityTypeDef
 * @brief Busmust Device capability flags, retrieved when enumerating devices using BM_Enumerate().
 */
typedef enum
{
    BM_NONE_CAP = 0x0000U,      /**< No capability */
    BM_LIN_CAP = 0x0001U,       /**< The device is capable of handling LIN messages */
    BM_CAN_CAP = 0x0002U,       /**< The device is capable of handling CAN messages */
    BM_CAN_FD_CAP = 0x0004U,    /**< The device is capable of handling CANFD (and CAN) messages */
    BM_FLEXRAY_CAP = 0x0008U,   /**< The device is capable of handling FLEXRAY messages */
    BM_MODBUS_CAP = 0x0010U,    /**< The device is capable of handling MODBUS messages */
    BM_ETHERNET_CAP = 0x0020U,  /**< The device is capable of handling ETHERNET messages */
    BM_AIO_CAP = 0x0100U,       /**< The device is capable of analog IO */
    BM_DIO_CAP = 0x0200U,       /**< The device is capable of digital IO */
    BM_VIRTUAL_CAP = 0x4000U,  /**< The device is virtual, for simulation purpose */
    BM_REMOTE_CAP = 0x8000U,    /**< The device is connected remotely and is capable of remote management */
    BM_ALL_CAP = 0xFFFFU        /**< Typically used for masking the CAP fields when programming */
} BM_CapabilityTypeDef;

/**
 * @enum  BM_DataTypeTypeDef
 * @brief Busmust data type flags, must be given in BM_DataTypeDef.
 */
typedef enum
{
    BM_UNKNOWN_DATA = 0,        /**< Unknown data type */
    BM_LIN_DATA,                /**< LIN message data type */
    BM_CAN_FD_DATA,             /**< CAN or CAN-FD message data type (check FDF flag further) */
    BM_FLEXRAY_DATA,            /**< Flexray message data type */
    BM_MODBUS_DATA,             /**< MODBUS message data type */
    BM_ETHERNET_DATA,           /**< Ethernet message data type */
    BM_ACK_DATA = 0x8U,         /**< ACK from bus, which indicates TXCMPLT event if this is BM_CAN_FD_DATA */
    BM_SYSTEM_DATA = 0xFU,      /**< Reserved for system internal usage, user shall discard the message when received */
} BM_DataTypeTypeDef;

/**
 * @enum  BM_StatusTypeDef
 * @brief Busmust device & operation status, most APIs would return a status code to indicate the result of an operation.
 */
typedef enum
{
    BM_ERROR_OK = 0x00000,                      /**< SUCCESS: No error occurred */
    BM_ERROR_XMTFULL = 0x00001,                 /**< Low level Transmit buffer is full */
    BM_ERROR_OVERRUN = 0x00002,                 /**< Bus overrun (the device cannot keep up with the high bus throughput) */
    BM_ERROR_BUSLIGHT = 0x00004,                /**< CAN Bus communication is light, see ISO11898 for details */
    BM_ERROR_BUSHEAVY = 0x00008,                /**< CAN Bus communication is heavy, see ISO11898 for details */
    BM_ERROR_BUSWARNING = BM_ERROR_BUSHEAVY,    /**< CAN Bus communication is in warning state, see ISO11898 for details */
    BM_ERROR_BUSPASSIVE = 0x40000,              /**< CAN node is in passive state, see ISO11898 for details */
    BM_ERROR_BUSTIMEOUT = 0x80000,              /**< CAN node failed to transmit message within specified time, the node might be in PASSIVE or BUSOFF state */
    BM_ERROR_BUSOFF = 0x00010,                  /**< CAN bus is in BUSOFF state, see ISO11898 for details */
    BM_ERROR_ANYBUSERR = (BM_ERROR_BUSWARNING | BM_ERROR_BUSLIGHT | BM_ERROR_BUSHEAVY | BM_ERROR_BUSOFF | BM_ERROR_BUSPASSIVE), /**< CAN bus error occurred */
    BM_ERROR_QRCVEMPTY = 0x00020,               /**< Receive buffer is empty, this might NOT be an error if you use BMAPI in polling mode */
    BM_ERROR_QOVERRUN = 0x00040,                /**< BMAPI internal Q overrun */
    BM_ERROR_QXMTFULL = 0x00080,                /**< High level Transmit queue is full */
    BM_ERROR_REGTEST = 0x00100,                 /**< Reserved */
    BM_ERROR_NODRIVER = 0x00200,                /**< Reserved */
    BM_ERROR_HWINUSE = 0x00400,                 /**< Hardware is in use (opened by another application) */
    BM_ERROR_NETINUSE = 0x00800,                /**< Reserved */
    BM_ERROR_ILLHW = 0x01400,                   /**< Hardware error or invalid hardware version/handle */
    BM_ERROR_ILLNET = 0x01800,                  /**< Invalid bus */
    BM_ERROR_ILLCLIENT = 0x01C00,               /**< Invalid client */
    BM_ERROR_ILLHANDLE = (BM_ERROR_ILLHW | BM_ERROR_ILLNET | BM_ERROR_ILLCLIENT), /* Invalid handle*/
    BM_ERROR_RESOURCE = 0x02000,                /**< Out of resource */
    BM_ERROR_ILLPARAMTYPE = 0x04000,            /**< Invalid parameter type in API call */
    BM_ERROR_ILLPARAMVAL = 0x08000,             /**< Invalid parameter value in API call */
    BM_ERROR_UNKNOWN = 0x10000,                 /**< Unknown error */
    BM_ERROR_ILLDATA = 0x20000,                 /**< Invalid data received/transmitted */
    BM_ERROR_CONFIG = 0X1000000,                /**< Invalid configuration */
    BM_ERROR_CAUTION = 0x2000000,               /**< Reserved */
    BM_ERROR_INITIALIZE = 0x4000000,            /**< The device/library is not initialized */
    BM_ERROR_ILLOPERATION = 0x8000000           /**< Invalid operation */
} BM_StatusTypeDef;

typedef enum 
{
    BM_DEFAULT_BUFFER = 0x0000,                 /**< Default buffer: txq for write, rxq for read */
    BM_LOGGINGFILE_BUFFER = 0x0001,             /**< Buffer for logging file operations */
    BM_REPLAYFILE_BUFFER = 0x0002,              /**< Buffer for replay file operations */
    BM_LOGGINGQ_BUFFER = 0x0111,                /**< Buffer for logging queue */
    BM_REPLAYQ_BUFFER = 0x0222,                 /**< Buffer for replay queue */
    BM_RXQ_BUFFER = 0x1111,                     /**< RX queue buffer (compatible with BM_ClearBuffer) */
    BM_TXQ_BUFFER = 0x2222,                     /**< TX queue buffer (compatible with BM_ClearBuffer) */
    BM_NO_BUFFER = 0xFFFF                       /**< No buffer: direct physical channel access for low latency */
} BM_BufferId;

typedef enum 
{ 
    BM_WRITE_BUFFER = 0,                        /**< Write buffer*/
    BM_READ_BUFFER = 0x8000U                    /**< Read buffer*/
} BM_BufferTypeDef;

/**
 * @enum  BM_CanModeTypeDef
 * @brief CAN mode IDs, used by BM_SetCanMode() to change the operation mode of CAN device.
 */
typedef enum
{
    BM_CAN_OFF_MODE = 0x01,                     /**< The device is logically disconnected from CAN bus */
    BM_CAN_NORMAL_MODE = 0x00,                  /**< The device is running normally (with the capability to handle CAN and CANFD messages */
    BM_CAN_SLEEP_MODE = 0x01,                   /**< The device is logically disconnected from CAN bus */
    BM_CAN_INTERNAL_LOOPBACK_MODE = 0x02,       /**< The device is looping back messages internally without impacting the physical CAN bus */
    BM_CAN_LISTEN_ONLY_MODE = 0x03,             /**< The device is receiving messages without impacting the physical CAN bus (do not send ACKs to the bus) */
    BM_CAN_CONFIGURATION_MODE = 0x04,           /**< The device is under configuration and temporarily disconnected from CAN bus, For Internal usage only */
    BM_CAN_EXTERNAL_LOOPBACK_MODE = 0x05,       /**< The device is looping back messages externally, all transmitted messages are echoed by the device itself */
    BM_CAN_CLASSIC_MODE = 0x06,                 /**< The device is running normally (with the capability to handle only classical CAN2.0 messages */
    BM_CAN_RESTRICTED_MODE = 0x07,              /**< Reserved */
    BM_CAN_NON_ISO_MODE = 0x08,                 /**< OR-Bitmask: The device is running with the capability to handle CAN and NON-ISO(Bosch) CANFD messages */
    BM_CAN_NON_AUTORETX_MODE = 0x10,            /**< OR-Bitmask: The device will not try to re-transmit failed messages if this bitmask is set */
    BM_CAN_NOACK_MODE = 0x20,                   /**< OR-Bitmask: ACK from remote ECU is not checked, always try to send message */
} BM_CanModeTypeDef;

/**
 * @enum  BM_LinModeTypeDef
 * @brief LIN mode IDs, used by BM_SetLinMode() to change the operation mode of LIN device.
 */
typedef enum
{
    BM_LIN_OFF_MODE = 0x00,                     /**< The device is logically disconnected from LIN bus */
    BM_LIN_SLAVE_MODE = 0x01,                   /**< The device is running as a slave node (with the capability to transmit and receive LIN messages */
    BM_LIN_INTERNAL_LOOPBACK_MODE = 0x02,       /**< The device is looping back messages internally without impacting the physical LIN bus */
    BM_LIN_LISTEN_ONLY_MODE = 0x03,             /**< The device is receiving messages without impacting the physical LIN bus */
    BM_LIN_MASTER_MODE = 0x08,                  /**< The device is running as a master node */
} BM_LinModeTypeDef;

/**
 * @enum  BM_EthModeTypeDef
 * @brief ETH mode IDs, used by BM_SetEthMode() to change the operation mode of ETH device.
 */
typedef enum
{
    BM_ETH_OFF_MODE = 0x00,                     /**< The device is logically disconnected from ETH network */
    BM_ETH_IN_MODE = 0x01,                      /**< The device can only accept incoming frames */
    BM_ETH_OUT_MODE = 0x02,                     /**< The device can only transmit outgoing frames */
    BM_ETH_INOUT_MODE = 0x03,                   /**< The device can both receive and transmit */
    BM_ETH_SWITCH_MODE = 0x04,                  /**< The device acts as a transparent Ethernet switch */
    BM_ETH_CAPTURE_MODE = 0x08,                 /**< The device captures the traffic on port (silently) */
} BM_EthModeTypeDef;

/**
 * @enum  BM_PtpModeTypeDef
 * @brief PTP timestamp synchronization mode values, used by BM_SetPtpMode() to change the synchronization mode for a given hardware channel.
 */
typedef enum
{
    BM_PTP_DISABLED = 0,                    /**< Disable PTP feature, work asynchronously using hardware local timestamp                 */
    BM_PTP_INPUT_USB_SOF = 1,               /**< Use host computer's USB SOF packet to sync with host computer                           */
    BM_PTP_INPUT_PPS = 2,                   /**< PTP slave: Use dedicated PPS input pin (if supported by hardware) as sync pulse input   */
    BM_PTP_OUTPUT_PPS = 4,                  /**< PTP master: Use dedicated PPS input pin (if supported by hardware) as sync pulse output */
} BM_PtpModeTypeDef;

/**
 * @enum  BM_TerminalResistorTypeDef
 * @brief Terminal resistor values, used by BM_SetTerminalResistor() to change the terminal/pull-up resistor of CAN/LIN device.
 */
typedef enum
{
    BM_TRESISTOR_AUTO = 0,              /**< Reserved, currently unsupported */
    BM_TRESISTOR_60 = 60,               /**< Currently unsupported */
    BM_TRESISTOR_120 = 120,             /**< 120Ohm */
    BM_TRESISTOR_PULLUP_1K = 1000,      /**< 1000Ohm pull-up resistor for LIN bus */
    BM_TRESISTOR_DISABLED = 0xFFFFU,    /**< Disable terminal resistor */
} BM_TerminalResistorTypeDef;

/**
 * @enum  BM_LinVoltageTypeDef
 * @brief LIN voltage configuration values, used by BM_SetLinVoltage().
 */
typedef enum
{
    BM_LIN_VOLTAGE_12V_IN = 0,          /**< Accept 12V as input */
    BM_LIN_VOLTAGE_12V_OUT,             /**< Use internal 12V DC-DC for pulling up */
} BM_LinVoltageTypeDef;

/**
 * @enum  BM_SleepStatusTypeDef
 * @brief Sleep status codes, used by BM_SetSleepStatus() to change the sleep status of a physical channel.
 */
typedef enum
{
    BM_WAKEUP = 0,                    /**< Channel is awake */
    BM_SLEEP = 1,                     /**< Channel is sleeping */
} BM_SleepStatusTypeDef;

/**
 * @enum  BM_LedTypeDef
 * @brief LED indicator status codes, used by BM_SetLed() to change the CAN LED indicator of CAN device.
 */
typedef enum
{
    BM_LED_OFF = 0,                     /**< CAN LED is OFF */
    BM_LED_ON = 1,                      /**< CAN LED is ON */
} BM_LedTypeDef;

/**
 * @enum  BM_MessageChannelTypeDef
 * @brief Message channel IDs in BM_DataTypeDef header, used for routing indication.
 * @note  You could also use integers directly, please note that valid channel IDs start from zero.
 */
typedef enum
{
    BM_MESSAGE_CHANNEL_0 = 0x0U,        /**< Channel 0 */
    BM_MESSAGE_CHANNEL_1 = 0x1U,        /**< Channel 1 */
    BM_MESSAGE_CHANNEL_2 = 0x2U,        /**< Channel 2 */
    BM_MESSAGE_CHANNEL_3 = 0x3U,        /**< Channel 3 */
    BM_MESSAGE_CHANNEL_4 = 0x4U,        /**< Channel 4 */
    BM_MESSAGE_CHANNEL_5 = 0x5U,        /**< Channel 5 */
    BM_MESSAGE_CHANNEL_6 = 0x6U,        /**< Channel 6 */
    BM_MESSAGE_CHANNEL_7 = 0x7U,        /**< Channel 7 */
    BM_MESSAGE_ANY_CHANNEL = 0xFU,      /**< Any channel, set this value in BM_DataTypeDef header if not used (e.g. TX.header.schn or RX.header.dchn) */
} BM_MessageChannelTypeDef;

/**
 * @enum  BM_CanMessageFlagsTypeDef
 * @brief CAN Message type flags, used in BM_TxTaskTypeDef.
 */
typedef enum
{
    BM_CAN_MESSAGE_FLAGS_NORMAL = 0,                        /**< Normal CAN message */
    BM_CAN_MESSAGE_FLAGS_IDE = 0x01,                        /**< Extended CAN message */
    BM_CAN_MESSAGE_FLAGS_RTR = 0x02,                        /**< Remote CAN message */
    BM_CAN_MESSAGE_FLAGS_BRS = 0x04,                        /**< CAN-FD bitrate switching is enabled */
    BM_CAN_MESSAGE_FLAGS_FDF = 0x08,                        /**< CAN-FD message */
    BM_CAN_MESSAGE_FLAGS_ESI = 0x10,                        /**< Reserved for gateways */

    BM_MESSAGE_FLAGS_NORMAL = BM_CAN_MESSAGE_FLAGS_NORMAL,  /**< For backward-compatibility only */
    BM_MESSAGE_FLAGS_IDE = BM_CAN_MESSAGE_FLAGS_IDE,        /**< For backward-compatibility only */
    BM_MESSAGE_FLAGS_RTR = BM_CAN_MESSAGE_FLAGS_RTR,        /**< For backward-compatibility only */
    BM_MESSAGE_FLAGS_BRS = BM_CAN_MESSAGE_FLAGS_BRS,        /**< For backward-compatibility only */
    BM_MESSAGE_FLAGS_FDF = BM_CAN_MESSAGE_FLAGS_FDF,        /**< For backward-compatibility only */
    BM_MESSAGE_FLAGS_ESI = BM_CAN_MESSAGE_FLAGS_ESI,        /**< For backward-compatibility only */
} BM_CanMessageFlagsTypeDef;

/**
 * @enum  BM_LinMessageFlagsTypeDef
 * @brief LIN Message type flags, used in BM_TxTaskTypeDef.
 */
typedef enum
{
    BM_LIN_MESSAGE_FLAGS_TRANSMIT = 0x01U,                      /**< Transfer direction: 0=receive, 1=transmit */
    BM_LIN_MESSAGE_FLAGS_WAKEUP = 0x02U,                        /**< This is a wakeup event */
    BM_LIN_MESSAGE_FLAGS_SLEEP = 0x04U,                         /**< This is a sleep event */
    BM_LIN_MESSAGE_FLAGS_ENHANCED_CHECKSUM = 0x08U,             /**< Flag for enhanced checksum */
    BM_LIN_MESSAGE_FLAGS_USER_CHECKSUM = 0x10U,                 /**< Flag for user defined checksum, use value from 'CHECKSUM' field if USER_CHECKSUM==1  */
} BM_LinMessageFlagsTypeDef;

/**
 * @enum  BM_MessageFlagsTypeDef
 * @brief This type is only defined for backward-compatibility, please consider using BM_CanMessageFlagsTypeDef instead.
 */
typedef BM_CanMessageFlagsTypeDef BM_MessageFlagsTypeDef;

/**
 * @enum  BM_RxFilterTypeTypeDef
 * @brief CAN RX filter type IDs, used in BM_RxFilterTypeDef.
 */
typedef enum
{
    BM_RXFILTER_INVALID = 0,            /**< Invalid (unused) RX filter entry */
    BM_RXFILTER_BASIC,                  /**< Basic RX filter, traditional acceptance filter based on message ID mask */
    BM_RXFILTER_ADVANCED,               /**< Busmust advanced RX filter, check both message ID and message payload */
    BM_RXFILTER_E2EPASS,                /**< Busmust E2E RX filter, accept only messages that passed E2E checking */
    BM_RXFILTER_E2EFAIL,                /**< Busmust E2E RX filter, accept only messages that failed E2E checking (for debugging purpose) */
} BM_RxFilterTypeTypeDef;

/**
 * @enum  BM_TxTaskTypeTypeDef
 * @brief CAN TX task type IDs, used in BM_TxTaskTypeDef.
 */
typedef enum
{
    BM_TXTASK_INVALID = 0,              /**< Invalid (unused) TX task entry */
    BM_TXTASK_FIXED,                    /**< Basic TX task, send fixed ID and fixed payload */
    BM_TXTASK_INCDATA,                  /**< Self-increment Data TX task */
    BM_TXTASK_INCID,                    /**< Self-increment ID TX task */
    BM_TXTASK_RANDOMDATA,               /**< Random Data TX task */
    BM_TXTASK_RANDOMID,                 /**< Random ID TX task */
} BM_TxTaskTypeTypeDef;

/**
 * @enum  BM_StatTypeDef
 * @brief CAN runtime statistics item IDs, used in BM_GetStat().
 */
typedef enum
{
    BM_STAT_NONE = 0U,                  /**< Invalid statistics item */
    BM_STAT_TX_MESSAGE,                 /**< Number of TX messages */
    BM_STAT_RX_MESSAGE,                 /**< Number of RX messages */
    BM_STAT_TX_BYTE,                    /**< Number of TX bytes */
    BM_STAT_RX_BYTE,                    /**< Number of RX bytes */
    BM_STAT_TX_ERROR,                   /**< Number of TX errors */
    BM_STAT_RX_ERROR,                   /**< Number of RX errors */
    BM_STAT_TOTAL_STORAGE_SIZE_KB,      /**< Offline storage total size (kB) */
    BM_STAT_FREE_STORAGE_SIZE_KB,       /**< Offline storage free size (kB) */
    BM_STAT_TXQ_BYTE,                   /**< Current used size of TXQ, in bytes */
    BM_STAT_RXQ_BYTE,                   /**< Current used size of RXQ, in bytes */
    BM_STAT_REPLAYQ_BYTE,               /**< Current used size of REPLAYQ, in bytes */
    BM_STAT_LOGGINGQ_BYTE,              /**< Current used size of LOGGINGQ, in bytes */
    BM_STAT_REPLAYFILE_BYTE,            /**< Current size of replay file, in bytes */
    BM_STAT_LOGGINGFILE_BYTE,           /**< Current size of logging file, in bytes */
    BM_STAT_TXQ_MAXSIZE_BYTE,           /**< Max acceptable size of TXQ, in bytes */
    BM_STAT_RXQ_MAXSIZE_BYTE,           /**< Max acceptable size of RXQ, in bytes */
    BM_STAT_REPLAYQ_MAXSIZE_BYTE,       /**< Max acceptable size of REPLAYQ, in bytes */
    BM_STAT_LOGGINGQ_MAXSIZE_BYTE,      /**< Max acceptable size of LOGGINGQ, in bytes */
    BM_STAT_REPLAYFILE_MAXSIZE_BYTE,    /**< Max acceptable size of replay file, in bytes */
    BM_STAT_LOGGINGFILE_MAXSIZE_BYTE,   /**< Max acceptable size of logging file, in bytes */
    BM_STAT_CAP = 0x40U,
    BM_STAT_MAX_RXFILTER,               /**< Max allowed RX filter count */
    BM_STAT_MAX_TXTASK,                 /**< Max allowed TX Task count */
    BM_STAT_MAX_MESSAGE_INFO,           /**< Max allowed Router message count */
    BM_STAT_MAX_SIGNAL_INFO,            /**< Max allowed Router signal count */
    BM_STAT_MAX_E2E_INFO,               /**< Max allowed Router E2E count */
    BM_STAT_MAX_ROUTE,                  /**< Max allowed Router route count  */
    BM_STAT_SUPPORT_OFFLINE = 0x60U,    /**< This device supports offline storage of configuration info */
    BM_STAT_SUPPORT_ROUTE,              /**< This device supports message route */
    BM_STAT_SUPPORT_LOGGING,            /**< This device supports logging CAN traffic into offline storage */
    BM_STAT_SUPPORT_REPLAY,             /**< This device supports replaying CAN traffic from offline storage */
    BM_SUPPORT_BUFFER_API,
    BM_SUPPORT_FATFS_API,
} BM_StatTypeDef;

/**
 * @enum  BM_IsotpModeTypeDef
 * @brief ISOTP operation mode, used in BM_IsotpConfigTypeDef.
 */
typedef enum
{
    BM_ISOTP_NORMAL_TESTER = 0,     /**< Default mode: normal (non-extended-addressing) UDS client(tester) */
    BM_ISOTP_NORMAL_ECU,            /**< normal (non-extended-addressing) UDS server(ECU)                  */
    BM_ISOTP_EXTENDED_TESTER,       /**< Currently unsupported: extended-addressing UDS client(tester)     */
    BM_ISOTP_EXTENDED_ECU,          /**< Currently unsupported: extended-addressing UDS server(ECU)        */
} BM_IsotpModeTypeDef;

/**
 * @enum  BM_LinProtocolVersionTypeDef
 * @brief LIN protocol version.
 */
typedef enum
{
    BM_LIN_VER_1_3 = 0x13,          /**< Version1.3 */
    BM_LIN_VER_2_0 = 0x20,          /**< Version2.0 */
    BM_LIN_VER_2_1 = 0x21,          /**< Version2.1 */
    BM_LIN_VER_2_2 = 0x22,          /**< Version2.2 */
} BM_LinProtocolVersionTypeDef;

/**
 * @enum  BM_LinProtocolChecksumTypeDef
 * @brief LIN protocol checksum options.
 */
typedef enum
{
    BM_LIN_MANUAL_CHECKSUM = 0,            /**< Reserved */
    BM_LIN_NORMAL_CHECKSUM = 1,            /**< Normal checksum (usually used by LIN1.3) */
    BM_LIN_ENHANCED_CHECKSUM = 2,          /**< Enhanced checksum (can only be used by LIN2.x) */
} BM_LinProtocolChecksumTypeDef;

/**
 * @enum  BM_LinErrorTypeDef
 * @brief LIN error code, used in BM_LinStatusInfoTypedef.
 */
typedef enum
{
    BM_LIN_NO_ERROR = 0x00,             /**< No error detected */
    BM_LIN_BIT_ERROR = 0x01,            /**< Bit error */
    BM_LIN_CHECKSUM_ERROR = 0x02,       /**< Checksum error */
    BM_LIN_PARITY_ERROR = 0x04,         /**< Parity error */
    BM_LIN_BREAK_ERROR = 0x08,          /**< Break error */
    BM_LIN_BUS_IDLE_TIMEOUT = 0x10,     /**< Bus idle timeout */
    BM_LIN_TRANSMIT_TIMEOUT = 0x20,     /**< Transmit timeout */
} BM_LinErrorTypeDef;

/**
 * @enum  BM_StorageModeTypeDef
 * @brief Logging or replay mode, used in BM_LoggingConfigTypeDef and BM_ReplayConfigTypeDef.
 */
typedef enum
{
    BM_STORAGE_DISABLED = 0,
    BM_STORAGE_ALWAYS_ON,
    BM_STORAGE_TRIGGERED,
} BM_StorageModeTypeDef;

/**
 * @enum  BM_StorageDirectionTypeDef
 * @brief Logging or replay mode, used in BM_LoggingConfigTypeDef and BM_ReplayConfigTypeDef.
 */
typedef enum
{
    BM_STORAGE_DIRECTION_NONE = 0,
    BM_STORAGE_DIRECTION_RX,
    BM_STORAGE_DIRECTION_TX,
    BM_STORAGE_DIRECTION_ALL = BM_STORAGE_DIRECTION_RX | BM_STORAGE_DIRECTION_TX,
} BM_StorageDirectionTypeDef;

/**
 * @enum  BM_StorageFormatTypeDef
 * @brief Logging or replay mode, used in BM_LoggingConfigTypeDef and BM_ReplayConfigTypeDef.
 */
typedef enum
{
    BM_STORAGE_DEFAULT_FORMAT = 0,
    BM_STORAGE_BBD_FORMAT = BM_STORAGE_DEFAULT_FORMAT,
    BM_STORAGE_PCAP_FORMAT = 1,
    BM_STORAGE_LOG_FORMAT = 2,
    BM_STORAGE_ASC_FORMAT = 3,
    BM_STORAGE_BLF_FORMAT = 4,
} BM_StorageFormatTypeDef;

/**
 * @enum  BM_StoragePathModeTypeDef
 * @brief Logging or replay mode, used in BM_LoggingConfigTypeDef and BM_ReplayConfigTypeDef.
 */
typedef enum
{
    BM_STORAGE_FIXED_PATH = 0,
    BM_STORAGE_INDEX_PATH,
    BM_STORAGE_TIME_PATH
} BM_StoragePathModeTypeDef;

/**
 * @enum  BM_FileAttributeTypeDef
 * @brief Offline storage file attributes.
 */
typedef enum
{
    BM_FILE_ATTRIBUTE_READONLY = 0x00000001U,
    BM_FILE_ATTRIBUTE_HIDDEN = 0x00000002U,
    BM_FILE_ATTRIBUTE_SYSTEM = 0x00000004U,
    BM_FILE_ATTRIBUTE_DIRECTORY = 0x00000010U,
    BM_FILE_ATTRIBUTE_ARCHIVE = 0x00000020U,
    BM_FILE_ATTRIBUTE_DEVICE = 0x00000040U,
    BM_FILE_ATTRIBUTE_NORMAL = 0x00000080U,
    BM_FILE_ATTRIBUTE_TEMPORARY = 0x00000100U,
    BM_FILE_ATTRIBUTE_COMPRESSED = 0x00000800U,
    BM_FILE_ATTRIBUTE_OFFLINE = 0x00001000U,
    BM_FILE_ATTRIBUTE_ENCRYPTED = 0x00004000U,
    BM_FILE_ATTRIBUTE_VIRTUAL = 0x00010000U,
} BM_FileAttributeTypeDef;

/**
 * @typedef BM_DataHeaderTypeDef
 * @brief   Busmust data header, each BM_DataTypeDef contains a header which indicates payload information.
 */
typedef struct
{
    uint16_t type : 4;                  /**< Data type, see BM_DataTypeTypeDef for details. */
    uint16_t flags : 1;                 /**< If this packet has an additional tail or not, packet.length = payload.length + tail.length if flags=1. */
    uint16_t group : 3;                 /**< Channel group index, starting from zero, each group contains 16 channels */
                                        /**< e.g.Use[group = 0, schn = 7] to access the 7th port(in BM_ChannelInfoTypeDef) of a device */
                                        /**< e.g.Use[group = 2, schn = 3] to access the 35th port(in BM_ChannelInfoTypeDef) of a device */
    uint16_t dchn : 4;                  /**< Destination channel ID, starting from zero, used by TX data to indicate the hardware about the target port. */
    uint16_t schn : 4;                  /**< Source channel ID, starting from zero, used by RX data to indicate the application about the source port. */
} BM_DataHeaderTypeDef;

/**
 * @typedef BM_DataTailTypeDef
 * @brief   Busmust data tail, each BM_DataTypeDef contains an optional (if header.hastail) tail which indicates packet side-band information, i.e. 64-bit UTC timestamp.
 */
typedef struct
{
    uint8_t type;                  /**< Tail type, reserved for future, default as zero. */
    uint8_t reserved[3];
    uint16_t packetid;             /**< Packet id, this is usually a sequential counter which can be used to detect packet loss */
    uint16_t checksum;             /**< Checksum of the whole packet, including header and tail, 0 if invalid or disabled. */
    uint32_t utctsl;               /**< Low part of 64-bit UTC high precision timestamp in microseconds, since 1970-1-1. */
    uint32_t utctsh;               /**< High part of 64-bit UTC high precision timestamp in microseconds, since 1970-1-1. */
} BM_DataTailTypeDef;

/**
 * @def   BM_DATA_HEADER
 * @brief Helper macro to define a BM data header value
 */
#define BM_DATA_HEADER(type, flags, dchn, schn) (((type) & 0x0FU) | (((flags) << 4U) & 0x10U) | (((dchn) << 8U) & 0x0F00U) | (((schn) << 12U) & 0xF000U))
#define BM_DATA_HEADER_EX(type, flags, dchn, schn, group) (((type) & 0x0FU) | (((flags) << 4U) & 0x10U) | (((dchn) << 8U) & 0x0F00U) | (((schn) << 12U) & 0xF000U) | (((group) << 5U) & 0xE0U))
#define BM_DATA_SCHN(header) (((uint16_t)((header).group)) * 16U + ((uint16_t)((header).schn)))
#define BM_DATA_DCHN(header) (((uint16_t)((header).group)) * 16U + ((uint16_t)((header).dchn)))

/**
 * @typedef BM_DataTypeDef
 * @brief   Busmust data, abstract structure which holds concrete payload messages of various types (i.e. CAN messages).
 */
typedef struct
{
    BM_DataHeaderTypeDef header;                /**< data header, see BM_DataHeaderTypeDef for details. */
    uint16_t length;                            /**< length in bytes of the payload byte array (header excluded) */
    uint32_t timestamp;                         /**< 32-bit device local high precision timestamp in microseconds. */
    uint8_t payload[BM_DATA_PAYLOAD_MAX_SIZE];  /**< buffer holding concrete message payload (i.e. a CAN message in BM_CanMessageTypeDef format), followed by an optional tail. */
} BM_DataTypeDef;

/**
 * @def     BM_INIT_CAN_FD_MESSAGE_DATA
 * @brief   Helper macro to initialize a BM_DataTypeDef object using can message information.
 */
#define BM_INIT_CAN_FD_DATA(_data, _id, _dlc, _ide, _fdf, _brs, _rtr, _esi, _payload) { \
    BM_CanMessageTypeDef* can = (BM_CanMessageTypeDef*)(_data).payload; \
    (_data).header.type = BM_CAN_FD_DATA; \
    (_data).header.group = 0x7U; \
    (_data).header.schn = 0xFU; \
    (_data).header.dchn = 0xFU; \
    (_data).header.flags = 0x0U; \
    (_data).length = sizeof(BM_CanMessageTypeDef); \
    BM_INIT_CAN_MSG(*can, (_id), (_dlc), (_ide), (_fdf), (_brs), (_rtr), (_esi), (_payload)); \
}

/**
 * @typedef BM_MessageIdTypeDef
 * @brief   Busmust CAN Message ID.
 * @note    You could also use a uint32_t, but please take care of memory alignments.
 */
typedef struct {
    uint32_t SID : 11;                          /**< Standard ID */
    uint32_t EID : 18;                          /**< Extended ID */
    uint32_t SID11 : 1;                         /**< Reserved */
    uint32_t unimplemented1 : 2;                /**< Reserved */
} BM_MessageIdTypeDef;

/**
 * @def   BM_SET_STD_MSG_ID
 * @brief Helper macro to initialize a BM_MessageIdTypeDef object using a 11-bit CAN standard message ID.
 * @note  Example Usage: 
 *        BM_MessageIdTypeDef id;
 *        BM_SET_STD_MSG_ID(id, 0x7DF);
 */
#define BM_SET_STD_MSG_ID(_struct, _id11) { \
    (_struct).SID = (((uint32_t)(_id11))) & 0x7FFU; \
    (_struct).EID = 0U; \
    (_struct).SID11 = 0U; \
    (_struct).unimplemented1 = 0U; \
}

 /**
  * @def   BM_SET_EXT_MSG_ID
  * @brief Helper macro to initialize a BM_MessageIdTypeDef object using a 29-bit CAN extended message ID.
  * @note  Example Usage:
  *        BM_MessageIdTypeDef id;
  *        BM_SET_STD_MSG_ID(id, 0x18FFAAA0);
  */
#define BM_SET_EXT_MSG_ID(_struct, _id29) { \
    (_struct).SID = (((uint32_t)(_id29)) >> 18) & 0x7FFU; \
    (_struct).EID = (((uint32_t)(_id29)) & 0x3FFFFU); \
    (_struct).SID11 = 0U; \
    (_struct).unimplemented1 = 0U; \
}

/**
 * @def   BM_GET_STD_MSG_ID
 * @brief Helper macro to get a 11-bit CAN standard message ID from a BM_MessageIdTypeDef object.
 * @note  Example Usage: 
 *        BM_MessageIdTypeDef id;
 *        // Query the message using BM_ReadCanMessage()
 *        uint32_t id11 = BM_GET_STD_MSG_ID(id);
 */
#define BM_GET_STD_MSG_ID(_struct) (((_struct).SID))

 /**
  * @def   BM_GET_STD_MSG_ID
  * @brief Helper macro to get a 29-bit CAN extended message ID from a BM_MessageIdTypeDef object.
  * @note  Example Usage:
  *        BM_MessageIdTypeDef id;
  *        // Query the message using BM_ReadCanMessage()
  *        uint32_t id29 = BM_GET_EXT_MSG_ID(id);
  */
#define BM_GET_EXT_MSG_ID(_struct) (((_struct).SID << 18) | (_struct).EID)

/**
 * @typedef BM_TxMessageCtrlTypeDef
 * @brief   Busmust TX CAN Message control fields.
 * @note    The first a few fields (until FDF) are bit compatible with BM_RxMessageCtrlTypeDef.
 */
typedef struct {
    uint32_t DLC : 4;                           /**< CAN message DLC(0-F), note this is not the message length */
    uint32_t IDE : 1;                           /**< This message is an extended CAN message */
    uint32_t RTR : 1;                           /**< This message is a remote CAN message */
    uint32_t BRS : 1;                           /**< This message requires CAN-FD bitrate switching */
    uint32_t FDF : 1;                           /**< This message is a CAN-FD CAN message */
    uint32_t ESI : 1;                           /**< Reserved for gateways */
    uint32_t SEQ : 8;                          /**< hardware-sync message ID, the ACK message's SEQ is always equal to the TX message's SEQ */
    uint32_t unimplemented1 : 15;               /**< Reserved */
} BM_TxMessageCtrlTypeDef;

/**
 * @typedef BM_RxMessageCtrlTypeDef
 * @brief   Busmust RX CAN Message control fields.
 * @note    The first a few fields (until FDF) are bit compatible with BM_TxMessageCtrlTypeDef.
 */
typedef struct {
    uint32_t DLC : 4;                           /**< CAN message DLC(0-F), note this is not the message length */
    uint32_t IDE : 1;                           /**< This message is an extended CAN message */
    uint32_t RTR : 1;                           /**< This message is a remote CAN message */
    uint32_t BRS : 1;                           /**< This message requires CAN-FD bitrate switching */
    uint32_t FDF : 1;                           /**< This message is a CAN-FD CAN message */
    uint32_t ESI : 1;                           /**< Reserved for gateways */
    uint32_t unimplemented1 : 2;                /**< Reserved */
    uint32_t FilterHit : 5;                     /**< By wich RX filter the message is accepted */
    uint32_t unimplemented2 : 16;               /**< Reserved */
} BM_RxMessageCtrlTypeDef;

/**
 * @typedef BM_CanMessageTypeDef
 * @brief   Busmust CAN Message concrete type, usually used as payload of BM_DataTypeDef.
 * @note    The total length of this structure is 72B, it support both classic and FD CAN messages.
 */
typedef struct 
{
    BM_MessageIdTypeDef id;                     /**< CAN message ID, see BM_MessageIdTypeDef for details. */
    union
    {
        BM_TxMessageCtrlTypeDef tx;             /**< TX CAN message control fields, invalid if this is NOT a TX can message. */
        BM_RxMessageCtrlTypeDef rx;             /**< RX CAN message control fields, invalid if this is NOT a RX can message. */
    } ctrl;                                     /**< CAN message control fields, whether TX or RX is taken depends on the message direction. */
    uint8_t payload[64];                        /**< CAN message payload */
} BM_CanMessageTypeDef;

/**
 * @def   BM_INIT_CAN_MESSAGE
 * @brief Helper macro to initialize a BM_CanMessageTypeDef object.
 */
#define BM_INIT_CAN_MSG(_msg, _id, _dlc, _ide, _fdf, _brs, _rtr, _esi, _payload) { \
    (_msg).ctrl.tx.DLC = (_dlc); \
    (_msg).ctrl.tx.IDE = (_ide); \
    (_msg).ctrl.tx.FDF = (_fdf); \
    (_msg).ctrl.tx.BRS = (_brs); \
    (_msg).ctrl.tx.RTR = (_rtr); \
    (_msg).ctrl.tx.ESI = (_esi); \
    BM_SET_CAN_MSG_ID((_msg), (_id)); \
    if ((_payload) != NULL) { uint8_t i; for (i = 0; i < sizeof((_msg).payload); i++) { (_msg).payload[i] = ((uint8_t*)(_payload))[i]; } } \
}

/**
 * @def   BM_SET_CAN_MSG_ID
 * @brief Helper macro to set(update after initialization) a BM_CanMessageTypeDef object's 11-bit/29-bit ID.
 */
#define BM_SET_CAN_MSG_ID(_msg, _id) { \
    if ((_msg).ctrl.tx.IDE) { BM_SET_EXT_MSG_ID((_msg).id, (_id)); } else { BM_SET_STD_MSG_ID((_msg).id, (_id)); } \
}

/**
 * @def   BM_GET_CAN_MSG_ID
 * @brief Helper macro to get(read after initialization) a BM_CanMessageTypeDef object's 11-bit/29-bit ID.
 */
#define BM_GET_CAN_MSG_ID(_msg) (((_msg).ctrl.tx.IDE) ? BM_GET_EXT_MSG_ID((_msg).id, (_id)) : BM_GET_STD_MSG_ID((_msg).id, (_id)))

/**
 * @typedef BM_LinMessageCtrlTypeDef
 * @brief   Busmust LIN Message control fields.
 */
typedef struct {
    uint32_t DLC : 4;                           /**< LIN message DLC(0-8), if DLC=0b1111, data length is given by ID[5:4]: 0:2, 1:2, 2:4, 3:8 */
    uint32_t TRANSMIT : 1;                      /**< Transfer direction: 0=receive, 1=transmit */
    uint32_t WAKEUP : 1;                        /**< This is a wakeup event */
    uint32_t SLEEP : 1;                         /**< This is a sleep event */
    uint32_t ENHANCED_CHECKSUM : 1;             /**< Flag for enhanced checksum */
    uint32_t USER_CHECKSUM : 1;                 /**< Flag for user defined checksum, use value from 'CHECKSUM' field if USER_CHECKSUM==1  */
    uint32_t SEQ : 7;                           /**< hardware-sync message ID, the ACK message's SEQ is always equal to the TX message's SEQ or TXTASK index */
    uint32_t ISTXTASK : 1;                      /**< A indicator for TXTASK TEF event, SEQ=TXTASK index if ISTXTASK=1 */
    uint32_t ERRORS : 6;                        /**< Error code, which is a bitmask, see BM_LinErrorTypeDef for details. */
    uint32_t ID_PARITY : 1;                     /**< Parity bit */
    uint32_t CHECKSUM : 8;                      /**< Checksum value */
} BM_LinMessageCtrlTypeDef;


/**
 * @typedef BM_LinMessageTypeDef
 * @brief   Busmust CAN Message concrete type, usually used as payload of BM_DataTypeDef.
 * @note    The total length of this structure is 16B.
 */
typedef struct 
{
    uint8_t id;                                /**< LIN message ID */
    uint8_t padding[3];
    union
    {
        BM_LinMessageCtrlTypeDef lin;          /**< LIN message control fields, invalid if this is NOT a LIN message. */
    } ctrl;                                    /**< Message control fields. */
    uint8_t payload[8];                        /**< LIN message payload */
} BM_LinMessageTypeDef;

/**
 * @def   BM_INIT_LIN_MSG
 * @brief Helper macro to initialize a BM_LinMessageTypeDef object.
 */
#define BM_INIT_LIN_MSG(_msg, _id, _dlc, _payload) { \
    (_msg).ctrl.lin.DLC = (_dlc); \
    (_msg).id = (_id); \
    if ((_payload) != NULL) { uint8_t i; for (i = 0; i < sizeof((_msg).payload); i++) { (_msg).payload[i] = ((uint8_t*)(_payload))[i]; } } \
}

/**
 * @typedef BM_EthMessageTypeDef
 * @brief   Busmust ETH Message concrete type, usually used as payload of BM_DataTypeDef.
 */
typedef struct 
{
    uint8_t dstmac[6];                                /**< Destination MAC address */
    uint8_t srcmac[6];                                /**< Source MAC address */
    uint16_t type;                                    /**< Message Type */
    uint8_t payload[BM_JUMBO_DATA_PAYLOAD_MAX_SIZE];  /**< ETH message payload */
} BM_EthMessageTypeDef;


/**
 * @typedef BM_ChannelInfoTypeDef
 * @brief   Channel information, created when enumerating devices by BM_Enumerate() and used when opening device by BM_OpenEx().
 */
typedef struct
{
    char name[64];                              /**< Device full name, for display purpose */
    uint8_t  sn[16];                            /**< Device SN */
    uint8_t  uid[12];                           /**< Device UID */
    uint8_t  version[4];                        /**< Device Firmware Version */
    uint16_t vid;                               /**< Device VID */
    uint16_t pid;                               /**< Device PID */
    uint16_t port;                              /**< Port ID (0-7) of the device, note a multi-port device is enumerated as multiple dedicated BM_ChannelInfoTypeDef entries */
    uint16_t cap;                               /**< Device Capability flags, see BM_CapabilityTypeDef for details. */
    uint8_t  addr[4];                           /**< Device address, this is usually a IPV4 address and can be converted to in_addr struct pointer (address in network byte order) */
} BM_ChannelInfoTypeDef;

/**
 * @def   BM_VERSION_CODE
 * @brief Helper macro to format a 32-bit version code from channelinfo.version, which is an 8-bit array.
 */
#define BM_VERSION_CODE(version) (\
    (((uint32_t)((version)[0])) << 24) |\
    (((uint32_t)((version)[1])) << 16) |\
    (((uint32_t)((version)[2])) <<  8) |\
    (((uint32_t)((version)[3])) <<  0) )

/**
 * @typedef BM_CanStatusInfoTypedef
 * @brief   CAN channel status detailed information, retrieved by calling BM_GetCanStatus() or BM_GetStatus(), see ISO11898 for details.
 */
typedef struct
{
    uint8_t TXBO;                               /**< The CAN channel is in BUSOFF state */
    uint8_t reserved[1];                        /**< Reserved */
    uint8_t TXBP;                               /**< The CAN channel is in TX bus passive state */
    uint8_t RXBP;                               /**< The CAN channel is in RX bus passive state */
    uint8_t TXWARN;                             /**< The CAN channel is in TX warn state */
    uint8_t RXWARN;                             /**< The CAN channel is in RX warn state */
    uint8_t TEC;                                /**< TX Bus Error counter */
    uint8_t REC;                                /**< RX Bus Error counter */
} BM_CanStatusInfoTypeDef;
typedef BM_CanStatusInfoTypeDef BM_CanStatusInfoTypedef; /**< Keep compatible with BMAPI1.x */

/**
 * @typedef BM_LinStatusInfoTypedef
 * @brief   LIN channel status detailed information, retrieved by calling BM_GetLinStatus() or BM_GetStatus().
 */
typedef struct
{
    uint8_t ACTIVE;                               /**< The LIN channel is in ACTIVE state */
    uint8_t SLEEP;                                /**< The LIN channel is in SLEEP state */
    uint8_t ERRORS;                               /**< Error status bitmask for the LIN channel, see BM_LinErrorTypeDef for details */
    uint8_t reserved[5];                          /**< Reserved */
} BM_LinStatusInfoTypeDef;

/**
 * @typedef BM_StatusInfoHandle
 * @brief   Abstract type for generic channel info, Keep compatible with BMAPI1.x, used by BM_GetStatus().
 */
typedef void* BM_StatusInfoHandle;

/**
 * @typedef BM_BitrateTypeDef
 * @brief   CAN channel bitrate configuration, used by BM_SetBitrate().
 */
typedef struct
{
    uint16_t nbitrate;                          /**< Nominal bitrate, note this is the only valid birate for CAN CLASSIC, LIN and eth channels. 
                                                     bitrate is in kbps For CAN/CANFD channels, bps for LIN channels and Mbps for ETH channels. */
    uint16_t dbitrate;                          /**< Data bitrate in kbps, default as 500, note this is only valid in CANFD mode. */
    uint8_t nsamplepos;                         /**< Nominal sample position (percentage), 0-100, default as 75 */
    uint8_t dsamplepos;                         /**< Data sample position (percentage), 0-100, default as 75 */
    /* Setting any of the fields below would override the nbitrate configuration */
    uint8_t  clockfreq;                         /**< CAN controller clock in Mhz, default as 0 */
    uint8_t  reserved;                          /**< Reserved */
    uint8_t  nbtr0;                             /**< Nominal BTR0 register value, note this value is calculated using clockfreq, which might not be 16MHz */
    uint8_t  nbtr1;                             /**< Nominal BTR1 register value, note this value is calculated using clockfreq, which might not be 16MHz */
    uint8_t  dbtr0;                             /**< Data BTR0 register value, note this value is calculated using clockfreq, which might not be 16MHz */
    uint8_t  dbtr1;                             /**< Data BTR1 register value, note this value is calculated using clockfreq, which might not be 16MHz */
} BM_BitrateTypeDef;

/**
 * @typedef BM_RxFilterTypeDef
 * @brief   CAN channel RX filter item structure, used by BM_SetRxFilter().
 * @note    The filter support masking ID, flags and payload according to its type, 
 *          in order for a message to be accepted, all the fields are masked using AND logic:
 *          (flags & filter.flags_mask == filter.flags_value) AND (ID & filter.id_mask == filter.id_value) AND (payload & filter.payload_mask == filter.payload_value)
 */
typedef struct
{
    uint8_t  type;                              /**< Type ID of the RX filter, see BM_RxFilterTypeTypeDef for details. */
    uint8_t  unused;                            /**< Reserved */
    uint8_t  flags_mask;                        /**< CAN message control Flags masks, see BM_MessageFlagsTypeDef for details. */
    uint8_t  flags_value;                       /**< CAN message control Flags values, see BM_MessageFlagsTypeDef for details. */
    uint8_t  reserved[4];                       /**< Reserved */
    uint32_t id_mask;                           /**< CAN message ID masks, see BM_MessageIdTypeDef for details. */
    uint32_t id_value;                          /**< CAN message ID values, see BM_MessageIdTypeDef for details. */
    uint8_t payload_mask[8];                    /**< CAN message payload masks, for CAN-FD messages, only the first 8 bytes are checked. */
    uint8_t payload_value[8];                   /**< CAN message payload values, for CAN-FD messages, only the first 8 bytes are checked. */
} BM_RxFilterTypeDef;

/**
 * @typedef BM_TxTaskTypeDef
 * @brief   CAN channel TX task item structure, used by BM_SetTxTask().
 * @note    Once the CAN device is armed with TX tasks, it will try to parse the TX task and send CAN messages automatically.
 *          The difference with a software triggered CAN message in BusMaster is that 
 *          hardware triggered CAN messages are more precise in time and could reach a higher throughput.
 */
typedef struct
{
    uint8_t  type;                              /**< Type ID of the TX task, see BM_TxTaskTypeTypeDef for details. */
    union
    {
        uint8_t version;                        /**< Version of BM_TxTaskTypeDef, set to 1 for BMAPI1.x. */
        uint8_t unused;                         /**< For backward-compatibility only */
    };
    uint8_t  flags;                             /**< CAN message control Flags, see BM_MessageFlagsTypeDef for details. */
    struct
    {
        uint8_t length : 7;                     /**< Length of payload in unit given by 'lengthunit' (not DLC) */
        uint8_t lengthunit : 1;                 /**< Unit of length, 0=1B, 1=128B, default as 0, that is, length in bytes */
                                                /**< Note that not payload buffer might not be 100% used, 
                                                     user would need to check specific data header in payload for further information, 
                                                     i.e. ((BM_CanfdDataTypeDef*)txtask.payload)->ctrl.tx.DLC */
    };
    uint16_t delay;                             /**< Delay within tx cycle, that is, offset of tx timing slot within tx cycle (given by 'cycle' field)*/
                                                /**< i.e. If cycle=50 and delay=10, this txtask will be executed at 10, 60, 110, 160, etc. */
    uint16_t cycle;                             /**< ms delay between rounds */
    uint16_t nrounds;                           /**< num of cycles, nrounds=0xFFFFU indicates INFINITE */
    uint16_t nmessages;                         /**< messages per round, default as 1 message/cycle */

    union
    {
        uint32_t id;                            /**< Generic ID field, normally you would need to set specific ID structure instead (e.g. CAN.SID). */
        struct
        {
            uint32_t SID : 11;                  /**< CAN Standard ID */
            uint32_t EID : 18;                  /**< CAN Extended ID */
            uint32_t SID11 : 1;                 /**< Reserved */
            uint32_t unimplemented1 : 2;        /**< Reserved */
        } can;
        struct
        {
            uint8_t ID;                         /**< LIN message id, 0~63 */
            uint8_t CHECKSUM;                   /**< LIN manual checksum value, only valid if (flags & BM_LIN_MESSAGE_FLAGS_USER_CHECKSUM) != 0 */
            uint16_t reserved;
        } lin;
    };

    union
    {
        /* The child fields are invalid if the TX task type is NOT BM_TXTASK_INCDATA */
        struct {
            uint16_t startbit;                  /**< Start bit of data increment, currently only 8-bit aligned value is accepted */
            uint8_t nbits;                      /**< Number of bits of data increment, currently only 32 is accepted */
            uint8_t format;                     /**< 0x80=Intel, 0x00=Motorola */
            uint32_t min;                       /**< Minimum value of the Increment range */
            uint32_t max;                       /**< Maximum value of the Increment range */
            uint32_t step;                      /**< Step of the Increment range */
        } incdata;

        /* The child fields are invalid if the TX task type is NOT BM_TXTASK_INCID */
        struct {
            uint32_t min;                       /**< Minimum value of the Increment range */
            uint32_t max;                       /**< Maximum value of the Increment range */
            uint32_t step;                      /**< Step of the Increment range */
        } incid;

        /* The child fields are invalid if the TX task type is NOT BM_TXTASK_RANDOMDATA */
        struct {
            uint16_t startbit;                  /**< Start bit of random data, currently only 8-bit aligned value is accepted */
            uint8_t nbits;                      /**< Number of bits of random data, currently only 32 is accepted */
            uint8_t format;                     /**< 0x80=Intel, 0x00=Motorola */
            uint32_t min;                       /**< Minimum value of the Increment range */
            uint32_t max;                       /**< Maximum value of the Increment range */
            uint32_t seed;                      /**< Random seed */
        } randomdata;

        /* The child fields are invalid if the TX task type is NOT BM_TXTASK_RANDOMID */
        struct {
            uint32_t min;                       /**< Minimum value of the random range */
            uint32_t max;                       /**< Maximum value of the random range */
            uint32_t seed;                      /**< Random seed */
        } randomid;

        uint8_t unused[32];                     /**< Reserved */
    } pattern;                                  /**< Changing pattern of a Volatile TX task */

    uint8_t e2e;                                /**< Index of E2E (in E2E table), currently unsupported */
    uint8_t e2e_reserved[15];                   /**< Reserved for future (e.g. E2E information) */

    uint8_t  payload[64];                       /**< Default payload data, note this is also the template payload of the unchanged part in a volatile TX task */
} BM_TxTaskTypeDef;

typedef struct
{
	uint8_t type;           /**< 0 = invalid, 1 = unicast, 2 = broadcast                                                  */
	uint8_t source;         /**< Source channel index (0-15)                                                              */
	uint16_t target;        /**< Target channel: index if type==1, bitmask of target channels if type==2                  */
    uint16_t reserved;      /**< Reserved for future                                                                      */
    uint8_t flagsmask;      /**< Source message flag mask, message will be routed if msg.flags & flagsmask == flagsvalue  */
	uint8_t flagsvalue;     /**< Source message flag value, See BM_MessageFlagsTypeDef for details                        */
    uint32_t idmask;        /**< Source message ID mask, message will be routed if msg.id & idmask == idvalue             */
	uint32_t idvalue;       /**< Source message ID value, See BM_MessageIdTypeDef for details                             */
} BM_MessageRouteTypeDef;

/**
 * @typedef BM_IsotpStatusTypeDef
 * @brief   ISOTP status report, used by ISOTP operation callback function.
 */
typedef struct  
{
    uint8_t  version;                           /**< Currently always 0x01 */
    uint8_t  flowcontrol;                       /**< Current flow control status, 0=continue, 1=wait, 2=overflow, ff=timeout, */
    uint8_t  stmin;                             /**< Current flow control status, i.e. 30 00 00 */
    uint8_t  blocksize;                         /**< Current flow control status, i.e. 30 00 00 */
    uint32_t ntransferredbytes;                 /**< Number of transferred bytes by now. */
    uint32_t ntotalbytes;                       /**< Number of total bytes indicated by ISOTP FF or SF. */
    uint32_t timestamp;                         /**< Current timestamp reported by device. */
    uint32_t reserved[4];                       /**< Reserved for future */
} BM_IsotpStatusTypeDef;

/**
 * @typedef BM_IsotpCallbackHandle
 * @brief   Pointer to a Callback function when ISOTP transaction progress updates, normally this would occur at least once per FC frame.
 * @param[in] status              Current ISOTP status reported by ISOTP context.
 * @param[in] userarg             Arbitrary user argument passed by BM_IsotpConfigTypeDef.
 * @return                        Currently not used, please return 0 for further compatibility.
 */
typedef uint8_t (*BM_IsotpCallbackHandle)(const BM_IsotpStatusTypeDef* status, uintptr_t userarg);

/**
 * @typedef BM_IsotpConfigTypeDef
 * @brief   ISOTP Protocol (See ISO15765-2 for details) configuration, used by BM_ConfigIsotp().
 */
typedef struct 
{
    uint8_t version;                            /**< Currently must be set to 0x01                                                            */
    uint8_t mode;                               /**< See BM_IsotpModeTypeDef for details, Default mode is normal (non-extended-addressing) UDS client(tester) */
    struct  
    {
        uint16_t a;                             /**< A timeout in milliseconds: =N_As if writing as tester or reading as ECU, otherwise =N_Ar */
        uint16_t b;                             /**< B timeout in milliseconds: =N_Bs if writing as tester or reading as ECU, otherwise =N_Br */
        uint16_t c;                             /**< C timeout in milliseconds: =N_Cs if writing as tester or reading as ECU, otherwise =N_Cr */
    } testerTimeout;
    struct
    {
        uint16_t a;                             /**< A timeout in milliseconds: =N_Ar if writing as tester or reading as ECU, otherwise =N_As */
        uint16_t b;                             /**< B timeout in milliseconds: =N_Br if writing as tester or reading as ECU, otherwise =N_Bs */
        uint16_t c;                             /**< C timeout in milliseconds: =N_Cr if writing as tester or reading as ECU, otherwise =N_Cs */
    } ecuTimeout;
    struct  
    {
        uint8_t stmin;                          /**< STmin raw value (0x00-0x7F or 0xF1-0xF9) if Busmust device is acting as UDS server. Set as 0 if can card is acting as UDS client(normal case). */
        uint8_t blockSize;                      /**< Blocksize if can card is acting as UDS server, 0 means no further FC is needed. Set as 0 if can card is acting as UDS client(normal case). */
        uint8_t fcFrameLength;                  /**< Flow control frame length in bytes                                                                                  */
        uint8_t reserved;
    } flowcontrol;
    uint8_t extendedAddress;                    /**< UDS Address in Extended Addressing mode                                                                             */
    uint8_t paddingEnabled;                     /**< Enable padding for unused payload bytes/                                                                            */
    uint8_t paddingValue;                       /**< Padding byte value (i.e. 0xCC) for unused payload bytes                                                             */
    uint8_t longPduEnabled;                     /**< Enable long PDU (only if CAN message DLC>8 and (CAN_DL>8 or FF_DL>4095)), otherwise BM_WriteIsotp returns an error on long write request */
    uint8_t functionalAddressingEnabled;        /**< Enable BM_ReadIsotp() to handle functional addressing UDS requests, currently only 0x7DF is supported               */
    uint8_t padding[1];
    BM_IsotpCallbackHandle callbackFunc;        /**< Callback function when any progress is made, used typically by GUI to show progress bar                             */
    uintptr_t callbackUserarg;                  /**< Callback userarg when any progress is made, used typically by GUI to show progress bar                              */
    BM_DataTypeDef testerDataTemplate;          /**< All tester messages will be formatted/checked using this template, configure CAN message ID and IDE/FDF flags here  */
    BM_DataTypeDef ecuDataTemplate;             /**< All ECU messages will be formatted/checked using this template, configure CAN message ID and IDE/FDF flags here     */
} BM_IsotpConfigTypeDef;

/**
 * @typedef BM_HwIsotpConfigTypeDef
 * @brief   Hardware ISOTP Protocol (See ISO15765-2 for details) configuration, user application shall not use this structure.
 */
typedef struct
{
    uint8_t version;                            /**< Currently must be set to 0x01                                                            */
    uint8_t mode;                               /**< Currently only 0 is supported: normal (non-extended-addressing) UDS client(tester)       */
    struct
    {
        uint16_t a;                             /**< A timeout in milliseconds: =N_As if writing as tester or reading as ECU, otherwise =N_Ar */
        uint16_t b;                             /**< B timeout in milliseconds: =N_Bs if writing as tester or reading as ECU, otherwise =N_Br */
        uint16_t c;                             /**< C timeout in milliseconds: =N_Cs if writing as tester or reading as ECU, otherwise =N_Cr */
    } testerTimeout;
    struct
    {
        uint16_t a;                             /**< A timeout in milliseconds: =N_Ar if writing as tester or reading as ECU, otherwise =N_As */
        uint16_t b;                             /**< B timeout in milliseconds: =N_Br if writing as tester or reading as ECU, otherwise =N_Bs */
        uint16_t c;                             /**< C timeout in milliseconds: =N_Cr if writing as tester or reading as ECU, otherwise =N_Cs */
    } ecuTimeout;
    struct
    {
        uint8_t stmin;                          /**< STmin raw value (0x00-0x7F or 0xF1-0xF9) if Busmust device is acting as UDS server. Set as 0 if can card is acting as UDS client(normal case). */
        uint8_t blockSize;                      /**< Blocksize if can card is acting as UDS server, 0 means no further FC is needed. Set as 0 if can card is acting as UDS client(normal case). */
        uint8_t fcFrameLength;                  /**< Flow control frame length in bytes                                                       */
        uint8_t reserved;
    } flowcontrol;
    uint8_t extendedAddress;                    /**< UDS Address in Extended Addressing mode                                                  */
    uint8_t paddingEnabled;                     /**< Enable padding for unused payload bytes/                                                 */
    uint8_t paddingValue;                       /**< Padding byte value (i.e. 0xCC) for unused payload bytes                                  */
    uint8_t longPduEnabled;                     /**< Enable long PDU (>4095), note if CAN message DLC>8, long PDU is enabled by default       */
    uint8_t functionalAddressingEnabled;        /**< Enable BM_ReadIsotp() to handle functional addressing UDS requests, currently only 0x7DF is supported */
    uint8_t padding[1];
    uint32_t unused1;
    uint32_t unused2;
    BM_DataTypeDef testerDataTemplate;          /**< All tester messages will be formatted/checked using this template, configure CAN message ID and IDE/FDF flags here  */
    BM_DataTypeDef ecuDataTemplate;             /**< All ECU messages will be formatted/checked using this template, configure CAN message ID and IDE/FDF flags here */
} BM_HwIsotpConfigTypeDef;

/**
 * @typedef BM_LinProtocolConfigTypeDef
 * @brief   LIN Protocol configuration, used by BM_ConfigLinProtocol().
 */
typedef struct
{
    uint8_t version;                            /**< 0x13=LIN1.3, 0x20=LIN2.0, 0x21=LIN2.1, 0x22=LIN2.2                                       */
    uint8_t checksum;                           /**< 0x00=manual, 0x01=normal, 0x02=enhanced                                                  */
    uint8_t reserved[2];
    struct  
    {
        uint8_t wakeLength;                     /**< 0:250us, 1:500us, 2:750us, ..., 7:2000us                                                 */
        uint8_t breakLength;                    /**< 0:13bit, 1:15bit, 2:17bit, ..., 7:27bit                                                  */
        uint8_t busInactiveTime;                /**< 0:4s, 1:6s, 2:8s, 3:10s                                                                  */
        uint8_t wakeupRepeatTime;               /**< 0:180ms, 1:200ms, 2:220ms, 3:240ms                                                       */
    } timing;
    uint8_t reserved2[8];
} BM_LinProtocolConfigTypeDef;


/**
 * @typedef BM_PathSpecTypeDef
 * @brief   File Path spec (on data files), used by BM_LoggingConfigTypeDef and BM_ReplayConfigTypeDef.
 */
typedef struct
{
    uint8_t mode;                           /**< Naming mode, see BM_StoragePathModeTypeDef for details                                   */
    uint8_t arg;                            /**< Mode argument, reserved for future                                                       */
    char format[30];                        /**< A printf-like string, used by path mode parser                                           */
} BM_PathSpecTypeDef;

/**
 * @typedef BM_EventTriggerTypeDef
 * @brief   Event trigger (on rx messages), used by BM_LoggingConfigTypeDef and BM_ReplayConfigTypeDef.
 */
typedef struct
{
    uint16_t channels;                      /**< A Bitmask of RX channels, only channels with '1' are allowed, channels=0 means invalid   */
    uint16_t reserved;
    uint16_t flags_mask;                     /**< Source message flag mask, message will be routed if msg.flags & flagsmask == flagsvalue  */
    uint16_t flags_value;                    /**< Source message flag value, See BM_MessageFlagsTypeDef for details                        */
    uint32_t id_mask;                        /**< Source message ID mask, message will be routed if msg.id & idmask == idvalue             */
    uint32_t id_value;                       /**< Source message ID value, See BM_MessageIdTypeDef for details                             */
} BM_EventTriggerTypeDef;

/**
 * @typedef BM_LoggingConfigTypeDef
 * @brief   Logging configuration, used by BM_ConfigLogging().
 */
typedef struct
{
    uint8_t version;                            /**< Currently must be set to 0x01                                                            */
    uint8_t mode;                               /**< Logging mode, see BM_StorageModeTypeDef for details                                      */
    uint8_t format;                             /**< Log file format: See BM_StorageFormatTypeDef for details.                                */
    uint8_t reserved;
    uint16_t channels;                          /**< A bitmask of TX channels, only channels with '1' are allowed as logging source channel   */
    uint8_t direction;                          /**< Logging direction, see BM_StorageDirectionTypeDef for details                            */
    uint8_t padding[9];
    BM_PathSpecTypeDef path;                    /**< Pathspec to find available files, see BM_StoragePathModeTypeDef for details.             */
    BM_EventTriggerTypeDef starttrigger;        /**< Trigger condition, recording starts automatically when it is met, only valid if mode=2   */
    BM_EventTriggerTypeDef stoptrigger;         /**< Trigger condition, recording starts automatically when it is met, only valid if mode=2   */
    struct
    {
        /**< For FIXED PATH mode: Remove all data in existing logging file on logging startup/trigger, otherwise append to existing file      */
        /**< For INDEX PATH mode: Increase file index and create new file on logging startup/trigger, otherwise continue using existing file  */
        uint8_t  createNewFileOnStart;
        uint8_t  overwriteOldFileOnFull;        /**< Enable overwriting, e.g. Overwrite 000.bbd if failed to create 003.bbd since disk is full*/
        uint16_t nfiles;                        /**< Max number of files, file name will wrap around on upper limit                           */
        uint32_t nmessagesPerFile;              /**< Max number of messages per log file (currently unsupported)                              */
        uint32_t nbytesPerFile;                 /**< Max number of bytes per log file                                                         */
        uint32_t nsecondsPerFile;               /**< Max number of seconds per log file                                                       */
    } segmentation;                             /**< Log File segmentation criteria, OR logic                                                 */
} BM_LoggingConfigTypeDef;

/**
 * @typedef BM_ReplayConfigTypeDef
 * @brief   Replay configuration, used by BM_ConfigReplay().
 */
typedef struct  
{
    uint8_t version;                            /**< Currently must be set to 0x01                                                          */
    uint8_t mode;                               /**< Replay mode, see BM_StorageModeTypeDef for details                                     */
    uint8_t format;                             /**< Log file format: See BM_StorageFormatTypeDef for details.                              */
    uint8_t reserved;
    uint16_t channels;                          /**< A bitmask of TX channels, only channels with '1' are allowed as replay TARGET channel  */
    uint8_t direction;                          /**< Replay direction, see BM_StorageDirectionTypeDef for details                           */
    uint8_t cyclic;                             /**< Replay is cyclic, otherwise the device only replay input file once on startup          */
    uint8_t padding[8];
    BM_PathSpecTypeDef path;                    /**< Pathspec to find available files, see BM_StoragePathModeTypeDef for details.           */
    BM_EventTriggerTypeDef starttrigger;        /**< Trigger condition, replay starts automatically when it is met, only valid if mode=2    */
    BM_EventTriggerTypeDef stoptrigger;         /**< Trigger condition, replay stops automatically when it is met, only valid if mode=2     */
    struct
    {
        uint16_t msgdelay;                      /**< Delay in ms between replay messages, 0 means keeping original delay                    */
        uint16_t sessiondelay;                  /**< Delay in ms between replay sessions, 0 means keeping original delay                    */
        uint16_t cycledelay;                    /**< Delay in ms between replay cycles, 0 means keeping original delay                      */
        uint8_t forceZeroTimestampOnFirstMsg;   /**< Align 1st replayable message with zero timestamp, and send it immediately on startup   */
        uint8_t reserved[1];
    } timing;
} BM_ReplayConfigTypeDef;

/**
 * @typedef BM_FileInfoTypeDef
 * @brief   File (in device storage) information, used by BM_GetFileInfo().
 */
typedef struct
{
    uint32_t nbytes;
    uint32_t nmessages;
    uint32_t attributes;        /* File attribute bitmask, see BM_FileAttributeTypeDef for details. */
    uint32_t crc32;
    uint32_t createdtime;
    uint32_t padding1;
    uint32_t modifiedtime;
    uint32_t padding2;
    char path[32];
} BM_FileInfoTypeDef;

/**
 * @typedef BM_VolumeInfoTypeDef
 * @brief   Volume (of device storage) information, used by BM_GetVolumeInfo().
 */
typedef struct
{
    uint32_t volumesizel;
    uint32_t volumesizeh;
    uint32_t freesizel;
    uint32_t freesizeh;
    uint32_t nrecordings;
    uint32_t nreplays;
    uint32_t nconfigs;
    uint32_t ncodecs;
    uint32_t ne2es;
    uint32_t nmappings;
    uint32_t reserved[6];
} BM_VolumeInfoTypeDef;

#ifdef __cplusplus
};
#endif

#endif /* #ifndef __BM_USB_DEF_H__ */
/**
 * End of file
 */
