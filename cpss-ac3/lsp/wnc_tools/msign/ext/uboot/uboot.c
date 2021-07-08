/*
 * Copyright (C) 2017 Cisco Systems, Inc.
 *
 * FOR INTERNAL USE ONLY - DO NOT DISTRIBUTE PUBLICLY
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include "ruby.h"
#include "uboot.h"

VALUE cUboot;

#define MMAP_SIZE_INC 20000
// conveniently make the compiler warnings go away
#define rb_str_new_bytestr(b,l) rb_str_new((const char *)(b),(l))

// C -> Ruby functions (crypto funcs)
int rb_uboot_sign(struct image_sign_info *info,
		  const struct image_region region[], int region_count,
		  uint8_t **sigp, uint *sig_len)
{
	VALUE array;
	VALUE sig;
	int i;

	// 1. Convert C array to Ruby Array
	array = rb_ary_new();
	for (i = 0; i < region_count; i++)
		rb_ary_store(array, i, rb_str_new_bytestr(region[i].data,
							  region[i].size));

	// 2. Call Ruby 'sign_regions'
	sig = rb_funcall((VALUE)info->self, rb_intern("sign_regions"), 3,
			 rb_str_new_cstr(info->checksum->name),
			 rb_str_new_cstr(info->keyname),
			 array);

	if (NIL_P(sig))
		return -1;

	// 3. Convert Ruby String to C string
	*sigp = malloc(RSTRING_LEN(sig));
	memcpy(*sigp, RSTRING_PTR(sig), RSTRING_LEN(sig));
	*sig_len = RSTRING_LEN(sig);

	return 0;
}

int rb_uboot_verify(struct image_sign_info *info,
		    const struct image_region region[], int region_count,
		    uint8_t *sig, uint sig_len)
{
	VALUE array;
	VALUE res;
	int i;

	// 1. Convert C array to Ruby Array
	array = rb_ary_new();
	for (i = 0; i < region_count; i++)
		rb_ary_store(array, i, rb_str_new_bytestr(region[i].data,
							  region[i].size));

	// 2. Call Ruby 'verify_regions'
	res = rb_funcall((VALUE)info->self, rb_intern("verify_regions"), 4,
			 rb_str_new_cstr(info->checksum->name),
			 rb_str_new_cstr(info->keyname),
			 rb_str_new_bytestr(sig, sig_len),
			 array);

	return (res == Qtrue) ? 0 : -1;
}

int rb_uboot_hash(const void *data, int data_len, const char *algo,
		  uint8_t *value, int *value_len)
{
	VALUE array;
	VALUE hash;

	// 1. Convert C data to Ruby Array
	array = rb_ary_new();
	rb_ary_store(array, 0, rb_str_new_bytestr(data, data_len));

	// 2. Call Ruby 'hash_regions'
	hash = rb_funcall(cUboot, rb_intern("hash_regions"), 2,
			  rb_str_new_cstr(algo),
			  array);

	if (NIL_P(hash))
		return -1;

	// 3. Convert Ruby String to C string
	memcpy(value, RSTRING_PTR(hash), RSTRING_LEN(hash));
	*value_len = RSTRING_LEN(hash);

	return 0;
}

// Public functions for Ruby
static VALUE msign_uboot_sign(VALUE self, VALUE file)
{
	// 1. validate input
	if (TYPE(file) != T_STRING)
		rb_raise(rb_eTypeError, "file is not a string");

	// 2. mmap the file
	void *blob;
	struct stat st;
	int fd = mmap_fdt("msign", rb_string_value_cstr(&file),
			  MMAP_SIZE_INC, &blob, &st, false);

	if (fd < 0)
		rb_raise(rb_eIOError, "could not mmap %s",
			 rb_string_value_cstr(&file));

	// 3. sign
	if (fit_config_sign(blob, NULL, (const void*)self))
		rb_raise(rb_eStandardError, "failed to sign the config");

	// 4. pack and truncate
	// XXX this step is NOT compatible with external data
	fdt_pack(blob);

	int size = fdt_totalsize(blob);
	munmap(blob, size);
	if (ftruncate(fd, size))
		rb_raise(rb_eIOError, "failed to truncate");

	close(fd);
	return Qnil;
}

static VALUE msign_uboot_verify(VALUE self, VALUE file)
{
	// 1. validate input
	if (TYPE(file) != T_STRING)
		rb_raise(rb_eTypeError, "file is not a string");

	// 2. mmap the file
	void *blob;
	struct stat st;
	int fd = mmap_fdt("msign", rb_string_value_cstr(&file),
			  0, &blob, &st, false);

	if (fd < 0)
		rb_raise(rb_eIOError, "could not mmap %s",
			 rb_string_value_cstr(&file));

	// 3. verify
	if (fit_config_verify(blob, (const void*)self))
		rb_raise(rb_eStandardError, "failed to verify the config");

	int size = fdt_totalsize(blob);
	munmap(blob, size);
	close(fd);
	return Qnil;
}

static VALUE msign_uboot_hash(VALUE self, VALUE file)
{
	// 1. validate input
	if (TYPE(file) != T_STRING)
		rb_raise(rb_eTypeError, "file is not a string");

	VALUE algo = rb_iv_get(self, "@algo");
	if (TYPE(algo) != T_STRING)
		rb_raise(rb_eTypeError, "algo is not a string");

	// 2. mmap the file
	void *blob;
	struct stat st;
	int fd = mmap_fdt("msign", rb_string_value_cstr(&file),
			  MMAP_SIZE_INC, &blob, &st, false);

	if (fd < 0)
		rb_raise(rb_eIOError, "could not mmap %s",
			 rb_string_value_cstr(&file));

	// 3. add hashes
	if (fit_image_hash(blob, rb_string_value_cstr(&algo)))
		rb_raise(rb_eStandardError, "failed to hash images");

	// 4. pack and truncate
	// XXX this step is NOT compatible with external data
	fdt_pack(blob);

	int size = fdt_totalsize(blob);
	munmap(blob, size);
	if (ftruncate(fd, size))
		rb_raise(rb_eIOError, "failed to truncate");

	close(fd);
	return Qnil;
}

void Init_uboot() {
	cUboot = rb_define_class_under(rb_define_module("MSign"), "Uboot", rb_cObject);
	rb_define_method(cUboot, "sign", msign_uboot_sign, 1);
	rb_define_method(cUboot, "verify", msign_uboot_verify, 1);
	rb_define_method(cUboot, "hash", msign_uboot_hash, 1);
}
