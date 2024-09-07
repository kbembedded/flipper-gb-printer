// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#ifndef PNG_H
#define PNG_H

#pragma once

void png_stuff(void *png_handle, uint8_t *gb_buf);

/* Allocates pixel array */
void *png_init(uint32_t width, uint32_t height);

void png_free(void *png_handle);

uint8_t *png_get_buf(void *png_handle);

size_t png_len(void *png_handle);

#endif // PNG_H
