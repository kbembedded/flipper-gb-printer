// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 Esteban Fuentealba
// Copyright (c) 2024 KBEmbedded

#ifndef FGP_PALETTE_H
#define FGP_PALETTE_H

#pragma once

size_t palette_count_get(void);

char *palette_name_get(unsigned int idx);

char *palette_shortname_get(unsigned int idx);

void *palette_rgb16_get(unsigned int idx);

#endif // FGP_PALETTE_H
