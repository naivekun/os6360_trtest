/*
 * Copyright (C) 2017 Cisco Systems, Inc.
 *
 * FOR INTERNAL USE ONLY - DO NOT DISTRIBUTE PUBLICLY
 *
 * Copyright (c) 2013, Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "uboot.h"

#define IMAGE_MAX_HASHED_NODES		100

struct checksum_algo checksum_algos[] = {
	{
		"sha1",
		SHA1_SUM_LEN,
	},
	{
		"sha256",
		SHA256_SUM_LEN,
	},
	{
		"sha384",
		SHA384_SUM_LEN,
	},
	{
		"sha512",
		SHA512_SUM_LEN,
	}

};

struct crypto_algo crypto_algos[] = {
	{
		"rsa2048",
		RSA2048_BYTES,
		rb_uboot_sign,
		rb_uboot_verify,
	},
	{
		"rsa3072",
		RSA3072_BYTES,
		rb_uboot_sign,
		rb_uboot_verify,
	},
	{
		"rsa4096",
		RSA4096_BYTES,
		rb_uboot_sign,
		rb_uboot_verify,
	},
	{
		"secp256r1",
		ECP256_BYTES,
		rb_uboot_sign,
		rb_uboot_verify,
	},
	{
		"secp384r1",
		ECP384_BYTES,
		rb_uboot_sign,
		rb_uboot_verify,
	},

};

struct checksum_algo *image_get_checksum_algo(const char *full_name)
{
	int i;
	const char *name;

	for (i = 0; i < ARRAY_SIZE(checksum_algos); i++) {
		name = checksum_algos[i].name;
		/* Make sure names match and next char is a comma */
		if (!strncmp(name, full_name, strlen(name)) &&
		    full_name[strlen(name)] == ',')
			return &checksum_algos[i];
	}

	return NULL;
}

struct crypto_algo *image_get_crypto_algo(const char *full_name)
{
	int i;
	const char *name;

	/* Move name to after the comma */
	name = strchr(full_name, ',');
	if (!name)
		return NULL;
	name += 1;

	for (i = 0; i < ARRAY_SIZE(crypto_algos); i++) {
		if (!strcmp(crypto_algos[i].name, name))
			return &crypto_algos[i];
	}

	return NULL;
}

/**
 * fit_region_make_list() - Make a list of image regions
 *
 * Given a list of fdt_regions, create a list of image_regions. This is a
 * simple conversion routine since the FDT and image code use different
 * structures.
 *
 * @fit: FIT image
 * @fdt_regions: Pointer to FDT regions
 * @count: Number of FDT regions
 * @region: Pointer to image regions, which must hold @count records. If
 * region is NULL, then (except for an SPL build) the array will be
 * allocated.
 * @return: Pointer to image regions
 */
struct image_region *fit_region_make_list(const void *fit,
					  struct fdt_region *fdt_regions,
					  int count,
					  struct image_region *region)
{
	int i;

	debug("Hash regions:\n");
	debug("%10s %10s\n", "Offset", "Size");

	/*
	 * Use malloc() except in SPL (to save code size). In SPL the caller
	 * must allocate the array.
	 */
#ifndef CONFIG_SPL_BUILD
	if (!region)
		region = calloc(sizeof(*region), count);
#endif
	if (!region)
		return NULL;
	for (i = 0; i < count; i++) {
		debug("%10x %10x\n", fdt_regions[i].offset,
		      fdt_regions[i].size);
		region[i].data = fit + fdt_regions[i].offset;
		region[i].size = fdt_regions[i].size;
	}

	return region;
}

static int fit_image_setup_verify(struct image_sign_info *info,
				  const void *fit, int noffset,
				  char **err_msgp, const void *self)
{
	char *algo_name;

	if (fit_image_hash_get_algo(fit, noffset, &algo_name)) {
		*err_msgp = "Can't get hash algo property";
		return -1;
	}
	memset(info, '\0', sizeof(*info));
	info->keyname = fdt_getprop(fit, noffset, "key-name-hint", NULL);
	info->fit = (void *)fit;
	info->node_offset = noffset;
	info->name = algo_name;
	info->checksum = image_get_checksum_algo(algo_name);
	info->crypto = image_get_crypto_algo(algo_name);
	info->self = self;
	printf("%s:%s", algo_name, info->keyname);

	if (!info->checksum || !info->crypto) {
		*err_msgp = "Unknown signature algorithm";
		return -1;
	}

	return 0;
}

