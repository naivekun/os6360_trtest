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
* @file prvCpssConfigTypes.h
* @version   53
********************************************************************************
*/

#ifndef __prvCpssConfigTypes_h
#define __prvCpssConfigTypes_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
/* get public types */
#include <cpss/common/cpssTypes.h>
/* get the OS , extDrv functions*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
/* get common defs */
#include <cpssCommon/cpssPresteraDefs.h>
/* include HW init definitions */
#include <cpss/common/cpssHwInit/cpssHwInit.h>
/* get multi-port group utilities*/
#include <cpss/common/cpssHwInit/private/prvCpssHwMultiPortGroups.h>
/* get registers access utilities : counters ,busy wait , multi-port groups */
#include <cpss/common/cpssHwInit/private/prvCpssHwRegisters.h>
/* get the interrupts control info */
#include <cpss/common/config/private/prvCpssGenIntDefs.h>
/* include the "cpss drivers APIs" */
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
/* get the port control info */
#include <cpss/common/port/private/prvCpssPortTypes.h>
/* get common registers DB definition */
#include <cpss/common/config/private/prvCpssCommonRegs.h>
/* get the common diag info */
#include <cpss/common/diag/private/prvCpssCommonDiag.h>

#ifdef CHX_FAMILY
/*needed for PRV_CPSS_TM_DEVS_ENT */
#include <cpss/generic/tm/prvCpssTmDefs.h>
/*needed for CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS */
#include <cpss/generic/tm/cpssTmCtl.h>
/* needed for PRV_CPSS_TRUNK_DB_INFO_STC */
#include <cpss/generic/trunk/private/prvCpssTrunkTypes.h>
/* needed for PRV_CPSS_NETIF_MII_TX_CTRL_STC*/
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfMii.h>
#else
typedef GT_U32  PRV_CPSS_TM_DEVS_ENT;
#define CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS 1
#endif /*CHX_FAMILY*/


/* check if device is EXMX , according to it's family */
#define CPSS_IS_EXMX_FAMILY_MAC(devFamily)      \
    (((devFamily) > CPSS_PP_FAMILY_START_EXMX_E &&    \
      (devFamily) < CPSS_PP_FAMILY_END_EXMX_E)?1:0)


/* check if device is EXMXMP , according to it's family */
#define CPSS_IS_EXMXPM_FAMILY_MAC(devFamily)      \
    (((devFamily) > CPSS_PP_FAMILY_START_EXMXPM_E &&    \
      (devFamily) < CPSS_PP_FAMILY_END_EXMXPM_E)?1:0)

/* check if device is DXCH , according to it's family */
#define CPSS_IS_DXCH_FAMILY_MAC(devFamily)      \
    (((devFamily) > CPSS_PP_FAMILY_START_DXCH_E &&    \
      (devFamily) < CPSS_PP_FAMILY_END_DXCH_E)?1:0)

/* check if device is DXSAL , according to it's family */
#define CPSS_IS_DXSAL_FAMILY_MAC(devFamily)      \
    (((devFamily) == CPSS_PP_FAMILY_SALSA_E) ? 1 : 0)

/* check if device is PX , according to it's family */
#define CPSS_IS_PX_FAMILY_MAC(devFamily)      \
    (((devFamily) > CPSS_PX_FAMILY_START_E &&    \
      (devFamily) < CPSS_PX_FAMILY_END_E)?1:0)


/* check that the device exists */
#define PRV_CPSS_IS_DEV_EXISTS_MAC(devNum)       \
    (((devNum) >= PRV_CPSS_MAX_PP_DEVICES_CNS || \
        prvCpssPpConfig[devNum] == NULL) ? 0 : 1)

/* check that the device exists
    return GT_BAD_PARAM on error
*/
#define PRV_CPSS_DEV_CHECK_MAC(devNum)                  \
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))         \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, prvCpssLogErrorMsgDeviceNotExist, devNum);



/* macro to get a pointer on the ExMx device (exMx format)
    devNum - the device id of the exMx device

    NOTE : the macro do NO validly checks !!!!
           (caller responsible for checking with other means/macro)
*/
#ifndef CPSS_USE_MUTEX_PROFILER
#define PRV_CPSS_PP_MAC(devNum) \
    ((PRV_CPSS_GEN_PP_CONFIG_STC*)prvCpssPpConfig[devNum])
#else
#ifdef  WIN32
 #define PRV_CPSS_PP_MAC(devNum)  ((PRV_CPSS_GEN_PP_CONFIG_STC*)prvCpssGetPrvCpssPpConfigInGenFormat(devNum,__FUNCTION__))
#else
 #define PRV_CPSS_PP_MAC(devNum)   ((PRV_CPSS_GEN_PP_CONFIG_STC*)prvCpssGetPrvCpssPpConfigInGenFormat(devNum,__func__))
#endif
#endif/*CPSS_USE_MUTEX_PROFILER*/

/* check that the port number is valid virtual port (CPU port is invalid)
    return GT_BAD_PARAM on error
*/
#define PRV_CPSS_VIRTUAL_PORT_CHECK_MAC(devNum,portNum)        \
    if(portNum >= PRV_CPSS_PP_MAC(devNum)->numOfVirtPorts) \
    {                                 \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);          \
    }

/* check that the port number is valid physical port (CPU port is invalid)
    return GT_BAD_PARAM on error
*/
#define PRV_CPSS_PHY_PORT_CHECK_MAC(devNum,portNum)            \
    if((portNum >= PRV_CPSS_PP_MAC(devNum)->numOfPorts) ||     \
        (0 == (CPSS_PORTS_BMP_IS_PORT_SET_MAC(                 \
            &(PRV_CPSS_PP_MAC(devNum)->existingPorts) , portNum)))) \
    {                                 \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "MAC[%d] not exist in [existingPorts]",portNum); \
    }

/* check that port is exist in the existing ports bitmap,       */
/* the macro does not check that port not exeeds the numOfPorts */
#define PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portNum)              \
        (0 != (CPSS_PORTS_BMP_IS_PORT_SET_MAC(                       \
            &(PRV_CPSS_PP_MAC(devNum)->existingPorts) , portNum)))

/* Check for device's port existence and skip if port does not exist */
#define PRV_CPSS_SKIP_NOT_EXIST_PORT_MAC(_dev, _port) \
    if(!PRV_CPSS_PHY_PORT_IS_EXIST_MAC(_dev, _port)) \
        continue;

/* access to the device's trunk info of the device */
#define PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)  \
    (&(PRV_CPSS_PP_MAC(devNum)->trunkInfo))

/* bmp for hw interfaces */
#define PRV_CPSS_HW_IF_PCI_BMP_CNS      (1 << CPSS_CHANNEL_PCI_E)
#define PRV_CPSS_HW_IF_SMI_BMP_CNS      (1 << CPSS_CHANNEL_SMI_E)
#define PRV_CPSS_HW_IF_TWSI_BMP_CNS     (1 << CPSS_CHANNEL_TWSI_E)
#define PRV_CPSS_HW_IF_PEX_BMP_CNS      (1 << CPSS_CHANNEL_PEX_E)
#define PRV_CPSS_HW_IF_PEX_MBUS_BMP_CNS (1 << CPSS_CHANNEL_PEX_MBUS_E)

