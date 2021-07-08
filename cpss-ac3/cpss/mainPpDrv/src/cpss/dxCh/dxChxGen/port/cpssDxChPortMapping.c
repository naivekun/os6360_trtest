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
* @file cpssDxChPortMapping.c
*
* @brief CPSS physical port mapping
*
* @version   26
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitGlobal2localDma.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortTxQHighSpeedPortsDrv.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/cscd/private/prvCpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxq.h>


#define PRV_CPSS_MAPPING_DEBUG_CNS           0
#if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
    /*#define cpssOsPrintf printf*/
    #define PRV_CPSS_MAPPING_CHECK_PRINT_CNS     1
    #define PRV_CPSS_MAPPING_UNIT_LIST_DEBUG_CNS 0
#else
    #define PRV_CPSS_MAPPING_CHECK_PRINT_CNS     0
    #define PRV_CPSS_MAPPING_UNIT_LIST_DEBUG_CNS 0
#endif


#define PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS ((GT_U32)(~0))

/*-----------------------------------------------------------------------------------------------------
+-----+-----------------------------------------------------------------------+-----------------+----------------------+----------------------------------------------------------------------------------
| #   |       Mapping                                                         |  Type           | Device               | cider
+-----+-----------------------------------------------------------------------+-----------------+----------------------+----------------------------------------------------------------------------------
| 1   | RX DMA to Local Device Physical Source Port                           | RxDMA  -> Phys  | Bc2,BobK/Aldrin/Bc3  | RxDMA IP Units/Single Channel DMA Configurations/SCDMA %n Configuration 1
| 2   | Cut Through TX DMA map of Local Device Physical Port to RX DMA number | Phys --> TxDMA  |                 Bc3  | TxDMA IP Units/TxDMA Global Configurations/Local Dev Src Port to DMA Number Map %i
| 3   | BMA map of Local Device Physical Port to RX DMA number                | Phys -> RxDMA   | Bc2,BobK/Aldrin/Bc3  |
| 4.1 | EGF Target Local Physical Port to TxQ port map                        | Phys -> TxQ     | Bc2,BobK/Aldrin/Bc3  |
| 4.2 | Target Local Physical Port to MAC number map                          | Phys -> MAC     | Bc2,BobK/Aldrin/Bc3  |
| 5   | TxQ port to TX DMA map                                                | TxQ  -> TxDMA   | Bc2,BobK/Aldrin/Bc3  |
| 6   | PFC Response Local Device Source Port to TxQ port map                 | Phys -> TxQ     | Bc2,BobK/Aldrin/Bc3  |
| 7   | PFC Trigger Local Device Source Port to MAC map                       | Phys -> MAC     | Bc2,BobK/Aldrin/Bc3  |
| 8   | Local Device Target Physical to hemisphere map                        | Phys -> TxQ (?) |                      |
| 9   | CPU TxQ port number                                                   | Phys -> TxQ     | Bc2,BobK/Aldrin/Bc3  |
+-----+-----------------------------------------------------------------------+-----------------+----------------------+

Also used ...

/Cider/EBU/Bobcat2B/Bobcat2 {Current}/Switching Core/TM_Egress_Glue/Target Interface       TM --> TxDMA ?
                                                                                           TM --> Ilkn channel
/Cider/EBU/Bobcat2B/Bobcat2 {Current}/Switching Core/TM_FCU/EthDma2TM Port Mapping Table   RxDMA --> TM
/Cider/EBU/Bobcat2B/Bobcat2 {Current}/Switching Core/TM_FCU/IlkDma2TM Port Mapping Table   Ilkn  --> TM

Mapping related to PFC message to TM:
/Cider/EBU/Bobcat2B/Bobcat2 {Current}/Switching Core/TM_FCU/TM FCU Ingress Port Mapping<<%r>>
/Cider/EBU/Bobcat2B/Bobcat2 {Current}/Switching Core/TM_FCU/TC Port to CNode Port Mapping Table



+------------+-----------+-----------+-----------+-----------+-----------+
|            |  0  Phys  |  1  MAC   |  2  RxDMA |  3 TxDMA  |  4  TxQ   |
+------------+-----------+-----------+-----------+-----------+-----------+
| Phys     0 | ---       |  7 4.2    |  3        | 2         | 4.1 6     |
+------------+-----------+-----------+-----------+-----------+-----------+
| MAC      1 |           | ---       |           |           |           |
+------------+-----------+-----------+-----------+-----------+-----------+
| RxDMA    2 |   1       |           | ---       |           |           |
+------------+-----------+-----------+-----------+-----------+-----------+
| TxDMA    3 |           |           |           | ---       |           |
+------------+-----------+-----------+-----------+-----------+-----------+
| TxQ      4 |           |           |           | 5         | ---       |
+------------+-----------+-----------+-----------+-----------+-----------+



+-----------------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+
|                                                                             | ETHER_MAC | CPU_SDMA  |  ILNK_MAC |  ILNK_CHNL|  REMOTE_P |
+-----------------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+
| 1    | RX DMA to Local Device Physical Source Port                          |    x      |    x      |  x(const) |           |           |
+------+----------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+
| 2    | Cut Through TX DMA map of Local Device Physical Port to RX DMA number|    x      |    x      |    x      |    x      |    x      | Bobcat3 only
+------+----------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+
| 3    | BMA map of Local Device Physical Port to RX DMA number               |    x      |    x      |    x      |    x      |    x      |
+------+----------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+
| 4.1  | EGF Target Local Physical Port to TxQ port map                       |    x      |    x      |    x      |    x      |    x      |
+------+----------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+
| 4.2  | EGF Target Local Physical Port to MAC number                         |    x      |    x      |    x      |    x      |    x      |
+------+----------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+
| 5    | TxQ port to TX DMA map                                               |    x      |    x      |    x      |    x      |    x      |
+------+----------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+
| 6    | PFC Response Local Device Source Port to TxQ port map                |    x      |    x      |    x      |    x      |    x      |
+------+----------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+
| 7    | PFC Trigger Local Device Source Port to MAC map                      |    x      |           |    x      |    x      |    x      |
+------+----------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+
| 8    | Local Device Target Physical to hemisphere map                       |    x      |    ?      |           |           |           | Lion 3 only
+------+----------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+
| 9    | CPU TxQ port number                                                  |           |    x      |           |           |           |
+------+----------------------------------------------------------------------+-----------+-----------+-----------+-----------+-----------+

ILNK_MAC : MAC 1 DMA 1 64 channels (physical port : base + channel#)
   mapping 5 TxQ port to TX DMA map

   if tmEnable == TRUE
       txDMA TM port 73 instead of inteface number

there is need to INLK_CHANNEL (all 9 mapping)


GetInteface

                      +----> 72 (BobCat2) CPU_SDMA)
                      |
      (3)             |             (1)
Phy -------->RxDMA ---+----> 73  -------->RxDMA --->  +-- n1 != n2 ---------> ILKN_CNHL
              (n1)    |                    (n2)       |
                      |                               +---n1 == n2 --------> INLKN_MAC
                      |               (1)
                      +---> !(72,73) ------> RxDMA --> +-- n1 != n2 ---------> ETHERNET
                                              n2       |
                                                       +---n1 == n2 --------> REMOTE_PORT


(1) RX DMA to Local Device Physical Source Port
[cider] RxDMA IP Units/Single Channel DMA Configurations/SCDMA %n Configuration 1

formula : 0x17000950 + n*0x4: where n (0-72) represents Port
  7       0     Port Number <<%n>>  0x<<%n>> The source port number assigned to the packet upon its reception.
                                     According to this field, the port number will be changed when sending it to the control pipe.


(2) TX DMA map of Local Device Physical Port to RX DMA number
    [cider]  TxDMA IP Units/TxDMA Global Configurations/Local Dev Src Port to DMA Number Map %i

  Address: 0x26000020 + [i/4]*0x4: where i (0-63) represents entry

   0  -  7      4 ports per register, total
   8  - 15
   16 - 23
   24 - 31

(3)  BMA map of Local Device Physical Port to RX DMA number
  Port Mapping table %n
  Description: Virtual => Physical source port mapping
  Address: CPU 0x2D01A000
  Size: 1024 Bytes

(4.1) EGF Target Local Physical Port to TxQ port map
  Internal Name: Target_Port_Mapper
  Address: CPU 0x3B900000

  Size: 1024 Bytes
  Width (Line Width): 8 Bits
  No. of Lines: 256
  Line Address Alignment: 4 bytes

(4.2) EGF Target Local Physical Port to Mac number
    Offset Formula: 0x35010120 + 4*n: where n (0-63) represents offset
    0- 7    Remaps port <<4*%n+0>> to its corresponding MAC number
    8-15    Remaps port <<4*%n+1>> to its corresponding MAC number
   16-23    Remaps port <<4*%n+2>> to its corresponding MAC number
   24-31    Remaps port <<4*%n+3>> to its corresponding MAC number

(5) TxQ port to TX DMA map

    Address: CPU   0x400002C0
    formula : 0x400002C0 + n*0x4: where n (0-71) represents Port

(6) PFC Response Local Device Source Port to TxQ port map  (Phys --> TxQ)
    0x4000C500 + p*4: where p (0-255) represents Source Port
    0-6

(7) PFC Trigger Local Device Source Port to MAC map  (Phys --> MAC)
    Offset Formula: 0x3E000900 + p*4: where p (0-255) represents Global Port
     0-3 PFC Counter Index
     4-6 PFC Counter block
     7-9 Local Core ID
    10-17 MAC Index
*/
/*---------------------------------------------------------*/
/* Useful data structures                                  */
/*---------------------------------------------------------*/

#define PRV_CPSS_DXCH_UNUSED_PARAM_MAC(p) p = p

typedef GT_STATUS (*PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 resId,
    IN GT_U32 valueToSet
);

typedef GT_STATUS (*PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 resId,
   OUT GT_U32 * valuePtr
);



/**
* @enum PRV_MAPTYPE_ENT
 *
 * @brief port type used in mapping functions
*/
typedef enum{

    /** invalide maping type */
    PRV_MAPTYPE_INVALID_E          = 0x0,

    /** Ethernet MAC */
    PRV_MAPTYPE_ETHERNET_MAC_E     = BIT_0,

    /** CPU SDMA */
    PRV_MAPTYPE_CPU_SDMA_E         = BIT_1,

    /** Interlaken Channel where mac shall be configured ( first in list) */
    PRV_MAPTYPE_ILKN_MAC_E         = BIT_2,

    /** Interlaken Channel where mac shall be not configured */
    PRV_MAPTYPE_ILKN_CHANNEL_E     = BIT_3,

    /** Port represent physical interface on remote device or Interlaken channel. */
    PRV_MAPTYPE_REMOTE_PHYS_PORT_E = BIT_4

} PRV_MAPTYPE_ENT;

/******************************************************************
 * partial mapping description structure
 *      describe operation set/get on particular mapping
 *      ---------------------------
 *      mapping[srcPort] = dstPort
 *      ---------------------------
 *
 *      stringMapTypePtr - pointer mapping name (debug purpose only) , at the end of list NULL
 *      mapingTypeBmp    - bitmap of all mapping (system) when this particular mapping type is used
 *                         0 at the end of list of all mapping
 *      applicableDevice - applicable device (same mapping are supported on specific device
 *                         (APPLICABLE DEVICES: Bobcat2, Caelum, Bobcat3; Aldrin2; Falcon, Aldrin, AC3X, or both, 0 and the end of list)
 *      srcPortType      - mapping source from;
 *                         (APPLICABLE RANGES: PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
 *                                             PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E,
 *                                             PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E,
 *                                             PRV_CPSS_DXCH_PORT_TYPE_MAC_E,
 *                                             PRV_CPSS_DXCH_PORT_TYPE_TXQ_E;
 *                              PRV_CPSS_DXCH_PORT_TYPE_INVALID_E at the end of list)
 *      dstPortType      - mapping destination from;
 *                         (APPLICABLE RANGES: PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
 *                                             PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E,
 *                                             PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E,
 *                                             PRV_CPSS_DXCH_PORT_TYPE_MAC_E,
 *                                             PRV_CPSS_DXCH_PORT_TYPE_TXQ_E;
 *                              PRV_CPSS_DXCH_PORT_TYPE_INVALID_E at the end of list)
 *      setFun          - pointer to set function (NULL at the end)
 *      getFun          - pointer to get function (NULL at the end)
 *
 * Comments:
 *      array of ports (of size CPSS_DXCH_PORT_MAPPING_TYPE_MAX_E) shall be build by the system arguments;
 *      +-------+-------+-------+------+-------+
 *      | phys  | rxdma | txdma | mac  |  txq  |
 *      +-------+-------+-------+------+-------+
 *      the caller to set function shall get argument     from source index and
 *                                           destination from destination index
 *      +--------------+-----------+----------+-------+------+-----+----+
 *      | "map-type-1" | ETHER,CPU | BC2      | rxdma | phys | SetF|GetF|
 *      | "map-type-2" | ETHER     | CAELUM   | rxdma | txqs | SetF|GetF|
 *      | NULL         | 0         | 0        | INV   | INV  |NULL |NULL|
 *      +--------------+-----------+----------+-------+------+-----+----+
 *        the called iterate over the list till mappingTypeBmp is not zero.
 *             if mapping type and system type are appropriate,
 *             calls the set function , taking argument from argument array at relevant array.
 *******************************************************************/
typedef struct
{
    #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        GT_CHAR                          * stringMapTypePtr;
    #endif
    GT_U32                                 mapingTypeBmp;
    GT_U32                                 applicableDevice;
    PRV_CPSS_DXCH_PORT_TYPE_ENT            srcPortType;
    PRV_CPSS_DXCH_PORT_TYPE_ENT            dstPortType;
    PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR      setFun;
    PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR      getFun;
}PRV_DXCH_CPSS_MAPPING_SET_GET_FUN_STC;


#define PRV_CPSS_DXCH_MAPPING_BOBCAT2_MAX_DMA_NUM_CNS   71
#define PRV_CPSS_DXCH_MAPPING_BOBCAT2_MAX_MAC_NUM_CNS   71
#define PRV_CPSS_DXCH_MAPPING_MAX_TXQ_PORT_CNS(dev)          \
        (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ?                  \
            SIP_5_20_TXQ_DQ_MAX_PORTS_MAC(dev):              \
             71)


#define PRV_CPSS_DXCH_MAPPING_ETH_MIN_TM_PORT_IDX_CNS   0    /* range 0 .. 71 */
#define PRV_CPSS_DXCH_MAPPING_ETH_MAX_TM_PORT_IDX_CNS   71
#define PRV_CPSS_DXCH_MAPPING_ILKN_MIN_TM_PORT_IDX_CNS  128  /* range 128 .. 191 */
#define PRV_CPSS_DXCH_MAPPING_ILKN_MAX_TM_PORT_IDX_CNS  191

#define PRV_CPSS_DXCH_MAPPING_BOBCAT2_ILKN_CHANNEL_MAX_PHYS_PORT_DIFF_CNS  64

/* TM port index range is 0..71 and 128..191*/
#define PRV_CPSS_DXCH_MAPPING_BOBCAT2_TM_PORT_NUM_LOW_RANGE_MAX_CNS  \
            (PRV_CPSS_DXCH_MAPPING_ETH_MAX_TM_PORT_IDX_CNS + 1)
#define PRV_CPSS_DXCH_MAPPING_BOBCAT2_TM_PORT_NUM_HIGH_RANGE_START_CNS  PRV_CPSS_DXCH_MAPPING_ILKN_MIN_TM_PORT_IDX_CNS
#define PRV_CPSS_DXCH_MAPPING_BOBCAT2_TM_PORT_NUM_HIGH_RANGE_MAX_CNS \
            (PRV_CPSS_DXCH_MAPPING_ILKN_MAX_TM_PORT_IDX_CNS + 1)

/*--------------------------------------------------------------*/
/* 1. RxDMA 2 Local Physical Port                               */
/*--------------------------------------------------------------*/
/*Allowed range 0:72 */
#define PRV_CPSS_DXCH_RXDMA_2_PHYS_MAX_RxDMA_PORT_CNS  72
#define PRV_CPSS_DXCH_RXDMA_2_PHYS_MAX_PHYS_PORT_CNS(devNum)   MAX(255,(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.maxPhysicalPorts-1))

#define PRV_CPSS_DXCH_RXDMA_2_PHYS_OFFS_CNS 0
#define PRV_CPSS_DXCH_RXDMA_2_PHYS_LEN_CNS(devNum)  MAX(8,PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).phyPort)

/**
* @internal internal_prvCpssDxChPortMappingRxDMA2LocalPhysSet function
* @endinternal
*
* @brief   configure RxDMA 2 phys port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] rxDmaNum                 - rxDMA port
* @param[in] physPort                 - physical port
* @param[in] isSdmaCpuPort            - is this CPU SDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS internal_prvCpssDxChPortMappingRxDMA2LocalPhysSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 rxDmaNum,
    IN GT_U32 physPort,
    IN GT_BOOL isSdmaCpuPort
)
{
    GT_STATUS rc = GT_OK;
    GT_U32      regAddr;            /* register's address */
    GT_U32 dpIdx,localDmaNum;
    GT_U32  TTI_myPhysicalPortAttributes_numEntries;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);


    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,rxDmaNum,/*OUT*/&dpIdx,&localDmaNum);
    if (GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (physPort > PRV_CPSS_DXCH_RXDMA_2_PHYS_MAX_PHYS_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* NOTE: this field is per ingress port (unlike most bits in this table) */
        rc = prvCpssDxChWriteTableEntryField(devNum,
             CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
             physPort, /*global port*/
             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
             SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INVALID_CRC_MODE_E, /* field name */
             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
#ifdef GM_USED
             0);
#else
             1/* the MAC at the Raven stripped the CRC so packets from it not include the 4 bytes of CRC*/);
#endif
        if(rc != GT_OK)
        {
            return rc;
        }

#ifdef GM_USED
        if(prvCpssDxchFalconCiderVersionGet() >= PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_06_06_E/*former18_04_22*/)
        {
            /* disable the PCH on the RxDma channel of ALL used ports (because get packets without it from the SLAN) */
            regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
                configs.channelConfig.PCHConfig[rxDmaNum];
            /*<Ingress Packet Include PCH>*/
            rc = prvCpssHwPpSetRegField(devNum,regAddr, 0, 1, 0/*disable the PCH*/);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
#endif


        /* according to the number of entries in
           PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).phyPortAndEPortAttributes.myPhysicalPortAttributes[] */
        TTI_myPhysicalPortAttributes_numEntries = 288;

        if (physPort >= TTI_myPhysicalPortAttributes_numEntries)
        {
            /* due to limitation in setting in function prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet(...) */
            /* NOTE : in BC3 we allowed a 'trick' to set value >= 256..511
                    but that was for testing purposes ... that no longer valid for Falcon device.
            */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "physPort[%d] must be less than [%d] (TTI unit limitation)",
                physPort,
                TTI_myPhysicalPortAttributes_numEntries);
        }

        /*  JIRA: RX7-90 : Wrong default value of recalc_crc field in Channel %n General Configurations register */
        /* All the ports must have recalc CRC , because it not comes from the D2D ports ,
           and also from MG unit it is NOT connected to 'recalc CRC' from the MG descriptor */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
            configs.channelConfig.channelGeneralConfigs[rxDmaNum];
        /* <Recalc CRC> */
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
            0, 1,
#ifdef GM_USED
             0);
#else
             1/* the MAC at the Raven stripped the CRC so packets from it not include the 4 bytes of CRC*/);
