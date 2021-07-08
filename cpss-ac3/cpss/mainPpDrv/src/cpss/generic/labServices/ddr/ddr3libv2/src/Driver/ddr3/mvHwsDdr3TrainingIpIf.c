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
* mvHwsDdr3TrainingIpIf.c
*
* DESCRIPTION:
*       Training IP function interface
*
* FILE REVISION NUMBER:
*       $Revision: 14 $
*
*******************************************************************************/

#include "mvDdr3TrainingIpPrvIf.h"
#include "mvDdr3TrainingIpBist.h"

static MV_HWS_TRAINING_IP_FUNC_PTRS hwsTrainingIpFuncsPtr;


/*******************************************************************************
* mvHwsTrainingIpIfWrite
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpIfWrite
(
    GT_U32              devNum,
    MV_HWS_ACCESS_TYPE  accessType,
    GT_U32              dunitId,
    GT_U32              regAddr,
    GT_U32              data,
    GT_U32              mask
)
{
    if (hwsTrainingIpFuncsPtr.trainingIpIfWrite == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpIfWrite(devNum, accessType, dunitId, regAddr, data, mask);
}

/*******************************************************************************
* mvHwsTrainingIpIfRead
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpIfRead
(
    GT_U32              devNum,
    MV_HWS_ACCESS_TYPE  accessType,
    GT_U32              dunitId,
    GT_U32              regAddr,
    GT_U32              *data,
    GT_U32              mask
)
{
    if (hwsTrainingIpFuncsPtr.trainingIpIfRead == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpIfRead(devNum, accessType, dunitId, regAddr, data, mask);
}



/*******************************************************************************
* mvHwsTrainingIpBusRead
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpBusRead
(
    GT_U32              devNum,
    GT_U32              uiInterfaceId,
    MV_HWS_ACCESS_TYPE  phyAccessType,
    GT_U32              phyId,
    MV_HWS_DDR_PHY      phyType,
    GT_U32              regAddr,
    GT_U32              *data
)
{
    if (hwsTrainingIpFuncsPtr.trainingIpBusRead == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpBusRead(devNum, uiInterfaceId, phyAccessType, phyId,
                                                   phyType, regAddr, data);
}

/*******************************************************************************
* mvHwsTrainingIpBusWrite
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpBusWrite
(
    GT_U32             devNum,
    MV_HWS_ACCESS_TYPE  dunitAccessType,
    GT_U32             interfaceId,
    MV_HWS_ACCESS_TYPE  phyAccessType,
    GT_U32             phyId,
    MV_HWS_DDR_PHY     phyType,
    GT_U32             regAddr,
    GT_U32             data
)
{
    if (hwsTrainingIpFuncsPtr.trainingIpBusWrite == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpBusWrite(devNum, dunitAccessType, interfaceId, phyAccessType,
                                                    phyId, phyType, regAddr, data);
}

/*******************************************************************************
* mvHwsTrainingIpAlgoRun
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpAlgoRun
(
    GT_U32            devNum,
    MV_HWS_ALGO_TYPE  algoType
)
{
    if (hwsTrainingIpFuncsPtr.trainingIpAlgoRun == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpAlgoRun(devNum, algoType);
}

/*******************************************************************************
* mvHwsTrainingIpSetFreq
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpSetFreq
(
    GT_U32              devNum,
    MV_HWS_ACCESS_TYPE   accessType,
    GT_U32              interfaceId,
    MV_HWS_DDR_FREQ frequency
)
{
    if (hwsTrainingIpFuncsPtr.trainingIpSetFreq == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpSetFreq(devNum, accessType, interfaceId, frequency);
}

/*******************************************************************************
* mvHwsTrainingIpInitController
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpInitController
(
    GT_U32   devNum,
    InitCntrParam   *initCntrPrm
)
{
    if (hwsTrainingIpFuncsPtr.trainingIpInitController == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpInitController(devNum, initCntrPrm);
}

/*******************************************************************************
* mvHwsTrainingIpPbsRx
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpPbsRx
(
    GT_U32   devNum
    )
{
    if (hwsTrainingIpFuncsPtr.trainingIpPbsRx == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpPbsRx(devNum);
}

/*******************************************************************************
* mvHwsTrainingIpPbsTx
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpPbsTx
(
    GT_U32   devNum
    )
{
    if (hwsTrainingIpFuncsPtr.trainingIpPbsTx == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpPbsTx(devNum);
}

/*******************************************************************************
* mvHwsTrainingIpSelectController
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpSelectController
(
    GT_U32    devNum,
    GT_BOOL   enable
    )
{
    if (hwsTrainingIpFuncsPtr.trainingIpSelectController == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpSelectController(devNum, enable);
}



/*******************************************************************************
* mvHwsTrainingIpExtRead
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpExtRead
(
    GT_U32    devNum,
    GT_U32    InterfcaeID,
    GT_U32    DdrAddr,
    GT_U32    numBursts,
    GT_U32    *Addr
    )
{
    if (hwsTrainingIpFuncsPtr.trainingIpExternalRead == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpExternalRead(devNum, InterfcaeID, DdrAddr, numBursts, Addr);
}


/*******************************************************************************
* mvHwsTrainingIpExtWrite
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpExtWrite
(
    GT_U32    devNum,
    GT_U32    InterfcaeID,
    GT_U32    DdrAddr,
    GT_U32    numBursts,
    GT_U32    *Addr
    )
{
    if (hwsTrainingIpFuncsPtr.trainingIpExternalWrite == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpExternalWrite(devNum, InterfcaeID, DdrAddr, numBursts, Addr);
}


/*******************************************************************************
* mvHwsTrainingIpTopologyMapLoad
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpTopologyMapLoad
(
    GT_U32                devNum,
    MV_HWS_TOPOLOGY_MAP   *topologyMap
    )
{
    if (hwsTrainingIpFuncsPtr.trainingIpTopologyMapLoad == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpTopologyMapLoad(devNum, topologyMap);
}

/*******************************************************************************
* mvHwsTrainingIpIfWrite
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpStaticConfig
(
    GT_U32                      devNum,
    MV_HWS_DDR_FREQ         frequency,
    MV_HWS_STATIC_COFIG_TYPE    staticConfigType,
    GT_U32                      interfaceId
)
{
    if (hwsTrainingIpFuncsPtr.trainingIpStaticConfig == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpStaticConfig(devNum, frequency, staticConfigType, interfaceId);
}

/*******************************************************************************
* mvHwsTrainingIpBistActivate
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpBistActivate
(
    GT_U32                  devNum,
    MV_HWS_PATTERN          pattern,
    MV_HWS_ACCESS_TYPE      accessType,
    GT_U32                  ifNum,
    MV_HWS_DIRECTION        direction,
    MV_HWS_STRESS_JUMP      addrStressJump,
    MV_HWS_PATTERN_DURATION duration,
    MV_HWS_BIST_OPERATION   operType,
    GT_U32                  offset,
    GT_U32                  csNum,
    GT_U32                  patternAddrLength
)
{
    if (hwsTrainingIpFuncsPtr.trainingIpBistActivate == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpBistActivate(devNum, pattern, accessType, ifNum,
                                                        direction, addrStressJump, duration, operType,
                                                        offset, csNum, patternAddrLength);
}

/*******************************************************************************
* mvHwsTrainingIpBistReadResult
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS    mvHwsTrainingIpBistReadResult
(
    GT_U32          devNum,
    GT_U32          interfaceId,
    BistResult      *pstBistResult
)
{
    if (hwsTrainingIpFuncsPtr.trainingIpBistReadResult == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    return hwsTrainingIpFuncsPtr.trainingIpBistReadResult(devNum, interfaceId, pstBistResult);
}

/*******************************************************************************
* mvHwsTrainingIpGetFuncPtr
*
* DESCRIPTION:
*       Get function structure pointer.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void mvHwsTrainingIpGetFuncPtr(MV_HWS_TRAINING_IP_FUNC_PTRS **hwsFuncsPtr)
{
    *hwsFuncsPtr = &hwsTrainingIpFuncsPtr;
}



/*******************************************************************************
* mvHwsDdr3TipIfInit
*
* DESCRIPTION:
*       Register DDR Training functions
*
* INPUTS:
*       MV_HWS_TRAINING_IP_FUNC_PTRS *funcPtrArray
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void mvHwsDdr3TipIfInit(MV_HWS_TRAINING_IP_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray->trainingIpInitController = mvHwsDdr3TipInitController;
    funcPtrArray->trainingIpAlgoRun= mvHwsDdr3TipRunAlg;
    funcPtrArray->trainingIpBusRead = mvHwsDdr3TipBUSRead;
    funcPtrArray->trainingIpBusWrite = mvHwsDdr3TipBUSWrite;
    funcPtrArray->trainingIpIfRead = mvHwsDdr3TipIFRead;
    funcPtrArray->trainingIpIfWrite = mvHwsDdr3TipIFWrite;
    funcPtrArray->trainingIpPbsRx = ddr3TipPbsRx;
    funcPtrArray->trainingIpPbsTx = ddr3TipPbsTx;
    funcPtrArray->trainingIpSelectController = mvHwsDdr3TipSelectDdrController;
    funcPtrArray->trainingIpSetFreq = ddr3TipFreqSet;
    funcPtrArray->trainingIpStaticConfig = NULL;
    funcPtrArray->trainingIpTopologyMapLoad = mvHwsDdr3TipLoadTopologyMap;
    funcPtrArray->trainingIpExternalRead = ddr3TipExtRead;
    funcPtrArray->trainingIpExternalWrite = ddr3TipExtWrite;
    funcPtrArray->trainingIpBistActivate = ddr3TipBistActivate;
    funcPtrArray->trainingIpBistReadResult = ddr3TipBistReadResult;
}

