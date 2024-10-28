// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#include <furi.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <storage/storage.h>
#include <lib/toolbox/value_index.h>
#include <src/include/fgp_app.h>
#include <src/scenes/include/fgp_scene.h>

#include <src/include/fgp_palette.h>

static const char * const list_text[] = {
	"Select Pinout",
	"Send!",
};

static void enter_callback(void* context, uint32_t index)
{
	struct fgp_app *fgp = context;

	scene_manager_set_scene_state(fgp->scene_manager,
				      fgpSceneSendConf,
				      index);
	/* When the user presses enter on the last option in the list, that is
	 * the cue to switch to the next scene.
	 */
	if (index == COUNT_OF(list_text) - 1)
		view_dispatcher_send_custom_event(fgp->view_dispatcher, 0);
	if (index == COUNT_OF(list_text) - 2)
		view_dispatcher_send_custom_event(fgp->view_dispatcher, 1);
}

void fgp_scene_send_conf_on_enter(void* context)
{
	struct fgp_app *fgp = context;
	VariableItem *item = NULL;
	UNUSED(item);

	variable_item_list_reset(fgp->variable_item_list);
	variable_item_list_set_selected_item(fgp->variable_item_list,
					     scene_manager_get_scene_state(fgp->scene_manager,
									   fgpSceneSendConf));


	item = variable_item_list_add(fgp->variable_item_list,
				      list_text[0],
				      0,
				      NULL,
				      fgp);

	item = variable_item_list_add(fgp->variable_item_list,
				      list_text[1],
				      0,
				      NULL,
				      fgp);

	variable_item_list_set_enter_callback(fgp->variable_item_list,
					      enter_callback,
					      fgp);

	view_dispatcher_switch_to_view(fgp->view_dispatcher, fgpViewVariableItemList);
}

bool fgp_scene_send_conf_on_event(void* context, SceneManagerEvent event)
{
	struct fgp_app *fgp = context;
	bool consumed = false;

	if (event.type == SceneManagerEventTypeCustom) {
		if (event.event == 0)
			view_dispatcher_switch_to_view(fgp->view_dispatcher, fgpViewSend);
		else
			scene_manager_next_scene(fgp->scene_manager, fgpSceneSelectPins);
		consumed = true;
	}
	return consumed;
}

void fgp_scene_send_conf_on_exit(void* context) {
	UNUSED(context);
}
