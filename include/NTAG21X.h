/**
 * \file NTAG21X.h
 * \author Orion Serup (orionserup@gmail.com)
 * \brief 
 * \version 0.1
 * \date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef NTAG21X_H
#define NTAG21X_H

#include <stdint.h>
#include <stdbool.h>

/// @brief All of the Different Tags
typedef enum NTAG21XTYPE {

    NTAG_213,   ///< We are using the NTAG213 IC
    NTAG_215,   ///< We are using the NTAG215 IC
    NTAG_216    ///< We are using the NTAG216 IC

} NTAG21XType;

/// @brief All of the Commands for the Tag
typedef enum NTAG21XCOMMAND {

    REQUEST     = 0x26, ///< Request To Connect to A TAG
    HALT        = 0x52, ///< Halt the device 
    GET_VERSION = 0x60, ///< Get the Product Version Information
    READ        = 0x30, ///< Reads 8 bytes for the Tag
    FAST_READ   = 0x3A, ///< Reads A Variable Number of Bytes from the Device
    WRITE       = 0xA2, ///< Write 32-bits to the device
    COMP_WRITE  = 0xA0, ///< Write 32-bits to the address, but write 16 bytes for compatibility 
    READ_CNT    = 0x39, ///< Read the 24-bit counter value from the device
    PWD_AUTH    = 0x1B, ///< Authenticate with the Password
    READ_SIG    = 0x3C  ///< Read the Unique Device Signature

} NTAG21XCommand;

/// @brief What the Chip Will send back after receiving a Message/Request
typedef enum NTAG21XACK {

    ACK             = 0xA,  ///< The Message was Acknowledged 
    NAK_ARG         = 0x0,  ///< The Message was Not Acknowledged due to a Invalid Argument or Page Address 
    NAK_CRC         = 0x1,  ///< The Message was Not Acknowledged due to a CRC or Parity Error
    NAK_AUTH_OVF    = 0x4,  ///< The Message was Not Acknowledged due to a bad Authentication counter overflow
    NAK_WE          = 0x5,  ///< The Message was Not Acknowledged Due to a EEPROM Write Error   
    NAK_TIMEOUT     = 0xF   ///< The Message was not acknowledged because of a Timeout event

} NTAG21XACK;

typedef enum NTAG21XMIRROR {

    NO_MIRROR         = 0x0,  ///< No Mirroring in the User Data 
    UID_MIRROR        = 0x1,  ///< UID Mirrored in the User Data in Hex ASCII 
    NFC_CNT_MIRROR    = 0x2,  ///< NFC Counter Mirrored in User Space in HEX ASCII 
    UID_NFC_CNT_MIRROR= 0x3   ///< Both the Both the UID and the Counter are mirrored into user dat in hex ascii

} NTAG21XMirror;

/// @brief The Configuration Parameters for the Device 
typedef struct NTAG21XCONFIG {

    uint16_t (*transmit_bits)(const void* const data, const uint16_t bits, const bool withcrc);
    uint16_t (*receive_bits)(void* const data, const uint16_t bits, const bool withcrc);

    bool hw_crc;            ///< Using Hardware CRC 
    uint16_t (*calculate_crc16)(const void* const data, const uint16_t size);

    uint16_t (*detectcollision)(void);

    uint8_t mirror_page;    ///< The Page with the Mirroring
    uint8_t pwd_prot_base;  ///< The Base Page that is Password Protected

    bool strong_mod;        ///< Strong Modulation enable 

    bool pwd_lock;          ///< If the Reading and Writing is Password Protected
    bool cfg_lock;          ///< If the Configuration Has been Permanently Locked
    bool nfc_cntr_en;       ///< Enable the NFC Access counter

    uint8_t auth_lim : 4;   ///< How many Authentication Attempts are allowed Before Getting Locked out

    uint16_t pwd_ack;       ///< What The Password Acknowledgement Is 
    uint32_t password;      ///< What The Access Password is

    NTAG21XType tag;        ///< What Type of Chip we are using

} NTAG21XConfig;

/// @brief Device Struct 
typedef struct NTAG21X {

    NTAG21XConfig config;   ///< Device Configuration
    uint8_t serialnum[7];   ///< Device Serial Number

} NTAG21X;

/**
 * \brief 
 * 
 * \param dev
 * \param config
 * \return NTAG21X* 
 */
NTAG21X* NTAG21XInit(NTAG21X* const dev, const NTAG21XConfig* const config);

/**
 * \brief 
 * 
 * \return NTAG21XConfig 
 */
NTAG21XConfig NTAG21XDefaultConfig();

/**
 * \brief 
 * 
 * \param dev
 * \param version
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XGetVersion(NTAG21X* const dev, void* const version);

/**
 * \brief 
 * 
 * \param dev
 * \param pass
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XPwdAuth(NTAG21X* const dev, const uint32_t pass);

/**
 * \brief 
 * 
 * \param dev
 * \param singature
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XReadSig(NTAG21X* const dev, void* const singature);

/**
 * \brief 
 * 
 * \param dev
 * \param start
 * \param stop
 * \param output
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XFastRead(NTAG21X* const dev, const uint8_t start, const uint8_t stop, void* const output);

/**
 * \brief 
 * 
 * \param dev
 * \param page
 * \param output
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XRead(NTAG21X* const dev, const uint8_t page, void* const output);

/**
 * \brief 
 * 
 * \param dev
 * \param counterval
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XReadCntr(NTAG21X* const dev, uint32_t* const counterval);

/**
 * \brief 
 * 
 * \param dev
 * \param start
 * \param data
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XWrite(NTAG21X* const dev, const uint8_t start, void* const data);

/**
 * \brief 
 * 
 * \param dev
 * \param page
 * \param data
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XCompWrite(NTAG21X* const dev, const uint8_t page, const void* const data);

#endif