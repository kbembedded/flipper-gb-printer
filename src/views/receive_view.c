// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#include <furi.h>
#include <gui/modules/submenu.h>
#include <storage/storage.h>
#include <src/include/fgp_app.h>
#include <src/scenes/include/fgp_scene.h>

#include <protocols/printer_proto.h>
#include <protocols/printer_receive.h>

#include <src/helpers/bmp.h>

/* XXX: TODO turn this in to an enum */
#define DATA		0x80000000
#define PRINT		0x40000000
#define COMPLETE	0x20000000

#define WIDTH 160L
#define HEIGHT 144L

struct recv_model {
	int count;
	int converted;
};

struct recv_ctx {
	View *view;
	struct recv_model *model;
	struct fgp_app *fgp;
	FuriTimer *timer;
	ViewDispatcher *view_dispatcher;
	
	// Printer handling
	void *gblink_handle;
	void *printer_handle;

	struct gb_image *volatile_image;
	struct gb_image *image;
	int packet_cnt;

	// File operations
	Storage *storage;
};

static void fgp_receive_view_timer(void *context)
{
	struct recv_ctx *ctx = context;

	with_view_model(ctx->view,
			struct recv_model * model,
			{ UNUSED(model); },
			true);
}

static void printer_callback(void *context, struct gb_image *image, enum cb_reason reason)
{
	struct recv_ctx *ctx = context;
	
	FURI_LOG_D("printer", "printer_callback reason %d", (int)reason);

	switch (reason) {
	case reason_data:
		ctx->packet_cnt++;
		view_dispatcher_send_custom_event(ctx->view_dispatcher, DATA);
		break;
	case reason_print:
		/* TODO: XXX: 
		 * Interleave prints and processing here.
		 * For the first photo, its safe to mark this as "printed" immediately
		 * after copying the buffer. This lets the next print start quickly, e.g.
		 * for doing a print all from Photo! or other prints that may take multiple
		 * prints like panoramics.
		 * Once the next photo comes in, we need to check if the save process is
		 * still running, if not, then repeat above. If so, then I don't know yet.
		 * Need to put this somewhere, somehow, maybe have a custom event re-call
		 * this?
		 * Set up a return.
		 * Hmm.
		 * This logic might need to move to the event handler, not here. In that
		 * case, we probably need to double buffer. So if the first print is not
		 * yet done, the second one can be copied here, and the event can, do something
		 * with that and hold off on marking it printed.
		 */
		ctx->volatile_image = image;
		view_dispatcher_send_custom_event(ctx->view_dispatcher, PRINT);
		break;
	case reason_complete:
		view_dispatcher_send_custom_event(ctx->view_dispatcher, COMPLETE);
		break;
	}

}


static bool fgp_receive_view_event(uint32_t event, void *context)
{
	struct recv_ctx *ctx = context;
	struct gb_image *image = ctx->image;
	File *file = NULL;
	FuriString *path = NULL;
	FuriString *basename = NULL;
	FuriString *path_bmp = NULL;
	bool consumed = false;

	if (event == DATA)
		consumed = true;

	if (event == PRINT) {
		/* Copy the buffer from the printer data here, and then tell the
		 * printer it can continue receiving.
		 */
		/* XXX: TODO: NOTE: I believe this is safe as I don't think this
		 * event callback handle will be called again until this completes.
		 * I need to verify which thread calls this and how the queue is
		 * managed.
		 */
		printer_image_buffer_copy(image, ctx->volatile_image);
		printer_receive_print_complete(ctx->printer_handle);
		with_view_model(ctx->view,
				struct recv_model * model,
				{ model->count++; },
				false);

		basename = furi_string_alloc();
		file = storage_file_alloc(ctx->storage);

		//	Save Binary
		path = furi_string_alloc_set(APP_DATA_PATH(""));
		storage_get_next_filename(ctx->storage, APP_DATA_PATH(), "GC_", ".bin", basename, 20);
		furi_string_cat(path, basename);
		furi_string_cat_str(path, ".bin");

		/* XXX: Stop using this, time to complete this increases as there are more files.
		 * Probably best to key file names on date, or seconds from epoc, or something.
		 * Maybe let that be flexible?
		 */
		storage_common_resolve_path_and_ensure_app_directory(ctx->storage, path);
		FURI_LOG_D("printer", "Using file %s", furi_string_get_cstr(path));
		storage_file_open(file, furi_string_get_cstr(path), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS);
		storage_file_write(file, "GB-BIN01", 8);
		storage_file_write(file, image->data, image->data_sz);
		storage_file_free(file);
		furi_string_free(path);
		furi_string_free(basename);

		//	Save Bmp Picture
		File *bmp_file = storage_file_alloc(ctx->storage);
		basename = furi_string_alloc();
		path_bmp = furi_string_alloc_set_str(APP_DATA_PATH(""));
		storage_get_next_filename(ctx->storage, APP_DATA_PATH(), "GC_", ".bmp", basename, 20);
		furi_string_cat_str(path_bmp, furi_string_get_cstr(basename));
		furi_string_cat_str(path_bmp, ".bmp");

		storage_common_resolve_path_and_ensure_app_directory(ctx->storage, path_bmp);

		// Abre el archivo BMP para escritura
		static char bmp[BMP_SIZE(WIDTH, HEIGHT)];
		bmp_init(bmp, WIDTH, HEIGHT);
		
		//  Palette
		//	TODO: Agregar selector de paletas
		uint32_t palette[] = {
			bmp_encode(0xFFFFFF),
			bmp_encode(0xAAAAAA),
			bmp_encode(0x555555),
			bmp_encode(0x000000)};
		storage_file_open(bmp_file, furi_string_get_cstr(path_bmp), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS);
		uint8_t tile_data[16];
		for(int y = 0; y < HEIGHT / 8; y++) {
			for(int x = 0; x < WIDTH / 8; x++) {
				int tile_index = (y * (WIDTH / 8) + x) * 16; // 16 bytes por tile
				memcpy(tile_data, &((uint8_t*)image->data)[tile_index], 16);
				for(int row = 0; row < 8; row++) {
					uint8_t temp1 = tile_data[row * 2];
					uint8_t temp2 = tile_data[row * 2 + 1];

					for(int pixel = 7; pixel >= 0; pixel--) {
						bmp_set(
							bmp,
							(x * 8) + pixel,
							(y * 8) + row,
							palette[((temp1 & 1) + ((temp2 & 1) * 2))]
						);
						temp1 >>= 1;
						temp2 >>= 1;
					}   
				}
			}
		}
		// Finalizar la imagen BMP
		storage_file_write(bmp_file, bmp, sizeof(bmp));
		storage_file_free(bmp_file);

		furi_string_free(path_bmp);
		furi_string_free(basename);

		with_view_model(ctx->view,
				struct recv_model * model,
				{ model->converted++; },
				false);

		consumed = true;
	}

	return consumed;
}

