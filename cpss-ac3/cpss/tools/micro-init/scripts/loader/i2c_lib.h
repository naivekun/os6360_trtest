/*******************************************************************************
*                Copyright 2016, MARVELL SEMICONDUCTOR, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL ISRAEL LTD. (MSIL).                                          *
********************************************************************************
* i2c_lib.h
*******************************************************************************/
#ifndef exp_h_lib_hlib_i2c_h_INCLUDED
#define exp_h_lib_hlib_i2c_h_INCLUDED

#include <defs.h>

enum i2c_offset_type_ENT {
	i2c_offset_type_none_E,
	i2c_offset_type_8_E,
	i2c_offset_type_16_E
};


extern void i2c_sem_init (void);

extern int i2c_receive(
	uint8_t    slave_address,   /* the target device slave address on I2C bus */
	uint32_t   buffer_size,     /* buffer length */
	uint8_t    bus_id,          /* the I2C bus id */
	uint8_t   *buffer           /* received buffer */
);

extern int i2c_receive_with_offset(
	uint8_t     slave_address,   /* the target device slave address on I2C bus */
	uint32_t    buffer_size,     /* buffer length */
	uint8_t     bus_id,          /* the I2C bus id */
	enum        i2c_offset_type_ENT   offset_type,            /* receive mode */
	uint32_t    offset,          /* internal offset to read from */
	uint8_t    *buffer           /* received buffer */
);

extern int i2c_transmit_with_offset(
	uint8_t     slave_address,   /* the target device slave address on I2C bus */
	uint32_t    buffer_size,     /* buffer length */
	uint8_t     bus_id,          /* the I2C bus id if only one bus then bus_id=0*/
	enum        i2c_offset_type_ENT  offset_type,            /* receive mode */
	uint32_t    offset,          /* internal offset to write to */
	uint8_t    *buffer           /* transmited buffer */
);

extern int i2c_init(void);

#endif

