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
* @file cpssDrvPpIntDefDxChAldrin2.c
*
* @brief This file includes the definition and initialization of the interrupts
* init. parameters tables. -- Aldrin2 devices
*
* @version   1
********************************************************************************
*/

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterruptsInit.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxEventsAldrin2.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxUtilLion.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddr.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>

#define ALDRIN2_GOP_PIPE_OFFSET_MAC     0x400000

extern GT_STATUS prvCpssDrvBobKXlgGopExtUnitsIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
);

GT_STATUS prvCpssDrvAldrin2XlgGopExtUnitsIsrRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
)
{
    GT_U32 port;
    GT_STATUS rc;
    GT_U32  mgChainId;

    rc = prvCpssDrvBobKXlgGopExtUnitsIsrRead(devNum, portGroupId, regAddr, dataPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    port = (regAddr >> 12) & 0x3F;/* 'local to MG chain' port number is 0..63 */

    if(regAddr & ALDRIN2_GOP_PIPE_OFFSET_MAC)
    {
        mgChainId = 1;
    }
    else
    {
        mgChainId = 0;
    }

    /* Check CG port interrupt summary cause (only on chain 0) */
    if ((mgChainId == 0) && (port % 4) == 0)
    {
        /* no need to read the CG register because is set ... we will read it again by the 'scan'
            that will make it read 'twice' (redundant!) */
        /* and if not read here ... we will read only once  by the 'scan' */

        /* CG port may got interrupt too. Add this indication to output XLG register dummy bit#8 */
        *dataPtr |= BIT_8 | BIT_0;
    }

    return GT_OK;
}

/* state for 73 ports that event hold extra param as : ((portNum) << port_offset) |  ext_param */
#define SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset,portNum) \
    SET_EVENT_PER_PORT__WITH_PARAM_MAC(_prefix,_postFix, portNum , ((portNum << port_offset) | (ext_param)))

/* state for 73 ports that event hold extra param as : ((portNum) << port_offset) |  ext_param */
#define SET_EVENT_PER_73_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix, _postFix, ext_param, port_offset)   \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset,  0 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset,  1 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset,  2 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset,  3 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset,  4 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset,  5 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset,  6 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset,  7 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset,  8 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset,  9 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 10 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 11 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 12 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 13 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 14 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 15 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 16 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 17 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 18 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 19 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 20 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 21 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 22 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 23 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 24 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 25 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 26 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 27 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 28 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 29 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 30 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 31 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 32 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 33 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 34 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 35 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 36 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 37 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 38 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 39 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 40 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 41 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 42 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 43 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 44 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 45 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 46 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 47 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 48 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 49 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 50 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 51 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 52 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 53 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 54 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 55 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 56 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 57 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 58 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 59 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 60 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 61 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 62 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 63 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 64 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 65 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 66 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 67 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 68 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 69 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 70 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 71 ) ,    \
    SET_EVENT_PER_PORT_WITH_PORT_OFFSET_AND_PARAM_MAC(_prefix,_postFix,ext_param,port_offset, 72 )

/* state for ports 0..71 that event hold extra port number */
#define SET_EVENT_PER_72_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,        0    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,        1    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,        2    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,        3    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,        4    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,        5    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,        6    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,        7    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,        8    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,        9    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       10    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       11    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       12    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       13    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       14    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       15    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       16    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       17    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       18    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       19    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       20    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       21    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       22    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       23    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       24    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       25    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       26    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       27    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       28    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       29    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       30    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       31    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       32    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       33    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       34    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       35    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       36    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       37    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       38    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       39    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       40    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       41    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       42    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       43    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       44    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       45    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       46    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       47    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       48    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       49    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       50    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       51    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       52    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       53    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       54    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       55    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       56    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       57    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       58    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       59    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       60    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       61    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       62    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       63    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       64    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       65    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       66    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       67    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       68    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       69    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       70    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       71    )

/* state for ports 0..72 that event hold extra port number */
#define SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    /* the first 72 ports */                                        \
    SET_EVENT_PER_72_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix),    \
    /* port 72 */                                                   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       72    )



/* state for a port that event hold extra port number */
/*__prefix - is between 'device' and "_PORT" */
/*_postFix - must include the "_E" */
/*convert local TXQ DQ port to global TXQ port using txq index*/
#define SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,_portNum, _dq)   \
    _prefix##_##PORT##_##_portNum##_##_postFix,       MARK_PER_PORT_INT_MAC((_portNum) + (_dq)*25)

/* state for ALL 0..24 ports that event hold extra port number . convert local TXQ ports to global TXQ ports*/
#define SET_EVENT_PER_25_PORTS_FOR_DQ_MAC(_prefix,_postFix, _dq)        \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     0, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     1, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     2, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     3, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     4, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     5, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     6, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     7, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     8, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,     9, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    10, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    11, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    12, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    13, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    14, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    15, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    16, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    17, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    18, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    19, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    20, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    21, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    22, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    23, _dq ), \
    SET_EVENT_PER_PORT_FOR_DQ_MAC(_prefix,_postFix,    24, _dq )

/* state for ALL 0..99 ports that event hold extra port number */
#define SET_EVENT_PER_100_PORTS_MAC(_prefix,_postFix)       \
    /* the first 73 ports */                                \
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix), \
    /* the ports 73..99 */                                  \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       73    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       74    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       75    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       76    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       77    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       78    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       79    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       80    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       81    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       82    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       83    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       84    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       85    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       86    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       87    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       88    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       89    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       90    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       91    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       92    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       93    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       94    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       95    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       96    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       97    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       98    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       99    )

/* state for ALL 0..127 ports that event hold extra port number */
#define SET_EVENT_PER_128_PORTS_MAC(_prefix,_postFix)       \
    /* the first 100 ports */                               \
    SET_EVENT_PER_100_PORTS_MAC(_prefix,_postFix),          \
    /* the ports 100..127 */                                \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      100    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      101    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      102    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      103    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      104    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      105    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      106    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      107    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      108    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      109    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      110    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      111    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      112    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      113    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      114    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      115    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      116    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      117    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      118    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      119    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      120    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      121    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      122    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      123    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      124    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      125    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      126    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      127    )

/* state for ports 0..91 that event hold extra port number */
#define SET_EVENT_PER_92_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       73    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       74    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       75    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       76    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       77    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       78    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       79    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       80    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       81    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       82    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       83    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       84    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       85    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       86    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       87    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       88    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       89    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       90    ), \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,       91    )

/* state for CG ports that event hold extra index equal to event */
#define SET_INDEX_EQUAL_EVENT_PER_CG_PORTS___ON_ALL_PORTS_MAC(_prefix,_postFix)   \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      0     ),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      4     ),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,      8     ),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     12     ),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     16     ),    \
    SET_EVENT_PER_PORT_MAC(_prefix,_postFix,     20     )

#define SET_EVENT_PER_INDEX____0_24___MAC(_prefix,_postFix)   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     0       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     1       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     2       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     3       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     4       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     5       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     6       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     7       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     8       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     9       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     10      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     11      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     12      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     13      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     14      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     15      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     16      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     17      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     18      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     19      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     20      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     21      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     22      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     23      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     24      )

#define SET_EVENT_PER_INDEX____0_127___MAC(_prefix,_postFix)   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     0       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     1       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     2       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     3       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     4       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     5       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     6       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     7       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     8       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     9       ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     10      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     11      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     12      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     13      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     14      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     15      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     16      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     17      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     18      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     19      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     20      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     21      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     22      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     23      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     24      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     25      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     26      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     27      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     28      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     29      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     30      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     31      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     32      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     33      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     34      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     35      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     36      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     37      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     38      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     39      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     40      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     41      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     42      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     43      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     44      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     45      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     46      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     47      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     56      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     57      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     58      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     59      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     64      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     65      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     66      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     67      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     68      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     69      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     70      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     71      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     72      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     73      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     74      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     75      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     76      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     77      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     78      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     79      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     80      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     81      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     82      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     83      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     84      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     85      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     86      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     87      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     88      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     89      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     90      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     91      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     92      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     93      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     94      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     95      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     96      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     97      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     98      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     99      ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     100     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     101     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     102     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     103     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     104     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     105     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     106     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     107     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     108     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     109     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     110     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     111     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     112     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     113     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     114     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     115     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     116     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     117     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     118     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     119     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     120     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     121     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     122     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     123     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     124     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     125     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     126     ),   \
            SET_EVENT_PER_INDEX_MAC(_prefix,_postFix,     127     )

/* get the first value of event in register (clear its 5 LSBits)*/
#define GET_FIRST_EVENT_IN_REG_MAC(_event)  ((_event) & (~0x1f))
/* the full 32 events for event that belongs to a register ,
   will have index equal to the 32 events of the register */
#define SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(_event) \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 0 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 1 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 2 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 3 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 4 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 5 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 6 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 7 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 8 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 9 ),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 10),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 11),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 12),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 13),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 14),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 15),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 16),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 17),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 18),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 19),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 20),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 21),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 22),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 23),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 24),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 25),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 26),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 27),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 28),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 29),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 30),   \
    SET_INDEX_EQUAL_EVENT_MAC(GET_FIRST_EVENT_IN_REG_MAC(_event) + 31)

