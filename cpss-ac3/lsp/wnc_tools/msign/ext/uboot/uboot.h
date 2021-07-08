/*
 * Copyright (C) 2017 Cisco System, Inc.
 *
 * FOR INTERNAL USE ONLY - DO NOT DISTRIBUTE PUBLICLY
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __UBOOT_H__
#define __UBOOT_H__

#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "libfdt_compat.h"

/* compiler.h */
typedef unsigned int uint;

/* PLAIN_VERSION defined by Makefile */
/* #define PLAIN_VERSION		"msign-1.0" */

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))

#ifdef DEBUG
#define debug(fmt,args...)	printf(fmt,##args)
#else
#define debug(fmt,args...)
#endif

#ifndef O_BINARY
#define O_BINARY	0
#endif

/* shaX.h */
#define SHA1_SUM_LEN		20
#define SHA256_SUM_LEN		32
#define SHA384_SUM_LEN		48
#define SHA512_SUM_LEN		64

/* rsa.h */
#define RSA2048_BYTES		(2048 / 8)
#define RSA3072_BYTES		(3072 / 8)
#define RSA4096_BYTES		(4096 / 8)

/* ecdsa.h */
#define ECP256_BYTES		(256 / 8)
#define ECP384_BYTES		(384 / 8)

/* image.h */

#define FIT_IMAGES_PATH		"/images"
#define FIT_CONFS_PATH		"/configurations"

/* hash/signature node */
#define FIT_HASH_NODENAME	"hash"
#define FIT_ALGO_PROP		"algo"
#define FIT_VALUE_PROP		"value"
#define FIT_IGNORE_PROP		"uboot-ignore"
#define FIT_SIG_NODENAME	"signature"

/* image node */
#define FIT_DATA_PROP		"data"
#define FIT_TIMESTAMP_PROP	"timestamp"
#define FIT_DESC_PROP		"description"
#define FIT_ARCH_PROP		"arch"
#define FIT_TYPE_PROP		"type"
#define FIT_OS_PROP		"os"
#define FIT_COMP_PROP		"compression"
#define FIT_ENTRY_PROP		"entry"
#define FIT_LOAD_PROP		"load"

/* configuration node */
#define FIT_KERNEL_PROP		"kernel"
#define FIT_RAMDISK_PROP	"ramdisk"
#define FIT_FDT_PROP		"fdt"
#define FIT_LOADABLE_PROP	"loadables"
#define FIT_DEFAULT_PROP	"default"
#define FIT_SETUP_PROP		"setup"
#define FIT_FPGA_PROP		"fpga"

#define FIT_MAX_HASH_LEN	64

#define uimage_to_cpu(x)	be32toh(x)
#define cpu_to_uimage(x)	htobe32(x)

/* Information passed to the signing routines */
struct image_sign_info {
	const char *keyname;		/* Name of key to use */
	void *fit;			/* Pointer to FIT blob */
	int node_offset;		/* Offset of signature node */
	const char *name;		/* Algorithm name */
	struct checksum_algo *checksum;	/* Checksum algorithm information */
	struct crypto_algo *crypto;	/* Crypto algorithm information */
	const void *self;
};

/* A part of an image, used for hashing */
struct image_region {
	const void *data;
	int size;
};

struct checksum_algo {
	const char *name;
	const int checksum_len;
};

struct crypto_algo {
	const char *name;		/* Name of algorithm */
	const int key_len;

	/**
	 * sign() - calculate and return signature for given input data
	 *
	 * @info:	Specifies key and FIT information
	 * @data:	Pointer to the input data
	 * @data_len:	Data length
	 * @sigp:	Set to an allocated buffer holding the signature
	 * @sig_len:	Set to length of the calculated hash
	 *
	 * This computes input data signature according to selected algorithm.
	 * Resulting signature value is placed in an allocated buffer, the
	 * pointer is returned as *sigp. The length of the calculated
	 * signature is returned via the sig_len pointer argument. The caller
	 * should free *sigp.
	 *
	 * @return: 0, on success, -ve on error
	 */
	int (*sign)(struct image_sign_info *info,
		    const struct image_region region[], int region_count,
		    uint8_t **sigp, uint *sig_len);

	/**
	 * verify() - Verify a signature against some data
	 *
	 * @info:	Specifies key and FIT information
	 * @data:	Pointer to the input data
	 * @data_len:	Data length
	 * @sig:	Signature
	 * @sig_len:	Number of bytes in signature
	 * @return 0 if verified, -ve on error
	 */
	int (*verify)(struct image_sign_info *info,
		      const struct image_region region[], int region_count,
		      uint8_t *sig, uint sig_len);
};

static inline const char *fit_get_name(const void *fit_hdr,
				       int noffset, int *len)
{
	return fdt_get_name(fit_hdr, noffset, len);
}

/* image-host.c */
int fit_image_hash(void *fit, const char *algo);
int fit_config_sign(void *fit, const char *comment, const void *self);

/* image-fit.c */
int fit_image_get_node(const void *fit, const char *image_uname);
int fit_image_get_data(const void *fit, int noffset, const void **data,
		       size_t *size);
int fit_image_hash_get_algo(const void *fit, int noffset, char **algo);
int fit_image_hash_get_value(const void *fit, int noffset, uint8_t **value,
			     int *value_len);
int fit_set_timestamp(void *fit, int noffset, time_t timestamp);
int fit_conf_get_node(const void *fit, const char *conf_uname);
int fit_conf_get_prop_node(const void *fit, int noffset,
			   const char *prop_name);

/* image-sig.c */
struct checksum_algo *image_get_checksum_algo(const char *full_name);
struct crypto_algo *image_get_crypto_algo(const char *full_name);
struct image_region *fit_region_make_list(const void *fit,
					  struct fdt_region *fdt_regions,
					  int count,
					  struct image_region *region);
int fit_config_verify(const void *fit, const void *self);

/* fit_image.c */
int mmap_fdt(const char *cmdname, const char *fname, size_t size_inc,
	     void **blobp, struct stat *sbuf, bool delete_on_error);

/* uboot.c (ruby) */
int rb_uboot_sign(struct image_sign_info *info,
		  const struct image_region region[],
		  int region_count, uint8_t **sigp, uint *sig_len);
int rb_uboot_verify(struct image_sign_info *info,
		    const struct image_region region[], int region_count,
		    uint8_t *sig, uint sig_len);
int rb_uboot_hash(const void *data, int data_len, const char *algo,
		  uint8_t *value, int *value_len);

#endif /* __UBOOT_H__ */