#endif
        if(rc != GT_OK)
        {
            return rc;
        }

        if(localDmaNum == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].cpuPortDmaNum)
        {
            regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum,dpIdx).
                configs.globalConfigs.globalConfig1;
            /*
                NOTE: this bit can NOT be 'switched' from 'non-SDMA' back to 'SDMA'
                    or vis versa. (unless you do reset)

                in RTLF2.0 :
                this issue caused TxFifo to stack and not send traffic to the SDMA
                (since we set '0'(non-SDMA) to register and then we did '1'(SDMA)
                but the device already 'loaded' 0 credits from the D2D and would
                not get any more.)
            */

            /* enable/Disable SDMA Port*/
            rc = prvCpssHwPpSetRegField(devNum,regAddr, 0, 1, BOOL2BIT_MAC(isSdmaCpuPort));
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).configs.channelConfig.channelToLocalDevSourcePort[rxDmaNum];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig1[rxDmaNum];
    }
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                            PRV_CPSS_DXCH_RXDMA_2_PHYS_OFFS_CNS,
                                            PRV_CPSS_DXCH_RXDMA_2_PHYS_LEN_CNS(devNum),
                                            physPort);

    return rc;
}

/**
* @internal prvCpssDxChPortMappingRxDMA2LocalPhysSet function
* @endinternal
*
* @brief   configure RxDMA 2 phys port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] rxDmaNum                 - rxDMA port
* @param[in] physPort                 - physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingRxDMA2LocalPhysSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 rxDmaNum,
    IN GT_U32 physPort
)
{
    return internal_prvCpssDxChPortMappingRxDMA2LocalPhysSet(
        devNum,portGroupId,rxDmaNum,physPort,GT_FALSE/* non-SDMA CPU port */);
}
/**
* @internal prvCpssDxChPortMappingRxDMA2LocalPhysSet_cpuSdma function
* @endinternal
*
* @brief   sip6 : CPU SDMA port : configure RxDMA 2 phys port mapping
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] rxDmaNum                 - rxDMA port
* @param[in] physPort                 - physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingRxDMA2LocalPhysSet_cpuSdma
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 rxDmaNum,
    IN GT_U32 physPort
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;
    GT_U32  dpIdx,localDmaNum;

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* do standard port rxDma settings */
    rc = internal_prvCpssDxChPortMappingRxDMA2LocalPhysSet(
        devNum,portGroupId,rxDmaNum,physPort,GT_TRUE/* SDMA CPU port*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /**********************************/
    /* do extra CPU SDMA port mapping */
    /**********************************/

    /* disable the PCH on the RxDma channel of the SDMA CPU port */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
        configs.channelConfig.PCHConfig[rxDmaNum];
    /*<Ingress Packet Include PCH>*/
    rc = prvCpssHwPpSetRegField(devNum,regAddr, 0, 1, 0/*disable the PCH*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* NOTE: this field is per ingress port (unlike most bits in this table) */
    rc = prvCpssDxChWriteTableEntryField(devNum,
         CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
         physPort, /*global port*/
         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
         SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INVALID_CRC_MODE_E, /* field name */
         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
         0/* the CPU send with 4 bytes of CRC*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /***********************/
    /* each CPU SDMA port is in different DP units , need to set the mux in the TxFifo */
    /***********************/
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,rxDmaNum,/*OUT*/&dpIdx,&localDmaNum);
    if (GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(localDmaNum != PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].cpuPortDmaNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The CPS SDMA must be with local DMA number[%d] but got [%d] \n",
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].cpuPortDmaNum,
            localDmaNum);
    }

    return GT_OK;
}



/**
* @internal prvCpssDxChPortMappingRxDMA2LocalPhysGet function
* @endinternal
*
* @brief   RxDMA 2 phys port mapping get
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] rxDmaNum                 - rxDMA port
*
* @param[out] physPortPtr              - (pointer to) physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingRxDMA2LocalPhysGet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 rxDmaNum,
   OUT GT_U32 *physPortPtr
)
{
    GT_STATUS rc;
    GT_U32      regAddr;            /* register's address */
    GT_U32 dpIdx,localDmaNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(physPortPtr);

    *physPortPtr = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;

    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,rxDmaNum,/*OUT*/&dpIdx,&localDmaNum);
    if (GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* it is enough to get data from single pipe mapping, since them both are configured same */

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).configs.channelConfig.channelToLocalDevSourcePort[rxDmaNum];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig1[rxDmaNum];
    }
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                            PRV_CPSS_DXCH_RXDMA_2_PHYS_OFFS_CNS,
                                            PRV_CPSS_DXCH_RXDMA_2_PHYS_LEN_CNS(devNum),
                                            physPortPtr);

    return rc;
}


/*-----------------------------------------------------------------------------*/
/* 2.  Cut Through TX DMA map of Local Device Physical Port to RX DMA number   */
/*-----------------------------------------------------------------------------*/
/* allowed port 0-127 */
/* [cider]  TxDMA IP Units/TxDMA Global Configurations/Local Dev Src Port to DMA Number Map %i */
#define PRV_CPSS_DXCH_TXDMA_PHYS_2_RxDMA_MAX_PHYS_PORT_CNS(devNum)            MAX(512,(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.maxPhysicalPorts-1))

#define PRV_CPSS_DXCH_TXDMA_PHYS_2_DMA_OFFS_CNS         0
#define PRV_CPSS_DXCH_TXDMA_PHYS_2_DMA_LEN_CNS          8
#define PRV_CPSS_DXCH_TXDMA_PHYS_2_DMA_FLD_PER_REG_CNS  4

/*----------------------------------------------------------------
 *  BC3:
 *    TxDMA 0 :  128*4 = 512 entries (local phys port)   [x] = 5
 *    TxDMA 1 :  128*4 = 512                             [x] = 5
 *    TxDMA 2 :                                          [x] = 5
 *    TxDMA 3 :                                          [x] = 5
 *    TxDMA 4 :                                          [x] = 5
 *    TxDMA 5 :                                          [x] = 5
 *  example:
 *    local-phys-port         globalDma,         dp-index,  localDma
 *        x               ,{     29,       {         2,         5 }    }
 *
 *----------------------------------------------------------------*/
static GT_STATUS prvCpssDxChPortMappingCutThroughTxDMALocalPhys2RxDMASet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 physPort,
    IN  GT_U32 rxDmaNum

)
{
    GT_STATUS rc = GT_OK;
    GT_U32      regAddr;            /* register's address */
    GT_U32      fldOffs;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC * pReg;
    GT_U32 dpIndex;
    GT_U32 localDmaNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E
                                            | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if (physPort > PRV_CPSS_DXCH_TXDMA_PHYS_2_RxDMA_MAX_PHYS_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,rxDmaNum,/*OUT*/&dpIndex,&localDmaNum);
    if (GT_OK != rc )
    {
        return rc;
    }

    for (dpIndex = 0 ; dpIndex < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp; dpIndex++)
    {
        pReg = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

        regAddr = pReg->txDMA[dpIndex].txDMAGlobalConfigs.localDevSrcPort2DMANumberMap[physPort/PRV_CPSS_DXCH_TXDMA_PHYS_2_DMA_FLD_PER_REG_CNS];
        fldOffs = (physPort %PRV_CPSS_DXCH_TXDMA_PHYS_2_DMA_FLD_PER_REG_CNS) * PRV_CPSS_DXCH_TXDMA_PHYS_2_DMA_LEN_CNS;
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                fldOffs,
                                                PRV_CPSS_DXCH_TXDMA_PHYS_2_DMA_LEN_CNS,
                                                localDmaNum);

        if (GT_OK != rc )
        {
            return rc;
        }
    }
    return GT_OK;
}


static GT_STATUS prvCpssDxChPortMappingCutThroughTxDMALocalPhys2RxDMAGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 physPort,
    OUT GT_U32 *rxDmaNumPtr

)
{
    GT_STATUS rc = GT_OK;
    GT_U32      regAddr;            /* register's address */
    GT_U32      fldOffs;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC * pReg;
    GT_U32 dpIndex;
    GT_U32 convertedLocalRxDmaPort,hwLocalRxDmaPort;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E
                                            | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC (rxDmaNumPtr);

    if (physPort > PRV_CPSS_DXCH_TXDMA_PHYS_2_RxDMA_MAX_PHYS_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physPort, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check that the info match the HW value */
    /* need to write the 'local' DMA number */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
                                                            /*IN*/ portMapShadowPtr->portMap.rxDmaNum/*global number*/,
                                                            /*OUT*/&dpIndex,
                                                            /*OUT*/&convertedLocalRxDmaPort);
    if (GT_OK != rc )
    {
        return rc;
    }

    for (dpIndex = 0; dpIndex < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq; dpIndex++)
    {
        pReg = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
        regAddr = pReg->txDMA[dpIndex].txDMAGlobalConfigs.localDevSrcPort2DMANumberMap[physPort/PRV_CPSS_DXCH_TXDMA_PHYS_2_DMA_FLD_PER_REG_CNS];
        fldOffs = (physPort % PRV_CPSS_DXCH_TXDMA_PHYS_2_DMA_FLD_PER_REG_CNS)*PRV_CPSS_DXCH_TXDMA_PHYS_2_DMA_LEN_CNS;
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                fldOffs,
                                                PRV_CPSS_DXCH_TXDMA_PHYS_2_DMA_LEN_CNS,
                                                &hwLocalRxDmaPort);

        if (GT_OK != rc )
        {
            return rc;
        }


        if(convertedLocalRxDmaPort != hwLocalRxDmaPort)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DB management error : the DB hold global rxDMA [%d] "
                "that considered local rxDMA[%d] that not match the "
                "value [%d] read from the HW for physical port[%d] \n",
                portMapShadowPtr->portMap.rxDmaNum,
                convertedLocalRxDmaPort,
                hwLocalRxDmaPort,
                physPort
                );
        }
    }

    *rxDmaNumPtr = portMapShadowPtr->portMap.rxDmaNum;
    return GT_OK;
}



/*-----------------------------------------------------------------------------*/
/* 3. BMA map of Local Device Physical Port to RX DMA number                   */
/*-----------------------------------------------------------------------------*/
#define PRV_CPSS_DXCH_BMA_PHYS_2_RXDMA_MAX_PHYS_PORT_CNS(devNum) MAX(255,(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.maxPhysicalPorts-1))
#define PRV_CPSS_DXCH_BMA_PHYS_2_RXDMA_MAX_RXDMA_PORT_CNS 255

/**
* @internal prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet function
* @endinternal
*
* @brief   configure BMA local physical port 2 RxDMA port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - physical port
*                                      rxDmaNum   - rxDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 physPort,
    IN GT_U32 rxDMAPort
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    dataPath;        /* data path number */
    GT_U32    dataPathIterNum; /* data path iterations number */
    GT_U32    localRxDMA;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_UNUSED_PARAM_MAC(portGroupId);

    if (physPort > PRV_CPSS_DXCH_BMA_PHYS_2_RXDMA_MAX_PHYS_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* need to write the 'local' DMA number */
    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
              rxDMAPort/*global number*/,
              /*OUT*/ NULL,
              &localRxDMA);/*converted to local number*/
    if (GT_OK != rc )
    {
        return rc;
    }

    if (localRxDMA > PRV_CPSS_DXCH_BMA_PHYS_2_RXDMA_MAX_RXDMA_PORT_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "rxDMAPort[%d] lead to localRxDMA[%d] > max[%d]",
            rxDMAPort , localRxDMA , PRV_CPSS_DXCH_BMA_PHYS_2_RXDMA_MAX_RXDMA_PORT_CNS);
    }

    if ((PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) ||
        (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportMultiDataPath == 0))
    {
        /* Bobcat3 is multiDataPath device but it use only one instance of the BMA map table */
        dataPathIterNum = 1;
    }
    else
    {
        dataPathIterNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;
    }


    /* need to configure all instances of BMA map for multi-datapath devices like Caelum.
       CPSS use one logical table for all instances. Each instance has 256 entires.*/
    for (dataPath = 0; dataPath < dataPathIterNum; dataPath++)
    {
        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP5_TABLE_BMA_PORT_MAPPING_E,
                                        physPort + (256 * dataPath),
                                        &localRxDMA);
        if (GT_OK != rc )
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMAGet function
* @endinternal
*
* @brief   get mapping of BMA local physical port 2 RxDMA port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - physical port
*
* @param[out] rxDmaPortPtr             - rxDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMAGet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 physPort,
   OUT GT_U32 *rxDmaPortPtr
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_UNUSED_PARAM_MAC(portGroupId);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    if (physPort > PRV_CPSS_DXCH_BMA_PHYS_2_RXDMA_MAX_PHYS_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(rxDmaPortPtr);

    *rxDmaPortPtr = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;

    rc = prvCpssDxChReadTableEntry(devNum,
                                    CPSS_DXCH_SIP5_TABLE_BMA_PORT_MAPPING_E,
                                    physPort,
                                    rxDmaPortPtr);
    if (GT_OK != rc )
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportRelativePortNum)
    {
        /* the value in (*rxDmaPortPtr) is 'local DMA' number , and we don't know the 'DP_index'
           so we need to use the DB.
        */
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
        GT_U32  hwLocalRxDmaPort = *rxDmaPortPtr;
        GT_U32  convertedLocalRxDmaPort;

        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physPort, /*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(portMapShadowPtr->valid)
        {
            /* check that the info match the HW value */
            /* need to write the 'local' DMA number */
            rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
                portMapShadowPtr->portMap.rxDmaNum/*global number*/,
                NULL,
                &convertedLocalRxDmaPort);/*converted to local number*/
            if (GT_OK != rc )
            {
                return rc;
            }

            if(convertedLocalRxDmaPort != hwLocalRxDmaPort)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DB management error : the DB hold global rxDMA [%d] "
                    "that considered local rxDMA[%d] that not match the "
                    "value [%d] read from the HW for physical port[%d] \n",
                    portMapShadowPtr->portMap.rxDmaNum,
                    convertedLocalRxDmaPort,
                    hwLocalRxDmaPort,
                    physPort
                    );
            }

            *rxDmaPortPtr = portMapShadowPtr->portMap.rxDmaNum;
        }
    }



    return GT_OK;
}


/*-----------------------------------------------------------------------------*/
/* 4.1 EGF Target  Local Device Physical Port to TxQ port Map                  */
/*-----------------------------------------------------------------------------*/
#define PRV_CPSS_DXCH_EGF_PHYS_2_TXQ_MAX_PHYS_PORT_CNS(dev) \
        (GT_U32)(PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ?         \
             1024/*512*/:                                    \
             255)
#define PRV_CPSS_DXCH_EGF_PHYS_2_TXQ_MAX_TXQ_PORT_CNS(dev)  \
        (PRV_CPSS_SIP_6_CHECK_MAC(dev)    ? (BIT_10-1):     \
         PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ? SIP_5_20_TXQ_DQ_MAX_PORTS_MAC(dev): \
             255)

/**
* @internal prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapSet function
* @endinternal
*
* @brief   configure EGF target local physical port 2 TxQ port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - physical port
* @param[in] txqNum                   - txq port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapSet
(
    IN GT_U8                devNum,
    IN GT_U32               portGroupId,
    IN GT_PHYSICAL_PORT_NUM physPort,
    IN GT_U32               txqNum
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E );

    PRV_CPSS_DXCH_UNUSED_PARAM_MAC(portGroupId);

    if (CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS == txqNum)
    {
        return GT_OK;
    }

    if (physPort > PRV_CPSS_DXCH_EGF_PHYS_2_TXQ_MAX_PHYS_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (txqNum > PRV_CPSS_DXCH_EGF_PHYS_2_TXQ_MAX_TXQ_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "EGF-QAG <Port Base> limited to [0..%d] but got [%d]",
            PRV_CPSS_DXCH_EGF_PHYS_2_TXQ_MAX_TXQ_PORT_CNS(devNum),txqNum);
    }

    rc = prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E, physPort,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_BASE_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        txqNum);
    if (GT_OK != rc )
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapGet function
* @endinternal
*
* @brief   get mapping of EGF target local physical port 2 TxQ port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - physical port
*
* @param[out] txqNumPtr                - txq port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 physPort,
    OUT GT_U32 *txqNumPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    PRV_CPSS_DXCH_UNUSED_PARAM_MAC(portGroupId);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E );


    if (physPort > PRV_CPSS_DXCH_EGF_PHYS_2_TXQ_MAX_PHYS_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(txqNumPtr);

    /* A device table "Target Port Mapper" can't hold "nonexistent value".
     * So read portMapShadowDb first to check if txq port is "invalid".
     * If yes - return "invalid",
     * if not - read the device table.*/
    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physPort, &portMapShadowPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    if (portMapShadowPtr->valid == GT_TRUE  &&
        portMapShadowPtr->portMap.txqNum  ==  PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
    {
        *txqNumPtr = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
    }
    else
    {
        rc = prvCpssDxChReadTableEntryField(
            devNum, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E, physPort,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP5_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_BASE_E,
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            txqNumPtr);
        if (GT_OK != rc )
        {
            return rc;
        }
    }

    return GT_OK;
}


/*-----------------------------------------------------------------------------*/
/* 4.2 EGF Target  Local Device Physical Port to TxQ port Map                   */
/*-----------------------------------------------------------------------------*/
#define PRV_CPSS_DXCH_EGF_PHYS_2_MAC_MAX_PHYS_PORT_CNS 255
#define PRV_CPSS_DXCH_EGF_PHYS_2_MAC_MAX_MAC_PORT_CNS  255

/**
* @internal prvCpssDxChPortMappingEGFTargetLocalPhys2MACNumMapSet function
* @endinternal
*
* @brief   configure EGF target local physical port 2 Mac number mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - physical port
* @param[in] macNum                   - mac number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingEGFTargetLocalPhys2MACNumMapSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 physPort,
    IN GT_U32 macNum
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_UNUSED_PARAM_MAC(portGroupId);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    if (physPort > PRV_CPSS_DXCH_EGF_PHYS_2_MAC_MAX_PHYS_PORT_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (macNum > PRV_CPSS_DXCH_EGF_PHYS_2_MAC_MAX_MAC_PORT_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChHwEgfEftFieldSet(devNum,physPort/*virtual port (target port)*/,
        PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_VIRTUAL_2_PHYSICAL_PORT_REMAP_E,
        macNum/*global physical port*/);

    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortMappingEGFTargetLocalPhys2MACNumMapGet function
* @endinternal
*
* @brief   get mapping of EGF target local physical port 2 Mac number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - physical port
*
* @param[out] macNumPtr                - mac number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingEGFTargetLocalPhys2MACNumMapGet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 physPort,
   OUT GT_U32 *macNumPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_UNUSED_PARAM_MAC(portGroupId);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    if (physPort > PRV_CPSS_DXCH_EGF_PHYS_2_MAC_MAX_PHYS_PORT_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(macNumPtr);


    rc = prvCpssDxChHwEgfEftFieldGet(devNum,physPort/*virtual port (target port)*/,
        PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_VIRTUAL_2_PHYSICAL_PORT_REMAP_E,
        macNumPtr/*global physical port*/);
    if (GT_OK != rc)
        {
            return rc;
    }
        return GT_OK;
}

/*-----------------------------------------------------------------------------*/
/* 4.3 sip6  EGF Target  Local Device Physical Port to TxQ port Base Map       */
/*-----------------------------------------------------------------------------*/
/*
NOTE: we have different set of functions for SIP6 to show that the TXQ 'queue' is
   NOT defined in the EGF ... the 'queue' is only known at the TXQ after mapping
   <port base> to it.
*/

