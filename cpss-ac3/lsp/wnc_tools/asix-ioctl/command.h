/*
 *********************************************************************************
 *     Copyright (c) 2005 ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
 
#ifndef __COMMAND_H__
#define __COMMAND_H__

/* NAMING CONSTANT DECLARATIONS */
#define AX8817XX_SIGNATURE	"AX88179_178A"
#define AX8817XX_DRV_NAME	"AX88179_178A"

/* ioctl Command Definition */
#define AX_PRIVATE		SIOCDEVPRIVATE

/* private Command Definition */
#define AX_SIGNATURE			0
#define AX_READ_REG			1
#define AX_WRITE_REG			2

typedef struct _AX_IOCTL_COMMAND {
	unsigned short	ioctl_cmd;
	unsigned char	sig[16];
	unsigned char	offset;
	unsigned char	*buf;
	unsigned short	size;
	unsigned char	delay;
}AX_IOCTL_COMMAND;

#endif /* end of command.h */
