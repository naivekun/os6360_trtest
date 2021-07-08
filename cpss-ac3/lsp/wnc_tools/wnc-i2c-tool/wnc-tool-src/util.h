/*
    util - helper functions
    Copyright (C) 2006-2009  Jean Delvare <jdelvare@suse.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
*/

#ifndef _UTIL_H
#define _UTIL_H

#define MODE_AUTO	0
#define MODE_QUICK	1
#define MODE_READ	2


void intHandler(int dummy);
void wnc_i2c_dev_open(void);
void wnc_i2c_dev_close(void);
int scan_i2c_bus(int mode, int first, int last);
int check_i2c_address(int mode, int addr);
void wnc_i2c_dev_scan(void);
int  wnc_i2c_dev_write_byte_data(int address, unsigned char daddress, unsigned char val);
int wnc_i2c_dev_read_byte_data(unsigned char address, unsigned char daddress);
char* wnc_i2c_dev_read_bytes(unsigned char address, unsigned char daddress, int bytes, int show_hex);
char* wnc_i2c_dev_read_char(unsigned char address, unsigned char daddress, int bytes);
char* wnc_i2c_dev_read_bytes_inc(unsigned char address, unsigned char daddress, int bytes, int show_hex);
unsigned int log2_(int n);
int write_to_device(unsigned int addr, char addr_hi, char addr_lo, unsigned char * buf, int len);
int read_from_device(unsigned int addr, char addr_hi, char addr_lo, unsigned char * buf, int len);
extern int user_ack(int def);

#endif /* _UTIL_H */
