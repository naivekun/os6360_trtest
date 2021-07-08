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
*****************************************************************************
* @file mvCfgElementDb.h
*
* @brief
*
* @version   12
*****************************************************************************
*/

#ifndef __mvCfgElementDb_H
#define __mvCfgElementDb_H

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

/********************** new format sequence DB ****************************************/
typedef struct
{
    GT_U16   unitId;
    GT_U32   regOffset;
    GT_U32   operData;
    GT_U32   mask;

}MV_OP_PARAMS;

typedef struct
{
    MV_OP_PARAMS   *seqParams;
    GT_U8           seqSize;

}MV_SEQ_OP_PARAMS;

/*******************************************************************************************/

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
    GT_U16   indexOffset;
    GT_U32   regOffset;
    GT_U32   data;
    GT_U32   mask;

}MV_WRITE_OP_PARAMS;

typedef struct
{
    GT_U16   indexOffset;
    GT_U32   regOffset;
    GT_U32   data;
    GT_U32   mask;

}MV_POLLING_OP_PARAMS;

typedef struct
{
    GT_U16   delay; /* msec */

}MV_DELAY_OP_PARAMS;

typedef struct
{
    MV_EL_DB_OPERATION op;
    MV_OP_PARAMS       params; 

}MV_CFG_ELEMENT;

#define MV_SEQ_SIZE(seqArray) (sizeof(seqArray) / sizeof(MV_CFG_ELEMENT))

typedef struct
{
    MV_CFG_ELEMENT  *cfgSeq;
    GT_U8           cfgSeqSize;
}MV_CFG_SEQ;


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
/**
* @internal mvCfgSeqExec function
* @endinternal
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvCfgSeqExec
(
    GT_U8            devNum,
    GT_UOPT          portGroup,
    GT_UOPT          index,
    MV_CFG_ELEMENT  *element,
    GT_U32           elementSize
);

/**
* @internal mvCfgSerdesSeqExec function
* @endinternal
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvCfgSerdesSeqExec
(
    GT_U8         devNum,
    GT_UOPT       portGroup,
    GT_UOPT       serdesNum,
    GT_U8         seqId
);


/**
* @internal mvCfgSerdesPowerUpCtrl function
* @endinternal
 *
*/
GT_STATUS mvCfgSerdesPowerUpCtrl
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        numOfSer,
    GT_U32        *serdesArr
);

/**
* @internal mvCfgSerdesSeqParallelExec function
* @endinternal
*
* @brief   Run Serdes parallel configuration sequence (in the manner of lanes)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvCfgSerdesSeqParallelExec
(
    GT_U8            devNum,
    GT_UOPT          portGroup,
    GT_UOPT          numOfSer,
    GT_UOPT          *serdesArr,
    MV_CFG_ELEMENT   *element,
    GT_U32           elementSize
);

/**
* @internal mvHwsSerdesWaitForPllConfig function
* @endinternal
 *
*/
GT_STATUS mvHwsSerdesWaitForPllConfig
(
    GT_U8            devNum,
    GT_UOPT          portGroup,
    GT_UOPT          numOfSer,
    GT_UOPT          *serdesArr
);

/**
* @internal mvHwsSerdesRxIntUpConfig function
* @endinternal
 *
*/
GT_STATUS mvHwsSerdesRxIntUpConfig
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_U32 numOfSer,
    GT_U32 *serdesArr
);

/**
* @internal mvCfgSerdesNewSeqParallelExec function
* @endinternal
*
* @brief   Run Serdes parallel configuration sequence (in the manner of lanes)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvCfgSerdesNewSeqParallelExec
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         numOfSer,
    GT_UOPT         *serdesArr,
    GT_UOPT         seqId
);

/**
* @internal mvCfgSeqBulkExec function
* @endinternal
*
* @brief   Run bulk configuration sequence (in the manner of lanes)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvCfgSeqBulkExec
(
    GT_U32           numOfSer,
    MV_HWS_ELEMENTS_ARRAY   *serdesArr,
    MV_CFG_ELEMENT   *element,
    GT_U32           elementSize
);

/**
* @internal mvCfgSeqExecInit function
* @endinternal
*
* @brief   Init configuration sequence executer
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvCfgSeqExecInit (void);


/**
* @internal mvCfgSerdesSeqExecInit function
* @endinternal
*
* @brief   Init SERDES configuration sequence executer
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvCfgSerdesSeqExecInit
(
    MV_CFG_SEQ *seqDb,
    GT_U8       numberOfSeq
);


#endif /* __mvCfgElementDb_H */