static int fit_config_check_sig(const void *fit, int noffset,
				char **err_msgp, const void *self)
{
	char * const exc_prop[] = {"data"};
	const char *prop, *end, *name;
	struct image_sign_info info;
	const uint32_t *strings;
	uint8_t *fit_value;
	int fit_value_len;
	int max_regions;
	int i, prop_len;
	char path[200];
	int count;

	debug("%s: fdt=%p, conf='%s', sig='%s'\n", __func__, gd_fdt_blob(),
	      fit_get_name(fit, noffset, NULL),
	      fit_get_name(gd_fdt_blob(), NULL));
	*err_msgp = NULL;
	if (fit_image_setup_verify(&info, fit, noffset,
				   err_msgp, self))
		return -1;

	if (fit_image_hash_get_value(fit, noffset, &fit_value,
				     &fit_value_len)) {
		*err_msgp = "Can't get hash value property";
		return -1;
	}

	/* Count the number of strings in the property */
	prop = fdt_getprop(fit, noffset, "hashed-nodes", &prop_len);
	end = prop ? prop + prop_len : prop;
	for (name = prop, count = 0; name < end; name++)
		if (!*name)
			count++;
	if (!count) {
		*err_msgp = "Can't get hashed-nodes property";
		return -1;
	}

	/* Add a sanity check here since we are using the stack */
	if (count > IMAGE_MAX_HASHED_NODES) {
		*err_msgp = "Number of hashed nodes exceeds maximum";
		return -1;
	}

	/* Create a list of node names from those strings */
	char *node_inc[count];

	debug("Hash nodes (%d):\n", count);
	for (name = prop, i = 0; name < end; name += strlen(name) + 1, i++) {
		debug("   '%s'\n", name);
		node_inc[i] = (char *)name;
	}

	/*
	 * Each node can generate one region for each sub-node. Allow for
	 * 7 sub-nodes (hash@1, signature@1, etc.) and some extra.
	 */
	max_regions = 20 + count * 7;
	struct fdt_region fdt_regions[max_regions];

	/* Get a list of regions to hash */
	count = fdt_find_regions(fit, node_inc, count,
				 exc_prop, ARRAY_SIZE(exc_prop),
				 fdt_regions, max_regions - 1,
				 path, sizeof(path), 0);
	if (count < 0) {
		*err_msgp = "Failed to hash configuration";
		return -1;
	}
	if (count == 0) {
		*err_msgp = "No data to hash";
		return -1;
	}
	if (count >= max_regions - 1) {
		*err_msgp = "Too many hash regions";
		return -1;
	}

	/* Add the strings */
	strings = fdt_getprop(fit, noffset, "hashed-strings", NULL);
	if (strings) {
		fdt_regions[count].offset = fdt_off_dt_strings(fit) +
			fdt32_to_cpu(strings[0]);
		fdt_regions[count].size = fdt32_to_cpu(strings[1]);
		count++;
	}

	/* Allocate the region list on the stack */
	struct image_region region[count];

	fit_region_make_list(fit, fdt_regions, count, region);
	if (info.crypto->verify(&info, region, count, fit_value,
				fit_value_len)) {
		*err_msgp = "Verification failed";
		return -1;
	}

	return 0;
}

static int fit_config_verify_node(const void *fit, int conf_noffset,
				  const void *self)
{
	int noffset;
	char *err_msg = "";
	int verified = 0;
	int ret;

	/* Process all hash subnodes of the component conf node */
	fdt_for_each_subnode(noffset, fit, conf_noffset) {
		const char *name = fit_get_name(fit, noffset, NULL);

		if (!strncmp(name, FIT_SIG_NODENAME,
			     strlen(FIT_SIG_NODENAME))) {
			printf("Verifying node '%s/%s' with ",
			       fit_get_name(fit, conf_noffset, NULL),
			       name);
			ret = fit_config_check_sig(fit, noffset,
						   &err_msg, self);
			if (ret) {
				puts("- ");
			} else {
				puts("+ ");
				verified = 1;
				break;
			}
		}
	}

	if (noffset == -FDT_ERR_TRUNCATED || noffset == -FDT_ERR_BADSTRUCTURE) {
		err_msg = "Corrupted or truncated tree";
		goto error;
	}

	return verified ? 0 : -EPERM;

error:
	printf(" error!\n%s for '%s' hash node in '%s' config node\n",
	       err_msg, fit_get_name(fit, noffset, NULL),
	       fit_get_name(fit, conf_noffset, NULL));
	return -1;
}

int fit_config_verify(const void *fit, const void *self)
{
	int confs_noffset;
	int noffset;
	int ret;
	int failed = 0;

	/* Find configurations parent node offset */
	confs_noffset = fdt_path_offset(fit, FIT_CONFS_PATH);
	if (confs_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_CONFS_PATH, fdt_strerror(confs_noffset));
		return -ENOENT;
	}

	/* Process its subnodes, print out component images details */
	for (noffset = fdt_first_subnode(fit, confs_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		ret = fit_config_verify_node(fit, noffset, self);
		if (ret)
			failed = 1;
	}

	return failed;
}
