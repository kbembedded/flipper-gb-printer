#include <furi.h>
#include <furi_hal_rtc.h>
#include <datetime/datetime.h>
#include <storage/storage.h>

struct fgp_storage {
	Storage *storage;
	File *file;
	FuriString *base_path;
	FuriString *file_name;
	uint16_t count;
};

/* XXX: TODO: This doesn't check for exceeding 9999 */
void fgp_storage_next_count(void *fgp_storage)
{
	struct fgp_storage *storage = fgp_storage;

	storage->count++;
};

/* TODO: Add a tell() function */

/* True if file opened successfully */
bool fgp_storage_open(void *fgp_storage, const char *extension)
{
	struct fgp_storage *storage = fgp_storage;
	bool ret = false;

	FuriString *fs_tmp = furi_string_alloc_printf("%s/%s%04d%s", furi_string_get_cstr(storage->base_path),
								 furi_string_get_cstr(storage->file_name),
								 storage->count,
								 extension);

	ret = storage_file_open(storage->file,
				furi_string_get_cstr(fs_tmp),
				FSAM_WRITE,
				FSOM_OPEN_APPEND);

	furi_string_free(fs_tmp);

	return ret;
}

size_t fgp_storage_write(void *fgp_storage, const void *buf, size_t len)
{
	struct fgp_storage *storage = fgp_storage;
	return storage_file_write(storage->file, buf, len);
}

bool fgp_storage_close(void *fgp_storage)
{
	struct fgp_storage *storage = fgp_storage;
	return storage_file_close(storage->file);
}

bool fgp_storage_seek(void *fgp_storage, off_t offs, bool from_start)
{
	struct fgp_storage *storage = fgp_storage;

	if (from_start)
		return storage_file_seek(storage->file, (uint32_t)offs, true);
	else
		return storage_file_seek(storage->file,
					 (uint32_t)(storage_file_tell(storage->file) + offs),
					 true);
}

void fgp_storage_free(void *fgp_storage)
{
	struct fgp_storage *storage = fgp_storage;

	/* Before closing all of our file handles, try and delete the directory
	 * we created earlier, if its empty it will remove, it if files were added
	 * to it, it will remain.
	 */
	storage_simply_remove(storage->storage, furi_string_get_cstr(storage->base_path));

	/* Close storage and file records */
	storage_file_free(storage->file);
	furi_record_close(RECORD_STORAGE);

	/* Close strings */
	furi_string_free(storage->base_path);
	furi_string_free(storage->file_name);

	free(storage);
}

/* Extension is used for making a full file */
void *fgp_storage_alloc(char *file_prefix, char *extension)
{
	struct fgp_storage *storage = malloc(sizeof(struct fgp_storage));
	DateTime date = {0};
	FuriString *fs_tmp = furi_string_alloc();
	uint16_t num = 0;

	storage->storage = furi_record_open(RECORD_STORAGE);
	storage->file = storage_file_alloc(storage->storage);

	storage->file_name = furi_string_alloc_set(file_prefix);

	/* Set up base file path */
	storage->base_path = furi_string_alloc_set(APP_DATA_PATH(""));
	storage_common_resolve_path_and_ensure_app_directory(storage->storage, storage->base_path);

	/* Get today's date */
	furi_hal_rtc_get_datetime(&date);

	/* Build directory name based on date */
	furi_string_cat_printf(storage->base_path, "%d-%02d-%02d", date.year, date.month, date.day);
	/* TODO: Error checking ? */

	/* Make directory if it doesn't exist */
	storage_simply_mkdir(storage->storage, furi_string_get_cstr(storage->base_path));

	/* Check for most recent file number */
	/* This reimplements storage_get_next_filename() so we can just get the
	 * number and not a string of it.
	 */
	do {
		furi_string_printf(fs_tmp, "%s/%s%04d%s", furi_string_get_cstr(storage->base_path),
							  furi_string_get_cstr(storage->file_name),
							  num++,
							  extension);
	} while (storage_common_stat(storage->storage, furi_string_get_cstr(fs_tmp), NULL) == FSE_OK);
	storage->count = --num;
	/* HACK: 9999 is the max count we allow, its arbitrary, but 4 digits of
	 * photo numbering is pretty common. If we hit it, return null after
	 * free()ing everything.
	 */
	if (storage->count > 9999) {
		fgp_storage_free(storage);
		storage = NULL;
	}

	furi_string_free(fs_tmp);

	return storage;
}
