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
* @file prvCpssDxChTxq.h
*
* @brief CPSS SIP6 TXQ definitions
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTxq
#define __prvCpssDxChTxq

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/generic/tm/cpssTmPublicDefs.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPdq.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
#include <cpss/generic/port/cpssPortTx.h>



#define PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR           cpssDeviceRunCheck_onEmulator

#define PRV_CPSS_DXCH_SIP6_TXQ_ALL_Q_MAC             0xFF

/* the number of DPs in tile */
#define MAX_DP_IN_TILE(devNum)  \
    (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp /         \
     PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)

#define PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC 0

/*SIP 6 maximal PDS per tile number*/
#define CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC      MAX_DP_PER_TILE_CNS

/*SIP 6 maximal SDQ per tile number*/
#define CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC      MAX_DP_PER_TILE_CNS

/*SIP 6 maximal QFC per tile number*/
#define CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC      MAX_DP_PER_TILE_CNS


/*SIP 6 maximal global PDQ  number*/
#define CPSS_DXCH_SIP_6_MAX_PDQ_NUM                4

/*SIP 6 maximal global PDX  number*/
#define CPSS_DXCH_SIP_6_MAX_PDX_NUM_MAC            4

/*SIP 6 maximal local port  number*/
#define CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC     9

/*SIP 6 maximal global PDS  number*/
#define CPSS_DXCH_SIP_6_MAX_GLOBAL_PDS_NUM_MAC     32

/*SIP 6 maximal global SDQ  number*/
#define CPSS_DXCH_SIP_6_MAX_GLOBAL_SDQ_NUM_MAC     32


/*SIP 6 maximal  number of sceduler A nodes*/
#define CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC                      512
/*SIP 6 maximal  number of sceduler port nodes*/
#define CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM                         72
/*SIP 6 maximal  number of profiles at PDS*/
#define CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM                      16

/*SIP 6 maximal  number of profiles at scheduler*/
#define CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM                      16

/*SIP 6 maximal  number PDQ queues*/
#define CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM                        _2K
/*SIP 6 maximal  number PDS queues*/
#define CPSS_DXCH_SIP_6_MAX_PDS_QUEUE_NUM_MAC                    400
/*SIP 6 maximal  number of queue group*/
#define  CPSS_DXCH_SIP_6_MAX_Q_GROUP_SIZE_MAC                    _1K

/*SIP 6  max TC*/
#define CPSS_DXCH_SIP_6_MAX_TC_QUEUE_NUM_MAC                    16

#define PRV_TC_MAP_PB_MAC    0x4


/*macro define maximal number of configured slices*/
#define TXQ_PDX_MAX_SLICE_NUMBER_MAC 33

/*macro define minimal number of configured slices*/
#define TXQ_PDX_MIN_SLICE_NUMBER_MAC 32
/*SIP 6 macro for getting field offset by unit/sub-unit/register/filed name*/
#define GET_REGISTER_FIELD_OFFSET_MAC(_UNIT,_SUB_UNIT,_REG,_FIELD)   _UNIT##_##_SUB_UNIT##_##_REG##_##_FIELD##_FIELD_OFFSET
/*SIP 6 macro for getting field size by unit/sub-unit/register/filed name*/
#define GET_REGISTER_FIELD_SIZE_MAC(_UNIT,_SUB_UNIT,_REG,_FIELD)   _UNIT##_##_SUB_UNIT##_##_REG##_##_FIELD##_FIELD_SIZE


#define PDS_PROFILE_MAX_MAC 16

/*SIP 6 macro for checking validity of local PDS  number*/
#define TXQ_SIP_6_CHECK_TILE_NUM_MAC(_tileNum)\
    do\
      {\
        GT_U32 numOfTilesInMacro;\
        switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)\
        {\
            case 0:\
            case 1:\
                numOfTilesInMacro = 1;\
                break;\
           case 2:\
           case 4:\
                numOfTilesInMacro =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;\
                break;\
            default:\
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);\
                break;\
        }\
        if(_tileNum>=numOfTilesInMacro)\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tile number [%d] must be less than [%d]",_tileNum,numOfTilesInMacro);\
        }\
      }while(0)


/*SIP 6 macro for checking validity of local PDS  number*/
#define TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(_pds)\
    do\
      {\
        if(_pds>=CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC)\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local pds[%d] must be less than [%d]",_pds,CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC);\
        }\
      }while(0)
/*SIP 6 macro for checking validity of global PDS  number*/
#define TXQ_SIP_6_CHECK_GLOBAL_PDS_NUM_MAC(_pds)\
            do\
              {\
                if(_pds>=CPSS_DXCH_SIP_6_MAX_GLOBAL_PDS_NUM_MAC)\
                {\
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "global pds[%d] must be less than [%d]",_pds,CPSS_DXCH_SIP_6_MAX_GLOBAL_PDS_NUM_MAC);\
                }\
              }while(0)
/*SIP 6 macro for checking validity of PDX*/
#define TXQ_SIP_6_CHECK_PDX_NUM_MAC(_pdx)\
            do\
              {\
                if(_pdx>=CPSS_DXCH_SIP_6_MAX_PDX_NUM_MAC)\
                {\
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "pdx[%d] must be less than [%d]",_pdx,CPSS_DXCH_SIP_6_MAX_PDX_NUM_MAC);\
                }\
              }while(0)

                /*SIP 6 macro for checking validity of global port*/
 #define TXQ_SIP_6_CHECK_GLOBAL_PORT_NUM_MAC(_portNum)\
    do\
      {\
        if(_portNum>= CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM)\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " portNum[%d] must be less than [%d]",_portNum,CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM);\
        }\
      }while(0)


