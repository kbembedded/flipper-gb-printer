// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#ifndef FGP_APP_H
#define FGP_APP_H

#pragma once

// Libraries
#include <gblink.h>

// FURI
#include <furi.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <storage/storage.h>

struct fgp_app {
	ViewDispatcher *view_dispatcher;
	SceneManager *scene_manager;
	Submenu *submenu;
	Storage *storage;

	void *printer_handle;
	void *data; // Buffer used to send or receive
	size_t len; // Length of buffer data

	void *gblink_handle;

};

typedef enum {
	fgpViewSubmenu,
	fgpViewReceive,
} fgpView;

#endif // FGP_APP_H