/**
* @internal prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapSet function
* @endinternal
*
* @brief   configure EGF target local physical port 2 TxQ <port base> mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - physical port
* @param[in] txqPortBase              - txq port base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapSet
(
    IN GT_U8                devNum,
    IN GT_U32               portGroupId,
    IN GT_PHYSICAL_PORT_NUM physPort,
    IN GT_U32               txqPortBase
)
{
/*
NOTE: we have different set of functions for SIP6 to show that the TXQ 'queue' is
   NOT defined in the EGF ... the 'queue' is only known at the TXQ after mapping
   <port base> to it.
*/
    /* call sip5 implementation */
    return prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapSet(devNum,portGroupId,physPort,txqPortBase);
}

/**
* @internal prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapGet function
* @endinternal
*
* @brief   get mapping of EGF target local physical port 2 TxQ port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - physical port
*
* @param[out] txqPortBasePtr           - (pointer to) txq port base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 physPort,
    OUT GT_U32 *txqPortBasePtr
)
{
/*
NOTE: we have different set of functions for SIP6 to show that the TXQ 'queue' is
   NOT defined in the EGF ... the 'queue' is only known at the TXQ after mapping
   <port base> to it.
*/
    /* call sip5 implementation */
    return prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapGet(devNum,portGroupId,physPort,txqPortBasePtr);
}

/*-----------------------------------------------------------------------------*/
/* 5. TxQ port to TX DMA map                                                   */
/*-----------------------------------------------------------------------------*/
#define PRV_CPSS_MAPPING_TXQ_2_TXDMA_MAX_TXQ_PORT_CNS(dev)    \
        (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) ?                 \
            SIP_5_20_TXQ_DQ_MAX_PORTS_MAC(dev):              \
             71)
#define PRV_CPSS_MAPPING_TXQ_2_TXDMA_MAX_TXDMA_PORT_CNS  255

#define PRV_CPSS_MAPPING_TXQ_2_TXDMA_OFFS_CNS          0
#define PRV_CPSS_MAPPING_TXQ_2_TXDMA_LEN_CNS           8

/**
* @internal prvCpssDxChPortMappingTxQPort2TxDMAMapSet function
* @endinternal
*
* @brief   configure TxQ port 2 TxDMA mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] txqNum                   - txq port
* @param[in] txDmaNum                 - txDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssDxChPortMappingTxQPort2TxDMAMapSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 txqNum,
    IN GT_U32 txDmaNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32      regAddr;            /* register's address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);


    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if(CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS == txqNum)
    {
        return GT_OK;
    }

    if (txqNum > PRV_CPSS_MAPPING_TXQ_2_TXDMA_MAX_TXQ_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* need to write the 'local' DMA number */
        rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
            txDmaNum/*global number*/,
            NULL,
            &txDmaNum);/*converted to local number*/
        if (GT_OK != rc )
        {
            return rc;
        }
    }

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.portToDMAMapTable[txqNum];

    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                         PRV_CPSS_MAPPING_TXQ_2_TXDMA_OFFS_CNS,
                                         PRV_CPSS_MAPPING_TXQ_2_TXDMA_LEN_CNS,
                                         txDmaNum);
    if (GT_OK != rc )
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortMappingTxQPort2TxDMAMapGet function
* @endinternal
*
* @brief   get mapping of TxQ port 2 TxDMA
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] txqNum                   - txq port
*
* @param[out] txDmaNumPtr              - txDMA port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingTxQPort2TxDMAMapGet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroupId,
    IN GT_U32  txqNum,
    OUT GT_U32 *txDmaNumPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32      regAddr;            /* register's address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E | CPSS_FALCON_E);


    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if (txqNum > PRV_CPSS_MAPPING_TXQ_2_TXDMA_MAX_TXQ_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.portToDMAMapTable[txqNum];

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                            PRV_CPSS_MAPPING_TXQ_2_TXDMA_OFFS_CNS,
                                            PRV_CPSS_MAPPING_TXQ_2_TXDMA_LEN_CNS,
                                            txDmaNumPtr);
    if (GT_OK != rc )
    {
        return rc;
    }


    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* need to convert the 'local' DMA number to global DMA number */

        /* convert global to dq unit index */
        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(devNum,
            SIP_5_20_GLOBAL_TXQ_DQ_PORT_TO_DQ_UNIT_INDEX_MAC(devNum,txqNum),
            *txDmaNumPtr,/* local DMA */
            txDmaNumPtr);/* converted global DMA */

        if (GT_OK != rc )
        {
            return rc;
        }
    }

    return GT_OK;
}


/*-----------------------------------------------------------------------------*/
/* 6 PFC Response Local Device Source Port to TxQ port map                     */
/*-----------------------------------------------------------------------------*/
#define PRV_CPSS_MAPPING_PHYS_2_TXQ_MAX_PHYS_PORT_CNS(_devNum)\
         (GT_U32)(PRV_CPSS_SIP_5_20_CHECK_MAC(_devNum) ?  511 : 255 )

#define PRV_CPSS_MAPPING_PHYS_2_TXQ_MAX_TXQ_PORT_CNS     127

#define PRV_CPSS_MAPPING_PHYS_2_TXQ_OFFS_CNS          0
#define PRV_CPSS_MAPPING_PHYS_2_TXQ_LEN_CNS           7

/**
* @internal prvCpssDxChPortMappingPFCResponsePhysPort2TxQMapSet function
* @endinternal
*
* @brief   configure PFC response Phys port mapping to txq
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - phys port
* @param[in] txqNum                   - txq port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS prvCpssDxChPortMappingPFCResponsePhysPort2TxQMapSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 physPort,
    IN GT_U32 txqNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32      regAddr;            /* register's address */
    GT_U32      regData;            /* register's data */
    GT_U32      dqIndex = 0;/*dq unit index*/
    GT_U32      first, last;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if (CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS == txqNum)
    {
        return GT_OK;
    }

    if (physPort > PRV_CPSS_MAPPING_PHYS_2_TXQ_MAX_PHYS_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if (txqNum > PRV_CPSS_DXCH_MAPPING_MAX_TXQ_PORT_CNS(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* convert global to dq unit index */
        dqIndex = SIP_5_20_GLOBAL_TXQ_DQ_PORT_TO_DQ_UNIT_INDEX_MAC(devNum,txqNum);
        /* convert global to local */
        txqNum = SIP_5_20_GLOBAL_TXQ_DQ_PORT_TO_LOCAL_TXQ_DQ_PORT_MAC(devNum,txqNum);

        /* save the mapping */
        SIP_5_20_TXQ_DQ_UNIT_INDEX_OF_PORT_MAC(devNum,physPort) = dqIndex;

        first = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dqIndex].pfcResponseFirstPortInRange;
        last  = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dqIndex].pfcResponseLastPortInRange;
        if (first > last)
        {
            /* not initialized unit */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* allow initialization of ALDRIN2 */
        if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
        {
            /* support the 'Ethernet cpu port' (global MAC 72) (in DQ[0]) that hold
               physical ports that is more than the first physical port of DQ[1] */
            if(dqIndex == 0 && physPort > last)
            {
                /* add an extra entry at the table */
                last++;
                regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dqIndex).flowCtrlResponse.ttiToPfcResponseTableEntry;
                regData = ((last - first) << 8)         |   /* Response User FC Port Table Entry */
                            ((physPort << 1) & 0xFE)    |   /* Response User FC Port */
                            (1 << 0);                       /* Response User FC Enabled */
                rc = prvCpssHwPpPortGroupWriteRegister(devNum,
                                                        portGroupId,
                                                        regAddr,
                                                        regData);
                if (GT_OK != rc )
                {
                    return rc;
                }
                /* extra entry will be set up at this index */
                physPort = last;
            }
        }

        if ((physPort >= first) && (physPort <= last))
        {
#if (PRV_CPSS_MAPPING_CHECK_PRINT_CNS == 1)
            cpssOsPrintf(
                "pfcResponse port %d tx_que %d first %d last %d\n",
                physPort, txqNum, first, last);
#endif
            physPort -= first;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    if (txqNum > PRV_CPSS_MAPPING_PHYS_2_TXQ_MAX_TXQ_PORT_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChPortGroupWriteTableEntry(devNum, portGroupId,
            CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E + dqIndex,
            physPort, &txqNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dqIndex).flowCtrlResponse.portToTxQPortMap[physPort];

        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                             PRV_CPSS_MAPPING_PHYS_2_TXQ_OFFS_CNS,
                                             PRV_CPSS_MAPPING_PHYS_2_TXQ_LEN_CNS,
                                             txqNum);
        if (GT_OK != rc )
        {
            return rc;
        }
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChPortMappingPFCResponsePhysPort2TxQMapGet function
* @endinternal
*
* @brief   configure PFC response Phys port mapping to txq
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - phys port
*
* @param[out] txqNumPtr                - txq port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingPFCResponsePhysPort2TxQMapGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 physPort,
    OUT GT_U32 *txqNumPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr;            /* register's address */
    GT_U32      dqIndex = 0;/*dq unit index*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    if (physPort > PRV_CPSS_MAPPING_PHYS_2_TXQ_MAX_PHYS_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(txqNumPtr);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        dqIndex = SIP_5_20_TXQ_DQ_UNIT_INDEX_OF_PORT_MAC(devNum,physPort);

        if(physPort >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dqIndex].pfcResponseFirstPortInRange &&
           physPort <= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dqIndex].pfcResponseLastPortInRange)
        {
            physPort -= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dqIndex].pfcResponseFirstPortInRange;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChPortGroupReadTableEntry(devNum, portGroupId,
            CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E + dqIndex,
            physPort, txqNumPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* convert local to global */
        (*txqNumPtr) = SIP_5_20_LOCAL_TXQ_DQ_PORT_TO_GLOBAL_TXQ_DQ_PORT_MAC(devNum,(*txqNumPtr),dqIndex);

    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dqIndex).flowCtrlResponse.portToTxQPortMap[physPort];

        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                             PRV_CPSS_MAPPING_PHYS_2_TXQ_OFFS_CNS, PRV_CPSS_MAPPING_PHYS_2_TXQ_LEN_CNS, txqNumPtr);
        if (GT_OK != rc )
        {
            return rc;
        }

    }

    return GT_OK;
}


/*-----------------------------------------------------------------------------                */
/* 7 PFC Trigger Local Device Source Port to MAC map                                           */
/*    BC2 : CPFC_IP_SIP6.10 Units/PFC Source Port <%p> to PFC Index Map                        */
/*    BC3 : CPFC_IP TLU/Units/CPFC_IP_SIP6.50 Units/PFC Source Port <%p> to PFC Index Map      */
/*                                                                                             */
/*    Action : "PFC Trigger Local Device Source Port to MAC map"[physPort] = mac.              */
/*                                                                                             */
/*    for BC3 : instead of true mac the following number shall be used (CPSS-5960)             */
/*-----------------------------------------------------------------------------                */
/*  +----+-----------------+--------+-----+--------+
 *  | DP |   mapping Type  | rx-dma | mac |   PFC  |
 *  |    |                 | dp loc |     |   HW   |
 *  +----+-----------------+--------+-----+--------+
 *  |  0 | ETHERNET        |  0   0 |   0 |    0   |
 *  |    | ETHERNET        |  0   1 |   1 |    1   |
 *  |    | ETHERNET        |  0   2 |   2 |    2   |
 *  |    | ETHERNET        |  0   3 |   3 |    3   |
 *  |    | ETHERNET        |  0   4 |   4 |    4   |
 *  |    | ETHERNET        |  0   5 |   5 |    5   |
 *  |    | ETHERNET        |  0   6 |   6 |    6   |
 *  |    | ETHERNET        |  0   7 |   7 |    7   |
 *  |    | ETHERNET        |  0   8 |   8 |    8   |
 *  |    | ETHERNET        |  0   9 |   9 |    9   |
 *  |    | ETHERNET        |  0  10 |  10 |   10   |
 *  |    | ETHERNET        |  0  11 |  11 |   11   |
 *  |    | CPU-SDMA        |  0  12 |     |   --   |   not applicable
 *  +----+-----------------+--------+-----+--------+
 *  |  1 | ETHERNET        |  1   0 |  12 |   13   |
 *  |    | ETHERNET        |  1   1 |  13 |   14   |
 *  |    | ETHERNET        |  1   2 |  14 |   15   |
 *  |    | ETHERNET        |  1   3 |  15 |   16   |
 *  |    | ETHERNET        |  1   4 |  16 |   17   |
 *  |    | ETHERNET        |  1   5 |  17 |   18   |
 *  |    | ETHERNET        |  1   6 |  18 |   19   |
 *  |    | ETHERNET        |  1   7 |  19 |   20   |
 *  |    | ETHERNET        |  1   8 |  20 |   21   |
 *  |    | ETHERNET        |  1   9 |  21 |   22   |
 *  |    | ETHERNET        |  1  10 |  22 |   23   |
 *  |    | ETHERNET        |  1  11 |  23 |   24   |
 *  |    | CPU-SDMA        |  1  12 |  -- |   --   |   not applicable
 *  +----+-----------------+--------+-----+--------+
 *  |  2 | ETHERNET        |  2   0 |  24 |   26   |
 *  |    | ETHERNET        |  2   1 |  25 |   27   |
 *  |    | ETHERNET        |  2   2 |  26 |   28   |
 *  |    | ETHERNET        |  2   3 |  27 |   29   |
 *  |    | ETHERNET        |  2   4 |  28 |   30   |
 *  |    | ETHERNET        |  2   5 |  29 |   31   |
 *  |    | ETHERNET        |  2   6 |  30 |   32   |
 *  |    | ETHERNET        |  2   7 |  31 |   33   |
 *  |    | ETHERNET        |  2   8 |  32 |   34   |
 *  |    | ETHERNET        |  2   9 |  33 |   35   |
 *  |    | ETHERNET        |  2  10 |  34 |   36   |
 *  |    | ETHERNET        |  2  11 |  35 |   37   |
 *  |    | ETHERNET        |  2  12 |  72 |   38   |
 *  +----+-----------------+--------+-----+--------+
 *  |  3 | ETHERNET        |  3   0 |  36 |   39   |
 *  |    | ETHERNET        |  3   1 |  37 |   40   |
 *  |    | ETHERNET        |  3   2 |  38 |   41   |
 *  |    | ETHERNET        |  3   3 |  39 |   42   |
 *  |    | ETHERNET        |  3   4 |  40 |   43   |
 *  |    | ETHERNET        |  3   5 |  41 |   44   |
 *  |    | ETHERNET        |  3   6 |  42 |   45   |
 *  |    | ETHERNET        |  3   7 |  43 |   46   |
 *  |    | ETHERNET        |  3   8 |  44 |   47   |
 *  |    | ETHERNET        |  3   9 |  45 |   48   |
 *  |    | ETHERNET        |  3  10 |  46 |   49   |
 *  |    | ETHERNET        |  3  11 |  47 |   50   |
 *  |    | CPU-SDMA        |  3  12 |  -- |   --   |  not applicable
 *  +----+-----------------+--------+-----+--------+
 *  |  4 | ETHERNET        |  4   0 |  48 |   52   |
 *  |    | ETHERNET        |  4   1 |  49 |   53   |
 *  |    | ETHERNET        |  4   2 |  50 |   54   |
 *  |    | ETHERNET        |  4   3 |  51 |   55   |
 *  |    | ETHERNET        |  4   4 |  52 |   56   |
 *  |    | ETHERNET        |  4   5 |  53 |   57   |
 *  |    | ETHERNET        |  4   6 |  54 |   58   |
 *  |    | ETHERNET        |  4   7 |  55 |   59   |
 *  |    | ETHERNET        |  4   8 |  56 |   60   |
 *  |    | ETHERNET        |  4   9 |  57 |   61   |
 *  |    | ETHERNET        |  4  10 |  58 |   62   |
 *  |    | ETHERNET        |  4  11 |  59 |   63   |
 *  |    | CPU-SDMA        |  4  12 |  -- |   --   |  not applicable
 *  +----+-----------------+--------+-----+--------+
 *  |  5 | ETHERNET        |  5   0 |  60 |   65   |
 *  |    | ETHERNET        |  5   1 |  61 |   66   |
 *  |    | ETHERNET        |  5   2 |  62 |   67   |
 *  |    | ETHERNET        |  5   3 |  63 |   68   |
 *  |    | ETHERNET        |  5   4 |  64 |   69   |
 *  |    | ETHERNET        |  5   5 |  65 |   70   |
 *  |    | ETHERNET        |  5   6 |  66 |   71   |
 *  |    | ETHERNET        |  5   7 |  67 |   72   |
 *  |    | ETHERNET        |  5   8 |  68 |   73   |
 *  |    | ETHERNET        |  5   9 |  69 |   74   |
 *  |    | ETHERNET        |  5  10 |  70 |   75   |
 *  |    | ETHERNET        |  5  11 |  71 |   76   |
 *  |    | ETHERNET        |  5  12 |  73 |   77   |
 *  +----+-----------------+--------+-----+--------+

 *-----------------------------------------------------------------------------*/
#define PRV_CPSS_PFC_COUNTER_IDX_OFFS_CNS    0
#define PRV_CPSS_PFC_COUNTER_IDX_LEN_CNS     4

#define PRV_CPSS_PFC_COUNTER_BLK_OFFS_CNS    4
#define PRV_CPSS_PFC_COUNTER_BLK_LEN_CNS     3

#define PRV_CPSS_PFC_LOCAL_CORE_IDX_OFFS_CNS 7
#define PRV_CPSS_PFC_LOCAL_CORE_IDX_LEN_CNS  3

#define PRV_CPSS_PFC_MAC_IDX_OFFS_CNS        10
#define PRV_CPSS_PFC_MAC_IDX_LEN_CNS         8

typedef struct
{
    GT_U32 mac;
    GT_U32 pfc_hw;
}PRV_CPSS_MAC_2_PFC_HW_STC;

