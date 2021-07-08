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
* @file main.c
*
* @brief SatR utility
*
* @version   1
********************************************************************************
*/

#include <stdio.h>
#include "pps/sar_sw_lib.h"
#include "hlib_i2c.h"


extern struct satr_info* getPP(const char** boardName);

char* CMD_NAME;


int main(int argc, char** argv)
{
    HOSTG_i2c_status_ENT st;
    CMD_NAME = argv[0];
    struct satr_info *pSatr_info;
    const char *devName;

    pSatr_info = getPP(&devName);
    if (!pSatr_info) {
        printf("getPP failed\n");
        return -1;
    }

    st = HOSTC_i2c_init();
    if (st != HOSTG_I2C_SUCCESS_E) {
        printf("HOSTC_i2c_init failed\n");
        return -1;
    }

    do_sar_switch(argc, argv, pSatr_info);

    if (argc == 1)
        printf("%s board\n", devName);

    return 0;
}