/*set the HW interface bmp */
#define PRV_CPSS_HW_IF_BMP_MAC(mngInterfaceType)\
    (1 << (GT_U32)mngInterfaceType)

/* bmp of "PCI" compatible behavior */
#define PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(devNum)\
    ((PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp &        \
     PRV_CPSS_HW_IF_PCI_BMP_CNS) ? 1 : 0)

/* bmp of "SMI" compatible behavior */
#define PRV_CPSS_HW_IF_SMI_COMPATIBLE_MAC(devNum)\
    ((PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp &        \
     PRV_CPSS_HW_IF_SMI_BMP_CNS) ? 1 : 0)

/* bmp of "TWSI" compatible behavior */
#define PRV_CPSS_HW_IF_TWSI_COMPATIBLE_MAC(devNum)\
    ((PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp &        \
     PRV_CPSS_HW_IF_TWSI_BMP_CNS) ? 1 : 0)

/* bmp of "PEX" compatible behavior */
#define PRV_CPSS_HW_IF_PEX_COMPATIBLE_MAC(devNum)\
    ((PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp &        \
     PRV_CPSS_HW_IF_PEX_BMP_CNS) ? 1 : 0)

/* bmp of "PEX_MBUS" compatible behavior */
#define PRV_CPSS_HW_IF_PEX_MBUS_COMPATIBLE_MAC(devNum)   \
    ((PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp &        \
     PRV_CPSS_HW_IF_PEX_MBUS_BMP_CNS) ? 1 : 0)

/* Current primary AUQ index */
#define PRV_CPSS_AUQ_INDEX_MAC(devNum, portGroup)\
    (PRV_CPSS_PP_MAC(devNum)->intCtrl.activeAuqIndex[portGroup])

/* Current secondary AUQ index */
#define PRV_CPSS_SECONDARY_AUQ_INDEX_MAC(devNum, portGroup)\
    (PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[portGroup].activeSecondaryAuqIndex)

/* Secondary AUQ check condition */
#define PRV_CPSS_SECONDARY_AUQ_CHECK_MAC(devNum, portGroup)\
    ((PRV_CPSS_PP_MAC(devNum)->intCtrl.activeAuqIndex[portGroup] == \
      PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[portGroup].activeSecondaryAuqIndex) \
      ? GT_TRUE : GT_FALSE)

/* check that the AU descriptors initialized (relevant only to DMA supporting device) */
#define PRV_CPSS_AUQ_DESCRIPTORS_INIT_DONE_CHECK_MAC(devNum)        \
    if(PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl[PRV_CPSS_FIRST_ACTIVE_PORT_GROUP_ID_MAC(devNum)].blockAddr == 0)  \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG)
/* check that the secondary AU descriptors initialized (relevant only to DMA supporting device) */
#define PRV_CPSS_SECONDARY_AUQ_DESCRIPTORS_INIT_DONE_CHECK_MAC(devNum)        \
    if(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl[PRV_CPSS_FIRST_ACTIVE_PORT_GROUP_ID_MAC(devNum)].blockAddr == 0)  \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG)

/* check that the FU descriptors initialized (relevant only to DMA supporting device) */
#define PRV_CPSS_FUQ_DESCRIPTORS_INIT_DONE_CHECK_MAC(devNum)        \
    if(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[PRV_CPSS_FIRST_ACTIVE_PORT_GROUP_ID_MAC(devNum)].blockAddr == 0)  \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG)

/* use HW devNum and not the 'SW devNum' */
#define PRV_CPSS_HW_DEV_NUM_MAC(devNum)  \
    (PRV_CPSS_PP_MAC(devNum)->hwDevNum)

/* macro to check if current iteration is not valid any more
    devNum - the device number to check with it it's max number
             the device number is controlling the "HW interface bus" that allow
             us to tune the needed value for max number of iterations
    currIter - the current iteration number , should be 0...max in increasing
               manner
*/
#define  __MAX_NUM_ITERATIONS_CHECK_CNS(devNum,currIter)    \
    if((currIter) >= PRV_CPSS_PP_MAC(devNum)->maxIterationsOfBusyWait) \
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, prvCpssLogErrorMsgIteratorNotValid, devNum,currIter);

#ifndef ASIC_SIMULATION
    #define  PRV_CPSS_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,currIter)           \
        __MAX_NUM_ITERATIONS_CHECK_CNS(devNum,currIter)
#else /* ASIC_SIMULATION */
    /* for the GM most operations ended in context of 'triggering' except for
       FDB 'aging daemon' */
    /* for non GM (WM): we need to allow the 'other task' to process the operation
       that we wait for */
    #define  PRV_CPSS_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,currIter)           \
        {                                                                     \
            /* use 'tmp' because the currIter used with ++ at the caller */   \
            GT_U32  tmpIter = currIter;                                       \
            __MAX_NUM_ITERATIONS_CHECK_CNS(devNum,tmpIter);                   \
            /* currently do not change behavior on GM of EXMXPM */            \
            if(CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))   \
            {                                                                 \
                if((tmpIter) >= 2000)/*in any case 20 seconds are enough */   \
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);                                        \
                else if(tmpIter > 3)                                          \
                    cpssOsTimerWkAfter(10);                                   \
                else /* iteration 0,1,2,3 */                                  \
                    cpssOsTimerWkAfter(0); /* allow context switch */         \
            }                                                                 \
        }
#endif /*ASIC_SIMULATION*/

/* maximal number of iterations for registers read through SMI bus */
#define PRV_CPSS_SMI_RETRY_COUNTER_CNS       1000

/* macro to check if SMI current iteration is not valid any more
    devNum - the device number to check with it it's max number
             the device number is controlling the "SMI bus" that allow
             us to tune the needed value for max number of iterations
    currIter - the current iteration number , should be 0...max in increasing
               manner

    NOTE: macro return GT_NOT_READY if 'over the limit'
*/
#define  __SMI_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,currIter,rc_error)    \
    if((currIter) >= PRV_CPSS_PP_MAC(devNum)->maxIterationsOfSmiWait) \
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc_error, prvCpssLogErrorMsgIteratorNotValid, devNum,currIter);

#ifndef ASIC_SIMULATION
    #define  PRV_CPSS_SMI_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,currIter,rc_error)       \
        __SMI_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,currIter,rc_error)
