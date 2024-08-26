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
#include <gui/modules/variable_item_list.h>
#include <storage/storage.h>

struct fgp_app {
	ViewDispatcher *view_dispatcher;

	// Scenes
	SceneManager *scene_manager;
	Submenu *submenu;
	VariableItemList *variable_item_list;

	Storage *storage;

	void *receive_handle;
	void *gblink_handle;

	bool add_header;
};

typedef enum {
	fgpViewSubmenu,
	fgpViewVariableItemList,
	fgpViewReceive,
} fgpView;

#endif // FGP_APP_H
