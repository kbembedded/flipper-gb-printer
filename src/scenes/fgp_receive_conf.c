// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#include <furi.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <storage/storage.h>

#include <src/include/fgp_app.h>
#include <src/scenes/include/fgp_scene.h>

const char * const list_text[] = {
	"Add header?",
	"Receive",
};


const char * const yes_no_text[] = {
	"No",
	"Yes",
};

static void add_header_change(VariableItem *item)
{
	struct fgp_app *fgp = variable_item_get_context(item);
	uint8_t index = variable_item_get_current_value_index(item);

	variable_item_set_current_value_text(item, yes_no_text[index]);
	fgp->add_header = index;
}

static void enter_callback(void* context, uint32_t index) {
	struct fgp_app *fgp = context;

	/* When the user presses enter on the last option in the list, that is
	 * the cue to switch to the next scene.
	 */
	FURI_LOG_D("RECV", "index %ld", index);
	if (index == COUNT_OF(list_text) - 1)
		view_dispatcher_send_custom_event(fgp->view_dispatcher, fgpViewReceive);
}

void fgp_scene_receive_conf_on_enter(void* context)
{
	struct fgp_app *fgp = context;
	VariableItem *item;

	variable_item_list_reset(fgp->variable_item_list);

	/* Default enable adding the header */
	fgp->add_header = true;
	item = variable_item_list_add(fgp->variable_item_list,
				      list_text[0],
				      COUNT_OF(yes_no_text),
				      add_header_change,
				      fgp);
	variable_item_set_current_value_index(item, fgp->add_header);
	variable_item_set_current_value_text(item, yes_no_text[fgp->add_header]);

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

bool fgp_scene_receive_conf_on_event(void* context, SceneManagerEvent event)
{
	struct fgp_app *fgp = context;
	bool consumed = false;
	
	if (event.type == SceneManagerEventTypeCustom) {
		view_dispatcher_switch_to_view(fgp->view_dispatcher, event.event);
		consumed = true;
	}
	return consumed;
}

void fgp_scene_receive_conf_on_exit(void* context) {
	UNUSED(context);
}