#else /* ASIC_SIMULATION */
    /* for the GM most operations ended in context of 'triggering' except for
       FDB 'aging daemon' */
    /* for non GM (WM): we need to allow the 'other task' to process the operation
       that we wait for */
    #define  PRV_CPSS_SMI_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,currIter,rc_error)       \
        {                                                                     \
            /* use 'tmp' because the currIter used with ++ at the caller */   \
            GT_U32  tmpIter = currIter;                                       \
            __SMI_MAX_NUM_ITERATIONS_CHECK_CNS(devNum,tmpIter,rc_error);      \
            /* currently do not change behavior on GM of EXMXPM */            \
            if(CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))   \
            {                                                                 \
                if((tmpIter) >= 2000)/*in any case 20 seconds are enough */   \
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc_error, LOG_ERROR_NO_MSG);\
                else if(tmpIter > 50)                                         \
                    cpssOsTimerWkAfter(10);                                   \
                else /* iteration 0-50 */                                     \
                    cpssOsTimerWkAfter(0); /* allow context switch */         \
            }                                                                 \
        }
#endif /*ASIC_SIMULATION*/

/* macro to check if current iteration is not valid any more for PHY Driver
    currIter - the current iteration number , should be 0...max in increasing
               manner
*/
#define  PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS(currIter)    \
    if((currIter) >= PRV_CPSS_SMI_RETRY_COUNTER_CNS)                    \
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, prvCpssLogErrorMsgGlobalIteratorNotValid, currIter);

/* number of directions that the device support - Egress and Ingress*/
#define PRV_CPSS_MAX_DIRECTIONS_CNS 2

/* Each direction ingress and egress have 2 Timestamp Queues */
#define PRV_CPSS_MAX_PTP_TIMESTAMP_QUEUES_CNS 2
typedef struct{
    GT_U32  numOfPipes;/* number of pipes. (per core)
                        the bobcat3 is single core but with 2 control pipes
                        value 0 and value 1 meaning : no multi pipe.
                    */
    GT_U32  numOfPortsPerPipe; /* number of GOP ports per pipe , NOT including the 'CPU Port' . */

    GT_U32  numOfTiles; /* number of tiles , This is multi tile device .*/
    GT_U32  numOfPipesPerTile; /* number of pipes per tile , This is multi tile device . */
    GT_U32  tileOffset;/* offset in bytes between tiles */
    GT_U32  mirroredTilesBmp; /* BMP of mirrored tiles . in Falcon tile 1,3 are mirror image of tile 0 */
}PRV_CPSS_GEN_PP_MULTI_PIPES_INFO_STC;

/* check if the device is 'multi-pipes' device */
#define PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum) \
    ((PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes >= 2) ? 1 : 0)

/* max number of ports in single port group.
   NOTE: event that the Lion actually support only 12 ports,
        still it 'reserve' 16 ports for calculations:
            <<4 , & 0xf

    (APPLICABLE DEVICES: Lion; Lion2)
*/
#define PRV_CPSS_GEN_PP_MAX_PORTS_IN_PORT_GROUP_CNS   16

/**
* @struct PRV_CPSS_GEN_PP_PORT_GROUPS_INFO_STC
 *
 * @brief A structure to hold info about the port groups of a device
 * NOTE: this structure relevant ALSO to a 'non-multi port groups' device
 * -- currently used only in the DXCH and not in the exmxpm,exmx,dxsal
*/
typedef struct{

    /** is the device : 'multi */
    GT_BOOL isMultiPortGroupDevice;

    /** @brief The number of port groups that the device support.
     *  NOTE: this number not depend on the number of active port groups ,
     *  for example this value is 4 always for Lion.
     */
    GT_U32 numOfPortGroups;

    /** Bitmap of active port groups */
    GT_U32 activePortGroupsBmp;

    /** The portGroupId of the first active port group id */
    GT_U32 firstActivePortGroup;

    /** @brief The portGroupId of the last active port group id,
     *  Note that there can be non active port groups
     *  between this port group and the firstActivePortGroup
     *  used for loops and for allocations for port groups.
     *  --> used instead of : 'num Of port groups'
     *  dynamic allocate fields that are 'per port group' , according
     *  to (lastValidPortGroup + 1)
     */
    GT_U32 lastActivePortGroup;

    /** @brief The portGroupId that used for 'cpu port' as SDMA/RGMII.
     *  for 'non multi port group' device --> 0
     *  NOTE: 1. Lion: For RGMII should be 3
     *  2. Lion: For SDMA should be CPSS_PORT_GROUP_UNAWARE_MODE_CNS
     *  3. This portGroupId used only for 'cpu port' and not for all networkIf
     *  configurations that may use firstActivePortGroup!!!
     *  4. in Lion2/Puma3 the physical CPU port is valid only
     *  in one of the network ports groups.
     */
    GT_U32 cpuPortPortGroupId;

    /** @brief in Puma3 the physical loopback port is valid only
     *  in one of the network ports groups.
     */
    GT_U32 loopbackPortGroupId;

    /** @brief 'last served' port group# for AUQ .
     *  needed for the 'Round Robin' of the AUQ manager
     *  for 'non multi port groups' device --> 0
     */
    GT_U32 auqPortGroupIdLastServed;

    /** @brief 'last served' port group# for FUQ .
     *  needed for the 'Round Robin' of the FUQ manager
     *  for 'non multi port groups' device --> 0
     */
    GT_U32 fuqPortGroupIdLastServed;

    /** @brief 'Next to serve' port group# for Secure Breach
     *  messages. Needed for the 'Round Robin' of the Secure Breach messages
     *  manager. for 'non multi port groups' device --> 0
     */
    GT_U32 secureBreachNextPortGroupToServe;

    /** @brief 'Next to serve' port group# for PTP FIFO entries.
     *  Needed for the 'Round Robin' of the PTP FIFO entries
     *  manager. for 'non multi port groups' device --> 0
     *  applicable devices : Lion,Lion2
     */
    GT_U32 ptpNextPortGroupToServe;

    GT_U32 ptpIngressEgressNextPortGroupToServe[PRV_CPSS_MAX_DIRECTIONS_CNS][PRV_CPSS_MAX_PTP_TIMESTAMP_QUEUES_CNS];

    /** @brief 'Next to serve' port group# for 'illegal access to shared tables' entries.
     *  Needed for the 'Round Robin' manager.
     *  NOTE: used also for 'non multi port groups' device !!! (due to the 'table to serve')
     *  applicable devices : Falcon
     */
    struct {
        GT_U32 nextPortGroupToServe;
        GT_U32 nextTableToServe;/* one of : CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ENT */
    }sharedTablesIllegalAccess;


    /** @brief 'Next to serve' port group# for pha violation captured register(s).
     *  Needed for the 'Round Robin' of the registers.
     *  for 'non multi port groups' device --> 0
     *  applicable devices : Falcon.
     */
    GT_U32 phaViolationCapturedNextPortGroupToServe;


} PRV_CPSS_GEN_PP_PORT_GROUPS_INFO_STC;

/* macro to start a loop on indexes : from iiStart to iiLast (inclusive) and skip those not in bmp of iiMask */
#define PRV_CPSS_GEN_START_LOOP_INDEX_MAC(devNum,ii,iiStart,iiLast,iiMask)   \
    /* loop on all port groups */                   \
    for((ii) = (iiStart); (ii) <= (iiLast); (ii)++) \
    {                                               \
        if(0 == ((iiMask) & (1<<(ii))))             \
        {                                           \
            continue;                               \
        }

/* macro to end a loop on active port groups */
#define PRV_CPSS_GEN_END_LOOP_INDEX_MAC(devNum,ii)  \
    } /* end of 'for' */


/* macro to start a loop on active port groups */
#define PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)   \
    /* loop on all port groups */                                        \
    PRV_CPSS_GEN_START_LOOP_INDEX_MAC(devNum,portGroupId,                \
        PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup,    \
        PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup,     \
        PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp)

/* macro to end a loop on active port groups */
#define PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)     \
    PRV_CPSS_GEN_END_LOOP_INDEX_MAC(devNum,portGroupId)


/* macro to start to loop on port groups according to portGroupsBmp , but only
   on active port groups */
#define PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)   \
    /* loop on all port groups */                                        \
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)       \
    {                                                                    \
        /* skip port groups that are not in bmp */                       \
        if(0 == (portGroupsBmp & (1<<(portGroupId))))                    \
        {                                                                \
            continue;                                                    \
        }

/* macro to end a loop on port groups according to portGroupsBmp */
#define PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)     \
    }                                                               \
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)


/* macro to start a loop on NON-active port groups */
#define PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum,portGroupId)     \
    /* loop on all port groups */                                     \
    for((portGroupId) = 0; (portGroupId) < PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.numOfPortGroups; (portGroupId)++)\
    {                                                                    \
        if(0 != (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (1<<(portGroupId))))  \
        {                                                                \
            continue;                                                    \
        }

/* macro to end a loop on NON-active port groups */
#define PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum,portGroupId)     \
    } /* end of 'for' */


/* check if the device is 'multi-port groups' device */
#define PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) \
    (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.isMultiPortGroupDevice == GT_TRUE)

/* check if the device is 'multi-port groups' device , non-sip5 devices style.
    (Applicable devices: Lion/Lion2/Hooper/Puma3)
   those devices not hold local port 12,13,14,15 in each port group */
#define PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum)    \
    ((PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.isMultiPortGroupDevice == GT_TRUE) && !PRV_CPSS_SIP_5_CHECK_MAC(devNum))

/* check if the device is SIP5 'multi-port groups' device , sip5 devices style.
    (Applicable devices: Bobcat3)
*/
#define PRV_CPSS_IS_SIP5_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) \
    ((PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.isMultiPortGroupDevice == GT_TRUE) && PRV_CPSS_SIP_5_CHECK_MAC(devNum))

/* macro to get the first active port group */
#define PRV_CPSS_FIRST_ACTIVE_PORT_GROUP_ID_MAC(devNum)     \
     PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup

/* macro to get the last active port group */
#define PRV_CPSS_LAST_ACTIVE_PORT_GROUP_ID_MAC(devNum)     \
     PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup

/* macro to get the 'CPU port' portGroupId.
    NOTE: for RGMII interface it will be single portGroupId (in Lion portGroupId #3)
          for SDMA interface it will be 'All port groups'
*/
#define PRV_CPSS_CPU_PORT_PORT_GROUP_ID_MAC(devNum)     \
     PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.cpuPortPortGroupId

/* macro to get the physical 'loopback port' portGroupId. */
#define PRV_CPSS_LOOPBACK_PORT_PORT_GROUP_ID_MAC(devNum)     \
     PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.loopbackPortGroupId


/* macro to get the portGroupId for SDMA configurations .
   that way we allow application to give SDMA memory to single port group instead
   of split it between all active port groups.

   ONLY the settings of descriptors that relate to RX/TX packets, will use this
   portGroupId.

   NOTE:
       1. all other non-SDMA relate registers, are set to all active port groups.
       2. this is not relate to 'CPU port' configurations
*/
#define PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum)   \
    PRV_CPSS_PP_MAC(devNum)->netifSdmaPortGroupId

/* check if port number in range that need local port conversion */

#define PRV_CPSS_PORT_GROUP_CONVERT_CHECK_MAC(portNum)  \
    (((portNum) < CPSS_NULL_PORT_NUM_CNS || (portNum) > CPSS_CPU_PORT_NUM_CNS)? 1 : 0)

extern GT_U32 /*portGroupId*/ prvCpssGlobalPortToPortGroupIdConvert(IN GT_U8 devNum , IN GT_U32 global_macPortNum);

/* convert 'global' port number to 'portGroupId' */
#define PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum) \
    prvCpssGlobalPortToPortGroupIdConvert(devNum, portNum)

extern GT_U32 /*local_macPortNum*/ prvCpssGlobalPortToLocalPortConvert(IN GT_U8 devNum , IN GT_U32 global_macPortNum);

/* convert 'global' port number to 'local' port number */
#define PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum)     \
    prvCpssGlobalPortToLocalPortConvert(devNum, portNum)

/*
NOTE: the #define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
    should be only in C files that handle the MAC for the CPU port

    and they need the difference between
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC ,
        PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC
*/
extern GT_U32 /*portGroupId*/ prvCpssPhysicalGlobalPortToPortGroupIdConvert(IN GT_U8 devNum , IN GT_U32 global_macPortNum);

#ifdef PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
    /* convert 'Physical global' port number to 'portGroupId'
        this macro must be used for 'MAC registers' for physical ports.
        (in the 'Port Library')

        this macro different from the PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC
        for the treatment for CPU port.
    */
    #define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum) \
        prvCpssPhysicalGlobalPortToPortGroupIdConvert(devNum, portNum)

#else  /*!PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS*/
    /* Not allowing this C file to use this macro since it is not dealing with
        the MAC of the port.

        !!! So we create compilation error !!!   'Undeclared identifier'

        caller must instead use PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum)
    */
    #define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum) \
        use_of_this_macro_not_allowed_from_this_C_file  /* undeclared identifier */

#endif /*!PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS*/

extern GT_U32 /*global_macPortNum*/ prvCpssLocalPortToGlobalPortConvert(IN GT_U8 devNum ,IN GT_U32 portGroupId , IN GT_U32 local_macPortNum);

/* convert 'local' port number to 'global' port number */
#define PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum, portGroupId, portNum) \
    prvCpssLocalPortToGlobalPortConvert(devNum, portGroupId, portNum)


extern GT_U32 /*local_macPortNum*/ prvCpssSip5GlobalPhysicalPortToLocalMacConvert(IN GT_U8 devNum , IN GT_U32 global_physicalPortNum);

/* sip5 : convert 'global' physical port number to 'local' MAC port number */
#define PRV_CPSS_SIP5_GLOBAL_PHYSICAL_PORT_TO_LOCAL_MAC_CONVERT_MAC(devNum, global_physicalPortNum) \
    prvCpssSip5GlobalPhysicalPortToLocalMacConvert(devNum, global_physicalPortNum)

extern GT_U32 /*portGroupId*/ prvCpssSip5GlobalPhysicalPortToPortGroupIdConvert(IN GT_U8 devNum , IN GT_U32 global_physicalPortNum);

/* sip5 : convert 'global' physical port number to port group number */
#define PRV_CPSS_SIP5_GLOBAL_PHYSICAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, global_physicalPortNum) \
    prvCpssSip5GlobalPhysicalPortToPortGroupIdConvert(devNum, global_physicalPortNum)

/* check and return 'bad param' when device is multi-port group device , but
   portGroupsBmp indicate non active port groups
   for non-multi port groups device --> macro SETs the  portGroupsBmp to
   CPSS_PORT_GROUP_UNAWARE_MODE_CNS

   macro's logic :
   1. for non-multi port group device --> SETs the  portGroupsBmp to
      CPSS_PORT_GROUP_UNAWARE_MODE_CNS
   2. when portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS --> never error
   3. when portGroupsBmp == 0 --> error
   4. when ((portGroupsBmp &activePortGroupsBmp) != portGroupsBmp)   --> error
*/

#define PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp)  \
    if(!PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))               \
    {                                                                   \
        /* we ignore parameter from application and use 'unaware' */    \
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;               \
    }                                                                   \
    /* support multi core device */                                     \
    else if ((portGroupsBmp != CPSS_PORT_GROUP_UNAWARE_MODE_CNS) &&     \
             (portGroupsBmp == 0 ||                                     \
             ((PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & portGroupsBmp) != portGroupsBmp)))  {\
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, prvCpssLogErrorMsgPortGroupBitmapNotValid, devNum, portGroupsBmp);   \
             }

extern GT_STATUS prvCpssMultiPortGroupsBmpCheckSpecificUnit(IN GT_U8 devNum ,INOUT GT_PORT_GROUPS_BMP *portGroupsBmpPtr , IN PRV_CPSS_DXCH_UNIT_ENT unitId);
/* check port group bmp awareness for specific unit.
    for example:
    the Bobcat3 is multi-port group device , but the FDB unit is single one, so
    the portGroupsBmp from the application should be ignored (like in BC2 / xcat3)

    unitId - is one of PRV_CPSS_DXCH_UNIT_ENT

        NOTE: the MACRO relevant to SIP5 port groups devices. other devices are treated as :
            PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC
*/
#define PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,unitId)  \
    {                                                                                          \
        GT_STATUS   rc;                                                                        \
        rc = prvCpssMultiPortGroupsBmpCheckSpecificUnit(devNum,&portGroupsBmp,unitId); \
        if(rc != GT_OK)                                                                        \
        {                                                                                      \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                     \
        }                                                                                      \
    }

extern GT_STATUS prvCpssMultiPortGroupsBmpCheckSpecificTable(IN GT_U8 devNum ,INOUT GT_PORT_GROUPS_BMP *portGroupsBmpPtr,IN GT_U32 /*CPSS_DXCH_TABLE_ENT*/tableId);
/*
*       Check port group bmp awareness for specific table in the device.
*       check and return 'bad param' when device is multi-port group device ,
*       but portGroupsBmp indicate non active port groups
*       for non-multi port groups device --> function updates the portGroupsBmp
*       to CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*
*       for example:
*       the Bobcat3 is multi-port group device , but the ARP/TS table is single one,
*       although the HA unit is 'per pipe'. So the portGroupsBmp from the application
*       should be ignored (like in BC2 / xcat3)
*
*        NOTE: the MACRO relevant to SIP5 port groups devices. other devices are treated as :
*            PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC
*/
#define PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_TABLE_MAC(devNum,portGroupsBmp,tableId)  \
    {                                                                                          \
        GT_STATUS   rc;                                                                        \
        rc = prvCpssMultiPortGroupsBmpCheckSpecificTable(devNum,&portGroupsBmp,tableId); \
        if(rc != GT_OK)                                                                        \
        {                                                                                      \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                     \
        }                                                                                      \
    }

/*
    This macro assigns the portGroup variable to first port group
    of the given device contained in given bitmap.
    If there is no such it returns GT_BAD_PARAM.
*/
#define PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(            \
    devNum, portGroupsBmp, portGroupId)                                 \
    if (GT_OK != prvCpssPpConfigPortGroupFirstActiveGet(                \
        devNum, portGroupsBmp, &portGroupId))                           \
    {                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);  \
    }

/* check device by not applicable families bitmap */
#define PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(_devNum, _unappFamBmp)  \
   if (PRV_CPSS_PP_MAC(_devNum)->appDevFamily & (_unappFamBmp))       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG) /* ; in caller */

extern CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT prvCpssHwDevNumModeDb[BIT_12];

/* check if HW device number represents device that uses dual device Id
   input : HW Device Number (not SW devNum !)
   return :
   0 - not dual device Id
   1 - dual device Id
*/
#define PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(hwDevNum) \
    ((((hwDevNum) < BIT_12) && (prvCpssHwDevNumModeDb[(hwDevNum)] == CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E) ?\
      1 : 0))

/* check that when HW device number represents device with dual device Id used
   only even number (not odd)
   returns GT_BAD_PARAM when : dual device Id with odd number
*/
#define PRV_CPSS_DUAL_HW_DEVICE_CHECK_MAC(hwDevNum) \
        if (PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(hwDevNum) && ((hwDevNum) %2)) \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)


/**
* @enum CPSS_PP_FAMILY_BIT_ENT
 *
 * @brief The PP family representing bit.
 * Used to check the device in not applicable families bitmap.
 * The same values used for DxCh and for ExMxPm families
*/
typedef enum{

    /** undefined device */
    CPSS_UNDEFINED_DEVICE_E  = 0,

    /** (DXCH) Cheetah family devices */
    CPSS_CH1_E         = BIT_0,

    /** (DXCH) Cheetah Diamond family devices */
    CPSS_CH1_DIAMOND_E = BIT_1,

    /** (DXCH) Cheetah 2 family devices */
    CPSS_CH2_E         = BIT_2,

    /** (DXCH) Cheetah 3 family devices */
    CPSS_CH3_E         = BIT_3,

    /** (DXCH) xCat family devices */
    CPSS_XCAT_E        = BIT_4,

    /** (DXCH) Lion family devices */
    CPSS_LION_E        = BIT_5,

    /** (DXCH) xCat2 family devices */
    CPSS_XCAT2_E       = BIT_6,

    /** (DXCH) Lion2 family devices */
    CPSS_LION2_E       = BIT_7,

    /** (EXMXPM) Puma(2) family devices */
    CPSS_PUMA_E        = BIT_8,

    /** (EXMXPM) Puma3 family devices */
    CPSS_PUMA3_E       = BIT_9,

    /** (DXCH) Bobcat2 family devices */
    CPSS_BOBCAT2_E     = BIT_11,

    /** (DXCH) xCat3 family devices */
    CPSS_XCAT3_E       = BIT_12,

    /** (DXCH) Bobcat3 family devices */
    CPSS_BOBCAT3_E     = BIT_13,

    /** (DXCH) Caelum family devices */
    CPSS_CAELUM_E      = BIT_14,

    /** (DXCH) Aldrin family devices */
    CPSS_ALDRIN_E      = BIT_15,

    /** (DXCH) AC3X family devices */
    CPSS_AC3X_E        = BIT_16,

    /** (DXCH) Aldrin2 family devices */
    CPSS_ALDRIN2_E     = BIT_17,

    /** (DXCH) Falcon family devices */
    CPSS_FALCON_E      = BIT_18,

    /** (PX) Pipe family devices */
    CPSS_PIPE_E        = BIT_20,

    /** last in the enumeration (NOT TO BE USED) */
    CPSS_PP_FAMILY_BIT_LAST_E

} CPSS_PP_FAMILY_BIT_ENT;