/*SIP 6 macro for checking validity of local port*/
 #define TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(_portNum)\
    do\
      {\
        if(_portNum>= CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC)\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local portNum[%d] must be less than [%d]",_portNum,CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC);\
        }\
      }while(0)
/*SIP 6 macro for checking validity of queue group*/
#define TXQ_SIP_6_CHECK_QUEUE_GROUP_NUM_MAC(_queueGroup)\
           do\
             {\
               if(_queueGroup>= CPSS_DXCH_SIP_6_MAX_Q_GROUP_SIZE_MAC)\
               {\
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "queueGroup[%d] must be less than [%d]",_queueGroup,CPSS_DXCH_SIP_6_MAX_Q_GROUP_SIZE_MAC);\
               }\
             }while(0)

/*SIP 6 macro for checking validity of global sdq num*/
#define TXQ_SIP_6_CHECK_GLOBAL_SDQ_NUM_MAC(_sdq)\
           do\
             {\
               if(_sdq>=CPSS_DXCH_SIP_6_MAX_GLOBAL_SDQ_NUM_MAC)\
               {\
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "global sdq[%d] must be less than [%d]",_sdq,CPSS_DXCH_SIP_6_MAX_GLOBAL_SDQ_NUM_MAC);\
               }\
             }while(0)
/*SIP 6 macro for checking validity of  pdq num*/
#define TXQ_SIP_6_CHECK_PDQ_NUM_MAC(_pdq)\
           do\
             {\
               if(_pdq>=CPSS_DXCH_SIP_6_MAX_PDQ_NUM)\
               {\
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "pdq[%d] must be less than [%d]",_pdq,CPSS_DXCH_SIP_6_MAX_PDQ_NUM);\
               }\
             }while(0)

