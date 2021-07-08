/*
 * Copyright (C) 2017 Cisco Systems, Inc.
 *
 * FOR INTERNAL USE ONLY - DO NOT DISTRIBUTE PUBLICLY
 *
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

/**
 * fit_image_check_hash() - check if a hash node exists with given algo
 *
 * returns
 *     0, on match
 *     else, number from name
 *     -ERRNO, on failure
 */
static int fit_image_check_hash(void *fit, const char *image_name,
				int noffset, const char *check_algo)
{
	const char *node_name;
	char *algo;
	int number = -1;

	node_name = fit_get_name(fit, noffset, NULL);

	if (fit_image_hash_get_algo(fit, noffset, &algo)) {
		printf("Can't get hash algo property for '%s' hash node in '%s' image node\n",
		       node_name, image_name);
		return -ENOENT;
	}

	if (!strcmp(algo, check_algo)) {
		printf("Found algo '%s' in node '%s/%s'\n",
		       check_algo, image_name, node_name);
		return 0;
	}

	sscanf(node_name, "hash@%d", &number);
	if (number <= 0) {
		printf("Invalid hash number '%d' for node '%s/%s'\n",
		       number, image_name, node_name);
		return -EINVAL;
	}
	return number;
}

/**
 * fit_image_add_hash() - add a new hash node with the given algo
 *
 * returns
 *     0, on success
 *     -ERRNO, on failure
 */
static int fit_image_add_hash(void *fit, int image_noffset, int number,
			      const char *algo, const void *data,
			      size_t size)
{
	uint8_t value[FIT_MAX_HASH_LEN];
	char node_name[10];
	int value_len;
	int noffset;
	int ret;

	sprintf(node_name, "hash@%d", number);
	printf("Adding hash node '%s/%s' for algo '%s'\n",
	       fit_get_name(fit, image_noffset, NULL), node_name, algo);

	noffset = fdt_add_subnode(fit, image_noffset, node_name);
	if (noffset < 0) {
		printf("Can't add subnode '%s' to '%s' node(%s)\n",
		       node_name, fit_get_name(fit, image_noffset, NULL),
		       fdt_strerror(noffset));
		return noffset == -FDT_ERR_NOSPACE ? -ENOSPC : -EIO;
	}

	if (rb_uboot_hash(data, size, algo, value, &value_len)) {
		printf("Unsupported hash algorithm (%s) for '%s' hash node in '%s' image node\n",
		       algo, node_name, fit_get_name(fit, image_noffset, NULL));
		return -EINVAL;
	}

	ret = fdt_setprop_string(fit, noffset, FIT_ALGO_PROP, algo);
	if (ret) {
		printf("Can't set hash '%s' property for '%s' node(%s)\n",
		       FIT_ALGO_PROP, fit_get_name(fit, noffset, NULL),
		       fdt_strerror(ret));
		return ret == -FDT_ERR_NOSPACE ? -ENOSPC : -EIO;
	}
	ret = fdt_setprop(fit, noffset, FIT_VALUE_PROP, value, value_len);
	if (ret) {
		printf("Can't set hash '%s' property for '%s' node(%s)\n",
		       FIT_VALUE_PROP, fit_get_name(fit, noffset, NULL),
		       fdt_strerror(ret));
		return ret == -FDT_ERR_NOSPACE ? -ENOSPC : -EIO;
	}

	return 0;
}

/**
 * fit_image_ensure_hash() - add a hash node for the given algo if none exists
 *
 * returns
 *     0, on success
 *     -ERRNO, on failure
 */