#define BAD_VALUE PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS
static PRV_CPSS_MAC_2_PFC_HW_STC prvCpssBc3Mac2PfcHWTranslTbl[] =
{
    /*+----------------+-------------+------------------+-------------+------------------+-------------+ */
    /*|  dp loc  | mac |   PFC  HW   |    dp loc  | mac |   PFC  HW   |    dp loc  | mac |   PFC  HW   | */
    /*+----------------+-------------+------------------+-------------+------------------+-------------+ */
      {/* 0   0 */   0 ,    0        },{/* 1   0 */  12 ,   13        },{/* 2   0 */  24 ,   26        }
     ,{/* 0   1 */   1 ,    1        },{/* 1   1 */  13 ,   14        },{/* 2   1 */  25 ,   27        }
     ,{/* 0   2 */   2 ,    2        },{/* 1   2 */  14 ,   15        },{/* 2   2 */  26 ,   28        }
     ,{/* 0   3 */   3 ,    3        },{/* 1   3 */  15 ,   16        },{/* 2   3 */  27 ,   29        }
     ,{/* 0   4 */   4 ,    4        },{/* 1   4 */  16 ,   17        },{/* 2   4 */  28 ,   30        }
     ,{/* 0   5 */   5 ,    5        },{/* 1   5 */  17 ,   18        },{/* 2   5 */  29 ,   31        }
     ,{/* 0   6 */   6 ,    6        },{/* 1   6 */  18 ,   19        },{/* 2   6 */  30 ,   32        }
     ,{/* 0   7 */   7 ,    7        },{/* 1   7 */  19 ,   20        },{/* 2   7 */  31 ,   33        }
     ,{/* 0   8 */   8 ,    8        },{/* 1   8 */  20 ,   21        },{/* 2   8 */  32 ,   34        }
     ,{/* 0   9 */   9 ,    9        },{/* 1   9 */  21 ,   22        },{/* 2   9 */  33 ,   35        }
     ,{/* 0  10 */  10 ,   10        },{/* 1  10 */  22 ,   23        },{/* 2  10 */  34 ,   36        }
     ,{/* 0  11 */  11 ,   11        },{/* 1  11 */  23 ,   24        },{/* 2  11 */  35 ,   37        }
                                                                       ,{/* 2  12 */  72 ,   38        }
    /*+----------------+-------------+------------------+-------------+------------------+-------------+ */
     ,{/* 3   0 */  36 ,   39        },{/* 4   0 */  48 ,   52        },{/* 5   0 */  60 ,   65        }
     ,{/* 3   1 */  37 ,   40        },{/* 4   1 */  49 ,   53        },{/* 5   1 */  61 ,   66        }
     ,{/* 3   2 */  38 ,   41        },{/* 4   2 */  50 ,   54        },{/* 5   2 */  62 ,   67        }
     ,{/* 3   3 */  39 ,   42        },{/* 4   3 */  51 ,   55        },{/* 5   3 */  63 ,   68        }
     ,{/* 3   4 */  40 ,   43        },{/* 4   4 */  52 ,   56        },{/* 5   4 */  64 ,   69        }
     ,{/* 3   5 */  41 ,   44        },{/* 4   5 */  53 ,   57        },{/* 5   5 */  65 ,   70        }
     ,{/* 3   6 */  42 ,   45        },{/* 4   6 */  54 ,   58        },{/* 5   6 */  66 ,   71        }
     ,{/* 3   7 */  43 ,   46        },{/* 4   7 */  55 ,   59        },{/* 5   7 */  67 ,   72        }
     ,{/* 3   8 */  44 ,   47        },{/* 4   8 */  56 ,   60        },{/* 5   8 */  68 ,   73        }
     ,{/* 3   9 */  45 ,   48        },{/* 4   9 */  57 ,   61        },{/* 5   9 */  69 ,   74        }
     ,{/* 3  10 */  46 ,   49        },{/* 4  10 */  58 ,   62        },{/* 5  10 */  70 ,   75        }
     ,{/* 3  11 */  47 ,   50        },{/* 4  11 */  59 ,   63        },{/* 5  11 */  71 ,   76        }
                                                                       ,{/* 5  12 */  73 ,   77        }
    /*+----------------+-------------+------------------+-------------+------------------+-------------+ */
     ,{      BAD_VALUE ,   BAD_VALUE }
};
/*NOTE: ALDRIN2 : per MAC numbers there is same index as for BC3 */
static PRV_CPSS_MAC_2_PFC_HW_STC prvCpssAldrin2Mac2PfcHWTranslTbl[] =
{
    /*+----------------+-------------+------------------+-------------+------------------+-------------+ */
    /*|  dp loc  | mac |   PFC  HW   |    dp loc  | mac |   PFC  HW   |    dp loc  | mac |   PFC  HW   | */
    /*+----------------+-------------+------------------+-------------+------------------+-------------+ */
      {/* 0   0 */   0 ,    0        },{/* 1   0 */  12 ,   13        },{/* 2   0 */  24 ,   26        }
     ,{/* 0   1 */   1 ,    1        },{/* 1   1 */  13 ,   14        },{/* 2   1 */  25 ,   27        }
     ,{/* 0   2 */   2 ,    2        },{/* 1   2 */  14 ,   15        },{/* 2   2 */  26 ,   28        }
     ,{/* 0   3 */   3 ,    3        },{/* 1   3 */  15 ,   16        },{/* 2   3 */  27 ,   29        }
     ,{/* 0   4 */   4 ,    4        },{/* 1   4 */  16 ,   17        },{/* 2   4 */  28 ,   30        }
     ,{/* 0   5 */   5 ,    5        },{/* 1   5 */  17 ,   18        },{/* 2   5 */  29 ,   31        }
     ,{/* 0   6 */   6 ,    6        },{/* 1   6 */  18 ,   19        },{/* 2   6 */  30 ,   32        }
     ,{/* 0   7 */   7 ,    7        },{/* 1   7 */  19 ,   20        },{/* 2   7 */  31 ,   33        }
     ,{/* 0   8 */   8 ,    8        },{/* 1   8 */  20 ,   21        },{/* 2   8 */  32 ,   34        }
     ,{/* 0   9 */   9 ,    9        },{/* 1   9 */  21 ,   22        },{/* 2   9 */  33 ,   35        }
     ,{/* 0  10 */  10 ,   10        },{/* 1  10 */  22 ,   23        },{/* 2  10 */  34 ,   36        }
     ,{/* 0  11 */  11 ,   11        },{/* 1  11 */  23 ,   24        },{/* 2  11 */  35 ,   37        }
     ,{/* 0  24 */  72 ,   38        }
    /*+----------------+-------------+------------------+-------------+------------------+-------------+ */
     ,{/* 2  12 */  36 ,   39        },{/* 3   0 */  48 ,   52        },{/* 3  12 */  60 ,   65        }
     ,{/* 2  13 */  37 ,   40        },{/* 3   1 */  49 ,   53        },{/* 3  13 */  61 ,   66        }
     ,{/* 2  14 */  38 ,   41        },{/* 3   2 */  50 ,   54        },{/* 3  14 */  62 ,   67        }
     ,{/* 2  15 */  39 ,   42        },{/* 3   3 */  51 ,   55        },{/* 3  15 */  63 ,   68        }
     ,{/* 2  16 */  40 ,   43        },{/* 3   4 */  52 ,   56        },{/* 3  16 */  64 ,   69        }
     ,{/* 2  17 */  41 ,   44        },{/* 3   5 */  53 ,   57        },{/* 3  17 */  65 ,   70        }
     ,{/* 2  18 */  42 ,   45        },{/* 3   6 */  54 ,   58        },{/* 3  18 */  66 ,   71        }
     ,{/* 2  19 */  43 ,   46        },{/* 3   7 */  55 ,   59        },{/* 3  19 */  67 ,   72        }
     ,{/* 2  20 */  44 ,   47        },{/* 3   8 */  56 ,   60        },{/* 3  20 */  68 ,   73        }
     ,{/* 2  21 */  45 ,   48        },{/* 3   9 */  57 ,   61        },{/* 3  21 */  69 ,   74        }
     ,{/* 2  22 */  46 ,   49        },{/* 3  10 */  58 ,   62        },{/* 3  22 */  70 ,   75        }
     ,{/* 2  23 */  47 ,   50        },{/* 3  11 */  59 ,   63        },{/* 3  23 */  71 ,   76        }

    /*+----------------+-------------+------------------+-------------+------------------+-------------+ */
     ,{      BAD_VALUE ,   BAD_VALUE }
};

/**
* @internal prvCpssDxChPortMappingPFCTriggerPhysPort2MACMapSet function
* @endinternal
*
* @brief   configure PFC trigger Phys port mapping to mac number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - phys port
* @param[in] macNum                   - mac port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS prvCpssDxChPortMappingPFCTriggerPhysPort2MACMapSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 physPort,
    IN GT_U32 macNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32      regAddr;            /* register's address */
    PRV_CPSS_MAC_2_PFC_HW_STC   *pfcMappPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCSourcePortToPFCIndexMap[physPort];

    rc = prvCpssHwPpPortGroupSetRegField(devNum, 0, regAddr,
                                             PRV_CPSS_PFC_LOCAL_CORE_IDX_OFFS_CNS, PRV_CPSS_PFC_LOCAL_CORE_IDX_LEN_CNS, portGroupId);
    if (GT_OK != rc )
    {
        return rc;
    }

    /*---------------------------------------------*
     *  write to field PFC_MAC_ID_%p bits 10-17    *
     *---------------------------------------------*/
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) /* (CPSS-5960)  convert mac to HW values */
    {
        pfcMappPtr = &prvCpssBc3Mac2PfcHWTranslTbl[0];
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* currently Falcon as BC3 */
        CPSS_TBD_BOOKMARK_FALCON
        pfcMappPtr = &prvCpssBc3Mac2PfcHWTranslTbl[0];
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        pfcMappPtr = &prvCpssAldrin2Mac2PfcHWTranslTbl[0];
    }
    else
    {
        pfcMappPtr = NULL;
    }

    if(pfcMappPtr)
    {
        GT_U32 i;
        GT_BOOL found;
        found = GT_FALSE;

        for (i = 0 ; pfcMappPtr[i].mac != BAD_VALUE; i++)
        {
            if (pfcMappPtr[i].mac == macNum)
            {
                found = GT_TRUE;
                break;
            }
        }
        if (found == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (pfcMappPtr[i].pfc_hw == BAD_VALUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        macNum = pfcMappPtr[i].pfc_hw;
    }
    regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCSourcePortToPFCIndexMap[physPort];

    rc = prvCpssHwPpPortGroupSetRegField(devNum, 0, regAddr,
                                             PRV_CPSS_PFC_MAC_IDX_OFFS_CNS, PRV_CPSS_PFC_MAC_IDX_LEN_CNS, macNum);
    if (GT_OK != rc )
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMappingPFCTriggerPhysPort2MACMapGet function
* @endinternal
*
* @brief   configure PFC trigger Phys port mapping to mac number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] physPort                 - phys port
*
* @param[out] macNumPtr                - mac port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingPFCTriggerPhysPort2MACMapGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 physPort,
    OUT GT_U32 *macNumPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);
    CPSS_NULL_PTR_CHECK_MAC(macNumPtr);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    *macNumPtr = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCSourcePortToPFCIndexMap[physPort];

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                             PRV_CPSS_PFC_MAC_IDX_OFFS_CNS, PRV_CPSS_PFC_MAC_IDX_LEN_CNS, macNumPtr);
    if (GT_OK != rc )
    {
        return rc;
    }



    return GT_OK;
}

/*-----------------------------------------------------------------------------*/
/* 9 CPU TxQ port number                                                       */
/*-----------------------------------------------------------------------------*/
#define PRV_CPSS_MAPPING_CPU_PORT_EN_OFFS_CNS  0
#define PRV_CPSS_MAPPING_CPU_PORT_EN_LEN_CNS   1
#define PRV_CPSS_MAPPING_CPU_TXQ_PORT_OFFS_CNS 1
#define PRV_CPSS_MAPPING_CPU_TXQ_PORT_LEN_CNS  7

/**
* @internal prvCpssDxChPortMappingCPU_2_TxQPortSet function
* @endinternal
*
* @brief   configure CPU txq number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] cpuNum                   - cpu port
*
* @param[out] txqNum                   - txq number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingCPU_2_TxQPortSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 cpuNum,
    IN GT_U32 txqNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr;
    GT_U32    dpIdx,localTxQNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_UNUSED_PARAM_MAC(cpuNum);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    rc = prvCpssDxChPpResourcesTxqGlobal2LocalConvert(devNum,txqNum,/*OUT*/&dpIdx,&localTxQNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dpIdx).global.globalDQConfig.globalDequeueConfig;

    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                             PRV_CPSS_MAPPING_CPU_TXQ_PORT_OFFS_CNS,
                                             PRV_CPSS_MAPPING_CPU_TXQ_PORT_LEN_CNS,
                                             localTxQNum);
    if (GT_OK != rc )
    {
        return rc;
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChPortMappingCPU_2_TxQPortGet function
* @endinternal
*
* @brief   configure CPU txq number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] cpuNum                   - cpu port
*
* @param[out] txqNumPtr                - txq number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingCPU_2_TxQPortGet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroupId,
    IN GT_U32  cpuNum,
   OUT GT_U32 *txqNumPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    dpIdx,localTxQNum;
    GT_U32    hwTxQNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_UNUSED_PARAM_MAC(cpuNum);

    rc =  prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,cpuNum, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPpResourcesTxqGlobal2LocalConvert(devNum,portMapShadowPtr->portMap.txqNum,/*OUT*/&dpIdx,&localTxQNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    CPSS_NULL_PTR_CHECK_MAC(txqNumPtr);

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dpIdx).global.globalDQConfig.globalDequeueConfig;
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                             PRV_CPSS_MAPPING_CPU_TXQ_PORT_OFFS_CNS,
                                             PRV_CPSS_MAPPING_CPU_TXQ_PORT_LEN_CNS,
                                             /*OUT*/&hwTxQNum);
    if (GT_OK != rc )
    {
        return rc;
    }
    if (hwTxQNum != localTxQNum) /* something bad */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    *txqNumPtr = portMapShadowPtr->portMap.txqNum;

    return GT_OK;
}


/*--------------------------------------------------------------*/
/* Mapping Declaration                                          */
/*--------------------------------------------------------------*/
static PRV_DXCH_CPSS_MAPPING_SET_GET_FUN_STC prv_MappingFunList[] =
{
    /* 1  RX DMA to Local Device Physical Source Port */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "1 RxDMA-rxDma2Phys",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_ETHERNET_MAC_E | PRV_MAPTYPE_CPU_SDMA_E | PRV_MAPTYPE_ILKN_MAC_E,
        /*  GT_U32                             applicableDevice; */ CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E ,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingRxDMA2LocalPhysSet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingRxDMA2LocalPhysGet
    },
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "1 RxDMA-rxDma2Phys (sip6 non CPU SDMA)",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_ETHERNET_MAC_E,
        /*  GT_U32                             applicableDevice; */ CPSS_FALCON_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingRxDMA2LocalPhysSet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingRxDMA2LocalPhysGet
    },
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "1 RxDMA-rxDma2Phys (sip6 CPU SDMA)",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_CPU_SDMA_E,
        /*  GT_U32                             applicableDevice; */ CPSS_FALCON_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingRxDMA2LocalPhysSet_cpuSdma,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingRxDMA2LocalPhysGet
    },
    /* 2 TXDMA Local Device Physical Port to RX DMA number */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "2 TxDMA phys2rxDma",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_ETHERNET_MAC_E  | PRV_MAPTYPE_CPU_SDMA_E | PRV_MAPTYPE_REMOTE_PHYS_PORT_E,
        /*  GT_U32                             applicableDevice; */ CPSS_BOBCAT3_E | CPSS_ALDRIN2_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingCutThroughTxDMALocalPhys2RxDMASet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingCutThroughTxDMALocalPhys2RxDMAGet
    },

    /* 3 BMA map of Local Device Physical Port to RX DMA number */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "3 BMA-phys2rxDma",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_ETHERNET_MAC_E  | PRV_MAPTYPE_CPU_SDMA_E | PRV_MAPTYPE_ILKN_MAC_E | PRV_MAPTYPE_ILKN_CHANNEL_E | PRV_MAPTYPE_REMOTE_PHYS_PORT_E,
        /*  GT_U32                             applicableDevice; */ CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMAGet
    },
    /* 4.1 EGF Target Local Physical Port to TxQ port map */
    /* EGF for sip6 see 4.3 */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "4.1 EGF-phys2txq",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_ETHERNET_MAC_E  | PRV_MAPTYPE_CPU_SDMA_E | PRV_MAPTYPE_ILKN_MAC_E | PRV_MAPTYPE_ILKN_CHANNEL_E | PRV_MAPTYPE_REMOTE_PHYS_PORT_E,
        /*  GT_U32                             applicableDevice; */ CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E /*| CPSS_FALCON_E*/,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_TXQ_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapSet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapGet
    },
    /* 4.2 EGF Target Local Physical Port to MAC port map */
    /* EGF for sip6 see 4.3 */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "4.2 EGF-phys2mac",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_ETHERNET_MAC_E  | PRV_MAPTYPE_CPU_SDMA_E | PRV_MAPTYPE_ILKN_MAC_E | PRV_MAPTYPE_ILKN_CHANNEL_E | PRV_MAPTYPE_REMOTE_PHYS_PORT_E,
        /*  GT_U32                             applicableDevice; */ CPSS_BOBCAT2_E | CPSS_CAELUM_E /*| CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E removed from sip_5_20*/,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_MAC_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingEGFTargetLocalPhys2MACNumMapSet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingEGFTargetLocalPhys2MACNumMapGet
    },
    /* 4.3 (sip6 only) EGF Target Local Physical Port to <Port Base> */
    /*
    NOTE: we have different set of functions for SIP6 to show that the TXQ 'queue' is
       NOT defined in the EGF ... the 'queue' is only known at the TXQ after mapping
       <port base> to it.
    */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "4.3 EGF-phys2PortBase",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_ETHERNET_MAC_E  | PRV_MAPTYPE_CPU_SDMA_E | PRV_MAPTYPE_REMOTE_PHYS_PORT_E,
        /*  GT_U32                             applicableDevice; */ CPSS_FALCON_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,/* MAP 1:1 because TXQ hold mapping from <port base> to actual TXQ */
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapSet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapGet
    },
    /* 5 TxQ port to TX DMA map */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "5 TXQ-rxq2txDma",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_ETHERNET_MAC_E  | PRV_MAPTYPE_CPU_SDMA_E | PRV_MAPTYPE_ILKN_MAC_E | PRV_MAPTYPE_ILKN_CHANNEL_E | PRV_MAPTYPE_REMOTE_PHYS_PORT_E,
        /*  GT_U32                             applicableDevice; */ CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E /*| CPSS_FALCON_E*/,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_TXQ_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingTxQPort2TxDMAMapSet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingTxQPort2TxDMAMapGet
    },

    /* 6.1 PFC Response Local Device Source Port to TxQ port map: BC2 and Caelum */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "6.1 PFCResp-phys22txq",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_ETHERNET_MAC_E  | PRV_MAPTYPE_CPU_SDMA_E | PRV_MAPTYPE_ILKN_MAC_E | PRV_MAPTYPE_ILKN_CHANNEL_E | PRV_MAPTYPE_REMOTE_PHYS_PORT_E,
        /*  GT_U32                             applicableDevice; */ CPSS_BOBCAT2_E | CPSS_CAELUM_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_TXQ_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingPFCResponsePhysPort2TxQMapSet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingPFCResponsePhysPort2TxQMapGet
    },

    /* 6.2 PFC Response Local Device Source Port to TxQ port map: BC3 */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "6.2 PFCResp-phys22txq",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_ETHERNET_MAC_E  | PRV_MAPTYPE_ILKN_MAC_E | PRV_MAPTYPE_ILKN_CHANNEL_E | PRV_MAPTYPE_REMOTE_PHYS_PORT_E,
        /*  GT_U32                             applicableDevice; */ CPSS_BOBCAT3_E | CPSS_ALDRIN2_E /*| CPSS_FALCON_E */,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_TXQ_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingPFCResponsePhysPort2TxQMapSet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingPFCResponsePhysPort2TxQMapGet
    },

    /* 7 PFC Trigger Local Device Source Port to MAC map */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "7 PFCTrig-phys2mac",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_ETHERNET_MAC_E  |  PRV_MAPTYPE_ILKN_MAC_E | PRV_MAPTYPE_ILKN_CHANNEL_E | PRV_MAPTYPE_REMOTE_PHYS_PORT_E,
        /*  GT_U32                             applicableDevice; */ CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E /*| CPSS_FALCON_E*/,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_MAC_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingPFCTriggerPhysPort2MACMapSet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingPFCTriggerPhysPort2MACMapGet
    },
    /* 9 CPU TxQ port number  */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  "9 CPU-txq",
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ PRV_MAPTYPE_CPU_SDMA_E,
        /*  GT_U32                             applicableDevice; */ CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E /*| CPSS_FALCON_E*/,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_PHYS_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_TXQ_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ prvCpssDxChPortMappingCPU_2_TxQPortSet,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ prvCpssDxChPortMappingCPU_2_TxQPortGet
    },
    /* Last Mapping : Empty Mapping   */
    {
        #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        /* GT_CHAR                            stringMapType[20];*/  NULL,
        #endif
        /*  GT_U32                             mapingTypeBmp;    */ 0,
        /*  GT_U32                             applicableDevice; */ 0,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        srcPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_INVALID_E,
        /*  PRV_CPSS_DXCH_PORT_TYPE_ENT        dstPortType;      */ PRV_CPSS_DXCH_PORT_TYPE_INVALID_E,
        /*  PRV_DXCH_CPSS_MAPPING_SET_FUN_PTR  setFun;           */ NULL,
        /*  PRV_DXCH_CPSS_MAPPING_GET_FUN_PTR  getFun;           */ NULL
    }
};


