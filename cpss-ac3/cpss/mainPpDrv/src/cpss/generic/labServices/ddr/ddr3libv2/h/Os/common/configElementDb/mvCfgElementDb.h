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
/*
* mvCfgElementDb.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 12 $
*
*******************************************************************************/

#ifndef __mvCfgElementDb_H
#define __mvCfgElementDb_H

#include "gtGenTypes.h"
#include "mvSiliconIf.h"

/* action type */
typedef enum {

    WRITE_OP,
    DELAY_OP,
    POLLING_OP,
    DUNIT_WRITE_OP,
    DUNIT_POLLING_OP,
    LAST_NON_OP

}MV_EL_DB_OPERATION;

typedef struct
{
    GT_U32   indexOffset;
    GT_U32   regOffset;
    GT_U32   data;
    GT_U32   mask;

}MV_WRITE_OP_PARAMS;

typedef struct
{
    GT_U32   indexOffset;
    GT_U32   regOffset;
    GT_U32   data;
    GT_U32   mask;
    GT_U32   waitTime;
    GT_U32   numOfLoops;

}MV_POLLING_OP_PARAMS;

typedef struct
{
    GT_U32   delay; /* msec */

}MV_DELAY_OP_PARAMS;

typedef struct
{
    MV_EL_DB_OPERATION op;
    void               *params;

}MV_CFG_ELEMENT;

typedef struct
{
    GT_U32          seqId;
    GT_U32          cfgSeqSize;
    MV_CFG_ELEMENT  *cfgSeq;

}MV_CFG_SEQ;


/********************** new format sequence DB ****************************************/
typedef struct
{
    GT_U16   unitId;
    GT_U32   regOffset;
    GT_U32   operData;
    GT_U32   mask;
    GT_U32   waitTime;
    GT_U32   numOfLoops;

}MV_OP_PARAMS;

/*******************************************************************************************/

typedef GT_STATUS (*MV_OPER_RUN_FUNCPTR)
(
    GT_U8            devNum,
    GT_UOPT          portGroup,
    GT_U32           unitBaseAddr,
    MV_CFG_ELEMENT  *element
);

typedef GT_STATUS (*MV_PARALLEL_OPER_RUN_FUNCPTR)
(
    GT_U8            devNum,
    GT_UOPT          portGroup,
    GT_UOPT          numOfSer,
    GT_UOPT          *serdesArr,
    MV_CFG_ELEMENT   *element
);

typedef GT_STATUS (*MV_BULK_SERDES_OPER_RUN_FUNCPTR)
(
    GT_U32                  numOfSer,
    MV_HWS_ELEMENTS_ARRAY   *serdesArr,
    MV_CFG_ELEMENT          *element
);

typedef GT_STATUS (*MV_SERDES_OPER_RUN_FUNCPTR)
(
    GT_U8          devNum,
    GT_UOPT        portGroup,
    GT_UOPT        serdesNum,
    MV_OP_PARAMS   *params
);

typedef GT_STATUS (*MV_PARALLEL_SERDES_OPER_RUN_FUNCPTR)
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         numOfSer,
    GT_UOPT         *serdesArr,
    MV_OP_PARAMS    *element
);

/*******************************************************************************
* mvCfgSeqExec
*
* DESCRIPTION:
*
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvCfgSeqExec
(
    GT_U8            devNum,
    GT_UOPT          portGroup,
    GT_UOPT          index,
    MV_CFG_ELEMENT  *element,
    GT_U32           elementSize
);

/*******************************************************************************
* mvCfgSerdesSeqExec
*
* DESCRIPTION:
*
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvCfgSerdesSeqExec
(
    GT_U8         devNum,
    GT_UOPT       portGroup,
    GT_UOPT       serdesNum,
    GT_U8         seqId
);

/*******************************************************************************
* mvCfgSerdesSeqparallelExec
*
* DESCRIPTION:
*      Run Serdes parallel configuration sequence (in the manner of lanes)
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvCfgSerdesSeqParallelExec
(
    GT_U8            devNum,
    GT_UOPT          portGroup,
    GT_UOPT          numOfSer,
    GT_UOPT          *serdesArr,
    MV_CFG_ELEMENT   *element,
    GT_U32           elementSize
);

/*******************************************************************************
* mvCfgSerdesNewSeqParallelExec
*
* DESCRIPTION:
*      Run Serdes parallel configuration sequence (in the manner of lanes)
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvCfgSerdesNewSeqParallelExec
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         numOfSer,
    GT_UOPT         *serdesArr,
    GT_UOPT         seqId
);

/*******************************************************************************
* mvCfgSeqBulkExec
*
* DESCRIPTION:
*      Run bulk configuration sequence (in the manner of lanes)
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvCfgSeqBulkExec
(
    GT_U32                  numOfSer,
    MV_HWS_ELEMENTS_ARRAY   *serdesArr,
    MV_CFG_ELEMENT          *element,
    GT_U32                  elementSize
);

/*******************************************************************************
* mvCfgSeqExecInit
*
* DESCRIPTION:
*      Init configuration sequence executer
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void mvCfgSeqExecInit ();


/*******************************************************************************
* mvCfgSerdesSeqExecInit
*
* DESCRIPTION:
*      Init SERDES configuration sequence executer
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void mvCfgSerdesSeqExecInit
(
    MV_CFG_SEQ *seqDb,
    GT_U8       numberOfSeq
);

void hwsGenSeqInit(MV_CFG_SEQ *dbPtr, GT_U8 numOfSeq);


#endif /* __mvCfgElementDb_H */