/*-----------------------------------------------*/
/*  Traffic Manager Description                  */
/*-----------------------------------------------*/

/**
* @struct PRV_CPSS_GEN_TM_DB_STC
 *
 * @brief A structure to hold info about the Traffic Manager Unit of a device
*/
typedef struct{

    /** is TM device info initialized per device. */
    GT_BOOL tmDevIsInitilized;

    /** is TM LIB initialized per device. */
    GT_BOOL tmLibIsInitilized;

    /** Traffic Manager Handle. */
    GT_VOID_PTR tmHandle;

    /** Traffic Manager internal supported devices. */
    PRV_CPSS_TM_DEVS_ENT prvCpssTmDevId;

    GT_U8 BapMapping[CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS];

    /** @brief indication that the PFC is enabled on the TM of device.
     *  pfcTmGluePfcDevInfoPtr - allocation of PFC info (type PRV_CPSS_DXCH_TM_GLUE_PFC_DEV_INFO_STC)
     */
    GT_BOOL pfcTmGluePfcIsInitialized;

    void *pfcTmGluePfcDevInfoPtr;

} PRV_CPSS_GEN_TM_DB_STC;


typedef struct
{
    GT_BOOL                 regDataBaseInitialized;
    GT_BOOL                 schedLibIsInitilized[4];
    GT_VOID_PTR             schedHandle[4];
}PRV_CPSS_DXCH_SIP6_PDQ_SCHED_DB_STC;



/* check if the SIP supported by the device
    devNum - the device id of the device
    sipId - the SIP to check. (one of CPSS_GEN_SIP_ENT)

    ---
    macro returns :
    1 - the SIP supported
    0 - the SIP supported
*/
#define PRV_CPSS_SIP_CHECK_MAC(_devNum,sipId) \
  ((PRV_CPSS_PP_MAC(_devNum)->supportedSipArr[(sipId)>>5] &(1<<((sipId)&0x1f)))? \
      1 : 0)

/* set that the device support the SIP
    devNum - the device id of the device
    sipId - the SIP to check. (one of CPSS_GEN_SIP_ENT)
*/
#define PRV_CPSS_SIP_SET_MAC(_devNum,sipId)    \
    PRV_CPSS_PP_MAC(_devNum)->supportedSipArr[(sipId)>>5] |= (1<<((sipId)&0x1f))

/* check SIP_5 */
#define PRV_CPSS_SIP_5_CHECK_MAC(_devNum)                    \
    (PRV_CPSS_SIP_CHECK_MAC(_devNum,CPSS_GEN_SIP_5_E))

/* check SIP_5_10 */
#define PRV_CPSS_SIP_5_10_CHECK_MAC(_devNum)                  \
    (PRV_CPSS_SIP_CHECK_MAC(_devNum,CPSS_GEN_SIP_5_10_E))

/* check SIP_5_15 */
#define PRV_CPSS_SIP_5_15_CHECK_MAC(_devNum)                  \
    (PRV_CPSS_SIP_CHECK_MAC(_devNum,CPSS_GEN_SIP_5_15_E))

/* check SIP_5_16 */
#define PRV_CPSS_SIP_5_16_CHECK_MAC(_devNum)                  \
    (PRV_CPSS_SIP_CHECK_MAC(_devNum,CPSS_GEN_SIP_5_16_E))

/* check SIP_5_20 */
#define PRV_CPSS_SIP_5_20_CHECK_MAC(_devNum)                    \
    (PRV_CPSS_SIP_CHECK_MAC(_devNum,CPSS_GEN_SIP_5_20_E))

/* check SIP_5_25 */
#define PRV_CPSS_SIP_5_25_CHECK_MAC(_devNum)                    \
    (PRV_CPSS_SIP_CHECK_MAC(_devNum,CPSS_GEN_SIP_5_25_E))

/* check SIP_6 */
#define PRV_CPSS_SIP_6_CHECK_MAC(_devNum)                    \
    (PRV_CPSS_SIP_CHECK_MAC(_devNum,CPSS_GEN_SIP_6_E))

/* check that the device is applicable for the specified sip */
/* sipId - is one of CPSS_GEN_SIP_ENT */
#define PRV_CPSS_APPLICABLE_SIP_CHECK_MAC(_devNum , sipId)      \
    if(0 == PRV_CPSS_SIP_CHECK_MAC(_devNum,sipId))              \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG) /* ; in caller */

/* access to the Reset and Init Controller registers addresses of the 'eArch' device */
#define PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(_devNum)  \
    (&(PRV_CPSS_PP_MAC(_devNum)->resetAndInitControllerRegsAddr))

/*  check device DFX server support  */
#define PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(_devNum)  \
    (PRV_CPSS_PP_MAC(_devNum)->hwInfo.dfxServer.supported)

/**
* @struct PRV_CPSS_PP_HW_INFO_STC
 *
 * @brief A Structure including PP's HW major differences between devices.
 * Notes:
 * the initialization of those fields should be done during "phase 1".
*/
typedef struct{
    struct{
       /** @brief DFX server support */
        GT_BOOL supported;
    }dfxServer;

}PRV_CPSS_PP_HW_INFO_STC;

