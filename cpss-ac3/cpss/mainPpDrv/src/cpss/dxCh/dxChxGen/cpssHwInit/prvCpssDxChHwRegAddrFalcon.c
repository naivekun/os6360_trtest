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
* @file prvCpssDxChHwRegAddrFalcon.c
*
* @brief This file implement DB of units base addresses for Falcon.
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#define FALCON_PIPE_MEM_SIZE         0x08000000
#define FALCON_PIPE_0_START_ADDR     0x08000000/*start of pipe 0*/
#define FALCON_PIPE_0_END_ADDR       (FALCON_PIPE_0_START_ADDR + FALCON_PIPE_MEM_SIZE - 4)/* end of pipe 0 */
#define FALCON_PIPE_1_START_ADDR     0x10000000/*start of pipe 1*/
#define FALCON_PIPE_1_END_ADDR       (FALCON_PIPE_1_START_ADDR + FALCON_PIPE_MEM_SIZE - 4)/* end of pipe 1 */

/* offset between tile 0 to tile 1 to tile 2 to tile 3 */
#define FALCON_NUM_PIPES_PER_TILE       2

#define FALCON_PIPE_1_FROM_PIPE_0_OFFSET                            (FALCON_PIPE_1_START_ADDR - FALCON_PIPE_0_START_ADDR)
#define FALCON_DUMMY_PER_PIPE_EMULATE_PIPE_1_FROM_PIPE_0_OFFSET       0x00400000

#define FALCON_D2D_CP_TILE_OFFSET             FALCON_TILE_OFFSET_CNS /*was 0x00080000 in Cider 171111*/
#define FALCON_D2D_CP_BASE_ADDR               0x002E0000             /*was 0x02800000 in Cider 171111*/
#define FALCON_D2D_CP_SIZE                    64                     /* was 512 (K)in Cider 171111 */


/* encapsulate the pipeId and the unitType in the GT_U32     field */
/* pipeId must be >= 1 , unitType is of type PRV_CPSS_DXCH_UNIT_ENT */
#define FALCON_PIPE_OFFSET(pipeId,unitType) ((BIT_0 << ((pipeId) - 1)) + ((unitType)<<(pipeId)))

#define MIRRORED_TILES_BMP_CNS (BIT_1 | BIT_3)/* in Falcon tile 1,3 are mirror image of tile 0 */


#define UNIT___NOT_VALID PRV_CPSS_DXCH_UNIT___NOT_VALID___E

typedef enum{
    UNIT_DUPLICATION_TYPE_PER_PIPE_E,
    UNIT_DUPLICATION_TYPE_PER_TILE_E,
    UNIT_DUPLICATION_TYPE_PER_2_TILES_E,

    UNIT_DUPLICATION_TYPE___NOT_VALID__E

}UNIT_DUPLICATION_TYPE_ENT;

typedef struct{
    UNIT_DUPLICATION_TYPE_ENT   duplicationType;
    PRV_CPSS_DXCH_UNIT_ENT pipe0Id;
    GT_U32                 pipe0baseAddr;
    GT_U32                 sizeInBytes;
    PRV_CPSS_DXCH_UNIT_ENT nextPipeId[MAX_PIPES_CNS];/*must be terminated by UNIT___NOT_VALID or PRV_CPSS_DXCH_UNIT___NOT_VALID___E*/
}UNIT_IDS_IN_PIPES_STC;

#define MG_SIZE (1   * _1M)
#define NUM_MG_PER_CNM   8
/*
NOTE: last alignment according to excel file in:

http://webilsites.marvell.com/sites/EBUSites/Switching/VLSIDesign/ChipDesign/Projects/EAGLE/
Shared%20Documents/Design/AXI/Address%20Space/Eagle_Address_Space_falcon_171111.xlsm
*/
static UNIT_IDS_IN_PIPES_STC falconUnitsIdsInPipes[] =
{
    /****************************************************************/
    /* NOTE: the table is sorted according to the base address ...  */
    /* BUT excluding the 'dummy' GOP addresses                      */
    /****************************************************************/


    /***********************************************************/
    /* per pipe units - addresses point to pipe0 unit instance */
    /***********************************************************/
    /* pipe0Id */                     /*pipe0baseAddr*/  /*sizeInBytes*/ /*pipe 1*/                     /* termination*/
     {UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_LPM_E           ,0x08000000,     16  * _1M,{PRV_CPSS_DXCH_UNIT_LPM_1_E       ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_IPVX_E          ,0x09000000,     8   * _1M,{PRV_CPSS_DXCH_UNIT_IPVX_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_L2I_E           ,0x09800000,     8   * _1M,{PRV_CPSS_DXCH_UNIT_L2I_1_E       ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_CNC_0_E         ,0x0A000000,     256 * _1K,{PRV_CPSS_DXCH_UNIT_CNC_0_1_E     ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_CNC_1_E         ,0x0A040000,     256 * _1K,{PRV_CPSS_DXCH_UNIT_CNC_1_1_E     ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_PCL_E           ,0x0A080000,     512 * _1K,{PRV_CPSS_DXCH_UNIT_IPCL_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EPLR_E          ,0x0A100000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_EPLR_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EOAM_E          ,0x0A200000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_EOAM_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EPCL_E          ,0x0A300000,     512 * _1K,{PRV_CPSS_DXCH_UNIT_EPCL_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_PREQ_E          ,0x0A380000,     512 * _1K,{PRV_CPSS_DXCH_UNIT_PREQ_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_ERMRK_E         ,0x0A400000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_ERMRK_1_E     ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_IOAM_E          ,0x0AB00000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_IOAM_1_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_MLL_E           ,0x0AC00000,     2   * _1M,{PRV_CPSS_DXCH_UNIT_MLL_1_E       ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_IPLR_E          ,0x0AE00000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_IPLR_0_1_E    ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_IPLR_1_E        ,0x0AF00000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_IPLR_1_1_E    ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EQ_E            ,0x0B000000,     16  * _1M,{PRV_CPSS_DXCH_UNIT_EQ_1_E        ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EGF_QAG_E       ,0x0C900000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_EGF_QAG_1_E   ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EGF_SHT_E       ,0x0CA00000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_EGF_SHT_1_E   ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EGF_EFT_E       ,0x0CB00000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EGF_EFT_1_E   ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TTI_E           ,0x0CC00000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_TTI_1_E       ,    UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EREP_E          ,0x0D3F0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EREP_1_E      ,    UNIT___NOT_VALID}    }/* moved from 0x0C800000 */
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_BMA_E           ,0x0D400000,     1   * _1M,{PRV_CPSS_DXCH_UNIT_BMA_1_E       ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_IA_E            ,0x0D510000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_IA_1_E        ,    UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_RXDMA_E         ,0x0D520000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_RXDMA4_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_RXDMA1_E        ,0x0D530000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_RXDMA5_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_RXDMA2_E        ,0x0D540000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_RXDMA6_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_RXDMA3_E        ,0x0D550000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_RXDMA7_E      ,    UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXDMA_E         ,0x0D560000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXDMA4_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TX_FIFO_E       ,0x0D570000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TX_FIFO4_E    ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXDMA1_E        ,0x0D580000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXDMA5_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TX_FIFO1_E      ,0x0D590000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TX_FIFO5_E    ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXDMA2_E        ,0x0D5a0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXDMA6_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TX_FIFO2_E      ,0x0D5b0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TX_FIFO6_E    ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXDMA3_E        ,0x0D5c0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXDMA7_E      ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TX_FIFO3_E      ,0x0D5d0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TX_FIFO7_E    ,    UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E,0x0D600000,     128  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ0_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E,0x0D620000,     128  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ1_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E,0x0D640000,     256  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS0_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E,0x0D680000,     256  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS1_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E,0x0D6c0000,     64   * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC0_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E,0x0D6d0000,     64   * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC1_E,UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E,0x0D700000,     128  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ2_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E,0x0D720000,     128  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ3_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E,0x0D740000,     256  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS2_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E,0x0D780000,     256  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_PDS3_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E,0x0D7c0000,     64   * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC2_E,UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E,0x0D7d0000,     64   * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_QFC3_E,UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E   ,0x0D7F0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EAGLE_D2D_4_E   ,  UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EAGLE_D2D_1_E   ,0x0D800000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EAGLE_D2D_5_E   ,  UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EAGLE_D2D_2_E   ,0x0D810000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EAGLE_D2D_6_E   ,  UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_EAGLE_D2D_3_E   ,0x0D820000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_EAGLE_D2D_7_E   ,  UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_HA_E            ,0x0E000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_HA_1_E          ,  UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_PHA_E           ,0x0C000000,     8   * _1M,{PRV_CPSS_DXCH_UNIT_PHA_1_E         ,  UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI0_E       ,0x0A800000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE1_TAI0_E,  UNIT___NOT_VALID}  }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI1_E       ,0x0A810000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE1_TAI1_E,  UNIT___NOT_VALID}  }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI0_E   ,0x0D6E0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE1_TAI0_E,  UNIT___NOT_VALID}  }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI1_E   ,0x0D7E0000,     64  * _1K,{PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE1_TAI1_E,  UNIT___NOT_VALID}  }

    /*********************************/
    /* per tile units (every 2 pipes)*/
    /*********************************/

    /* pipe0Id */                     /* pipe0baseAddr */  /*sizeInBytes*/ /* termination*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E               ,0x00000000,     8   * _1M ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E               ,0x01000000,     8   * _1M ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E               ,0x02000000,     8   * _1M ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E               ,0x03000000,     8   * _1M ,{UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_0_TAI0_E               ,0x00500000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_0_TAI1_E               ,0x00501000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_1_TAI0_E               ,0x01500000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_1_TAI1_E               ,0x01501000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_2_TAI0_E               ,0x02500000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_2_TAI1_E               ,0x02501000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_3_TAI0_E               ,0x03500000,     4   * _1K ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_RAVEN_3_TAI1_E               ,0x03501000,     4   * _1K ,{UNIT___NOT_VALID}    }


    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TCAM_E                       ,0x18000000,     8   * _1M ,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TXQ_PDX_E                    ,0x18840000,    256  * _1K ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E              ,0x18800000,     64  * _1K ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_1_E              ,0x18810000,     64  * _1K ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E              ,0x18880000,     64  * _1K ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_SHM_E                        ,0x18890000,     64  * _1K ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_EM_E                         ,0x188A0000,     64  * _1K ,{UNIT___NOT_VALID}    }/*EM unit*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_FDB_E                        ,0x188B0000,     64  * _1K ,{UNIT___NOT_VALID}    }/*MT unit*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TXQ_PSI_E                    ,0x18900000,     1   * _1M ,{UNIT___NOT_VALID}    }/* in TILE0_CENTRAL*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E              ,0x19000000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E       ,0x19030000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E  ,0x19040000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E     ,0x19060000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E     ,0x19070000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E   ,0x19080000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E   ,0x19090000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E  ,0x190A0000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E     ,0x190C0000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E     ,0x190D0000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E   ,0x190E0000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E   ,0x190F0000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E     ,0x19100000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E     ,0x19110000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_2_E     ,0x19120000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E     ,0x19130000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_1_E     ,0x19140000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E     ,0x19150000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_1_E     ,0x19160000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_0_E     ,0x19170000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_1_E     ,0x19180000,     64  * _1K ,{UNIT___NOT_VALID}    } /* in TILE0 Packet Buffer*/

    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_DFX_SERVER_E                 ,PRV_CPSS_FALCON_DFX_BASE_ADDRESS_CNS,      1  * _1M ,{UNIT___NOT_VALID}    } /* in TILE0_CENTRAL*/
    /* TAI 0,1 are units per tile */
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TAI_E                        ,0x1BF00000,     64  * _1K,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_TILE_E,PRV_CPSS_DXCH_UNIT_TAI1_E                       ,0x1BF80000,     64  * _1K,{UNIT___NOT_VALID}    }

    /************************************************/
    /* per 2 tiles units (every 4 pipes)            */
    /* tile 0 uses MG_0_0 to MG_0_3 (4 MG units)    */
    /* tile 1 uses MG_1_0 to MG_1_3 (4 MG units)    */
    /************************************************/
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_E            ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*0,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_0_1_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*1,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_0_2_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*2,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_0_3_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*3,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_1_0_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*4,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_1_1_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*5,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_1_2_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*6,     MG_SIZE,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_MG_1_3_E        ,PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS+MG_SIZE*7,     MG_SIZE,{UNIT___NOT_VALID}    }


    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_CNM_RFU_E    ,0x1C100000,     64  * _1K,{UNIT___NOT_VALID}    }
    /* SMI is in the CnM section */
    /* NOTE : SMI 2,3 are in 12.8T device only (in the second CNM unit) */
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_SMI_0_E      ,0x1C110000,     64  * _1K,{UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_SMI_1_E      ,0x1C120000,     64  * _1K,{UNIT___NOT_VALID}    }


    /*Eagle D2D-CP internal        524032        B                00280000        002FFEFF*/
    /*Eagle D2D-CP AMB internal        256       B                002FFF00        002FFFFF*/
    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_0_IN_RAVEN_E    ,FALCON_D2D_CP_BASE_ADDR                          ,    FALCON_D2D_CP_SIZE  * _1K ,{UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_2_TILES_E,PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_1_IN_RAVEN_E    ,FALCON_D2D_CP_BASE_ADDR+FALCON_D2D_CP_TILE_OFFSET,    FALCON_D2D_CP_SIZE  * _1K ,{UNIT___NOT_VALID}    }



#if 0
        /* the 'GOP' base address also used by : */
        GOP_BASE_ADDR_CNS
        /* at file cpssDriver\pp\interrupts\dxExMx\cpssDrvPpIntDefDxChFalcon.c */
#endif
    /***********************************************************/
    /* per pipe units - addresses point to pipe0 unit instance */
    /***********************************************************/
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_GOP_E           ,0x04000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_GOP1_E        ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_MIB_E           ,0x04800000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_MIB1_E        ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_SERDES_E        ,0x05000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_SERDES_1_E    ,    UNIT___NOT_VALID}    }

    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_LED_0_E         ,0x05800000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_LED_2_E       ,    UNIT___NOT_VALID}    }
    ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_LED_1_E         ,0x06000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_LED_3_E       ,    UNIT___NOT_VALID}    }

     /* must be last */
    ,{UNIT_DUPLICATION_TYPE___NOT_VALID__E, PRV_CPSS_DXCH_UNIT_LAST_E, 0,0,{UNIT___NOT_VALID}}
};