/**
* @internal prvCpssDxChPortMappingTypeCheck function
* @endinternal
*
* @brief   basic mapping check for HW feature
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device
* @param[in] portMapPtr               - pointer to mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingTypeCheck
(
    IN GT_U8                        devNum,
    IN CPSS_DXCH_PORT_MAP_STC      *portMapPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(portMapPtr);
    if (portMapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E && PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_ilkn.supported == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /*
    if (portMapPtr->tmEnable == GT_TRUE && PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.TmSupported == GT_FALSE)
    {
        return GT_BAD_PARAM;
    }
    */
    return GT_OK;
}

/**
* @internal prvCpssDxChPortMappingTypeGet function
* @endinternal
*
* @brief   build mapping type in internal format (bit coded)
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] systemMappingType        - system mapping type
*
* @param[out] prvMappingTypePtr        - internal mapping type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingTypeGet
(
    IN  CPSS_DXCH_PORT_MAPPING_TYPE_ENT  systemMappingType,
    OUT PRV_MAPTYPE_ENT                 *prvMappingTypePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(prvMappingTypePtr);

    switch (systemMappingType)
    {
        case CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E:
            *prvMappingTypePtr = PRV_MAPTYPE_ETHERNET_MAC_E;
        break;
        case CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E:
            *prvMappingTypePtr = PRV_MAPTYPE_CPU_SDMA_E    ;
        break;
        case CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E:
            *prvMappingTypePtr = PRV_MAPTYPE_ILKN_CHANNEL_E;
        break;
        case CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E:
            *prvMappingTypePtr = PRV_MAPTYPE_REMOTE_PHYS_PORT_E;
        break;
        default:
        {
            *prvMappingTypePtr = PRV_MAPTYPE_INVALID_E;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}


/**
* @internal prvCpsssDxChPortMappingDeviceTypeGet function
* @endinternal
*
* @brief   build device type in bit coded style
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devFamily                - device family
*
* @param[out] devTypePtr               device type in bit coded style
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpsssDxChPortMappingDeviceTypeGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PP_FAMILY_TYPE_ENT devFamily,
    IN  CPSS_PP_SUB_FAMILY_TYPE_ENT devSubFamily,
    OUT CPSS_PP_FAMILY_BIT_ENT *devTypePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(devTypePtr);

    switch (devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if(devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                *devTypePtr = CPSS_CAELUM_E;
            }
            else
            {
                *devTypePtr = CPSS_BOBCAT2_E;
            }
        break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            *devTypePtr = CPSS_CAELUM_E;
        break;
        default:
            *devTypePtr = PRV_CPSS_PP_MAC(devNum)->appDevFamily;
            break;
    }
    return GT_OK;
}

/*-------------------------------------*/
/* check function */
/*-------------------------------------*/
/**
* @internal prvCpssDxChCheckDMAPort function
* @endinternal
*
* @brief   check DMA Port range
*
* @note   APPLICABLE DEVICES:      Bobcat2
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device family
* @param[in] dmaNum                   - dma num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on error in params
*/
static GT_STATUS prvCpssDxChCheckDMAPort
(
    IN GT_U8  devNum,
    IN GT_U32 dmaNum
)
{

    /* the 'Regular DMA' that is not CPU/ILKN is actually 'MAC number' that
       exists in the 'Existing ports' (in valid range of MACs) */
    PRV_CPSS_PHY_PORT_CHECK_MAC(devNum,dmaNum);

    return GT_OK;
}


/**
* @internal prvCpssDxChCheckTxQPort function
* @endinternal
*
* @brief   check txq Port range
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device family
* @param[in] macNum                   - interface number
* @param[in] txqNum                   - txq num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on error in params
*/
static GT_STATUS prvCpssDxChCheckTxQPort
(
    IN GT_U8  devNum,
    IN GT_U32 macNum,
    IN GT_U32 txqNum
)
{
    GT_STATUS   rc;
    GT_U32  dpIndex;/* DP[] number that the macNum belongs to */
    GT_U32  dqIndex;/* DQ[] number that the txqNum belongs to */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    if(txqNum == CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
    {
        return GT_OK;
    }

    if (txqNum > PRV_CPSS_DXCH_MAPPING_MAX_TXQ_PORT_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_TRUE &&
        macNum != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
    {
        /* convert the MAC to DP */
        rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(
            devNum,
            macNum,/* macNum is the same as globalDmaNum */
            &dpIndex,
            NULL/*localDmaNumPtr*/);
        if(rc != GT_OK)
        {
            return rc;
        }

        dqIndex = txqNum / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;

        /* check that DQ[] and DP[] are the same index */
        if(dpIndex != dqIndex)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "macNum[%d] belongs to DP[%d] but got txqNum[%d] that belongs to DQ[%d] \n",
                macNum,dpIndex,txqNum,dqIndex);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCheckIlknChannelPort function
* @endinternal
*
* @brief   check Interlaken channel Port range
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] ilknChannel              - Interlaken channel num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on error in params
*/
static GT_STATUS prvCpssDxChCheckIlknChannelPort
(
    IN GT_U32 ilknChannel
)
{
    if (ilknChannel >=
            PRV_CPSS_DXCH_MAPPING_BOBCAT2_ILKN_CHANNEL_MAX_PHYS_PORT_DIFF_CNS )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/*-----------------------------------------------------*/
#define PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS         1024 /*512*/

#define SIP6_MAX_TXQ_CNS    _8K
/**
* @struct PRV_CPSS_DXCH_PORT_MAPPING_STC
 *
 * @brief general port map structure used for many-2-one check
*/
typedef struct{

    /** device number */
    GT_U8 devNum;

    GT_BOOL isPhysPortUsed         [PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS];

    GT_BOOL physIsEqualt2OldMapping[PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS];

    GT_U32 rxDma2PhysMapArr       [PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS];

    GT_U32 phys2rxDmaMapArr       [PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS];

    GT_U32 phys2txDmaMapArr       [PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS];

    GT_U32 phys2macMapArr         [PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS];

    GT_U32 phys2txqMapArr         [PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS];

    GT_U32    txq2txDmaMapArr        [SIP6_MAX_TXQ_CNS];

    GT_U32 tm2physMapArr          [PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS];

    GT_U32 Ilkn2physMapArr        [PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS];

    /** min interlaken physical port */
    GT_U32 minIlknChnnalPhysPort;

    /** @brief max interlaken physical port, the difference shall be less than 64
     *  Comments:
     */
    GT_U32 maxIlknChnnalPhysPort;

    GT_BOOL isMacExtendedCascadePort [PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS];

    GT_BOOL isPhysExtendedCascadePort[PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS];

} PRV_CPSS_DXCH_PORT_MAPPING_STC;


/**
* @internal prvCpsssDxChPortMappingArgumentsBuild function
* @endinternal
*
* @brief   build argumenst for mappings (from user format into internal format) based on
*         mapping flag.
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
* @param[in] portMapPtr               - pointer to array of mappings
*
* @param[out] portArg[PRV_CPSS_DXCH_PORT_TYPE_MAX_E] - argumenst array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_BAD_PTR               - on bad pointer
*
* @note takes into account specific settings like
*       1. MAC number is equal to RxDMA port in all cases.
*       2. RxDMA port = TxDMA port in all cases exept for TM is enabled
*       TxDMA port (73) for traffic manager if TM flag is true
*       3. for CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E :   Rx/Tx DMA port is set to 72
*       4. for CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E : Rx/Tx DMA port is set to 64
*
*/
static GT_STATUS prvCpsssDxChPortMappingArgumentsBuild
(
     IN GT_U8                    devNum,
     IN CPSS_DXCH_PORT_MAP_STC  *portMapPtr,
     OUT GT_U32  portArg[PRV_CPSS_DXCH_PORT_TYPE_MAX_E]
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portMapPtr->physicalPortNumber);
    CPSS_NULL_PTR_CHECK_MAC(portMapPtr);
    CPSS_NULL_PTR_CHECK_MAC(portArg);

    if (GT_TRUE == portMapPtr->tmEnable)
    {
        if(CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E == portMapPtr->mappingType)
        {
            if((portMapPtr->tmPortInd < PRV_CPSS_DXCH_MAPPING_BOBCAT2_TM_PORT_NUM_HIGH_RANGE_START_CNS)
               || (portMapPtr->tmPortInd > PRV_CPSS_DXCH_MAPPING_BOBCAT2_TM_PORT_NUM_HIGH_RANGE_MAX_CNS))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            if (portMapPtr->tmPortInd > PRV_CPSS_DXCH_MAPPING_ETH_MAX_TM_PORT_IDX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (portMapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            /* the SDMA not hold MAC number , check only txqPortNumber */
            rc = prvCpssDxChCheckTxQPort(devNum, CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS, portMapPtr->txqPortNumber);
            if (GT_OK != rc )
            {
                return rc;
            }
        }
        else
        {
            rc = prvCpssDxChCheckTxQPort(devNum, portMapPtr->interfaceNum, portMapPtr->txqPortNumber);
            if (GT_OK != rc )
            {
                return rc;
            }
        }
    }

    /* clear mapping ports */
    portArg[PRV_CPSS_DXCH_PORT_TYPE_PHYS_E          ] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    portArg[PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E         ] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    portArg[PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E         ] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    portArg[PRV_CPSS_DXCH_PORT_TYPE_MAC_E           ] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    portArg[PRV_CPSS_DXCH_PORT_TYPE_TXQ_E           ] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    portArg[PRV_CPSS_DXCH_PORT_TYPE_ILKN_CHANNEL_E  ] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    portArg[PRV_CPSS_DXCH_PORT_TYPE_TM_PORT_IDX_E   ] = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;

    /* now compute mapping */
    portArg[PRV_CPSS_DXCH_PORT_TYPE_PHYS_E] = (GT_U32)portMapPtr->physicalPortNumber;
    if (portMapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
    {
        GT_U32  dmaCpuPort;

        rc = prvCpssDxChHwPpSdmaCpuPortReservedDmaIdxGet(devNum,
            PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? portMapPtr->interfaceNum :
                                               portMapPtr->txqPortNumber,
            /*OUT*/&dmaCpuPort);

        if ((rc != GT_OK) || (dmaCpuPort == CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS))
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "did not find cpu DMA matching Port number[%d] ",portMapPtr->physicalPortNumber);
        }

        portArg[PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E ] = dmaCpuPort;
        portArg[PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E ] = dmaCpuPort;

        rc = prvCpssDxChHwPpSdmaCpuPortReservedMacIdxGet(devNum, portMapPtr->physicalPortNumber, /*OUT*/&portArg[PRV_CPSS_DXCH_PORT_TYPE_MAC_E   ]);
        if (rc != GT_OK)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "did not find cpu mac matching Port number[%d] ",portMapPtr->physicalPortNumber);
        }
    }
    else if (portMapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E)
    {
            portArg[PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E ] = PRV_CPSS_DXCH_PORT_NUM_ILKN_DMA_CNS;
            portArg[PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E ] = PRV_CPSS_DXCH_PORT_NUM_ILKN_DMA_CNS;
            portArg[PRV_CPSS_DXCH_PORT_TYPE_MAC_E   ] = PRV_CPSS_DXCH_PORT_NUM_ILKN_MAC_CNS;

            if( GT_TRUE == portMapPtr->tmEnable )
            {
                rc = prvCpssDxChCheckIlknChannelPort(portMapPtr->interfaceNum);
                if (GT_OK != rc )
                {
                    return rc;
                }

                if (   portMapPtr->tmPortInd <  PRV_CPSS_DXCH_MAPPING_ILKN_MIN_TM_PORT_IDX_CNS
                    || portMapPtr->tmPortInd >  PRV_CPSS_DXCH_MAPPING_ILKN_MAX_TM_PORT_IDX_CNS)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                }


                portArg[PRV_CPSS_DXCH_PORT_TYPE_ILKN_CHANNEL_E] = portMapPtr->interfaceNum;
            }
            else
            {
                /*only channel 0 is supported without TM*/
                if(portMapPtr->interfaceNum == 0)
                {
                    portArg[PRV_CPSS_DXCH_PORT_TYPE_ILKN_CHANNEL_E] = portMapPtr->interfaceNum;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
    }
    else
    {
        if (portMapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
            rc = prvCpssDxChCheckDMAPort(devNum,portMapPtr->interfaceNum);
            if (GT_OK != rc )
            {
                return rc;
            }
        }
        if( GT_TRUE == portMapPtr->tmEnable )
        {
            if (portMapPtr->tmPortInd >  PRV_CPSS_DXCH_MAPPING_ETH_MAX_TM_PORT_IDX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
        }
        portArg[PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E ] = (GT_U32)portMapPtr->interfaceNum;
        portArg[PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E ] = (GT_U32)portMapPtr->interfaceNum;
        portArg[PRV_CPSS_DXCH_PORT_TYPE_MAC_E   ] = (GT_U32)portMapPtr->interfaceNum;
    }
    portArg[PRV_CPSS_DXCH_PORT_TYPE_TXQ_E ] = (GT_U32)portMapPtr->txqPortNumber;

    /* TM enable - Tx-DMA/TxQ related configuration */
    if (GT_TRUE == portMapPtr->tmEnable)
    {
        portArg[PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E      ] = PRV_CPSS_DXCH_PORT_NUM_TM_TxDMA_CNS;
        portArg[PRV_CPSS_DXCH_PORT_TYPE_TXQ_E        ] = PRV_CPSS_DXCH_PORT_NUM_TM_TxQ_CNS;
        portArg[PRV_CPSS_DXCH_PORT_TYPE_TM_PORT_IDX_E] = portMapPtr->tmPortInd;
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChPortPhysicalPortTmMapping function
* @endinternal
*
* @brief   Configure TM realted port mappings.
*
* @param[in] devNum                   - device number
* @param[in] portMapPtr               - pointer to array of mappings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortPhysicalPortTmMapping
(
     IN GT_U8                    devNum,
     IN CPSS_DXCH_PORT_MAP_STC  *portMapPtr
)
{
    GT_STATUS rc;                         /* return value */

                                          /* table type definition */
    CPSS_DXCH_TABLE_ENT tableTypeArr[2] = {CPSS_DXCH_TABLE_LAST_E, CPSS_DXCH_TABLE_LAST_E};
    GT_U32    regAddr;                    /* register address */
    GT_U32    regData;                    /* register data */
    GT_U32    bitOffset;                  /* data offset in register*/
    GT_U32    i;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.trafficManager.supported == GT_TRUE)
    {
       /* TM enable - Physical port enabling */
       regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                    TMDROP.portsInterface[(portMapPtr->physicalPortNumber)/32];
       rc = prvCpssHwPpSetRegField(devNum, regAddr,
                                (portMapPtr->physicalPortNumber)%32, 1,
                                BOOL2BIT_MAC(portMapPtr->tmEnable));
       if (GT_OK != rc )
       {
           return rc;
       }

       /* TM port interface mapping */
       if( (CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E == portMapPtr->mappingType) ||
           (CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E == portMapPtr->mappingType) )
       {
           if(CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E == portMapPtr->mappingType)
           {
               switch((portMapPtr->interfaceNum)/32)
               {
                    case 0: regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                                                    TM_EGR_GLUE.portInterface0;
                            break;
                    case 1: regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                                                    TM_EGR_GLUE.portInterface1;
                            break;
                    case 2: regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                                                    TM_EGR_GLUE.portInterface2;
                            break;
                    default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
               }

               bitOffset = (portMapPtr->interfaceNum)%32;
           }
           else /* CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E == portMapPtr->mappingType */
           {
               regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                                                    TM_EGR_GLUE.portInterface2;
               bitOffset = 9;
           }

           rc = prvCpssHwPpSetRegField(devNum, regAddr, bitOffset, 1,
                                            BOOL2BIT_MAC(portMapPtr->tmEnable));
           if (GT_OK != rc )
           {
               return rc;
           }
       }

       /* TM enable - Tx DMA related configuration */
       if (GT_TRUE == portMapPtr->tmEnable)
       {
           if( ((portMapPtr->tmPortInd >= PRV_CPSS_DXCH_MAPPING_BOBCAT2_TM_PORT_NUM_LOW_RANGE_MAX_CNS) &&
                (portMapPtr->tmPortInd <  PRV_CPSS_DXCH_MAPPING_BOBCAT2_TM_PORT_NUM_HIGH_RANGE_START_CNS)) ||
                (portMapPtr->tmPortInd >=  PRV_CPSS_DXCH_MAPPING_BOBCAT2_TM_PORT_NUM_HIGH_RANGE_MAX_CNS) )
           {
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
           }

           regData = 0;
           /* TM can be used for Ethernet & Interlaken port type */
           switch(portMapPtr->mappingType)
           {
              case CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E:
                  U32_SET_FIELD_MAC(regData, 7, 1, 0);
                  tableTypeArr[0] = CPSS_DXCH_SIP5_TABLE_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_E;
                  break;

              case CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E:
                  U32_SET_FIELD_MAC(regData, 7, 1, 1);
                  tableTypeArr[0] = CPSS_DXCH_SIP5_TABLE_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_E;
                  break;

              default:
                  CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
           }

           U32_SET_FIELD_MAC(regData, 0, 7, portMapPtr->interfaceNum);

           /* Set TM port to physical port or interlaken channel mapping */
           rc = prvCpssDxChWriteTableEntryField
                            (devNum,
                            CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_TARGET_INTERFACE_E,
                            (portMapPtr->tmPortInd)/4,
                            0,
                            ((portMapPtr->tmPortInd)%4)*8,
                            8,
                            regData);

           if( GT_OK != rc )
           {
               return rc;
           }

           /* for bobk, ILKN mapping is used low ports, and Eth mapping is used for high ports  */
           if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E &&
              PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
           {
               tableTypeArr[1] = CPSS_DXCH_SIP5_TABLE_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_E;
           }

           /* Set physical port or interlaken channel to TM port mapping */
           for (i = 0; i < 2; i++)
           {

               if(tableTypeArr[i] != CPSS_DXCH_TABLE_LAST_E)
               {
                   rc = prvCpssDxChWriteTableEntryField
                                (devNum,
                                 tableTypeArr[i],
                                 (portMapPtr->interfaceNum)/4,
                                 0,
                                 ((portMapPtr->interfaceNum)%4)*8,
                                 8,
                                 portMapPtr->tmPortInd);
               }

               if( GT_OK != rc )
               {
                   return rc;
               }
           }
       }
    }
    else /* no TM */
    {
        if (GT_TRUE == portMapPtr->tmEnable)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMappingInit function
* @endinternal
*
* @brief   Init mapping structure to default (no mapping)
*
* @note   APPLICABLE DEVICES:     DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] portMappingPtr           - pointer to mappings structure
* @param[in] devNum                   - device number
* @param[in,out] portMappingPtr           - pointer to mappings structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssDxChPortMappingInit
(
    INOUT PRV_CPSS_DXCH_PORT_MAPPING_STC *portMappingPtr,
       IN GT_U8                           devNum
)
{
    GT_U32    i;

    CPSS_NULL_PTR_CHECK_MAC(portMappingPtr);

    portMappingPtr->devNum = devNum;
    for (i = 0 ; i < PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS;  i++)
    {
        portMappingPtr->isPhysPortUsed           [i] = GT_FALSE;
        portMappingPtr->physIsEqualt2OldMapping  [i] = GT_FALSE;
        portMappingPtr->rxDma2PhysMapArr         [i] = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
        portMappingPtr->phys2rxDmaMapArr         [i] = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
        portMappingPtr->phys2txDmaMapArr         [i] = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
        portMappingPtr->phys2macMapArr           [i] = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
        portMappingPtr->phys2txqMapArr           [i] = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
        portMappingPtr->txq2txDmaMapArr          [i] = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
        portMappingPtr->tm2physMapArr            [i] = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
        portMappingPtr->Ilkn2physMapArr          [i] = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
        portMappingPtr->isMacExtendedCascadePort [i] = GT_FALSE;
        portMappingPtr->isPhysExtendedCascadePort[i] = GT_FALSE;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        for (/*i = 0*//*continue*/ ; i < SIP6_MAX_TXQ_CNS;  i++)
        {
            portMappingPtr->txq2txDmaMapArr          [i] = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
        }
    }


    portMappingPtr->minIlknChnnalPhysPort = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
    portMappingPtr->maxIlknChnnalPhysPort = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;


    /* init PFC response ranges */
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        GT_U32 dq;
        for(dq = 0; dq < PRV_CPSS_SIP_5_20_MAX_DP_MAC(devNum); dq++)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseFirstPortInRange =
                0xFFFFFF; /* big positive number more than any physical port number */
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseLastPortInRange = 0;
        }
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChPortMappingCheckAndAdd function
* @endinternal
*
* @brief   Check current mapping and add it
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in,out] portMappingPtr           - pointer to mappings structure
* @param[in] devNum                   - device number
* @param[in] prvMappingType           - private mapping type (first ILKN channel is ILKN MAC channel)
* @param[in] portArgArr[PRV_CPSS_DXCH_PORT_TYPE_MAX_E] - entry of single user mapping
* @param[in,out] portMappingPtr           - pointer to mappings structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/

static GT_STATUS prvCpssDxChPortMappingCheckAndAdd
(
    INOUT PRV_CPSS_DXCH_PORT_MAPPING_STC *portMappingPtr,
       IN GT_U8                           devNum,
       IN PRV_MAPTYPE_ENT                 prvMappingType,
       IN GT_U32                          portArgArr[PRV_CPSS_DXCH_PORT_TYPE_MAX_E]
)
{
    GT_STATUS rc;

    GT_U32 phys  ;
    GT_U32 rxDma ;
    GT_U32 txDma ;
    GT_U32 mac   ;
    GT_U32 txq   ,maxTxq;
    GT_U32 ilknIdx;
    GT_U32 tmIdx;
    GT_BOOL isEqual2OldMapping;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)) /* currently only Bobcat2/3 family is supported */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(portMappingPtr) ;
    CPSS_NULL_PTR_CHECK_MAC(portArgArr);

    if (devNum != portMappingPtr->devNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    phys     = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_PHYS_E ];
    rxDma    = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E];
    txDma    = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E];
    mac      = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_MAC_E  ];
    txq      = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TXQ_E  ];
    ilknIdx  = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_ILKN_CHANNEL_E];
    tmIdx    = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TM_PORT_IDX_E];

    PRV_CPSS_DXCH_PHY_PORT_RESERVED_CHECK_MAC(devNum, phys);

    CPSS_LOG_INFORMATION_MAC(
        "Adding phys %d rxDma %d txDma %d mac %d txq %d ilknIdx %d tmIdx %d",
        phys, rxDma, txDma, mac, txq, ilknIdx, tmIdx);
    if (phys < PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS)
    {
        CPSS_LOG_INFORMATION_MAC(
        "isPhysPortUsed[phys] %d", portMappingPtr->isPhysPortUsed[phys]);
    }
    if (phys < PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS)
    {
        CPSS_LOG_INFORMATION_MAC(
            "physIsEqualt2OldMapping[phys] %d", portMappingPtr->physIsEqualt2OldMapping[phys]);
    }
    if (rxDma < PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS)
    {
        CPSS_LOG_INFORMATION_MAC(
            "rxDma2PhysMapArr[rxDma] %d", portMappingPtr->rxDma2PhysMapArr[rxDma]);
    }
    if (phys < PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS)
    {
        CPSS_LOG_INFORMATION_MAC(
            "phys2rxDmaMapArr[phys] %d", portMappingPtr->phys2rxDmaMapArr[phys]);
    }
    if (phys < PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS)
    {
        CPSS_LOG_INFORMATION_MAC(
            "phys2txDmaMapArr[phys] %d", portMappingPtr->phys2txDmaMapArr[phys]);
    }
    if (phys < PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS)
    {
        CPSS_LOG_INFORMATION_MAC(
            "phys2txqMapArr[phys] %d", portMappingPtr->phys2txqMapArr[phys]);
    }

    maxTxq = PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? SIP6_MAX_TXQ_CNS : PRV_CPSS_DXCH_PORT_NUM_MAPPING_MAX_PORT_CNS;

    if (txq < maxTxq)
    {
        CPSS_LOG_INFORMATION_MAC(
            "txq2txDmaMapArr[txq] %d", portMappingPtr->txq2txDmaMapArr[txq]);
    }
    else
    {
        if(txq != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "txq[%d] >= [%d] (the max)",
                txq,maxTxq);
        }
    }

    /* check that physical is stil not in use */
    if (portMappingPtr->isPhysPortUsed[phys] == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Physical port[%d] already exists in the DB (check for duplications)",phys);
    }

    /* ILKN channel has no DMA port */
    /* Several remote ports could be mapped to same DMA/TXQ */
    if ((prvMappingType != PRV_MAPTYPE_ILKN_CHANNEL_E) &&
        (prvMappingType != PRV_MAPTYPE_CPU_SDMA_E) &&
        (prvMappingType != PRV_MAPTYPE_REMOTE_PHYS_PORT_E))
    {
        if (portMappingPtr->rxDma2PhysMapArr[rxDma] != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS && portMappingPtr->rxDma2PhysMapArr[rxDma] != phys)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if (portMappingPtr->phys2rxDmaMapArr[phys] != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS && portMappingPtr->phys2rxDmaMapArr[phys] != rxDma)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (portMappingPtr->phys2txDmaMapArr[phys] != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS && portMappingPtr->phys2txDmaMapArr[phys] != txDma)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (portMappingPtr->phys2macMapArr[phys] != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS && portMappingPtr->phys2macMapArr[phys] != mac)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (portMappingPtr->phys2txqMapArr[phys] != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS && portMappingPtr->phys2txqMapArr[phys] != txq)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (txq != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
    {   if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)==GT_FALSE)/*SIP 6 ignore txq id field,TXQ  is checked at prvCpssFalconTxqUtilsConvertToDb */
        {
            if (portMappingPtr->txq2txDmaMapArr[txq] != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS && portMappingPtr->txq2txDmaMapArr[txq] != txDma)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    GT_BAD_PARAM, LOG_ERROR_NO_MSG,
                    "txq %d portMappingPtr->txq2txDmaMapArr[txq] %d txDma %d portMappingPtr->txq2txDmaMapArr[txq] %d",
                    txq, portMappingPtr->txq2txDmaMapArr[txq],txDma, portMappingPtr->txq2txDmaMapArr[txq]);
            }
        }
    }

    if (tmIdx != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
    {
        if (portMappingPtr->tm2physMapArr[tmIdx] != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS && portMappingPtr->tm2physMapArr[tmIdx] != phys)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    if (ilknIdx != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
    {
        if (portMappingPtr->Ilkn2physMapArr[ilknIdx] != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS && portMappingPtr->Ilkn2physMapArr[ilknIdx] != phys)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /*---------------------------------------------------------------------*/
    /* test Interlaken channel physical port that shall be smaller than 64 */
    /*---------------------------------------------------------------------*/
    if (prvMappingType == PRV_MAPTYPE_ILKN_CHANNEL_E || prvMappingType == PRV_MAPTYPE_ILKN_MAC_E)
    {
        if (portMappingPtr->minIlknChnnalPhysPort == PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
        {
            portMappingPtr->minIlknChnnalPhysPort = phys;
            portMappingPtr->maxIlknChnnalPhysPort = phys;
        }
        else
        {
            if (phys < portMappingPtr->minIlknChnnalPhysPort)
            {
                portMappingPtr->minIlknChnnalPhysPort = phys;
            }
            else
            {
                if (phys > portMappingPtr->maxIlknChnnalPhysPort)
                {
                    portMappingPtr->maxIlknChnnalPhysPort = phys;
                }
            }
        }
        if (portMappingPtr->maxIlknChnnalPhysPort - portMappingPtr->minIlknChnnalPhysPort > PRV_CPSS_DXCH_MAPPING_BOBCAT2_ILKN_CHANNEL_MAX_PHYS_PORT_DIFF_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, phys, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    isEqual2OldMapping = GT_FALSE;
    if (portMapShadowPtr->valid == GT_TRUE)
    {
        if (/*prvMappingType == portMapShadowPtr->portMap.mappingType */
               mac         == portMapShadowPtr->portMap.macNum
            && rxDma       == portMapShadowPtr->portMap.rxDmaNum
            && txDma       == portMapShadowPtr->portMap.txDmaNum
            && txq         == portMapShadowPtr->portMap.txqNum
            && tmIdx       == portMapShadowPtr->portMap.tmPortIdx
            && ilknIdx     == portMapShadowPtr->portMap.ilknChannel
            )
        {
            isEqual2OldMapping = GT_TRUE;
        }
    }
    /* store mapping */
    portMappingPtr->isPhysPortUsed   [phys] = GT_TRUE;
    portMappingPtr->physIsEqualt2OldMapping[phys] = isEqual2OldMapping;
    if (prvMappingType != PRV_MAPTYPE_REMOTE_PHYS_PORT_E) /* remote physical port mac shall not be used in mapping */
    {
        portMappingPtr->rxDma2PhysMapArr[rxDma] = phys ;
    }
    portMappingPtr->phys2rxDmaMapArr[phys ] = rxDma;
    portMappingPtr->phys2txDmaMapArr[phys ] = txDma;
    portMappingPtr->phys2macMapArr  [phys ] = mac;
    portMappingPtr->phys2txqMapArr  [phys ] = txq;
    if (txq != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
    {
        portMappingPtr->txq2txDmaMapArr [txq  ] = txDma;
    }

    if (tmIdx != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
    {
        portMappingPtr->tm2physMapArr   [tmIdx]   = phys;
    }
    if (ilknIdx != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
    {
        portMappingPtr->Ilkn2physMapArr [ilknIdx] = phys;
    }
    return GT_OK;
}


#if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
    static GT_CHAR * prv_mappingTypeStr[CPSS_DXCH_PORT_MAPPING_TYPE_MAX_E];
    static GT_CHAR * prv_portArgStr[PRV_CPSS_DXCH_PORT_TYPE_MAX_E];
#endif


GT_STATUS prvCpssDxChPortPhysicalPortMapCheckCascadeExtenderPorts
(
    INOUT PRV_CPSS_DXCH_PORT_MAPPING_STC *portMappingPtr,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_DXCH_PORT_MAP_STC     *portMapArrayPtr
)
{
    GT_U32 i;
    GT_U32 mac;
    CPSS_DXCH_PORT_MAP_STC *portMapPtr;

    /*--------------------------------------------------------------------------*/
    /* assumption that at first pass cpssDxChPortPhysicalPortMapSet()           */
    /*   the portMappingPtr table (PRV_CPSS_DXCH_PORT_MAPPING_STC) is filled    */
    /*--------------------------------------------------------------------------*/
    /* pass 1 : build list of possble extended-cascade ports by remotes (not by GT_NA -- may changed !!! )*/
    /*-----------------------------------------------------------------*/
    /* compute number of extended-cascade ( link remote physical ports) ports */
    portMapPtr = portMapArrayPtr;
    for ( i = 0; i< portMapArraySize ; i++, portMapPtr++)
    {
        if (portMapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
        {
            GT_PHYSICAL_PORT_NUM correspPhysPort;

            mac = portMapPtr->interfaceNum;
            portMappingPtr->isMacExtendedCascadePort[mac] = GT_TRUE;

            correspPhysPort = portMappingPtr->rxDma2PhysMapArr[mac];
            if (correspPhysPort == PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "remote port %d : no corresponding cascade port with mac %d",portMapPtr->physicalPortNumber,mac);
            }
            else
            {
                portMappingPtr->isPhysExtendedCascadePort[correspPhysPort] = GT_TRUE;
            }
            if (portMapPtr->tmEnable == GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "remote port %d : defined as passed through TM",portMapPtr->physicalPortNumber);
            }
        }
    }
    /*--------------------------------------------------------------------------------------*/
    /* pass 2 : check that if port has No TXQ defined than it has at least on remote client */
    /*--------------------------------------------------------------------------------------*/
    portMapPtr = portMapArrayPtr;
    for ( i = 0; i< portMapArraySize ; i++, portMapPtr++)
    {
        if (portMapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E && portMapPtr->tmEnable == GT_FALSE)
        {
            if (portMapPtr->txqPortNumber == GT_NA)  /* port is decalred as mapped */
            {
                mac = portMapPtr->interfaceNum;
                if (portMappingPtr->isMacExtendedCascadePort[mac] != GT_TRUE) /* port is declared as remote, but has no remotes clients */
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "cascade port %d : no corresponding remotes port with mac %d",portMapPtr->physicalPortNumber,mac);
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortInitPfcRanges function
* @endinternal
*
* @brief   Checks current mapping and add data to PFC port ranges (per DQ)
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portMapPtr               - pointer to mappings structure
*                                       None.
*/
static GT_VOID prvCpssDxChPortInitPfcRanges
(
    GT_U8                   devNum,
    CPSS_DXCH_PORT_MAP_STC *portMapPtr
)
{
    GT_U32 dq;

    if (portMapPtr->txqPortNumber == PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
    {
        /* no TXQ so no DQ */
        return;
    }

    dq = SIP_5_20_GLOBAL_TXQ_DQ_PORT_TO_DQ_UNIT_INDEX_MAC(devNum,portMapPtr->txqPortNumber);

    CPSS_TBD_BOOKMARK_ALDRIN2
    /* allow initialization of ALDRIN2 */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        /* support the 'Ethernet cpu port' (global MAC 72) (in DQ[0]) that hold
           physical ports that is more than the first physical port of DQ[1] */
        if(dq == 0 &&portMapPtr->physicalPortNumber > 11)
        {
            CPSS_LOG_INFORMATION_MAC("NOTE : port[%d] must not be used with PFC (as it breaks the 'PFC response ports ranges')\n",
                portMapPtr->physicalPortNumber);

            /* do not update the ranges */
            return;
        }
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseFirstPortInRange =
            MIN(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseFirstPortInRange,
                portMapPtr->physicalPortNumber);

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseLastPortInRange =
            MAX(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseLastPortInRange,
                portMapPtr->physicalPortNumber);
}


#if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
/**
* @internal prvCpssDxChPortPrintPfcRanges function
* @endinternal
*
* @brief   Prints PFC port ranges
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*                                       None.
*/
static GT_VOID prvCpssDxChPortPrintPfcRanges
(
    GT_U8   devNum
)
{
    GT_U32 i;

    for(i = 0; i < PRV_CPSS_SIP_5_20_MAX_DP_MAC(devNum); i++)
    {
        cpssOsPrintf("DQ index %d: ports %3d .. %3d\n", i,
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[i].pfcResponseFirstPortInRange,
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[i].pfcResponseLastPortInRange);
    }
}
#endif

/**
* @internal prvCpssDxChPortCheckPfcRanges function
* @endinternal
*
* @brief   Checks PFC port ranges overlap
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad ranges scheme
*/
static GT_STATUS prvCpssDxChPortCheckPfcRanges
(
    GT_U8   devNum
)
{
    GT_U32 dq, j;                         /* indexes of dq subunits                        */
    GT_U32 first0, first1, last0, last1;  /* bounds of compared ranges                     */
    GT_U32 dq0Bound, dq1Bound;            /* upper bounds of dq indexes in pipe0 and pipe1 */
    GT_U32 arrSize;                       /* size of hwInfo.multiDataPath.info array       */

    arrSize = MAX_DP_CNS;

    if(PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum))
    {
        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes == 2)
        {
            /* BC3 (2 control pipes each supports 3 DPs) */
            dq0Bound = (PRV_CPSS_SIP_5_20_MAX_DP_MAC(devNum)/2);
            dq1Bound = PRV_CPSS_SIP_5_20_MAX_DP_MAC(devNum);
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "not implemented for more than 2 pipes");
        }
    }
    else
    {
        /*Aldrin2 (single control pipe supports 4 DPs)*/
        dq0Bound = dq1Bound = PRV_CPSS_SIP_5_20_MAX_DP_MAC(devNum);
    }

    if ((dq0Bound > arrSize) || (dq1Bound > arrSize))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* check dq0 .. dq2 overlap */
    for(dq = 0; (dq < dq0Bound); dq++)
    {
        first0 = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseFirstPortInRange;
        last0  = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseLastPortInRange;

        if (first0 > last0) continue; /* not used range */

        for(j = dq+1; (j < dq0Bound); j++)
        {
            first1 = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[j].pfcResponseFirstPortInRange;
            last1  = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[j].pfcResponseLastPortInRange;

            if (first1 > last1) continue; /* not used range */

            if ((first0 <= last1) && (first1 <= last0))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "Overlapping found : DQ[%d] with 'physical port' range [%d..%d] overlapped with DQ[%d] with 'physical port' range [%d..%d]",
                    dq,first0,last0,
                    j,first1,last1);
            }
        }
    }

    /* check dq3 .. dq5 overlap */
    for(dq = dq0Bound; (dq < dq1Bound); dq++)
    {
        first0 = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseFirstPortInRange;
        last0  = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseLastPortInRange;

        if (first0 > last0) continue; /* not used range */

        for(j = dq+1; (j < dq1Bound); j++)
        {
            first1 = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[j].pfcResponseFirstPortInRange;
            last1  = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[j].pfcResponseLastPortInRange;

            if (first1 > last1) continue; /* not used range */

            if ((first0 <= last1) && (first1 <= last0))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "Overlapping found : DQ[%d] with 'physical port' range [%d..%d] overlapped with DQ[%d] with 'physical port' range [%d..%d]",
                    dq,first0,last0,
                    j,first1,last1);
            }
        }
    }

    /* check range sizes - range cannot be more than 128 ports as a size of table */
    for(dq = 0; (dq < dq1Bound); dq++)
    {
        first0 = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseFirstPortInRange;
        last0  = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseLastPortInRange;

        if (first0 > last0) continue; /* not used range */

        if ((first0 + 128) <= last0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSetPfcRanges function
* @endinternal
*
* @brief   Sets PFC port ranges to HW registers
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad ranges scheme
*/
static GT_STATUS prvCpssDxChPortSetPfcRanges
(
    GT_U8   devNum
)
{
    GT_STATUS rc;
    GT_U32    dq;
    GT_U32    regAddr;
    GT_U32    first, last;

    PRV_CPSS_DXCH_SIP6_NOT_SUPPORT_SIP5_TXQ_CHECK_MAC(devNum);

    for(dq = 0; dq < PRV_CPSS_SIP_5_20_MAX_DP_MAC(devNum); dq++)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum, dq).flowCtrlResponse.flowCtrlResponseConfig;
        first = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseFirstPortInRange;
        last  = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dq].pfcResponseLastPortInRange;

        if (first > last)
        {
            /* not initialized range */
            continue;
        }

        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 3, 10, first);
        if (GT_OK != rc)
        {
            return rc;
        }

        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 13, 10, last);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}



/**
* @internal prvCpssFalconPreqTxqCncUpdate function
* @endinternal
*
* @brief   PreQ update info for TxQ CNC counting
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

static GT_STATUS prvCpssFalconPreqTxqCncUpdate
(
    IN  GT_U8                       devNum
)
{
    GT_STATUS   rc;
    GT_U32 queueGroupIndex,globalQueueBase;
    GT_PHYSICAL_PORT_NUM    physPort;
    GT_U32  tileId,dpIndex,localPortInDp,queueBaseInDp,queueBaseInTile;
    GT_BOOL isCascade;

    for(physPort = 0 ; physPort < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum); physPort++)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPort].valid == GT_FALSE)
        {
            continue;
        }

        rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physPort,&isCascade);
        if (GT_OK != rc )
        {
            return rc;
        }

        if(isCascade==GT_TRUE)
         {
                PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPort].portMap.txqNum =GT_NA;
                continue;
         }

        rc = prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapGet(devNum,0/*unused*/,physPort,&queueGroupIndex);
        if (GT_OK != rc )
        {
            return rc;
        }



        rc = prvCpssFalconTxqUtilsGetQueueGroupAttributes(devNum,queueGroupIndex,
            &tileId,&dpIndex,&localPortInDp,&queueBaseInDp,&queueBaseInTile);
        if (GT_OK != rc )
        {
            return rc;
        }

        /******************************************/
        /* use global number from the application */
        /******************************************/
        globalQueueBase = tileId << 11 | queueBaseInTile;

        /* The PreQ hold global 'queue base' */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                CPSS_DXCH_SIP6_TABLE_PREQ_CNC_PORT_MAPPING_E,
                queueGroupIndex,
                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                SIP6_PREQ_QUEUE_PORT_MAPPING_TABLE_FIELDS_PASS_DROP_PORT_BASE_E, /* field name */
                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                globalQueueBase );/*13 bits in PreQ*/
        if (GT_OK != rc )
        {
            return rc;
        }

        /* save the 'Global TXQ' number */
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[physPort].portMap.txqNum =
            globalQueueBase;
    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChPortPhysicalPortMapSet function
* @endinternal
*
* @brief   configure ports mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portMapArraySize         - Number of ports to map, array size
* @param[in] portMapArrayPtr          - pointer to array of mappings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. All usable ports shall be configured at ONCE. Any port, that was mapped previously and is
*       not included into current mapping, losses its mapping.
*       2. Changing mapping under traffic may cause to traffic interruption.
*       3. when CPSS_DXCH_CFG_PORT_MAPPING_TYPE_INLK_CHANNEL_E is defined the first channel
*       physical port is used for DMA configuration
*       4. The function checks whether all mapping are consistent.
*       In case of contradiction between mappings
*       (e,g. single TxQ port is mapped to several Tx DMA ports) the function returns with BAD_PARAM.
*       5. Other CPSS APIs that receive GT_PHYSICAL_PORT_NUM as input parameter will use this mapping
*       to convert the physical port to mac/dma/txq to configure them
*       6. Usage example
*       #define GT_NA ((GT_U32)-1)
*       CPSS_DXCH_CFG_PORT_MAP_STC bc2ExampleMap[] =
*       (
*       ( 0, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   0, 0, GT_FALSE),
*       ( 1, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   1, 1, GT_FALSE),
*       ( 50, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   50, 2, GT_FALSE),
*       ( 53, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   53, 3, GT_FALSE),
*       ( 63, _INTRFACE_TYPE_CPU_SDMA_E,   GT_NA, GT_NA, 71, GT_FALSE),
*       (100, _INTRFACE_TYPE_ILNK_MAC_E,   GT_NA, GT_NA, 40, GT_TRUE),
*       (101, _INTRFACE_TYPE_REMOTE_PORT_E, GT_NA, GT_NA, 41, GT_TRUE),
*       (102, _INTRFACE_TYPE_REMOTE_PORT_E, GT_NA, GT_NA, 42, GT_TRUE),
*       (103, _INTRFACE_TYPE_REMOTE_PORT_E, GT_NA, GT_NA, 43, GT_TRUE)
*       );
*       rc = cpssDxChCfgPhysicalPortMapSet(devNum, sizeof(bc2ExampleMap)/sizeof(bc2ExampleMap[0]), &bc2ExampleMap[0]);
*
*/
static GT_STATUS internal_cpssDxChPortPhysicalPortMapSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_DXCH_PORT_MAP_STC     *portMapArrayPtr
)
{
    GT_U32 i;
    GT_U32 srcPort;
    GT_U32 dstPort;
    CPSS_PP_FAMILY_BIT_ENT devType;
    PRV_MAPTYPE_ENT        prvMappingType;
    GT_U32 physPort;
    GT_U32 numberOfTiles;

    static PRV_CPSS_DXCH_PORT_MAPPING_STC   portMapping;
    GT_U32                                  portArgArr[PRV_CPSS_DXCH_PORT_TYPE_MAX_E];
    CPSS_DXCH_PORT_MAP_STC                * portMapPtr;
    PRV_DXCH_CPSS_MAPPING_SET_GET_FUN_STC * funListPtr;
    GT_STATUS rc;
    GT_BOOL   firstInterlackingChannelFound;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);



    if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(portMapArrayPtr);

    rc = prvCpssDxChPortMappingInit(&portMapping,devNum);
    if (GT_OK != rc )
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) &&
      !PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* init PFC response ranges for each dq                */
        /* must be finished before calling                     */
        /* prvCpssDxChPortMappingPFCResponsePhysPort2TxQMapSet */
        portMapPtr = portMapArrayPtr;
        for ( i = 0; i< portMapArraySize ; i++, portMapPtr++)
        {
            if(portMapPtr->mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                prvCpssDxChPortInitPfcRanges(devNum, portMapPtr);
            }
        }
        /* check PFC response ranges for each dq */
        rc = prvCpssDxChPortCheckPfcRanges(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* save PFC response ranges for each dq to HW registers */
        rc = prvCpssDxChPortSetPfcRanges(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }




    /*----------------------------------*/
    /* first pass : check the arguments */
    /*----------------------------------*/
    #if (PRV_CPSS_MAPPING_DEBUG_CNS == 1)
        prv_portArgStr[PRV_CPSS_DXCH_PORT_TYPE_PHYS_E         ] = "phys ";
        prv_portArgStr[PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E        ] = "rxDma";
        prv_portArgStr[PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E        ] = "txDma";
        prv_portArgStr[PRV_CPSS_DXCH_PORT_TYPE_MAC_E          ] = "mac  ";
        prv_portArgStr[PRV_CPSS_DXCH_PORT_TYPE_TXQ_E          ] = "txq  ";
        prv_portArgStr[PRV_CPSS_DXCH_PORT_TYPE_TM_PORT_IDX_E  ] = "tm   ";

        prv_mappingTypeStr[CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ]        = "ETHERNET";
        prv_mappingTypeStr[CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E     ]        = "CPU-SDMA";
        prv_mappingTypeStr[CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E ]        = "ILKN-CHL";
        prv_mappingTypeStr[CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E] = "REMOTE-P";
    #endif

    #if (PRV_CPSS_MAPPING_CHECK_PRINT_CNS == 1)
        cpssOsPrintf("\nCheck Mapping:");
    #endif
    #if (PRV_CPSS_MAPPING_CHECK_PRINT_CNS == 1)
        cpssOsPrintf("\n+-----------------+----------------------------+-----------------------------------+-----+");
        cpssOsPrintf("\n|                 |      Input                 |          Computed                 |     |");
        cpssOsPrintf("\n|   mapping Type  |------+---------------------+-----------------------------------+ Res |");
        cpssOsPrintf("\n|                 | phys |  if txq tmEn tmPort | phys rxdma txdma mac txq ilkn  tm |     |");
        cpssOsPrintf("\n+-----------------+------+---------------------+-----------------------------------+-----+");
    #endif

    firstInterlackingChannelFound = GT_FALSE;
    portMapPtr = portMapArrayPtr;
    for ( i = 0; i< portMapArraySize ; i++, portMapPtr++)
    {
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
        #if (PRV_CPSS_MAPPING_CHECK_PRINT_CNS == 1)
            cpssOsPrintf("\n| %-15s | %4d | %3d %3d %4d %6d |",
                                               prv_mappingTypeStr[portMapPtr->mappingType],
                                               portMapPtr->physicalPortNumber,
                                               portMapPtr->interfaceNum,
                                               portMapPtr->txqPortNumber,
                                               portMapPtr->tmEnable,
                                               portMapPtr->tmPortInd);
        #endif

        rc = prvCpssDxChPortMappingTypeCheck(devNum,portMapPtr);
        if (GT_OK != rc )
        {
            return rc;
        }

        rc = prvCpssDxChPortMappingTypeGet(portMapPtr->mappingType, /*OUT*/&prvMappingType);
        if (GT_OK != rc )
        {
            return rc;
        }
        if (prvMappingType == PRV_MAPTYPE_ILKN_CHANNEL_E)
        {
            if (firstInterlackingChannelFound == GT_FALSE)
            {
                prvMappingType = PRV_MAPTYPE_ILKN_MAC_E;
                firstInterlackingChannelFound = GT_TRUE;
            }
        }
        /*-------------------------*/
        /* build arguments          */
        /*-------------------------*/
        rc = prvCpsssDxChPortMappingArgumentsBuild(devNum,portMapPtr,/*OUT*/&portArgArr[0]);
        if (GT_OK != rc )
        {
            return rc;
        }
        /*---------------------------------------------------------------------------------*/
        /* check if port already initialized i.e. has bandwidth , that mapping is unchanged */
        /*---------------------------------------------------------------------------------*/
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portMapPtr->physicalPortNumber,/*OUT*/&portMapShadowPtr);
        if (portMapShadowPtr->valid == GT_TRUE)
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                CPSS_PORT_INTERFACE_MODE_ENT ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMapShadowPtr->portMap.macNum);
                CPSS_PORT_SPEED_ENT          speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,   portMapShadowPtr->portMap.macNum);
                if(CPSS_PORT_INTERFACE_MODE_NA_E != ifMode)
                {
                    if (CPSS_PORT_SPEED_NA_E != speed)
                    {
                        if (portMapShadowPtr->portMap.macNum   != portArgArr[PRV_CPSS_DXCH_PORT_TYPE_MAC_E])
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "index [%d] physicalPortNumber[%d]",
                                i, portMapPtr->physicalPortNumber);
                        }
                        if (portMapShadowPtr->portMap.rxDmaNum != portArgArr[PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E])
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "index [%d] physicalPortNumber[%d]",
                                i, portMapPtr->physicalPortNumber);
                        }
                        if (portMapShadowPtr->portMap.txDmaNum != portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E])
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "index [%d] physicalPortNumber[%d]",
                                i, portMapPtr->physicalPortNumber);
                        }
                        if (portMapShadowPtr->portMap.txqNum != portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TXQ_E])
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "index [%d] physicalPortNumber[%d]",
                                i, portMapPtr->physicalPortNumber);
                        }
                    }
                }
            }
        }


        #if (PRV_CPSS_MAPPING_CHECK_PRINT_CNS == 1)
            cpssOsPrintf(" %4d %5d %5d %3d %3d %4d %3d |",
                        portArgArr[PRV_CPSS_DXCH_PORT_TYPE_PHYS_E],
                        portArgArr[PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E],
                        portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TxDMA_E],
                        portArgArr[PRV_CPSS_DXCH_PORT_TYPE_MAC_E],
                        portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TXQ_E],
                        portArgArr[PRV_CPSS_DXCH_PORT_TYPE_ILKN_CHANNEL_E],
                        portArgArr[PRV_CPSS_DXCH_PORT_TYPE_TM_PORT_IDX_E]);
        #endif

        /*-------------------------*/
        /*  check                  */
        /*-------------------------*/
        rc = prvCpssDxChPortMappingCheckAndAdd(/*INOUT*/&portMapping, /*IN*/devNum,/*IN*/prvMappingType,/*IN*/portArgArr);
        if (GT_OK != rc )
        {
            return rc;
        }
        #if (PRV_CPSS_MAPPING_CHECK_PRINT_CNS == 1)
            cpssOsPrintf("  OK |");
        #endif
    }
    #if (PRV_CPSS_MAPPING_CHECK_PRINT_CNS == 1)
        cpssOsPrintf("\n+-----------------+------+---------------------+-----------------------------------+-----+");
    #endif

    rc = prvCpssDxChPortPhysicalPortMapCheckCascadeExtenderPorts(/*INOUT*/&portMapping, /*IN*/portMapArraySize,portMapArrayPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*****************************/
    /*   Shadow DB maintenance   */
    /*****************************/
    /* Clear all shadow DB */
    rc = prvCpssDxChPortPhysicalPortMapShadowDBInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*----------------------------------*/
    /* second pass : perform mapping    */
    /*----------------------------------*/
    firstInterlackingChannelFound = GT_FALSE;
    portMapPtr = portMapArrayPtr;

    for ( i = 0; i< portMapArraySize ; i++, portMapPtr++)
    {
        /*-----------------------------*/
        /* build internal mapping type */
        /*------------------------------------------------------------------------------------------*/
        /* physical port of first interlaken channel is used for RxDMA configuration find           */
        /* it shall be NOT called for any other interlaken channel                                  */
        /* therefore its type is changed to PRV_MAPTYPE_ILKN_MAC_E                                  */
        /*------------------------------------------------------------------------------------------*/
        rc = prvCpssDxChPortMappingTypeGet(portMapPtr->mappingType, /*OUT*/&prvMappingType);
        if (GT_OK != rc )
        {
            return rc;
        }
        if (prvMappingType == PRV_MAPTYPE_ILKN_CHANNEL_E)
        {
            if (firstInterlackingChannelFound == GT_FALSE)
            {
                prvMappingType = PRV_MAPTYPE_ILKN_MAC_E;
                firstInterlackingChannelFound = GT_TRUE;
            }
        }

        /*------------------*/
        /* build dev type   */
        /*------------------*/
        rc = prvCpsssDxChPortMappingDeviceTypeGet(devNum,PRV_CPSS_PP_MAC(devNum)->devFamily,PRV_CPSS_PP_MAC(devNum)->devSubFamily,/*OUT*/&devType);
        if (GT_OK != rc )
        {
            return rc;
        }

        /*------------------*/
        /* build arguments   */
        /*------------------*/
        rc = prvCpsssDxChPortMappingArgumentsBuild(devNum,portMapPtr,/*OUT*/&portArgArr[0]);
        if (GT_OK != rc )
        {
            return rc;
        }


        /*-------------------------------------------------------------------*/
        /* call all function from list                                       */
        /*-------------------------------------------------------------------*/
        #if (PRV_CPSS_MAPPING_UNIT_LIST_DEBUG_CNS == 1)
            cpssOsPrintf("\n%-15s phys = %3d if = %3d txq = %3d tm = %d tmPort = %d",
                                               prv_mappingTypeStr[portMapPtr->mappingType],
                                               portMapPtr->physicalPortNumber,
                                               portMapPtr->interfaceNum,
                                               portMapPtr->txqPortNumber,
                                               portMapPtr->tmEnable,
                                               portMapPtr->tmPortInd);
        #endif
        physPort = portArgArr[PRV_CPSS_DXCH_PORT_TYPE_PHYS_E];
        for (funListPtr = &prv_MappingFunList[0]; funListPtr->mapingTypeBmp != 0; funListPtr++)
        {
            if ((portMapping.physIsEqualt2OldMapping[physPort] != GT_FALSE)
                && (funListPtr->setFun != prvCpssDxChPortMappingPFCResponsePhysPort2TxQMapSet))
            {
                /* PFCResponsePhysPort2TxQMap should be rewritten anyway:         */
                /* if base changed the same values written to other table indexes */
                continue;
            }
            /* if function is applicable for the mapping type */
            if (0 != (prvMappingType & funListPtr->mapingTypeBmp))
            {
                /* if function is applicable for the input device */
                if (0 != (devType & funListPtr->applicableDevice))
                {
                    srcPort = portArgArr[funListPtr->srcPortType];
                    dstPort = portArgArr[funListPtr->dstPortType];
                    #if (PRV_CPSS_MAPPING_UNIT_LIST_DEBUG_CNS == 1)
                        cpssOsPrintf("\n    %-20s %s (%3d) --> %s (%3d)",funListPtr->stringMapTypePtr,
                                                                          prv_portArgStr[funListPtr->srcPortType],srcPort,
                                                                          prv_portArgStr[funListPtr->dstPortType],dstPort);
                    #endif
                    if (systemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
                    {
                        rc = funListPtr->setFun(devNum, portMapPtr->portGroup, srcPort, dstPort);
                        if (GT_OK != rc )
                        {
                            return rc;
                        }
                    }
                }
            }
        }



        /*-----------------------------*/
        /* TM related configurations   */
        /*-----------------------------*/
        if (systemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
        {
            rc = prvCpssDxChPortPhysicalPortTmMapping(devNum,portMapPtr);
            if (GT_OK != rc )
            {
                return rc;
            }
        }

        /****************************************************************************************/
        /*   Shadow DB maintenance shall be performed in any case (previous mapping was erased  */
        /****************************************************************************************/
        rc = prvCpssDxChPortPhysicalPortMapShadowDBSet(devNum, portMapPtr->physicalPortNumber, portMapPtr, portArgArr,portMapping.isPhysExtendedCascadePort[portMapPtr->physicalPortNumber]);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            /* NOTE: cpssDxChCscdPortTypeSet(...) must be called for 'eArch' device
                only after 'tables initialization' because the setting done using 'tables engine' */

            /* set the CPU as '4 words' cascade port , because the 2 words(extended DSA)
                tag can't support receive from physical port > 63 and do not support eArch */
            rc = prvCpssDxChCscdPortSet(devNum,
                                        portMapPtr->physicalPortNumber,
                                        portArgArr[PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E],
                                        1,/*do ingress*/
                                        1,/*do egress*/
                                        CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
            if(rc != GT_OK)
            {
               return rc;
            }
        }
    }


    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
        {
            case 0:
            case 1:
                numberOfTiles = 1;
                break;
           case 2:
           case 4:
                numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
                break;

        }

        prvCpssFalconTxqUtilsInitDmaToPnodeDb(devNum);
        prvCpssFalconTxqUtilsInitPhysicalPortToAnodeDb(devNum);
        prvCpssFalconTxqUtilsInitSchedProfilesDb(devNum);


        /*First build data base*/
         for(i=0;i<numberOfTiles;i++)
         {
              tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[i]);

              prvCpssFalconTxqUtilsInitDb(tileConfigsPtr,i,numberOfTiles);

             rc = prvCpssFalconTxqUtilsConvertToDb(devNum,portMapArrayPtr,portMapArraySize,i,&(tileConfigsPtr->mapping));

              if (rc != GT_OK)
              {
                    return rc;
              }
         }


         /*Now configure the device*/
        for(i=0;i<numberOfTiles;i++)
        {

                /*PDX*/
                rc = prvCpssFalconTxqUtilsInitPdxInterfaceTable(devNum,i);

                if (GT_OK != rc )
                {
                    return rc;
                }

                tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigs[i]);


                rc = prvCpssFalconTxqUtilsInitPdxQroupMapTable(devNum,&(tileConfigsPtr->mapping));
                if (GT_OK != rc )
                {
                    return rc;
                }



                rc =prvCpssDxChTxQFalconPizzaArbiterInitPerTile(devNum,i,&(tileConfigsPtr->mapping.pdxPizza.pdxPizzaNumberOfSlices[0]));

                if (GT_OK != rc )
                {
                    return rc;
                }

                /*SDQ*/
                rc = prvCpssFalconTxqUtilsInitSdq(devNum,i,numberOfTiles,&(tileConfigsPtr->mapping));
                if (GT_OK != rc )
                {
                    return rc;
                }


                /*PSI*/
                rc = prvCpssFalconTxqUtilsInitPsi(devNum,i);
                if (GT_OK != rc )
                {
                    return rc;
                }


                /*PDQ - build the tree*/
                rc = prvCpssFalconTxqUtilsBuildPdqTree(devNum,i,tileConfigsPtr);
                if (GT_OK != rc )
                {
                    return rc;
                }


            }


           /*Init PFCC table*/
            rc = prvCpssDxChTxqPfccTableInit(devNum);

             if (rc != GT_OK)
              {
                      return rc;
              }

             /*Init QFC units*/
            rc = prvCpssDxChTxqQfcUnitsInit(devNum);

             if (rc != GT_OK)
          {
                  return rc;
          }

            /* PreQ update info after TxQ did port/queue mapping */
            rc = prvCpssFalconPreqTxqCncUpdate(devNum);
            if (GT_OK != rc )
            {
                return rc;
            }
    }

    /* BC3 does not have high speed ports units */
    if (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChBobKHighSpeedPortConfigure(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* notify the networkIf library that the port mapping finished */
    rc = prvCpssDxChNetIfInitAfterPortMapping(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    return GT_OK;
}

/**
* @internal cpssDxChPortPhysicalPortMapSet function
* @endinternal
*
* @brief   configure ports mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portMapArraySize         - Number of ports to map, array size
* @param[in] portMapArrayPtr          - pointer to array of mappings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. All usable ports shall be configured at ONCE. Any port, that was mapped previously and is
*       not included into current mapping, losses its mapping.
*       2. Changing mapping under traffic may cause to traffic interruption.
*       3. when CPSS_DXCH_CFG_PORT_MAPPING_TYPE_INLK_CHANNEL_E is defined the first channel
*       physical port is used for DMA configuration
*       4. The function checks whether all mapping are consistent.
*       In case of contradiction between mappings
*       (e,g. single TxQ port is mapped to several Tx DMA ports) the function returns with BAD_PARAM.
*       5. Other CPSS APIs that receive GT_PHYSICAL_PORT_NUM as input parameter will use this mapping
*       to convert the physical port to mac/dma/txq to configure them
*       6. Usage example
*       #define GT_NA ((GT_U32)-1)
*       CPSS_DXCH_CFG_PORT_MAP_STC bc2ExampleMap[] =
*       (
*       ( 0, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   0, 0, GT_FALSE),
*       ( 1, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   1, 1, GT_FALSE),
*       ( 50, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   50, 2, GT_FALSE),
*       ( 53, _INTRFACE_TYPE_ETHERNET_MAC_E, GT_NA,   53, 3, GT_FALSE),
*       ( 63, _INTRFACE_TYPE_CPU_SDMA_E,   GT_NA, GT_NA, 71, GT_FALSE),
*       (100, _INTRFACE_TYPE_ILNK_MAC_E,   GT_NA, GT_NA, 40, GT_TRUE),
*       (101, _INTRFACE_TYPE_REMOTE_PORT_E, GT_NA, GT_NA, 41, GT_TRUE),
*       (102, _INTRFACE_TYPE_REMOTE_PORT_E, GT_NA, GT_NA, 42, GT_TRUE),
*       (103, _INTRFACE_TYPE_REMOTE_PORT_E, GT_NA, GT_NA, 43, GT_TRUE)
*       );
*       rc = cpssDxChCfgPhysicalPortMapSet(devNum, sizeof(bc2ExampleMap)/sizeof(bc2ExampleMap[0]), &bc2ExampleMap[0]);
*
*/
GT_STATUS cpssDxChPortPhysicalPortMapSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_DXCH_PORT_MAP_STC     *portMapArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPhysicalPortMapSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portMapArraySize, portMapArrayPtr));

    rc = internal_cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portMapArraySize, portMapArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortPhysicalPortMapGet function
