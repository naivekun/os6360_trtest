/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtDbDxBobcat2Mappings.h
*
* @brief Initialization Mapping and Port for the Bobcat2 - SIP5 - board.
*
* @version   1
********************************************************************************
*/
#ifndef __GT_DB_DX_BOBCAT2_MAPPING_H
#define __GT_DB_DX_BOBCAT2_MAPPING_H

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>


/*******************************************************************************
* configBoardAfterPhase1 : relevant data structures for port mapping
*
*       This function performs all needed configurations that should be done
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


 

typedef struct
{
    GT_U8                   boardRevId;
    CPSS_DXCH_PORT_MAP_STC *mappingPtr;
    GT_U32                  mappingSize;
}CPSS_DXCH_REVID_2_PORT_MAP_STC;



/**
* @internal configBoardAfterPhase1MappingGet function
* @endinternal
*
* @brief   This function gets relevant mapping for boardRevId,
*         if not found , selects default
* @param[in] boardRevId               - The board revision Id.
*                                      revId2PortMapList - mapping list
*                                      listSize          - mapping list size
*
* @param[out] mappingPtrPtr            - pointer to found mapping
* @param[out] mappingPtrPtr            - pointer to mapping size
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on bad ptr
* @retval GT_BAD_PARAM             - on empty list (list size = 0)
*/
GT_STATUS configBoardAfterPhase1MappingGet
(
    IN  GT_U8                             boardRevId,
    OUT CPSS_DXCH_PORT_MAP_STC          **mappingPtrPtr,
    OUT GT_U32                           *mappingSizePtr
);

/**
* @internal configBoardAfterPhase1MappingGetCetus function
* @endinternal
*
* @brief   This function gets relevant mapping for boardRevId,
*         if not found , selects default
* @param[in] boardRevId               - The board revision Id.
*                                      revId2PortMapList - mapping list
*                                      listSize          - mapping list size
*
* @param[out] mappingPtrPtr            - pointer to found mapping
* @param[out] mappingPtrPtr            - pointer to mapping size
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on bad ptr
* @retval GT_BAD_PARAM             - on empty list (list size = 0)
*
* @note Special version of configBoardAfterPhase1MappingGet()
*       to return correct mapping for Cetus during fastboot test
*
*/
GT_STATUS configBoardAfterPhase1MappingGetCetus
(
    IN  GT_U8                             boardRevId,
    OUT CPSS_DXCH_PORT_MAP_STC          **mappingPtrPtr,
    OUT GT_U32                           *mappingSizePtr
);

/**
* @internal configBoardAfterPhase1MappingGetAldrin function
* @endinternal
*
* @brief   This function gets relevant mapping for boardRevId,
*         if not found , selects default
* @param[in] boardRevId               - The board revision Id.
*                                      revId2PortMapList - mapping list
*                                      listSize          - mapping list size
*
* @param[out] mappingPtrPtr            - pointer to found mapping
* @param[out] mappingPtrPtr            - pointer to mapping size
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on bad ptr
* @retval GT_BAD_PARAM             - on empty list (list size = 0)
*
* @note Special version of configBoardAfterPhase1MappingGet()
*       to return correct mapping for Aldrin during fastboot test
*
*/
GT_STATUS configBoardAfterPhase1MappingGetAldrin
(
    IN  GT_U8                             boardRevId,
    OUT CPSS_DXCH_PORT_MAP_STC          **mappingPtrPtr,
    OUT GT_U32                           *mappingSizePtr
);

/**
* @internal configBoardAfterPhase1MappingGetBobcat3 function
* @endinternal
*
* @brief   This function gets relevant mapping for boardRevId,
*         if not found , selects default
* @param[in] boardRevId               - The board revision Id.
*                                      revId2PortMapList - mapping list
*                                      listSize          - mapping list size
*
* @param[out] mappingPtrPtr            - pointer to found mapping
* @param[out] mappingPtrPtr            - pointer to mapping size
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on bad ptr
* @retval GT_BAD_PARAM             - on empty list (list size = 0)
*
* @note Special version of configBoardAfterPhase1MappingGet()
*       to return correct mapping for Bobcat3 during fastboot test
*
*/
GT_STATUS configBoardAfterPhase1MappingGetBobcat3
(
    IN  GT_U8                             boardRevId,
    OUT CPSS_DXCH_PORT_MAP_STC          **mappingPtrPtr,
    OUT GT_U32                           *mappingSizePtr
);

