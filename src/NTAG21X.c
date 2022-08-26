/**
 * \file NTAG21X.c
 * \author Orion Serup (orionserup@gmail.com)
 * \brief 
 * \version 0.1
 * \date 2022-08-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "NTAG21X.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static const uint16_t atqa = 0x0044;
static const uint8_t sak = 0x00;

const NTAG21XConfig NTAG21XDefaultConfig() {

    const static NTAG21XConfig config = {    
        .receive_bits = NULL,
        .transmit_bits = NULL,
        .receive_bits_crc = NULL,
        .transmit_bits_crc = NULL,
        .detectcollision = NULL,
        .calculate_crc16 = NULL,
    };

    return config;
}

const NTAG21XSettings NTAG21XDefaultSettings() {

    const static NTAG21XSettings settings = {

        .auth_lim = 0,
        .cfg_lock = false,
        .mirror = NO_MIRROR,
        .mirror_page = 0xFF,
        .nfc_cntr_en = false,
        .pwd_ack = 0x0000,
        .password = 0xFFFFFFFF,
        .pwd_lock = false,
        .strong_mod = false,
        .pwd_prot_base = 0xFF

    };

    return settings;

}

NTAG21X* NTAG21XInit(NTAG21X* const dev, NTAG21XConfig* const config) {

    assert(dev && config);

    memcpy(&dev->config, config, sizeof(NTAG21XConfig));

    const NTAG21XType tag = dev->config.tag;

    const uint8_t cfgpage = tag == NTAG_213? 0x29:
                            tag == NTAG_215? 0x83:
                            tag == NTAG_216? 0xE3: 0xFF;

    return dev;

}

void NTAG21XDeinit(NTAG21X* const dev) {

    assert(dev);

    dev->config = NTAG21XDefaultConfig();
    dev->settings = NTAG21XDefaultSettings();
    
    memset(dev->uid, 0, 7);

}


bool NTAG21XConnect(NTAG21X* const dev, const uint8_t uid[7]) {

    assert(dev && uid);




}



uint16_t NTAG21XSend(const NTAG21X* const dev, const void* const buffer, const uint16_t bits, const bool crc) {

    assert(dev && buffer && bits);

    if(!dev->connected)
        return 0;

    if(crc) {

        assert((dev->config.calculate_crc16 && dev->config.transmit_bits) || dev->config.transmit_bits_crc);
        bool hwcrc = (dev->config.transmit_bits_crc != NULL);

        if(hwcrc)
            return dev->config.transmit_bits_crc(buffer, bits);

        else {

            static uint8_t sendbuffer[512] = {0};
            uint16_t bytes = (bits >> 3) + (bits & 0x7 != 0); // how may bytes are needed to represent all of the data
            uint8_t numbits = bits & 0x7; // how many incomplete bits there are
            
            if(numbits != 0)
                return 0;
            
            uint16_t crcval = dev->config.calculate_crc16(buffer, bytes); // if we are doing whole bytes just to regular crc otherwise go one byte more for the bits
            
            memcpy(sendbuffer, buffer, bytes); // copy the whole array including bits as an extra byte if needed
            memcpy(buffer + bytes, &crcval, 2);

            return dev->config.transmit_bits(sendbuffer, (bytes + 2) * 8);

        }
    }
    else {
        assert(dev->config.transmit_bits);
        return dev->config.transmit_bits(buffer, bits);
    }
}

NTAG21XACK NTAG21XRecv(NTAG21X* const dev, void* const buffer, const uint16_t bits, const bool crc) {

    assert(dev && buffer && bits);

    if(!dev->connected)
        return NAK_DISCON;

    uint16_t result = 0;

    if(crc) {

        assert((dev->config.calculate_crc16 && dev->config.receive_bits) || dev->config.receive_bits_crc);
        bool hwcrc = (dev->config.receive_bits_crc != NULL);

        if(hwcrc)
            result = dev->config.receive_bits_crc(buffer, bits);

        else {
            uint16_t bytes = (bits >> 3) + (bits & 0x7 != 0); // how may bytes are needed to represent all of the data
            uint8_t numbits = bits & 0x7; // how many incomplete bits there are
            
            if(numbits != 0) // we can always 
                return 0;

            uint16_t res = dev->config.receive_bits(buffer, bits);

            if(res == 0) {
                dev->connected = false;
                return NAK_TIMEOUT;
            }

            if(res == 8)
                return ((NTAG21XACK*)(buffer))[0];

            uint16_t crcval = dev->config.calculate_crc16(buffer, bytes); // if we are doing whole bytes just to regular crc otherwise go one byte more for the bits

            int res = memcmp(buffer + res - 2, &crcval, 2);
            if(!res)
                return NAK_CRC;

            return ACK;
        }
    }
    else {
        assert(dev->config.transmit_bits);
        uint16_t val = dev->config.receive_bits(buffer, bits);
        if(val == 0) // if we didn't receive anything its a timeout
            return NAK_TIMEOUT;
        if(val == 8) // if we received only one 
            return ((NTAG21XACK*)buffer)[0];
        return ACK;
    }
}

NTAG21XACK NTAG21XWriteSettings(const NTAG21X* const dev, const NTAG21XSettings* const settings) {

    assert(dev && settings);

    if(!dev->connected)
        return NAK_DISCON;

    NTAG21XType chip = dev->config.tag;
    uint8_t basepage =  chip == NTAG_213? 0x29:
                        chip == NTAG_215? 0x83:
                        chip == NTAG_216? 0xE3: 0xFF;
    
    for (uint8_t i = 0; i < 4; i++) {
        NTAG21XACK ack = NTAG21XWrite(dev, basepage + i, settings + 4 * i);        // write the first page of the config
        if(ack != ACK)
            return ack;
    }
    return ACK;
}

NTAG21XACK NTAG21XReadSettings(const NTAG21X* const dev, NTAG21XSettings* const settings) {

    assert(dev && settings);

    if(!dev->connected)
        return NAK_DISCON;

    NTAG21XType chip = dev->config.tag;
    uint8_t basepage =  chip == NTAG_213? 0x29:
                        chip == NTAG_215? 0x83:
                        chip == NTAG_216? 0xE3: 0xFF;

    return NTAG21XFastRead(dev, basepage, basepage + 3, settings);

}

NTAG21XACK NTAG21XGetVersion(NTAG21X* const dev, NTAG21XVersion* const version) {

    assert(dev && version);

    if(!dev->connected)
        return NAK_DISCON;

    uint8_t cmd = GET_VERSION;

    NTAG21XSend(dev, &cmd, 8, true);
    return NTAG21XRecv(dev, version, 64, true);

}

NTAG21XACK NTAG21XPwdAuth(NTAG21X* const dev, const uint32_t pass) {

    assert(dev);

    if(!dev->connected)
        return NAK_DISCON;

    static uint8_t buffer[5];
    buffer[0] = PWD_AUTH;
    memcpy(buffer + 1, &pass, sizeof(uint32_t));
    NTAG21XSend(dev, buffer, 5 * 8, true);
    NTAG21XACK ack = NTAG21XRecv(dev, buffer, 8, true);

    if(ack == ACK && dev->settings.pwd_ack == buffer[0])
        return ACK;

    return ack;

}

NTAG21XACK NTAG21XReadSig(NTAG21X* const dev, void* const signature) {

    assert(dev && signature);

    if(!dev->connected)
        return NAK_DISCON;

    static uint8_t buffer[2];
    buffer[0] = READ_SIG;
    buffer[1] = 0;
    
    NTAG21XSend(dev, buffer, 8 * 2, true);
    return NTAG21XRecv(dev, signature, 8 * 32, true);

}

NTAG21XACK NTAG21XFastRead(NTAG21X* const dev, const uint8_t start, const uint8_t stop, void* const output) {

    assert(dev && stop >= start && output);

    if(!dev->connected)
        return NAK_DISCON;

    static uint8_t buffer[3];
    buffer[0] = FAST_READ;
    buffer[1] = start;
    buffer[2] = stop;

    NTAG21XSend(dev, buffer, 8 * 3, true);
    return NTAG21XRecv(dev, output, (stop - start + 1) * 32, true);

}

NTAG21XACK NTAG21XRead(NTAG21X* const dev, const uint8_t page, void* const output) {

    assert(dev && output);

    if(!dev->connected)
        return NAK_DISCON;

    static uint8_t buffer[2];
    buffer[0] = READ;
    buffer[1] = page;

    NTAG21XSend(dev, buffer, 16, true);
    return NTAG21XRecv(dev, output, 16 * 8, true);

}

NTAG21XACK NTAG21XReadCntr(NTAG21X* const dev, const uint8_t counter, uint32_t* const counterval) {

    assert(dev && counterval);

    if(!dev->connected)
        return NAK_DISCON;

    static uint8_t buffer[2];
    buffer[0] = READ_CNT;
    buffer[1] = counter;

    NTAG21XSend(dev, buffer, 16, true);
    return NTAG21XRecv(dev, counterval, 24, true);

}

NTAG21XACK NTAG21XWrite(NTAG21X* const dev, const uint8_t start, void* const data) {

    assert(dev && data);

    if(!dev->connected)
        return NAK_DISCON;

    static uint8_t buffer[6];
    buffer[0] = WRITE;
    buffer[1] = start;
    memcpy(buffer + 2, data, 4);

    NTAG21XSend(dev, buffer, 48, true);        // send the command and the data
    return NTAG21XRecv(dev, buffer, 4, false); // listen for an ACK

}

NTAG21XACK NTAG21XCompWrite(const NTAG21X* const dev, const uint8_t page, const uint32_t data) {

    assert(dev && data);

    if(!dev->connected)
        return NAK_DISCON;

    static uint8_t buffer[16];
    buffer[0] = COMP_WRITE;
    buffer[1] = page;

    NTAG21XSend(dev, buffer, 16, true);
    NTAG21XACK ack = NTAG21XRecv(dev, buffer, 4, false);
    if(ack != ACK)
        return ACK;

    memcpy(buffer + 12, &data, 4);
    NTAG21XSend(dev, buffer, 16 * 8, true);
    return NTAG21XRecv(dev, buffer, 4, false);

}
