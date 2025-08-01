/* Declarations which are public-facing, but private. See internal.h for
 * declarations which are only used internally by vips and which are not
 * externally visible.
 *
 * 6/7/09
 * 	- from vips.h
 */

/*

	This file is part of VIPS.

	VIPS is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
	02110-1301  USA

 */

/*

	These files are distributed with VIPS - http://www.vips.ecs.soton.ac.uk

 */

#ifndef VIPS_PRIVATE_H
#define VIPS_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include <stdio.h>

#define VIPS_SPARE (8)

/* sizeof() a VIPS header on disc.
 */
#define VIPS_SIZEOF_HEADER (64)

/* What we track for each mmap window. Have a list of these on an openin
 * VipsImage.
 */
typedef struct {
	int ref_count;		   /* # of regions referencing us */
	struct _VipsImage *im; /* VipsImage we are attached to */

	int top; /* Area of image we have mapped, in pixels */
	int height;
	VipsPel *data; /* First pixel of line 'top' */

	void *baseaddr; /* Base of window */
	size_t length;	/* Size of window */
} VipsWindow;

VIPS_API
int vips_window_unref(VipsWindow *window);
VIPS_API
void vips_window_print(VipsWindow *window);

/* Per-thread buffer state. Held in a GPrivate.
 */
typedef struct {
	GHashTable *hash; /* VipsImage -> VipsBufferCache* */
	GThread *thread;  /* Just for sanity checking */
} VipsBufferThread;

/* Per-image buffer cache. This keeps a list of "done" VipsBuffer that this
 * worker has generated. We use this to reuse results within a thread.
 *
 * Hash to this from VipsBufferThread::hash.
 * We can't store the GSList directly in the hash table as GHashTable lacks an
 * update operation and we'd need to _remove() and _insert() on every list
 * operation.
 */
typedef struct _VipsBufferCache {
	GSList *buffers; /* GSList of "done" VipsBuffer* */
	GThread *thread; /* Just for sanity checking */
	struct _VipsImage *im;
	VipsBufferThread *buffer_thread;
	GSList *reserve; /* VipsBuffer kept in reserve */
	int n_reserve;	 /* Number in reserve */
} VipsBufferCache;

/* What we track for each pixel buffer. These can move between caches and
 * between threads, but not between images.
 *
 * Moving between threads is difficult, use region ownership stuff.
 */
typedef struct _VipsBuffer {
	int ref_count;		   /* # of regions referencing us */
	struct _VipsImage *im; /* VipsImage we are attached to */

	VipsRect area;			/* Area this pixel buffer covers */
	gboolean done;			/* Calculated and in a cache */
	VipsBufferCache *cache; /* The cache this buffer is published on */
	VipsPel *buf;			/* Private malloc() area */
	size_t bsize;			/* Size of private malloc() */
} VipsBuffer;

VIPS_API
void vips_buffer_dump_all(void);
VIPS_API
void vips_buffer_done(VipsBuffer *buffer);
VIPS_API
void vips_buffer_undone(VipsBuffer *buffer);
VIPS_API
void vips_buffer_unref(VipsBuffer *buffer);
VIPS_API
VipsBuffer *vips_buffer_new(struct _VipsImage *im, VipsRect *area);
VIPS_API
VipsBuffer *vips_buffer_ref(struct _VipsImage *im, VipsRect *area);
VIPS_API
VipsBuffer *vips_buffer_unref_ref(VipsBuffer *buffer,
	struct _VipsImage *im, VipsRect *area);
VIPS_API
void vips_buffer_print(VipsBuffer *buffer);

void vips__render_shutdown(void);

/* Sections of region.h that are private to VIPS.
 */

/* Region types.
 */
typedef enum _RegionType {
	VIPS_REGION_NONE,
	VIPS_REGION_BUFFER,		  /* A VipsBuffer */
	VIPS_REGION_OTHER_REGION, /* Memory on another region */
	VIPS_REGION_OTHER_IMAGE,  /* Memory on another image */
	VIPS_REGION_WINDOW		  /* A VipsWindow on fd */
} RegionType;

/* Private to iofuncs: the size of the `tiles' requested by
 * vips_image_generate() when acting as a data sink.
 */