/* Interrupt cause to unified event map for Aldrin2
 This Table is for unified event with extended data

  Array structure:
  1. The first element of the array and the elements after MARK_END_OF_UNI_EV_CNS are unified event types.
  2. Elements after unified event type are pairs of interrupt cause and event extended data,
     until MARK_END_OF_UNI_EV_CNS.
*/
static GT_U32 aldrin2UniEvMapTableWithExtData[] = {
 /* Per Queue events */
 CPSS_PP_TX_BUFFER_QUEUE_E,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_BUFFER_QUEUE_0_E,  0,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_BUFFER_QUEUE_1_E,  1,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_BUFFER_QUEUE_2_E,  2,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_BUFFER_QUEUE_3_E,  3,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_BUFFER_QUEUE_4_E,  4,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_BUFFER_QUEUE_5_E,  5,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_BUFFER_QUEUE_6_E,  6,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_BUFFER_QUEUE_7_E,  7,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_BUFFER_QUEUE_0_E,  8,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_BUFFER_QUEUE_1_E,  9,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_BUFFER_QUEUE_2_E,  10,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_BUFFER_QUEUE_3_E,  11,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_BUFFER_QUEUE_4_E,  12,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_BUFFER_QUEUE_5_E,  13,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_BUFFER_QUEUE_6_E,  14,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_BUFFER_QUEUE_7_E,  15,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_BUFFER_QUEUE_0_E,  16,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_BUFFER_QUEUE_1_E,  17,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_BUFFER_QUEUE_2_E,  18,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_BUFFER_QUEUE_3_E,  19,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_BUFFER_QUEUE_4_E,  20,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_BUFFER_QUEUE_5_E,  21,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_BUFFER_QUEUE_6_E,  22,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_BUFFER_QUEUE_7_E,  23,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_BUFFER_QUEUE_0_E,  24,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_BUFFER_QUEUE_1_E,  25,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_BUFFER_QUEUE_2_E,  26,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_BUFFER_QUEUE_3_E,  27,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_BUFFER_QUEUE_4_E,  28,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_BUFFER_QUEUE_5_E,  29,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_BUFFER_QUEUE_6_E,  30,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_BUFFER_QUEUE_7_E,  31,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TX_ERR_QUEUE_E,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_ERROR_QUEUE_0_E,  0,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_ERROR_QUEUE_1_E,  1,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_ERROR_QUEUE_2_E,  2,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_ERROR_QUEUE_3_E,  3,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_ERROR_QUEUE_4_E,  4,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_ERROR_QUEUE_5_E,  5,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_ERROR_QUEUE_6_E,  6,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_ERROR_QUEUE_7_E,  7,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_ERROR_QUEUE_0_E,  8,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_ERROR_QUEUE_1_E,  9,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_ERROR_QUEUE_2_E,  10,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_ERROR_QUEUE_3_E,  11,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_ERROR_QUEUE_4_E,  12,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_ERROR_QUEUE_5_E,  13,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_ERROR_QUEUE_6_E,  14,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_ERROR_QUEUE_7_E,  15,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_ERROR_QUEUE_0_E,  16,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_ERROR_QUEUE_1_E,  17,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_ERROR_QUEUE_2_E,  18,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_ERROR_QUEUE_3_E,  19,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_ERROR_QUEUE_4_E,  20,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_ERROR_QUEUE_5_E,  21,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_ERROR_QUEUE_6_E,  22,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_ERROR_QUEUE_7_E,  23,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_ERROR_QUEUE_0_E,  24,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_ERROR_QUEUE_1_E,  25,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_ERROR_QUEUE_2_E,  26,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_ERROR_QUEUE_3_E,  27,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_ERROR_QUEUE_4_E,  28,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_ERROR_QUEUE_5_E,  29,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_ERROR_QUEUE_6_E,  30,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_ERROR_QUEUE_7_E,  31,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TX_END_E,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_END_QUEUE_0_E,  0,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_END_QUEUE_1_E,  1,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_END_QUEUE_2_E,  2,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_END_QUEUE_3_E,  3,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_END_QUEUE_4_E,  4,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_END_QUEUE_5_E,  5,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_END_QUEUE_6_E,  6,
    PRV_CPSS_ALDRIN2_TX_SDMA_TX_END_QUEUE_7_E,  7,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_END_QUEUE_0_E,  8,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_END_QUEUE_1_E,  9,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_END_QUEUE_2_E,  10,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_END_QUEUE_3_E,  11,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_END_QUEUE_4_E,  12,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_END_QUEUE_5_E,  13,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_END_QUEUE_6_E,  14,
    PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_END_QUEUE_7_E,  15,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_END_QUEUE_0_E,  16,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_END_QUEUE_1_E,  17,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_END_QUEUE_2_E,  18,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_END_QUEUE_3_E,  19,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_END_QUEUE_4_E,  20,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_END_QUEUE_5_E,  21,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_END_QUEUE_6_E,  22,
    PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_END_QUEUE_7_E,  23,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_END_QUEUE_0_E,  24,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_END_QUEUE_1_E,  25,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_END_QUEUE_2_E,  26,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_END_QUEUE_3_E,  27,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_END_QUEUE_4_E,  28,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_END_QUEUE_5_E,  29,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_END_QUEUE_6_E,  30,
    PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_END_QUEUE_7_E,  31,
 MARK_END_OF_UNI_EV_CNS,


CPSS_PP_GTS_GLOBAL_FIFO_FULL_E,
    PRV_CPSS_ALDRIN2_ERMRK_SUM_INGRESS_TIMESTAMP_QUEUE0_FULL_E,  0,
    PRV_CPSS_ALDRIN2_ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE0_FULL_E,   1,
    PRV_CPSS_ALDRIN2_ERMRK_SUM_INGRESS_TIMESTAMP_QUEUE1_FULL_E,  2,
    PRV_CPSS_ALDRIN2_ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE1_FULL_E,   3,
    SET_EVENT_PER_73_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN2_PTP,TIMESTAMP_QUEUE0_FULL_E,GTS_INT_EXT_PARAM_MAC(1,0,1),8/*port<<8*/),
    SET_EVENT_PER_73_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN2_PTP,TIMESTAMP_QUEUE1_FULL_E,GTS_INT_EXT_PARAM_MAC(1,1,1),8/*port<<8*/),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,
    PRV_CPSS_ALDRIN2_ERMRK_SUM_NEW_INGRESS_TIMESTAMP_Q0_E,  0,
    PRV_CPSS_ALDRIN2_ERMRK_SUM_NEW_EGRESS_TIMESTAMP_Q0_E,   1,
    PRV_CPSS_ALDRIN2_ERMRK_SUM_NEW_INGRESS_TIMESTAMP_Q1_E,  2,
    PRV_CPSS_ALDRIN2_ERMRK_SUM_NEW_EGRESS_TIMESTAMP_Q1_E,   3,
    SET_EVENT_PER_73_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN2_PTP,NEW_TIMESTAMP_QUEUE0_E,GTS_INT_EXT_PARAM_MAC(1,0,1),8/*port<<8*/),
    SET_EVENT_PER_73_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN2_PTP,NEW_TIMESTAMP_QUEUE1_E,GTS_INT_EXT_PARAM_MAC(1,1,1),8/*port<<8*/),
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_RX_BUFFER_QUEUE0_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_BUFFER_QUEUE_0_E,     0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE1_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_BUFFER_QUEUE_1_E,     1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE2_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_BUFFER_QUEUE_2_E,     2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE3_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_BUFFER_QUEUE_3_E,     3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE4_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_BUFFER_QUEUE_4_E,     4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE5_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_BUFFER_QUEUE_5_E,     5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE6_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_BUFFER_QUEUE_6_E,     6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_BUFFER_QUEUE7_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_BUFFER_QUEUE_7_E,     7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE0_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_ERROR_QUEUE_0_E,      0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE1_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_ERROR_QUEUE_1_E,      1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE2_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_ERROR_QUEUE_2_E,      2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE3_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_ERROR_QUEUE_3_E,      3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE4_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_ERROR_QUEUE_4_E,      4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE5_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_ERROR_QUEUE_5_E,      5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE6_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_ERROR_QUEUE_6_E,      6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_ERR_QUEUE7_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RX_ERROR_QUEUE_7_E,      7,
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_MAC_SFLOW_E,
    SET_EVENT_PER_128_PORTS_MAC(PRV_CPSS_ALDRIN2_INGRESS_STC, INGRESS_SAMPLE_LOADED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_EGRESS_SFLOW_E,
    SET_EVENT_PER_25_PORTS_FOR_DQ_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ0_EGR_STC,E,0),
    SET_EVENT_PER_25_PORTS_FOR_DQ_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ1_EGR_STC,E,1),
    SET_EVENT_PER_25_PORTS_FOR_DQ_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ2_EGR_STC,E,2),
    SET_EVENT_PER_25_PORTS_FOR_DQ_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ3_EGR_STC,E,3),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_TXQ2_FLUSH_PORT_E,
    SET_EVENT_PER_25_PORTS_FOR_DQ_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ0_FLUSH_DONE_SUM,E,0),
    SET_EVENT_PER_25_PORTS_FOR_DQ_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ1_FLUSH_DONE_SUM,E,1),
    SET_EVENT_PER_25_PORTS_FOR_DQ_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ2_FLUSH_DONE_SUM,E,2),
    SET_EVENT_PER_25_PORTS_FOR_DQ_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ3_FLUSH_DONE_SUM,E,3),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_AN_COMPLETED_E,
    /* GIGA ports 0..72 */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_GIGA,AN_COMPLETED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_EEE_E,
    /* GIGA ports 0..72 */
    SET_EVENT_PER_73_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN2_GIGA,PCS_RX_PATH_RECEIVED_LPI_E,0 ,8/*port<<8*/),
    SET_EVENT_PER_73_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN2_GIGA,PCS_TX_PATH_RECEIVED_LPI_E,1 ,8/*port<<8*/),
    SET_EVENT_PER_73_PORTS_WITH_PORT_OFFSET_AND_PARAM_MAC(PRV_CPSS_ALDRIN2_GIGA,MAC_RX_PATH_RECEIVED_LPI_E,2 ,8/*port<<8*/),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_SYNC_STATUS_CHANGED_E,
    /* GIGA ports */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_GIGA,SYNC_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
    /* XLG ports 0..36 */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_MPCS,SIGNAL_DETECT_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
    /* XLG ports */
   SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_MPCS,ALIGN_LOCK_LOST_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
    /* XLG ports */
   SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_MPCS,GB_LOCK_SYNC_CHANGE_E),
 MARK_END_OF_UNI_EV_CNS,

  CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
    /* GIGA ports 0..72 */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_GIGA,LINK_STATUS_CHANGED_E),
    /* XLG ports 0..72 */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_XLG,LINK_STATUS_CHANGED_E),
    /* CG ports 0,4,8,12,16,20 */
    SET_INDEX_EQUAL_EVENT_PER_CG_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_CG, LINK_STATUS_CHANGED_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_RX_FIFO_OVERRUN_E,
    /* GIGA ports 0..72 */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_GIGA,RX_FIFO_OVERRUN_E),
    /* XLG ports 0..72 */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_XLG,RX_FIFO_OVERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_UNDERRUN_E,
    /* GIGA ports 0..72 */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_GIGA,TX_UNDERRUN_E),
    /* XLG ports 0..72 */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_XLG,TX_UNDERRUN_E),
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_PORT_PRBS_ERROR_E,
    /* GIGA ports 0..72 */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_GIGA,PRBS_ERROR_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_FIFO_OVERRUN_E,
    /* GIGA ports 0..72 */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_GIGA,TX_FIFO_OVERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
    /* GIGA ports 0..72 */
    SET_EVENT_PER_73_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_GIGA,TX_FIFO_UNDERRUN_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_PORT_DESC_FULL_E,
    SET_EVENT_PER_92_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_TXQ_PORT_DESC_SUM_DESC_FULL,E),
 MARK_END_OF_UNI_EV_CNS,


  /* Per Index events */
 CPSS_PP_GPP_E,
    PRV_CPSS_ALDRIN2_MISC_GPP_0_E,  0,
    PRV_CPSS_ALDRIN2_MISC_GPP_1_E,  1,
    PRV_CPSS_ALDRIN2_MISC_GPP_2_E,  2,
    PRV_CPSS_ALDRIN2_MISC_GPP_3_E,  3,
    PRV_CPSS_ALDRIN2_MISC_GPP_4_E,  4,
    PRV_CPSS_ALDRIN2_MISC_GPP_5_E,  5,
    PRV_CPSS_ALDRIN2_MISC_GPP_6_E,  6,
    PRV_CPSS_ALDRIN2_MISC_GPP_7_E,  7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,    0,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK1_WRAPAROUND_E,    1,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK2_WRAPAROUND_E,    2,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK3_WRAPAROUND_E,    3,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK4_WRAPAROUND_E,    4,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK5_WRAPAROUND_E,    5,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK6_WRAPAROUND_E,    6,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK7_WRAPAROUND_E,    7,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK8_WRAPAROUND_E,    8,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK9_WRAPAROUND_E,    9,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK10_WRAPAROUND_E,  10,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK11_WRAPAROUND_E,  11,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK12_WRAPAROUND_E,  12,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK13_WRAPAROUND_E,  13,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK14_WRAPAROUND_E,  14,
    PRV_CPSS_ALDRIN2_CNC0_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  15,

    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,   16,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK1_WRAPAROUND_E,   17,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK2_WRAPAROUND_E,   18,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK3_WRAPAROUND_E,   19,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK4_WRAPAROUND_E,   20,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK5_WRAPAROUND_E,   21,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK6_WRAPAROUND_E,   22,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK7_WRAPAROUND_E,   23,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK8_WRAPAROUND_E,   24,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK9_WRAPAROUND_E,   25,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK10_WRAPAROUND_E,  26,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK11_WRAPAROUND_E,  27,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK12_WRAPAROUND_E,  28,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK13_WRAPAROUND_E,  29,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK14_WRAPAROUND_E,  30,
    PRV_CPSS_ALDRIN2_CNC1_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  31,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_BM_MISC_E,
    PRV_CPSS_ALDRIN2_BM_GEN1_SUM_GLOBAL_RX_FULL_E,           4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_RX_CNTR_OVERFLOW_E,
    PRV_CPSS_ALDRIN2_RX_SDMA_RESOURCE_ERROR_CNT_OF_E,   0,
    PRV_CPSS_ALDRIN2_RX_SDMA_BYTE_CNT_OF_E,             1,
    PRV_CPSS_ALDRIN2_RX_SDMA_PACKET_CNT_OF_E,           2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_TQ_MISC_E,
    PRV_CPSS_ALDRIN2_TXQ_GEN_SUM_QUEUE_WRONG_ADDR_E,    0,
    PRV_CPSS_ALDRIN2_TXQ_GEN_SUM_GLOBAL_DESC_FULL_E,    1,
    PRV_CPSS_ALDRIN2_TXQ_GEN_SUM_GLOBAL_BUFF_FULL_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_SCT_RATE_LIMITER_E,
    SET_EVENT_PER_INDEX____0_127___MAC(PRV_CPSS_ALDRIN2_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER,PKT_DROPED_E),
 MARK_END_OF_UNI_EV_CNS,

  CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,
    PRV_CPSS_ALDRIN2_IPLR0_ADDRESS_OUT_OF_MEMORY_E,       0,
    PRV_CPSS_ALDRIN2_EPLR_SUM_ADDRESS_OUT_OF_MEMORY_E,    1,
    PRV_CPSS_ALDRIN2_IPLR1_SUM_ADDRESS_OUT_OF_MEMORY_E,   2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_DATA_ERR_E,
    PRV_CPSS_ALDRIN2_EPLR_SUM_DATA_ERROR_E,           1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E,
    PRV_CPSS_ALDRIN2_IPLR0_IPFIX_WRAPAROUND_E,        0,
    PRV_CPSS_ALDRIN2_EPLR_SUM_IPFIX_WRAPAROUND_E,     1,
    PRV_CPSS_ALDRIN2_IPLR1_SUM_IPFIX_WRAPAROUND_E,    2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_ALARM_E,
    PRV_CPSS_ALDRIN2_IPLR0_IPFIX_ALARM_E,             0,
    PRV_CPSS_ALDRIN2_EPLR_SUM_IPFIX_ALARM_E,          1,
    PRV_CPSS_ALDRIN2_IPLR1_SUM_IPFIX_ALARM_E,         2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,
    PRV_CPSS_ALDRIN2_IPLR0_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,       0,
    PRV_CPSS_ALDRIN2_EPLR_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    1,
    PRV_CPSS_ALDRIN2_IPLR1_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,   2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL0_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL0_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL0_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 0,
    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL0_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 0,

    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL1_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL1_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL1_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 1,
    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL1_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 1,

    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL2_ACTION0_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL2_ACTION1_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL2_ACTION2_TCAM_ACCESS_DATA_ERROR_E  , 2,
    PRV_CPSS_ALDRIN2_IPCL_SUM_PCL2_ACTION3_TCAM_ACCESS_DATA_ERROR_E  , 2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PCL_LOOKUP_FIFO_FULL_E,
    PRV_CPSS_ALDRIN2_IPCL_SUM_LOOKUP0_FIFO_FULL_E,        0,
    PRV_CPSS_ALDRIN2_IPCL_SUM_LOOKUP1_FIFO_FULL_E,        1,
    PRV_CPSS_ALDRIN2_IPCL_SUM_LOOKUP2_FIFO_FULL_E,        2,
MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_EB_NA_FIFO_FULL_E,
    PRV_CPSS_ALDRIN2_FDB_NA_FIFO_FULL0_E    , 0 ,
    PRV_CPSS_ALDRIN2_FDB_NA_FIFO_FULL1_E    , 1 ,
    PRV_CPSS_ALDRIN2_FDB_NA_FIFO_FULL2_E    , 2 ,
    PRV_CPSS_ALDRIN2_FDB_NA_FIFO_FULL3_E    , 3 ,
    PRV_CPSS_ALDRIN2_FDB_NA_FIFO_FULL4_E    , 4 ,
    PRV_CPSS_ALDRIN2_FDB_NA_FIFO_FULL5_E    , 5 ,
    PRV_CPSS_ALDRIN2_FDB_NA_FIFO_FULL6_E    , 6 ,
    PRV_CPSS_ALDRIN2_FDB_NA_FIFO_FULL7_E    , 7 ,
 MARK_END_OF_UNI_EV_CNS,


 CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
    PRV_CPSS_ALDRIN2_TTI_DATA_ERROR_INT0_E, 0,
    PRV_CPSS_ALDRIN2_TTI_DATA_ERROR_INT1_E, 1,
    PRV_CPSS_ALDRIN2_TTI_DATA_ERROR_INT2_E, 2,
    PRV_CPSS_ALDRIN2_TTI_DATA_ERROR_INT3_E, 3,
 MARK_END_OF_UNI_EV_CNS,

 /* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */
 CPSS_PP_CRITICAL_HW_ERROR_E,
/* The following event is in valid traffic case. The event is when allocation counter get to total buffers limit.
   This is legal situation for stress test scenarios.
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PIPE0_RX_DMA0_SUM_GLOBAL_ALLOC_COUNTER_OVERFLOW_E), */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_RX_DMA0_INT0_CREDIT_COUNTER_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_RX_DMA0_INT0_IBUF_FIFO_OUT_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_RX_DMA1_INT0_CREDIT_COUNTER_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_RX_DMA1_INT0_IBUF_FIFO_OUT_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_RX_DMA2_INT0_CREDIT_COUNTER_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_RX_DMA2_INT0_IBUF_FIFO_OUT_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_RX_DMA3_INT0_CREDIT_COUNTER_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_RX_DMA3_INT0_IBUF_FIFO_OUT_OVERRUN_E),


    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_0_SUM_CREDITS_COUNTER_UNDERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_0_SUM_RX_LB_TOKEN_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_0_SUM_RX_WR_DATA_TO_BANK_WITHOUT_VALID_BANK_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_0_SUM_RX_IDDB_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_0_SUM_RX_PHYSICAL_BANK_SYNC_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_0_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_TX_CLIENT_0_SUM_TX_RD_BURST_FIFO_OVERRUN_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_BANK_0_SUM_BANK_LATENCY_FIFO_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_BANK_1_SUM_BANK_LATENCY_FIFO_OVERRUN_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_PARITY_SUM_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_CNTR_OVER_UNDER_FLOW_SUM_PFC_PORT_GROUP0_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_CNTR_OVER_UNDER_FLOW_SUM_PFC_PORT_GROUP0_OVERFLOW_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC0_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC1_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC2_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC3_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC4_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC5_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC6_UNDERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_PORT_GROUP_0_UNDER_FLOW_SUM_PFC_PORT_GROUP0_GLOBAL_PFC_TC7_UNDERFLOW_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_0_INVALID_BUFFER_CLEAR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_0_NEXT_TABLE_UPDATE_WAS_LOST_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BMA_SUM_MC_CNT_COUNTER_FLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BMA_SUM_SHIFTER_LATENCY_FIFO_OVERFLOW_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BMA_SUM_MC_CNT_PARITY_ERROR_E),


    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TX_DMA0_SUM_RXDMA_UPDATES_FIFOS_OVERRUN_INTERRUPT_CAUSE_REG1_SUMMARY_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TX_DMA0_GENENAL_TX_Q_TO_TX_DMA_HA_DESC_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TX_DMA0_GENENAL_TX_Q_TO_TX_DMA_DESC_OVERRUN_E),

    SET_EVENT_PER_INDEX____0_24___MAC(PRV_CPSS_ALDRIN2_TX_FIFO0_NOT_READY1_SHIFTER,SOP_EOP_FIFO_NOT_READY_E),
    SET_EVENT_PER_INDEX____0_24___MAC(PRV_CPSS_ALDRIN2_TX_FIFO1_NOT_READY1_SHIFTER,SOP_EOP_FIFO_NOT_READY_E),
    SET_EVENT_PER_INDEX____0_24___MAC(PRV_CPSS_ALDRIN2_TX_FIFO2_NOT_READY1_SHIFTER,SOP_EOP_FIFO_NOT_READY_E),
    SET_EVENT_PER_INDEX____0_24___MAC(PRV_CPSS_ALDRIN2_TX_FIFO3_NOT_READY1_SHIFTER,SOP_EOP_FIFO_NOT_READY_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TX_DMA3_GENENAL_TX_Q_TO_TX_DMA_HA_DESC_OVERRUN_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TX_DMA3_GENENAL_TX_Q_TO_TX_DMA_DESC_OVERRUN_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BMX_FREE_ENTRY_FIFO_SER_ERROR0_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BMX_FREE_ENTRY_FIFO_SER_ERROR1_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BMX_FREE_ENTRY_FIFO_SER_ERROR2_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BMX_FREE_ENTRY_FIFO_SER_ERROR3_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BMX_FREE_ENTRY_FIFO_SER_ERROR4_E),

 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_DATA_INTEGRITY_ERROR_E,
    /* -- _ECC_ --*/

/*full register*/  SET_INDEX_EQUAL_EVENT__FULL_REG_MAC(PRV_CPSS_ALDRIN2_CRITICAL_ECC_ERROR_BM_CRITICAL_ECC_SUM_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TTI_SECOND_CRITICAL_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TTI_SECOND_CRITICAL_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_HA_SUM_ECC_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_HA_SUM_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MLL_SUM_FILE_ECC_1_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MLL_SUM_FILE_ECC_2_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_EFT_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_EFT_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ0_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ0_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ0_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ0_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ1_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ1_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ1_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ1_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ2_GEN_SUM_DQ_INC_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ2_GEN_SUM_DQ_INC_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ2_GEN_SUM_DQ_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ2_GEN_SUM_DQ_CLR_ECC_TWO_ERROR_DETECTED_E),

#if 0
/*
 *      The interrupt should not be mapped to CPSS general events, and it should
 *      not be tied to a callback function.
*/
    /* 'WA' name : it is for 'reference' purposes */
    PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ0_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ1_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ2_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_DQ3_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E),