static int fit_image_ensure_hash(void *fit, int image_noffset,
				 const char *algo)
{
	const char *image_name;
	const void *data;
	size_t size;
	int noffset;
	int max = 0;

	image_name = fit_get_name(fit, image_noffset, NULL);

	/* Process all hash subnodes of the component image node */
	for (noffset = fdt_first_subnode(fit, image_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		const char *node_name;
		int ret = 0;

		/*
		 * Check subnode name, must be equal to "hash"
		 * Multiple hash nodes require unique unit node
		 * names, e.g. hash@1, hash@2, etc.
		 */
		node_name = fit_get_name(fit, noffset, NULL);
		if (!strncmp(node_name, FIT_HASH_NODENAME,
			     strlen(FIT_HASH_NODENAME))) {
			ret = fit_image_check_hash(fit, image_name,
						   noffset, algo);
			if (ret <= 0)
				return ret;
			else if (ret > max)
				max = ret;
		}
	}

	/* Get image data and data length */
	if (fit_image_get_data(fit, image_noffset, &data, &size)) {
		printf("Can't get image data/size\n");
		return -ENOENT;
	}
	/* Add the hash node if we didn't find one */
	return fit_image_add_hash(fit, image_noffset, max+1, algo,
				  data, size);
}

int fit_image_hash(void *fit, const char *algo)
{
	int images_noffset;
	int noffset;
	int ret;

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return -ENOENT;
	}

	/* Process its subnodes, print out component images details */
	for (noffset = fdt_first_subnode(fit, images_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		ret = fit_image_ensure_hash(fit, noffset, algo);
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * fit_image_write_sig() - write the signature to a FIT
 *
 * This writes the signature and signer data to the FIT.
 *
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @value: signature value to be set
 * @value_len: signature value length
 * @comment: Text comment to write (NULL for none)
 *
 * returns
 *     0, on success
 *     -FDT_ERR_..., on failure
 */
static int fit_image_write_sig(void *fit, int noffset, uint8_t *value,
			       int value_len, const char *comment,
			       const char *region_prop,
			       int region_proplen)
{
	int string_size;
	int ret;

	/*
	 * Get the current string size, before we update the FIT and add
	 * more
	 */
	string_size = fdt_size_dt_strings(fit);

	ret = fdt_setprop(fit, noffset, FIT_VALUE_PROP, value, value_len);
	if (!ret) {
		ret = fdt_setprop_string(fit, noffset, "signer-name",
					 "mkimage");
	}
	if (!ret) {
		ret = fdt_setprop_string(fit, noffset, "signer-version",
					 PLAIN_VERSION);
	}
	if (comment && !ret)
		ret = fdt_setprop_string(fit, noffset, "comment", comment);
	if (!ret)
		ret = fit_set_timestamp(fit, noffset, time(NULL));
	if (region_prop && !ret) {
		uint32_t strdata[2];

		ret = fdt_setprop(fit, noffset, "hashed-nodes",
				  region_prop, region_proplen);
		strdata[0] = 0;
		strdata[1] = cpu_to_fdt32(string_size);
		if (!ret) {
			ret = fdt_setprop(fit, noffset, "hashed-strings",
					  strdata, sizeof(strdata));
		}
	}

	return ret;
}

static int fit_image_setup_sig(struct image_sign_info *info, void *fit,
			       const char *image_name, int noffset,
			       const void *self)
{
	const char *node_name;
	char *algo_name;

	node_name = fit_get_name(fit, noffset, NULL);
	if (fit_image_hash_get_algo(fit, noffset, &algo_name)) {
		printf("Can't get algo property for '%s' signature node in '%s' image node\n",
		       node_name, image_name);
		return -1;
	}

	memset(info, '\0', sizeof(*info));
	info->keyname = fdt_getprop(fit, noffset, "key-name-hint", NULL);
	info->fit = fit;
	info->node_offset = noffset;
	info->name = algo_name;
	info->checksum = image_get_checksum_algo(algo_name);
	info->crypto = image_get_crypto_algo(algo_name);
	info->self = self;
	if (!info->checksum || !info->crypto) {
		printf("Unsupported signature algorithm (%s) for '%s' signature node in '%s' image node\n",
		       algo_name, node_name, image_name);
		return -1;
	}

	return 0;
}

struct strlist {
	int count;
	char **strings;
};

static void strlist_init(struct strlist *list)
{
	memset(list, '\0', sizeof(*list));
}

static void strlist_free(struct strlist *list)
{
	int i;

	for (i = 0; i < list->count; i++)
		free(list->strings[i]);
	free(list->strings);
}

static int strlist_add(struct strlist *list, const char *str)
{
	char *dup;

	dup = strdup(str);
	list->strings = realloc(list->strings,
				(list->count + 1) * sizeof(char *));
	if (!list || !str)
		return -1;
	list->strings[list->count++] = dup;

	return 0;
}

static const char *fit_config_get_image_list(void *fit, int noffset,
					     int *lenp,
					     int *allow_missingp)
{
	static const char default_list[] = FIT_KERNEL_PROP "\0"
		FIT_FDT_PROP;
	const char *prop;

	/* If there is an "image" property, use that */
	prop = fdt_getprop(fit, noffset, "sign-images", lenp);
	if (prop) {
		*allow_missingp = 0;
		return *lenp ? prop : NULL;
	}

	/* Default image list */
	*allow_missingp = 1;
	*lenp = sizeof(default_list);

	return default_list;
}

static int fit_config_get_hash_list(void *fit, int conf_noffset,
				    int sig_offset,
				    struct strlist *node_inc)
{
	int allow_missing;
	const char *prop, *iname, *end;
	const char *conf_name, *sig_name;
	char name[200], path[200];
	int image_count;
	int ret, len;

	conf_name = fit_get_name(fit, conf_noffset, NULL);
	sig_name = fit_get_name(fit, sig_offset, NULL);

	/*
	 * Build a list of nodes we need to hash. We always need the root
	 * node and the configuration.
	 */
	strlist_init(node_inc);
	snprintf(name, sizeof(name), "%s/%s", FIT_CONFS_PATH, conf_name);
	if (strlist_add(node_inc, "/") ||
	    strlist_add(node_inc, name))
		goto err_mem;

	/* Get a list of images that we intend to sign */
	prop = fit_config_get_image_list(fit, sig_offset, &len,
					&allow_missing);
	if (!prop)
		return 0;

	/* Locate the images */
	end = prop + len;
	image_count = 0;
	for (iname = prop; iname < end; iname += strlen(iname) + 1) {
		int noffset;
		int image_noffset;
		int hash_count;

		image_noffset = fit_conf_get_prop_node(fit, conf_noffset,
						       iname);
		if (image_noffset < 0) {
			printf("Failed to find image '%s' in  configuration '%s/%s'\n",
			       iname, conf_name, sig_name);
			if (allow_missing)
				continue;

			return -ENOENT;
		}

		ret = fdt_get_path(fit, image_noffset, path, sizeof(path));
		if (ret < 0)
			goto err_path;
		if (strlist_add(node_inc, path))
			goto err_mem;

		snprintf(name, sizeof(name), "%s/%s", FIT_CONFS_PATH,
			 conf_name);

		/* Add all this image's hashes */
		hash_count = 0;
		for (noffset = fdt_first_subnode(fit, image_noffset);
		     noffset >= 0;
		     noffset = fdt_next_subnode(fit, noffset)) {
			const char *name = fit_get_name(fit, noffset, NULL);

			if (strncmp(name, FIT_HASH_NODENAME,
				    strlen(FIT_HASH_NODENAME)))
				continue;
			ret = fdt_get_path(fit, noffset, path, sizeof(path));
			if (ret < 0)
				goto err_path;
			if (strlist_add(node_inc, path))
				goto err_mem;
			hash_count++;
		}

		if (!hash_count) {
			printf("Failed to find any hash nodes in configuration '%s/%s' image '%s' - without these it is not possible to verify this image\n",
			       conf_name, sig_name, iname);
			return -ENOMSG;
		}

		image_count++;
	}

	if (!image_count) {
		printf("Failed to find any images for configuration '%s/%s'\n",
		       conf_name, sig_name);
		return -ENOMSG;
	}

	return 0;

err_mem:
	printf("Out of memory processing configuration '%s/%s'\n", conf_name,
	       sig_name);
	return -ENOMEM;

err_path:
	printf("Failed to get path for image '%s' in configuration '%s/%s': %s\n",
	       iname, conf_name, sig_name, fdt_strerror(ret));
	return -ENOENT;
}

static int fit_config_get_data(void *fit, int conf_noffset, int noffset,
			       struct image_region **regionp,
			       int *region_countp, char **region_propp,
			       int *region_proplen)
{
	char * const exc_prop[] = {"data"};
	struct strlist node_inc;
	struct image_region *region;
	struct fdt_region fdt_regions[100];
	const char *conf_name, *sig_name;
	char path[200];
	int count, i;
	char *region_prop;
	int ret, len;

	conf_name = fit_get_name(fit, conf_noffset, NULL);
	sig_name = fit_get_name(fit, conf_noffset, NULL);
	debug("%s: conf='%s', sig='%s'\n", __func__, conf_name, sig_name);

	/* Get a list of nodes we want to hash */
	ret = fit_config_get_hash_list(fit, conf_noffset, noffset, &node_inc);
	if (ret)
		return ret;

	/* Get a list of regions to hash */
	count = fdt_find_regions(fit, node_inc.strings, node_inc.count,
				 exc_prop, ARRAY_SIZE(exc_prop),
				 fdt_regions, ARRAY_SIZE(fdt_regions),
				 path, sizeof(path), 1);
	if (count < 0) {
		printf("Failed to hash configuration '%s/%s': %s\n", conf_name,
		       sig_name, fdt_strerror(ret));
		return -EIO;
	}
	if (count == 0) {
		printf("No data to hash for configuration '%s/%s': %s\n",
		       conf_name, sig_name, fdt_strerror(ret));
		return -EINVAL;
	}

	/* Build our list of data blocks */
	region = fit_region_make_list(fit, fdt_regions, count, NULL);
	if (!region) {
		printf("Out of memory hashing configuration '%s/%s'\n",
		       conf_name, sig_name);
		return -ENOMEM;
	}

	/* Create a list of all hashed properties */
	debug("Hash nodes:\n");
	for (i = len = 0; i < node_inc.count; i++) {
		debug("   %s\n", node_inc.strings[i]);
		len += strlen(node_inc.strings[i]) + 1;
	}
	region_prop = malloc(len);
	if (!region_prop) {
		printf("Out of memory setting up regions for configuration '%s/%s'\n",
		       conf_name, sig_name);
		return -ENOMEM;
	}
	for (i = len = 0; i < node_inc.count;
	     len += strlen(node_inc.strings[i]) + 1, i++)
		strcpy(region_prop + len, node_inc.strings[i]);
	strlist_free(&node_inc);

	*region_countp = count;
	*regionp = region;
	*region_propp = region_prop;
	*region_proplen = len;

	return 0;
}

static int fit_config_process_sig(void *fit, const char *conf_name,
				  int conf_noffset, int noffset,
				  const char *comment, const void *self)
{
	struct image_sign_info info;
	const char *node_name;
	struct image_region *region;
	char *region_prop;
	int region_proplen;
	int region_count;
	uint8_t *value;
	uint value_len;
	int ret;

	node_name = fit_get_name(fit, noffset, NULL);
	if (fit_config_get_data(fit, conf_noffset, noffset, &region,
				&region_count, &region_prop,
				&region_proplen))
		return -1;

	if (fit_image_setup_sig(&info, fit, conf_name, noffset, self))
		return -1;

	ret = info.crypto->sign(&info, region, region_count, &value,
				&value_len);
	free(region);
	if (ret) {
		printf("Failed to sign '%s' signature node in '%s' conf node\n",
		       node_name, conf_name);

		/* We allow keys to be missing */
		if (ret == -ENOENT)
			return 0;
		return -1;
	}

	printf("Signing node '%s/%s' with %s:%s\n",
	       conf_name, node_name, info.name, info.keyname);
	ret = fit_image_write_sig(fit, noffset, value, value_len, comment,
				  region_prop, region_proplen);
	if (ret) {
		if (ret == -FDT_ERR_NOSPACE)
			return -ENOSPC;
		printf("Can't write signature for '%s' signature node in '%s' conf node: %s\n",
		       node_name, conf_name, fdt_strerror(ret));
		return -1;
	}
	free(value);
	free(region_prop);

	return 0;
}

static int fit_config_sign_node(void *fit, int conf_noffset,
				const char *comment, const void *self)
{
	const char *conf_name;
	int noffset;

	conf_name = fit_get_name(fit, conf_noffset, NULL);

	/* Process all sig subnodes of the configuration node */
	for (noffset = fdt_first_subnode(fit, conf_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		const char *node_name;
		int ret = 0;

		node_name = fit_get_name(fit, noffset, NULL);
		if (!strncmp(node_name, FIT_SIG_NODENAME,
			     strlen(FIT_SIG_NODENAME))) {
			ret = fit_config_process_sig(fit, conf_name,
						     conf_noffset,
						     noffset, comment,
						     self);
		}
		if (ret)
			return ret;
	}

	return 0;
}

int fit_config_sign(void *fit, const char *comment, const void *self)
{
	int confs_noffset;
	int noffset;
	int ret;

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
		ret = fit_config_sign_node(fit, noffset, comment, self);
		if (ret)
			return ret;
	}

	return 0;
}