* @endinternal
*
* @brief   get ports mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] physicalPortNum          - physical port to bring
*
* @param[out] portMapPtr               - pointer to ports mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_FAIL                  - when cannot deliver requested number of items
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPhysicalPortMapGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        physicalPortNum,
    OUT CPSS_DXCH_SHADOW_PORT_MAP_STC  *portMapPtr
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM physPort4Cntrol;
    GT_U32               cpuDmaNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, physicalPortNum);
    CPSS_NULL_PTR_CHECK_MAC(portMapPtr);

    if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum)) /* currently only Bobcat2/3 is supported */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }


    portMapPtr->txqNum      = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
    portMapPtr->macNum      = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
    portMapPtr->rxDmaNum    = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
    portMapPtr->txDmaNum    = PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS;
    portMapPtr->portGroup   = 0;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum)) /* Falcon */
    {
        /* retrieve data from shadow - not from HW */
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(
            devNum, physicalPortNum, /*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (portMapShadowPtr->valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        portMapPtr->txqNum      = portMapShadowPtr->portMap.txqNum;
        portMapPtr->macNum      = portMapShadowPtr->portMap.macNum;
        portMapPtr->rxDmaNum    = portMapShadowPtr->portMap.rxDmaNum;
        portMapPtr->txDmaNum    = portMapShadowPtr->portMap.txDmaNum;
        portMapPtr->mappingType = portMapShadowPtr->portMap.mappingType;
        portMapPtr->portGroup   = portMapShadowPtr->portMap.portGroup;
        portMapPtr->ilknChannel = PRV_CPSS_DXCH_PP_MAC(devNum)->
                    port.portsMapInfoShadowArr[physicalPortNum].portMap.ilknChannel;
        return GT_OK;
    }

    rc = prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMAGet(devNum,portMapPtr->portGroup,physicalPortNum,/*OUT*/&portMapPtr->rxDmaNum); /* 3  Phys --> RxDma */
    if (GT_OK != rc )
    {
        return rc;
    }

    rc = prvCpssDxChPortMappingPFCTriggerPhysPort2MACMapGet(devNum,portMapPtr->portGroup,physicalPortNum,/*OUT*/&portMapPtr->macNum);  /* 7 Phys --> Mac */
    if (GT_OK != rc )
    {
        return rc;
    }

    rc = prvCpssDxChPortMappingRxDMA2LocalPhysGet(devNum,portMapPtr->portGroup,portMapPtr->rxDmaNum,/*OUT*/&physPort4Cntrol);  /* 1 RxDma --> Phys */
    if (GT_OK != rc )
    {
        return rc;
    }

    rc = prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortMapGet(devNum,portMapPtr->portGroup,physicalPortNum,/*OUT*/&portMapPtr->txqNum);  /* 4.1 Phys --> TxQ */
    if (GT_OK != rc )
    {
        return rc;
    }

    if (portMapPtr->txqNum != PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS)
    {
        rc = prvCpssDxChPortMappingTxQPort2TxDMAMapGet(devNum,portMapPtr->portGroup,portMapPtr->txqNum,/*OUT*/&portMapPtr->txDmaNum);             /* 5 TxQ --> txDma */
        if (GT_OK != rc )
        {
            return rc;
        }
    }


    rc = prvCpssDxChHwPpSdmaCpuPortReservedDmaIdxGet(devNum,
        PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? portMapPtr->rxDmaNum :
            portMapPtr->txqNum,
            &cpuDmaNum);
    if (GT_OK != rc )
    {
        return rc;
    }

    if (portMapPtr->rxDmaNum == cpuDmaNum)
    {
        portMapPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E;
    }
    else if (portMapPtr->rxDmaNum == PRV_CPSS_DXCH_PORT_NUM_ILKN_DMA_CNS)
    {/* this DMA number can be either ILKN or usual ethernet */
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physicalPortNum, /*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
                portMapPtr->mappingType = portMapShadowPtr->portMap.mappingType;
    }
    else /* dmaPort != CPU (72) && dmaPort != ILKN (64) */
    {
        if (physPort4Cntrol == physicalPortNum)
        {
            portMapPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
        }
        else
        {
            portMapPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E;
        }
    }

    portMapPtr->ilknChannel = PRV_CPSS_DXCH_PP_MAC(devNum)->
                port.portsMapInfoShadowArr[physicalPortNum].portMap.ilknChannel;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortPhysicalPortTmMappingGet function
* @endinternal
*
* @brief   Get TM related port mapping
*
* @param[in] devNum                   - device number
* @param[in] physicalPortNum          - physical port to bring
* @param[in,out] portMapPtr               - pointer to port mapping
* @param[in,out] portMapPtr               - pointer to port mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_FAIL                  - when cannot deliver requested number of items
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortPhysicalPortTmMappingGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        physicalPortNum,
    INOUT CPSS_DXCH_PORT_MAP_STC    *portMapPtr
)
{
    GT_STATUS rc;                       /* return value */
    CPSS_DXCH_TABLE_ENT tableType;  /* table type definition */
    GT_U32    regAddr;                  /* register address */
    GT_U32    regData;                  /* register data */

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, physicalPortNum);

    if( PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.trafficManager.supported == GT_TRUE)
    {
        /* TM enable - Physical port enabling status */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                TMDROP.portsInterface[(portMapPtr->physicalPortNumber)/32];
        rc = prvCpssHwPpGetRegField(devNum, regAddr,
                            (portMapPtr->physicalPortNumber)%32, 1,
                            &regData);
        if (GT_OK != rc )
        {
            return rc;
        }

        portMapPtr->tmEnable = BIT2BOOL_MAC(regData);

        if( GT_TRUE == portMapPtr->tmEnable )
        {
            switch(portMapPtr->mappingType)
            {
                case CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E:
                    tableType =
                  CPSS_DXCH_SIP5_TABLE_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_E;
                    break;

                case CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E:
                    tableType =
                  CPSS_DXCH_SIP5_TABLE_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_E;
                    break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            /* Get physical port or interlaken channel to TM port mapping */
            rc = prvCpssDxChReadTableEntryField
                        (devNum,
                         tableType,
                         (portMapPtr->interfaceNum)/4,
                         0,
                         ((portMapPtr->interfaceNum)%4)*8,
                         8,
                         &portMapPtr->tmPortInd);

            if( GT_OK != rc )
            {
                return rc;
            }
        }
    }
    else /* no TM */
    {
        portMapPtr->tmEnable = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortPhysicalPortMapGet function
* @endinternal
*
* @brief   get ports mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] firstPhysicalPortNumber  - first physical port to bring
* @param[in] portMapArraySize         - Mapping array size
*
* @param[out] portMapArrayPtr          - pointer to ports mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_FAIL                  - when cannot deliver requested number of items
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPhysicalPortMapGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        firstPhysicalPortNumber,
    IN GT_U32                      portMapArraySize,
    OUT CPSS_DXCH_PORT_MAP_STC    *portMapArrayPtr
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_PHYSICAL_PORT_NUM physPort;

    CPSS_DXCH_SHADOW_PORT_MAP_STC detailedPortMap;
    CPSS_DXCH_PORT_MAP_STC * portMapPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))  /* currently only Bobcat2/3 is supported */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(portMapArrayPtr);

    if ((firstPhysicalPortNumber + portMapArraySize - 1) > PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    portMapPtr = portMapArrayPtr;
    physPort = firstPhysicalPortNumber;
    for ( i = 0; i< portMapArraySize ; i++, portMapPtr++, physPort++)
    {
        rc = prvCpssDxChPortPhysicalPortMapGet(devNum,physPort,/*OUT*/&detailedPortMap);
        if (GT_OK != rc )
        {
            return rc;
        }

        portMapPtr->physicalPortNumber = physPort;
        portMapPtr->mappingType        = detailedPortMap.mappingType;
        if( CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E == portMapPtr->mappingType )
        {
            portMapPtr->interfaceNum = detailedPortMap.ilknChannel;
        }
        else
        {
            portMapPtr->interfaceNum       = detailedPortMap.rxDmaNum;       /* MAC == DMA , interface num is used for MAC type of mappings */
        }
        portMapPtr->portGroup          = detailedPortMap.portGroup;
        portMapPtr->txqPortNumber      = detailedPortMap.txqNum;

        rc = prvCpssDxChPortPhysicalPortTmMappingGet(devNum, physPort, /*OUT*/portMapPtr);
        if (GT_OK != rc )
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssDxChPortPhysicalPortMapGet function
* @endinternal
*
* @brief   get ports mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] firstPhysicalPortNumber  - first physical port to bring
* @param[in] portMapArraySize         - Mapping array size
*
* @param[out] portMapArrayPtr          - pointer to ports mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_FAIL                  - when cannot deliver requested number of items
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPhysicalPortMapGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        firstPhysicalPortNumber,
    IN GT_U32                      portMapArraySize,
    OUT CPSS_DXCH_PORT_MAP_STC    *portMapArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPhysicalPortMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, firstPhysicalPortNumber, portMapArraySize, portMapArrayPtr));

    rc = internal_cpssDxChPortPhysicalPortMapGet(devNum, firstPhysicalPortNumber, portMapArraySize, portMapArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, firstPhysicalPortNumber, portMapArraySize, portMapArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPhysicalPortMapIsValidGet function
* @endinternal
*
* @brief   Check if physical port was mapped
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isValidPtr               - (pointer to) is valid mapping
*                                      GT_TRUE - given port was mapped
*                                      GT_FALSE - there is no valid mapping for given port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPhysicalPortMapIsValidGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isValidPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(isValidPtr);

    *isValidPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid;

    return GT_OK;
}