#define VIPS__TILE_WIDTH (128)
#define VIPS__TILE_HEIGHT (128)

/* The height of the strips for the other two request styles.
 */
#define VIPS__THINSTRIP_HEIGHT (1)
#define VIPS__FATSTRIP_HEIGHT (16)

/* Functions on regions.
 */
struct _VipsRegion;
void vips__region_take_ownership(struct _VipsRegion *reg);
void vips__region_check_ownership(struct _VipsRegion *reg);
/* TODO(kleisauke): VIPS_API is required by vipsdisp.
 */
VIPS_API
void vips__region_no_ownership(struct _VipsRegion *reg);

typedef int (*VipsRegionFillFn)(struct _VipsRegion *, void *);
VIPS_API
int vips_region_fill(struct _VipsRegion *reg,
	const VipsRect *r, VipsRegionFillFn fn, void *a);

int vips__image_wio_output(struct _VipsImage *image);
int vips__image_pio_output(struct _VipsImage *image);

/* VIPS_ARGUMENT_FOR_ALL() needs to have this visible.
 */
VIPS_API
VipsArgumentInstance *vips__argument_get_instance(
	VipsArgumentClass *argument_class,
	VipsObject *object);
VipsArgument *vips__argument_table_lookup(VipsArgumentTable *table,
	GParamSpec *pspec);

/* im_demand_hint_array() needs to have this visible.
 */
#if VIPS_ENABLE_DEPRECATED
VIPS_API
#endif
void vips__demand_hint_array(struct _VipsImage *image,
	int hint, struct _VipsImage **in);
/* im_cp_desc_array() needs to have this visible.
 */
#if VIPS_ENABLE_DEPRECATED
VIPS_API
#endif
int vips__image_copy_fields_array(struct _VipsImage *out,
	struct _VipsImage *in[]);

void vips__region_count_pixels(struct _VipsRegion *region, const char *nickname);
VIPS_API
void vips_region_dump_all(void);

VIPS_API
int vips_region_prepare_many(struct _VipsRegion **reg, const VipsRect *r);

/* Handy for debugging.
 */
int vips__view_image(struct _VipsImage *image);

/* Pre 8.7 libvipses used this for allocating argument ids.
 */
VIPS_API
int _vips__argument_id;

// autoptr needs typed functions for autofree ... this needs to be in the
// public API since downstream projects can use our auto defs
VIPS_API
void VipsArrayDouble_unref(VipsArrayDouble *array);
VIPS_API
void VipsArrayImage_unref(VipsArrayImage *array);

extern gboolean vips__thread_profile;

void vips__thread_gate_start(const char *gate_name);
void vips__thread_gate_stop(const char *gate_name);

void vips__thread_malloc_free(gint64 size);

FILE *vips__file_open_read(const char *filename,
	const char *fallback_dir, gboolean text_mode);
FILE *vips__file_open_write(const char *filename,
	gboolean text_mode);

/* TODO(kleisauke): VIPS_API is required by vipsedit.
 */
VIPS_API
int vips__write(int fd, const void *buf, size_t count);

/* TODO(kleisauke): VIPS_API is required by test_connections.
 */
VIPS_API
int vips__open(const char *filename, int flags, int mode);

/* TODO(kleisauke): VIPS_API is required by vipsedit.
 */
VIPS_API
char *vips__file_read(FILE *fp, const char *name, size_t *length_out);

/* TODO(kleisauke): VIPS_API is required by the magick module.
 */
VIPS_API
gint64 vips__get_bytes(const char *filename, unsigned char buf[], gint64 len);

/* TODO(kleisauke): VIPS_API is required by vipsedit.
 */
VIPS_API
gint64 vips__seek(int fd, gint64 pos, int whence);

/* TODO(kleisauke): VIPS_API is required by libvips-cpp and vipsheader.
 */
VIPS_API
void vips__filename_split8(const char *name,
	char *filename, char *option_string);

/* TODO(kleisauke): VIPS_API is required by jpegsave_file_fuzzer.
 */
VIPS_API
char *vips__temp_name(const char *format);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*VIPS_PRIVATE_H*/
