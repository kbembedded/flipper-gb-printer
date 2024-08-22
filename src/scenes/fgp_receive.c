#include <furi.h>
#include <gui/modules/submenu.h>
#include <storage/storage.h>
#include <src/include/fgp_app.h>
#include <src/scenes/include/fgp_scene.h>

#include <protocols/printer_proto.h>
#include <protocols/printer_receive.h>

/* XXX: TODO turn this in to an enum */
#define DATA		0x80000000
#define PRINT		0x40000000
#define COMPLETE	0x20000000

static void printer_callback(void *context, void *buf, size_t len, enum cb_reason reason)
{
	struct fgp_app *fgp = context;
	
	FURI_LOG_D("printer", "printer_callback reason %d", (int)reason);

	switch (reason) {
	case reason_data:
		break;
	case reason_print:
		/* Copy the buffer from the printer to our local buffer to buy
		 * us more time.
		 */
		memcpy(fgp->data, buf, len);
		fgp->len = len;

		view_dispatcher_send_custom_event(fgp->view_dispatcher, PRINT);
		break;
	case reason_complete:
		view_dispatcher_send_custom_event(fgp->view_dispatcher, COMPLETE);
		break;
	}

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
	printer_callback_set(fgp->printer_handle, printer_callback);
	printer_receive(fgp->printer_handle);

	view_dispatcher_switch_to_view(fgp->view_dispatcher, fgpViewSubmenu);
}

bool fgp_scene_receive_on_event(void* context, SceneManagerEvent event)
{
	struct fgp_app *fgp = context;
	File *file = NULL;
	FuriString *path = NULL;
	FuriString *basename = NULL;
	bool consumed = false;

	if (event.type == SceneManagerEventTypeCustom) {
		if (event.event == PRINT) {
			basename = furi_string_alloc();
			file = storage_file_alloc(fgp->storage);

			path = furi_string_alloc_set(APP_DATA_PATH(""));
			storage_get_next_filename(fgp->storage, APP_DATA_PATH(), "GC_", ".bin", basename, 20);
			furi_string_cat(path, basename);
			furi_string_cat_str(path, ".bin");

			storage_common_resolve_path_and_ensure_app_directory(fgp->storage, path);
			FURI_LOG_D("printer", "Using file %s", furi_string_get_cstr(path));
			storage_file_open(file, furi_string_get_cstr(path), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS);
			storage_file_write(file, "GB-BIN01", 8);
			storage_file_write(file, fgp->data, fgp->len);
			storage_file_free(file);
			furi_string_free(path);
			furi_string_free(basename);

			printer_receive_print_complete(fgp->printer_handle);

			consumed = true;
		}
		if (event.event == COMPLETE) {
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