#endif

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TX_DMA0_GENENAL_ECC_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TX_DMA0_GENENAL_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TX_FIFO0_GENERAL1_ECC_0_SINGLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TX_FIFO0_GENERAL1_ECC_0_DOUBLE_ERROR_E),
    /*-- DATA_ERROR --*/

    /* no other _DATA_ERROR_ beside those that already bound in:
        CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
        CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
        CPSS_PP_POLICER_DATA_ERR_E
    */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_IPLR0_DATA_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_IPLR1_SUM_DATA_ERROR_E),

    /*-- DATA_ERROR --*/

    /* no other _DATA_ERROR_ beside those that already bound in:
        CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
        CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
        CPSS_PP_POLICER_DATA_ERR_E
    */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_IPLR0_DATA_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_IPLR1_SUM_DATA_ERROR_E),


    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_GEN_SUM_TD_CLR_ECC_ONE_ERROR_CORRECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TXQ_GEN_SUM_TD_CLR_ECC_TWO_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_PARITY_SUM_PFC_PORT_GROUP0_PFC_IND_FIFO_ECC_ERR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_QCN_SUM_DESC_ECC_SINGLE_ERROR_DETECTED_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_QCN_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_SUM_ECC_ERROR_SUM_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E),

    /* -- _PARITY_ -- */
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_PFC_PARITY_SUM_PFC_PORT_GROUP0_COUNTERS_PARITY_ERR_E),

    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E),
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E),
  /* BMA is internal unit -- SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_BMA_SUM_MC_CNT_PARITY_ERROR_E),*/
    SET_INDEX_EQUAL_EVENT_MAC(PRV_CPSS_ALDRIN2_FUNCTIONAL1_SERVER_INT_E),

 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E,
    PRV_CPSS_ALDRIN2_INGR_OAM_SUM_KEEP_ALIVE_AGING_E,       0,
    PRV_CPSS_ALDRIN2_EGR_OAM_SUM_KEEP_ALIVE_AGING_E,        1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,
    PRV_CPSS_ALDRIN2_INGR_OAM_SUM_EXCESS_KEEPALIVE_E,       0,
    PRV_CPSS_ALDRIN2_EGR_OAM_SUM_EXCESS_KEEPALIVE_E,        1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E,
    PRV_CPSS_ALDRIN2_INGR_OAM_SUM_INVALID_KEEPALIVE_HASH_E, 0,
    PRV_CPSS_ALDRIN2_EGR_OAM_SUM_INVALID_KEEPALIVE_HASH_E,  1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,
    PRV_CPSS_ALDRIN2_INGR_OAM_SUM_RDI_STATUS_E,             0,
    PRV_CPSS_ALDRIN2_EGR_OAM_SUM_RDI_STATUS_E,              1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E,
    PRV_CPSS_ALDRIN2_INGR_OAM_SUM_TX_PERIOD_E,              0,
    PRV_CPSS_ALDRIN2_EGR_OAM_SUM_TX_PERIOD_E,               1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,
    PRV_CPSS_ALDRIN2_INGR_OAM_SUM_MEG_LEVEL_EXCEPTION_E,    0,
    PRV_CPSS_ALDRIN2_EGR_OAM_SUM_MEG_LEVEL_EXCEPTION_E,     1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E,
    PRV_CPSS_ALDRIN2_INGR_OAM_SUM_SOURCE_ERFACE_EXCEPTION_E,   0,
    PRV_CPSS_ALDRIN2_EGR_OAM_SUM_SOURCE_ERFACE_EXCEPTION_E,    1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,
    PRV_CPSS_ALDRIN2_INGR_OAM_SUM_ILLEGAL_OAM_ENTRY_INDEX_E,    0,
    PRV_CPSS_ALDRIN2_EGR_OAM_SUM_ILLEGAL_OAM_ENTRY_INDEX_E,     1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E,
    SET_EVENT_PER_72_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_AP_DOORBELL,LINK_STATUS_CHANGE_E),
 MARK_END_OF_UNI_EV_CNS,

 CPSS_SRVCPU_PORT_802_3_AP_E,
    SET_EVENT_PER_72_PORTS___ON_ALL_PORTS_MAC(PRV_CPSS_ALDRIN2_AP_DOORBELL,802_3_AP_E),
 MARK_END_OF_UNI_EV_CNS,
 /* RX SDMA MG 1*/
 CPSS_PP_MG1_RX_BUFFER_QUEUE0_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_BUFFER_QUEUE_0_E,     8+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE1_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_BUFFER_QUEUE_1_E,     8+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE2_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_BUFFER_QUEUE_2_E,     8+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE3_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_BUFFER_QUEUE_3_E,     8+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE4_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_BUFFER_QUEUE_4_E,     8+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE5_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_BUFFER_QUEUE_5_E,     8+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE6_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_BUFFER_QUEUE_6_E,     8+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_BUFFER_QUEUE7_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_BUFFER_QUEUE_7_E,     8+7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE0_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_ERROR_QUEUE_0_E,      8+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE1_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_ERROR_QUEUE_1_E,      8+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE2_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_ERROR_QUEUE_2_E,      8+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE3_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_ERROR_QUEUE_3_E,      8+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE4_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_ERROR_QUEUE_4_E,      8+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE5_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_ERROR_QUEUE_5_E,      8+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE6_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_ERROR_QUEUE_6_E,      8+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG1_RX_ERR_QUEUE7_E,
    PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_ERROR_QUEUE_7_E,      8+7,
 MARK_END_OF_UNI_EV_CNS,

 /* RX SDMA MG 2*/
 CPSS_PP_MG2_RX_BUFFER_QUEUE0_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_BUFFER_QUEUE_0_E,     16+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE1_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_BUFFER_QUEUE_1_E,     16+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE2_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_BUFFER_QUEUE_2_E,     16+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE3_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_BUFFER_QUEUE_3_E,     16+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE4_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_BUFFER_QUEUE_4_E,     16+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE5_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_BUFFER_QUEUE_5_E,     16+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE6_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_BUFFER_QUEUE_6_E,     16+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_BUFFER_QUEUE7_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_BUFFER_QUEUE_7_E,     16+7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE0_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_ERROR_QUEUE_0_E,      16+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE1_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_ERROR_QUEUE_1_E,      16+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE2_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_ERROR_QUEUE_2_E,      16+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE3_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_ERROR_QUEUE_3_E,      16+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE4_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_ERROR_QUEUE_4_E,      16+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE5_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_ERROR_QUEUE_5_E,      16+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE6_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_ERROR_QUEUE_6_E,      16+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG2_RX_ERR_QUEUE7_E,
    PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_ERROR_QUEUE_7_E,      16+7,
 MARK_END_OF_UNI_EV_CNS,

 /* RX SDMA MG 3*/
 CPSS_PP_MG3_RX_BUFFER_QUEUE0_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_BUFFER_QUEUE_0_E,     24+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE1_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_BUFFER_QUEUE_1_E,     24+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE2_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_BUFFER_QUEUE_2_E,     24+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE3_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_BUFFER_QUEUE_3_E,     24+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE4_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_BUFFER_QUEUE_4_E,     24+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE5_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_BUFFER_QUEUE_5_E,     24+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE6_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_BUFFER_QUEUE_6_E,     24+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_BUFFER_QUEUE7_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_BUFFER_QUEUE_7_E,     24+7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE0_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_ERROR_QUEUE_0_E,      24+0,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE1_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_ERROR_QUEUE_1_E,      24+1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE2_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_ERROR_QUEUE_2_E,      24+2,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE3_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_ERROR_QUEUE_3_E,      24+3,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE4_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_ERROR_QUEUE_4_E,      24+4,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE5_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_ERROR_QUEUE_5_E,      24+5,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE6_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_ERROR_QUEUE_6_E,      24+6,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_MG3_RX_ERR_QUEUE7_E,
    PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_ERROR_QUEUE_7_E,      24+7,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E,
    PRV_CPSS_ALDRIN2_TAI_INCOMING_TRIGGER_INT_E,          0,
    PRV_CPSS_ALDRIN2_TAI1_INCOMING_TRIGGER_INT_E,         1,
 MARK_END_OF_UNI_EV_CNS,

 CPSS_PP_PTP_TAI_GENERATION_E,
    PRV_CPSS_ALDRIN2_TAI_GENERATION_INT_E,                0,
    PRV_CPSS_ALDRIN2_TAI1_GENERATION_INT_E,               1,
 MARK_END_OF_UNI_EV_CNS
};
/* Interrupt cause to unified event map for Aldrin2 with extended data size */
static GT_U32 aldrin2UniEvMapTableWithExtDataSize = (sizeof(aldrin2UniEvMapTableWithExtData)/(sizeof(GT_U32)));

/* Interrupt cause to unified event map for Aldrin2
 This Table is for unified event without extended data
*/
static GT_U32 aldrin2UniEvMapTable[][2] =
{
 {CPSS_PP_MISC_TWSI_TIME_OUT_E,          PRV_CPSS_ALDRIN2_MISC_TWSI_TIME_OUT_E},
 {CPSS_PP_MISC_TWSI_STATUS_E,            PRV_CPSS_ALDRIN2_MISC_TWSI_STATUS_E},
 {CPSS_PP_MISC_ILLEGAL_ADDR_E,           PRV_CPSS_ALDRIN2_MISC_ILLEGAL_ADDR_E},
 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_ALDRIN2_TXQ_DQ0_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E},
 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_ALDRIN2_TXQ_DQ1_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E},
 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_ALDRIN2_TXQ_DQ2_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E},
 {CPSS_PP_TQ_SNIFF_DESC_DROP_E,          PRV_CPSS_ALDRIN2_TXQ_DQ3_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E},

 {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,   PRV_CPSS_ALDRIN2_BRIDGE_UPDATE_SECURITY_BREACH_E},
 {CPSS_PP_MAC_NUM_OF_HOP_EXP_E,          PRV_CPSS_ALDRIN2_FDB_NUM_OF_HOP_EX_P_E},
 {CPSS_PP_MAC_NA_LEARNED_E,              PRV_CPSS_ALDRIN2_FDB_NA_LEARNT_E},
 {CPSS_PP_MAC_NA_NOT_LEARNED_E,          PRV_CPSS_ALDRIN2_FDB_NA_NOT_LEARNT_E},
 {CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E,   PRV_CPSS_ALDRIN2_FDB_AGE_VIA_TRIGGER_ENDED_E},
 {CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E,    PRV_CPSS_ALDRIN2_FDB_AU_PROC_COMPLETED_E},
 {CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,    PRV_CPSS_ALDRIN2_FDB_AU_MSG_TOCPU_READY_E},
 {CPSS_PP_MAC_NA_SELF_LEARNED_E,         PRV_CPSS_ALDRIN2_FDB_NA_SELF_LEARNED_E},
 {CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E,     PRV_CPSS_ALDRIN2_FDB_N_AFROM_CPU_LEARNED_E},
 {CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E,     PRV_CPSS_ALDRIN2_FDB_N_AFROM_CPU_DROPPED_E},
 {CPSS_PP_MAC_AGED_OUT_E,                PRV_CPSS_ALDRIN2_FDB_AGED_OUT_E},
 {CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E,     PRV_CPSS_ALDRIN2_FDB_AU_FIFO_TO_CPU_IS_FULL_E},
 {CPSS_PP_EB_AUQ_PENDING_E,              PRV_CPSS_ALDRIN2_MISC_AUQ_PENDING_E},
 {CPSS_PP_EB_AUQ_FULL_E,                 PRV_CPSS_ALDRIN2_MISC_AU_QUEUE_FULL_E},
 {CPSS_PP_EB_AUQ_OVER_E,                 PRV_CPSS_ALDRIN2_MISC_AUQ_OVERRUN_E},
 {CPSS_PP_EB_AUQ_ALMOST_FULL_E,          PRV_CPSS_ALDRIN2_MISC_AUQ_ALMOST_FULL_E},
 {CPSS_PP_EB_FUQ_PENDING_E,              PRV_CPSS_ALDRIN2_MISC_FUQ_PENDING_E},
 {CPSS_PP_EB_FUQ_FULL_E,                 PRV_CPSS_ALDRIN2_MISC_FU_QUEUE_FULL_E},
 {CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,    PRV_CPSS_ALDRIN2_MISC_GENXS_READ_DMA_DONE_E},
 {CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E,      PRV_CPSS_ALDRIN2_MISC_PEX_ADDR_UNMAPPED_E},
 {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,     PRV_CPSS_ALDRIN2_BRIDGE_ADDRESS_OUT_OF_RANGE_E},

 {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E,PRV_CPSS_ALDRIN2_TTI_CPU_ADDRESS_OUT_OF_RANGE_E},

 {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,    PRV_CPSS_ALDRIN2_IPCL_SUM_MG_ADDR_OUT_OF_RANGE_E},
 {CPSS_PP_PORT_802_3_AP_E,               PRV_CPSS_ALDRIN2_MISC_Z80_INTERRUPT_E},
 {CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, PRV_CPSS_ALDRIN2_FDB_BLC_OVERFLOW_E},
 {CPSS_SRVCPU_IPC_E,                     PRV_CPSS_ALDRIN2_AP_DOORBELL_MAIN_IPC_E}
};

/* Interrupt cause to unified event map for Aldrin2 without extended data size */
static GT_U32 aldrin2UniEvMapTableSize = (sizeof(aldrin2UniEvMapTable)/(sizeof(GT_U32)*2));

#define   cnc_SUB_TREE_MAC(bit,instance)                                   \
    /* CNC-0  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,                 \
        0x0E000100,                \
        0x0E000104,                \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_CNC##instance##_SUM_WRAPAROUND_FUNC_SUM_E, \
        PRV_CPSS_ALDRIN2_CNC##instance##_SUM_MISC_FUNC_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                  \
                                                                                \
        /* WraparoundFuncInterruptSum  */                                       \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,               \
            0x0E000190,            \
            0x0E0001A4,            \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_CNC##instance##_WRAPAROUND_SUM_BLOCK0_WRAPAROUND_E,   \
            PRV_CPSS_ALDRIN2_CNC##instance##_WRAPAROUND_SUM_BLOCK15_WRAPAROUND_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* RateLimitFuncInterruptSum  */                                        \
        {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,               \
            0x0E0001B8,            \
            0x0E0001CC,            \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_CNC##instance##_RATE_LIMIT_SUM_BLOCK0_RATE_LIMIT_FIFO_DROP_E,  \
            PRV_CPSS_ALDRIN2_CNC##instance##_RATE_LIMIT_SUM_BLOCK15_RATE_LIMIT_FIFO_DROP_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* MiscFuncInterruptSum  */                                             \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_CNC_##instance##_E, NULL,               \
            0x0E0001E0,            \
            0x0E0001E4,            \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_CNC##instance##_MISC_SUM_DUMP_FINISHED_E,             \
            PRV_CPSS_ALDRIN2_CNC##instance##_MISC_SUM_CNC_UPDATE_LOST_E,           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eft_SUB_TREE_MAC(bit)                                           \
        /* eft  */                                                                  \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EGF_EFT_E, NULL, 0x0A0010A0, 0x0A0010B0,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_EFT_SUM_EGRESS_WRONG_ADDR_E,              \
            PRV_CPSS_ALDRIN2_EFT_SUM_INC_BUS_IS_TOO_SMALL_INT_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eoam_SUB_TREE_MAC(bit)                                          \
        /* egress OAM  */                                                           \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EOAM_E, NULL, 0x180000F0 , 0x180000F4,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_EGR_OAM_SUM_KEEP_ALIVE_AGING_E,           \
            PRV_CPSS_ALDRIN2_EGR_OAM_SUM_TX_PERIOD_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   epcl_SUB_TREE_MAC(bit)                                          \
        /* EPCL  */                                                                 \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EPCL_E, NULL, 0x16000010, 0x16000014,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_EPCL_SUM_REGFILE_ADDRESS_ERROR_E,         \
            PRV_CPSS_ALDRIN2_EPCL_SUM_REGFILE_ADDRESS_ERROR_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eplr_SUB_TREE_MAC(bit)                                          \
        /* eplr  */                                                                 \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EPLR_E, NULL, 0x17000200, 0x17000204,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_EPLR_SUM_DATA_ERROR_E,                                  \
            PRV_CPSS_ALDRIN2_EPLR_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for ingress STC interrupts leaf */
#define INGRESS_STC_NODE_FOR_16_PORTS_MAC(bit, index) \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09000620 + (index * 0x10) , 0x09000624 + (index * 0x10), \
            prvCpssDrvHwPpPortGroupIsrRead,                                        \
            prvCpssDrvHwPpPortGroupIsrWrite,                                       \
            PRV_CPSS_ALDRIN2_INGRESS_STC_PORT_0_INGRESS_SAMPLE_LOADED_E   + (index * 32),    \
            PRV_CPSS_ALDRIN2_INGRESS_STC_PORT_15_INGRESS_SAMPLE_LOADED_E  + (index * 32),    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define   eqIngressStc_SUB_TREE_MAC(bit)                                     \
        /* Ingress STC  */                                                     \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09000600 , 0x09000604,    \
            prvCpssDrvHwPpPortGroupIsrRead,                                   \
            prvCpssDrvHwPpPortGroupIsrWrite,                                  \
            PRV_CPSS_ALDRIN2_ING_STC_SUM_REG0_INGRESS_SAMPLE_LOADED_E, \
            PRV_CPSS_ALDRIN2_ING_STC_SUM_REG15_INGRESS_SAMPLE_LOADED_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                \
                                                                                    \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 1,    0),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 2,    1),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 3,    2),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 4,    3),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 5,    4),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 6,    5),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 7,    6),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 8,    7),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 9,    8),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 10,   9),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 11,   10),                     \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 12,   11),                     \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 13,   12),                     \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 14,   13),                     \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 15,   14),                     \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 16,   15)

#define   eqIngressStc1_SUB_TREE_MAC(bit)                                     \
        /* Ingress STC1 */                                                          \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09000608 , 0x0900060C,       \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_ING_STC_1_SUM_REG16_INGRESS_SAMPLE_LOADED_E,        \
            PRV_CPSS_ALDRIN2_ING_STC_1_SUM_REG31_INGRESS_SAMPLE_LOADED_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                \
                                                                                    \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 1,   16),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 2,   17),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 3,   18),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 4,   19),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 5,   20),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 6,   21),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 7,   22),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 8,   23),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC( 9,   24),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(10,   25),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(11,   26),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(12,   27),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(13,   28),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(14,   29),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(15,   30),                      \
            INGRESS_STC_NODE_FOR_16_PORTS_MAC(16,   31)

