#include <furi.h>
#include <gui/modules/submenu.h>
#include <storage/storage.h>
#include <src/include/fgp_app.h>
#include <src/scenes/include/fgp_scene.h>

#include <protocols/printer_proto.h>
#include <protocols/printer_receive.h>

#define PRINT	0x80000000
#define DONE	0x40000000

static void print_callback(void *context, void *buf, size_t len)
{
	struct fgp_app *fgp = context;

	/* If called with a NULL buffer, that means a print occurred, was
	 * completed by the printer side, and the printing side was given
	 * acknowledgment that the print was done.
	 */
	if (buf == NULL) {
		view_dispatcher_send_custom_event(fgp->view_dispatcher, DONE);
		return;
	}

	/* Copy the buffer from the printer to our local buffer to buy
	 * us more time.
	 */
	memcpy(fgp->data, buf, len);
	fgp->len = len;

	view_dispatcher_send_custom_event(fgp->view_dispatcher, PRINT);
}


static void scene_change_from_main_cb(void* context, uint32_t index) {
	UNUSED(context);
	UNUSED(index);
	struct fgp_app *fgp = context;
	UNUSED(fgp);

	/* Set scene state to the current index so we can have that element highlighted when
	* we return.
	*/
	//scene_manager_set_scene_state(fgp->scene_manager, fgpSceneMenu, index);

	//view_dispatcher_send_custom_event(fgp->view_dispatcher, index);
}

void fgp_scene_receive_on_enter(void* context)
{
	struct fgp_app *fgp = context;

	fgp->storage = furi_record_open(RECORD_STORAGE);

	submenu_reset(fgp->submenu);
	submenu_set_header(fgp->submenu, "Game Boy Printer");

	submenu_add_item(
	fgp->submenu,
	"Go!",
	fgpSceneReceive,
	scene_change_from_main_cb,
	fgp);

	submenu_set_selected_item(
	fgp->submenu,
	scene_manager_get_scene_state(fgp->scene_manager, fgpSceneMenu));

	printer_callback_context_set(fgp->printer_handle, fgp);
	printer_complete_callback_set(fgp->printer_handle, print_callback);
	printer_receive(fgp->printer_handle);

	view_dispatcher_switch_to_view(fgp->view_dispatcher, fgpViewSubmenu);
}

bool fgp_scene_receive_on_event(void* context, SceneManagerEvent event)
{
	struct fgp_app *fgp = context;
	File *file = NULL;
	FuriString *path = NULL;
	bool consumed = false;

	if (event.type == SceneManagerEventTypeCustom) {
		if (event.event == PRINT) {
			file = storage_file_alloc(fgp->storage);
			path = furi_string_alloc_set(APP_DATA_PATH("fgp-image.bin"));
			storage_common_resolve_path_and_ensure_app_directory(fgp->storage, path);
			storage_file_open(file, furi_string_get_cstr(path), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS);
			storage_file_write(file, fgp->data, fgp->len);
			storage_file_free(file);
			furi_string_free(path);

			printer_receive_print_complete(fgp->printer_handle);

			consumed = true;
		}
		if (event.event == DONE) {
			scene_manager_previous_scene(fgp->scene_manager);
			consumed = true;
		}
	}

	return consumed;
}

void fgp_scene_receive_on_exit(void* context) {
	struct fgp_app *fgp = context;
	furi_record_close(RECORD_STORAGE);
	printer_stop(fgp->printer_handle);
}