/*SIP 6 macro for checking validity of local sdq num*/
#define TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(_sdq)\
       do\
         {\
           if(_sdq>=CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC)\
           {\
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local sdq[%d] must be less than [%d]",_sdq,CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC);\
           }\
         }while(0)

           /*SIP 6 macro for checking validity of local qfc num*/
#define TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(_qfc)\
          do\
            {\
              if(_qfc>=CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC)\
              {\
                  CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local qfc[%d] must be less than [%d]",_qfc,CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC);\
              }\
            }while(0)

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_DMA_TO_PNODE_MAP_STC
*
* @brief Describe Pnode
*/
  typedef struct
  {
       /** index of Pnode[0..71]*/
      GT_U32              pNodeInd;

       /** index of tile[0..3]*/
      GT_U32              tileInd;

  } PRV_CPSS_DXCH_TXQ_SIP_6_DMA_TO_PNODE_MAP_STC;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PHYSICAL_PORT_TO_ANODE_MAP_STC
 *
 * @brief  Describe Pnode
*/
  typedef struct
  {
      /** index of aNodeInd[0..511] */
      GT_U32              aNodeInd;
	  /** index of tile[0..3] */
      GT_U32              tileInd;

  } PRV_CPSS_DXCH_TXQ_SIP_6_PHYSICAL_PORT_TO_ANODE_MAP_STC;



/************start of mapping configurations***************/






/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PORT_BW_DATA
 *
 * @brief Port speed shadow.
*/
typedef struct{

      /** port speed */

    GT_U32 speed;

} PRV_CPSS_DXCH_TXQ_SIP_6_PORT_BW_DATA;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE
 *
 * @brief Describe list of A node connected to local port
*/
typedef struct{

    /** @brief size of A node list [0..CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC].
     *  0 means that local port is disabled
     */
    GT_U32 aNodeListSize;

    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE aNodelist[CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC];

    /** the number of the first queue in this range */
    GT_U32 sdqQueueFirst;

    /** the number of the last queue in this range */
    GT_U32 sdqQueueLast;

     /** if equal GT_TRUE then then Pnode represent cascade port,GT_FALSE otherwise */
    GT_BOOL isCascade;

      /** if isCascade equal GT_TRUE then this  parameter represent cascade port physical number */
    GT_U32 cascadePhysicalPort;
} PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE;



/**
* @struct PRV_CPSS_DXCH_TXQ_SIP6_SEARCH_STC
 *
 * @brief Data base for quick search in txQ mapping data base
*/
typedef struct{

    GT_U32 aNodeIndexToPnodeIndex[CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC];

    GT_U32 qNodeIndexToAnodeIndex[CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM];



} PRV_CPSS_DXCH_TXQ_SIP6_SEARCH_STC;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PDX_PIZZA_CONFIG_STC
 *
 * @brief Describe PDX pizza mapping
*/
typedef struct{

     /** number of slices per DP (dp without CPU port get less slices*/
    GT_U32 pdxPizzaNumberOfSlices[CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC];

} PRV_CPSS_DXCH_TXQ_SIP_6_PDX_PIZZA_CONFIG_STC;



/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION
 *
 * @brief Describe mapping of P node to A node and physical port
*/
typedef struct{

    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE pNodeMappingConfiguration[CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM];

    /** size of pNodeMappingConfiguration array */
    GT_U32 size;

    /** Data base for quick search in txQ mapping data base */
    PRV_CPSS_DXCH_TXQ_SIP6_SEARCH_STC searchTable;

    /** Describe PDX pizza mapping */
    PRV_CPSS_DXCH_TXQ_SIP_6_PDX_PIZZA_CONFIG_STC pdxPizza;

    GT_U32                                       firstQInDp[MAX_DP_PER_TILE_CNS];

    GT_U32                                       mappingMode;

} PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION;



/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_GENERAL_CONFIGURATION
 *
 * @brief General configuration of tile
*/
typedef struct{

    GT_U32 pdxMapping[4];

} PRV_CPSS_DXCH_TXQ_SIP_6_GENERAL_CONFIGURATION;


/************end of mapping configurations***************/

/************start of BW configurations***************/




/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PDS_PROFILE_LENGTH_ADJUST
 *
 * @brief Describe SDQ profile length adjust on deque
*/

typedef struct
{
    GT_BOOL enable;
    GT_BOOL substruct;
    GT_U32  byteCount;
} PRV_CPSS_DXCH_TXQ_SIP_6_PDS_PROFILE_LENGTH_ADJUST;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PDS_PROFILE
 *
 * @brief Describe SDQ profile
*/
typedef struct{

    /** Aging threshold in clock cycles [0 */
    GT_BOOL longQueueEnable;

    /** Credit High Threshold[0 */
    GT_U32 headEmptyLimit;

    /** Credit Low Threshold[0 */
    GT_U32 longQueueLimit;

    /** Minimum credit budget for selection.[0 */
    PRV_CPSS_DXCH_TXQ_SIP_6_PDS_PROFILE_LENGTH_ADJUST lengthAdjust;

} PRV_CPSS_DXCH_TXQ_SIP_6_PDS_PROFILE;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_NODE_SHAPING_PROFILE_STC
 *
 * @brief Txq profiles
*/
typedef struct{

    /** GT_TRUE if shaping profile is valid ,GT_FALSE otherwise */
    GT_BOOL valid;

    /** Shaping profile */
    CPSS_PDQ_SCHED_SHAPING_PROFILE_PARAMS_STC profile;

} PRV_CPSS_DXCH_TXQ_SIP_6_NODE_SHAPING_PROFILE_STC;





/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PROFILES_DATA
 *
 * @brief Txq profiles
*/
typedef struct{

    /** Number of pds profiles [0 */
    GT_U32 pdsProfilesNum;

    PRV_CPSS_DXCH_TXQ_SIP_6_PDS_PROFILE pdsProfiles[CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM];

} PRV_CPSS_DXCH_TXQ_SIP_6_PROFILES_DATA;






/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_QUEUE_DATA
 *
 * @brief Queue description
*/
typedef struct{

    /** traffic class [0 */
    GT_U32 tc;

    /** index of sdq profiles [0 */
    GT_U32 sdqProfileIndex;

    /** index of shaping profiles[0 */
    GT_U32 shapingProfilePtr;

    /** Aging threshold in clock cycles [0 */
    GT_U32 agingTh;

    /** @brief Credit High Threshold[0
     *  highCreditTh           - Credit Low Threshold[0 - 524287]
     */
    GT_U32 highCreditTh;

    GT_U32 lowCreditTh;

    /** Minimum credit budget for selection.[0 */
    GT_U32 negativeCreditTh;

} PRV_CPSS_DXCH_TXQ_SIP_6_QUEUE_DATA;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE_BW_DATA
 *
 * @brief A-Node Parameters Data Structure.
*/
typedef struct{

    /** @brief Index of Shaping profile
     *  (APPLICABLE RANGES: 1..CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC,
     *  CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS)
     *  number of tree levels nodes can be retrived with cpssTmTreeParamsGet.
     */
    GT_U32 shapingProfilePtr;

    /** @brief DWRR Quantum in resolution of CPSS_TM_NODE_QUANTUM_UNIT_CNS (256) bytes
     *  (APPLICABLE RANGES: 0x40..0x3FFF).
     *  Quantum Limits depend on system MTU and can be retrieved with
     *  cpssTmNodeQuantumLimitsGet API.
     *  schdModeArr[8]      - RR/DWRR Priority for A-Node Scheduling
     *  (APPLICABLE RANGES: (CPSS_TM_SCHD_MODE_ENT) 0..1).
     */
    GT_U32 quantum;

    CPSS_PDQ_SCHD_MODE_ENT schdModeArr[8];

} PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE_BW_DATA;





/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_BANDWITH_CONFIGURATION
 *
 * @brief Bandwith related configuration
*/
typedef struct{

    /** sdq , pds and A node shaping profiles */
    PRV_CPSS_DXCH_TXQ_SIP_6_PROFILES_DATA profiles;


} PRV_CPSS_DXCH_TXQ_SIP_6_BANDWITH_CONFIGURATION;


/************************************end of BW configurations**************************/
/************************************start  of global configurations**************************/

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION
 *
 * @brief SIP 6 mapping and bandwith related txq configuration
*/
typedef struct{

    /** General configuration of tile */
    PRV_CPSS_DXCH_TXQ_SIP_6_GENERAL_CONFIGURATION general;

    /** Bandwith related configuration */
    PRV_CPSS_DXCH_TXQ_SIP_6_BANDWITH_CONFIGURATION bandwith;

    /** Describe mapping of P node to A node and physical port */
    PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION mapping;

} PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION;






/**
* @internal prvCpssFalconTxqUtilsConvertToDb function
* @endinternal
*
* @brief   Convert to txq database format from CPSS_DXCH_PORT_MAP_STC .
*         Create mapping between P nodes /A nodes and physical port numbers
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] sourcePtr                - Pointer to CPSS_DXCH_PORT_MAP_STC to be converted into scheduler tree
* @param[in] size                     - of source array.Number of ports to map, array size
* @param[in] tileNum                  -  The number of tile
*
* @param[out] targetPtr                - Pointer to schedualler mappping configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS prvCpssFalconTxqUtilsConvertToDb
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_PORT_MAP_STC * sourcePtr,
    IN GT_U32 size,
    IN GT_U32 tileNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION  *  targetPtr
);

/**
* @internal prvCpssFalconTxqUtilsInitPdxQroupMapTable function
* @endinternal
*
* @brief   Initialize PDX DX_QGRPMAP table (write to HW) accoring to SW txq shadow
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] mappingPtr               - pointer to SW mapping shadow
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsInitPdxQroupMapTable
(
    IN GT_U8 devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION  * mappingPtr

);
/**
* @internal prvCpssFalconTxqUtilsInitSdq function
* @endinternal
*
* @brief   Initialize SDQ (write to HW) accoring to SW txq shadow
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
* @param[in] totalTiles                  -Number of tiles in the device (Applicable range 1..4)
* @param[in] mappingPtr               - pointer to SW mapping shadow
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsInitSdq
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 totalTiles,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION  * mappingPtr

);
/**
* @internal prvCpssFalconTxqUtilsInitPsi function
* @endinternal
*
* @brief   Initialize PSI (write to HW) accoring to SW txq shadow
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS prvCpssFalconTxqUtilsInitPsi
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
);
/**
* @internal prvCpssFalconTxqUtilsBuildPdqTree function
* @endinternal
*
* @brief   Build default schedualing tree
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNumber               -Then number of tile (Applicable range 0..3)
* @param[in] configPtr                - pointer to SW mapping shadow
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsBuildPdqTree
(
    IN GT_U8 devNum,
    IN  GT_U32 tileNumber,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION  * configPtr
);

/**
* @internal prvCpssFalconTxqUtilsInitDb function
* @endinternal
*
* @brief   Initialize txq database
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] tileConfigsPtr           - pointer to SW  shadow
* @param[in] tileNumber               - the number of the tile(APPLICABLE RANGES:0..3)
*
*/

GT_VOID prvCpssFalconTxqUtilsInitDb
(
    IN PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr,
    IN GT_U32                              tileNumber,
    IN GT_U32                              numberOfTiles
);

/**
* @internal prvCpssFalconTxqUtilsInitDmaToPnodeDb function
* @endinternal
 *
* @brief   Initialize DmaToPnode database
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
*/

GT_VOID  prvCpssFalconTxqUtilsInitDmaToPnodeDb
(
    IN GT_U8 devNum
);


/**
* @internal prvCpssFalconTxqUtilsInitSchedProfilesDb function
* @endinternal
 *
* @brief   Initialize schedProfiles database . All the queues are in WRR gropu 0. TC 0-3 weight 1 , TC 0-3 weight 5
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
*/

GT_VOID  prvCpssFalconTxqUtilsInitSchedProfilesDb
(
    IN GT_U8 devNum
);


/**
* @internal prvCpssFalconTxqUtilsPsiConfigDump function
* @endinternal
*
* @brief   Debug function that dump PDQ queue to SDP/PDS mapping
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] psiNum                   - tile number (APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsPsiConfigDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum
);
/**
* @internal prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump function
* @endinternal
*
* @brief   Debug function that dump physical port to P node and A node mapping.
*         Also PDQ queue numbers that are mapped to A node are dumped
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
*                                      tileNum      - tile number.(APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump
(
    IN  GT_U8 devNum,
    IN  GT_U32 tileNumber,
    IN  GT_U32 firstPort,
    IN  GT_U32 lastPort
);

/**
* @internal prvCpssFalconTxqUtilsPdxTablesDump function
* @endinternal
*
* @brief   Debug function that dump PDX routing table and PDX pizza arbiter
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
*                                      psiNum       - tile number (APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsPdxTablesDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 pdxNum
);
/**
* @internal prvCpssFalconTxqUtilsSdqPortRangeDump function
* @endinternal
*
* @brief   Debug function that dump mapping of local port to queues at SDQ
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     PP's device number.
* @param[in] tileNumber                tile number (APPLICABLE RANGES:0..3)
* @param[in] sdqNum                   -global SDQ number (APPLICABLE RANGES:0..31)
* @param[in] portSpecific               if equal GT_TRUE then print range for specific local port,else print for all ports.
* @param[in] port                   -       local port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsSdqPortRangeDump
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileNumber,
    IN  GT_U32  sdqNum,
    IN  GT_BOOL portSpecific,
    IN  GT_U32  port
);

/**
* @internal prvCpssFalconTxqUtilsSdqQueueAttributesDump function
* @endinternal
*
* @brief   Debug function that dump queue attributes
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    PP's device number.
* @param[in]  tileNum                    tile number.(APPLICABLE RANGES:0..3)
* @param[in] sdqNum                    global SDQ number (APPLICABLE RANGES:0..31)
* @param[in] startQ                     -queue to start from in the dump
* @param[in] size                     -     number of queues to dump
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsSdqQueueAttributesDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNumber,
    IN  GT_U32 sdqNum,
    IN  GT_U32 startQ,
    IN  GT_U32 size
);


/**
* @internal prvCpssDxChTxQFalconPizzaArbiterInitPerTile function
* @endinternal
*
* @brief   Initialize the number of silices ta each PDX PDS pizza arbiter,also initialize all the slices to unused
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    -PP's device number.
* @param[in] pdxNum                    -tile number
* @param[in] pdxPizzaNumberOfSlicesArr -number of slices per dp
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChTxQFalconPizzaArbiterInitPerTile
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 * pdxPizzaNumberOfSlicesArr
);

/**
* @internal prvCpssFalconTxqUtilsInitPdxInterfaceTable function
* @endinternal
*
* @brief   Initialize PDX interface table
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                  - tile number.(APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsInitPdxInterfaceTable
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
);

/**
* @internal prvCpssFalconTxqUtilsSetShapingParameters function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and write shaping parameters to specific node
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[in,out] maxRatePtr               - (pointer to)Requested Rate in Kbps
* @param[in,out] maxRatePtr               -(pointer to) the actual Rate value in Kbps.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqUtilsSetShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    IN    GT_U16    burstSize,
    INOUT GT_U32    *maxRatePtr
);
/**
* @internal prvCpssFalconTxqUtilsGetShapingParameters function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and read shaping parameters from specific node(SW)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[out] burstSizePtr             - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[out] maxRatePtr               -(pointer to) the requested shaping Rate value in Kbps.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsGetShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    OUT    GT_U16    * burstSizePtr,
    OUT    GT_U32    *maxRatePtr
);

/**
* @internal prvCpssFalconTxqUtilsSetShapingEnable function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqUtilsSetShapingEnable
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    IN    GT_BOOL    enable
);
/**
* @internal prvCpssFalconTxqUtilsGetShapingEnable function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and get Enable/Disable Token Bucket rate shaping on specified port or queue of specified device.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[out] enablePtr                - GT_TRUE, enable Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsGetShapingEnable
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    IN  GT_BOOL  *  enablePtr
);
/**
* @internal prvCpssFalconTxqUtilsPortTxBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified port.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
*                                      portNum  - physical or CPU port number
*
* @param[out] numPtr                   - number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsPortTxBufNumberGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    OUT GT_U32 * numPtr
);
/**
* @internal prvCpssFalconTxqUtilsQueueBufPerTcNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated on specified port
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] trafClass                - trafiic class (0..7)
*
* @param[out] numPtr                   - number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsQueueBufPerTcNumberGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8       trafClass,
    OUT GT_U16      *numPtr
);

/**
* @internal prvCpssFalconTxqUtilsQueueEnableSet function
* @endinternal
*
* @brief   Enable/disable queue
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical or CPU port number.
* @param[in] tcQueue                  - traffic class queue on this device (0..7).
* @param[in] enable                   - if GT_TRUE queue  is enabled, otherwise disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsQueueEnableSet
(
     IN GT_U8 devNum,
     IN  GT_PHYSICAL_PORT_NUM portNum,
     IN  GT_U8     tcQueue,
     IN GT_BOOL enable
);
/**
* @internal prvCpssFalconTxqUtilsQueueEnableGet function
* @endinternal
*
* @brief   Get Enable/disable queue status
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] portNum                  - physical or CPU port number.
* @param[in] tcQueue                  - traffic class queue on this device (0..7).
*
* @param[out] enablePtr                - if GT_TRUE queue  is enabled , otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsQueueEnableGet
(
     IN GT_U8 devNum,
     IN  GT_PHYSICAL_PORT_NUM portNum,
     IN  GT_U8     tcQueue,
     IN GT_BOOL * enablePtr
);
/**
* @internal prvCpssFalconTxqUtilsInitPhysicalPortToAnodeDb function
* @endinternal
 *
* @brief  Initialize physical port to A nodedatabase
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                  -  device number
*/

GT_VOID  prvCpssFalconTxqUtilsInitPhysicalPortToAnodeDb
(
    IN GT_U8 devNum
);

/**
* @internal prvCpssFalconTxqUtilsGetQueueGroupAttributes function
* @endinternal
 *
* @brief  Get attributes of queue group
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                  -  device number
* @param[in] queueGroup           - queue group index
* @param[out] tilePtr                   - tile of the queue group
* @param[out] dpPtr  - dp of the queue group
* @param[out]  queueBaseInDpPtr - local port of the queue group
* @param[out] queueBaseInTilePtr - global queue index (per PDQ)
*/

GT_STATUS  prvCpssFalconTxqUtilsGetQueueGroupAttributes
(
    IN GT_U8 devNum,
    IN GT_U32 queueGroup,
    OUT GT_U32 * tilePtr,
    OUT GT_U32 * dpPtr,
    OUT GT_U32 * localPortPtr,
    OUT GT_U32 * queueBaseInDpPtr,
    OUT GT_U32 * queueBaseInTilePtr
);

/**
* @internal prvCpssDxChTxQFalconPizzaArbiterInitPerDp function
* @endinternal
 *
* @brief   Initialize the number of silices to each PDS pizza arbiter,also initialize all the slices to unused
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   -PP's device number.
*@param[in] pdxNum                   -tile number
*@param[in] pdsNum                   -pdsNum number
*@param[in] pdxPizzaNumberOfSlicesArr-number of slices per dp
*/

GT_STATUS prvCpssDxChTxQFalconPizzaArbiterInitPerDp
(
    IN  GT_U8                       devNum,
    IN GT_U32                      pdxNum,
    IN GT_U32                      pdsNum,
    IN GT_U32                      pdxPizzaNumberOfSlices
);

/**
* @internal prvCpssDxChTxQFalconPizzaConfigurationSet function
* @endinternal
 *
* @brief   Update pizza arbiter
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   -PP's device number.
*@param[in] pdxNum                   -tile number
*@param[in] pdsNum                   -pdsNum number
*@param[in] localChannelIndex -the slice owner[0-7]
*@param[in] portPizzaSlicesBitMap -the slice owner[0-7]
*@param[in] enable -equal GT_TRUE in case this is owned ,GT_FALSE otherwise
*@param[in] isCpu -equal GT_TRUE in case this is slice for CPU ,GT_FALSE otherwise
*/

GT_STATUS prvCpssDxChTxQFalconPizzaConfigurationSet
(
    IN  GT_U8                       devNum,
    IN GT_U32                      pdxNum,
    IN GT_U32                      pdsNum,
    IN  GT_U32                     localChannelIndex,
    IN  GT_U32                     portPizzaSlicesBitMap,
    IN  GT_BOOL                 enable,
    IN  GT_BOOL                 isCpu
);
/**
* @internal prvCpssFalconTxqUtilsIsCascadePort function
* @endinternal
 *
* @brief  Check if port has queues mapped at TxQ
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                  -  device number
* @param[in] physicalPortNum           -port number
* @param[out] isCascadePtr  - if equal GT_TRUE this port is cascade,GT_FALSE otherwise

*/

GT_STATUS prvCpssFalconTxqUtilsIsCascadePort
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    OUT GT_BOOL * isCascadePtr
);

/**
* @internal prvCpssFalconTxqUtilsBindPortToSchedulerProfile function
* @endinternal
 *
* @brief  Bind a port to scheduler profile set.
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                  -  device number
* @param[in] physicalPortNum           -port number
* @param[in] profileSet  -The Profile Set in which the scheduler's parameters are
*                                      associated.
*/

GT_STATUS prvCpssFalconTxqUtilsBindPortToSchedulerProfile
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
);

/**
* @internal prvCpssFalconTxqUtilsWrrProfileSet function
* @endinternal
 *
* @brief   Set Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue on this Port (0..7)
* @param[in] wrrWeight                - proportion of bandwidth assigned to this queue
*                                      relative to the other queues in this Arbitration Group
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
*                                      Parameter is associated.
*/

GT_STATUS prvCpssFalconTxqUtilsWrrProfileSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  GT_U8                                   wrrWeight,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
);
/**
* @internal prvCpssFalconTxqUtilsWrrProfileGet function
* @endinternal
 *
* @brief   Get Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue on this Port (0..7)
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
*                                      Parameter is associated.
*
* @param[out] wrrWeightPtr             - Pointer to proportion of bandwidth assigned to this queue
*                                      relative to the other queues in this  Arbitration Group
*/
GT_STATUS prvCpssFalconTxqUtilsWrrProfileGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT  GT_U8                                  *wrrWeightPtr
);
/**
* @internal prvCpssFalconTxqUtilsArbGroupSet function
* @endinternal
 *
* @brief   Set Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue (0..7)
* @param[in] arbGroup                 - scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
*/
GT_STATUS prvCpssFalconTxqUtilsArbGroupSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_Q_ARB_GROUP_ENT            arbGroup,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
);
/**
* @internal prvCpssFalconTxqUtilsArbGroupGet function
* @endinternal
 *
* @brief   Get Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue (0..7)
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
*
* @param[out] arbGroupPtr              - Pointer to scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
*/
GT_STATUS prvCpssFalconTxqUtilsArbGroupGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT CPSS_PORT_TX_Q_ARB_GROUP_ENT            *arbGroupPtr
);
/**
* @internal prvCpssFalconTxqUtilsDumpBindPortsToSchedDb function
* @endinternal
 *
* @brief   Print port to schedule profile binding
*
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
*/
GT_STATUS prvCpssFalconTxqUtilsDumpBindPortsToSchedDb
(
    IN GT_U8 devNum
);
/**
* @internal prvCpssFalconTxqUtilsDumpBindPortsToSchedDb function
* @endinternal
 *
* @brief   Print  schedule profile attributes
*
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
*/
GT_STATUS prvCpssFalconTxqUtilsDumpSchedDbProfiles
(
    IN GT_U8 devNum
);
/**
* @internal prvCpssFalconTxqUtilsBindPortToSchedulerProfile function
* @endinternal
 *
* @brief   Get scheduler profile set that is binded to the port.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                  -  device number
* @param[in] physicalPortNum           -port number
* @param[out] profileSetPtr  -The Profile Set in which the scheduler's parameters are
*                                      associated.
*/