/**
* @internal configBoardAfterPhase1MappingGetAldrin2 function
* @endinternal
*
* @brief   This function gets relevant mapping for boardRevId,
*         if not found , selects default
* @param[in] boardRevId               - The board revision Id.
*                                      revId2PortMapList - mapping list
*                                      listSize          - mapping list size
*
* @param[out] mappingPtrPtr            - pointer to found mapping
* @param[out] mappingPtrPtr            - pointer to mapping size
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on bad ptr
* @retval GT_BAD_PARAM             - on empty list (list size = 0)
*
* @note Special version of configBoardAfterPhase1MappingGet()
*       to return correct mapping for Aldrin2 during fastboot test
*
*/
GT_STATUS configBoardAfterPhase1MappingGetAldrin2
(
    IN  GT_U8                             boardRevId,
    OUT CPSS_DXCH_PORT_MAP_STC          **mappingPtrPtr,
    OUT GT_U32                           *mappingSizePtr
);

/*******************************************************************************
* bcat2PortInterfaceInit
*
*******************************************************************************/
#define APP_INV_PORT_CNS ((GT_U32)(~0))

typedef enum
{
     PORT_LIST_TYPE_EMPTY = 0
    ,PORT_LIST_TYPE_INTERVAL
    ,PORT_LIST_TYPE_LIST
}PortListType_ENT;

typedef struct
{
    PortListType_ENT             entryType;
    GT_PHYSICAL_PORT_NUM         portList[20];  /* depending on list type */
                                                /* interval : 0 startPort     */
                                                /*            1 stopPort      */
                                                /*            2 step          */
                                                /*            3 APP_INV_PORT_CNS */
                                                /* list     : 0...x  ports     */
                                                /*            APP_INV_PORT_CNS */
    CPSS_PORT_SPEED_ENT          speed;
    CPSS_PORT_INTERFACE_MODE_ENT interfaceMode;
}PortInitList_STC;

typedef struct
{
    GT_PHYSICAL_PORT_NUM         portNum;
    CPSS_PORT_SPEED_ENT          speed;
    CPSS_PORT_INTERFACE_MODE_ENT interfaceMode;
}PortInitInternal_STC;

typedef struct
{
    GT_U32                       coreClock;
    GT_U8                        boardRevId;
    CPSS_PP_SUB_FAMILY_TYPE_ENT  subFamily;
    GT_U32                       enabledPipesBmp;
    PortInitList_STC            *portsInitListPtr;
}boardRevId2PortsInitList_STC;

typedef struct
{
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32               laneBmp;
    GT_BOOL              invertTx;
    GT_BOOL              invertRx;
}APPDEMO_BC2_SERDES_POLARITY_STC;

/**
* @internal bcat2PortInterfaceInitPortInitListGet function
* @endinternal
*
* @brief   Execute predefined ports configuration.
*
* @param[in] devNum                   - devNum
* @param[in] boardRevId               - board revision ID
* @param[in] coreClock                - core clock
*
* @param[out] portInitListPtrPtr       =  (pointer to) port init list
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS bcat2PortInterfaceInitPortInitListGet
(
    IN  GT_U8                             devNum,
    IN  GT_U8                             boardRevId,
    IN  GT_U32                            boardType,
    IN  GT_U32                            coreClock,
    OUT PortInitList_STC                **portInitListPtrPtr
);

/**
* @internal appDemoDxChMaxMappedPortSet function
* @endinternal
*
* @brief   Calculate and set maximal mapped port number,
*         keep port mapping in appDemoPpConfigList DB.
* @param[in] dev                      - device number
* @param[in] mapArrLen                - number of ports to map, array size
* @param[in] mapArrPtr                - pointer to array of mappings
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong mapAttLen
*/
GT_STATUS appDemoDxChMaxMappedPortSet
(
    IN  GT_U8  dev,
    IN  GT_U32 mapArrLen,
    IN  CPSS_DXCH_PORT_MAP_STC *mapArrPtr
);

/**
* @internal appDemoDxChMaxMappedPortGet function
* @endinternal
*
* @brief   Get port mapping from appDemoPpConfigList DB.
*
* @param[in] dev                      - device number
*
* @param[out] mapArrLenPtr             - number of ports
* @param[out] mapArrPtr                - ports mapping
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong mapAttLen
*/
GT_STATUS appDemoDxChMaxMappedPortGet
(
    IN  GT_U8                   dev,
    OUT  GT_U32                 *mapArrLenPtr,
    OUT  CPSS_DXCH_PORT_MAP_STC **mapArrPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif


