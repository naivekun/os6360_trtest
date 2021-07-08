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
*/
/**
********************************************************************************
* @file hlib_i2c.c
*
* @brief Routers Host Interface
*
* @version   1
********************************************************************************
*/

/*!**************************************************************************
 *$              Created using CodeUnifier Ver.1.0 (02/25/14 12:12:23)
 *!**************************************************************************
 *!*/
/*!**************************************************RND Template version 4.1
 *!                      P A C K A G E   B O D Y
 *!==========================================================================
 *$ TITLE: Routers Host Interface
 *!--------------------------------------------------------------------------
 *$ FILENAME: hostg\g_host\src\h_lib\i2c\hlib_i2c.c
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM:
 *!--------------------------------------------------------------------------
 *$ AUTHORS: EyalK,MickeyR,orenv,RazA
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 03-Aug-2009, 11:51 AM
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ PROCESS AND ALGORITHM: (local)
 *!
 *$ PACKAGE GLOBAL SERVICES:
 *!     (A list of package global services).
 *!
 *$ PACKAGE LOCAL SERVICES:  (local)
 *!     (A list of package local services).
 *!
 *$ PACKAGE USAGE:
 *!     (How to use the package services,
 *!     routines calling order, restrictions, etc.)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ SIDE EFFECTS:
 *!
 *$ RELATED DOCUMENTS:     (local)
 *!
 *$ REMARKS:               (local)
 *!
 *!**************************************************************************
 *!*/

/*$ ROUTERS_HOST_INTERFACE BODY */

/*! General definitions */


//#include         <inc/defs.h>
/*!**************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT AND EXPORT)
 *!**************************************************************************
 *!*/

/*!**************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include    <linux/i2c.h>
#include    <linux/i2c-dev.h>
#include    <fcntl.h>
#include    <stropts.h>
#include    <string.h>
#include    <stdio.h>

#include    "hlib_i2c.h"
/*!**************************************************************************
 *$              PUBLIC DECLARATIONS
 *!**************************************************************************
 *!*/

/*!**************************************************************************
 *$              PUBLIC VARIABLE DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/

/*!**************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
/*!**************************************************RND Template version 4.1
 *!           L O C A L   V A R I A B L E   D E C L A R A T I O N S
 *!==========================================================================
 *$ TITLE: I2C lib
 *!--------------------------------------------------------------------------
 *$ FILENAME:
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: <SysName>
 *!--------------------------------------------------------------------------
 *$ AUTHORS: orenV
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 12-Jun-2003, 5:32 PM CREATION DATE: 3-Feb-2003
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ INCLUDE REQUIRED:
 *!
 *$ REMARKS:
 *!
 *!**************************************************************************
 *!*/
#define MAX_NUM_I2C_BUSSES      1
#define BUS_ID_0                0
#define I2C_PREFIX_DEV_NAME     "/dev/i2c"
#define MAX_LEN_I2C_MSG         128

static int                  HOSTP_i2c_bus_fd_ARR[MAX_NUM_I2C_BUSSES];



/*!**************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/
/*!**************************************************RND Template version 4.1
*!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
*!==========================================================================
*$ TITLE:
*!--------------------------------------------------------------------------
*$ FILENAME:
*!--------------------------------------------------------------------------
*$ SYSTEM, SUBSYSTEM:
*!--------------------------------------------------------------------------
*$ AUTHORS: AharonG,orenv
*!--------------------------------------------------------------------------
*$ LATEST UPDATE: 3-Feb-2003, 11:51 AM CREATION DATE: 19-Jan-2003
*!**************************************************************************
*/





/**
* @internal HOSTG_i2c_receive function
* @endinternal
*
*/

extern HOSTG_i2c_status_ENT  HOSTG_i2c_receive(
  /*!     INPUTS:             */
  UINT_8     slave_address,   /* the target device slave address on I2C bus */
  UINT_32    buffer_size,     /* buffer length */
  UINT_8     bus_id,          /* the I2C bus id */
  /*!     OUTPUTS:            */
  UINT_8    *buffer           /* received buffer */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    HOSTG_i2c_status_ENT RetVal;
	int ret;
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg msg[1];
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/

	/*
	 * Only a single transaction - receive
	 */
	i2c_data.msgs = msg;
	i2c_data.nmsgs = 1;    /* two i2c_msg */

	i2c_data.msgs[0].addr = slave_address;
	i2c_data.msgs[0].flags = I2C_M_RD;      /* read command */
	i2c_data.msgs[0].len = buffer_size;
	i2c_data.msgs[0].buf = (__u8 *)buffer;

	ret = ioctl(HOSTP_i2c_bus_fd_ARR[bus_id], I2C_RDWR, &i2c_data);
	RetVal = ret < 0 ? HOSTG_I2C_ERROR_E : HOSTG_I2C_SUCCESS_E;

    return RetVal;
}

/*$ END OF FuncName */


/**
* @internal HOSTG_i2c_receive_with_offset function
* @endinternal
*
*/

