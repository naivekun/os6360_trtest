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
* @file hlib_i2c.h
*
* @brief Routers Host Interface
*
* @version   1
********************************************************************************
*/
#ifndef exp_h_lib_hlib_i2c_h_INCLUDED
#define exp_h_lib_hlib_i2c_h_INCLUDED

#include <defs.h>
#include <mvTypes.h>

typedef enum {
    HOSTG_I2C_SUCCESS_E,
    HOSTG_I2C_ADDRESS_E,
    HOSTG_I2C_ERROR_E,
    HOSTG_I2C_BUFFFULL_E,
    HOSTG_I2C_BUFFEMPTY_E,
    HOSTG_I2C_XMITERROR_E,
    HOSTG_I2C_RCVERROR_E,
    HOSTG_I2C_BUSBUSY_E,
    HOSTG_I2C_ALOSS_E,
    HOSTG_I2C_NOEVENT_E
} HOSTG_i2c_status_ENT;

typedef enum {
    HOSTG_i2c_offset_type_none_E,
    HOSTG_i2c_offset_type_8_E,
    HOSTG_i2c_offset_type_16_E
} HOSTG_i2c_offset_type_ENT;


/*!**************************************************RND Template version 4.1
*!                      P A C K A G E       S P E C I F I C A T I O N
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
/*!**************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT AND EXPORT)
 *!**************************************************************************
 *!*/
 /*!**************************************************************************
 *$              PUBLIC DECLARATIONS (EXPORT)
 *!**************************************************************************
 *!*/
/*!**************************************************************************
 *$              PUBLIC VARIABLE DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/
/*!**************************************************************************
 *$              PUBLIC FUNCTION DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/
/*===========================================================================*/
/**
* @internal HOSTC_i2c_sem_init function
* @endinternal
*
*/
extern void HOSTC_i2c_sem_init (
  /*!     INPUTS:             */
  void
);
/*$ END OF  HOSTC_i2c_sem_init */

/*===========================================================================*/
/*===========================================================================*/
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
);
/*$ END OF  HOSTG_i2c_receive */

/*===========================================================================*/
/*===========================================================================*/
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
);
/*$ END OF  HOSTG_i2c_receive_with_offset */

/*===========================================================================*/
/*===========================================================================*/
/**
* @internal HOSTG_i2c_transmit_with_offset function
* @endinternal
*
*/

extern HOSTG_i2c_status_ENT  HOSTG_i2c_transmit_with_offset(
  /*!     INPUTS:             */
  UINT_8     slave_address,   /* the target device slave address on I2C bus */
  UINT_32    buffer_size,     /* buffer length */
  UINT_8     bus_id,          /* the I2C bus id if only one bus then bus_id=0*/
  HOSTG_i2c_offset_type_ENT  offset_type,            /* receive mode */
  UINT_8    offset,          /* internal offset to write to */
  /*!     OUTPUTS:            */
  UINT_8    *buffer           /* transmited buffer */
);
/*$ END OF  HOSTG_i2c_transmit */

/*===========================================================================*/
/*===========================================================================*/
/**
* @internal HOSTC_i2c_init function
* @endinternal
*
*/

extern HOSTG_i2c_status_ENT HOSTC_i2c_init(void);
/*$ END OF  HOSTC_i2c_init */

/*===========================================================================*/

MV_U8 tread_msys(MV_U8 addr, int reg, MV_BOOL moreThen256);
MV_STATUS twrite_msys(MV_U8 addr, int reg, MV_U8 regVal, MV_BOOL moreThen256);


#endif
/*$ END OF hlib_i2c */


