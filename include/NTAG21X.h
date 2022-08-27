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

    REQUEST         = 0x26,     ///< Request To Connect to A TAG / 7-bits
    WAKEUP          = 0x52,     ///< Wakeup the Device From A Halt
    HALT            = 0x50,     ///< Halts the Device But stays connected
    GET_VERSION     = 0x60,     ///< Get the Product Version Information
    READ            = 0x30,     ///< Reads 8 bytes for the Tag
    FAST_READ       = 0x3A,     ///< Reads A Variable Number of Bytes from the Device
    WRITE           = 0xA2,     ///< Write 32-bits to the device
    COMP_WRITE      = 0xA0,     ///< Write 32-bits to the address, but write 16 bytes for compatibility 
    READ_CNT        = 0x39,     ///< Read the 24-bit counter value from the device
    PWD_AUTH        = 0x1B,     ///< Authenticate with the Password
    READ_SIG        = 0x3C,     ///< Read the Unique Device Signature
    SELECT_CL1      = 0x93,     ///< Selects a Chip or does the anticollision procedure fro level 1
    SELECT_CL2      = 0x95      ///< Selects or does the anticollision procedure for level 2

} NTAG21XCommand;

/// @brief What the Chip Will send back after receiving a Message/Request
typedef enum NTAG21XACK {

    ACK             = 0xA,  ///< The Message was Acknowledged 
    NAK_ARG         = 0x0,  ///< The Message was Not Acknowledged due to a Invalid Argument or Page Address 
    NAK_CRC         = 0x1,  ///< The Message was Not Acknowledged due to a CRC or Parity Error
    NAK_AUTH_OVF    = 0x4,  ///< The Message was Not Acknowledged due to a bad Authentication counter overflow
    NAK_WE          = 0x5,  ///< The Message was Not Acknowledged Due to a EEPROM Write Error   
    NAK_TIMEOUT     = 0xF,  ///< The Message was not acknowledged because of a Timeout event
    NAK_DISCON      = 0xC   ///< If the device is Disconnected 

} NTAG21XACK;

typedef enum NTAG21XMIRROR {

    NO_MIRROR         = 0x0,  ///< No Mirroring in the User Data 
    UID_MIRROR        = 0x1,  ///< UID Mirrored in the User Data in Hex ASCII 
    NFC_CNT_MIRROR    = 0x2,  ///< NFC Counter Mirrored in User Space in HEX ASCII 
    UID_NFC_CNT_MIRROR= 0x3   ///< Both the Both the UID and the Counter are mirrored into user dat in hex ascii

} NTAG21XMirror;

/// @brief Version Information
typedef struct NTAG21XVERSION {

    uint8_t fixed_header;   ///< Will Always be 0x00
    uint8_t vendor_id;      ///< Vendor Identification, in this case 0x4 for NXP
    uint8_t product_type;   ///< Type of Product, in this case 0x4 for NTAG
    uint8_t product_subtype;///< Product Subtype, in this case 0x2 for 50pF
    uint8_t major_prod_vers;///< Major Product Version
    uint8_t minor_prod_vers;///< Minor Product Version
    uint8_t storage_size;   ///< The Storage Size Signifier, Most Significant 7-bits, n indicate that the storage is >= 2^n bytes, the Least Significant bit indicates if it is greater 
    uint8_t protocol_type;  ///< The Access Protocol, in this case 0x3 for ISO14443-3

} NTAG21XVersion;

/// @brief The Run Time Settings for the Device, Associated with the Configuration Page
typedef struct NTAG21XSETTINGS {

    // Mirror Byte //
    uint8_t mirror : 2;     ///< The Mirror Type, See \ref NTAG21XMirror
    uint8_t mirror_byte : 2;///< The Bit Position Within the Page to Mirror  
    uint8_t rfui : 1;       ///< Reserved For Future Use
    bool strong_mod : 1;    ///< Using string modulation or not
    uint8_t rfui1 : 2;      ///< Reserved for future use
    uint8_t rfui2;          ///< Reserved For Future Use
    
    // Mirror Page Byte //
    uint8_t mirror_page;    ///< The Page with the Mirroring
    
    // Password Protection Register //
    uint8_t pwd_prot_base;  ///< The Base Page that is Password Protected

    // Access Byte //
    bool pwd_lock : 1;      ///< If the Reading and Writing is Password Protected
    bool cfg_lock : 1;      ///< If the Configuration Has been Permanently Locked
    uint8_t rfui3 : 1;      ///< Reserved For future use
    bool nfc_cntr_en : 1;   ///< Enable the NFC Access counter
    bool nfc_cntr_prot : 1; ///< If the NFC Counter is Password Protected
    uint8_t auth_lim : 3;   ///< How many Authentication Attempts are allowed Before Getting Locked out

    uint8_t rfui4[3];       ///< Reserved for future use

    // Password Word //
    uint32_t password;      ///< What The Access Password is
    
    // Password Acknowledgement Message //
    uint16_t pwd_ack;       ///< What The Password Acknowledgement Is 
    
    uint8_t rfui5[2];       ///< Reserved for Future Use

} NTAG21XSettings;