/* node for ingress SCT rate limiters interrupts leaf */
#define SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(index)                          \
        {(index+1), GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09010020  + (index * 0x10) , 0x09010024  + (index * 0x10), \
            prvCpssDrvHwPpPortGroupIsrRead,                                 \
            prvCpssDrvHwPpPortGroupIsrWrite,                                \
            PRV_CPSS_ALDRIN2_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER_0_PKT_DROPED_E   + (index * 32),    \
            PRV_CPSS_ALDRIN2_CPU_CODE_RATE_LIMITERS_SCT_RATE_LIMITER_15_PKT_DROPED_E  + (index * 32),    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eqSctRateLimiters_SUB_TREE_MAC(bit)                                  \
        /* SCT Rate Limiters */                                                 \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09010000, 0x09010004,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                    \
            prvCpssDrvHwPpPortGroupIsrWrite,                                   \
            PRV_CPSS_ALDRIN2_SCT_RATE_LIMITERS_REG0_CPU_CODE_RATE_LIMITER_E,       \
            PRV_CPSS_ALDRIN2_SCT_RATE_LIMITERS_REG15_CPU_CODE_RATE_LIMITER_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},             \
                                                                               \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(0),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(1),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(2),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(3),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(4),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(5),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(6),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(7),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(8),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(9),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(10),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(11),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(12),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(13),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(14),                        \
            SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(15)


#define   eq_SUB_TREE_MAC(bit)                                            \
    /* Pre-Egress Interrupt Summary Cause */                                    \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x09000058, 0x0900005c,       \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_EQ_INGRESS_STC_E,                         \
        PRV_CPSS_ALDRIN2_EQ_CRITICAL_ECC_ERROR_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4, NULL, NULL},                 \
                                                                                \
        eqIngressStc_SUB_TREE_MAC(1),                                     \
        eqSctRateLimiters_SUB_TREE_MAC(2),                                \
                                                                                \
        /* Critical ECC Error Int  */                                           \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_EQ_E, NULL, 0x090000A0 , 0x090000A4 ,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_CRITICAL_ECC_ERROR_CLEAR_DESC_CRITICAL_ECC_ONE_ERROR_E,     \
            PRV_CPSS_ALDRIN2_CRITICAL_ECC_ERROR_INCREMENT_DESC_CRITICAL_ECC_TWO_ERROR_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},  \
                                                                   \
        eqIngressStc1_SUB_TREE_MAC(5)

#define   ermrk_SUB_TREE_MAC(bit)                                         \
    /* ERMRK  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_ERMRK_E, NULL, 0x15000004, 0x15000008,    \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_ERMRK_SUM_REGFILE_ADDRESS_ERROR_E,        \
        PRV_CPSS_ALDRIN2_ERMRK_SUM_EGRESS_TIMESTAMP_QUEUE1_FULL_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   ha_SUB_TREE_MAC(bit)                                                  \
   /* HA  */                                                                    \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_HA_E, NULL, 0x14000300, 0x14000304,       \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_HA_SUM_REGFILE_ADDRESS_ERROR_E,           \
        PRV_CPSS_ALDRIN2_HA_SUM_OVERSIZE_HEADER_SIZE_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   ioam_SUB_TREE_MAC(bit)                                          \
    /* ingress OAM  */                                                          \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IOAM_E, NULL, 0x070000F0, 0x070000F4,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_INGR_OAM_SUM_KEEP_ALIVE_AGING_E,          \
        PRV_CPSS_ALDRIN2_INGR_OAM_SUM_TX_PERIOD_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   iplr0_SUB_TREE_MAC(bit)                                         \
    /* iplr0  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IPLR_E, NULL, 0x05000200, 0x05000204,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_IPLR0_DATA_ERROR_E,                       \
        PRV_CPSS_ALDRIN2_IPLR0_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   iplr1_SUB_TREE_MAC(bit)                                         \
    /* iplr1  */                                                                \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IPLR_1_E, NULL, 0x06000200, 0x06000204,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_IPLR1_SUM_DATA_ERROR_E,                                   \
        PRV_CPSS_ALDRIN2_IPLR1_SUM_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   router_SUB_TREE_MAC(bit)                                              \
    /* router  */                                                               \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_IPVX_E, NULL, 0x04000970, 0x04000974,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_ROUTER_I_PV_X_BAD_ADDR_E,                                 \
        PRV_CPSS_ALDRIN2_ROUTER_STG13_SIPSA_RETURNS_LFIFO_FULL_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   l2i_SUB_TREE_MAC(bit)                                            \
    /* L2 Bridge  */                                                       \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_L2I_E, NULL, 0x03002100, 0x03002104, \
        prvCpssDrvHwPpPortGroupIsrRead,                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                   \
        PRV_CPSS_ALDRIN2_BRIDGE_ADDRESS_OUT_OF_RANGE_E,       \
        PRV_CPSS_ALDRIN2_BRIDGE_UPDATE_SECURITY_BREACH_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   mll_SUB_TREE_MAC(bit)                                                 \
    /* MLL  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_MLL_E, NULL, 0x08000030, 0x08000034,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_MLL_SUM_INVALID_IP_MLL_ACCESS_E,                          \
        PRV_CPSS_ALDRIN2_MLL_SUM_FILE_ECC_2_ERROR_E,                              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   pcl_SUB_TREE_MAC(bit)                                            \
    /* pcl  */                                                             \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_PCL_E, NULL, 0x02000004, 0x02000008, \
        prvCpssDrvHwPpPortGroupIsrRead,                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                   \
        PRV_CPSS_ALDRIN2_IPCL_SUM_MG_ADDR_OUT_OF_RANGE_E,                   \
        PRV_CPSS_ALDRIN2_IPCL_SUM_LOOKUP2_FIFO_FULL_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   tti_SUB_TREE_MAC(bit)                                            \
    /* tti  */                                                             \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TTI_E, NULL, 0x01000004, 0x01000008, \
        prvCpssDrvHwPpPortGroupIsrRead,                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                   \
        PRV_CPSS_ALDRIN2_TTI_CPU_ADDRESS_OUT_OF_RANGE_E,                       \
        PRV_CPSS_ALDRIN2_TTI_PORT_PROTOCOL_TABLE_WRONG_ADDRESS_INTERRUPT_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define PRV_CPSS_DXCH_UNIT_TXQ_DQ0_E     PRV_CPSS_DXCH_UNIT_TXQ_DQ_E

#define ALDRIN2_UNIT_3_OFFSET_MAC(unit)   (((unit) == 3 ) ? 0x80000000 : 0)

#define ALDRIN2_TXQ_DQ_UNIT_OFFSET(unit)  0x1000000 * (unit%3) + ALDRIN2_UNIT_3_OFFSET_MAC(unit)

#define   txqDq_SUB_TREE_MAC(bit, unit)                                 \
    /*  TxQ DQ Interrupt Summary Cause  */                                    \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x0B000600 + ALDRIN2_TXQ_DQ_UNIT_OFFSET(unit), 0x0B000604 + ALDRIN2_TXQ_DQ_UNIT_OFFSET(unit), \
        prvCpssDrvHwPpPortGroupIsrRead,                                       \
        prvCpssDrvHwPpPortGroupIsrWrite,                                      \
        PRV_CPSS_ALDRIN2_TXQ_DQ##unit##_SUM_FLUSH_DONE0_SUM_E,   \
        PRV_CPSS_ALDRIN2_TXQ_DQ##unit##_SUM_GENERAL_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 4, NULL, NULL},              \
                                                                              \
        FLUSH_DONE_FOR_25_PORTS_MAC(1, unit),                           \
        txqDqMemoryError_NODE_MAC(5, unit),                             \
        EGRESS_STC_NODE_FOR_25_PORTS_MAC(6, unit),                      \
        txqDqGeneral_NODE_MAC(10, unit)

/* node for Flush Done Interrupt Cause */
#define FLUSH_DONE_FOR_25_PORTS_MAC(bit, unit)   \
            {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x0B000610 + ALDRIN2_TXQ_DQ_UNIT_OFFSET(unit), 0x0B000620 + ALDRIN2_TXQ_DQ_UNIT_OFFSET(unit), \
                prvCpssDrvHwPpPortGroupIsrRead,                                        \
                prvCpssDrvHwPpPortGroupIsrWrite,                                       \
                PRV_CPSS_ALDRIN2_TXQ_DQ##unit##_FLUSH_DONE_SUM_PORT_0_E,    \
                PRV_CPSS_ALDRIN2_TXQ_DQ##unit##_FLUSH_DONE_SUM_PORT_24_E,  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txqDqMemoryError_NODE_MAC(bit, unit)                                                    \
    /* TXQ-DQMemoryError  */                                                                            \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x0B000650 + ALDRIN2_TXQ_DQ_UNIT_OFFSET(unit), 0x0B000660 + ALDRIN2_TXQ_DQ_UNIT_OFFSET(unit),   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_ALDRIN2_TXQ_DQ##unit##_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E,      \
        PRV_CPSS_ALDRIN2_TXQ_DQ##unit##_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for egress STC interrupts leaf */
#define EGRESS_STC_NODE_FOR_25_PORTS_MAC(bit, unit) \
            {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x0B000630 + ALDRIN2_TXQ_DQ_UNIT_OFFSET(unit), 0x0B000640 + ALDRIN2_TXQ_DQ_UNIT_OFFSET(unit), \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_ALDRIN2_TXQ_DQ##unit##_EGR_STC_PORT_0_E,           \
                PRV_CPSS_ALDRIN2_TXQ_DQ##unit##_EGR_STC_PORT_24_E,         \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* node for TxQ DQ General Interrupt Cause */
#define txqDqGeneral_NODE_MAC(bit, unit)                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_DQ##unit##_E, NULL, 0x0B000670 + ALDRIN2_TXQ_DQ_UNIT_OFFSET(unit), 0x0B000680 + ALDRIN2_TXQ_DQ_UNIT_OFFSET(unit), \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        PRV_CPSS_ALDRIN2_TXQ_DQ##unit##_GEN_SUM_EGRESS_MIRORR_DESC_DROP_E,         \
        PRV_CPSS_ALDRIN2_TXQ_DQ##unit##_GEN_SUM_DQ_BURST_ABSORB_FIFO_FULL_INT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   sht_SUB_TREE_MAC(bit)                                                 \
    /* sht  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EGF_SHT_E, NULL, 0x36020010, 0x36020020,  \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_SHT_SUM_SHT_WRONG_ADDR_E,                                 \
        PRV_CPSS_ALDRIN2_SHT_SUM_SPANNING_TREE_STATE_TABLE_WRONG_ADDRESS_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   qag_SUB_TREE_MAC(bit)                                                 \
    /* QAG  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_EGF_QAG_E, NULL, 0x50F00010, 0x50F00020,  \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_QAG_SUM_QAG_WRONG_ADDR_E,                              \
        PRV_CPSS_ALDRIN2_QAG_SUM_EPORT_ATTRIBUTES_TABLE_WRONG_ADDRESS_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define ALDRIN2_LPM_UNIT_OFFSET_MAC(unit)                                       \
        0x1000000 * (unit)

#define ETA_SUB_TREE_MAC(bit, unit, instance)                                   \
    /* same address space as LPM */                                             \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_LPM_E, NULL,                              \
        0x44E00104 + ALDRIN2_LPM_UNIT_OFFSET_MAC(unit) + (0x00100000  * (instance)),                                 \
        0x44E00100 + ALDRIN2_LPM_UNIT_OFFSET_MAC(unit) + (0x00100000  * (instance)),                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_LPM##unit##_ETA_##instance##_ETA_WRONG_ADDRESS_E,      \
        PRV_CPSS_ALDRIN2_LPM##unit##_ETA_##instance##_ADDR_REPLACEMENT_INT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   lpm_SUB_TREE_MAC(bit, unit)                                           \
    /* LPM  */                                                                  \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_LPM_E, NULL, 0x44D00100 + ALDRIN2_LPM_UNIT_OFFSET_MAC(unit), 0x44D00110 + ALDRIN2_LPM_UNIT_OFFSET_MAC(unit),      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_LPM##unit##_LPM_GENERAL_SUM_E,                         \
        PRV_CPSS_ALDRIN2_LPM##unit##_ETA_1_SUM_E,                               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                  \
        /* LPM general*/                                                        \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_LPM_E, NULL, 0x44D00120 + ALDRIN2_LPM_UNIT_OFFSET_MAC(unit), 0x44D00130 + ALDRIN2_LPM_UNIT_OFFSET_MAC(unit),   \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_LPM##unit##_GENERAL_LPM_WRONG_ADDR_E,              \
            PRV_CPSS_ALDRIN2_LPM##unit##_GENERAL_AGING_CACHE_FULL_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
            /* bit 2 , instance 0 */                                            \
            ETA_SUB_TREE_MAC(2, unit, 0),                                       \
            /* bit 3 , instance 1 */                                            \
            ETA_SUB_TREE_MAC(3, unit, 1)

#define   tcam_SUB_TREE_MAC(bit)                                                \
    /* tcam  */                                                                 \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TCAM_E, NULL, 0x52501004, 0x52501000,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_TCAM_SUM_TCAM_LOGIC_SUMMARY_E,                         \
        PRV_CPSS_ALDRIN2_TCAM_SUM_TCAM_LOGIC_SUMMARY_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1,NULL, NULL},                  \
        /* tcam Logic*/                                                         \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TCAM_E, NULL, 0x5250100C, 0x52501008,   \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_TCAM_LOGIC_SUM_CPU_ADDRESS_OUT_OF_RANGE_E,         \
            PRV_CPSS_ALDRIN2_TCAM_LOGIC_SUM_ACTION_OOR_ADDRESS_INT_E,           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* convert bit in the enum of the huge interrupts of the device into BIT_0..BIT_31 in a register */
#define DEVICE_INT_TO_BIT_MAC(deviceInterruptId)    \
            (1 << ((deviceInterruptId) & 0x1f))

#define FDB_REG_MASK_CNS        \
    (0xFFFFFFFF & ~                                                             \
        (DEVICE_INT_TO_BIT_MAC(PRV_CPSS_ALDRIN2_FDB_AGE_VIA_TRIGGER_ENDED_E) |     \
         DEVICE_INT_TO_BIT_MAC(PRV_CPSS_ALDRIN2_FDB_AU_PROC_COMPLETED_E)))

#define   fdb_SUB_TREE_MAC(bit)                                                 \
    /* FDBIntSum - FDB Interrupt Cause,                                      */ \
    /*   maskRcvIntrEn - both AUProcCompletedInt(bit of PRV_CPSS_BOBK_FDB_AU_PROC_COMPLETED_E) and AgeViaTriggerEnded(bit of PRV_CPSS_BOBK_FDB_AGE_VIA_TRIGGER_ENDED_E)*/\
    /*   should never be masked to avoid missed events situation.  */           \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_FDB_E, NULL, 0x430001b0, 0x430001b4,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_FDB_NUM_OF_HOP_EX_P_E,                                 \
        PRV_CPSS_ALDRIN2_FDB_ADDRESS_OUT_OF_RANGE_E,                            \
        FILLED_IN_RUNTIME_CNS, 0x0, FDB_REG_MASK_CNS,                           \
             0,NULL, NULL}

/* merav done*/
#define   FuncUnitsIntsSum_SUB_TREE_MAC(bit)                                    \
    /* FuncUnitsIntsSum Interrupt Cause */                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000003f8, 0x000003fc,      \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_FUNCTIONAL_SUMMARY_FUNC_UNITS_CNC0_E,               \
        PRV_CPSS_ALDRIN2_FUNCTIONAL_SUMMARY_FUNC_UNITS_MG0_1_OUT2_E,            \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 27 , NULL, NULL},                        \
        cnc_SUB_TREE_MAC(1, 0),                                              \
        cnc_SUB_TREE_MAC(2, 1),                                              \
        eft_SUB_TREE_MAC(3),                                                 \
        eoam_SUB_TREE_MAC(4),                                                \
        epcl_SUB_TREE_MAC(5),                                                \
        eplr_SUB_TREE_MAC(6),                                                \
        eq_SUB_TREE_MAC(7),                                                  \
        ermrk_SUB_TREE_MAC(8),                                               \
        ha_SUB_TREE_MAC(9),                                                  \
        ioam_SUB_TREE_MAC(10),                                               \
        iplr0_SUB_TREE_MAC(11),                                               \
        iplr1_SUB_TREE_MAC(12),                                              \
        router_SUB_TREE_MAC(13),                                             \
        l2i_SUB_TREE_MAC(14),                                                \
        mll_SUB_TREE_MAC(15),                                                \
        pcl_SUB_TREE_MAC(16),                                                \
        tti_SUB_TREE_MAC(17),                                                \
        txqDq_SUB_TREE_MAC(18, 0),                                        \
        txqDq_SUB_TREE_MAC(19, 1),                                        \
        txqDq_SUB_TREE_MAC(20, 2),                                        \
        sht_SUB_TREE_MAC(21),                                                \
        qag_SUB_TREE_MAC(22),                                                   \
        lpm_SUB_TREE_MAC(23, 0),                                                \
        lpm_SUB_TREE_MAC(24, 1),                                                \
        fdb_SUB_TREE_MAC(27),                                                   \
        tcam_SUB_TREE_MAC(28),                                                   \
        {29, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00030, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00034, 1), \
                prvCpssDrvHwPpMg1ReadReg,                                      \
                prvCpssDrvHwPpMg1WriteReg,                                     \
                PRV_CPSS_ALDRIN2_MG1_GLOBAL_SUMMARY_TX_SDMA_SUM_E,             \
                PRV_CPSS_ALDRIN2_MG1_GLOBAL_SUMMARY_RX_SDMA_SUM_E,             \
                FILLED_IN_RUNTIME_CNS, 0, 0xFFFFFFFF, 2 , NULL, NULL},                \
                /* Tx SDMA  */                                                 \
            {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00002810, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002818, 1),   \
                prvCpssDrvHwPpMg1ReadReg,                                      \
                prvCpssDrvHwPpMg1WriteReg,                                     \
                PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_ALDRIN2_MG1_TX_SDMA_TX_REJECT_0_E,                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},         \
                                                                               \
            /* Rx SDMA  */                                                     \
            {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000280C, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002814, 1),   \
                prvCpssDrvHwPpMg1ReadReg,                                      \
                prvCpssDrvHwPpMg1WriteReg,                                     \
                PRV_CPSS_ALDRIN2_MG1_RX_SDMA_RX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_ALDRIN2_MG1_RX_SDMA_PACKET_CNT_OF_E,                  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define PRV_CPSS_DXCH_UNIT_RXDMA0_E         PRV_CPSS_DXCH_UNIT_RXDMA_E