extern HOSTG_i2c_status_ENT  HOSTG_i2c_receive_with_offset(
  /*!     INPUTS:             */
  UINT_8     slave_address,   /* the target device slave address on I2C bus */
  UINT_32    buffer_size,     /* buffer length */
  UINT_8     bus_id,          /* the I2C bus id */
  HOSTG_i2c_offset_type_ENT  offset_type,            /* receive mode */
  UINT_32    offset,          /* internal offset to read from */
  /*!     OUTPUTS:            */
  UINT_8    *buffer           /* received buffer */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    int ret, msg_index = 0, offset_len = 1;
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg[2];
    char tmp[2];
    char *pTmp = tmp;
    HOSTG_i2c_status_ENT RetVal;

    /* If offset type is 8 or 16 bits,
     * two messages are required within this transaction.
     * First determine offset address
     * Second - receive data.
     */
	i2c_data.msgs = msg;

	i2c_data.msgs[msg_index].addr = slave_address;
	i2c_data.nmsgs = 1;                   /* single i2c_msg */

	switch(offset_type) {
	case HOSTG_i2c_offset_type_none_E:
		break;

	case HOSTG_i2c_offset_type_16_E:
		/* "Write" part of the transaction */
		*pTmp = (char)((offset >> 8) & 0xFF);
        pTmp++;
		offset_len = 2;
	case HOSTG_i2c_offset_type_8_E:
		i2c_data.nmsgs = 2;                   /* two i2c_msg */
		i2c_data.msgs[msg_index].flags = 0;   /* No flags for write command */
		i2c_data.msgs[msg_index].len = offset_len;
		i2c_data.msgs[msg_index].buf =  (__u8 *)tmp;
		*pTmp = (char)(offset & 0xFF);
        msg_index++;
		break;
	}
	/*
	 * "Read" part of the transaction
	 */
	i2c_data.msgs[msg_index].addr = slave_address;
	i2c_data.msgs[msg_index].flags = I2C_M_RD;  /* read command */
	i2c_data.msgs[msg_index].len = buffer_size;
	i2c_data.msgs[msg_index].buf = (__u8 *)buffer;

	ret = ioctl(HOSTP_i2c_bus_fd_ARR[bus_id], I2C_RDWR, &i2c_data);
	RetVal = ret < 0 ? HOSTG_I2C_ERROR_E : HOSTG_I2C_SUCCESS_E;

    return(RetVal);
}

/*$ END OF HOSTG_i2c_receive_with_offset */



extern HOSTG_i2c_status_ENT  HOSTG_i2c_transmit_with_offset(
  /*!     INPUTS:             */
  UINT_8     slave_address,   /* the target device slave address on I2C bus */
  UINT_32    buffer_size,     /* buffer length */
  UINT_8     bus_id,          /* the I2C bus id if only one bus then bus_id=0*/
  HOSTG_i2c_offset_type_ENT  offset_type,            /* receive mode */
  UINT_8    offset,          /* internal offset to write to */
  /*!     OUTPUTS:            */
  UINT_8    *buffer           /* transmited buffer */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    HOSTG_i2c_status_ENT RetVal;
    int ret;
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg;
    uchar buf[MAX_LEN_I2C_MSG];
    uchar *pBuf = buf;

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
	if (buffer_size > MAX_LEN_I2C_MSG -3 )
		return HOSTG_I2C_ERROR_E;

    switch (offset_type) {
    case HOSTG_i2c_offset_type_none_E:
        break;

    case HOSTG_i2c_offset_type_16_E:
        *pBuf = (uchar)((offset >> 8) & 0xFF);
        pBuf++;
        /* fallthrough */
    case HOSTG_i2c_offset_type_8_E:
        *pBuf = (uchar)(offset & 0xFF);
        pBuf++;
    }

    memcpy(pBuf, buffer, buffer_size);
    buffer_size += (pBuf - buf);
	/*
	 * write operation
	 */
	i2c_data.msgs = &msg;
	i2c_data.nmsgs = 1;				/* use one message structure */

	i2c_data.msgs[0].addr = slave_address;
	i2c_data.msgs[0].flags = 0;		/* don't need flags */
	i2c_data.msgs[0].len = buffer_size;
	i2c_data.msgs[0].buf = (__u8 *)buf;

	/*
	 * ioctl() processes read & write.
	 * Operation is determined by flags field of i2c_msg
	 */
    ret = ioctl(HOSTP_i2c_bus_fd_ARR[bus_id], I2C_RDWR, &i2c_data);
	RetVal = ret < 0 ? HOSTG_I2C_ERROR_E : HOSTG_I2C_SUCCESS_E;

    return (RetVal);

}
/*$ END OF FuncName */



extern HOSTG_i2c_status_ENT HOSTC_i2c_init(void)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
  HOSTG_i2c_status_ENT status = HOSTG_I2C_SUCCESS_E;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    int  i;
    char i2c_dev_name[20];

    for (i = 0; i < MAX_NUM_I2C_BUSSES; i++) {
        sprintf(i2c_dev_name, "%s-%d", I2C_PREFIX_DEV_NAME, i);
        HOSTP_i2c_bus_fd_ARR[i] = open(i2c_dev_name, O_RDWR);
        if (HOSTP_i2c_bus_fd_ARR[i] <0) {
            status = HOSTG_I2C_ERROR_E;
            break;
        }
    }

    return(status);
}


MV_U8 tread_msys(MV_U8 addr, int reg, MV_BOOL moreThen256)
{
    HOSTG_i2c_status_ENT st;
    UINT_8    buf;

    st = HOSTG_i2c_receive_with_offset(addr,
                                       1,
                                       BUS_ID_0,
                                       moreThen256 ? HOSTG_i2c_offset_type_16_E : HOSTG_i2c_offset_type_8_E,
                                       reg,
                                       &buf);
    if (st != HOSTG_I2C_SUCCESS_E)
        return MV_ERROR;
    else
        return buf;
}


MV_STATUS twrite_msys(MV_U8 addr, int reg, MV_U8 regVal, MV_BOOL moreThen256)
{
    HOSTG_i2c_status_ENT st;

    st = HOSTG_i2c_transmit_with_offset(addr,
                                         1,
                                         BUS_ID_0,
                                        moreThen256 ? HOSTG_i2c_offset_type_16_E : HOSTG_i2c_offset_type_8_E,
                                        reg,
                                        &regVal);

    if (st != HOSTG_I2C_SUCCESS_E)
        return MV_ERROR;
    else
        return MV_OK;
}







