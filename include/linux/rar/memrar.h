/*
 *      RAR Handler (/dev/memrar) internal driver API.
 *      Copyright (C) 2009 Intel Corporation. All rights reserved.
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of version 2 of the GNU General
 *      Public License as published by the Free Software Foundation.
 *
 *      This program is distributed in the hope that it will be
 *      useful, but WITHOUT ANY WARRANTY; without even the implied
 *      warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *      PURPOSE.  See the GNU General Public License for more details.
 *      You should have received a copy of the GNU General Public
 *      License along with this program; if not, write to the Free
 *      Software Foundation, Inc., 59 Temple Place - Suite 330,
 *      Boston, MA  02111-1307, USA.
 *      The full GNU General Public License is included in this
 *      distribution in the file called COPYING.
 */


#ifndef _MEMRAR_H
#define _MEMRAR_H

#include <linux/ioctl.h>
#include <linux/types.h>


/*
 * Constants that specify different kinds of RAR regions that could be
 * set up.
 */
enum RAR_type {
	RAR_TYPE_VIDEO = 0,
	RAR_TYPE_AUDIO,
	RAR_TYPE_IMAGE,
	RAR_TYPE_DATA
};

/*
 * @struct RAR_stat
 *
 * @brief This structure is used for @c RAR_HANDLER_STAT ioctl and for
 *        @c RAR_get_stat() user space wrapper function.
 */
struct RAR_stat {
	/* Type of RAR memory (e.g., audio vs. video) */
	__u32 type;

	/*
	* Total size of RAR memory region.
	*/
	__u32 capacity;

	/* Size of the largest reservable block. */
	__u32 largest_block_size;
};


/*
 * @struct RAR_block_info
 *
 * @brief The argument for the @c RAR_HANDLER_RESERVE @c ioctl.
 *
 */
struct RAR_block_info {
	/* Type of RAR memory (e.g., audio vs. video) */
	__u32 type;

	/* Requested size of a block to be reserved in RAR. */
	__u32 size;

	/* Handle that can be used to refer to reserved block. */
	__u32 handle;
};


#define RAR_IOCTL_BASE 0xE0

/* Reserve RAR block. */
#define RAR_HANDLER_RESERVE _IOWR(RAR_IOCTL_BASE, 0x00, struct RAR_block_info)

/* Release previously reserved RAR block. */
#define RAR_HANDLER_RELEASE _IOW(RAR_IOCTL_BASE, 0x01, __u32)

/* Get RAR stats. */
#define RAR_HANDLER_STAT    _IOWR(RAR_IOCTL_BASE, 0x02, struct RAR_stat)


#ifdef __KERNEL__

/* -------------------------------------------------------------- */
/*               Kernel Side RAR Handler Interface                */
/* -------------------------------------------------------------- */

/*
 * @struct RAR_buffer
 *
 * Structure that contains all information related to a given block of
 * memory in RAR.  It is generally only used when retrieving bus
 * addresses.
 *
 * @note This structure is used only by RAR-enabled drivers, and is
 *       not intended to be exposed to the user space.
 */
struct RAR_buffer {
	/* Structure containing base RAR buffer information */
	struct RAR_block_info info;

	/* Buffer bus address */
	dma_addr_t bus_address;
};

/*
 * @function rar_reserve
 *
 * @brief Reserve RAR buffers.
 *
 * This function will reserve buffers in the restricted access regions
 * of given types.
 *
 * @return Number of successfully reserved buffers.
 *         Successful buffer reservations will have the corresponding
 *         @c bus_address field set to a non-zero value in the
 *         given @a buffers vector.
 */
extern size_t rar_reserve(struct RAR_buffer *buffers,
			  size_t count);

/*
 * @function rar_release
 *
 * @brief Release RAR buffers retrieved through call to
 *        @c rar_reserve() or @c rar_handle_to_bus().
 *
 * This function will release RAR buffers that were retrieved through
 * a call to @c rar_reserve() or @c rar_handle_to_bus() by
 * decrementing the reference count.  The RAR buffer will be reclaimed
 * when the reference count drops to zero.
 *
 * @return Number of successfully released buffers.
 *         Successful releases will have their handle field set to
 *         zero in the given @a buffers vector.
 */
extern size_t rar_release(struct RAR_buffer *buffers,
			  size_t count);

/*
 * @function rar_handle_to_bus
 *
 * @brief Convert a vector of RAR handles to bus addresses.
 *
 * This function will retrieve the RAR buffer bus addresses, type and
 * size corresponding to the RAR handles provided in the @a buffers
 * vector.
 *
 * @return Number of successfully converted buffers.
 *         The bus address will be set to @c 0 for unrecognized
 *         handles.
 *
 * @note The reference count for each corresponding buffer in RAR will
 *       be incremented.  Call @c rar_release() when done with the
 *       buffers.
 */
extern size_t rar_handle_to_bus(struct RAR_buffer *buffers,
				size_t count);


#endif  /* __KERNEL__ */

#endif  /* _MEMRAR_H */
