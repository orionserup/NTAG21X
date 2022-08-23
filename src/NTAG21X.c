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
#include <stdlib.h>

static const uint16_t atqa = 0x0044;
static const uint8_t sak = 0x00;

NTAG21XConfig NTAG21XDefaultConfig() {

    NTAG21XConfig config = {

        .auth_lim = 0,
        .cfg_lock = 0,
        .pwd_lock = 
        .hw_crc = true,
        .nfc_cntr_en = false,

        .receive_bits = NULL,
        .transmit_bits = NULL,
        .detectcollision = NULL,
        .calculate_crc16 = NULL,



    };

    return config;

}