/**
* @internal falconOffsetFromPipe0Get function
* @endinternal
*
* @brief   Falcon : (for pipeId != 0).Get The offset (in bytes) between the address
*         of register in pipe 0 of this unit to the register in the other pipe.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] pipeId                   - The PipeId. (in case of unit per tile --> use  = 2tileId)
*                                      NOTE: no meaning in calling the function with 'pipeId = 0'
* @param[in] pipe0Id                  - The unit name in Pipe0
*                                       number : The offset (in bytes) between the address of register in pipe 0 of this unit
*                                       to the register in the other pipe.
*/
static GT_U32   falconOffsetFromPipe0Get(
    IN GT_U32                   pipeId,
    IN PRV_CPSS_DXCH_UNIT_ENT   pipe0Id
)
{
    GT_U32  tileId       = pipeId / FALCON_NUM_PIPES_PER_TILE;
    GT_U32  tileOffset   = tileId * FALCON_TILE_OFFSET_CNS;
    GT_U32  pipeIdFactor = pipeId % FALCON_NUM_PIPES_PER_TILE;/* pipe 0,2,4,6 not need extra offset other than 'tile offset' */
    GT_U32  pipeOffset;
    GT_U32  instanceId;

    if(pipeId == 0)
    {
        /* 'no offset' from pipe 0 */
        return 0;
    }

    switch(pipe0Id)
    {
        case  PRV_CPSS_DXCH_UNIT_TTI_E              :
        case  PRV_CPSS_DXCH_UNIT_PCL_E              :
        case  PRV_CPSS_DXCH_UNIT_L2I_E              :
        case  PRV_CPSS_DXCH_UNIT_IPVX_E             :
        case  PRV_CPSS_DXCH_UNIT_IPLR_E             :
        case  PRV_CPSS_DXCH_UNIT_IPLR_1_E           :
        case  PRV_CPSS_DXCH_UNIT_IOAM_E             :
        case  PRV_CPSS_DXCH_UNIT_MLL_E              :
        case  PRV_CPSS_DXCH_UNIT_EQ_E               :
        case  PRV_CPSS_DXCH_UNIT_EGF_EFT_E          :
        case  PRV_CPSS_DXCH_UNIT_CNC_0_E            :
        case  PRV_CPSS_DXCH_UNIT_CNC_1_E            :
        case  PRV_CPSS_DXCH_UNIT_EGF_SHT_E          :
        case  PRV_CPSS_DXCH_UNIT_HA_E               :
        case  PRV_CPSS_DXCH_UNIT_PHA_E              :
        case  PRV_CPSS_DXCH_UNIT_ERMRK_E            :
        case  PRV_CPSS_DXCH_UNIT_EPCL_E             :
        case  PRV_CPSS_DXCH_UNIT_EPLR_E             :
        case  PRV_CPSS_DXCH_UNIT_EOAM_E             :
        case  PRV_CPSS_DXCH_UNIT_LPM_E              :
        case  PRV_CPSS_DXCH_UNIT_IA_E               :
        case  PRV_CPSS_DXCH_UNIT_EREP_E             :
        case  PRV_CPSS_DXCH_UNIT_PREQ_E             :
        case  PRV_CPSS_DXCH_UNIT_RXDMA_E            :
        case  PRV_CPSS_DXCH_UNIT_RXDMA1_E           :
        case  PRV_CPSS_DXCH_UNIT_RXDMA2_E           :
        case  PRV_CPSS_DXCH_UNIT_RXDMA3_E           :
        case  PRV_CPSS_DXCH_UNIT_TXDMA_E            :
        case  PRV_CPSS_DXCH_UNIT_TXDMA1_E           :
        case  PRV_CPSS_DXCH_UNIT_TXDMA2_E           :
        case  PRV_CPSS_DXCH_UNIT_TXDMA3_E           :
        case  PRV_CPSS_DXCH_UNIT_TX_FIFO_E          :
        case  PRV_CPSS_DXCH_UNIT_TX_FIFO1_E         :
        case  PRV_CPSS_DXCH_UNIT_TX_FIFO2_E         :
        case  PRV_CPSS_DXCH_UNIT_TX_FIFO3_E         :
        case  PRV_CPSS_DXCH_UNIT_BMA_E              :
        case  PRV_CPSS_DXCH_UNIT_EGF_QAG_E          :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E    :
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E    :
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E       :
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_1_E       :
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_2_E       :
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_3_E       :
        case PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI0_E :
        case PRV_CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI1_E :
        case PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI0_E:
        case PRV_CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI1_E:
            pipeOffset = FALCON_PIPE_1_FROM_PIPE_0_OFFSET;
            break;

        /* we are emulating those addresses */
        case  PRV_CPSS_DXCH_UNIT_MIB_E              :
        case  PRV_CPSS_DXCH_UNIT_GOP_E              :
        case  PRV_CPSS_DXCH_UNIT_SERDES_E           :
        case  PRV_CPSS_DXCH_UNIT_LED_0_E            :
        case  PRV_CPSS_DXCH_UNIT_LED_1_E            :
            pipeOffset = FALCON_DUMMY_PER_PIPE_EMULATE_PIPE_1_FROM_PIPE_0_OFFSET;
            break;

        /***********************/
        /* units per '2 tiles' */
        /***********************/
        case  PRV_CPSS_DXCH_UNIT_MG_E              :
        case  PRV_CPSS_DXCH_UNIT_SMI_0_E           :
        case  PRV_CPSS_DXCH_UNIT_SMI_1_E           :
            instanceId   = pipeId / (FALCON_NUM_PIPES_PER_TILE  * 2);
            tileOffset   = instanceId * FALCON_TILE_OFFSET_CNS;
            pipeIdFactor = 0;
            pipeOffset   = 0;
            break;

        case  PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_0_IN_RAVEN_E:/* per 2 tiles but serves tile */
            instanceId   = pipeId / FALCON_NUM_PIPES_PER_TILE;
            tileOffset   = (instanceId / 2) * (2 * FALCON_TILE_OFFSET_CNS);
            pipeIdFactor = 0;
            pipeOffset   = 0;

            tileOffset += (instanceId % 2) * FALCON_D2D_CP_TILE_OFFSET;
            break;

        /********************/
        /* units per 'tile' */
        /********************/
        default:
            /* all those considered to be in PIPE 0 (all those of shared in tile 0 too) !!! */
            /* so offset is '0' from pipe 0 */
            pipeOffset = 0;
            break;
    }

    if((1 << tileId) & MIRRORED_TILES_BMP_CNS)
    {
        /* handle swap of 'per pipe' in the mirrored tiles */
        pipeIdFactor = 1 - pipeIdFactor;
    }

    return tileOffset + (pipeOffset * pipeIdFactor);
}
/*
    structure to help convert Global to Local for DMA and for GOP ports
*/
typedef struct{
    GT_U32    dpIndex;
    GT_U32    localPortInDp;
    GT_U32    pipeIndex;
    GT_U32    localPortInPipe;
}SPECIAL_DMA_PORTS_STC;

#define LOCAL_GOP_CPU_NETWORK_PORT    32
#ifndef GM_USED
    /*The DP that hold MG0*/
    #define MG0_SERVED_DP   4
    /*The DP that hold MG1*/
    #define MG1_SERVED_DP   5
    /*The DP that hold MG2*/
    #define MG2_SERVED_DP   6
    /*The DP that hold MG3*/
    #define MG3_SERVED_DP   7
#else  /*GM_USED ... using single DP[0] (in pipe[0]) */
    #define MG0_SERVED_DP   0
    #define MG1_SERVED_DP   GT_NA
    #define MG2_SERVED_DP   GT_NA
    #define MG3_SERVED_DP   GT_NA
#endif /*GM_USED*/

/*The DP that hold MG4 (swapped to IAS document) */
#define MG4_SERVED_DP    11
/*The DP that hold MG5*/
#define MG5_SERVED_DP    10
/*The DP that hold MG6*/
#define MG6_SERVED_DP     9
/*The DP that hold MG7*/
#define MG7_SERVED_DP     8

/*The DP that hold MG8*/
#define MG8_SERVED_DP    (MG0_SERVED_DP + 16)
/*The DP that hold MG9*/
#define MG9_SERVED_DP    (MG1_SERVED_DP + 16)
/*The DP that hold MG10*/
#define MG10_SERVED_DP   (MG2_SERVED_DP + 16)
/*The DP that hold MG11*/
#define MG11_SERVED_DP   (MG3_SERVED_DP + 16)

/*The DP that hold MG12*/
#define MG12_SERVED_DP   (MG4_SERVED_DP + 16)
/*The DP that hold MG13*/
#define MG13_SERVED_DP   (MG5_SERVED_DP + 16)
/*The DP that hold MG14*/
#define MG14_SERVED_DP   (MG6_SERVED_DP + 16)
/*The DP that hold MG15*/
#define MG15_SERVED_DP   (MG7_SERVED_DP + 16)



