// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#ifndef RECEIVE_VIEW_H
#define RECEIVE_VIEW_H

#pragma once

#include <furi.h>
#include <src/include/fgp_app.h>

void *fgp_receive_view_alloc(struct fgp_app *fgp);

void fgp_receive_view_free(void *recv_ctx);

View *fgp_receive_view_get_view(void *recv_ctx);

#endif //RECEIVE_VIEW_H
