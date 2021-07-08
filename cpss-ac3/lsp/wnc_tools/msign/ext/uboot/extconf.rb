require 'mkmf'

# include 'libfdt'
have_library('fdt', 'fdt_check_header')

# these functions are not always properly exported
have_func('fdt_first_subnode')
have_func('fdt_next_subnode')
have_func('fdt_for_each_subnode')
have_func('fdt_for_each_property_offset')

# get the plain version from Ruby version
require_relative '../../lib/msign/version'
$defs.push("-DPLAIN_VERSION=\\\"msign-#{MSign::VERSION}\\\"")

create_makefile("uboot")