static SPECIAL_DMA_PORTS_STC falcon_3_2_SpecialDma_ports[]= {
#ifndef GM_USED
   /*64*/{6,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT/*localPortInPipe*/},/*CPU network port*/

   /*65*/{MG0_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 0*/
   /*66*/{MG1_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 1*/
   /*67*/{MG2_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/}, /*CPU SDMA MG 2*/
   /*68*/{MG3_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/}  /*CPU SDMA MG 3*/
#else  /*GM_USED ... using single DP[0] (in pipe[0]) */
   /* 8*/{MG0_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,0/*pipeIndex*/,GT_NA/*localPortInPipe*/} /*CPU SDMA MG 0*/
#endif /*GM_USED*/
   };
static GT_U32 num_ports_falcon_3_2_SpecialDma_ports =
    NUM_ELEMENTS_IN_ARR_MAC(falcon_3_2_SpecialDma_ports);

static SPECIAL_DMA_PORTS_STC falcon_6_4_SpecialDma_ports[]= {
   /*128*/{ 6,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT/*localPortInPipe*/},/*CPU network port*/
   /*129*/{ 8,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT/*localPortInPipe*/},/*CPU network port*/

   /*130*/{ MG0_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 0*/
   /*131*/{ MG1_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 1*/
   /*132*/{ MG2_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 2*/
   /*133*/{ MG3_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 3*/

   /*134*/{ MG4_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 7*/
   /*135*/{ MG5_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 6*/
   /*136*/{ MG6_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 5*/
   /*137*/{ MG7_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/} /*CPU SDMA MG 4*/
   };
static GT_U32 num_ports_falcon_6_4_SpecialDma_ports =
    NUM_ELEMENTS_IN_ARR_MAC(falcon_6_4_SpecialDma_ports);

static SPECIAL_DMA_PORTS_STC falcon_12_8_SpecialDma_ports[]= {
   /*256*/{ 2,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,0/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT/*localPortInPipe*/},/*CPU network port*/
   /*257*/{ 4,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,LOCAL_GOP_CPU_NETWORK_PORT/*localPortInPipe*/},/*CPU network port*/

   /*258*/{ MG0_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 0*/
   /*259*/{ MG1_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 1*/
   /*260*/{ MG2_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 2*/
   /*261*/{ MG3_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,1/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 3*/

   /*262*/{ MG4_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 4*/
   /*263*/{ MG5_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 5*/
   /*264*/{ MG6_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/}, /*CPU SDMA MG 6*/
   /*265*/{ MG7_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,2/*pipeIndex*/,GT_NA/*localPortInPipe*/}, /*CPU SDMA MG 7*/

   /*266*/{ MG8_SERVED_DP ,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,5/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 8 */
   /*267*/{ MG9_SERVED_DP ,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,5/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 9 */
   /*268*/{ MG10_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,5/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 10*/
   /*269*/{ MG11_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,5/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 11*/

   /*270*/{ MG12_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,6/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 12*/
   /*271*/{ MG13_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,6/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 13*/
   /*272*/{ MG14_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,6/*pipeIndex*/,GT_NA/*localPortInPipe*/},/*CPU SDMA MG 14*/
   /*273*/{ MG15_SERVED_DP,FALCON_PORTS_PER_DP_CNS/*localPortInDp*/,6/*pipeIndex*/,GT_NA/*localPortInPipe*/} /*CPU SDMA MG 15*/


   };
static GT_U32 num_ports_falcon_12_8_SpecialDma_ports =
    NUM_ELEMENTS_IN_ARR_MAC(falcon_12_8_SpecialDma_ports);

#define GOP_OFFSET_BETWEEN_PORTS        0x1000
#define SERDES_OFFSET_BETWEEN_PORTS     0x1000
#define MIB_OFFSET_BETWEEN_PORTS         0x400
#define HWS_BAD_ADDR                    0x11111111

/**
* @internal falconGopRegAddrCalc function
* @endinternal
*
* @brief   Calculate GOP/MIB/SERDES base address in Falcon
*
* @param[in] devNum                   - device Id
* @param[in] macNum                   - global GOP/MIB/SERDES number
* @param[in] unitId                   - the unit id (GOP/MIB/SERDES)
* @param[in] perPortOffset            - offset in bytes between ports
*                                       base address of the SERDES
*/
static GT_U32 falconGopRegAddrCalc
(
    IN GT_U8     devNum,
    IN GT_U32    macNum,
    IN PRV_CPSS_DXCH_UNIT_ENT     unitId,
    IN GT_U32    perPortOffset
)
{
    GT_STATUS   rc;
    GT_U32 pipeIndex;
    GT_U32 localMacPortNum;
    GT_U32 baseAddr,pipeOffset;

    /* convert the global MAC to pipeId and local MAC */
    rc = prvCpssFalconGopGlobalMacPortNumToLocalMacPortInPipeConvert(devNum,
        macNum,&pipeIndex,&localMacPortNum);
    if(rc != GT_OK)
    {
        /* not valid address */
        return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }

    baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);
    pipeOffset = falconOffsetFromPipe0Get(pipeIndex,unitId);

    return baseAddr + (perPortOffset * localMacPortNum) + pipeOffset;
}




/**
* @internal falconUnitBaseAddrCalc function
* @endinternal
*
* @brief   function for HWS to call to cpss to use for Calculate the base address
*         of a global port, for next units :
*         MIB/SERDES/GOP
* @param[in] unitId                   - the unit : MIB/SERDES/GOP
* @param[in] portNum                  - the global port num
*                                       The address of the port in the unit
*/
GT_UREG_DATA   falconUnitBaseAddrCalc(
    GT_U8                           devNum,
    HWS_UNIT_BASE_ADDR_TYPE_ENT     unitId,
    GT_UOPT                         portNum
)
{
    GT_STATUS   rc;
    GT_U32 localD2d, localRaven, tileNum, tileOffset, baseAddr;
    GT_U32 relativeAddr, localPortIndex, ravenNum;
    GT_U32 dpIndex,localDmaNumber;

    switch(unitId)
    {
        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E:
            rc = prvCpssFalconDmaGlobalNumToLocalNumInDpConvert(devNum,
                portNum,&dpIndex,&localDmaNumber);
            if(rc != GT_OK)
            {
                return HWS_BAD_ADDR;
            }

            if(localDmaNumber == FALCON_PORTS_PER_DP_CNS)
            {
                /* The 'CPU port' (port 16 of the Raven) */
                relativeAddr = 0x00515000;
                ravenNum = dpIndex / 2;/* 2 DP[] per Raven */
            }
            else
            {
                /*/Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Device Units/<GOP TAP 0>GOP TAP 0/<GOP TAP 0> <MPF>MPF/<MPF> <TSU IP> PTP IP/TSU IP Units %a %b*/
                localPortIndex = portNum  % 16;
                ravenNum = portNum  / 16;

                relativeAddr = 0x00405000 + 0x00080000 * ((localPortIndex) / 8) + 0x00004000 * (localPortIndex  % 8);
            }

            localRaven = ravenNum % 4;
            tileNum    = ravenNum / 4;

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            return baseAddr + tileOffset + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E:  /* 0..15 */
            localRaven = portNum % 4;
            tileNum = portNum  / 4;

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);
            return baseAddr + tileOffset;

        case HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E:   /* 0..31 */
            localD2d = portNum % 8;
            tileNum = portNum  / 8;

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localD2d = 7 - localD2d;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E + localD2d), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);
            return baseAddr + tileOffset;

        case HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E:   /* 0..31 */
            localRaven = (portNum % 8) / 2;
            tileNum = portNum  / 8;

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = 0x00680000 + prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            if(portNum % 2 == 1)
            {
                baseAddr += 0x00010000; /* offset of D2D_1 relating to D2D_0 */
            }
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);
            return baseAddr + tileOffset;

        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI64_MAC_E:   /* 0..255 */
            localPortIndex = portNum  % 16;
            ravenNum = portNum  / 16;
            localRaven = ravenNum % 4;
            tileNum = ravenNum  / 4;

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            if(localPortIndex < 8)
            {
                relativeAddr = 0 + localPortIndex * 0x1000;
            }
            else
            {
                relativeAddr = 0x00080000 + (localPortIndex - 8) * 0x1000;
            }
            relativeAddr += 0x00444000; /* start address of MAC64 unit */

            return baseAddr + tileOffset + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI400_MAC_E:   /* 0..255 */
            localPortIndex = portNum  % 16;
            ravenNum = portNum  / 16;
            localRaven = ravenNum % 4;
            tileNum = ravenNum  / 4;

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            switch(localPortIndex)
            {
                case 0:
                    relativeAddr = 0;
                    break;
                case 4:
                    relativeAddr = 0x00001000;
                    break;
                case 8:
                    relativeAddr = 0x00080000;
                    break;
                case 12:
                    relativeAddr = 0x00081000;
                    break;
                default:
                    return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            relativeAddr += 0x00440000; /* start address of MAC400 unit */

            return baseAddr + tileOffset + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MAC_STATISTICS_E:
            localPortIndex = portNum  % 16;
            ravenNum = portNum  / 16;
            localRaven = ravenNum % 4;
            tileNum = ravenNum  / 4;

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            relativeAddr = 0x00450000 + 0x80000 * (localPortIndex / 8);

            return baseAddr + tileOffset + relativeAddr;

        case HWS_UNIT_BASE_ADDR_TYPE_GOP_E:
            return falconGopRegAddrCalc(devNum,
                portNum,PRV_CPSS_DXCH_UNIT_GOP_E,
                GOP_OFFSET_BETWEEN_PORTS);

        case HWS_UNIT_BASE_ADDR_TYPE_MIB_E:
            return (GT_UREG_DATA)falconGopRegAddrCalc(devNum,
                portNum,PRV_CPSS_DXCH_UNIT_MIB_E,
                MIB_OFFSET_BETWEEN_PORTS);

        case HWS_UNIT_BASE_ADDR_TYPE_SERDES_E:   /* 0..255 */
            localPortIndex = portNum  % 16;
            ravenNum = portNum  / 16;
            localRaven = ravenNum % 4;
            tileNum = ravenNum  / 4;

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            relativeAddr = 0x00340000 + localPortIndex * 0x1000;

            return baseAddr + tileOffset + relativeAddr;
       case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MSDB_E:
           ravenNum = portNum  / 16;
           localRaven = ravenNum % 4;
           tileNum = ravenNum  / 4;
           localPortIndex = portNum  % 16;
            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }
            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);
            relativeAddr = 0x508000; /* start address of MSDB unit */
            relativeAddr+= 0x1000 * (localPortIndex/8);
           return  baseAddr + tileOffset+relativeAddr ;
         break;

        case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_EXT_E:
            rc = prvCpssFalconDmaGlobalNumToLocalNumInDpConvert(devNum,
                portNum,&dpIndex,&localDmaNumber);
            if(rc != GT_OK)
            {
                return HWS_BAD_ADDR;
            }

            if(localDmaNumber == FALCON_PORTS_PER_DP_CNS)
            {
                /* The 'CPU port' (port 16 of the Raven) is in the separate MTIP CPU EXT Units */
                relativeAddr = 0x0051C000;
                ravenNum = dpIndex / 2;/* 2 DP[] per Raven */
            }
            else
            {
                localPortIndex = portNum  % 16;
                ravenNum = portNum  / 16;

                relativeAddr = 0x00478000 + 0x00080000 * ((localPortIndex) / 8);
            }

            localRaven = ravenNum % 4;
            tileNum    = ravenNum / 4;

            if(0 != ((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
            {
                /* the tiles 1,3 : need 'mirror' conversion */
                localRaven = 3 - localRaven;
            }

            baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + localRaven), NULL);
            tileOffset = prvCpssSip6TileOffsetGet(devNum, tileNum);

            return baseAddr + tileOffset + relativeAddr;

        default:
            break;
    }
    /* indicate error */
    return PRV_CPSS_SW_PTR_ENTRY_UNUSED;
}

/**
* @internal prvDxChHwRegAddrFalconDbInit function
* @endinternal
*
* @brief   init the base address manager of the Falcon device.
*         prvDxChFalconUnitsIdUnitBaseAddrArr[]
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvDxChHwRegAddrFalconDbInit(GT_VOID)
{
    UNIT_IDS_IN_PIPES_STC   *currEntryPtr = &falconUnitsIdsInPipes[0];
    GT_U32                  pipeOffset;
    GT_U32                  globalIndex = 0;/* index into prvDxChFalconUnitsIdUnitBaseAddrArr */
    GT_U32                  ii,jj,kk;
    GT_U32                  tile0_startGlobalIndex,tile0_endGlobalIndex;
    GT_U32                  errorCase = 0;


    /* bind HWS with function that calc base addresses of units */
    hwsFalconUnitBaseAddrCalcBind(falconUnitBaseAddrCalc);

    for(ii = 0 ; currEntryPtr->pipe0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,currEntryPtr++)
    {
        tile0_startGlobalIndex = globalIndex;

        if(globalIndex >= FALCON_MAX_UNITS)
        {
            errorCase = 1;
            goto notEnoughUnits_lbl;
        }

        if(currEntryPtr->pipe0Id == PRV_CPSS_DXCH_UNIT_EREP_E &&
           prvCpssDxchFalconCiderVersionGet() == PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E)
        {
            currEntryPtr->pipe0baseAddr = 0x0C800000;
        }
        else
        if(currEntryPtr->pipe0Id == PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_0_IN_RAVEN_E &&
           prvCpssDxchFalconCiderVersionGet() == PRV_CPSS_DXCH_FALCON_CIDER_VERSION_17_11_11_E)
        {
            /* Changed D2D_CP registers base address */

            currEntryPtr->pipe0baseAddr = 0x02800000;
            currEntryPtr->sizeInBytes   = 512 * _1K;

            /*PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_1_IN_RAVEN_E */
            (currEntryPtr+1)->pipe0baseAddr = 0x02800000 + 0x00080000;
            (currEntryPtr+1)->sizeInBytes   = 512 * _1K;
        }


        /* add the unit in pipe 0 */
        prvDxChFalconUnitsIdUnitBaseAddrArr[globalIndex].unitId = currEntryPtr->pipe0Id;
        prvDxChFalconUnitsIdUnitBaseAddrArr[globalIndex].unitBaseAdrr = currEntryPtr->pipe0baseAddr;
        globalIndex++;

        /****************************/
        /* support multi-pipe units */
        /****************************/
        if(currEntryPtr->duplicationType == UNIT_DUPLICATION_TYPE_PER_PIPE_E)
        {
            /*jj is pipeId-1*/
            for(jj = 0;currEntryPtr->nextPipeId[jj] != UNIT___NOT_VALID ; jj++)
            {
                if(globalIndex >= FALCON_MAX_UNITS)
                {
                    errorCase = 2;
                    goto notEnoughUnits_lbl;
                }

                pipeOffset = falconOffsetFromPipe0Get((jj+1),currEntryPtr->pipe0Id);
                /* add the units in other pipes */
                prvDxChFalconUnitsIdUnitBaseAddrArr[globalIndex].unitId = currEntryPtr->nextPipeId[jj];
                prvDxChFalconUnitsIdUnitBaseAddrArr[globalIndex].unitBaseAdrr = currEntryPtr->pipe0baseAddr + pipeOffset;
                globalIndex++;
            }
        }

        tile0_endGlobalIndex = globalIndex;
        /****************************/
        /* support multi-tile units */
        /****************************/
        /* start from tile 1 , because tile 0 we already have */
        for(jj = 1 ; jj < 4/*tiles*/ ; jj++)
        {
            if(currEntryPtr->duplicationType == UNIT_DUPLICATION_TYPE_PER_2_TILES_E &&
                (jj & 1))/* odd tile */
            {
                /* unit exits in tile 0 and tile 2 */
                continue;
            }

            /* duplicate all units of 'tile 0' into those for tile 1..3 */
            for(kk = tile0_startGlobalIndex ; kk < tile0_endGlobalIndex; kk++)
            {
                if(globalIndex >= FALCON_MAX_UNITS)
                {
                    errorCase = 3;
                    goto notEnoughUnits_lbl;
                }

                prvDxChFalconUnitsIdUnitBaseAddrArr[globalIndex].unitId       =
                    prvDxChFalconUnitsIdUnitBaseAddrArr[kk].unitId +    /* unitId in tile 0 */
                    PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS * jj; /* unitId offset from tile 0 */

                prvDxChFalconUnitsIdUnitBaseAddrArr[globalIndex].unitBaseAdrr =
                    prvDxChFalconUnitsIdUnitBaseAddrArr[kk].unitBaseAdrr + /* address in tile 0 */
                    FALCON_TILE_OFFSET_CNS * jj;                           /* address offset from tile 0 */


                globalIndex++;
            }
        }
    }

    /* set the 'last index as invalid' */
    prvDxChFalconUnitsIdUnitBaseAddrArr[globalIndex].unitId = PRV_CPSS_DXCH_UNIT_LAST_E;
    prvDxChFalconUnitsIdUnitBaseAddrArr[globalIndex].unitBaseAdrr = NON_VALID_ADDR_CNS;
    globalIndex++;

    if(globalIndex >= FALCON_MAX_UNITS)
    {
        errorCase = 4;
        goto notEnoughUnits_lbl;
    }

    /* avoid warning if 'CPSS_LOG_ENABLE' not defined
      (warning: variable 'errorCase' set but not used [-Wunused-but-set-variable])
    */
    if(errorCase == 0)
    {
        errorCase++;
    }

    return GT_OK;

notEnoughUnits_lbl:
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "error case [%d] :FALCON_MAX_UNITS is [%d] but must be at least[%d] \n",
        errorCase,FALCON_MAX_UNITS , globalIndex+1);
}
/**
* @internal prvCpssDxChFalconHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
PRV_CPSS_DXCH_UNIT_ENT prvCpssDxChFalconHwRegAddrToUnitIdConvert
(
    IN GT_U32                   devNum,
    IN GT_U32                   regAddr
)
{
    GT_U32 pipeId;
    GT_U32 pipeOffset;
    GT_U32 addrInPipe0; /*address in pipe 0 of the register */
    UNIT_IDS_IN_PIPES_STC   *currEntryPtr;
    GT_U32  ii;
    GT_U32  tileId;
    PRV_CPSS_DXCH_UNIT_ENT  tileStartUnits;

    tileId = regAddr / FALCON_TILE_OFFSET_CNS;

    /******************************************/
    /* strip the tile offset from the regAddr */
    /******************************************/
    if(tileId)
    {
        if(tileId >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(PRV_CPSS_DXCH_UNIT_LAST_E,
                "address [0x%8.8x] is not supported by the device \n",
                regAddr);
        }

        regAddr       -= tileId*FALCON_TILE_OFFSET_CNS;
        tileStartUnits =  tileId * PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E;
    }
    else
    {
        tileStartUnits =  0;
    }

    if(regAddr >= FALCON_PIPE_0_START_ADDR && regAddr <= FALCON_PIPE_0_END_ADDR)
    {
        pipeId = 0;
        /* the address of pipe 0 hold 'clone' in pipe 1 */
        pipeOffset = FALCON_PIPE_1_FROM_PIPE_0_OFFSET;
    }
    else
    if(regAddr >= FALCON_PIPE_1_START_ADDR && regAddr <= FALCON_PIPE_1_END_ADDR)
    {
        pipeId = 1;
        /* the address already in pipe 1 ... */
        pipeOffset = FALCON_PIPE_1_FROM_PIPE_0_OFFSET;
    }
    else
    if((regAddr >= 0x04000000 && regAddr <= 0x07FFFFFC))
    {
        /* support dummy range of :
        dummy (expected in raven)
        ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_GOP_E           ,0x04000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_GOP1_E        ,    UNIT___NOT_VALID}    }
        ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_MIB_E           ,0x04800000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_MIB1_E        ,    UNIT___NOT_VALID}    }
        ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_SERDES_E        ,0x05000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_SERDES_1_E    ,    UNIT___NOT_VALID}    }

        ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_LED_0_E         ,0x05800000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_LED_2_E       ,    UNIT___NOT_VALID}    }
        ,{UNIT_DUPLICATION_TYPE_PER_PIPE_E,PRV_CPSS_DXCH_UNIT_LED_1_E         ,0x06000000,     4   * _1M,{PRV_CPSS_DXCH_UNIT_LED_3_E       ,    UNIT___NOT_VALID}    }
        */
        if(regAddr & FALCON_DUMMY_PER_PIPE_EMULATE_PIPE_1_FROM_PIPE_0_OFFSET)
        {
            pipeId = 1;
        }
        else
        {
            pipeId = 0;
        }

        pipeOffset = FALCON_DUMMY_PER_PIPE_EMULATE_PIPE_1_FROM_PIPE_0_OFFSET;
    }
    else
    {
        /* in shared memory (per tile) */
        pipeId = 0;
        pipeOffset = 0;
    }

    addrInPipe0 = regAddr - pipeId * pipeOffset;
    currEntryPtr = &falconUnitsIdsInPipes[0];

    /****************************************/
    /* search for addrInPipe0 in the ranges */
    /****************************************/
    for(ii = 0 ; currEntryPtr->pipe0Id != PRV_CPSS_DXCH_UNIT_LAST_E ;ii++,currEntryPtr++)
    {
        /* add the unit in pipe 0 */
        if(addrInPipe0 >= currEntryPtr->pipe0baseAddr  &&
           addrInPipe0 <  (currEntryPtr->pipe0baseAddr + currEntryPtr->sizeInBytes))
        {
            /* found the proper range */

            if(pipeId == 0)
            {
                return currEntryPtr->pipe0Id + tileStartUnits;
            }

            return currEntryPtr->nextPipeId[pipeId-1] + tileStartUnits;
        }
    }

    /* not found ! */
    return PRV_CPSS_DXCH_UNIT_LAST_E;
}

/* number of rx/tx DMA and txfifo 'per DP unit' */
#define FALCON_NUM_PORTS_DMA   (FALCON_PORTS_PER_DP_CNS + 1)

/*check if register address is per port in RxDMA unit */
static GT_BOOL  falcon_mustNotDuplicate_rxdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {

         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.cutThrough.channelCTConfig                ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.channelToLocalDevSourcePort ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.PCHConfig                   ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.channelGeneralConfigs       ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_rxDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfig.channelCascadePort          ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(rxDMA[NON_FIRST_UNIT_INDEX_CNS].globalRxDMAConfigs.preIngrPrioritizationConfStatus.channelPIPConfigReg), FALCON_NUM_PORTS_DMA}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };
    static GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = 0;
    if(addrOffset_NON_FIRST_UNIT_INDEX_CNS == 0)
    {
        /* need to set it once */
        addrOffset_NON_FIRST_UNIT_INDEX_CNS =
            prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA1_E,NULL) -
            prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_RXDMA_E,NULL)  ;

    }

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,addrOffset_NON_FIRST_UNIT_INDEX_CNS,perPortAddrArr,regAddr);
}


/*check if register address is per port in TxDMA unit */
static GT_BOOL  falcon_mustNotDuplicate_txdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.speedProfile                 ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.channelReset                 ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.descFIFOBase                 ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.interPacketRateLimiterConfig ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.interCellRateLimiterConfig   ), FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txDMA[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.inerPacketRateLimiterConfig  ), FALCON_NUM_PORTS_DMA}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };
    static GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = 0;
    if(addrOffset_NON_FIRST_UNIT_INDEX_CNS == 0)
    {
        /* need to set it once */
        addrOffset_NON_FIRST_UNIT_INDEX_CNS =
            prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA1_E,NULL) -
            prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXDMA_E,NULL)  ;
    }

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,addrOffset_NON_FIRST_UNIT_INDEX_CNS,perPortAddrArr,regAddr);
}

/*check if register address is per port in TxFifo unit */
static GT_BOOL  falcon_mustNotDuplicate_txfifoAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.speedProfile ),  FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.descFIFOBase ),  FALCON_NUM_PORTS_DMA}
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(sip6_txFIFO[NON_FIRST_UNIT_INDEX_CNS].configs.channelConfigs.tagFIFOBase  ),  FALCON_NUM_PORTS_DMA}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    static GT_U32   addrOffset_NON_FIRST_UNIT_INDEX_CNS = 0;
    if(addrOffset_NON_FIRST_UNIT_INDEX_CNS == 0)
    {
        /* need to set it once */
        addrOffset_NON_FIRST_UNIT_INDEX_CNS =
            prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,NULL) -
            prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TX_FIFO_E,NULL)  ;
    }
    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,addrOffset_NON_FIRST_UNIT_INDEX_CNS,perPortAddrArr,regAddr);
}

