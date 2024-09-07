// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#ifndef CRC_H
#define CRC_H

#pragma once

/* Return the CRC of the bytes buf[0..len-1]. */
uint32_t crc(uint8_t *buf, size_t len);

#endif // CRC_H