#define PRV_ALDRIN2_RX_DMA_ADDR(addr, unit) \
        (addr) + 0x1000000 * (unit%3) + ALDRIN2_UNIT_3_OFFSET_MAC(unit)

#define rxDma_SUB_TREE_MAC(bit,unit)                                                      \
    /* rxDmaIntSum */                                                                                   \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##unit##_E, NULL,                                          \
        PRV_ALDRIN2_RX_DMA_ADDR(0x19001548, unit), PRV_ALDRIN2_RX_DMA_ADDR(0x1900154c, unit),   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_ALDRIN2_RX_DMA##unit##_SUM_RX_DMA_INTERRUPT_SUMMARY0_E,                   \
        PRV_CPSS_ALDRIN2_RX_DMA##unit##_SUM_RX_DMA_INTERRUPT_SUMMARY0_E,                   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 1 , NULL, NULL},                                                 \
                                                                                                        \
        rxDmaSum0_NODE_MAC(1, unit)

#define   rxDmaSum0_NODE_MAC(bit,unit)                                \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##unit##_E, NULL, PRV_ALDRIN2_RX_DMA_ADDR(0x19001528, unit), PRV_ALDRIN2_RX_DMA_ADDR(0x1900152C, unit), \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_RX_DMA##unit##_SUM_GENERAL_RX_DMA_S_SUMMARY_E,  \
            PRV_CPSS_ALDRIN2_RX_DMA##unit##_SUM_SC_DMA24_S_SUM_E,            \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 26 , NULL, NULL},                        \
                                                                                    \
           {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##unit##_E, NULL, PRV_ALDRIN2_RX_DMA_ADDR(0x19000E00, unit), PRV_ALDRIN2_RX_DMA_ADDR(0x19000E04, unit), \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_RX_DMA##unit##_INT0_RX_DMA_RF_ERR_E,      \
            PRV_CPSS_ALDRIN2_RX_DMA##unit##_INT0_COMMON_BUFFER_EMPTY3_E,  \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                         \
                                                                                    \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 2,  0, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 3,  1, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 4,  2, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 5,  3, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 6,  4, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 7,  5, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 8,  6, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 9,  7, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(10,  8, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(11,  9, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(12, 10, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(13, 11, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(14, 12, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(15, 13, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(16, 14, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(17, 15, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(18, 16, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(19, 17, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(20, 18, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(21, 19, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(22, 20, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(23, 21, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(24, 22, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(25, 23, unit),           \
            ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(26, 24, unit)

/* RXDMA : rxDmaScdma %n IntSum  */
#define ALDRIN2_RXDMA_SCDMA_PORT_INT_ELEMENT_MAC(bit, port, unit)             \
         /*rxDmaSum0IntSum*/                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA##unit##_E, NULL,                   \
            PRV_ALDRIN2_RX_DMA_ADDR(0x19000E08,unit) + 4 * (port),           \
            PRV_ALDRIN2_RX_DMA_ADDR(0x1900120C,unit) + 4 * (port),           \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_RX_DMA##unit##_SCDMA_##port##_SUM_EOP_DESC_FIFO_OVERRUN_E, \
            PRV_CPSS_ALDRIN2_RX_DMA##unit##_SCDMA_##port##_SUM_OS_DROP_E,               \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define PRV_ALDRIN2_TX_DMA_ADDR(addr,unit) \
        addr + 0x1000000 * (unit%3) + ALDRIN2_UNIT_3_OFFSET_MAC(unit)

#define PRV_CPSS_DXCH_UNIT_TXDMA0_E         PRV_CPSS_DXCH_UNIT_TXDMA_E

#define txDma_SUB_TREE_MAC(bit, unit)                                                                     \
            /* txDmaIntSum */                                                                                   \
            {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##unit##_E, NULL,                                           \
                PRV_ALDRIN2_TX_DMA_ADDR(0x1C002040, unit), PRV_ALDRIN2_TX_DMA_ADDR(0x1C002044, unit),   \
                prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
                PRV_CPSS_ALDRIN2_TX_DMA##unit##_SUM_SCDMAS_INTERRUPT_CAUSE_REG0_SUMMARY_E,         \
                PRV_CPSS_ALDRIN2_TX_DMA##unit##_SUM_RXDMA_UPDATES_FIFOS_OVERRUN_INTERRUPT_CAUSE_REG1_SUMMARY_E, \
                FILLED_IN_RUNTIME_CNS, 0, 0x0, 3 , NULL, NULL},                                                 \
                                                                                                                \
                /* SCDMAs Interrupt Cause Reg0 Summary */                                                       \
                {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##unit##_E, NULL,                                         \
                    PRV_ALDRIN2_TX_DMA_ADDR(0x1C002050, unit), PRV_ALDRIN2_TX_DMA_ADDR(0x1C002054, unit),           \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_ALDRIN2_TX_DMA##unit##_SCDMA0_INTERRUPTS_SUM_SUM_E,                   \
                    PRV_CPSS_ALDRIN2_TX_DMA##unit##_SCDMA24_INTERRUPTS_SUM_SUM_E,                  \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 25, NULL, NULL},                                    \
                                                                                                                \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 1,    0,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 2,    1,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 3,    2,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 4,    3,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 5,    4,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 6,    5,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 7,    6,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 8,    7,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC( 9,    8,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(10,    9,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(11,   10,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(12,   11,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(13,   12,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(14,   13,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(15,   14,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(16,   15,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(17,   16,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(18,   17,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(19,   18,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(20,   19,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(21,   20,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(22,   21,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(23,   22,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(24,   23,  unit),                            \
                    ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(25,   24,  unit),                            \
                                                                                                                \
                /* TxDMA General Cause Reg1 */                                                                  \
                {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##unit##_E, NULL,                                        \
                    PRV_ALDRIN2_TX_DMA_ADDR(0x1C002038, unit), PRV_ALDRIN2_TX_DMA_ADDR(0x1C00203C , unit), \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_ALDRIN2_TX_DMA##unit##_GENENAL_REQUEST_EXECUTOR_NOT_READY_E,                   \
                    PRV_CPSS_ALDRIN2_TX_DMA##unit##_GENENAL_BANK_REQUESTS_ON_AIR_REACHED_MAXIMAL_THRESHOLD_E,  \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                /* RxDMA Updates FIFOs Overrun Interrupt Cause Reg1 */                                          \
                {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##unit##_E, NULL,                                        \
                    PRV_ALDRIN2_TX_DMA_ADDR(0x1C002018, unit), PRV_ALDRIN2_TX_DMA_ADDR(0x1C00201C, unit), \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_ALDRIN2_TX_DMA##unit##_XDMA_0_UPDATES_FIFOS_OVERRUN_INT_SUM_E,         \
                    PRV_CPSS_ALDRIN2_TX_DMA##unit##_NPROG_3_UPDATES_FIFOS_OVERRUN_INT_SUM_E,        \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* TXDMA: SCDMA %p interrupts Cause */
#define ALDRIN2_TXDMA_SCDMA_PORT_INT_ELEMENT_MAC(bit, port, unit)             \
         /*txDmaSum0IntSum*/                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA##unit##_E, NULL,                   \
            PRV_ALDRIN2_TX_DMA_ADDR(0x1C002100, unit) + 4 * (port),           \
            PRV_ALDRIN2_TX_DMA_ADDR(0x1C002300, unit) + 4 * (port),           \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_TX_DMA##unit##_SCDMA_##port##_CT_SM_BC_BANKS_NUMBER_MISMATCH_INT_E, \
            PRV_CPSS_ALDRIN2_TX_DMA##unit##_SCDMA_##port##_ID_FIFO_OVERRUN_INT_E,               \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}


#define PRV_ALDRIN2_TX_FIFO_ADDR(addr, unit) \
        addr + 0x1000000 * (unit%3) + ALDRIN2_UNIT_3_OFFSET_MAC(unit)

#define PRV_CPSS_DXCH_UNIT_TX_FIFO0_E         PRV_CPSS_DXCH_UNIT_TX_FIFO_E

#define txFifo_SUB_TREE_MAC(bit, unit)                                                             \
            /* txFifoIntSum */                                                                                  \
            {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##unit##_E, NULL,                                         \
                PRV_ALDRIN2_TX_FIFO_ADDR(0x1F000440, unit), PRV_ALDRIN2_TX_FIFO_ADDR(0x1F000444, unit),   \
                prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
                PRV_CPSS_ALDRIN2_TX_FIFO##unit##_TX_FIFO_GENERAL_REG1_SUMMARY_E,        \
                PRV_CPSS_ALDRIN2_TX_FIFO##unit##_CT_MODIFIED_BYTE_COUNT_IS_TOO_SHORT_CAUSE_REG_1_SUMMARY_E, \
                FILLED_IN_RUNTIME_CNS, 0, 0x0, 4 , NULL, NULL},                                                 \
                                                                                                                \
                /* TxFIFO General Cause Reg1 */                                                                 \
                {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##unit##_E, NULL,                                       \
                    PRV_ALDRIN2_TX_FIFO_ADDR(0x1F000400, unit), PRV_ALDRIN2_TX_FIFO_ADDR(0x1F000404, unit),           \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_ALDRIN2_TX_FIFO##unit##_GENERAL1_LATENCY_FIFO_NOT_READY_E,            \
                    PRV_CPSS_ALDRIN2_TX_FIFO##unit##_GENERAL1_CT_BC_IDDB_I_DS_RUN_OUT_E,           \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                /* Shifter SOP EOP FIFO Not Ready Interrupt Cause Reg1 */                                       \
                {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##unit##_E, NULL,                                       \
                    PRV_ALDRIN2_TX_FIFO_ADDR(0x1F000408, unit), PRV_ALDRIN2_TX_FIFO_ADDR(0x1F00040C, unit), \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_ALDRIN2_TX_FIFO##unit##_NOT_READY1_SHIFTER_0_SOP_EOP_FIFO_NOT_READY_E,                   \
                    PRV_CPSS_ALDRIN2_TX_FIFO##unit##_NOT_READY1_SHIFTER_24_SOP_EOP_FIFO_NOT_READY_E,                  \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                /* CT Byte Count Arrived Late Interrupt Cause Reg1*/                                            \
                {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##unit##_E, NULL,                                       \
                    PRV_ALDRIN2_TX_FIFO_ADDR(0x1F000420, unit), PRV_ALDRIN2_TX_FIFO_ADDR(0x1F000424, unit), \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_ALDRIN2_TX_FIFO##unit##_SCDMA0_CT_BYTE_COUNT_ARRIVED_LATE_INT_SUM_E,  \
                    PRV_CPSS_ALDRIN2_TX_FIFO##unit##_SCDMA24_CT_BYTE_COUNT_ARRIVED_LATE_INT_SUM_E, \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
                /* CT Modified Byte Count Is Too Short Interrupt Cause Reg1 */                                  \
                {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO##unit##_E, NULL,                                       \
                    PRV_ALDRIN2_TX_FIFO_ADDR(0x1F000448, unit), PRV_ALDRIN2_TX_FIFO_ADDR(0x1F00044C, unit), \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                             \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
                    PRV_CPSS_ALDRIN2_TX_FIFO##unit##_SCDMA0_CT_MODIFIED_BYTE_COUNT_IS_TOO_SHORT_INT_SUM_E,         \
                    PRV_CPSS_ALDRIN2_TX_FIFO##unit##_SCDMA24_CT_MODIFIED_BYTE_COUNT_IS_TOO_SHORT_INT_SUM_E,        \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* BM General Cause Reg1 */
#define   bm_gen1_SUB_TREE_MAC(bit)                                                         \
                {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x40000300, 0x40000304,      \
                    prvCpssDrvHwPpPortGroupIsrRead,                                         \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                    PRV_CPSS_ALDRIN2_BM_GEN1_SUM_IN_PROG_CT_CLEAR_FIFO_OVERRUN_CORE_0_E,    \
                    PRV_CPSS_ALDRIN2_BM_GEN1_SUM_GLOBAL_RX_FULL_E,                          \
                    FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

/* BM General Cause Reg2 */
#define   bm_gen2_SUB_TREE_MAC(bit)                                                         \
                {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x40000308, 0x4000030C,      \
                    prvCpssDrvHwPpPortGroupIsrRead,                                         \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                    PRV_CPSS_ALDRIN2_BM_GEN2_BUFFER_0_TERMINATION_COMPLETED_E,              \
                    PRV_CPSS_ALDRIN2_BM_GEN2_GLOBAL_BUFFER_COUNTER_UNDERFLOW_INTERRUPT_E,   \
                    FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define   bm_SUB_TREE_MAC(bit)                                                         \
                /* bmSumIntSum */                                                      \
                {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BM_E, NULL, 0x40000310, 0x40000314, \
                    prvCpssDrvHwPpPortGroupIsrRead,                                    \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                   \
                    PRV_CPSS_ALDRIN2_BM_SUM_SUM_BM_GENERAL_REG1_SUMMARY_E,             \
                    PRV_CPSS_ALDRIN2_BM_SUM_SUM_BM_GENERAL_REG2_SUMMARY_E,             \
                    FILLED_IN_RUNTIME_CNS, 0, 0x0, 2 , NULL, NULL},                    \
                    /* BM General Cause Reg1 */                                        \
                    bm_gen1_SUB_TREE_MAC(1),                                           \
                    /* BM General Cause Reg2 */                                        \
                    bm_gen2_SUB_TREE_MAC(2)

#define   mppm_SUB_TREE_MAC(bit)                                                \
    /* mppmIntSum */                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x510021B0, 0x510021B4,    \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_MPPM_SUM_RX_CLIENT_0_S_SUM_E,                          \
        PRV_CPSS_ALDRIN2_MPPM_SUM_GENERAL_SUM_E,                                \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 14, NULL, NULL},                         \
                                                                                \
        /* mppmRxClient 0 IntSum */                                             \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002008, 0x5100202C,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_0_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E,\
            PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_0_SUM_CREDITS_COUNTER_UNDERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmRxClient 1 IntSum */                                             \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x5100200c, 0x51002030,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_1_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E,\
            PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_1_SUM_CREDITS_COUNTER_UNDERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmRxClient 2 IntSum */                                             \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002010, 0x51002034,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_2_SUM_RX_IDDB_WR_ID_FIFO_UNDERRUN_E,\
            PRV_CPSS_ALDRIN2_MPPM_RX_CLIENT_2_SUM_CREDITS_COUNTER_UNDERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmTxClient 0 IntSum */                                             \
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002080, 0x510020b0,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_TX_CLIENT_0_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            PRV_CPSS_ALDRIN2_MPPM_TX_CLIENT_0_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmTxClient 1 IntSum */                                             \
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002084, 0x510020b4,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_TX_CLIENT_1_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            PRV_CPSS_ALDRIN2_MPPM_TX_CLIENT_1_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmTxClient 2 IntSum */                                             \
        {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002088, 0x510020b8,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_TX_CLIENT_2_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            PRV_CPSS_ALDRIN2_MPPM_TX_CLIENT_2_SUM_TX_RD_BURST_FIFO_OVERRUN_E,   \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
                                                                                \
        /* mppmBank 0 IntSum */                                                 \
        {15, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002120, 0x51002170, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_BANK_0_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_ALDRIN2_MPPM_BANK_0_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 1 IntSum */                                                 \
        {16, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002124, 0x51002174, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_BANK_1_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_ALDRIN2_MPPM_BANK_1_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 2 IntSum */                                                 \
        {17, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002128, 0x51002178, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_BANK_2_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_ALDRIN2_MPPM_BANK_2_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 3 IntSum */                                                 \
        {18, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x5100212c, 0x5100217c, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_BANK_3_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_ALDRIN2_MPPM_BANK_3_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 4 IntSum */                                                 \
        {19, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002130, 0x51002180, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_BANK_4_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_ALDRIN2_MPPM_BANK_4_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmBank 5 IntSum */                                                 \
        {20, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002134, 0x51002184, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_BANK_5_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            PRV_CPSS_ALDRIN2_MPPM_BANK_5_SUM_BANK_LATENCY_FIFO_OVERRUN_E,       \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmEccIntSum */                                                     \
        {27, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x51002000, 0x51002004, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E,              \
            PRV_CPSS_ALDRIN2_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E,              \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL},                     \
        /* mppmGenIntSum */                                                     \
        {29, GT_FALSE, PRV_CPSS_DXCH_UNIT_MPPM_E, NULL, 0x510021c0, 0x510021c4, \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_MPPM_GEN_SUM_MPPM_RF_ERR_E,                        \
            PRV_CPSS_ALDRIN2_MPPM_GEN_SUM_MPPM_RF_ERR_E,                        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0 , NULL, NULL}

#define   DataPathIntSum_SUB_TREE_MAC(bit)                                     \
    /* DataPathIntSum */                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000000A4, 0x000000A8,     \
        prvCpssDrvHwPpPortGroupIsrRead,                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                       \
        PRV_CPSS_ALDRIN2_DATA_PATH_RXDMA0_RXDMA_INT_E,                   \
        PRV_CPSS_ALDRIN2_DATA_PATH_MG3_INT_OUT0_E,                      \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 19/*without bma*/ , NULL, NULL},        \
                                                                               \
        rxDma_SUB_TREE_MAC(1,0),                                        \
        rxDma_SUB_TREE_MAC(2,1),                                        \
        rxDma_SUB_TREE_MAC(3,2),                                        \
                                                                               \
        txDma_SUB_TREE_MAC(4,0),                                        \
        txDma_SUB_TREE_MAC(5,1),                                        \
        txDma_SUB_TREE_MAC(6,2),                                        \
                                                                               \
        txFifo_SUB_TREE_MAC(7, 0),                                       \
        txFifo_SUB_TREE_MAC(8, 1),                                       \
        txFifo_SUB_TREE_MAC(9, 2),                                       \
                                                                               \
        rxDma_SUB_TREE_MAC(10, 3),                                       \
                                                                               \
        txDma_SUB_TREE_MAC(13, 3),                                       \
                                                                               \
        txFifo_SUB_TREE_MAC(16,3),                                      \
                                                                               \
        bm_SUB_TREE_MAC(19),                                                   \
        mppm_SUB_TREE_MAC(21),                                                 \
                                                                               \
        /* 0..7, GPIO_0_31_Interrupt Cause */                                  \
        {22, GT_TRUE, 0, prvCpssDrvDxExMxRunitGppIsrCall, 0x00018114, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118), \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                     \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                    \
                PRV_CPSS_ALDRIN2_GPIO_0_0_E,                                   \
                PRV_CPSS_ALDRIN2_GPIO_0_7_E,                                   \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},        \
                                                                               \
        /* 8..15, GPIO_0_31_Interrupt Cause */                                 \
        {23, GT_TRUE, 8, prvCpssDrvDxExMxRunitGppIsrCall, 0x00018114, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118), \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                     \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                    \
                PRV_CPSS_ALDRIN2_GPIO_0_8_E,                                   \
                PRV_CPSS_ALDRIN2_GPIO_0_15_E,                                  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},        \
                                                                               \
        /* 16..23, GPIO_0_31_Interrupt Cause */                                \
        {24, GT_TRUE, 16, prvCpssDrvDxExMxRunitGppIsrCall, 0x00018114, PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118), \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                     \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                    \
                PRV_CPSS_ALDRIN2_GPIO_0_16_E,                                  \
                PRV_CPSS_ALDRIN2_GPIO_0_23_E,                                  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},         \
                                                                               \
         /*Mg2_Int_Out0 */                                                     \
        {26, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00030, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00034, 2), \
                prvCpssDrvHwPpMg2ReadReg,                     \
                prvCpssDrvHwPpMg2WriteReg,                    \
                PRV_CPSS_ALDRIN2_MG2_GLOBAL_SUMMARY_TX_SDMA_SUM_E,             \
                PRV_CPSS_ALDRIN2_MG2_GLOBAL_SUMMARY_RX_SDMA_SUM_E,             \
                FILLED_IN_RUNTIME_CNS, 0, 0xFFFFFFFF, 2 , NULL, NULL},                \
                                                                               \
             /* Tx SDMA  */                                                    \
            {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00002810, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002818, 2),   \
                prvCpssDrvHwPpMg2ReadReg,                                      \
                prvCpssDrvHwPpMg2WriteReg,                                     \
                PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_ALDRIN2_MG2_TX_SDMA_TX_REJECT_0_E,                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},         \
                                                                               \
            /* Rx SDMA  */                                                     \
            {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000280C, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002814, 2), \
                prvCpssDrvHwPpMg2ReadReg,                                      \
                prvCpssDrvHwPpMg2WriteReg,                                     \
                PRV_CPSS_ALDRIN2_MG2_RX_SDMA_RX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_ALDRIN2_MG2_RX_SDMA_PACKET_CNT_OF_E,                  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},         \
                                                                               \
         /*Mg3_Int_Out0 */                                                     \
        {29, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00030, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00034, 3), \
                prvCpssDrvHwPpMg3ReadReg,                                      \
                prvCpssDrvHwPpMg3WriteReg,                                     \
                PRV_CPSS_ALDRIN2_MG3_GLOBAL_SUMMARY_TX_SDMA_SUM_E,             \
                PRV_CPSS_ALDRIN2_MG3_GLOBAL_SUMMARY_RX_SDMA_SUM_E,             \
                FILLED_IN_RUNTIME_CNS, 0, 0xFFFFFFFF, 2 , NULL, NULL},                \
                                                                               \
             /* Tx SDMA  */                                                    \
            {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00002810, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002818, 3), \
                prvCpssDrvHwPpMg3ReadReg,                                      \
                prvCpssDrvHwPpMg3WriteReg,                                     \
                PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_ALDRIN2_MG3_TX_SDMA_TX_REJECT_0_E,                    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},         \
                                                                               \
            /* Rx SDMA  */                                                     \
            {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000280C, PRV_CPSS_DRV_MGS_INDICATION_IN_ADDR_MAC(0x00002814, 3), \
                prvCpssDrvHwPpMg3ReadReg,                                      \
                prvCpssDrvHwPpMg3WriteReg,                                     \
                PRV_CPSS_ALDRIN2_MG3_RX_SDMA_RX_BUFFER_QUEUE_0_E,              \
                PRV_CPSS_ALDRIN2_MG3_RX_SDMA_PACKET_CNT_OF_E,                  \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* make global port out of local port of Aldrin2 */
/* port 36 is global port 72 but in address of port 63 in MG chain 1 */
#define ALDRIN2_GLOBAL_PORT_MAC(port) \
    ((port) == 72 ? (24+63) : port)

#define ALDRIN2_PORT(port)  ALDRIN2_GLOBAL_PORT_MAC(port)
/* calculate the address of the GOP ports in Aldrin2 */
#define ALDRIN2_GOP_BASE_MAC(port) \
    (0x10000000 + ((ALDRIN2_PORT(port) < 24) ? (0x1000*(port)) : ((0x1000*(ALDRIN2_PORT(port)-24)) | ALDRIN2_GOP_PIPE_OFFSET_MAC)))

/*
    0x10000000 + 0x1000*g: where g (0-36) represents Network Ports from 0 to 36
*/
#define ALDRIN2_GIGA_PORT_BASE_MAC(port)                  \
        ALDRIN2_GOP_BASE_MAC(port)

#define ALDRIN2_XLG_PORT_BASE_MAC(port)                   \
        (0x000C0000 + ALDRIN2_GOP_BASE_MAC(port))

#define ALDRIN2_CG_PORT_BASE_MAC(port)                   \
        (0x00340000 + ALDRIN2_GOP_BASE_MAC(port))

/* XLG summary - cause */
#define ALDRIN2_XLG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN2_XLG_PORT_BASE_MAC(port) + (0x58))

/* XLG summary - mask */
#define ALDRIN2_XLG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port) \
        (ALDRIN2_XLG_PORT_BASE_MAC(port) + (0x5c))

/* XLG - cause */
#define ALDRIN2_XLG_PORT_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN2_XLG_PORT_BASE_MAC(port) + (0x14))

/* XLG - mask */
#define ALDRIN2_XLG_PORT_INTERRUPT_MASK_MAC(port) \
        (ALDRIN2_XLG_PORT_BASE_MAC(port) + (0x18))

/* gig summary - cause */
#define ALDRIN2_GIGA_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN2_GIGA_PORT_BASE_MAC(port) + (0xA0))

/* gig summary - mask */
#define ALDRIN2_GIGA_PORT_SUMMARY_INTERRUPT_MASK_MAC(port) \
        (ALDRIN2_GIGA_PORT_BASE_MAC(port) + (0xA4))

/* gig - cause */
#define ALDRIN2_GIGA_PORT_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN2_GIGA_PORT_BASE_MAC(port) + (0x20))

/* gig - mask */
#define ALDRIN2_GIGA_PORT_INTERRUPT_MASK_MAC(port) \
        (ALDRIN2_GIGA_PORT_BASE_MAC(port) + (0x24))

/* CG summary - cause */
#define ALDRIN2_CG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN2_CG_PORT_BASE_MAC(port) + (0x48))

/* CG summary - mask */
#define ALDRIN2_CG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port) \
        (ALDRIN2_CG_PORT_BASE_MAC(port) + (0x4c))

/* CG - cause */
#define ALDRIN2_CG_PORT_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN2_CG_PORT_BASE_MAC(port) + (0x40))

/* CG - mask */
#define ALDRIN2_CG_PORT_INTERRUPT_MASK_MAC(port) \
        (ALDRIN2_CG_PORT_BASE_MAC(port) + (0x44))

/*
    0x10180800 + 0x1000*t: where t (0-36) represents Network PTP
*/
#define ALDRIN2_PTP_PORT_BASE_MAC(port)                 \
        (0x00180800 + ALDRIN2_GOP_BASE_MAC(port))

/* ptp - cause */
#define ALDRIN2_PTP_PORT_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN2_PTP_PORT_BASE_MAC(port) + (0x00))

/* ptp - mask */
#define ALDRIN2_PTP_PORT_INTERRUPT_MASK_MAC(port) \
        (ALDRIN2_PTP_PORT_BASE_MAC(port) + (0x04))

/* PTP interrupts for giga/XLG port */
#define ALDRIN2_PTP_FOR_PORT_INT_ELEMENT_MAC(bitIndexInCaller,port)   \
            {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL, \
              ALDRIN2_PTP_PORT_INTERRUPT_CAUSE_MAC(port) , \
              ALDRIN2_PTP_PORT_INTERRUPT_MASK_MAC(port)  , \
              prvCpssDrvHwPpPortGroupIsrRead,                    \
              prvCpssDrvHwPpPortGroupIsrWrite,                   \
              PRV_CPSS_ALDRIN2_PTP_PORT_##port##_PTP_RX_TOD_SYNC_OND_OVERRUN_SYNC_E, \
              PRV_CPSS_ALDRIN2_PTP_PORT_##port##_MIB_FRAGMENT_INT_E,\
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define ALDRIN2_FAKE_PTP_PORT_INTERRUPT_MASK_MAC(port) \
        (1/*not valid address*/ | (ALDRIN2_PTP_PORT_INTERRUPT_MASK_MAC(port) ))

/* the fake node needed to skip 'duplication' in the tree checked by : prvCpssDrvExMxDxHwPpMaskRegInfoGet(...)  --> maskRegInfoGet(...) */
/* after calling prvCpssDrvExMxDxHwPpMaskRegInfoGet */
/* 1. replace the bobcat3MaskRegMapArr[] with CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS */
/* 2. replace the BOBCAT3_FAKE_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe) with BOBCAT3_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe) */
#define ALDRIN2_FAKE_PTP_FOR_CG_PORT_INT_ELEMENT_MAC(bitIndexInCaller,port)   \
            {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL, \
              ALDRIN2_PTP_PORT_INTERRUPT_CAUSE_MAC(port) , \
              ALDRIN2_FAKE_PTP_PORT_INTERRUPT_MASK_MAC(port)  , \
              prvCpssDrvHwPpPortGroupIsrRead,                    \
              prvCpssDrvHwPpPortGroupIsrWrite,                   \
              PRV_CPSS_ALDRIN2_PTP_PORT_##port##_PTP_RX_TOD_SYNC_OND_OVERRUN_SYNC_E, \
              PRV_CPSS_ALDRIN2_PTP_PORT_##port##_MIB_FRAGMENT_INT_E,\
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define ALDRIN2_XLG_PORTS_0_30_SUMMARY_INT_SUB_TREE_MAC         \
                                                      /*bit, port */  \
            ALDRIN2_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  (  1,   0), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  2,   1), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  3,   2), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  4,   3), \
            ALDRIN2_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  (  5,   4), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  6,   5), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  7,   6), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  8,   7), \
            ALDRIN2_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  (  9,   8), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 10,   9), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 11,  10), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 12,  11), \
            ALDRIN2_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 13,  12), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 14,  13), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 15,  14), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 16,  15), \
            ALDRIN2_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 17,  16), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 18,  17), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 19,  18), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 20,  19), \
            ALDRIN2_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC  ( 21,  20), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 22,  21), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 23,  22), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 24,  23), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 25,  24), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 26,  25), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 27,  26), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 28,  27), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 29,  28), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 30,  29), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 31,  30)

#define COMMON_ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port,numSons)      \
                /* XLG - External Units Interrupts cause */                                         \
                {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                         \
                ALDRIN2_XLG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port),                           \
                ALDRIN2_XLG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port),                            \
                prvCpssDrvAldrin2XlgGopExtUnitsIsrRead,                                             \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_ALDRIN2_XLG_PORT_##port##_SUM_XLG_PORT_INTERRUPT_SUMMARY_E,   \
                PRV_CPSS_ALDRIN2_XLG_PORT_##port##_SUM_PTP_UNIT_INTERRUPT_SUMMARY_E + (numSons-4),   \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, numSons ,NULL, NULL},                       \
                    /* interrupts of the XLG mac */                                                 \
                    { 1 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                   \
                    ALDRIN2_XLG_PORT_INTERRUPT_CAUSE_MAC(port) ,                              \
                    ALDRIN2_XLG_PORT_INTERRUPT_MASK_MAC(port),                                \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_ALDRIN2_XLG_PORT_##port##_LINK_STATUS_CHANGED_E,          \
                    PRV_CPSS_ALDRIN2_XLG_PORT_##port##_PFC_SYNC_FIFO_OVERRUN_E,        \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                          \
                    /*bit2*/                                                                        \
                    ALDRIN2_GIGA_PORT_SUMMARY_FROM_XLG_INT_SUB_TREE_MAC(port),                \
                    /* interrupts of the MPCS mac (also called GB)*/                                \
                    { 5 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                   \
                    ALDRIN2_MPCS_PORT_INTERRUPT_CAUSE_MAC(port) ,                             \
                    ALDRIN2_MPCS_PORT_INTERRUPT_MASK_MAC(port),                               \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_ALDRIN2_MPCS_PORT_##port##_ACCESS_ERROR_E,                \
                    PRV_CPSS_ALDRIN2_MPCS_PORT_##port##_GB_LOCK_SYNC_CHANGE_E,         \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                          \
                    /*bit 7*/                                                                       \
                    ALDRIN2_PTP_FOR_PORT_INT_ELEMENT_MAC(7, port)

#define ALDRIN2_XLG_CG_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port)                   \
                COMMON_ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port, 5),    \
                    /*bit 8 (dummy bit - CG Ports interrupts support)*/                             \
                    ALDRIN2_CG_PORT_SUMMARY_INT_ELEMENT_MAC(8, port)

#define ALDRIN2_CG_PORT_SUMMARY_INT_ELEMENT_MAC(bitIndexInCaller, port)                       \
                /* CG - External Units Interrupts cause */                                          \
                {bitIndexInCaller, GT_FALSE, PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                        \
                ALDRIN2_CG_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port),                            \
                ALDRIN2_CG_PORT_SUMMARY_INTERRUPT_MASK_MAC(port),                             \
                prvCpssDrvHwPpPortGroupIsrRead,                                                     \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                    \
                PRV_CPSS_ALDRIN2_CG_PORT_##port##_SUM_CG_INT_SUMMARY_E,                \
                PRV_CPSS_ALDRIN2_CG_PORT_##port##_SUM_PTP_INT_SUMMARY_E,               \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2 ,NULL, NULL},                             \
                    /* interrupts of the CG  */                                                     \
                    { 1 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                   \
                    ALDRIN2_CG_PORT_INTERRUPT_CAUSE_MAC(port) ,                               \
                    ALDRIN2_CG_PORT_INTERRUPT_MASK_MAC(port),                                 \
                    prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                    prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                    PRV_CPSS_ALDRIN2_CG_PORT_##port##_RX_OVERSIZE_PACKET_DROP_E,       \
                    PRV_CPSS_ALDRIN2_CG_PORT_##port##_MLG_RX_FIFO_UNDERRUN3_INT_E,     \
                    FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                         \
                    /*bit 2*/                                                                       \
                    ALDRIN2_FAKE_PTP_FOR_CG_PORT_INT_ELEMENT_MAC(2, port)
/* summary of Giga mac that called from XLG mac */
#define ALDRIN2_GIGA_PORT_SUMMARY_FROM_XLG_INT_SUB_TREE_MAC(port)                         \
                {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                                    \
                ALDRIN2_GIGA_PORT_SUMMARY_INTERRUPT_CAUSE_MAC(port) ,                     \
                ALDRIN2_GIGA_PORT_SUMMARY_INTERRUPT_MASK_MAC(port)  ,                     \
                prvCpssDrvHwPpPortGroupIsrRead,                                                 \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                \
                PRV_CPSS_ALDRIN2_GIGA_PORT_##port##_SUM_INTERNAL_SUM_E,                 \
                PRV_CPSS_ALDRIN2_GIGA_PORT_##port##_SUM_INTERNAL_SUM_E, /*see PTP issue below */ \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1/*see PTP issue below */,NULL, NULL},  \
                    /* interrupts of the giga mac */                                            \
                    { 1 ,GT_FALSE,PRV_CPSS_DXCH_UNIT_GOP_E, NULL,                               \
                        ALDRIN2_GIGA_PORT_INTERRUPT_CAUSE_MAC(port) ,                     \
                        ALDRIN2_GIGA_PORT_INTERRUPT_MASK_MAC(port),                       \
                        prvCpssDrvHwPpPortGroupIsrRead,                                         \
                        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                        PRV_CPSS_ALDRIN2_GIGA_PORT_##port##_LINK_STATUS_CHANGED_E, \
                        PRV_CPSS_ALDRIN2_GIGA_PORT_##port##_MIB_COUNTER_WRAPAROUND_E,      \
                        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* 10180000 + 0x1000* k: where k (0-31)  */
#define ALDRIN2_MPCS_PORT_BASE_MAC(port)                 \
        (0x00180000 + ALDRIN2_GOP_BASE_MAC(port))

/* MPCS - cause */
#define ALDRIN2_MPCS_PORT_INTERRUPT_CAUSE_MAC(port) \
        (ALDRIN2_MPCS_PORT_BASE_MAC(port) + (0x08))

/* MPCS - mask */
#define ALDRIN2_MPCS_PORT_INTERRUPT_MASK_MAC(port) \
        (ALDRIN2_MPCS_PORT_BASE_MAC(port) + (0x0C))

#define ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port)\
    COMMON_ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC(bitIndexInCaller,port, 4)

GT_STATUS prvCpssGenericSrvCpuRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
);
GT_STATUS prvCpssGenericSrvCpuRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
);
#define   AP_Doorbell_SUB_TREE_MAC(bit)                                     \
    /* AP_DOORBELL_MAIN */                                                  \
    {bit,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000010, 0x00000011,    \
        prvCpssGenericSrvCpuRegisterRead,                                   \
        prvCpssGenericSrvCpuRegisterWrite,                                  \
        PRV_CPSS_ALDRIN2_AP_DOORBELL_MAIN_IPC_E,                            \
        PRV_CPSS_ALDRIN2_AP_DOORBELL_MAIN_RESERVED31_E,                     \
        FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0xFFFFF880, 6,NULL, NULL},       \
        /* AP_DOORBELL_PORT0_31_LINK_STATUS_CHANGE */                       \
        {4,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000012, 0x00000013,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_0_LINK_STATUS_CHANGE_E,       \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_31_LINK_STATUS_CHANGE_E,      \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT32_63_LINK_STATUS_CHANGE */                      \
        {5,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000014, 0x00000015,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_32_LINK_STATUS_CHANGE_E,      \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_63_LINK_STATUS_CHANGE_E,      \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT64_95_LINK_STATUS_CHANGE */                      \
        {6,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000016, 0x00000017,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_64_LINK_STATUS_CHANGE_E,      \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_95_LINK_STATUS_CHANGE_E,      \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT0_31_802_3_AP */                                 \
        {8,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000018, 0x00000019,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_0_802_3_AP_E,                 \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_31_802_3_AP_E,                \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT32_63_802_3_AP */                                \
        {9,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000001a, 0x0000001b,  \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_32_802_3_AP_E,                \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_63_802_3_AP_E,                \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL},          \
        /* AP_DOORBELL_PORT64_95_802_3_AP */                                \
        {10,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000001c, 0x0000001d, \
            prvCpssGenericSrvCpuRegisterRead,                               \
            prvCpssGenericSrvCpuRegisterWrite,                              \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_64_802_3_AP_E,                \
            PRV_CPSS_ALDRIN2_AP_DOORBELL_PORT_95_802_3_AP_E,                \
            FILLED_IN_RUNTIME_CNS, 0xFFFFFFFF, 0x0, 0,NULL, NULL}

#define   txqQueue_SUB_TREE_MAC(bit)                                            \
    /* TXQ-queue  */                                                            \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090000, 0x55090004,\
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_TXQ_SUM_GENERAL1_SUM_E,                                \
        PRV_CPSS_ALDRIN2_TXQ_SUM_PORT_BUFF_FULL2_SUM_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 12, NULL, NULL},                \
                                                                                \
        /* TXQ-queue : txqGenIntSum  */                                         \
        {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090008, 0x5509000c,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_TXQ_GEN_SUM_GLOBAL_DESC_FULL_E,                    \
            PRV_CPSS_ALDRIN2_TXQ_GEN_SUM_TD_CLR_ECC_TWO_ERROR_DETECTED_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        /* TXQ-queue : txqPortDesc0IntSum  */                                   \
        {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090040, 0x55090044,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_0_E,              \
            PRV_CPSS_ALDRIN2_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_30_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortDesc1IntSum  */                                   \
        {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090048, 0x5509004C,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_31_E,             \
            PRV_CPSS_ALDRIN2_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_61_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortDesc2IntSum  */                                   \
        {4, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090050, 0x55090054,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_62_E,             \
            PRV_CPSS_ALDRIN2_TXQ_PORT_DESC_SUM_DESC_FULL_PORT_92_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortBuff0IntSum  */                                   \
        {5, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090060, 0x55090064,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_TXQ_PORT_BUFF0_SUM_BUFF_FULL_PORT0_E,              \
            PRV_CPSS_ALDRIN2_TXQ_PORT_BUFF0_SUM_BUFF_FULL_PORT30_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortBuff1IntSum  */                                   \
        {6, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090068, 0x5509006C,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_TXQ_PORT_BUFF1_SUM_BUFF_FULL_PORT31_E,             \
            PRV_CPSS_ALDRIN2_TXQ_PORT_BUFF1_SUM_BUFF_FULL_PORT61_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : txqPortBuff2IntSum  */                                   \
        {7, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090070, 0x55090074,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            PRV_CPSS_ALDRIN2_TXQ_PORT_BUFF2_SUM_BUFF_FULL_PORT62_E,             \
            PRV_CPSS_ALDRIN2_TXQ_PORT_BUFF2_SUM_BUFF_FULL_PORT92_E,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},              \
        /* TXQ-queue : General */                                       \
        {8, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090010, 0x55090014,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_TXQ_GENERAL_INC_FIFO_0_FULL_INT_SUM_E,                 \
            PRV_CPSS_ALDRIN2_TXQ_GENERAL_BYPASS_BURST_FIFO_FULL_INT3_SUM_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
        /* TXQ-queue : High Port Descriptor Full Interrupt Summary Cause */         \
        {9, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550B0100, 0x550B0104,  \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_TXQ_HIGH_PORT_DESC_FULL_INT_SUM0_SUM_E,                \
            PRV_CPSS_ALDRIN2_TXQ_HIGH_PORT_DESC_FULL_INT_SUM0_SUM_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                 \
                                                                                    \
            ALDRIN2_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC( 1,  0,  93, 99),\
        /* TXQ-queue : High Port Buffer Full Interrupt Summary Cause */                  \
        {10, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550B0108, 0x550B010C,      \
            prvCpssDrvHwPpPortGroupIsrRead,                                       \
            prvCpssDrvHwPpPortGroupIsrWrite,                                      \
            PRV_CPSS_ALDRIN2_TXQ_HIGH_PORT_BUF_FULL_INT_SUM0_SUM_E,               \
            PRV_CPSS_ALDRIN2_TXQ_HIGH_PORT_BUF_FULL_INT_SUM0_SUM_E,              \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                \
                                                                                  \
            ALDRIN2_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC( 1,  0,  93, 99), \
                                                                                  \
        /* TXQ-queue :         Tail Drop Dequeue FIFO Full Interrupt Cause */            \
        {11, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090078, 0x5509007C, \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_ALDRIN2_TXQ_DEQUEUE_FIFO_FULL_MC_DESC_FIFO_FULL0_E,            \
            PRV_CPSS_ALDRIN2_TXQ_DEQUEUE_FIFO_FULL_DQ_TD_PARAM_FIFO_FULL3_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
                                                                                    \
        /* TXQ-queue : Crossing Interrupt summary cause */                          \
        {12, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090180, 0x55090184,     \
            prvCpssDrvHwPpPortGroupIsrRead,                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                            \
            PRV_CPSS_ALDRIN2_TXQ_QUEUE_STATISTIC_LOW_CROSSING_INTERRUPT_CAUSE_QUEUE_GROUPS_0_30_E,      \
            PRV_CPSS_ALDRIN2_TXQ_QUEUE_STATISTIC_HIGH_CROSSING_INTERRUPT_CAUSE_QUEUE_GROUPS_62_71_SUM_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6, NULL, NULL},                     \
            /* TXQ-queue : Low_Crossing_Int_Sum_Ports_30_to_0*/                         \
            {1, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090080, 0x55090084,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_ALDRIN2_LOW_CROSSING_SUM_PORTS_30_TO_0_LOW_THRESHOLD_CROSSED_PORT0_SUM_E,      \
                PRV_CPSS_ALDRIN2_LOW_CROSSING_SUM_PORTS_30_TO_0_LOW_THRESHOLD_CROSSED_PORT30_SUM_E,     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
            /* TXQ-queue : Low_Crossing_Int_Sum_Ports_61_to_31*/                        \
            {2, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x55090090, 0x55090094,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_ALDRIN2_LOW_CROSSING_SUM_PORTS_61_TO_31_LOW_THRESHOLD_CROSSED_PORT31_SUM_E,    \
                PRV_CPSS_ALDRIN2_LOW_CROSSING_SUM_PORTS_61_TO_31_LOW_THRESHOLD_CROSSED_PORT61_SUM_E,    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                  \
            /* TXQ-queue : Low_Crossing_Int_Sum_Ports_71_to_62*/                        \
            {3, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550900A0, 0x550900A4,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_ALDRIN2_LOW_CROSSING_SUM_PORTS_71_TO_62_LOW_THRESHOLD_CROSSED_PORT62_SUM_E,    \
                PRV_CPSS_ALDRIN2_LOW_CROSSING_SUM_PORTS_71_TO_62_LOW_THRESHOLD_CROSSED_PORT71_SUM_E,    \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
                                                                                        \
            /* TXQ-queue : High_Crossing_Int_Sum_Ports_30_to_0*/                        \
            {4, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550900B0, 0x550900B4,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_ALDRIN2_HIGH_CROSSING_SUM_PORTS_30_TO_0_HIGH_THRESHOLD_CROSSED_PORT0_SUM_E,  \
                PRV_CPSS_ALDRIN2_HIGH_CROSSING_SUM_PORTS_30_TO_0_HIGH_THRESHOLD_CROSSED_PORT30_SUM_E, \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                  \
            /* TXQ-queue : High_Crossing_Int_Sum_Ports_61_to_31*/                        \
            {5, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550900C0, 0x550900C4,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_ALDRIN2_HIGH_CROSSING_SUM_PORTS_61_TO_31_HIGH_THRESHOLD_CROSSED_PORT31_SUM_E, \
                PRV_CPSS_ALDRIN2_HIGH_CROSSING_SUM_PORTS_61_TO_31_HIGH_THRESHOLD_CROSSED_PORT61_SUM_E, \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                              \
            /* TXQ-queue : High_Crossing_Int_Sum_Ports_71_to_62*/                       \
            {6, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, 0x550900D0, 0x550900D4,  \
                prvCpssDrvHwPpPortGroupIsrRead,                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                        \
                PRV_CPSS_ALDRIN2_HIGH_CROSSING_SUM_PORTS_71_TO_62_HIGH_THRESHOLD_CROSSED_PORT62_SUM_E,  \
                PRV_CPSS_ALDRIN2_HIGH_CROSSING_SUM_PORTS_71_TO_62_HIGH_THRESHOLD_CROSSED_PORT71_SUM_E, \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* TXQ queue: High Port Descriptor Full Interrupt Cause */
#define ALDRIN2_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_CAUSE_MAC(index) \
        (0x550B0000 + 0x4 * (index))

/* TXQ queue: High Port Descriptor Full Interrupt Mask */
#define ALDRIN2_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MASK_MAC(index) \
        (0x550B0040 + 0x4 * (index))

/* TXQ queue: High Port Descriptor */
#define ALDRIN2_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MAC(bitIndexInCaller,index, startPort, endPort)   \
            {bitIndexInCaller, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL,       \
              ALDRIN2_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_CAUSE_MAC(index),    \
              ALDRIN2_TXQ_QUEUE_HIGH_PORT_DESCRIPTOR_FULL_INTERRUPT_MASK_MAC(index) ,       \
              prvCpssDrvHwPpPortGroupIsrRead,                                       \
              prvCpssDrvHwPpPortGroupIsrWrite,                                      \
              PRV_CPSS_ALDRIN2_TXQ_HIGH_PORT_DESC_FULL_##index##_INTERRUPT_CAUSE_##startPort##_SUM_E, \
              PRV_CPSS_ALDRIN2_TXQ_HIGH_PORT_DESC_FULL_##index##_INTERRUPT_CAUSE_##endPort##_SUM_E, \
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* TXQ queue: High Port Descriptor Full Interrupt Cause */
#define ALDRIN2_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_CAUSE_MAC(index) \
        (0x550B0080 + 0x4 * (index))

/* TXQ queue: High Port Descriptor Full Interrupt Mask */
#define ALDRIN2_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MASK_MAC(index) \
        (0x550B00C0 + 0x4 * (index))

/* TXQ queue: High Port Descriptor */
#define ALDRIN2_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MAC(bitIndexInCaller,index, startPort, endPort)   \
            {bitIndexInCaller, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E, NULL, \
              ALDRIN2_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_CAUSE_MAC(index) , \
              ALDRIN2_TXQ_QUEUE_HIGH_PORT_BUF_FULL_INTERRUPT_MASK_MAC(index)  , \
              prvCpssDrvHwPpPortGroupIsrRead,                    \
              prvCpssDrvHwPpPortGroupIsrWrite,                   \
              PRV_CPSS_ALDRIN2_TXQ_HIGH_PORT_BUF_FULL_##index##_INTERRUPT_CAUSE_##startPort##_SUM_E, \
              PRV_CPSS_ALDRIN2_TXQ_HIGH_PORT_BUF_FULL_##index##_INTERRUPT_CAUSE_##endPort##_SUM_E, \
              FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txqBmx_NODE_MAC(bit)                                                  \
    /* TXQ-BMX  */                                                              \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x56000028, 0x5600002C,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_BMX_ALMOST_FULL_INTERRUPT_E,                           \
        PRV_CPSS_ALDRIN2_BMX_FREE_ENTRY_FIFO_SER_ERROR4_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   FuncUnits1IntsSum_SUB_TREE_MAC(bit)                                   \
    /* FuncUnits1IntsSum  */                                                        \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000003F4, 0x000003F0,           \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        PRV_CPSS_ALDRIN2_FUNCTIONAL1_TXQ3_DQ_INT_E,                              \
        PRV_CPSS_ALDRIN2_FUNCTIONAL1_FUNC_UNITS_1_INT_SUM_31_E,                     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5 , NULL, NULL},                    \
                                                                                \
        txqDq_SUB_TREE_MAC(18, 3),                                               \
        txqLl_SUB_TREE_MAC(22),                                                     \
        txqQcn_SUB_TREE_MAC(23),                                                    \
        txqQueue_SUB_TREE_MAC(24),                                                  \
        txqBmx_NODE_MAC(26)

#define   txqLl_SUB_TREE_MAC(bit)                                               \
    /* TXQ-LL  */                                                               \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x53112020, 0x53112024,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_LL_SUMMARY_SUM_LL_SUM_E,                               \
        PRV_CPSS_ALDRIN2_LL_SUMMARY_SUM_LL_FIFO_INTERRUPT_CAUSE_SUMMARY_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                 \
        txqLl_NODE_MAC(1),                                                      \
        txqLl_fifo_NODE_MAC(2)

#define   txqQcn_SUB_TREE_MAC(bit)                                              \
    /* TXQ-QCN  */                                                              \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_QCN_E, NULL, 0x54000100, 0x54000110,  \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_QCN_SUM_CN_BUFFER_FIFO_OVERRUN_E,                      \
        PRV_CPSS_ALDRIN2_QCN_SUM_DESC_ECC_DOUBLE_ERROR_DETECTED_INT_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqLl_NODE_MAC(bit)                                                   \
    /* TXQ-LL  */                                                               \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x53112008, 0x5311200C,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_LL_SUM_ID_FIFO_OVERRUN_E,                              \
        PRV_CPSS_ALDRIN2_LL_SUM_BMX_NOT_READY_INT1_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   txqLl_fifo_NODE_MAC(bit)                                              \
    /* TXQ-LL  */                                                               \
    {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_LL_E, NULL, 0x5311202C, 0x53112030,   \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        PRV_CPSS_ALDRIN2_LL_FIFO_EQ0_Q_WR_LATENCY_FIFO_FULL_SUM_E,              \
        PRV_CPSS_ALDRIN2_LL_FIFO_EQ_CTRL_TDM_LATENCY_FIFO_OVERRUN_SUM_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define ALDRIN2_XLG_PORTS_31_61_SUMMARY_INT_SUB_TREE_MAC            \
                                                      /*bit, port */ \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  1,   31), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  2,   32), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  3,   33), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  4,   34), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  5,   35), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  6,   36), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  7,   37), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  8,   38), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  9,   39), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  10,  40), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  11,  41), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  12,  42), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  13,  43), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  14,  44), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  15,  45), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  16,  46), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  17,  47), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  18,  48), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  19,  49), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  20,  50), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  21,  51), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  22,  52), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  23,  53), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  24,  54), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  25,  55), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  26,  56), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  27,  57), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  28,  58), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  29,  59), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  30,  60), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (  31,  61)



#define ALDRIN2_XLG_PORTS_62_71_SUMMARY_INT_SUB_TREE_MAC        \
                                                      /*bit, port */  \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 1,   62), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 2,   63), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 3,   64), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 4,   65), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 5,   66), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 6,   67), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 7,   68), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 8,   69), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     ( 9,   70), \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (10,   71)

