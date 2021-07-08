/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
*
* mvHwsDdr3A38x.h
*
* DESCRIPTION:
*
*******************************************************************************/
#ifndef __mvHwsDdr3_A38x_H
#define __mvHwsDdr3_A38x_H

/* Termal Sensor Registers */
#define TSEN_CONTROL_LSB_REG                    0xE4070
#define TSEN_CONTROL_LSB_TC_TRIM_OFFSET             0
#define TSEN_CONTROL_LSB_TC_TRIM_MASK               (0x7 << TSEN_CONTROL_LSB_TC_TRIM_OFFSET)
#define TSEN_CONTROL_MSB_REG                    0xE4074
#define TSEN_CONTROL_MSB_RST_OFFSET             8
#define TSEN_CONTROL_MSB_RST_MASK               (0x1 << TSEN_CONTROL_MSB_RST_OFFSET)
#define TSEN_STATUS_REG                     0xE4078
#define TSEN_STATUS_READOUT_VALID_OFFSET    10
#define TSEN_STATUS_READOUT_VALID_MASK      (0x1 << TSEN_STATUS_READOUT_VALID_OFFSET)
#define TSEN_STATUS_TEMP_OUT_OFFSET         0
#define TSEN_STATUS_TEMP_OUT_MASK           (0x3FF << TSEN_STATUS_TEMP_OUT_OFFSET)

/* device ID and revision */
#define DEV_ID_REG                  0x18238
#define DEV_VERSION_ID_REG          0x1823C
#define REVISON_ID_OFFS             8
#define REVISON_ID_MASK             0xF00

#endif /*__mvHwsDdr3_A38x_H*/

