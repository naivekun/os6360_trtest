/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 * SPDX-License-Identifier:     GPL-2.0+ BSD-2-Clause
 */
#include "libfdt_compat.h"

#ifndef HAVE_FDT_FIRST_SUBNODE
int fdt_first_subnode(const void *fdt, int offset)
{
	int depth = 0;

	offset = fdt_next_node(fdt, offset, &depth);
	if (offset < 0 || depth != 1)
		return -FDT_ERR_NOTFOUND;

	return offset;
}
#endif

#ifndef HAVE_FDT_NEXT_SUBNODE
int fdt_next_subnode(const void *fdt, int offset)
{
	int depth = 1;

	/*
	 * With respect to the parent, the depth of the next subnode will be
	 * the same as the last.
	 */
	do {
		offset = fdt_next_node(fdt, offset, &depth);
		if (offset < 0 || depth < 1)
			return -FDT_ERR_NOTFOUND;
	} while (depth > 1);

	return offset;
}
#endif
