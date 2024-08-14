#include <furi.h>

#include <gui/scene_manager.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>

#include <src/include/fgp_app.h>
#include <src/scenes/include/fgp_scene.h>

#include <gblink.h>

bool fgp_custom_event_callback(void* context, uint32_t event)
{
	furi_assert(context);
	struct fgp_app *fgp = context;

	return scene_manager_handle_custom_event(fgp->scene_manager, event);
}

bool fgp_back_event_callback(void* context)
{
	furi_assert(context);
	struct fgp_app *fgp = context;
	return scene_manager_handle_back_event(fgp->scene_manager);
}


static struct fgp_app *fgp_alloc(void)
{
	struct fgp_app *fgp = NULL;

	fgp = malloc(sizeof(struct fgp_app));

	// View Dispatcher
	fgp->view_dispatcher = view_dispatcher_alloc();
	view_dispatcher_enable_queue(fgp->view_dispatcher);
	view_dispatcher_set_event_callback_context(fgp->view_dispatcher, fgp);
	view_dispatcher_set_custom_event_callback(fgp->view_dispatcher, fgp_custom_event_callback);
	view_dispatcher_set_navigation_event_callback(fgp->view_dispatcher, fgp_back_event_callback);
	view_dispatcher_attach_to_gui(fgp->view_dispatcher,
				      (Gui*)furi_record_open(RECORD_GUI),
				      ViewDispatcherTypeFullscreen);

	// Submenu
	fgp->submenu = submenu_alloc();
	view_dispatcher_add_view(fgp->view_dispatcher, fgpViewSubmenu, submenu_get_view(fgp->submenu));
	
	// Scene manager
	fgp->scene_manager = scene_manager_alloc(&fgp_scene_handlers, fgp);
	scene_manager_next_scene(fgp->scene_manager, fgpSceneMenu);

	fgp->gblink_handle = gblink_alloc();

	return fgp;
}

static void fgp_free(struct fgp_app *fgp)
{
	gblink_free(fgp->gblink_handle);

	// submenu
	view_dispatcher_remove_view(fgp->view_dispatcher, fgpViewSubmenu);
	submenu_free(fgp->submenu);

	free(fgp);
}

int32_t fgp_app(void* p)
{
	UNUSED(p);
	struct fgp_app *fgp = NULL;

	fgp = fgp_alloc();

	view_dispatcher_run(fgp->view_dispatcher);

	fgp_free(fgp);

	return 0;
}
