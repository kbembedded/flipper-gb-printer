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

struct fgp_app {
	ViewDispatcher *view_dispatcher;
	SceneManager *scene_manager;
	Submenu *submenu;

	void *gblink_handle;
};

typedef enum {
	fgpViewSubmenu,
} fgpView;

#endif // FGP_APP_H
