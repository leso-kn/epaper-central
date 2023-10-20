/*
 *  SPDX-License-Identifier: MIT
 *  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
 *
 *  epaper-central - Serial protocol definition.
 */

#include <stdint.h>

struct __attribute__((__packed__)) AvailableDataRequest
{
    uint8_t outerChecksum;
    uint64_t sourceMac;
    uint8_t innerChecksum;
    uint8_t lastPacketLQI;
    int8_t lastPacketRSSI;
    uint8_t temperature;
    uint16_t batteryMv;
    uint8_t hwType;
    uint8_t wakeupReason;
};

struct __attribute__((__packed__)) AvailDataInfo
{
    uint8_t checksum;
    uint64_t dataVer;
    uint32_t dataSize;
    uint8_t dataType;
    uint8_t dataTypeArgument;
    uint16_t nextCheckIn;
    uint16_t attemptsLeft;
    uint64_t targetMac;
};

struct __attribute__((__packed__)) BlockRequest
{
    uint8_t checksum;
    uint64_t dataVer;
    uint8_t blockId;
    uint64_t srcMac;
};

struct __attribute__((__packed__)) BlockHeader
{
    uint16_t length;
    uint16_t checksum;
};
