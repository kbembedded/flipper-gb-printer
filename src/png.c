// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2024 KBEmbedded

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <src/include/crc.h>
#include <src/include/fgp_palette.h>

/* XXX TODO NOTE:
 * the use of bswap32 is fine, but, should port some form of htonl and ntohl to
 * ensure future portability.
 */

struct __attribute__((__packed__)) png_hdr {
	uint8_t magic[8]; // 137 80 78 71 13 10 26 10
};

/* Must always be followed by a CRC of the data, and type! */
struct __attribute__((__packed__)) ihdr {
	uint32_t data_len; // len is just the data itself
	uint8_t type[4]; // Usually represented in ASCII
	uint32_t width;
	uint32_t height;
	uint8_t bit_depth;
	uint8_t color_type;
	uint8_t comp_method;
	uint8_t filter_method;
	uint8_t interlace_method;
};

/* Must always be followed by a CRC of the data, and type! */
struct __attribute__((__packed__)) plte {
	uint32_t data_len; // len is just the data itself
	uint8_t type[4]; // Usually represented in ASCII
	uint8_t color[4][3];
};

/* Must always be followed by a CRC of the data, and type! */
struct __attribute__((__packed__)) idat {
	uint32_t data_len; // len is just the data itself
	uint8_t type[4]; // Usually represented in ASCII
	uint8_t zlib_flags; // Should always be 0x78, 32k window size, DEFLATE
	uint8_t zlib_addl_flags; // Should always be 0x01, indicate no compression (unused for decomp), no dictionary, check bits for flags
	uint8_t deflate_btype; // Should always be 0x01, I think this says no compression?
	uint16_t len; // Len of image data, includes filter type
	uint16_t nlen; // 1s compliment of len
	uint8_t image[]; // Finally, the actual image data!
};

/* Must always be followed by a CRC of the data, and type! */
struct __attribute__((__packed__)) iend {
	uint32_t data_len; // len is just the data itself
	uint8_t type[4]; // Usually represented in ASCII
};

struct __attribute__((__packed__)) image {
	struct png_hdr header;
	struct ihdr ihdr;
	uint32_t ihdr_crc;
	struct plte plte;
	uint32_t plte_crc;
	struct idat idat;
};

/* Separately keep the ending data so the main image struct can be variable
 * size more easily. The data can be created static on the stack, and then
 * appended to the end of the image which was allocated long enough. */
struct __attribute__((__packed__)) image_footer {
	uint32_t check_data; // zlib check data, not big endian?
	uint32_t idat_crc;
	struct iend iend;
	uint32_t iend_crc;
};

struct png_handle {
	struct image *image;
	/* The footer will actually lie inside the full image length,
	 * which is allocated for struct image + image data.
	 * The footer on the stack is the real data, that we keep, and
	 * copy to the start of footer inside image.
	 */
	struct image_footer *footer_start;
	uint32_t height;
	uint32_t width;
	uint32_t image_len; // Just the image data
	uint32_t total_len; // This is the full image data from header to file end
};

static const struct png_hdr png_hdr_data = {
	.magic = { 137, 80, 78, 71, 13, 10, 26, 10},
};

static const struct ihdr ihdr_data = {
	.data_len = 218103808, // (uint32_t)__builtin_bswap32(13)
	.type = { 'I', 'H', 'D', 'R' },
	.width = 0, // Placeholder
	.height = 0, // Placeholder
	.bit_depth = 2, // Always 2bpp
	.color_type = 3, // Always indexed mode
	.comp_method = 0, // always 0
	.filter_method = 0, // No filters
	.interlace_method = 0, // No interlacing
};

static const struct plte plte_data = {
	.data_len = 201326592, // (uint32_t)__builtin_bswap32(12)
	.type = { 'P', 'L', 'T', 'E' },
};

static const struct idat idat_data = {
	.data_len = 0, // len is just the data itself
	.type = { 'I', 'D', 'A', 'T' }, // Usually represented in ASCII
	.zlib_flags = 0x58, // Should always be 0x58, 8k window size, DEFLATE
	.zlib_addl_flags = 0x09, // Should always be 0x09, indicate no compression (unused for decomp), no dictionary, check bits for flags
	.deflate_btype = 0x01, // This should be 0x01, no compression?
	.len = 0, // Len of image data, includes filter type
	.nlen = 0, // 1s compliment of len
};

static const struct iend iend_data = {
	.data_len = 0,
	.type = { 0x49, 0x45, 0x4e, 0x44 },
};

static const uint32_t iend_crc = 0x826042ae;

void png_palette_set(void *png_handle, uint8_t rgb[4][3])
{
	struct png_handle *png = png_handle;
	memcpy(&png->image->plte.color, rgb, 12); // This is a constant size
	png->image->plte_crc = __builtin_bswap32(crc(png->image->plte.type, __builtin_bswap32(png->image->plte.data_len) + 4));
}

