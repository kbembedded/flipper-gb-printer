// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#include <furi.h>
#include <gui/modules/submenu.h>
#include <storage/storage.h>
#include <src/include/fgp_app.h>
#include <src/scenes/include/fgp_scene.h>

// gblink protocol support
#include <protocols/printer/include/printer_proto.h>
#include <protocols/printer/include/printer_send.h>

#include <src/include/file_handling.h>
#include <src/include/png.h>
#include <src/include/tile_tools.h>

#include "nyanphin_head.h"
#include "nyanphin_tail.h"
#include "nyanphin_tail_signed.h"

/* XXX: TODO turn this in to an enum */
#define LINE_XFER		0x80000000
#define PRINT		0x40000000
#define COMPLETE	0x20000000

struct send_model {
	int percent;
};

struct send_ctx {
	View *view;
	struct send_model *model;
	struct fgp_app *fgp;
	FuriTimer *timer;
	ViewDispatcher *view_dispatcher;

	// Printer handling
	void *printer_handle;

	struct gb_image *image;

	uint8_t exposure;
	uint8_t tail_len;
};

static void fgp_send_view_timer(void *context)
{
	struct send_ctx *ctx = context;

	with_view_model(ctx->view,
			struct send_model * model,
			{ UNUSED(model); },
			true);
}

static void printer_callback(void *context, struct gb_image *image, enum cb_reason reason)
{
	UNUSED(image);
	struct send_ctx *ctx = context;

	FURI_LOG_D("printer", "printer_callback reason %d", (int)reason);
	if (reason == reason_complete)
		view_dispatcher_send_custom_event(ctx->view_dispatcher, COMPLETE);
}


static bool fgp_send_view_event(uint32_t event, void *context)
{
	UNUSED(context);
	//struct send_ctx *ctx = context;
	//struct gb_image *image = ctx->image;

	if (event == COMPLETE)
		FURI_LOG_D("send", "completed");

	return true;
}

static void fgp_send_view_enter(void *context)
{
	struct send_ctx *ctx = context;

	view_allocate_model(ctx->view, ViewModelTypeLockFree, sizeof(struct send_model));

	/* XXX: TODO: Startup of this takes a long time. Presumably, it is all
	 * of the storage allocation, directory creation, finding the latest
	 * file number, etc. Because this runs at a higher priority than the
	 * timer callback, setting up the draw timer first has no impact on
	 * the issue.
	 * I believe draw calls are also in the same context as this thread,
	 * so I don't think there is a good way to issue a draw callback here.
	 * Need to figure out a better way to handle this setup.
	 */
	ctx->image = malloc(sizeof(struct gb_image));
	ctx->printer_handle = ctx->fgp->printer_handle;

	printer_callback_context_set(ctx->printer_handle, ctx);
	printer_callback_set(ctx->printer_handle, printer_callback);

	ctx->timer = furi_timer_alloc(fgp_send_view_timer, FuriTimerTypePeriodic, ctx);
	furi_timer_start(ctx->timer, furi_ms_to_ticks(200));

	// XXX: Nice and hacky!
	scanline_to_tile(ctx->image->data, nyanphin_head_bin, 20, 18);
	ctx->image->data_sz = nyanphin_head_bin_len;
	ctx->image->margins = 0x10; // Header, no footer for first image
	ctx->image->num_sheets = 1;
	ctx->image->exposure = ctx->exposure;
	printer_send_start(ctx->printer_handle, ctx->image);

	// Next, the unsigned tail, if any
	if (ctx->tail_len > 1) {
		scanline_to_tile(ctx->image->data, nyanphin_tail_bin, 20, 18);
		ctx->image->data_sz = nyanphin_tail_bin_len;
		ctx->image->margins = 0x00; // No header or footer for these
		ctx->image->num_sheets = ctx->tail_len - 1;
		ctx->image->exposure = ctx->exposure;
		printer_send_start(ctx->printer_handle, ctx->image);
	}

	// Last, the signed tail
	scanline_to_tile(ctx->image->data, nyanphin_tail_signed_bin, 20, 18);
	ctx->image->data_sz = nyanphin_tail_signed_bin_len;
	ctx->image->margins = 0x03; // No header, but must have a footer of at least 3
	ctx->image->num_sheets = 1;
	ctx->image->exposure = ctx->exposure;
	printer_send_start(ctx->printer_handle, ctx->image);
}

static void fgp_send_view_exit(void *context)
{
	struct send_ctx *ctx = context;

	furi_timer_free(ctx->timer);

	printer_stop(ctx->printer_handle);

	free(ctx->image);
	view_free_model(ctx->view);
}


static bool fgp_send_view_input(InputEvent *event, void *context)
{
	UNUSED(event);
	UNUSED(context);
	bool ret = false;

	return ret;
}

static void fgp_send_view_draw(Canvas *canvas, void* view_model)
{
	struct send_model *model = view_model;
	char string[26];
	snprintf(string, sizeof(string), "%s %d", "Printing...", model->percent);
	canvas_draw_str(canvas, 18, 13, string);
}

View *fgp_send_view_get_view(void *context)
{
	struct send_ctx *ctx = context;
	return ctx->view;
}

void *fgp_send_view_alloc(struct fgp_app *fgp)
{
	struct send_ctx *ctx = malloc(sizeof(struct send_ctx));

	ctx->view_dispatcher = fgp->view_dispatcher;
	ctx->fgp = fgp;

	ctx->view = view_alloc();
	view_set_context(ctx->view, ctx);

	view_set_input_callback(ctx->view, fgp_send_view_input);
	view_set_draw_callback(ctx->view, fgp_send_view_draw);
	view_set_enter_callback(ctx->view, fgp_send_view_enter);
	view_set_exit_callback(ctx->view, fgp_send_view_exit);
	view_set_custom_callback(ctx->view, fgp_send_view_event);

	return ctx;
}

void fgp_send_view_exposure_set(void *send_ctx, uint8_t val)
{
	struct send_ctx *ctx = send_ctx;

	// Max exposure value is 0x7f
	furi_check(val <= 0x7f);

	ctx->exposure = val;
}

void fgp_send_view_tail_len_set(void *send_ctx, uint8_t val)
{
	struct send_ctx *ctx = send_ctx;

	// So long as the tail is at least one in length its fine
	furi_check(val > 0);

	ctx->tail_len = val;
}

void fgp_send_view_free(void *send_ctx)
{
	struct send_ctx *ctx = send_ctx;

	view_free(ctx->view);
	free(ctx);
}