GT_STATUS prvCpssFalconTxqUtilsPortSchedulerProfileGet
(
  IN GT_U8 devNum,
  IN GT_U32 physicalPortNum,
  OUT CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  * profileSetPtr
);

/**
* @internal prvCpssFalconTxqGlobalTxEnableSet function
* @endinternal
 *
* @brief   Enable/Disable transmission of specified device.
*                Set enable/disable on all tiles (1/2/4)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
*/
GT_STATUS   prvCpssFalconTxqGlobalTxEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);
/**
* @internal prvCpssFalconTxqGlobalTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission of specified device (Enable/Disable).
*                Note this function check the status in all availeble tiles (The status should be the SAME)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE, enable transmission
*                                      GT_FALSE, disable transmission
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqGlobalTxEnableGet
(
    IN GT_U8    devNum,
    IN GT_BOOL  * enablePtr
);


/**
* @internal prvCpssDxChTxqFalconDescCounterDump function
* @endinternal
*
* @brief   Print TxQ descriptor counters for specific tile and PDS :
*                1. Per port decriptor count at PDX PAC unit (Port numbering is local to PDS).
*                2. Total PDS decriptor count.
*                3. Per queue decriptor count
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                   - tile number (0..3)
* @param[in] pdsNum                   - pds number  (0..7)
* @param[in] queueStart                 - first queue to dump number  (0..399)
* @param[in] queueEnd                   - last queue to dump number  (0..399)
*
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconDescCounterDump
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueStart,
    IN GT_U32 queueEnd
);

/**
 * @internal prvCpssDxChTxqBindQueueToProfileSet function
 * @endinternal
 *
 * @brief Bind queue to queue  profile( Long queue and Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] tc                      - traffic class (0..7)
 * @param[in] profileIndex           - index of profile (0..15)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvCpssDxChTxqBindQueueToProfileSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    IN GT_U32  profileIndex
);

/**
 * @internal prvCpssDxChTxqBindQueueToProfileGet function
 * @endinternal
 *
 * @brief Get binded  queue  profile index( Long queue and Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] tc                      - traffic class (0..7)
 * @param[out] profileIndexPtr           - (pointer to)index of profile (0..15)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvCpssDxChTxqBindQueueToProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    OUT GT_U32  * profileIndexPtr
);

/**
 * @internal prvCpssDxChTxqSetPortSdqThresholds function
 * @endinternal
 *
 * @brief Set port and queue thresholds (credit allocation requests to PDQ)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] speed                      - port speed
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note   NOTE:      Remote ports are not handled.The configuration for remote ports should be done while configuring cascade port.
 */