/**
* @internal cpssDxChPortPhysicalPortMapIsValidGet function
* @endinternal
*
* @brief   Check if physical port was mapped
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isValidPtr               - (pointer to) is valid mapping
*                                      GT_TRUE - given port was mapped
*                                      GT_FALSE - there is no valid mapping for given port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPhysicalPortMapIsValidGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isValidPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPhysicalPortMapIsValidGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isValidPtr));

    rc = internal_cpssDxChPortPhysicalPortMapIsValidGet(devNum, portNum, isValidPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isValidPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChPortPhysicalPortMapIsCpuGet function
* @endinternal
*
* @brief   Check if physical port was mapped as CPU port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isCpuPtr                 - (pointer to) is mapped as CPU port
*                                      GT_TRUE - given port was mapped as CPU port
*                                      GT_FALSE - there is no valid mapping for given port,
*                                      or it is not mapped as CPU port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPhysicalPortMapIsCpuGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isCpuPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(isCpuPtr);

    if (PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum))
    {
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid == GT_TRUE)&&
            (PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.mappingType
                                                        == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
        {
            *isCpuPtr = GT_TRUE;
        }
        else
        {
            *isCpuPtr = GT_FALSE;
        }
    }
    else /*mapping not supported*/
    {
        if (portNum == CPSS_CPU_PORT_NUM_CNS)
        {
            *isCpuPtr = GT_TRUE;
        }
        else
        {
            *isCpuPtr = GT_FALSE;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortPhysicalPortMapIsCpuGet function
* @endinternal
*
* @brief   Check if physical port was mapped as CPU port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isCpuPtr                 - (pointer to) is mapped as CPU port
*                                      GT_TRUE - given port was mapped as CPU port
*                                      GT_FALSE - there is no valid mapping for given port,
*                                      or it is not mapped as CPU port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortPhysicalPortMapIsCpuGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isCpuPtr
)
{
    return prvCpssDxChPortPhysicalPortMapIsCpuGet(devNum, portNum, isCpuPtr);
}

/**
* @internal cpssDxChPortPhysicalPortMapIsCpuGet function
* @endinternal
*
* @brief   Check if physical port was mapped as CPU port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isCpuPtr                 - (pointer to) is mapped as CPU port
*                                      GT_TRUE - given port was mapped as CPU port
*                                      GT_FALSE - there is no valid mapping for given port,
*                                      or it is not mapped as CPU port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPhysicalPortMapIsCpuGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isCpuPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPhysicalPortMapIsCpuGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isCpuPtr));

    rc = internal_cpssDxChPortPhysicalPortMapIsCpuGet(devNum, portNum, isCpuPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isCpuPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortPhysicalPortMapReverseMappingGet function
* @endinternal
*
* @brief   Function checks and returns physical port number that mapped to given
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] origPortType             - original port type
* @param[in] origPortNum              - MAC/TXQ/DMA port number
*
* @param[out] physicalPortNumPtr       - (pointer to) physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, origPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_FOUND             - not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_IMPLEMENTED       - when this functionality is not implemented
*/
static GT_STATUS internal_cpssDxChPortPhysicalPortMapReverseMappingGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_PORT_MAPPING_TYPE_ENT origPortType,
    IN  GT_U32                          origPortNum,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumPtr
)
{

    PRV_CPSS_DXCH_PORT_TYPE_ENT         portType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(physicalPortNumPtr);

    switch (origPortType)
    {
        case CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E:

            portType = PRV_CPSS_DXCH_PORT_TYPE_MAC_E;
            break;

        case CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E:

            portType = PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E;
            break;

        default:

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChPortPhysicalPortMapReverseMappingGet(devNum,
        portType,origPortNum,physicalPortNumPtr);
}


/**
* @internal cpssDxChPortPhysicalPortMapReverseMappingGet function
* @endinternal
*
* @brief   Function checks and returns physical port number that mapped to given
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] origPortType             - original port type
* @param[in] origPortNum              - MAC/TXQ/DMA port number
*
* @param[out] physicalPortNumPtr       - (pointer to) physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, origPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_FOUND             - not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_IMPLEMENTED       - when this functionality is not implemented
*/
GT_STATUS cpssDxChPortPhysicalPortMapReverseMappingGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_PORT_MAPPING_TYPE_ENT origPortType,
    IN  GT_U32                          origPortNum,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPhysicalPortMapReverseMappingGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, origPortType, origPortNum, physicalPortNumPtr));

    rc = internal_cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, origPortType,
        origPortNum, physicalPortNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, origPortType, origPortNum, physicalPortNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


static GT_STATUS internal_cpssDxChPortPhysicalPortDetailedMapGet
(
    IN   GT_U8                   devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *tmp_portMapShadowPtr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);
    /* PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum); */

    CPSS_NULL_PTR_CHECK_MAC(portMapShadowPtr);

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum,/*OUT*/&tmp_portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    /*---------------------------------*
     *   copy data                     *
     *---------------------------------*/
    *portMapShadowPtr = *tmp_portMapShadowPtr;
    return GT_OK;
}


/**
* @internal cpssDxChPortPhysicalPortDetailedMapGet function
* @endinternal
*
* @brief   Function returns physical port detailed mapping
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portMapShadowPtr         - (pointer to) detailed mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortPhysicalPortDetailedMapGet
(
    IN   GT_U8                   devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortPhysicalPortDetailedMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portMapShadowPtr));

    rc = internal_cpssDxChPortPhysicalPortDetailedMapGet(devNum,portNum,portMapShadowPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portMapShadowPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

