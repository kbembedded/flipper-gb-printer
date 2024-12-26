// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#ifndef SEND_VIEW_H
#define SEND_VIEW_H

#pragma once

#include <furi.h>
#include <src/include/fgp_app.h>

void *fgp_send_view_alloc(struct fgp_app *fgp);

void fgp_send_view_free(void *send_ctx);

void fgp_send_view_exposure_set(void *send_ctx, uint8_t val);

void fgp_send_view_tail_len_set(void *send_ctx, uint8_t val);

View *fgp_send_view_get_view(void *recv_ctx);

#endif //SEND_VIEW_H