/*
 * Typedef: struct PRV_CPSS_GEN_PP_CONFIG_STC
 *
 * Description: A structure to hold all PP data needed in CPSS
 *
 * Fields:
 *      devNum       - PP's device number represented by this info.
 *      functionsSupportedBmp - bmp of types of supported functions
 *                    each CPSS API function will check if the device can "run"
 *                    it , so this BMP will allow the device to be called for
 *                    ExMx / Tiger / DxChx functions ..
 *      devType      - device type of the PP.
 *      revision     - The device's revision number.
 *      devFamily    - CPSS's device family that current device belongs to.
 *      devSubFamily - CPSS's device sub-family that current device belongs to.
 *      appDevFamily - device family bit to check in unapplicable families bitmap
 *      numOfPorts   - number of ports in this device
 *      numOfVirtPorts - number of virtual ports.
 *                       For Value Blade capable devices the value of
 *                       numOfVirtPorts may be different from numOfPorts.
 *                       For not VB devices numOfVirtPorts = numOfPorts.
 *      existingPorts - bmp of actual exiting ports
 *                      needed for devices that their physical ports not
 *                      continues from first port num to last port num
 *      baseCoreClock - the 'base core'  clock , as documented for the theoretical
 *                      calculations of 'clock' : shaper,Policer,aging...
 *      coreClock - device's core clock frequency and not system (in Mhz)
 *      hwIfSupportedBmp  - bmp of supported Management interface types.
 *                          this BMP allow to support "old" hw if with it's
 *                          newer protocol version, like in the case of "PCI"
 *                          and "PEX"(PCI Express)
 *      intCtrl - Includes control structures needed for managing the
 *                    interrupts received as a result of Rx / Tx / AU ...
 *      hwDevNum      - HW device number.
 *      macAddrQueryInfo - info from the PP about the query that SW triggered
 *      configInitAfterHwResetDone - set to GT_FALSE at "phase 2" and "shutdown
 *                                   Warm Hw Reset"
 *                                   set to GT_TRUE at "pp logical init"
 *                                   Only after HW reset or "Shutdown Warm Hw
 *                                   Reset" need to do full initialization
 *      ppHAState    - PP's High availability mode : Active / Standby
 *      trunkInfo    - DB of the about trunks in that device
 *      smiMasters   - array of pointers to SMI drivers on this PP
 *      xsmiMasters  - array of pointers to XSMI drivers on this PP
 *      phyPortInfoArray - array of port info structures
 *      phyInfo      - PHY related information
 *      useIsr        - indication that the device connected to the ISR of the
 *                      cpss. meaning that the device get ISR services from ISR.
 *                      This also imply that the events of the device managed
 *                      by the cpss of externally.
 *                      GT_TRUE - the cpss manage the ISR and the events of the
 *                                device.
 *                      GT_FALSE - the cpss NOT manage the ISR and NOT the events
 *                                of the device, and the application responsible
 *                                to have it's own ISR and manage the events.
 *                      NOTE: this relate to the
 *                      CPSS_DXCH_PP_PHASE1_INIT_INFO_STC::intVecNum ==
 *                       CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS
 *     portGroupsInfo -   Info about the port groups of the device.
 *                   NOTE: it is relevant also to 'non multi port groups' devices
 *                         for the simplicity of the macros that needed to
 *                         support loops on active port groups !!!
 *                         for example see macro PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC
 *     cpuPortMode - hold current CPU network interface mode sdma/mii/none
 *     netifSdmaPortGroupId - the port group on which CPU network interface is operational.
 *                            relevant to CPSS_NET_CPU_PORT_MODE_SDMA_E mode only.
 *      maxIterationsOfBusyWait - maximum iterations when doing busy wait on
 *                  value/filed in all loop. after that number of loops ,
 *                  the function will return GT_TIMEOUT
 *      maxIterationsOfSmiWait - maximum iterations when waiting for previous SMI transaction to end.
 *                          after that number of loops ,the function will return GT_NOT_READY
 *                          NOTE: actual iterator need to be 'volatile'
 *      hitlessWriteMethodEnable - In some cases under reconfiguration hitless write methood is used.
 *                                 This methood is in verifying memory contents before writing.
 *                                 If memory contents and writing data are the same no write is performed,
 *                                 otherwise write is done.
 *                                 GT_TRUE: hitless write methood is enable.
 *                                 GT_FALSE: hitless write methood is disable.
 *     tmInfo                      Traffic Manager related information
 *     noTraffic2CPU  - whether application requires traffic to CPU
 *                     (inverse logic,
 *                          GT_FALSE means traffic to CPU
 *                          G_TRUE   means no traffic to CPU
 *      duplicatedAddrGetFunc   - call back function (can be NULL) that is used
 *                                by the generic code to call the 'specific' device
 *                                function to get information about duplicated addresses
 *                                for given address.
 *                                function should be called from prvCpssDuplicatedMultiPortGroupsGet
 *      ppCommonPortFuncPtrsStc - callback functions per device for the use of port manager common module
 *      portEgfForceStatusForRestoreBitmap - bitmarp to mark for each port whether egf force status is
 *                                           link down or not - 1 bit per port. This bitmap is currently
 *                                           allocated and used only for sip_5_20
 *      resetAndInitControllerRegsAddr - DFX registers.
 *      diagInfo                - database for diagnostic features
 *      hwInfo                  - info about PP (HW parameters)
 *      exactMatchInfo          - info needed for Exact Match implementation
 */
#include <cpss/common/port/private/prvCpssPortTypes.h>
typedef struct
{
    GT_SW_DEV_NUM                          devNum;
    GT_U32                                 functionsSupportedBmp;
    CPSS_PP_DEVICE_TYPE                    devType;
    GT_U8                                  revision;
    CPSS_PP_FAMILY_TYPE_ENT                devFamily;
    CPSS_PP_SUB_FAMILY_TYPE_ENT            devSubFamily;
    CPSS_PP_FAMILY_BIT_ENT                 appDevFamily;
    GT_U32                                 numOfPorts;
    GT_U32                                 numOfVirtPorts;
    CPSS_PORTS_BMP_STC                     existingPorts;
    GT_U32                                 baseCoreClock;
    GT_U32                                 coreClock;
    GT_U32                                 hwIfSupportedBmp;
    GT_HW_DEV_NUM                          hwDevNum;
    PRV_CPSS_INTERRUPT_CTRL_STC            intCtrl;
    GT_BOOL                                configInitAfterHwResetDone;
    CPSS_SYS_HA_MODE_ENT                   ppHAState;
#ifdef CHX_FAMILY
    PRV_CPSS_TRUNK_DB_INFO_STC             trunkInfo;
#endif /*CHX_FAMILY*/
    CPSS_HW_DRIVER_STC*                    smiMasters[CPSS_MAX_PORT_GROUPS_CNS][CPSS_PHY_SMI_INTERFACE_MAX_E];
    CPSS_HW_DRIVER_STC*                    xsmiMasters[CPSS_MAX_PORT_GROUPS_CNS][CPSS_PHY_XSMI_INTERFACE_MAX_E];
    PRV_CPSS_PORT_INFO_ARRAY_STC          *phyPortInfoArray;
    PRV_CPSS_PHY_INFO_STC                  phyInfo;
    GT_BOOL                                useIsr;
    PRV_CPSS_GEN_PP_PORT_GROUPS_INFO_STC   portGroupsInfo;
    CPSS_NET_CPU_PORT_MODE_ENT             cpuPortMode;
    GT_U32                                 netifSdmaPortGroupId;
    GT_U32                                 maxIterationsOfBusyWait;
    GT_U32                                 maxIterationsOfSmiWait;
    GT_BOOL                                hitlessWriteMethodEnable;
    PRV_CPSS_GEN_TM_DB_STC                 tmInfo;
    GT_BOOL                                noTraffic2CPU;
    GT_U32                                 supportedSipArr[CPSS_GEN_SIP_BMP_MAX_NUM_CNS];
    PRV_CPSS_GEN_PP_MULTI_PIPES_INFO_STC   multiPipe;
    PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_GET_FUNC   duplicatedAddrGetFunc;
    PRV_CPSS_PORT_FUNC_PTRS_STC            ppCommonPortFuncPtrsStc;
    CPSS_PORTS_BMP_STC                     *portEgfForceStatusBitmapPtr;
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC resetAndInitControllerRegsAddr;
    PRV_CPSS_DIAG_STC                      diagInfo;
    PRV_CPSS_PP_HW_INFO_STC                hwInfo;

} PRV_CPSS_GEN_PP_CONFIG_STC;

