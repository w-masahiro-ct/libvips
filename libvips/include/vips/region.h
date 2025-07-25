/* Definitions for partial image regions.
 *
 * J.Cupitt, 8/4/93
 *
 * 2/3/11
 * 	- move to GObject
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

#ifndef VIPS_REGION_H
#define VIPS_REGION_H

#include <glib.h>
#include <glib-object.h>
#include <vips/object.h>
#include <vips/image.h>
#include <vips/rect.h>
#include <vips/private.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#define VIPS_TYPE_REGION (vips_region_get_type())
#define VIPS_REGION(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), \
		VIPS_TYPE_REGION, VipsRegion))
#define VIPS_REGION_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), \
		VIPS_TYPE_REGION, VipsRegionClass))
#define VIPS_IS_REGION(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), VIPS_TYPE_REGION))
#define VIPS_IS_REGION_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), VIPS_TYPE_REGION))
#define VIPS_REGION_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), \
		VIPS_TYPE_REGION, VipsRegionClass))

/**
 * VipsRegionShrink:
 * @VIPS_REGION_SHRINK_MEAN: use the average
 * @VIPS_REGION_SHRINK_MEDIAN: use the median
 * @VIPS_REGION_SHRINK_MODE: use the mode
 * @VIPS_REGION_SHRINK_MAX: use the maximum
 * @VIPS_REGION_SHRINK_MIN: use the minimum
 * @VIPS_REGION_SHRINK_NEAREST: use the top-left pixel
 *
 * How to calculate the output pixels when shrinking a 2x2 region.
 */
typedef enum {
	VIPS_REGION_SHRINK_MEAN,
	VIPS_REGION_SHRINK_MEDIAN,
	VIPS_REGION_SHRINK_MODE,
	VIPS_REGION_SHRINK_MAX,
	VIPS_REGION_SHRINK_MIN,
	VIPS_REGION_SHRINK_NEAREST,
	VIPS_REGION_SHRINK_LAST	/*< skip >*/
} VipsRegionShrink;

/* Sub-area of image.
 *
 * Matching typedef in basic.h.
 */
struct _VipsRegion {
	VipsObject parent_object;

	/* Users may read these two fields.
	 */
	/*< public >*/
	VipsImage *im;	/* Link back to parent image */
	VipsRect valid; /* Area of parent we can see */

	/* The rest of VipsRegion is private.
	 */
	/*< private >*/
	RegionType type; /* What kind of attachment */
	VipsPel *data;	 /* Off here to get data */
	int bpl;		 /* Bytes-per-line for data */
	void *seq;		 /* Sequence we are using to fill region */

	/* The thread that made this region. Used to assert() test that
	 * regions are not being shared between threads.
	 */
	GThread *thread;

	/* Ref to the window we use for this region, if any.
	 */
	VipsWindow *window;

	/* Ref to the buffer we use for this region, if any.
	 */
	VipsBuffer *buffer;

	/* The image this region is on has changed and caches need to be
	 * dropped.
	 */
	gboolean invalid;
};

typedef struct _VipsRegionClass {
	VipsObjectClass parent_class;

} VipsRegionClass;

VIPS_API
GType vips_region_get_type(void);

VIPS_API
VipsRegion *vips_region_new(VipsImage *image);

VIPS_API
int vips_region_buffer(VipsRegion *reg, const VipsRect *r);
VIPS_API
int vips_region_image(VipsRegion *reg, const VipsRect *r);
VIPS_API
int vips_region_region(VipsRegion *reg, VipsRegion *dest,
	const VipsRect *r, int x, int y);
VIPS_API
int vips_region_equalsregion(VipsRegion *reg1, VipsRegion *reg2);
VIPS_API
int vips_region_position(VipsRegion *reg, int x, int y);

VIPS_API
void vips_region_paint(VipsRegion *reg, const VipsRect *r, int value);
VIPS_API
void vips_region_paint_pel(VipsRegion *reg,
	const VipsRect *r, const VipsPel *ink);
VIPS_API
void vips_region_black(VipsRegion *reg);
VIPS_API
void vips_region_copy(VipsRegion *reg, VipsRegion *dest,
	const VipsRect *r, int x, int y);
VIPS_API
int vips_region_shrink_method(VipsRegion *from, VipsRegion *to,
	const VipsRect *target, VipsRegionShrink method);
VIPS_API
int vips_region_shrink(VipsRegion *from, VipsRegion *to,
	const VipsRect *target);

VIPS_API
int vips_region_prepare(VipsRegion *reg, const VipsRect *r);
VIPS_API
int vips_region_prepare_to(VipsRegion *reg,
	VipsRegion *dest, const VipsRect *r, int x, int y);

VIPS_API
VipsPel *vips_region_fetch(VipsRegion *region,
	int left, int top, int width, int height, size_t *len);
VIPS_API
int vips_region_width(VipsRegion *region);
VIPS_API
int vips_region_height(VipsRegion *region);

VIPS_API
void vips_region_invalidate(VipsRegion *reg);

/* Use this to count pixels passing through key points. Handy for spotting bad
 * overcomputation.
 */
#ifdef DEBUG_LEAK
#define VIPS_COUNT_PIXELS(R, N) vips__region_count_pixels(R, N)
#else /*!DEBUG_LEAK*/
#define VIPS_COUNT_PIXELS(R, N)
#endif /*DEBUG_LEAK*/

#define VIPS_REGION_LSKIP(R) \
	((size_t) ((R)->bpl))
#define VIPS_REGION_N_ELEMENTS(R) \
	((size_t) ((R)->valid.width * (R)->im->Bands))
#define VIPS_REGION_SIZEOF_ELEMENT(R) \
	(VIPS_IMAGE_SIZEOF_ELEMENT((R)->im))
#define VIPS_REGION_SIZEOF_PEL(R) \
	(VIPS_IMAGE_SIZEOF_PEL((R)->im))
#define VIPS_REGION_SIZEOF_LINE(R) \
	((size_t) ((R)->valid.width * VIPS_REGION_SIZEOF_PEL(R)))

/* If DEBUG is defined, add bounds checking.
 */
#ifdef DEBUG
#define VIPS_REGION_ADDR(R, X, Y) \
	((vips_rect_includespoint(&(R)->valid, (X), (Y))) \
			? ((R)->data + ((Y) - (R)->valid.top) * VIPS_REGION_LSKIP(R) + \
				  ((X) - (R)->valid.left) * VIPS_REGION_SIZEOF_PEL(R)) \
			: (fprintf(stderr, \
				   "VIPS_REGION_ADDR: point out of bounds, " \
				   "file \"%s\", line %d\n" \
				   "(point x=%d, y=%d\n" \
				   " should have been within VipsRect left=%d, top=%d, " \
				   "width=%d, height=%d)\n", \
				   __FILE__, __LINE__, \
				   (X), (Y), \
				   (R)->valid.left, \
				   (R)->valid.top, \
				   (R)->valid.width, \
				   (R)->valid.height), \
				  abort(), (VipsPel *) NULL))
#else /*DEBUG*/
#define VIPS_REGION_ADDR(R, X, Y) \
	((R)->data + \
		((Y) - (R)->valid.top) * VIPS_REGION_LSKIP(R) + \
		((X) - (R)->valid.left) * VIPS_REGION_SIZEOF_PEL(R))
#endif /*DEBUG*/

#define VIPS_REGION_ADDR_TOPLEFT(R) ((R)->data)

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*VIPS_REGION_H*/