/// @brief The Configuration Parameters for the Device 
typedef struct NTAG21XCONFIG {

    uint16_t (*transmit_bits_crc)(const void* const data, const uint16_t bits); ///< Function to Send Bytes And CRC Afterwards, Optional
    uint16_t (*receive_bits_crc)(void* const data, const uint16_t bits);        ///< Function To Receive Bytes and Verify CRC16, Optional

    uint16_t (*transmit_bits)(const void* const data, const uint16_t bits);     ///< Function to Transmit Raw Bits Over ISO1443A Signal, Required
    uint16_t (*receive_bits)(void* const data, const uint16_t bits);            ///< Function to Read Bits from the Device over the ISO1443A Signal, Required

    uint16_t (*calculate_crc16)(const void* const data, const uint16_t size);   ///< Function To Calculate CRC, if the transmit_bits_crc function member is NULL this is used to check and calculate crc

    uint16_t (*detectcollision)(void);                                          ///< Detects Collisions when detecting Chips in the Field 

    NTAG21XType tag;        ///< What Type of Chip we are using

} NTAG21XConfig;

/// @brief Device Struct 
typedef struct NTAG21X {

    NTAG21XConfig config;       ///< Device Configuration
    NTAG21XSettings settings;   ///< Device Settings

    uint8_t uid[7];             ///< Device Unique ID
    bool connected;             ///< If the Device is In the Field and is Writable is changed upon unsuccessful read or write
    bool awake;                 ///< If the Device is Woken Up and Can be halted

} NTAG21X;

// -------------------------- Init/Deinit Functions --------------------- //

/**
 * \brief 
 * 
 * \param[in] dev
 * \param[inout] config: Configuration to Write to Device, if Device Configuration is Locked then it 
 * \return NTAG21X* 
 */
NTAG21X* NTAG21XInit(NTAG21X* const dev, NTAG21XConfig* const config);

/**
 * \brief Deinits an Device
 * 
 * \param[in] dev: Device to Deinitialize
 */
void NTAG21XDeinit(NTAG21X* const dev);

// --------------------------- Connection and Disconnection ---------------- //

/**
 * \brief Detects if there is a tag in the field
 * 
 * \param dev
 * \return true 
 * \return false 
 */
bool NTAG21XDetect(NTAG21X* const dev); 

/**
 * \brief 
 * 
 * \param dev
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XAutoConnect(NTAG21X* const dev);

/**
 * \brief 
 * 
 * \param dev
 * \param uid
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XConnect(NTAG21X* const dev, const uint8_t uid[7]);

/**
 * \brief 
 * 
 * \param dev
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XDisconnnect(NTAG21X* const dev);

// ------------------------------- Utility Functions -----------------------------//

/**
 * \brief 
 * 
 * \return NTAG21XConfig 
 */
const NTAG21XConfig NTAG21XDefaultConfig();

/**
 * \brief 
 * 
 * \return NTAG21XSettings 
 */
const NTAG21XSettings NTAG21XDefaultSettings();

/**
 * \brief 
 * 
 * \param dev
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XWakeUp(NTAG21X* const dev);

/**
 * \brief 
 * 
 * \param dev
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XHalt(NTAG21X* const dev);

/**
 * \brief 
 * 
 * \param dev
 * \param pass
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XPwdAuth(NTAG21X* const dev, const uint32_t pass);

// -------------------------- Sending And Receiving Functions ------------------------------ //

/**
 * \brief 
 * 
 * \param dev
 * \param buffer
 * \param bits
 * \param crc
 * \return uint16_t 
 */
uint16_t NTAG21XSend(const NTAG21X* const dev, const void* const buffer, const uint16_t bits, const bool crc);

/**
 * \brief Reads from a device 
 * 
 * \param dev: Device to Read from 
 * \param buffer: Where to write the data read to
 * \param bits: How many bits to read
 * \param crc: If we want to Verify CRC
 * \return uint16_t: How many bits are read
 */
NTAG21XACK NTAG21XRecv(NTAG21X* const dev, void* const buffer, const uint16_t bits, const bool crc);

// ------------------------------ Reading and Writing Functions ----------------------- //

/**
 * \brief 
 * 
 * \param dev
 * \param settings
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XWriteSettings(const NTAG21X* const dev, const NTAG21XSettings* const settings);

/**
 * \brief 
 * 
 * \param dev
 * \param uid
 * \return NTAG21XACK 
 */
NTAG21XACK NTAGXReadUID(NTAG21X* const dev, void* const uid);

/**
 * \brief 
 * 
 * \param dev
 * \param version
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XGetVersion(NTAG21X* const dev, NTAG21XVersion* const version);

/**
 * \brief 
 * 
 * \param dev
 * \param settings
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XReadSettings(const NTAG21X* const dev, NTAG21XSettings* const settings);

/**
 * \brief 
 * 
 * \param dev
 * \param uid
 * \return NTAG21XACK 
 */
NTAG21XACK NTAGXReadUID(NTAG21X* const dev, void* const uid);

/**
 * \brief 
 * 
 * \param dev
 * \param singature
 * \return NTAG21XACK 
 */
NTAG21XACK NTAG21XReadSig(NTAG21X* const dev, void* const signature);

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
NTAG21XACK NTAG21XReadCntr(NTAG21X* const dev, const uint8_t counter, uint32_t* const counterval);

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