static void fgp_receive_view_enter(void *context)
{
	struct recv_ctx *ctx = context;

	view_allocate_model(ctx->view, ViewModelTypeLockFree, sizeof(struct recv_model));

	ctx->printer_handle = printer_alloc(ctx->gblink_handle);
	ctx->image = printer_image_buffer_alloc();

	ctx->storage = furi_record_open(RECORD_STORAGE);

	printer_callback_context_set(ctx->printer_handle, ctx);
	printer_callback_set(ctx->printer_handle, printer_callback);
	printer_receive_start(ctx->printer_handle);

	ctx->timer = furi_timer_alloc(fgp_receive_view_timer, FuriTimerTypePeriodic, ctx);
	furi_timer_start(ctx->timer, furi_ms_to_ticks(200));
}

static void fgp_receive_view_exit(void *context)
{
	struct recv_ctx *ctx = context;

	furi_record_close(RECORD_STORAGE);
	furi_timer_free(ctx->timer);

	printer_stop(ctx->printer_handle);
	printer_free(ctx->printer_handle);

	printer_image_buffer_free(ctx->image);
	view_free_model(ctx->view);
}


static bool fgp_receive_view_input(InputEvent *event, void *context)
{
	UNUSED(event);
	UNUSED(context);
	bool ret = false;

	return ret;
}

static void fgp_receive_view_draw(Canvas *canvas, void* view_model)
{
	struct recv_model *model = view_model;
	char string[26];
	snprintf(string, sizeof(string), "Received: %d", model->count);
	canvas_draw_str(canvas, 18, 13, string);
	snprintf(string, sizeof(string), "Converted to BMP: %d", model->converted);
	canvas_draw_str(canvas, 18, 21, string);
}

View *fgp_receive_view_get_view(void *context)
{
	struct recv_ctx *ctx = context;
	return ctx->view;
}

void *fgp_receive_view_alloc(struct fgp_app *fgp)
{
	struct recv_ctx *ctx = malloc(sizeof(struct recv_ctx));

	ctx->gblink_handle = fgp->gblink_handle;
	ctx->view_dispatcher = fgp->view_dispatcher;

	ctx->view = view_alloc();
	view_set_context(ctx->view, ctx);

	view_set_input_callback(ctx->view, fgp_receive_view_input);
	view_set_draw_callback(ctx->view, fgp_receive_view_draw);
	view_set_enter_callback(ctx->view, fgp_receive_view_enter);
	view_set_exit_callback(ctx->view, fgp_receive_view_exit);
	view_set_custom_callback(ctx->view, fgp_receive_view_event);

	return ctx;
}

void fgp_receive_view_free(void *recv_ctx)
{
	struct recv_ctx *ctx = recv_ctx;

	view_free(ctx->view);
	free(ctx);
}