/* array of pointers to the valid devices */
extern void* prvCpssPpConfig[PRV_CPSS_MAX_PP_DEVICES_CNS];

/**
* @struct PRV_CPSS_PP_HA_CONFIG_INFO_STC
 *
 * @brief structure for export/import data fro device config during HA catch up process
*/
typedef struct{

    /** device type of PP. */
    CPSS_PP_DEVICE_TYPE deviceId;

    /** revision of PP. */
    GT_U8 revision;

} PRV_CPSS_PP_HA_CONFIG_INFO_STC;

/**
* @struct RE_INITIALZATION_DEV_INFO_STC
 *
 * @brief This structure holds per device the DMA pointers allocated for it,
 * for the 'reuse' after re-initialization due to 'shutdown and restart'
*/
typedef struct{

    /** bus base address is used as 'key' for attaching a device number */
    GT_UINTPTR busBaseAddr;

    /** if device HW reset occured */
    GT_BOOL deviceDidHwReset;

    /** @brief indicate if network initialization is needed or
     *  configuration based on previously saved parameters
     */
    GT_BOOL deviceNeedNetIfInit;

    /** shows whether dma info valid */
    GT_BOOL dmaInfoValid;

    /** dma info */
    PRV_CPSS_DMA_RE_INIT_INFO_STC dmaInfo;

} RE_INITIALZATION_DEV_INFO_STC;

/**
* @struct PRV_CPSS_GEN_GLOBAL_DATA_STC
 *
 * @brief This structure contains all Cpss Generic global data
*/

typedef struct
{
    PRV_CPSS_PP_HA_CONFIG_INFO_STC *cpssPpHaDevData[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_BOOL  prvCpssAfterDbRelease;
    RE_INITIALZATION_DEV_INFO_STC reInitializationDbPtr[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32 prvMiiDevNum;
#ifdef CHX_FAMILY
    PRV_CPSS_NETIF_MII_TX_CTRL_STC   prvCpssGenNetIfMiiTxCtrl;
    PRV_CPSS_NETIF_MII_RX_Q_CTRL_STC  prvCpssGenNetIfMiiRxQCtrl;
    PRV_CPSS_GEN_NETIF_ETH_PORT_CTRL_VALID_STC prvCpssGenNetIfMiiCtrl;
#endif /*CHX_FAMILY*/
    GT_BOOL                 cpssPpInitCalled;
    GT_BOOL                 supportDualDeviceId;

} PRV_CPSS_GEN_GLOBAL_DATA_STC;

extern PRV_CPSS_GEN_GLOBAL_DATA_STC sysGenGlobalInfo;
/**
* @internal prvCpssSysConfigPhase1 function
* @endinternal
*
* @brief   This function sets cpss system level system configuration parameters
*         before any of the device's phase1 initialization .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
*/
GT_STATUS prvCpssSysConfigPhase1
(
    GT_VOID
);

/**
* @internal prvCpssSysConfigDestroy function
* @endinternal
*
* @brief   This function destroy cpss system level system configuration parameters
*         after all device removed .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
*/
GT_STATUS prvCpssSysConfigDestroy
(
    GT_VOID
);


/**
* @internal prvCpssHwIfSet function
* @endinternal
*
* @brief   This function sets cpss HW interface parameters.
*         called during "phase1" initialization .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mngInterfaceType         - Management interface type
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on bad device number or interface type
*/
GT_STATUS prvCpssHwIfSet(
    IN GT_U8                            devNum,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT    mngInterfaceType
);

/**
* @internal prvCpssNonMultiPortGroupsDeviceSet function
* @endinternal
*
* @brief   This function sets cpss DB to be 'non-multi-port-groups' device.
*         It is called during "phase1" initialization.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on bad device number
*/
GT_STATUS prvCpssNonMultiPortGroupsDeviceSet(
    IN GT_U8                            devNum
);


/**
* @internal prvCpssPpConfigBitmapFirstActiveBitGet function
* @endinternal
*
* @brief   Get the first active bit from the given bitmap.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] bitmap                   - sorce bitmap.
*
* @param[out] firstBitPtr              - pointer to the index of the found bit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPpConfigBitmapFirstActiveBitGet
(
    IN  GT_U32     bitmap,
    OUT GT_U32     *firstBitPtr
);

/**
* @internal prvCpssPpConfigPortGroupFirstActiveGet function
* @endinternal
*
* @brief   Get the first active port group from the given bitmap.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number .
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] portGroupIdPtr           - pointer to port Group Id.
*                                      for non multi-port groups device
*                                      gets the CPSS_PORT_GROUP_UNAWARE_MODE_CNS value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPpConfigPortGroupFirstActiveGet
(
    IN  GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP     portGroupsBmp,
    OUT GT_U32                 *portGroupIdPtr
);

/**
* @internal prvCpssPpConfigBitmapNumBitsGet function
* @endinternal
*
* @brief   Get the number of bits that set set to 1 in the given bitmap.
*
* @param[in] bitmap                   - source bitmap.
*                                       numbert of bits that are set to 1 in bitmap
*/
GT_U32 prvCpssPpConfigBitmapNumBitsGet
(
    IN  GT_U32     bitmap
);

/**
* @internal prvCpssPpConfigNumActivePortGroupsInBmpGet function
* @endinternal
*
* @brief   Get the number of active port groups in the given bitmap.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number .
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] numActivePortGroupsPtr   - (pointer to)number of active port groups in the BMP.
*                                      for non multi-port groups device : value 1 is returned.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPpConfigNumActivePortGroupsInBmpGet
(
    IN  GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP     portGroupsBmp,
    OUT GT_U32                 *numActivePortGroupsPtr
);

#if defined  CPSS_USE_MUTEX_PROFILER
extern void * prvCpssGetPrvCpssPpConfigInGenFormat(IN GT_U32       devNum,IN const char *               functionName);
#endif


/**
* @internal prvCpssGenXsmiInterfaceResourceTypeGet function
* @endinternal
*
* @brief   Return resource type (address space) appropriate to specified
*         XSMI interface number.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] xsmiInterface            - XSMI master interface number
*
* @param[out] resTypePtr               - (pointer to) resource type
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on unknown xsmiInterface number
*/
GT_STATUS prvCpssGenXsmiInterfaceResourceTypeGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT   xsmiInterface,
    OUT CPSS_DRV_HW_RESOURCE_TYPE_ENT *resTypePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssConfigTypes_h */