/* check if register address is per tile*/
static GT_BOOL  falcon_mustDuplicate_pdxAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
    {
        static PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC tableArr[] =
        {         /* table type */                                        /* number of entries */
            {CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,    0/*take from tabled DB*/},

            /* must be last */
            {CPSS_DXCH_TABLE_LAST_E , 0}
        };

        return prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,tableArr,regAddr) ;
    }



static GT_BOOL  falcon_mustDuplicate_qfcAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
    {
        static PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC tableArr[] =
        {         /* table type */                                        /* number of entries */
            {CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_TC_PFC_THRESHOLD_E,    0/*take from tabled DB*/},
            {CPSS_DXCH_SIP6_TXQ_QFC_TC_PFC_HYSTERESIS_CONF_E,    0/*take from tabled DB*/},
            /* must be last */
            {CPSS_DXCH_TABLE_LAST_E , 0}
        };



    return prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,tableArr,regAddr) ;

}





/* check if register address is per port in TTI[0] unit AND must not be duplicated to TTI[1] */
static GT_BOOL  falcon_mustNotDuplicate_ttiAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC perPortAddrArr[]=
    {
       /* no need to duplicate those registers in DB ... the caller will always call
          specific to portGroup , and without 'multiple' port groups
          {REG_ARR_OFFSET_FROM_REG_DB_1_STC_MAC(TTI.phyPortAndEPortAttributes.myPhysicalPortAttributes) , 288}
       */
         /* must be last */
        {END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,perPortAddrArr,regAddr);
}

/* SHM : check if register address is not to be duplicated */
static GT_BOOL  falcon_mustNotDuplicate_shmAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC regAddrArr[]=
    {
         {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.lpm_illegal_address), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.fdb_illegal_address), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.em_illegal_address ), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.arp_illegal_address), 1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(SHM.lpm_aging_illegal_address), 1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,regAddrArr,regAddr);
}



/* the LPM table duplicated per tile */
/* the pipe 0,1 duplicated by HW (simulation share it) */
/* the pipe 2,3 duplicated by HW (simulation share it) */
/* the pipe 4,5 duplicated by HW (simulation share it) */
/* the pipe 6,7 duplicated by HW (simulation share it) */
static GT_BOOL  falcon_mustNotDuplicatePerPipe_lpmMemAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32       regAddr
)
{
    static PRV_CPSS_DXCH_HW_RED_ADDR_OFFSET_IN_TABLE_INFO_STC tableArr[] =
    {         /* table type */                                        /* number of entries */
        /* the LPM table is duplicated by the HW (but not by simulation)
           within the single tile */
        /* but the SW need to duplicate the table per tile !!! */
        {CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,    0/*take from tabled DB*/}

        /* must be last */
        ,{CPSS_DXCH_TABLE_LAST_E , 0}
    };

    return prvCpssDxChHwRegAddrIsAddrInTableArray(devNum,tableArr,regAddr) ;
}

/* check if register address is one of FUQ related ... need special treatment */
static GT_BOOL  falcon_fuqSupport_mgAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC fuqRegAddrArr[]=
    {
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.fuQControl) , 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.fuQBaseAddr), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auqConfig_generalControl), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auqConfig_hostConfig    ), 1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,fuqRegAddrArr,regAddr);
}

/* check if register address is one of AUQ related ... need special treatment */
static GT_BOOL  falcon_auqSupport_mgAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC auqRegAddrArr[]=
    {
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auQControl) , 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.auQBaseAddr), 1}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,auqRegAddrArr,regAddr);
}

/* check if register address is one 'per MG' SDMA , and allow to convert address
   of MG 0 to address of other MG  */
static GT_BOOL  falcon_convertPerMg_mgSdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr[]=
    {
         /* cover full array of : PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs */
         /* allow MG[1] till MG[15] to be able to use MG[0] addresses !        */

                    /* 4 bits per queue , no global bits */
/*per queue*/        {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.rxQCmdReg ), 1}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaCdp)}

                    /* 2 bits per queue , no global bits */
/*per queue*/       ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.txQCmdReg             ), 1}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txDmaCdp)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaResErrCnt)}/* the same as rxSdmaResourceErrorCountAndMode[0,1]*/
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaPcktCnt)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.rxDmaByteCnt)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txQWrrPrioConfig)}

                    /* 1 bit per queue , no global bits */
/*per queue*/       ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.txQFixedPrioConfig    ), 1}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaTokenBucketQueueCnt)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaTokenBucketQueueConfig)}

/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaPacketGeneratorConfigQueue)}
/*per queue*/       ,{REG_ARR_OFFSET_FROM_REG_DB_AND_SIZE_STC_MAC(sdmaRegs.txSdmaPacketCountConfigQueue)}
                    /* the rxSdmaResourceErrorCountAndMode[0..7] not hold consecutive addresses !
                       need to split to 2 ranges [0..1] and [2..7] */
/*per queue: 0..1*/ ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.rxSdmaResourceErrorCountAndMode[0]) , 2}
/*per queue: 2..7*/ ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.rxSdmaResourceErrorCountAndMode[2]) , 6}



         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,globalRegAddrArr,regAddr);
}

/* check if register address is one MG 'per MG' for SDMA that need duplication  ... need special treatment */
static GT_BOOL  falcon_allowDuplicatePerMg_mgSdmaAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    /* using registers from regDb */
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr[]=
    {
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.sdmaCfgReg)                          ,1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(sdmaRegs.txSdmaWrrTokenParameters )           ,1}
         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    /* using registers from regDb1 */
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr_regDb1[]=
    {
         {REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[0])        ,1}/*array of 6 but NOT consecutive addresses*/
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[0] )        ,1}/*array of 6 but NOT consecutive addresses*/
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[1])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[1] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[2])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[2] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[3])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[3] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[4])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[4] )        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.baseAddress[5])        ,1}
        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.windowSize[5] )        ,1}

        ,{REG_ARR_OFFSET_FROM_REG_DB_1_AND_SIZE_STC_MAC(MG.addressDecoding.highAddressRemap)  }/*array of 6 consecutive addresses*/
        ,{REG_ARR_OFFSET_FROM_REG_DB_1_AND_SIZE_STC_MAC(MG.addressDecoding.windowControl)     }/*array of 6 consecutive addresses*/

        ,{REG_SINGLE_OFFSET_FROM_REG_DB_1_STC_MAC(MG.addressDecoding.UnitDefaultID)         ,1}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    if(GT_TRUE == prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,globalRegAddrArr,regAddr))
    {
        return GT_TRUE;
    }

    return prvCpssDxChHwRegAddrIsAddrInRegDb1Array(devNum,0,globalRegAddrArr_regDb1,regAddr);
}

/* check if register address is one MG 'per tile' that need duplication  ... need special treatment */
static GT_BOOL  falcon_mustDuplicatePerTile_mgAddr(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               regAddr
)
{
    static PRV_CPSS_DXCH_HW_REG_ADDR_OFFSET_IN_REG_DB_INFO_STC globalRegAddrArr[]=
    {
/*xsmi-TBD*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.XSMI.XSMIManagement    ), 1}*/
/*xsmi-TBD*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.XSMI.XSMIAddress       ), 1}*/
/*xsmi-TBD*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.XSMI.XSMIConfiguration ), 1}*/
/*not relevant*/    /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.addrCompletion), 1}*/
         {&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.sampledAtResetReg     ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.metalFix              ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.globalControl         ), 1}
/*special treatment*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.fuQBaseAddr           ), 1}*/
/*special treatment*/        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.fuQControl            ), 1}*/
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.interruptCoalescing   ), 1}
        /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(MG.globalRegs.lastReadTimeStampReg  ), 1}*/
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.extendedGlobalControl ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.generalConfigurations ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.genxsRateConfig       ), 1}
        ,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(twsiReg.serInitCtrl   ), 1}
/*not dup*/ /*,{&REG_ARR_OFFSET_FROM_REG_DB_STC_MAC(globalRegs.dummyReadAfterWriteReg), 1}*/

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    return prvCpssDxChHwRegAddrIsAddrInRegDbArray(devNum,0,globalRegAddrArr,regAddr);
}



static GT_BOOL mgUnitDuplicatedMultiPortGroupsGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    INOUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    IN    PRV_CPSS_DXCH_UNIT_ENT  unitId,
    INOUT PRV_CPSS_DXCH_UNIT_ENT  usedUnits[/*PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS*/],
    INOUT GT_U32                  *unitsIndexPtr,
    INOUT GT_U32                  *additionalBaseAddrPtr,
    INOUT GT_U32                  *dupIndexPtr,
    OUT   GT_BOOL                  *dupWasDonePtr,
    OUT   GT_BOOL                  *unitPerPipePtr,
    OUT   GT_BOOL                  *unitPer2TilesPtr,
    OUT   GT_BOOL                  *unitPerMgPtr
)
{
    GT_U32  ii;
    GT_U32  stepTiles;
    GT_U32  relativeRegAddr,pipe0Addr;
    GT_U32  tileId;
    GT_U32  unitsIndex    = *unitsIndexPtr;
    GT_U32  dupIndex      = *dupIndexPtr;
    GT_BOOL dupWasDone    = GT_FALSE;
    GT_BOOL unitPerPipe   = GT_FALSE;
    GT_BOOL unitPer2Tiles = GT_FALSE;
    GT_BOOL unitPerMg     = GT_FALSE;
    GT_U32  tileOffset;
    GT_U32  pipeId,portGroupIndex;
    GT_U32  mgId,isSdmaRegister = 0,isSdmaRegisterAllowDup = 0;


    if(GT_TRUE == falcon_fuqSupport_mgAddr(devNum, regAddr))
    {
        /* FUQ registers need special manipulations */

        /* NOTE: we assume that every access to WRITE to those register is done
           using 'port group' indication that is not 'unaware' !!!! */
        /* we allow 'read' / 'check status' on all port groups */

        /* in tile 0 : we get here with the address of 'MG_0_0' but we may need to
           convert it to address of 'MG_0_1'
           in tile 1 : need to convert to 'MG_1_0' or 'MG_1_1'
           in tile 2 : need to add '2 tiles' offset (like in tile 0)
           in tile 3 : need to add '2 tiles' offset (like in tile 1)
        */
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_1_E;/*for pipe 1*/
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_0_E;/*for pipe 2*/
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_1_E;/*for pipe 3*/

        relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        portGroupIndex = 0;
        pipeId = 1;
        /* support the tiles 0,1 */
        /* the additionalBaseAddrPtr[] not holding the 'orig regAddr' so we skip index 0 */
        for(ii = 1; ii < unitsIndex ; ii++)
        {
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */

            additionalRegDupPtr->portGroupsArr[portGroupIndex++] = pipeId++; /*per pipe*/
        }

        stepTiles = 2;
        /* support the tiles 2,3 */
        for(tileId = stepTiles ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ; tileId += stepTiles)
        {
            /* tile offset */
            tileOffset = prvCpssSip6TileOffsetGet(devNum,tileId);

            for(ii = 0; ii < unitsIndex ; ii++)
            {
                pipe0Addr = ii == 0 ?
                    regAddr :/* index 0 is 'orig' regAddr */
                    additionalBaseAddrPtr[ii-1];/* ii = 0.. unitsIndex-1 hold
                                                   address of the dup units in pipes 0,1,2,3 */

                additionalBaseAddrPtr[dupIndex++] = pipe0Addr + tileOffset;

                additionalRegDupPtr->portGroupsArr[portGroupIndex++] = pipeId++;/* pipe 4..7 */
            }
        }

        dupWasDone  = GT_TRUE;
        unitPerPipe = GT_TRUE;
    }
    else
    if(GT_TRUE == falcon_auqSupport_mgAddr(devNum, regAddr) ||
       GT_TRUE == falcon_mustDuplicatePerTile_mgAddr(devNum, regAddr) )
    {
        /* AUQ registers need special manipulations */
        /* NOTE: we assume that every access to WRITE to those register is done
           using 'port group' indication that is not 'unaware' !!!! */
        /* we allow 'read' / 'check status' on all port groups */

        /* in tile 1 : we get here with the address of 'MG_0_0' but we may need to
           convert it to address of 'MG_1_0'
           in tile 2 : need to add '2 tiles' offset (like in tile 0)
           in tile 3 : need to add '2 tiles' offset (like in tile 1)
        */
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_0_E;/*for tile 1*/

        relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        portGroupIndex = 0;
        pipeId = 2;

        /* support the tiles 0,1 */
        /* the additionalBaseAddrPtr[] not holding the 'orig regAddr' so we skip index 0 */
        for(ii = 1; ii < unitsIndex ; ii++)
        {
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */

            additionalRegDupPtr->portGroupsArr[portGroupIndex++] = pipeId; /*2 pipes per tile*/

            pipeId+=2;
        }

        stepTiles = 2;
        /* support the tiles 2,3 */
        for(tileId = stepTiles ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ; tileId += stepTiles)
        {
            /* tile offset */
            tileOffset = prvCpssSip6TileOffsetGet(devNum,tileId);

            for(ii = 0; ii < unitsIndex ; ii++)
            {
                pipe0Addr = ii == 0 ?
                    regAddr :/* index 0 is 'orig' regAddr */
                    additionalBaseAddrPtr[ii-1];/* ii = 0.. unitsIndex-1 hold
                                                   address of the dup units in tile 0,1 */

                additionalBaseAddrPtr[dupIndex++] = pipe0Addr + tileOffset;

                additionalRegDupPtr->portGroupsArr[portGroupIndex++] = pipeId;/* tiles 2,3 */ /*2 pipes per tile*/
                pipeId+=2;
            }
        }

        dupWasDone  = GT_TRUE;
        unitPer2Tiles = GT_TRUE;
    }
    else
    if (GT_TRUE == falcon_convertPerMg_mgSdmaAddr(devNum, regAddr))
    {
        /* allow MG[1] till MG[15] to be able to use MG[0] addresses !        */
        isSdmaRegister = 1;
    }
    else
    if (GT_TRUE == falcon_allowDuplicatePerMg_mgSdmaAddr(devNum, regAddr))
    {
        isSdmaRegister = 1;
        isSdmaRegisterAllowDup = 1;
    }
    else
    {
        return GT_FALSE;
    }

    if(isSdmaRegister)
    {

        if(!isSdmaRegisterAllowDup &&
           portGroupId >= PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits)
        {
            /* for those registers we NOT allow 'unaware mode' and NOT allow
               duplications.

               the caller must be specific about the needed 'MG unit' ...
               otherwise it will duplicate it to unneeded places !!!
            */
            /* ERROR */

            portGroupIndex = 0;

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_1_E;/*MG1*/
            additionalBaseAddrPtr[dupIndex++] = GT_NA;/* will cause fatal error */
            additionalRegDupPtr->portGroupsArr[portGroupIndex++] = portGroupId;

            dupWasDone  = GT_TRUE;
            unitPer2Tiles = GT_TRUE;/* allow code into switch(portGroupId)  */

            *unitsIndexPtr = unitsIndex;
            *dupIndexPtr   = dupIndex;
            *dupWasDonePtr = dupWasDone;
            *unitPerPipePtr  = unitPerPipe;
            *unitPer2TilesPtr= unitPer2Tiles;

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TRUE, "ERROR : the MG register [%x] must not be duplicated 'unaware' to all MG units",
                regAddr);
        }


        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_1_E;/*MG1*/
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_2_E;/*MG2*/
        usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_0_3_E;/*MG3*/
        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles >= 2)
        {
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_0_E;/*MG4*/
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_1_E;/*MG5*/
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_2_E;/*MG6*/
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_MG_1_3_E;/*MG7*/
        }

        relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        portGroupIndex = 0;
        mgId = 1;
        /* support the tiles 0,1 : MG 1..7 */
        /* the additionalBaseAddrPtr[] not holding the 'MG 0' so we skip index 0 */
        for(ii = 1; ii < unitsIndex ; ii++)
        {
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */

            additionalRegDupPtr->portGroupsArr[portGroupIndex++] = mgId++; /*per MG*/
        }

        stepTiles = 2;
        /* support the tiles 2,3 */
        for(tileId = stepTiles ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ; tileId += stepTiles)
        {
            /* tile offset */
            tileOffset = prvCpssSip6TileOffsetGet(devNum,tileId);

            for(ii = 0; ii < unitsIndex ; ii++)
            {
                pipe0Addr = ii == 0 ?
                    regAddr :/* index 0 is 'orig' regAddr */
                    additionalBaseAddrPtr[ii-1];/* ii = 0.. unitsIndex-1 hold
                                                   address of the dup units in pipes 0,1,2,3 */

                additionalBaseAddrPtr[dupIndex++] = pipe0Addr + tileOffset;

                additionalRegDupPtr->portGroupsArr[portGroupIndex++] = mgId++;/* MG 8..15 */
            }
        }

        dupWasDone  = GT_TRUE;
        unitPer2Tiles = GT_FALSE;/* NOT per 2 tiles , but per MG */
        unitPerMg = GT_TRUE;
    }

    *unitsIndexPtr = unitsIndex;
    *dupIndexPtr   = dupIndex;
    *dupWasDonePtr = dupWasDone;
    *unitPerPipePtr  = unitPerPipe;
    *unitPer2TilesPtr= unitPer2Tiles;
    *unitPerMgPtr= unitPerMg;


    return GT_TRUE;
}


/**
* @internal prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in Falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
GT_BOOL prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
)
{
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_U32  *additionalBaseAddrPtr;
    GT_U32  dupIndex;/* index in additionalBaseAddrPtr */
    GT_U32  ii;             /*iterator*/
    GT_BOOL unitPerPipe;    /*indication to use 'per pipe'    units */
    GT_BOOL unitPerTile;    /*indication to use 'per tile'    units */
    GT_BOOL unitPer2Tiles;  /*indication to use 'per 2 tiles' units */
    GT_BOOL unitPerMg;      /*indication to use 'per MG'      units */
    GT_BOOL dupWasDone;     /*indication that dup per pipe/tile/2 tiles was already done */
    GT_U32  stepTiles;      /* steps between tiles */
    PRV_CPSS_DXCH_UNIT_ENT  usedUnits[PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS];
    GT_U32  unitsIndex; /* index in usedUnits */
    GT_U32  pipe0Addr;/* address of the duplicated unit in pipe 0 */
    GT_U32  pipeId;/* pipeId iterator */
    GT_U32  tileId;/* tileId iterator */
    GT_U32  pipeOffset;/* offset of current pipe from pipe 0 */
    GT_U32  relativeRegAddr;/* relative register address to it's unit */
    const GT_U32    maskUnitsPerTile = 0x00000055;/* bits : 0,2,4,6 */

    if(!PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum))
    {
        *portGroupsBmpPtr = BIT_0;
        /* the GM device not supports pipe 1 and also not support multiple DP units */
        /* so do not give any duplications */
        return GT_FALSE;
    }

    *portGroupsBmpPtr = BIT_0;/* initialization that indicated that unit is single instance
        needed by prv_cpss_multi_port_groups_bmp_check_specific_unit_func(...) */

    /* initial needed variables */
    unitPerPipe = GT_FALSE;
    unitPerTile = GT_FALSE;
    unitPer2Tiles = GT_FALSE;
    unitPerMg = GT_FALSE;
    dupWasDone = GT_FALSE;
    additionalBaseAddrPtr = &additionalRegDupPtr->additionalAddressesArr[0];
    dupIndex = 0;

    /* call direct to prvCpssDxChFalconHwRegAddrToUnitIdConvert ... not need to
       get to it from prvCpssDxChHwRegAddrToUnitIdConvert(...) */
    unitId = prvCpssDxChFalconHwRegAddrToUnitIdConvert(devNum, regAddr);
    /* set the 'orig' unit at index 0 */
    unitsIndex = 0;
    usedUnits[unitsIndex++] = unitId;

    switch(unitId)
    {
        case PRV_CPSS_DXCH_UNIT_RXDMA_E:
            if(GT_TRUE == falcon_mustNotDuplicate_rxdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_RXDMA3_E;

            unitPerPipe = GT_TRUE;

            break;
        case PRV_CPSS_DXCH_UNIT_TXDMA_E:
            if(GT_TRUE == falcon_mustNotDuplicate_txdmaAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXDMA3_E;

            unitPerPipe = GT_TRUE;

            break;
        case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
            if(GT_TRUE == falcon_mustNotDuplicate_txfifoAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO1_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO2_E;
            usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TX_FIFO3_E;

            unitPerPipe = GT_TRUE;

            break;
      case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E:

            if(GT_TRUE == falcon_mustDuplicate_qfcAddr(devNum, regAddr))
            {
                /* the address is  for duplication */
                  usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E;
                  usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E;
                  usedUnits[unitsIndex++] = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E;

                   unitPerPipe = GT_TRUE;
            }
            else
            {
                return GT_FALSE;
            }



           break;

        case PRV_CPSS_DXCH_UNIT_LPM_E:
            if(GT_TRUE == falcon_mustNotDuplicatePerPipe_lpmMemAddr(devNum, regAddr))
            {
                /* the LPM table duplicated per tile */
                /* the pipe 0,1 duplicated by HW (simulation share it) */
                /* the pipe 2,3 duplicated by HW (simulation share it) */
                /* the pipe 4,5 duplicated by HW (simulation share it) */
                /* the pipe 6,7 duplicated by HW (simulation share it) */

                /* !!! TRICK !!! */
                unitPerTile = GT_TRUE;
            }
            else
            {
                /* the other tables duplicated per pipe !!! */
                unitPerPipe = GT_TRUE;
            }

            break;
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E                :
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E                :
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E                :
        case PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E                :
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_LPM_1_E:
            /* this unit is not for duplication */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_SERDES_E                      :
            /* probably ALL is per port ... and no global config */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_MIB_E                         :
            /* probably ALL is per port ... and no global config */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_GOP_E                         :
            /* probably ALL is per port ... and no global config */
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_TAI_E                         :
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_TAI1_E                         :
            return GT_FALSE;
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E:
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_1_E:
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_2_E:
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_3_E:
            return GT_FALSE;

        case PRV_CPSS_DXCH_UNIT_TTI_E                         :
            if(GT_TRUE == falcon_mustNotDuplicate_ttiAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }
            unitPerPipe = GT_TRUE;
            break;

        case PRV_CPSS_DXCH_UNIT_EQ_E                         :
            unitPerPipe = GT_TRUE;
            break;

        case PRV_CPSS_DXCH_UNIT_BMA_E                         :
        case PRV_CPSS_DXCH_UNIT_HA_E                          :
        case PRV_CPSS_DXCH_UNIT_PHA_E                         :
        case PRV_CPSS_DXCH_UNIT_EGF_SHT_E                     :
        case PRV_CPSS_DXCH_UNIT_EGF_QAG_E                     :
        case PRV_CPSS_DXCH_UNIT_PCL_E                         :
        case PRV_CPSS_DXCH_UNIT_L2I_E                         :
        case PRV_CPSS_DXCH_UNIT_IPVX_E                        :
        case PRV_CPSS_DXCH_UNIT_IPLR_E                        :
        case PRV_CPSS_DXCH_UNIT_IPLR_1_E                      :
        case PRV_CPSS_DXCH_UNIT_IOAM_E                        :
        case PRV_CPSS_DXCH_UNIT_MLL_E                         :
        case PRV_CPSS_DXCH_UNIT_EGF_EFT_E                     :
        case PRV_CPSS_DXCH_UNIT_CNC_0_E                       :
        case PRV_CPSS_DXCH_UNIT_CNC_1_E                       :
        case PRV_CPSS_DXCH_UNIT_ERMRK_E                       :
        case PRV_CPSS_DXCH_UNIT_EPCL_E                        :
        case PRV_CPSS_DXCH_UNIT_EPLR_E                        :
        case PRV_CPSS_DXCH_UNIT_EOAM_E                        :
        case PRV_CPSS_DXCH_UNIT_RXDMA_GLUE_E                  :
        case PRV_CPSS_DXCH_UNIT_TXDMA_GLUE_E                  :
        case PRV_CPSS_DXCH_UNIT_IA_E                          :
        case PRV_CPSS_DXCH_UNIT_EREP_E                        :
        case PRV_CPSS_DXCH_UNIT_PREQ_E                        :
            /* ALL those are units that ALL it's config should be also on pipe 1 */
            /* NOTE: if one of those units hold some special registers ....
                     take it out of this generic case */
            unitPerPipe = GT_TRUE;
            break;

        /***************/
        /* per 2 tiles */
        /***************/
        case  PRV_CPSS_DXCH_UNIT_MG_E                         :
            if(GT_FALSE == mgUnitDuplicatedMultiPortGroupsGet(
                devNum,portGroupId,regAddr,additionalRegDupPtr,unitId,
                usedUnits,&unitsIndex,additionalBaseAddrPtr,&dupIndex,
                &dupWasDone,&unitPerPipe,&unitPer2Tiles,&unitPerMg))
            {
                return GT_FALSE;
            }

            break;
        case PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_0_IN_RAVEN_E     :/*per 2 tiles , serve tile 0/2*/
            unitPer2Tiles = GT_TRUE;
            break;
        /************/
        /* per tile */
        /************/
        case PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E               :
        case PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_1_E               :
        case PRV_CPSS_DXCH_UNIT_TXQ_PSI_E                     :
        case PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E                     :
              return GT_FALSE;

        case PRV_CPSS_DXCH_UNIT_TXQ_PDX_E                     :
            if(GT_TRUE == falcon_mustDuplicate_pdxAddr(devNum, regAddr))
            {
                /* the address is  for duplication */
                unitPerTile = GT_TRUE;
            }
            else
            {
                return GT_FALSE;
            }
            break;

        case PRV_CPSS_DXCH_UNIT_SHM_E                         :
            if(GT_TRUE == falcon_mustNotDuplicate_shmAddr(devNum, regAddr))
            {
                /* the address is not for duplication */
                return GT_FALSE;
            }

            unitPerTile = GT_TRUE;
            break;

        case PRV_CPSS_DXCH_UNIT_TCAM_E                        :
        case PRV_CPSS_DXCH_UNIT_FDB_E                         :
        case PRV_CPSS_DXCH_UNIT_EM_E                          :
        case PRV_CPSS_DXCH_UNIT_DFX_SERVER_E                  :

            unitPerTile = GT_TRUE;
            break;
        case PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E                 :/* Falcon Packet Buffer Subunits */
        case PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E          :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E     :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E        :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E        :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E      :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E      :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E     :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E        :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E        :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E      :
        case PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E      :
        case PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E        :
        case PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E        :
        case PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_2_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_1_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_1_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_0_E        :
        case PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_1_E        :
            unitPerTile = GT_TRUE;
            break;


        default:
            /* we get here for 'non first instance' of duplicated units */

            /* we not need to duplicate the address */
            return GT_FALSE;
    }

    if(unitsIndex > PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS id [%d] but must be at least [%d]",
            PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS,unitsIndex);
    }

    if(dupWasDone == GT_TRUE)
    {
        /********************************/
        /* no extra duplication needed  */
        /* complex logic already applied*/
        /********************************/
    }
    else
    if(unitPerTile == GT_TRUE || unitPer2Tiles == GT_TRUE)
    {
        if(unitPer2Tiles == GT_TRUE)
        {
            /***************************************/
            /* start duplication for 'per 2 tiles' */
            /***************************************/
            stepTiles = 2;
        }
        else
        {
            /************************************/
            /* start duplication for 'per tile' */
            /************************************/
            stepTiles = 1;
        }

        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles < 2)
        {
            /* no duplications needed */
            return GT_FALSE;
        }

        pipe0Addr = regAddr;
        /* instance 0 already in the array */
        for(tileId = stepTiles ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ; tileId += stepTiles)
        {
            pipeOffset = prvCpssSip6TileOffsetGet(devNum,tileId);
            additionalBaseAddrPtr[dupIndex++] = pipe0Addr + pipeOffset;
            additionalRegDupPtr->portGroupsArr[(tileId/stepTiles)-1] = 2*tileId; /*2 pipes per tile*/
        }
    }
    else
    if(unitPerPipe == GT_TRUE)
    {
        /************************************/
        /* start duplication for 'per pipe' */
        /************************************/

        /* need to double the total indexes
           if unitsIndex = 1 meaning   'only' orig = 1 access --> need to become 2 access by setting unitsIndex = 2
           if unitsIndex = 3 meaning 2 dups + orig = 3 access --> need to become 6 access by setting unitsIndex = 6
        */

        relativeRegAddr = regAddr - prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,NULL);

        /*******************************************/
        /* first handle the duplications in pipe 0 */
        /*******************************************/
        /* the additionalBaseAddrPtr[] not holding the 'orig regAddr' so we skip index 0 */
        pipeId = 0;
        for(ii = 1; ii < unitsIndex ; ii++)
        {
            additionalRegDupPtr->portGroupsArr[dupIndex] = pipeId;
            additionalBaseAddrPtr[dupIndex++] =
                prvCpssDxChHwUnitBaseAddrGet(devNum,usedUnits[ii],NULL) +
                relativeRegAddr;/* baseOfUnit + 'relative' offset */
        }

        /**********************************************/
        /* now handle the duplications in other pipes */
        /**********************************************/
        for(pipeId = 1 ; pipeId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes ; pipeId++)
        {
            for(ii = 0; ii < unitsIndex ; ii++)
            {
                pipe0Addr = ii == 0 ?
                    regAddr :/* index 0 is 'orig' regAddr */
                    additionalBaseAddrPtr[ii-1];/* ii = 0.. unitsIndex-1 hold
                                                   address of the dup units in pipe 0 */

                /* jump from address of pipe0 to one of other pipe */
                pipeOffset = falconOffsetFromPipe0Get(pipeId,usedUnits[ii]);
                additionalRegDupPtr->portGroupsArr[dupIndex] = pipeId;
                additionalBaseAddrPtr[dupIndex++] = pipe0Addr +
                    pipeOffset;
            }
        }
    }

    if((unitPerTile == GT_TRUE)   || /* tile duplication */
       (unitPer2Tiles == GT_TRUE) || /* 2 tiles duplication */
       (unitPerPipe == GT_TRUE) ||   /* pipe indication  */
       (unitPerMg == GT_TRUE))         /* MG   indication  */
    {
        /* support multi-pipe awareness by parameter <portGroupId> */
        switch(portGroupId)
        {
            case CPSS_PORT_GROUP_UNAWARE_MODE_CNS:
                /* allow the loops on the 2 addresses (orig+additional) to be
                    accessed , each one in different iteration in the loop of :
                    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(...)
                    or
                    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(...)
                */

                additionalRegDupPtr->use_originalAddressPortGroup = GT_TRUE;
                additionalRegDupPtr->originalAddressPortGroup = 0; /* pipe 0 */

                additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
                *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
                break;
            case 0:
                *portGroupsBmpPtr = BIT_0;
                dupIndex = 0;/* access only to pipe 0 --> no duplications */
                break;
            default:
                if((unitPerMg == GT_FALSE && portGroupId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes) ||
                   (unitPerMg == GT_TRUE  && portGroupId < PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits))
                {
                    *portGroupsBmpPtr = (BIT_0 << portGroupId);
                    /* access only to this pipe (1/2/3/4/5/6/7) */
                    additionalRegDupPtr->originalAddressIsNotValid = GT_TRUE;
                    /* the only valid address is the one in additionalBaseAddrPtr[0] */
                    additionalRegDupPtr->use_portGroupsArr = GT_TRUE;
                }
                else
                {
                    /* should not get here */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : unsupported port group id [%d] ",
                        portGroupId);
                }
                break;
        }

    }
    else
    {
        /* the other units not care about multi-pipe */
    }

    if(dupIndex == 0)
    {
        return GT_FALSE;
    }

    if(dupIndex > PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "ERROR : PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS id [%d] but must be at least [%d]",
            PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_MAX_CNS,dupIndex);
    }

    if(unitPerMg == GT_TRUE)
    {
        /******************************************/
        /* mask the MGs BMP with existing MGs BMP */
        /******************************************/
        *portGroupsBmpPtr &= (1<<PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits)-1;

        /* state the caller not to do more mask */
        additionalRegDupPtr->skipPortGroupsBmpMask = GT_TRUE;
    }
    else
    {
        /**********************************************/
        /* mask the pipes BMP with existing pipes BMP */
        /**********************************************/
        *portGroupsBmpPtr &= PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;
        if((unitPerTile == GT_TRUE) || (unitPer2Tiles == GT_TRUE))
        {
            *portGroupsBmpPtr &= maskUnitsPerTile;
        }
    }

    additionalRegDupPtr->numOfAdditionalAddresses = dupIndex;

    *isAdditionalRegDupNeededPtr = GT_TRUE;
    *maskDrvPortGroupsPtr = GT_FALSE;

    return GT_TRUE;
}


/**
* @internal prvCpssSip6OffsetFromFirstInstanceGet function
* @endinternal
*
* @brief   for any instance of unit that is duplicated get the address offset from
*         first instance of the unit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] instanceId               - the instance index in which the unitId exists.
* @param[in] instance0UnitId          - the unitId 'name' of first instance.
*                                       the address offset from instance 0
*/
GT_U32   prvCpssSip6OffsetFromFirstInstanceGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   instanceId,
    IN PRV_CPSS_DXCH_UNIT_ENT   instance0UnitId
)
{
    GT_U32  numOfDpPerPipe,numOfPipesPerTile,mirroredTilesBmp;
    GT_U32  relativeInstanceInPipe0;
    GT_U32  pipe0UnitId , pipeId , tileId;
    GT_U32  instance0BaseAddr,instanceInPipe0BaseAddr,offsetFromPipe0;

    /* use 'prvCpssDrvPpConfig[devNum]' because it may be called from the cpssDriver
       before the 'PRV_CPSS_PP_MAC(devNum)' initialized ! */
    switch(prvCpssDrvPpConfig[devNum]->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            numOfDpPerPipe = 4;
            numOfPipesPerTile = 2;
            mirroredTilesBmp  = MIRRORED_TILES_BMP_CNS;
            break;
        default:
            /* was not implemented */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NA, LOG_ERROR_NO_MSG);
    }

    switch(instance0UnitId)
    {
        /************/
        /* per pipe */
        /************/
        default:
            pipe0UnitId = instance0UnitId;
            pipeId      = instanceId;
            break;

        /***************/
        /* per 2 tiles */
        /***************/
        case  PRV_CPSS_DXCH_UNIT_MG_E               :
        {
            /* tile 0 : MG 0,1,2,3 - serve 'tile 0' located at tile 0 memory */
            /* tile 1 : MG 0,1,2,3 - serve 'tile 1' located at tile 0 memory */
            /* tile 2 : MG 0,1,2,3 - serve 'tile 2' located at tile 2 memory */
            /* tile 3 : MG 0,1,2,3 - serve 'tile 3' located at tile 2 memory */
            GT_U32  tileOffset = ((2 * FALCON_TILE_OFFSET_CNS) * (instanceId / NUM_MG_PER_CNM));
            GT_U32  internalCnmOffset = (MG_SIZE            * (instanceId % NUM_MG_PER_CNM));

            return tileOffset + internalCnmOffset;
        }

        /***************/
        /* per 2 tiles */
        /***************/
        case  PRV_CPSS_DXCH_UNIT_SMI_0_E           :
        case  PRV_CPSS_DXCH_UNIT_SMI_1_E           :
            pipe0UnitId = instance0UnitId;
            pipeId      = 2 * numOfPipesPerTile * instanceId;
            break;

        case  PRV_CPSS_DXCH_UNIT_EAGLE_D2D_CP_0_IN_RAVEN_E:/* per 2 tiles but serve tile */
            pipe0UnitId = instance0UnitId;
            pipeId      = 2 * instanceId;
            break;

        /************/
        /* per tile */
        /************/
        case  PRV_CPSS_DXCH_UNIT_TCAM_E             :
        case  PRV_CPSS_DXCH_UNIT_TXQ_PDX_E          :
        case  PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E    :
        case  PRV_CPSS_DXCH_UNIT_TXQ_PDX_PAC_1_E    :
        case  PRV_CPSS_DXCH_UNIT_FDB_E              :
        case  PRV_CPSS_DXCH_UNIT_EM_E               :
        case  PRV_CPSS_DXCH_UNIT_SHM_E              :
        case  PRV_CPSS_DXCH_UNIT_TXQ_PSI_E          :
        case  PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E         :
        case  PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E     :
        case  PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E     :
        case  PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E     :
        case  PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E     :
            pipe0UnitId = instance0UnitId;
            pipeId      = 2 * instanceId;
            break;

        case  PRV_CPSS_DXCH_UNIT_RXDMA_E            :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_RXDMA1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_RXDMA2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_RXDMA3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_RXDMA_E ; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TXDMA_E            :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;
            pipeId                  = instanceId / numOfDpPerPipe;
            tileId = pipeId / numOfPipesPerTile;

            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXDMA1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXDMA2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXDMA3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_TXDMA_E ; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TX_FIFO_E          :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TX_FIFO1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TX_FIFO2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TX_FIFO3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_TX_FIFO_E ; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E   :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E; break;
            }
            break;
        case  PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E   :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E; break;
            }
            break;

         case  PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E   :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E; break;
            }
            break;

         case  PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E   :
            relativeInstanceInPipe0 = instanceId % numOfDpPerPipe;
            pipeId                  = instanceId / numOfDpPerPipe;

            tileId = pipeId / numOfPipesPerTile;
            if((1 << tileId) & mirroredTilesBmp)
            {
                /* the DP units also mirrored within the pipe */
                relativeInstanceInPipe0 = (numOfDpPerPipe - 1) - relativeInstanceInPipe0;
            }

            switch(relativeInstanceInPipe0)
            {
                case 1: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_EAGLE_D2D_1_E; break;
                case 2: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_EAGLE_D2D_2_E; break;
                case 3: pipe0UnitId  = PRV_CPSS_DXCH_UNIT_EAGLE_D2D_3_E; break;
                default: pipe0UnitId = PRV_CPSS_DXCH_UNIT_EAGLE_D2D_0_E; break;
            }
            break;
    }

    /* we got here for units that are per DP */
    /* 1. get the offset of the unit from first instance in Pipe 0 */
    /*  1.a get address of first  instance */
    instance0BaseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum,instance0UnitId,NULL);
    /*  1.b get address of needed instance (in pipe 0)*/
    instanceInPipe0BaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,pipe0UnitId,NULL);

    /* 2. get the offset of the unit from Pipe 0                   */
    offsetFromPipe0 = falconOffsetFromPipe0Get(pipeId,pipe0UnitId);

    return (instanceInPipe0BaseAddr - instance0BaseAddr) + offsetFromPipe0;
}


/**
* @internal falconSpecialPortsMapGet function
* @endinternal
*
* @brief   Falcon : get proper table for the special GOP/DMA port numbers.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
*
* @param[out] specialDmaPortsPtrPtr    - (pointer to) the array of special GOP/DMA port numbers
* @param[out] numOfSpecialDmaPortsPtr  - (pointer to) the number of elements in the array.
*                                       GT_OK on success
*/
static GT_STATUS falconSpecialPortsMapGet
(
    IN  GT_U8   devNum,
    OUT SPECIAL_DMA_PORTS_STC   **specialDmaPortsPtrPtr,
    OUT GT_U32  *numOfSpecialDmaPortsPtr
)
{
    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        case 0:
        case 1:
            *specialDmaPortsPtrPtr   =           falcon_3_2_SpecialDma_ports;
            *numOfSpecialDmaPortsPtr = num_ports_falcon_3_2_SpecialDma_ports;
            break;
        case 2:
            *specialDmaPortsPtrPtr   =           falcon_6_4_SpecialDma_ports;
            *numOfSpecialDmaPortsPtr = num_ports_falcon_6_4_SpecialDma_ports;
            break;
        case 4:
            *specialDmaPortsPtrPtr   =           falcon_12_8_SpecialDma_ports;
            *numOfSpecialDmaPortsPtr = num_ports_falcon_12_8_SpecialDma_ports;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "not supported number of tiles [%d]",
                PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
    }

    return GT_OK;
}


/**
* @internal prvCpssFalconGopGlobalMacPortNumToLocalMacPortInPipeConvert function
* @endinternal
*
* @brief   Falcon : convert the global GOP MAC port number in device to local GOP MAC port
*         in the pipe , and the pipeId.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalMacPortNum         - the MAC global port number.
*
* @param[out] pipeIndexPtr             - (pointer to) the pipe Index of the MAC port
* @param[out] localMacPortNumPtr       - (pointer to) the MAC local port number
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconGopGlobalMacPortNumToLocalMacPortInPipeConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalMacPortNum,
    OUT GT_U32  *pipeIndexPtr,
    OUT GT_U32  *localMacPortNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;
    GT_U32  numPortsInPipe;

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp * FALCON_PORTS_PER_DP_CNS;
    numPortsInPipe  = numRegularPorts / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes;

    if(globalMacPortNum < numRegularPorts)
    {
        if(pipeIndexPtr)
        {
            *pipeIndexPtr       = globalMacPortNum / numPortsInPipe;
        }

        if(localMacPortNumPtr)
        {
            *localMacPortNumPtr = globalMacPortNum % numPortsInPipe;
        }
    }
    else
    {
        rc = falconSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(globalMacPortNum >= (numRegularPorts + numOfSpecialDmaPorts))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "global MAC number [%d] is >= [%d] (the max)",
                globalMacPortNum,
                numRegularPorts + numOfSpecialDmaPorts);
        }

        if(specialDmaPortsPtr[globalMacPortNum-numRegularPorts].localPortInPipe == GT_NA)
        {
            /* the global port is NOT valid global GOP (only valid DMA port) */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "global MAC number [%d] is not valid",
                globalMacPortNum);
        }

        if(pipeIndexPtr)
        {
            *pipeIndexPtr       = specialDmaPortsPtr[globalMacPortNum-numRegularPorts].pipeIndex;
        }

        if(localMacPortNumPtr)
        {
            *localMacPortNumPtr = specialDmaPortsPtr[globalMacPortNum-numRegularPorts].localPortInPipe;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconDmaGlobalNumToLocalNumInDpConvert function
* @endinternal
*
* @brief   Falcon : convert the global DMA number in device to local DMA number
*         in the DataPath (DP), and the Data Path Id.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] dpIndexPtr               - (pointer to) the Data Path (DP) Index
* @param[out] localDmaNumPtr           - (pointer to) the DMA local number
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconDmaGlobalNumToLocalNumInDpConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *dpIndexPtr,
    OUT GT_U32  *localDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp * FALCON_PORTS_PER_DP_CNS;

    if(globalDmaNum < numRegularPorts)
    {
        if(dpIndexPtr)
        {
            *dpIndexPtr     = globalDmaNum / FALCON_PORTS_PER_DP_CNS;
        }

        if(localDmaNumPtr)
        {
            *localDmaNumPtr = globalDmaNum % FALCON_PORTS_PER_DP_CNS;
        }
    }
    else
    {
        rc = falconSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(globalDmaNum >= (numRegularPorts + numOfSpecialDmaPorts))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "global DMA number [%d] is >= [%d] (the max)",
                globalDmaNum,
                numRegularPorts + numOfSpecialDmaPorts);
        }

        if(dpIndexPtr)
        {
            *dpIndexPtr       = specialDmaPortsPtr[globalDmaNum-numRegularPorts].dpIndex;
        }

        if(localDmaNumPtr)
        {
            *localDmaNumPtr   = specialDmaPortsPtr[globalDmaNum-numRegularPorts].localPortInDp;
        }
    }

    return GT_OK;
}
/**
* @internal prvCpssFalconGlobalDpToTileAndLocalDp function
* @endinternal
*
* @brief   Falcon : convert the global DP number in device to local DP number and tile
*
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDpIndex             - the DP global number.
*
* @param[out] localDpIndexPtr               - (pointer to) the local Data Path (DP) Index
* @param[out] tileIndexPtr           - (pointer to) the  tile number
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconGlobalDpToTileAndLocalDp
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDpIndex,
    OUT  GT_U32 * localDpIndexPtr,
    OUT  GT_U32  * tileIndexPtr
)
{
        GT_U32 tileId,localDpIndexInTile;
        GT_U32  numDpPerTile = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp /
                                 PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;

      if(globalDpIndex>PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)
      {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "globalDpIndex [%d] > [%d] (the max)",
            globalDpIndex,PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp);
       }


       tileId = globalDpIndex / numDpPerTile;

     if(0 == ((1 << tileId) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
    {
        /* the tiles 0,2 : not need 'mirror' conversion */
        localDpIndexInTile = globalDpIndex % numDpPerTile;
    }
    else
    {
        /* the tiles 1,3 : need 'mirror' conversion */
        localDpIndexInTile = (numDpPerTile - 1) - (globalDpIndex % numDpPerTile);
    }

    *localDpIndexPtr = localDpIndexInTile;
    *tileIndexPtr = tileId;


    return GT_OK;
}

/**
* @internal prvCpssFalconDmaLocalNumInDpToGlobalNumConvert function
* @endinternal
*
* @brief   Falcon : convert the local DMA number in the DataPath (DP), to global DMA number
*         in the device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconDmaLocalNumInDpToGlobalNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    OUT GT_U32  *globalDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;

    if(localDmaNum > FALCON_PORTS_PER_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "local DMA number [%d] > [%d] (the max)",
            localDmaNum,FALCON_PORTS_PER_DP_CNS);
    }

    if(localDmaNum < FALCON_PORTS_PER_DP_CNS)
    {
        *globalDmaNumPtr = (FALCON_PORTS_PER_DP_CNS*dpIndex) + localDmaNum;
        return GT_OK;
    }

    rc = falconSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp * FALCON_PORTS_PER_DP_CNS;

    for(ii = 0 ; ii < numOfSpecialDmaPorts; ii++)
    {
        if(specialDmaPortsPtr[ii].dpIndex == dpIndex)
        {
            *globalDmaNumPtr = numRegularPorts + ii;
            return GT_OK;
        }
    }

    CPSS_LOG_INFORMATION_MAC("dpIndex[%d] and localDmaNum[%d] not found as existing DMA",
    dpIndex,localDmaNum);

    return /*do not log*/GT_NOT_FOUND;
}

/**
* @internal prvCpssFalconDmaLocalNumInDpToGlobalNumConvert_getNext function
* @endinternal
*
* @brief   Falcon : the pair of {dpIndex,localDmaNum} may support 'mux' of :
*                   SDMA CPU port or network CPU port
*          each of them hold different 'global DMA number'
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
* @param[out] globalDmaNumPtr          - (pointer to) the 'current' DMA global number.
*                                       if 'current' is GT_NA (0xFFFFFFFF) --> meaning need to 'get first'
*
* @param[out] globalDmaNumPtr          - (pointer to) the 'next' DMA global number.
*  GT_OK      on success
*  GT_NO_MORE on success but no more such global DMA port
*  GT_BAD_PARAM on bad param
*  GT_NOT_FOUND on non exists global DMA port
*/
GT_STATUS prvCpssFalconDmaLocalNumInDpToGlobalNumConvert_getNext
(
    IN  GT_U8   devNum,
    IN  GT_U32  dpIndex,
    IN  GT_U32  localDmaNum,
    INOUT GT_U32  *globalDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;
    GT_U32  globalDmaNum;

    /* check validity and get 'first globalDmaNum' */
    rc = prvCpssFalconDmaLocalNumInDpToGlobalNumConvert(devNum,
        dpIndex,localDmaNum,&globalDmaNum);

    if(rc != GT_OK)
    {
        /* there is error with {dpIndex,localDmaNum} ... so no 'first' and no 'next' ...*/
        return rc;
    }

    if(GT_NA == (*globalDmaNumPtr))
    {
        /* get first global DMA that match {dpIndex,localDmaNum} */
        *globalDmaNumPtr = globalDmaNum;
        return GT_OK;
    }

    /* check if there is 'next' */
    if(localDmaNum < FALCON_PORTS_PER_DP_CNS)
    {
        /* regular port without muxing */

        return /* not error for the LOG */ GT_NO_MORE;
    }

    /* use the pointer as 'IN' parameter */
    globalDmaNum = *globalDmaNumPtr;

    rc = falconSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp * FALCON_PORTS_PER_DP_CNS;

    /*************************************************/
    /* start looking from 'next' index after current */
    /*************************************************/
    ii = (globalDmaNum-numRegularPorts) + 1;

    for(/*already init*/ ; ii < numOfSpecialDmaPorts; ii++)
    {
        if(specialDmaPortsPtr[ii].dpIndex == dpIndex)
        {
            /****************/
            /* found 'next' */
            /****************/
            *globalDmaNumPtr = numRegularPorts + ii;
            return GT_OK;
        }
    }

    /********************/
    /* NOT found 'next' */
    /********************/

    return /* not error for the LOG */ GT_NO_MORE;
}

/**
* @internal prvCpssFalconDmaGlobalDmaMuxed_getNext function
* @endinternal
*
* @brief   Falcon : support 'mux' of SDMA CPU port or network CPU port.
*          the function return 'next' global DMA port number that may share the
*          same local dma number in the same DP.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[out] globalDmaNumPtr          - (pointer to) the 'current' DMA global number.
*
* @param[out] globalDmaNumPtr          - (pointer to) the 'next' DMA global number.
*  GT_OK      on success
*  GT_NO_MORE on success but no more such global DMA port
*  GT_BAD_PARAM on bad param
*  GT_NOT_FOUND on non exists global DMA port
*/
GT_STATUS prvCpssFalconDmaGlobalDmaMuxed_getNext
(
    IN  GT_U8   devNum,
    INOUT GT_U32  *globalDmaNumPtr
)
{
    GT_STATUS   rc;
    GT_U32 globalDmaNum,dpIndex,localDmaNum;

    globalDmaNum = *globalDmaNumPtr;

    /* first get the dpIndex and localDmaNum of the global DMA port number */
    rc = prvCpssFalconDmaGlobalNumToLocalNumInDpConvert(devNum,globalDmaNum,&dpIndex,&localDmaNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check if the dpIndex and localDmaNum hold other muxed global DMA port number */
    return prvCpssFalconDmaLocalNumInDpToGlobalNumConvert_getNext(devNum,dpIndex,localDmaNum,globalDmaNumPtr);
}

/**
* @internal prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert function
* @endinternal
*
* @brief   Falcon : convert the global DMA number in device to TileId and to local
*         DMA number in the Local DataPath (DP index in tile).
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] tileIdPtr                - (pointer to) the tile Id
*                                      dpIndexPtr          - (pointer to) the Data Path (DP) Index (local DP in the tile !!!)
*                                      localDmaNumPtr      - (pointer to) the DMA local number (local DMA in the DP !!!)
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *tileIdPtr,
    OUT GT_U32  *localDpIndexInTilePtr,
    OUT GT_U32  *localDmaNumInDpPtr
)
{
    GT_STATUS rc;
    GT_U32  tileId;
    GT_U32  globalDpIndex;
    GT_U32  temp_localDmaNumInDp;
    GT_U32  localDpIndexInTile , localDmaNumInDp;
    GT_U32  numDpPerTile = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp /
                           PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;

    /* get global DP[] and local DMA in the global DP */
    rc = prvCpssFalconDmaGlobalNumToLocalNumInDpConvert(devNum,globalDmaNum,
        &globalDpIndex,&temp_localDmaNumInDp);
    if(rc != GT_OK)
    {
        return rc;
    }

    tileId = globalDpIndex / numDpPerTile;

    if(0 == ((1 << tileId) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
    {
        /* the tiles 0,2 : not need 'mirror' conversion */
        localDpIndexInTile = globalDpIndex % numDpPerTile;
        localDmaNumInDp    = temp_localDmaNumInDp;
    }
    else
    {
        /* the tiles 1,3 : need 'mirror' conversion */
        localDpIndexInTile = (numDpPerTile - 1) - (globalDpIndex % numDpPerTile);

        localDmaNumInDp = temp_localDmaNumInDp;
    }

    if(tileIdPtr)
    {
        *tileIdPtr = tileId;
    }

    if(localDpIndexInTilePtr)
    {
        *localDpIndexInTilePtr = localDpIndexInTile;
    }

    if(localDmaNumInDpPtr)
    {
        *localDmaNumInDpPtr = localDmaNumInDp;
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert function
* @endinternal
*
* @brief   Falcon : convert the local DMA number in the Local DataPath (DP index in tile),
*         to global DMA number in the device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] tileId                   - The tile Id
* @param[in] localDpIndexInTile       - the Data Path (DP) Index (local DP in the tile !!!)
* @param[in] localDmaNumInDp          - the DMA local number (local DMA in the DP !!!)
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number in the device.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileId,
    IN  GT_U32  localDpIndexInTile,
    IN  GT_U32  localDmaNumInDp,
    OUT GT_U32  *globalDmaNumPtr
)
{
    GT_U32  globalDpIndex;
    GT_U32  temp_localDmaNumInDp;
    GT_U32  numDpPerTile = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp /
                           PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;

    if(localDpIndexInTile >= numDpPerTile)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "localDpIndexInTile[%d] must not be '>=' than [%d]",
            localDpIndexInTile,numDpPerTile);
    }

    if(localDmaNumInDp > FALCON_PORTS_PER_DP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "localDmaNumInDp[%d] must not be '>' than [%d]",
            localDmaNumInDp,FALCON_PORTS_PER_DP_CNS);
    }
/*      looking at the 'DPs' per tile :
        4        5        6        7        7        6        5        4
                        CP1                        CP1
3                                                                      3
2        CP0                                                  CP0      2
1                        T0                        T1                  1
0                                                                      0
0                                                                      0
1                        T1                        T0                  1
2        CP0                                                  CP0      2
3                                                                      3
                        CP1                        CP1
        4        5        6        7        7        6        5        4
*/
/*      looking at the 'DPs' per device :
        4        5        6        7        8        9        10       11
                        CP1                        CP2
3                                                                      12
2        CP0                                                 CP3       13
1                        T0                        T1                  14
0                                                                      15
31                                                                     16
30                       T3                        T2                  17
29        CP7                                                CP4       18
28                                                                     19
                        CP6                        CP5
        27      26       25      24        23       22        21       20
*/


    if(0 == ((1 << tileId) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
    {
        /* the tiles 0,2 : not need 'mirror' conversion */
        globalDpIndex = localDpIndexInTile + (tileId * numDpPerTile);
        temp_localDmaNumInDp = localDmaNumInDp;
    }
    else
    {
        /* the tiles 1,3 : need 'mirror' conversion */
        globalDpIndex = ((numDpPerTile - 1) - localDpIndexInTile) + (tileId * numDpPerTile);
        temp_localDmaNumInDp = localDmaNumInDp;
    }

    return prvCpssFalconDmaLocalNumInDpToGlobalNumConvert(devNum,
        globalDpIndex,temp_localDmaNumInDp,globalDmaNumPtr);
}

extern GT_U32 debug_force_numOfDp_get(void);
extern GT_U32 debug_force_numOfPipes_get(void);

/**
* @internal prvCpssFalconInitParamsSet function
* @endinternal
*
* @brief   Falcon : init the very first settings in the DB of the device:
*         numOfTiles , numOfPipesPerTile , numOfPipes ,
*         multiDataPath.maxDp ,
*         cpuPortInfo.info[index].(dmaNum,valid,dqNum)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconInitParamsSet
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc;
    GT_U32  numRegularPorts;
    GT_U32  numOfSpecialDmaPorts;
    SPECIAL_DMA_PORTS_STC   *specialDmaPortsPtr;
    PRV_CPSS_DXCH_PP_CONFIG_STC *dxDevPtr = PRV_CPSS_DXCH_PP_MAC(devNum);
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = &dxDevPtr->genInfo;
    GT_U32  ii,index;
    GT_U32  tileId,localDpIndex,numDpsInTile;
    GT_U32  debug_force_numOfDp    = debug_force_numOfDp_get();
    GT_U32  debug_force_numOfPipes = debug_force_numOfPipes_get();
    GT_U32  *sip6MgIndexToDpIndexMapPtr;
    GT_U32  lanesNumInDev;
    GT_U32  numMgUnitsPerTile = 4;
    GT_U32  numMgUnits = numMgUnitsPerTile * devPtr->multiPipe.numOfTiles;
    GT_U32  auqPortGroupIndex = 0,fuqPortGroupIndex = 0;

    PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits = numMgUnits;
    sip6MgIndexToDpIndexMapPtr = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgIndexToDpIndexMap;
    for(ii = 0 ; ii < numMgUnits; ii++)
    {
        if(0 == (ii % numMgUnitsPerTile))
        {
            /* AUQ per 4 MGs */
            /* each serve tile */
            PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgUnitsBmpWithAuq |= BIT_0 << ii;

            PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgIndexToAuqPortGroup[ii] = auqPortGroupIndex;

            PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6AuqPortGroupBmp |= BIT_0 << auqPortGroupIndex;

            /* the port groups bmp are 'per pipe' so every 2 pipes we have AUQ */
            auqPortGroupIndex += 2;

        }

        if((0 == (ii % numMgUnitsPerTile)) ||
           (1 == (ii % numMgUnitsPerTile)))
        {
            /* 2 FUQ per 4 MGs */
            /* each serve pipe */
            PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgUnitsBmpWithFuq |= BIT_0 << ii;

            PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgIndexToAuqPortGroup[ii] = fuqPortGroupIndex;

            PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6FuqPortGroupBmp |= BIT_0 << fuqPortGroupIndex;

            /* the port groups bmp are 'per pipe' so every pipe we have FUQ */
            fuqPortGroupIndex++;
        }

        sip6MgIndexToDpIndexMapPtr[ii] = GT_NA;
    }

    ii = 0;
    sip6MgIndexToDpIndexMapPtr[ii++] = MG0_SERVED_DP;
    sip6MgIndexToDpIndexMapPtr[ii++] = MG1_SERVED_DP;
    sip6MgIndexToDpIndexMapPtr[ii++] = MG2_SERVED_DP;
    sip6MgIndexToDpIndexMapPtr[ii++] = MG3_SERVED_DP;

    if(devPtr->multiPipe.numOfTiles >= 2)
    {
        sip6MgIndexToDpIndexMapPtr[ii++] = MG4_SERVED_DP;
        sip6MgIndexToDpIndexMapPtr[ii++] = MG5_SERVED_DP;
        sip6MgIndexToDpIndexMapPtr[ii++] = MG6_SERVED_DP;
        sip6MgIndexToDpIndexMapPtr[ii++] = MG7_SERVED_DP;
        if(devPtr->multiPipe.numOfTiles >= 4)
        {
            sip6MgIndexToDpIndexMapPtr[ii++] = MG8_SERVED_DP;
            sip6MgIndexToDpIndexMapPtr[ii++] = MG9_SERVED_DP;
            sip6MgIndexToDpIndexMapPtr[ii++] = MG10_SERVED_DP;
            sip6MgIndexToDpIndexMapPtr[ii++] = MG11_SERVED_DP;

            sip6MgIndexToDpIndexMapPtr[ii++] = MG12_SERVED_DP;
            sip6MgIndexToDpIndexMapPtr[ii++] = MG13_SERVED_DP;
            sip6MgIndexToDpIndexMapPtr[ii++] = MG14_SERVED_DP;
            sip6MgIndexToDpIndexMapPtr[ii++] = MG15_SERVED_DP;
        }
    }



    devPtr->multiPipe.numOfPipesPerTile = 2;
    devPtr->multiPipe.tileOffset        = FALCON_TILE_OFFSET_CNS;
    devPtr->multiPipe.mirroredTilesBmp  = MIRRORED_TILES_BMP_CNS;/* in Falcon tile 1,3 are mirror image of tile 0 */

    devPtr->multiPipe.numOfPipes =
        devPtr->multiPipe.numOfTiles *
        devPtr->multiPipe.numOfPipesPerTile;

    dxDevPtr->hwInfo.multiDataPath.maxDp = devPtr->multiPipe.numOfPipes * 4;/* 4 DP[] per pipe */
    /* number of GOP ports per pipe , NOT including the 'CPU Port' . */
    devPtr->multiPipe.numOfPortsPerPipe = FALCON_PORTS_PER_DP_CNS * 4;/* 4 DP[] per pipe */

    if(debug_force_numOfPipes)
    {
        devPtr->multiPipe.numOfPipes = debug_force_numOfPipes;
    }

    if(debug_force_numOfDp)
    {
        dxDevPtr->hwInfo.multiDataPath.maxDp = debug_force_numOfDp;
        devPtr->multiPipe.numOfPortsPerPipe = FALCON_PORTS_PER_DP_CNS *
            (dxDevPtr->hwInfo.multiDataPath.maxDp /
             devPtr->multiPipe.numOfPipes);
    }

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        default:
            lanesNumInDev = PRV_CPSS_FALCON_3_2_SERDES_NUM_CNS;
            break;
        case 2:
            lanesNumInDev = PRV_CPSS_FALCON_6_4_SERDES_NUM_CNS;
            break;
        case 4:
            lanesNumInDev = PRV_CPSS_FALCON_12_8_SERDES_NUM_CNS;
            break;
    }

    if(debug_force_numOfDp)
    {
        lanesNumInDev = debug_force_numOfDp * FALCON_PORTS_PER_DP_CNS;
    }

    PRV_CPSS_DXCH_PP_HW_INFO_SERDES_MAC(devNum).sip6LanesNumInDev = lanesNumInDev;


    /* get info for 'SDMA CPU' port numbers */
    rc = falconSpecialPortsMapGet(devNum,&specialDmaPortsPtr,&numOfSpecialDmaPorts);
    if(rc != GT_OK)
    {
        return rc;
    }

    numRegularPorts = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp * FALCON_PORTS_PER_DP_CNS;

    index = 0;
    for(ii = 0 ; ii < numOfSpecialDmaPorts; ii++)
    {
        if(specialDmaPortsPtr[ii].localPortInPipe == GT_NA)
        {
            /* this is valid 'SDMA CPU' port */

            if(index >= CPSS_MAX_SDMA_CPU_PORTS_CNS)
            {
                CPSS_TBD_BOOKMARK_FALCON
                /* The CPSS is not ready yet to more SDMAs */
                break;
            }
            dxDevPtr->hwInfo.cpuPortInfo.info[index].dmaNum = numRegularPorts + ii;
            dxDevPtr->hwInfo.cpuPortInfo.info[index].valid  = GT_TRUE;
            dxDevPtr->hwInfo.cpuPortInfo.info[index].usedAsCpuPort = GT_FALSE;/* the 'port mapping' should bind it */
            /* parameter called 'dqNum' but should be considered as 'DP index' */
            dxDevPtr->hwInfo.cpuPortInfo.info[index].dqNum  = specialDmaPortsPtr[ii].dpIndex;
            index++;
        }
    }

    numDpsInTile = 8;
    /* NOTE: in FALCON the mirrored Tiles cause the DP[] units to hold swapped DMAs */
    for(ii = 0 ; ii < dxDevPtr->hwInfo.multiDataPath.maxDp ; ii++)
    {
        tileId       = ii/numDpsInTile;
        localDpIndex = ii%numDpsInTile;

        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathFirstPort  = (localDpIndex + (tileId * numDpsInTile)) * FALCON_PORTS_PER_DP_CNS;
        dxDevPtr->hwInfo.multiDataPath.info[ii].dataPathNumOfPorts = FALCON_PORTS_PER_DP_CNS + 1;/* support CPU port*/
        dxDevPtr->hwInfo.multiDataPath.info[ii].cpuPortDmaNum = FALCON_PORTS_PER_DP_CNS;
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_numLinesPerBlockNonShared   = 640;/* falcon : 640 lines , in non shared block */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksNonShared  = 20;/* falcon : up to 20 blocks from non shared (to fill to total of 30) */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared = 30;/* falcon : 30 blocks supported */

    /* PHA info */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg = 3;
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpn = 9;


    return GT_OK;
}

/**
* @internal prvCpssSip6TileOffsetGet function
* @endinternal
*
* @brief   get the address offset in bytes of tile 'x' from tile 0.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] tileId                   - the  to get offset from tile 0.
*                                       the tile address offset in bytes from tile 0
*/
GT_U32   prvCpssSip6TileOffsetGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   tileId
)
{
    if(tileId >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        tileId = 0;
    }

    return tileId * PRV_CPSS_PP_MAC(devNum)->multiPipe.tileOffset;
}

#define TXFIFO_BASE_ADDR_GET(devNum,txFifoUnitIndex)    \
    (prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, NULL) + \
     prvCpssSip6OffsetFromFirstInstanceGet(devNum,txFifoUnitIndex,PRV_CPSS_DXCH_UNIT_TX_FIFO_E))


/**
* @internal prvCpssSip6TxFifoDebugCountersEnableSet function
* @endinternal
*
* @brief    enable the debug counter in the TxFifo unit
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] txFifoUnitIndex          - the  TxFifo unit index (0..numOfPipes*4)
*/
GT_STATUS   prvCpssSip6TxFifoDebugCountersEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32   txFifoUnitIndex,
    IN GT_BOOL  enable
)
{
    GT_STATUS   rc;
    /* Cider path : Eagle/Pipe/<TXFIFO> TxFIFO IP TLU/Units_%x/TxFIFO IP Unit/Debug */
    GT_U32  infoArr[] = {
        /*address , value */
        0x00005004, BIT_0      ,    /*enable counting*/
        0x0000500C, 0xffffffff ,    /*0xffffffff for threshold*/
        0x00005008, BIT_8      ,    /*count 'all channels'*/
        /* must be last */
        GT_NA, GT_NA
    };
    GT_U32  regAddr,value,ii;
    GT_U32  baseAddr;

    infoArr[1] = enable;/* enable/disable the counting  */

    baseAddr = TXFIFO_BASE_ADDR_GET(devNum, txFifoUnitIndex);

    for(ii = 0 ; infoArr[ii] !=  GT_NA ; ii += 2)
    {
        regAddr = infoArr[ii+0] + baseAddr ;
        value   = infoArr[ii+1];

        rc = prvCpssHwPpWriteRegister(devNum,regAddr,value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssSip6TxFifoDebugCountersDump function
* @endinternal
*
* @brief    dump the debug counter in the TxFifo unit
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] txFifoUnitIndex          - the  TxFifo unit index (0..numOfPipes*4)
*/
GT_STATUS   prvCpssSip6TxFifoDebugCountersDump
(
    IN GT_U8    devNum,
    IN GT_U32   txFifoUnitIndex

)
{
    GT_STATUS   rc;
    GT_U32  regAddr,value,ii;
    GT_U32  baseAddr;
    GT_CHAR*  counterNames[8] = {
       /*0*/  "descriptors read by read control"
       /*1*/  ,"cells split from the descriptors"
       /*2*/  ,"words generate in pre-fetch FIFO"
       /*3*/  ,"SOP read control derive to WB"
       /*4*/  ,"cycles from first descriptor"
       /*5*/  ,"Aligner input packets"
       /*6*/  ,"Aligner output packets"
       /*7*/  ,"Aligner output byte count"
    };

    baseAddr = TXFIFO_BASE_ADDR_GET(devNum, txFifoUnitIndex);

    baseAddr += 0x00005180;

    for(ii = 0 ; ii < 8 ; ii++)
    {
        regAddr = baseAddr + 4*ii;
        rc = prvCpssHwPpReadRegister(devNum,regAddr,&value);
        if(rc != GT_OK)
        {
            return rc;
        }

        cpssOsPrintf("counter[%s] at [0x%8.8x] with value [0x%8.8x] \n",
            counterNames[ii],regAddr,value);
    }

    return GT_OK;
}

GT_VOID prvCpssFalconTaiRegistersDump
(
    IN GT_U8    devNum
)
{
    GT_U32 gopIndex;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 totalTaiGop = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles * 9;

    for (gopIndex = 0; gopIndex < totalTaiGop; gopIndex++)
    {
        cpssOsPrintf("================= TAI Index [%d] ======================= \n", gopIndex % 9);

        cpssOsPrintf("TAIInterruptCause         [0x%8.8x] [0x%8.8x] \n", 
                     regsAddrPtr->GOP.TAI[gopIndex][0].TAIInterruptCause, regsAddrPtr->GOP.TAI[gopIndex][1].TAIInterruptCause);
        cpssOsPrintf("TAIInterruptMask          [0x%8.8x] [0x%8.8x] \n", 
                     regsAddrPtr->GOP.TAI[gopIndex][0].TAIInterruptMask, regsAddrPtr->GOP.TAI[gopIndex][1].TAIInterruptMask);
        cpssOsPrintf("TAICtrlReg0               [0x%8.8x] [0x%8.8x] \n", 
                     regsAddrPtr->GOP.TAI[gopIndex][0].TAICtrlReg0, regsAddrPtr->GOP.TAI[gopIndex][1].TAICtrlReg0);
        cpssOsPrintf("TAICtrlReg1               [0x%8.8x] [0x%8.8x] \n", 
                     regsAddrPtr->GOP.TAI[gopIndex][0].TAICtrlReg1, regsAddrPtr->GOP.TAI[gopIndex][1].TAICtrlReg1);
        cpssOsPrintf("timeCntrFunctionConfig0   [0x%8.8x] [0x%8.8x] \n", 
                     regsAddrPtr->GOP.TAI[gopIndex][0].timeCntrFunctionConfig0, regsAddrPtr->GOP.TAI[gopIndex][1].timeCntrFunctionConfig0);
        cpssOsPrintf("timeCntrFunctionConfig1   [0x%8.8x] [0x%8.8x] \n", 
                     regsAddrPtr->GOP.TAI[gopIndex][0].timeCntrFunctionConfig1, regsAddrPtr->GOP.TAI[gopIndex][1].timeCntrFunctionConfig1);
        cpssOsPrintf("timeCntrFunctionConfig1   [0x%8.8x] [0x%8.8x] \n", 
                     regsAddrPtr->GOP.TAI[gopIndex][0].timeCntrFunctionConfig2, regsAddrPtr->GOP.TAI[gopIndex][1].timeCntrFunctionConfig2);
    }
}

GT_VOID prvCpssFalconPtpRegistersDump
(
    IN GT_U8    devNum
)
{
    GT_U32 portNum;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 totalPorts = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles * 64;

    for (portNum = 0; portNum < totalPorts; portNum++)
    {
        if (portNum % 4 == 0)
        {
            cpssOsPrintf("================= PTP Index [%d] ======================= \n", portNum % totalPorts); 

            cpssOsPrintf("PTPInterruptCause         [0x%8.8x] [0x%8.8x] \n", 
                         regsAddrPtr->GOP.PTP[portNum][0].PTPInterruptCause,    regsAddrPtr->GOP.PTP[portNum][1].PTPInterruptCause);
            cpssOsPrintf("PTPInterruptMask          [0x%8.8x] [0x%8.8x] \n", 
                         regsAddrPtr->GOP.PTP[portNum][0].PTPGeneralCtrl,       regsAddrPtr->GOP.PTP[portNum][1].PTPGeneralCtrl);
            cpssOsPrintf("TAICtrlReg0               [0x%8.8x] [0x%8.8x] \n", 
                         regsAddrPtr->GOP.PTP[portNum][0].PTPGeneralCtrl,       regsAddrPtr->GOP.PTP[portNum][1].PTPGeneralCtrl);
            cpssOsPrintf("TAICtrlReg1               [0x%8.8x] [0x%8.8x] \n", 
                         regsAddrPtr->GOP.PTP[portNum][0].NTPPTPOffsetLow,      regsAddrPtr->GOP.PTP[portNum][1].NTPPTPOffsetLow);
            cpssOsPrintf("timeCntrFunctionConfig0   [0x%8.8x] [0x%8.8x] \n", 
                         regsAddrPtr->GOP.PTP[portNum][0].txPipeStatusDelay,    regsAddrPtr->GOP.PTP[portNum][1].txPipeStatusDelay);
            cpssOsPrintf("timeCntrFunctionConfig1   [0x%8.8x] [0x%8.8x] \n", 
                         regsAddrPtr->GOP.PTP[portNum][0].totalPTPPktsCntr,     regsAddrPtr->GOP.PTP[portNum][1].totalPTPPktsCntr);
            cpssOsPrintf("timeCntrFunctionConfig1   [0x%8.8x] [0x%8.8x] \n", 
                         regsAddrPtr->GOP.PTP[portNum][0].egressTimestampQueue, regsAddrPtr->GOP.PTP[portNum][1].egressTimestampQueue);
        }
    }
}