void png_palette_set_Palette(void *png_handle, const Palette palette)
{
	struct png_handle *png = png_handle;

	png->image->plte.color[0][0] = (palette.palette_color_hex_a >> 16) & 0xFF;
	png->image->plte.color[0][1] = (palette.palette_color_hex_a >> 8) & 0xFF;
	png->image->plte.color[0][2] = palette.palette_color_hex_a & 0xFF;

	png->image->plte.color[1][0] = (palette.palette_color_hex_b >> 16) & 0xFF;
	png->image->plte.color[1][1] = (palette.palette_color_hex_b >> 8) & 0xFF;
	png->image->plte.color[1][2] = palette.palette_color_hex_b & 0xFF;

	png->image->plte.color[2][0] = (palette.palette_color_hex_c >> 16) & 0xFF;
	png->image->plte.color[2][1] = (palette.palette_color_hex_c >> 8) & 0xFF;
	png->image->plte.color[2][2] = palette.palette_color_hex_c & 0xFF;

	png->image->plte.color[3][0] = (palette.palette_color_hex_d >> 16) & 0xFF;
	png->image->plte.color[3][1] = (palette.palette_color_hex_d >> 8) & 0xFF;
	png->image->plte.color[3][2] = palette.palette_color_hex_d & 0xFF;

	png->image->plte_crc = __builtin_bswap32(crc(png->image->plte.type, __builtin_bswap32(png->image->plte.data_len) + 4));
}

void png_populate(void *png_handle, uint8_t *image_buf)
{
	struct png_handle *png = png_handle;
	uint8_t *image_ptr = png->image->idat.image;
	uint32_t a = 1, b = 0;
	uint32_t i;

	for (i = 0; i < png->height; i++) {
		/* Each row needs to begin with a 0 byte to indicate no filter
		 * for that row.
		 */
		*image_ptr = 0x00;
		image_ptr++;
		memcpy(image_ptr, image_buf, 40); // The magic number is that each row is 40 bytes long
		image_ptr += 40;
		image_buf += 40;
	}

	/* Calculate zlib adler32 */
	image_ptr = png->image->idat.image;
	for (i = 0; i < png->image_len; i++) {
		a = (a + image_ptr[i]) % 65521;
		b = (b + a) % 65521;
	}
	png->footer_start->check_data = (__builtin_bswap32((b << 16) | a));

	/* data_len includes additional zlib header/footer */
	png->footer_start->idat_crc = __builtin_bswap32(crc(png->image->idat.type, __builtin_bswap32(png->image->idat.data_len) + 4));
}

void *png_reset(void *png_handle)
{
	struct png_handle *png = png_handle;
	struct image *image = png->image;

	memset(image, '\0', png->total_len);

	/* Copy in static data bits */
	memcpy(&image->header, &png_hdr_data, sizeof(struct png_hdr));
	memcpy(&image->ihdr, &ihdr_data, sizeof(struct ihdr));
	memcpy(&image->plte, &plte_data, sizeof(struct plte));
	png_palette_set(png_handle, palette_rgb16_get(0));
	memcpy(&image->idat, &idat_data, sizeof(struct idat));
	memcpy(&png->footer_start->iend, &iend_data, sizeof(struct iend));
	png->footer_start->iend_crc = iend_crc;

	/* Start adding in dynamic data */
	image->ihdr.width = __builtin_bswap32(png->width);
	image->ihdr.height = __builtin_bswap32(png->height);
	image->idat.len = png->image_len;
	image->idat.nlen = ~(png->image_len);
	image->idat.data_len = __builtin_bswap32(png->image_len + 11); // 12 is zlib header/footer size

	/* Calculate CRCs */
	image->ihdr_crc = __builtin_bswap32(crc(image->ihdr.type, __builtin_bswap32(image->ihdr.data_len) + 4));

	return png;
}

/* Allocates pixel array */
void *png_alloc(uint32_t width, uint32_t height)
{
	struct png_handle *png = NULL;

	/* Allocate the data we will need.
	 */
	png = malloc(sizeof(struct png_handle));
	png->width = width;
	png->height = height;

	/* Since image data is 4 px per bit, the image length is (height*width/4),
	 * and, PNG has a byte starting each "scanline" (each row), so add in
	 * one more lump of height.
	 */
	png->image_len = (height*width/4) + height;
	/* The toal length is the image struct and footer. Both of which are the
	 * complete amount of data needed to express everything except the image
	 * data itself; plus the actual image data.
	 */
	png->total_len = sizeof(struct image) + sizeof(struct image_footer) + png->image_len;
	png->image = malloc(png->total_len);
	png->footer_start = ((void *)(png->image) + sizeof(struct image) + png->image_len);

	png_reset(png);

	return png;
}

void png_free(void *png_handle)
{
	struct png_handle *png = png_handle;
	free(png->image);
	free(png);
}

size_t png_len_get(void *png_handle)
{
	struct png_handle *png = png_handle;
	return png->total_len;
}

uint8_t *png_buf_get(void *png_handle)
{
	struct png_handle *png = png_handle;
	return (uint8_t *)png->image;
}
