// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#include <furi.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <storage/storage.h>
#include <lib/toolbox/value_index.h>
#include <src/include/fgp_app.h>
#include <src/scenes/include/fgp_scene.h>
#include <src/views/include/send_view.h>

#include <src/include/fgp_palette.h>

static const char * const list_text[] = {
	"Tail Length",
	"Exposure",
	"Send!",
};

static const char * const exposure_text[] = {
	"-25%",
	"-20%",
	"-15%",
	"-10%",
	"-5%",
	"Default", // Default exposure
	"+5%",
	"+10%",
	"+15%",
	"+20%",
	"+25%",
};

static const uint8_t exposure_val[] = {
	0x00,
	0x0d,
	0x1a,
	0x27,
	0x34,
	0x40,
	0x4e,
	0x5b,
	0x68,
	0x75,
	0x7f,
};

static void tail_len_change(VariableItem *item)
{
	struct fgp_app *fgp = variable_item_get_context(item);
	uint8_t index = variable_item_get_current_value_index(item);
	char str[2] = {0};

	fgp_send_view_tail_len_set(fgp->send_view, index+1);
	str[0] = '1'+index;
	variable_item_set_current_value_text(item, str);
}


static void exposure_change(VariableItem *item)
{
	struct fgp_app *fgp = variable_item_get_context(item);
	uint8_t index = variable_item_get_current_value_index(item);

	fgp_send_view_exposure_set(fgp->send_view, exposure_val[index]);
	variable_item_set_current_value_text(item, exposure_text[index]);
}

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
		view_dispatcher_send_custom_event(fgp->view_dispatcher, fgpViewSend);
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
				      9,
				      tail_len_change,
				      fgp);
	variable_item_set_current_value_index(item, 0);
	variable_item_set_current_value_text(item, "1");

	item = variable_item_list_add(fgp->variable_item_list,
				      list_text[1],
				      COUNT_OF(exposure_text),
				      exposure_change,
				      fgp);
	variable_item_set_current_value_index(item, 5);
	variable_item_set_current_value_text(item, exposure_text[5]);

	item = variable_item_list_add(fgp->variable_item_list,
				      list_text[2],
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
		view_dispatcher_switch_to_view(fgp->view_dispatcher, event.event);
		consumed = true;
	}
	return consumed;
}

void fgp_scene_send_conf_on_exit(void* context) {
	UNUSED(context);
}
