/*
 * Copyright (c) 2013, Google Inc.
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "uboot.h"

static void fit_get_debug(const void *fit, int noffset,
		char *prop_name, int err)
{
	debug("Can't get '%s' property from FIT 0x%08lx, node: offset %d, name %s (%s)\n",
	      prop_name, (ulong)fit, noffset, fit_get_name(fit, noffset, NULL),
	      fdt_strerror(err));
}

/**
 * fit_image_get_node - get node offset for component image of a given unit name
 * @fit: pointer to the FIT format image header
 * @image_uname: component image node unit name
 *
 * fit_image_get_node() finds a component image (within the '/images'
 * node) of a provided unit name. If image is found its node offset is
 * returned to the caller.
 *
 * returns:
 *     image node offset when found (>=0)
 *     negative number on failure (FDT_ERR_* code)
 */
int fit_image_get_node(const void *fit, const char *image_uname)
{
	int noffset, images_noffset;

	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		debug("Can't find images parent node '%s' (%s)\n",
		      FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return images_noffset;
	}

	noffset = fdt_subnode_offset(fit, images_noffset, image_uname);
	if (noffset < 0) {
		debug("Can't get node offset for image unit name: '%s' (%s)\n",
		      image_uname, fdt_strerror(noffset));
	}

	return noffset;
}

/**
 * fit_image_get_data - get data property and its size for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @data: double pointer to void, will hold data property's data address
 * @size: pointer to size_t, will hold data property's data size
 *
 * fit_image_get_data() finds data property in a given component image node.
 * If the property is found its data start address and size are returned to
 * the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_get_data(const void *fit, int noffset,
		const void **data, size_t *size)
{
	int len;

	*data = fdt_getprop(fit, noffset, FIT_DATA_PROP, &len);
	if (*data == NULL) {
		fit_get_debug(fit, noffset, FIT_DATA_PROP, len);
		*size = 0;
		return -1;
	}

	*size = len;
	return 0;
}

/**
 * fit_image_hash_get_algo - get hash algorithm name
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @algo: double pointer to char, will hold pointer to the algorithm name
 *
 * fit_image_hash_get_algo() finds hash algorithm property in a given hash node.
 * If the property is found its data start address is returned to the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_hash_get_algo(const void *fit, int noffset, char **algo)
{
	int len;

	*algo = (char *)fdt_getprop(fit, noffset, FIT_ALGO_PROP, &len);
	if (*algo == NULL) {
		fit_get_debug(fit, noffset, FIT_ALGO_PROP, len);
		return -1;
	}

	return 0;
}

/**
 * fit_image_hash_get_value - get hash value and length
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @value: double pointer to uint8_t, will hold address of a hash value data
 * @value_len: pointer to an int, will hold hash data length
 *
 * fit_image_hash_get_value() finds hash value property in a given hash node.
 * If the property is found its data start address and size are returned to
 * the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
int fit_image_hash_get_value(const void *fit, int noffset, uint8_t **value,
                                int *value_len)
{
        int len;

        *value = (uint8_t *)fdt_getprop(fit, noffset, FIT_VALUE_PROP, &len);
        if (*value == NULL) {
                fit_get_debug(fit, noffset, FIT_VALUE_PROP, len);
                *value_len = 0;
                return -1;
        }

        *value_len = len;
        return 0;
}

/**
 * fit_set_timestamp - set node timestamp property
 * @fit: pointer to the FIT format image header
 * @noffset: node offset
 * @timestamp: timestamp value to be set
 *
 * fit_set_timestamp() attempts to set timestamp property in the requested
 * node and returns operation status to the caller.
 *
 * returns:
 *     0, on success
 *     -ENOSPC if no space in device tree, -1 for other error
 */
int fit_set_timestamp(void *fit, int noffset, time_t timestamp)
{
	uint32_t t;
	int ret;

	t = cpu_to_uimage(timestamp);
	ret = fdt_setprop(fit, noffset, FIT_TIMESTAMP_PROP, &t,
				sizeof(uint32_t));
	if (ret) {
		debug("Can't set '%s' property for '%s' node (%s)\n",
		      FIT_TIMESTAMP_PROP, fit_get_name(fit, noffset, NULL),
		      fdt_strerror(ret));
		return ret == -FDT_ERR_NOSPACE ? -ENOSPC : -1;
	}

	return 0;
}

/**
 * fit_conf_get_node - get node offset for configuration of a given unit name
 * @fit: pointer to the FIT format image header
 * @conf_uname: configuration node unit name
 *
 * fit_conf_get_node() finds a configuration (within the '/configurations'
 * parent node) of a provided unit name. If configuration is found its node
 * offset is returned to the caller.
 *
 * When NULL is provided in second argument fit_conf_get_node() will search
 * for a default configuration node instead. Default configuration node unit
 * name is retrieved from FIT_DEFAULT_PROP property of the '/configurations'
 * node.
 *
 * returns:
 *     configuration node offset when found (>=0)
 *     negative number on failure (FDT_ERR_* code)
 */
int fit_conf_get_node(const void *fit, const char *conf_uname)
{
        int noffset, confs_noffset;
        int len;

        confs_noffset = fdt_path_offset(fit, FIT_CONFS_PATH);
        if (confs_noffset < 0) {
                debug("Can't find configurations parent node '%s' (%s)\n",
                      FIT_CONFS_PATH, fdt_strerror(confs_noffset));
                return confs_noffset;
        }

        if (conf_uname == NULL) {
                /* get configuration unit name from the default property */
                debug("No configuration specified, trying default...\n");
                conf_uname = (char *)fdt_getprop(fit, confs_noffset,
                                                 FIT_DEFAULT_PROP, &len);
                if (conf_uname == NULL) {
                        fit_get_debug(fit, confs_noffset, FIT_DEFAULT_PROP,
                                      len);
                        return len;
                }
                debug("Found default configuration: '%s'\n", conf_uname);
        }

        noffset = fdt_subnode_offset(fit, confs_noffset, conf_uname);
        if (noffset < 0) {
                debug("Can't get node offset for configuration unit name: '%s' (%s)\n",
                      conf_uname, fdt_strerror(noffset));
        }

        return noffset;
}

int fit_conf_get_prop_node(const void *fit, int noffset,
		const char *prop_name)
{
	char *uname;
	int len;

	/* get kernel image unit name from configuration kernel property */
	uname = (char *)fdt_getprop(fit, noffset, prop_name, &len);
	if (uname == NULL)
		return len;

	return fit_image_get_node(fit, uname);
}