GT_STATUS prvCpssDxChTxqSetPortSdqThresholds
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
   IN CPSS_PORT_SPEED_ENT speed
);

/**
 * @internal prvCpssDxChTxqInitPdsLongQueueProfiles function
 * @endinternal
 *
 * @brief Init long queue and length adjust profiles (per all the possible speeds)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note   NOTE: CPSS copy all the profiles to all the DP cores in all tiles
 */

GT_STATUS prvCpssDxChTxqInitPdsLongQueueProfiles
(
    IN  GT_U8   devNum
);

/**
 * @internal prvCpssDxChTxqBindPortQueuesToPdsProfile function
 * @endinternal
 *
 * @brief Bind port queues to PDS profile  (depend on port speed)
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] speed                      - port speed
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note   NOTE:      Remote ports are not handled.The configuration for remote ports should be done while configuring cascade port.
 */

GT_STATUS prvCpssDxChTxqBindPortQueuesToPdsProfile
(
   IN  GT_U8   devNum,
   IN  GT_PHYSICAL_PORT_NUM portNum,
   IN CPSS_PORT_SPEED_ENT speed
);


/**
 * @internal prvCpssDxChTxqSetPlevelDwrr function
 * @endinternal
 *
 * @brief The function set correct weight at DWRR depending on port speed.
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] speed                      - port speed
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note   NOTE:      If the port is local port then P level weight is updated,if the port is remote port then A level weight is updated.
 */