#define ALDRIN2_XLG_CPU_PORTS_SUMMARY_INT_SUB_TREE_MAC                    \
            ALDRIN2_XLG_PORT_SUMMARY_INT_SUB_TREE_MAC     (11,   72)



/*indication that the aldrin2IntrScanArr was initalized and ready for use */
/* NOTE : needed in aldrin2 due to the 'PTP fake' manipultion (look for 'fake' key word)*/
static GT_U32 aldrin2IntrScanArr_initDone = 0;

static /*const*/ PRV_CPSS_DRV_INTERRUPT_SCAN_STC aldrin2IntrScanArr[] =
{
    /* Global Interrupt Cause */
    {0, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000030, 0x00000034,
        prvCpssDrvHwPpPortGroupIsrRead,
        prvCpssDrvHwPpPortGroupIsrWrite,
        PRV_CPSS_ALDRIN2_GLOBAL_SUMMARY_FUNC_UNITS_SUM_E,
        PRV_CPSS_ALDRIN2_GLOBAL_SUMMARY_MG1_INTERNAL_SUM_E,
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 15, NULL, NULL},

        /* PEX */
        {1,GT_FALSE,0, NULL, PRV_CPSS_DRV_SCAN_PEX_CAUSE_REG_PLACE_HOLDER_CNS, PRV_CPSS_DRV_SCAN_PEX_MASK_REG_PLACE_HOLDER_CNS,
            prvCpssDrvHwPpPortGroupReadInternalPciReg,
            prvCpssDrvHwPpPortGroupWriteInternalPciReg,
            PRV_CPSS_ALDRIN2_PEX_RCV_A_E,
            PRV_CPSS_ALDRIN2_PEX_RCV_D_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},


        /* PEX ERROR */
        {2,GT_FALSE,0, NULL, PRV_CPSS_DRV_SCAN_PEX_CAUSE_REG_PLACE_HOLDER_CNS, PRV_CPSS_DRV_SCAN_PEX_MASK_REG_PLACE_HOLDER_CNS,
            prvCpssDrvHwPpPortGroupReadInternalPciReg,
            prvCpssDrvHwPpPortGroupWriteInternalPciReg,
            PRV_CPSS_ALDRIN2_PEX_DL_DWN_TX_ACC_ERR_E,
            PRV_CPSS_ALDRIN2_PEX_PEX_LINK_FAIL_E,
            FILLED_IN_RUNTIME_CNS, 0xFFDFDB, 0xFFFFFFFF, 0,NULL, NULL},

        FuncUnitsIntsSum_SUB_TREE_MAC(3),

        DataPathIntSum_SUB_TREE_MAC(4),

        /* ports0SumIntSum */
        {5,GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000080, 0x00000084,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN2_PORTS0_PORT_INT0_SUM_E,
            PRV_CPSS_ALDRIN2_PORTS0_PORT_INT30_SUM_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 31, NULL, NULL},

            /* XLG ports 0..30 bits 1..31 */
            ALDRIN2_XLG_PORTS_0_30_SUMMARY_INT_SUB_TREE_MAC,

        /* dfxIntSum */
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000000ac, 0x000000b0,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN2_DFX_SUM_DFX_DFX_CLIENT_ALD2_DQ_MACRO_CORE_CLK_INT_E,
            PRV_CPSS_ALDRIN2_DFX_SUM_DFX_DFX_CLIENT_ALD2_MPPM_BANKS_MACRO_3_MPPM_CLK_INT_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}, /* without children yet */

        /* Miscellaneous */
        {7, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000038, 0x0000003C,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN2_MISC_TWSI_TIME_OUT_E,
            PRV_CPSS_ALDRIN2_MISC_SERDES_OUT_OF_RANGE_VIOLATION_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1,NULL, NULL},
            /* HostCpuDoorbellIntSum */
            {24, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000518, 0x0000051c,
                prvCpssDrvHwPpPortGroupIsrRead,
                prvCpssDrvHwPpPortGroupIsrWrite,
                PRV_CPSS_ALDRIN2_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_0_E,
                PRV_CPSS_ALDRIN2_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_30_E,
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 1,NULL, NULL},

                AP_Doorbell_SUB_TREE_MAC(4),

        /* Tx SDMA  */
        {8, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00002810, 0x00002818,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN2_TX_SDMA_TX_BUFFER_QUEUE_0_E,
            PRV_CPSS_ALDRIN2_TX_SDMA_TX_REJECT_0_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

        /* Rx SDMA  */
        {9, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000280C, 0x00002814,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN2_RX_SDMA_RX_BUFFER_QUEUE_0_E,
            PRV_CPSS_ALDRIN2_RX_SDMA_PACKET_CNT_OF_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

        /* dfx1IntSum */
        {10, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x000000B8, 0x000000BC,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN2_DFX1_DFX_CLIENT_ALD2_DQ_MACRO_CORE_CLK_0_INT_E,
            PRV_CPSS_ALDRIN2_DFX1_DFX_CLIENT_ALD2_TCAM_LOGIC_MACRO_CORE_CLK_INT_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},

        FuncUnits1IntsSum_SUB_TREE_MAC(11),

        /* xsmi0IntSum */
        {12, GT_FALSE, PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00030010, 0x00030014,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN2_MG_XSMI0_XG_SMI_WRITE_E,
            PRV_CPSS_ALDRIN2_MG_XSMI0_XG_SMI_WRITE_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},

        /* ports1SumIntSum */
        {16,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000150, 0x00000154,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN2_PORTS1_PORT_INT31_SUM_E,
            PRV_CPSS_ALDRIN2_PORTS1_PORT_INT61_SUM_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 31, NULL, NULL},
            /* XLG ports 31..61 bits 1..31 */
            ALDRIN2_XLG_PORTS_31_61_SUMMARY_INT_SUB_TREE_MAC,

        /* ports2SumIntSum */
        {17,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x00000158, 0x0000015C,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN2_PORTS2_PORT_INT62_SUM_E,
            PRV_CPSS_ALDRIN2_PORTS2_SD1_INT_SUM_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 13, NULL, NULL},

            /* XLG ports 62..71 bits 1..10 */
            ALDRIN2_XLG_PORTS_62_71_SUMMARY_INT_SUB_TREE_MAC,

            /* CPU port */
            ALDRIN2_XLG_CPU_PORTS_SUMMARY_INT_SUB_TREE_MAC,

            /* TAI0 Interrupt Cause */
            {13, GT_FALSE, PRV_CPSS_DXCH_UNIT_TAI_E, NULL, 0x57000000, 0x57000004,
                prvCpssDrvHwPpPortGroupIsrRead,
                prvCpssDrvHwPpPortGroupIsrWrite,
                PRV_CPSS_ALDRIN2_TAI_GENERATION_INT_E,
                PRV_CPSS_ALDRIN2_TAI_DECREMENT_LOST_INT_E,
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

            /* TAI1 Interrupt Cause */
            {14, GT_FALSE, PRV_CPSS_DXCH_UNIT_TAI1_E, NULL, 0x58000000, 0x58000004,
                prvCpssDrvHwPpPortGroupIsrRead,
                prvCpssDrvHwPpPortGroupIsrWrite,
                PRV_CPSS_ALDRIN2_TAI1_GENERATION_INT_E,
                PRV_CPSS_ALDRIN2_TAI1_DECREMENT_LOST_INT_E,
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},

        /* MG1 internal Interrupt Cause */
        {18,GT_FALSE,PRV_CPSS_DXCH_UNIT_MG_E, NULL, 0x0000009C, 0x000000A0,
            prvCpssDrvHwPpPortGroupIsrRead,
            prvCpssDrvHwPpPortGroupIsrWrite,
            PRV_CPSS_ALDRIN2_MG1_INTERNAL_INTERRUPT_HOST_TO_CM3_DOORBELL_INTERRUPT1_SUM_E,
            PRV_CPSS_ALDRIN2_MG1_INTERNAL_INTERRUPT_CM3_SRAM_OOR_INT1_SUM_E,
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},

};

/* number of elements in the array of aldrin2IntrScanArr[] */
#define ALDRIN2_NUM_ELEMENTS_IN_SCAN_TREE_CNS \
    (sizeof(aldrin2IntrScanArr)/sizeof(aldrin2IntrScanArr[0]))

#define ALDRIN2_NUM_MASK_REGISTERS_CNS  (PRV_CPSS_ALDRIN2_LAST_INT_E / 32)

/* This array maps an interrupt index to its relevant interrupt mask reg    */
/* default value, cell i indicates the address for interrupts               */
/* (32 * i  --> 32 * (i + 1) - 1).                                          */
/* summary bits are turned on and the specific cause mask is enabled when   */
/* bounded by appl.                                                         */
static GT_U32 aldrin2MaskRegDefaultSummaryArr[ALDRIN2_NUM_MASK_REGISTERS_CNS] =
{
    FILLED_IN_RUNTIME_CNS
};

/* This array maps an interrupt index to its    */
/* relevant interrupt mask registers address,   */
/* cell i indicates the address for interrupts  */
/* (32 * i  --> 32 * (i + 1) - 1).              */
static GT_U32 aldrin2MaskRegMapArr[ALDRIN2_NUM_MASK_REGISTERS_CNS] =
{
    FILLED_IN_RUNTIME_CNS
};

/**
* @internal setAldrin2DedicatedEventsConvertInfo function
* @endinternal
*
* @brief   set info needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*         Aldrin2 devices.
*/
static void setAldrin2DedicatedEventsConvertInfo(IN GT_U8   devNum)
{
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataArr   = aldrin2UniEvMapTableWithExtData;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableWithExtDataSize  = aldrin2UniEvMapTableWithExtDataSize;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableArr              = aldrin2UniEvMapTable;
    prvCpssDrvPpConfig[devNum]->intCtrl.uniEvMapTableSize             = aldrin2UniEvMapTableSize;
    prvCpssDrvPpConfig[devNum]->intCtrl.numReservedPorts              = 0;

    return;
}

/**
* @internal prvCpssDrvInterruptAldrin2MaskSet function
* @endinternal
*
* @brief   masks/unmasks a given interrupt bit in the relevant
*         interrupt mask register.
*         Makes the same as prvCpssDrvInterruptMaskSet but in case of
*         interrupt PRV_CPSS_ALDRIN2_FUNCTIONAL1_SERVER_INT_E (reflects state of
*         summary bit of DFX Server interrupt summary register) additionally
*         masks/unmasks DFX Server, DFX Clients interrupts summary registers bits
*         responsible for DFX RAMs Data Integrity events.
* @param[in] evNode                   - The interrupt node representing the interrupt to be
*                                      unmasked.
* @param[in] operation                - The  to perform, mask or unmask the interrupt
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvCpssDrvInterruptAldrin2MaskSet
(
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC *evNode,
    IN CPSS_EVENT_MASK_SET_ENT      operation
)
{
    GT_STATUS rc;
    rc = prvCpssDrvInterruptMaskSet(evNode, operation);

    if (GT_OK == rc &&
        evNode->intCause   == PRV_CPSS_ALDRIN2_FUNCTIONAL1_SERVER_INT_E &&
        evNode->uniEvCause == CPSS_PP_DATA_INTEGRITY_ERROR_E)
    {
        CPSS_API_LOCK_MAC(evNode->devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        rc = prvCpssDxChDiagDataIntegrityDfxServerClientsIntMaskSet(evNode->devNum,
                                                                    operation);

        CPSS_API_UNLOCK_MAC(evNode->devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    return rc;
}

/**
* @internal prvCpssDrvPpIntDefDxChAldrin2Init function
* @endinternal
*
* @brief   Interrupts initialization for the Aldrin2 devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChAldrin2Init
(
    IN GT_U8   devNum ,
    IN GT_U32   ppRevision
)
{
    GT_STATUS   rc;         /* return code */
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
                            /* pointer to device interrupt info */

    ppRevision = ppRevision;

    /*set info needed by chIntCauseToUniEvConvertDedicatedTables(...) */
    setAldrin2DedicatedEventsConvertInfo(devNum);

    if (!aldrin2IntrScanArr_initDone)
    {

        /*
           1. fill the array of mask registers addresses
           2. fill the array of default values for the mask registers
           3. update the bits of nonSumBitMask in the scan tree
        */
        rc = prvCpssDrvExMxDxHwPpMaskRegInfoGet(
                CPSS_PP_FAMILY_DXCH_ALDRIN2_E,
                ALDRIN2_NUM_ELEMENTS_IN_SCAN_TREE_CNS,
                aldrin2IntrScanArr,
                ALDRIN2_NUM_MASK_REGISTERS_CNS,
                aldrin2MaskRegDefaultSummaryArr,
                aldrin2MaskRegMapArr);
        if(rc != GT_OK)
        {
            /* error */
            return rc;
        }

        rc =  prvCpssDrvCgPtpIntTreeWa(ALDRIN2_NUM_ELEMENTS_IN_SCAN_TREE_CNS,aldrin2IntrScanArr,
                            ALDRIN2_NUM_MASK_REGISTERS_CNS,aldrin2MaskRegMapArr);
        if(rc != GT_OK)
        {
            /* error */
            return rc;
        }

        rc = prvCpssDrvPpIntUnMappedMgSetSkip(devNum, aldrin2MaskRegMapArr,  ALDRIN2_NUM_MASK_REGISTERS_CNS);
        if(rc != GT_OK)
        {
            /* error */
            return rc;
        }

        aldrin2IntrScanArr_initDone = 1;
    }
    /* aldrin2 info */
    devInterruptInfoPtr =
        &prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_ALDRIN2_E];

    devInterruptInfoPtr->numOfInterrupts = PRV_CPSS_ALDRIN2_LAST_INT_E;
    devInterruptInfoPtr->maskRegistersAddressesArray = aldrin2MaskRegMapArr;
    devInterruptInfoPtr->maskRegistersDefaultValuesArray = aldrin2MaskRegDefaultSummaryArr;
    devInterruptInfoPtr->interruptsScanArray = aldrin2IntrScanArr;
    devInterruptInfoPtr->numOfScanElements = ALDRIN2_NUM_ELEMENTS_IN_SCAN_TREE_CNS;
    devInterruptInfoPtr->fdbTrigEndedId = 0;/* don't care */
    devInterruptInfoPtr->fdbTrigEndedCbPtr = NULL;
    devInterruptInfoPtr->hasFakeInterrupts = GT_FALSE;
    devInterruptInfoPtr->firstFakeInterruptId = 0;/* don't care */
    devInterruptInfoPtr->drvIntCauseToUniEvConvertFunc = &prvCpssDrvPpPortGroupIntCheetahIntCauseToUniEvConvert;
    devInterruptInfoPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit = 0;
    devInterruptInfoPtr->notAccessibleBeforeStartInitPtr = NULL;
    devInterruptInfoPtr->intMaskSetFptr = prvCpssDrvInterruptAldrin2MaskSet;

    return GT_OK;
}

#ifdef DUMP_DEFAULT_INFO

/**
* @internal prvCpssDrvPpIntDefAldrin2Print function
* @endinternal
*
* @brief   print the interrupts arrays info of Aldrin2 devices
*/
void  prvCpssDrvPpIntDefAldrin2Print(
    void
)
{
    cpssOsPrintf("Aldrin2 - start : \n");
    /* port group 0 */
    prvCpssDrvPpIntDefPrint(ALDRIN2_NUM_ELEMENTS_IN_SCAN_TREE_CNS, aldrin2IntrScanArr,
                            ALDRIN2_NUM_MASK_REGISTERS_CNS, aldrin2MaskRegDefaultSummaryArr, aldrin2MaskRegMapArr);
    cpssOsPrintf("Aldrin2 - End : \n");

}
#endif/*DUMP_DEFAULT_INFO*/

/**
* @internal prvCpssDrvPpIntDefAldrin2Print_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print for Aldrin2 devices the register info according to value in
*         PRV_CPSS_ALDRIN2_INT_CAUSE_ENT (assuming that this register is part of
*         the interrupts tree)
*/
void  prvCpssDrvPpIntDefAldrin2Print_regInfoByInterruptIndex(
    IN PRV_CPSS_ALDRIN2_INT_CAUSE_ENT   interruptId
)
{
    cpssOsPrintf("Aldrin2 - start regInfoByInterruptIndex : \n");

    if(interruptId >= PRV_CPSS_ALDRIN2_LAST_INT_E)
    {
        cpssOsPrintf("interruptId[%d] >= maximum(%d) \n" ,
            interruptId, PRV_CPSS_ALDRIN2_LAST_INT_E);
    }
    else
    {
        prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(ALDRIN2_NUM_ELEMENTS_IN_SCAN_TREE_CNS, aldrin2IntrScanArr, interruptId);
    }
    cpssOsPrintf("Aldrin2 - End regInfoByInterruptIndex : \n");
}