GT_STATUS prvCpssDxChTxqSetDwrr
(
   IN  GT_U8   devNum,
   IN  GT_PHYSICAL_PORT_NUM portNum,
   IN CPSS_PORT_SPEED_ENT speed
);

/**
 * @internal prvCpssDxChTxqPfccTableInit function
 * @endinternal
 *
 * @brief The function initialize PFCC CFG table and Global_pfcc_CFG register
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                  - device number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 */
GT_STATUS prvCpssDxChTxqPfccTableInit
(
    IN GT_U8 devNum
);

/**
 * @internal prvCpssDxChTxqQfcUnitsInit function
 * @endinternal
 *
 * @brief Enable local PFC generation at all QFCs.PFC generation will be controlled at PFCC
 *
 * @note   APPLICABLE DEVICES:       Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                  - device number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 */
GT_STATUS prvCpssDxChTxqQfcUnitsInit
(
    IN GT_U8 devNum
);

/**
* @internal prvCpssFalconTxqPfcValidGet function
* @endinternal
*
* @brief   This function check if PFC is configured per port or port/TC
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                          PP's device number.
* @param[in] tileNum                           tile number
* @param[in] dpNum         -                 data path[0..7]
* @param[in] localPort                         local port number[0..8]
* @param[in] tcBmpPtr                           (pointer to) traffic class that is set for flow control,0x0 mean no flow control is set,0xFF mean flow control on port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssFalconTxqPfcValidGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    OUT  GT_U32                 *tcBmpPtr
);

/**
* @internal prvCpssFalconTxqPffcTableSyncSet function
* @endinternal
*
* @brief   This function read from PDX pizza and configure PFCC table according to number of lanes consumed by port
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                          PP's device number.
* @param[in] tileNum                           tile number
* @param[in] dpNum         -                 data path[0..7]
* @param[in] localPort                         local port number[0..8]
* @param[in] tcBmp                           traffic class that is set for flow control,0xFF mean flow control on port(relevant only if enable is set to GT_TRUE)
* @param[in] enable                           enable/disable entry at PFCC table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPffcTableSyncSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    IN  GT_U32                  tcBmp,
    IN  GT_BOOL                 enable
);
/**
* @internal prvCpssFalconTxqPffcTableSyncSet function
* @endinternal
*
* @brief   This function  PFCC table according to given configuration of consumed lanes
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                         PP's device number.
* @param[in] tileNum                           tile number
* @param[in] dpNum         -                data path[0..7]
* @param[in] localPort                        local port number[0..8]
* @param[in] portPizzaSlices-           desired PDX pizza configuration
* @param[in] tcBmp                            traffic class that is set for flow control,0xFF mean flow control on port(relevant only if enable is set to GT_TRUE)
* @param[in] enable                           enable/disable entry at PFCC table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPffcTableExplicitSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    IN  GT_U32                  portPizzaSlices,
    IN  GT_U32                  tcBmp,
    IN  GT_BOOL                 enable
);

/**
* @internal prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber function
* @endinternal
*
* @brief   Find dp index/local port of physical port
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] physPort                 - physical port number
*
* @param[out] tileNumPtr               - Number of the tile (APPLICABLE RANGES:0..3).
* @param[out] dpNumPtr                 - Number of the dp (APPLICABLE RANGES:0..7).
* @param[out] localPortNumPtr - Number of the local port (APPLICABLE RANGES:0..8).
* @param[out] mappingTypePtr           - mapping type
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong tile number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32      * tileNumPtr,
    OUT GT_U32      * dpNumPtr,
    OUT GT_U32      * localdpPortNumPtr,
    OUT CPSS_DXCH_PORT_MAPPING_TYPE_ENT * mappingTypePtr
);

/**
* @internal prvCpssFalconTxqUtilsFlowControlInit function
* @endinternal
*
* @brief   This function  initialize PFC quanta and PFC threshold for all mapped ports
*
* @note   APPLICABLE DEVICES:           Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                         PP's device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsFlowControlInit
(
    IN GT_U8 devNum
);

/**
* @internal prvCpssFalconTxqUtilsQueueTcSet function
* @endinternal
*
* @brief   Sets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] queueNumber                                              local queue offset [0..15]
* @param[in] tcForPfcResponce                       -      Traffic class[0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsQueueTcSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  queueNumber,
    IN GT_U32                  tc
);
/**
* @internal prvCpssFalconTxqUtilsQueueTcGet function
* @endinternal
*
* @brief   Gets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] queueNumber           local queue offset [0..15]
* @param[in] tcPtr                      -        (pointer to)Traffic class[0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsQueueTcGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  queueNumber,
    IN GT_U32                  *tcPtr
);

/**
* @internal prvCpssFalconTxqUtilsFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert function
* @endinternal
*
* @brief   Falcon : convert the local DMA number in the Local DataPath (DP index in tile),
*         to global DMA number in the device.
*         the global DMA number must be registered in 'port mapping' at this stage
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] tileId                   - The tile Id
* @param[in] localDpIndexInTile       - the Data Path (DP) Index (local DP in the tile !!!)
* @param[in] localDmaNumInDp          - the DMA local number (local DMA in the DP !!!)
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number in the device.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconTxqUtilsFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileId,
    IN  GT_U32  localDpIndexInTile,
    IN  GT_U32  localDmaNumInDp,
    OUT GT_U32  *globalDmaNumPtr
);

/**
* @internal prvCpssFalconTxqUtilsPfcDisable function
* @endinternal
*
* @brief   Read PFC  enable status for both generation and responce and then disable PFC on RX/TX at
*               TxQ and MAC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] portNum                                                         physical port number
* @param[out] pfcRespBitMapPtr                                         (pointer to)current PFC responce mode
* @param[out] pfcGenerationModePtr                      -        (pointer to)current PFC generation mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsPfcDisable
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               *pfcRespBitMapPtr,
    IN CPSS_DXCH_PORT_FC_MODE_ENT           *pfcGenerationModePtr
);

/**
* @internal prvCpssFalconTxqUtilsPfcDisable function
* @endinternal
*
* @brief   Set PFC  enable status for both generation and responce  at
*               TxQ and MAC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] portNum                                                         physical port number
* @param[out] pfcRespBitMapPtr                                         (pointer to)current PFC responce mode
* @param[out] pfcGenerationModePtr                      -        (pointer to)current PFC generation mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsPfcEnable
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               *pfcRespBitMapPtr,
    IN CPSS_DXCH_PORT_FC_MODE_ENT           *pfcGenerationModePtr
);


/**
* @internal prvCpssFalconTxqUtilsPortFlushSet function
* @endinternal
*
* @brief  Set port to "credit ignore" mode.This make port to transmit whenever there is data to send ignoring credits.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     physical device number
* @param[in] portNum                    physical port number
* @param[in] enable                       Enable/disable "credit ignore" mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsPortFlushSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_BOOL                              enable
);
/**
* @internal prvCpssFalconTxqUtilsPortEnableSet function
* @endinternal
*
* @brief  Set port to enable in SDQ.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] portNum                                                         physical port number
* @param[in] enable                                                             Enable/disable "credit ignore" mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsPortEnableSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_BOOL                              enable
);
/**
* @internal prvCpssFalconTxqUtilsPortEnableSet function
* @endinternal
*
* @brief  Flush queues of remote ports.Wait until the queues are empty.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] portNum                                                         physical port number
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsRemotePortFlush
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum
);

/**
* @internal prvCpssFalconTxqUtilsInitTailDrop function
* @endinternal
*
* @brief  Initialize PREQ tail drop parameters to default.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsInitTailDrop
(
    IN GT_U8                                devNum
);

/**
* @internal prvCpssFalconTxqUtilsDevClose function
* @endinternal
*
* @brief  Release dynamic memory allocation for TxQ scheduler
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsDevClose
(
    IN GT_U8                                devNum
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxq */

