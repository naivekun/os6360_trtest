/*******************************************************************************
*           Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsPortCalIf.c
*
* @brief This file contains API for port d2d calendars and credits configuartion
*
* @version   0
********************************************************************************
*/
#include <silicon/falcon/mvHwsPortCalIf.h>
#include <silicon/falcon/mvHwsFalconPortIf.h>
#include <cpss/common/labServices/port/gop/port/mac/d2dMac/mvHwsD2dMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#ifndef MV_HWS_REDUCED_BUILD
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>
#endif

extern  GT_BOOL hwsPpHwTraceFlag;

/*********************************D2D********************************************************/

/* falcon d2d port configuration */
#define D2D_PCS_NON_RSVD_CNS 59                  /*Define PCS calendar reseved slot ID */
#define D2D_LANES_NUM_CNS 17                     /*Define number of lanes in d2d */
#define D2D_PCS_CAL_LEN_CNS 85                   /*Define PCS calendar slice number*/
#define D2D_PCS_CAL_SLICES_IN_REGISTER_NUM_CNS 4 /*Define number of slices in PCS calendar register */
#define D2D_MAC_CAL_SLICES_IN_REGISTER_NUM_CNS 4 /*Define number of slices in MAC calendar register */
#define D2D_PCS_CAL_SLICE_OFFSET_CNS 6           /*Define offset between slices in PCS calendar register */
#define D2D_MAC_CAL_SLICE_OFFSET_CNS 8           /*Define offset between slices in MAC calendar register */
#define D2D_MAC_CAL_SLICES_NUM_CNS 160           /*Define MAC calendar slice number*/
#define D2D_CPU_CAL_SLICE_INDEX_CNS 16           /*Define cpu slice index in PCS and MAC calendars */
#define D2D_RAVEN_CPU_PORT_INDEX_CNS 16          /*Define cpu port number in d2d raven side */
#define D2D_EAGLE_CPU_PORT_INDEX_CNS 8           /*Define cpu port number in d2d eagle side */
#define D2D_RAVEN_PORTS_NUM_CNS 17               /*Define number of ports in d2d raven side */
#define D2D_EAGLE_PORTS_NUM_CNS 9                /*Define number of ports in d2d eagle side */
#define D2D_BANDWIDTH_CNS 400                    /*Define maximum bandwidth in d2d */

/* this table refer to 17 slices pcs calendars. the slices are according to 8 * 50G speed. cpu port get slice 16*/
static const GT_U8 pcsCalSliceMap[HWS_D2D_PORTS_NUM_CNS][2] =
{
    /*0*/ {0,   8   },
    /*1*/ {4,   12  },
    /*2*/ {2,   10  },
    /*3*/ {6,   14  },
    /*4*/ {1,   9   },
    /*5*/ {5,   13  },
    /*6*/ {3,   11  },
    /*7*/ {7,   15  },
};

/* this array refer to 16 slices mac calendar. the slices are according to 8 * 50G speed and set the offset of the first slice for each channel*/
static const GT_U8 macCalOffsets[D2D_RAVEN_PORTS_NUM_CNS] = {0, 4, 1, 5, 2, 6, 3, 7, 8, 12, 9, 13, 10, 14, 11, 15, 16};


#if (!defined MV_HWS_REDUCED_BUILD)
/* check if register address supported on emulator */
extern GT_BOOL prvCpssOnEmulatorSupportedAddrCheck(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      regAddr
);

GT_BOOL hwsIsEmultorSupportAddr(
    IN  GT_U8       devNum,
    IN  GT_U32      regAddr
)
{
    if(cpssDeviceRunCheck_onEmulator())
    {
        if(GT_FALSE == prvCpssOnEmulatorSupportedAddrCheck(devNum,0/*portGroupId*/,regAddr))
        {
            /* the memory/register is not in supported memory space */

            /* ignore the write operation */
            return GT_FALSE;
        }
    }

    return GT_TRUE;
}

/* if emulator not support the address ... just stop the function (return GT_OK)*/
/* support mode in which , we bypass ALL Raven access ,
   in such case the 'read' will return value '0' and not 'expectedData' */
#define HWS_RETURN_GT_OK_IF_EMULTOR_NOT_SUPPORT_ADDR_MAC(devNum,regAddr)    \
    if(GT_FALSE == hwsIsEmultorSupportAddr(devNum,regAddr))                 \
    {                                                                       \
        return GT_OK;                                                       \
    }
#else
#define HWS_RETURN_GT_OK_IF_EMULTOR_NOT_SUPPORT_ADDR_MAC(devNum,regAddr)    /*empty*/
#endif  /*!MV_HWS_REDUCED_BUILD*/


/*****************************************************************************************/

/**
* @internal mvHwsD2dSerdesSpeedAndLanesNumToBandwidthGet
*           function
* @endinternal
*
* @brief  calculates bandwidth acording to serdes speed and
*         lanes number
*
* @param[in] serdesSpeed                 - serdes speed
* @param[in] numOfActLanes               - port's active lanes
*       number
*
* @param[out] bandwidthPtr               - (pointer to) port
*       bandwidth
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dSerdesSpeedAndLanesNumToBandwidthGet
(
     MV_HWS_SERDES_SPEED   serdesSpeed,
     GT_U8                 numOfActLanes,
     HWS_D2D_BANDWIDTH_ENT *bandwidthPtr
)
{
    GT_U32 totalspeed = 0;
    GT_U32 speed;

    switch(serdesSpeed)
    {
        case _1_25G:
            speed = 1; /* 1 G */
            break;

        case _3_125G:
            speed = 3; /* 3 G */
            break;

        case _5_15625G:
            speed = 5; /* 5 G */
            break;

        case _10_3125G:
            speed = 10; /* 10 G */
            break;

        case _25_78125G:
        case _26_5625G :
        case _28_28125G:
            speed = 25; /* 25 G */
            break;

        case _26_5625G_PAM4:
        case _28_28125G_PAM4:
            speed = 50;  /* 50 G */
            break;

        default:
            return GT_BAD_PARAM;
    }

   totalspeed = (speed*((GT_U32)numOfActLanes));

   if (totalspeed == 0)
   {
       return GT_FAIL;
   }
   if (totalspeed <= 25) /* 25G and below -> 25G bandwidth*/
   {
       *bandwidthPtr = HWS_D2D_BANDWIDTH_25G_E;
   }
   else if (totalspeed <= 50) /* 40G , 50G -> 50G bandwidth*/
   {
        *bandwidthPtr = HWS_D2D_BANDWIDTH_50G_E;
   }
   else if (totalspeed == 100) /* 100G -> 100G bandwidth*/
   {
       *bandwidthPtr = HWS_D2D_BANDWIDTH_100G_E;
   }
   else if (totalspeed == 200) /* 200G -> 200G bandwidth*/
   {
       *bandwidthPtr = HWS_D2D_BANDWIDTH_200G_E;
   }
   else if (totalspeed == 400) /* 400G -> 400G bandwidth*/
   {
        *bandwidthPtr = HWS_D2D_BANDWIDTH_400G_E;
   }
   else
   {
       return  GT_FAIL;
   }

   return GT_OK;
}


/**
* @internal mvHwsD2dConfigChannelRxMacCreditsSet function
* @endinternal
*
* @brief  set Rx mac credits acording too channel index and
*         channel bandwidth
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
* @param[in] bandwidth                - port bandwidth
* @param[in] numChannels             - number of ports in d2d
* @param[in] rxMacUseCredits         - ignore credits flag.
*                                      True -  configure credits
*                                      False-  do not configure
*                                      credits
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dConfigChannelRxMacCreditsSet
(
    GT_U8                  devNum,
    GT_U32                 d2dNum,
    GT_U32                 channel,
    HWS_D2D_BANDWIDTH_ENT  bandwidth,
    GT_U32                 numChannels,
    GT_BOOL                rxMacUseCredits
)
{
    GT_U32       fifoPointer;            /* fifo pointer that point to the first segment index*/
    GT_U32       numRxSegments;          /* Rx channel segments number, channel fifo size = segments Number * segment size*/
    GT_U32       xfastLevel, xslowLevel; /* flow control levels for rate control.*/
    GT_U32       xonLevel, xoffLevel;    /* flow control levels for backpressure */
    GT_U32       counter;                /* use for calculations to set flow control levels: xonLevel, xoffLevel, xSlow , xFast*/
    GT_U32       d2dIndex;
    GT_U32       unitIndex, unitNum;
    GT_U32       regAddr;
    GT_U32       regMask, baseAddr;
    GT_U32       data;
    GT_U32       ii,index;

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (D2D_RAVEN_PORTS_NUM_CNS == numChannels) /*Raven*/
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
        fifoPointer = (channel < HWS_D2D_PORTS_NUM_CNS ? (channel*4) : (channel == D2D_RAVEN_CPU_PORT_INDEX_CNS ? 32: (((channel%HWS_D2D_PORTS_NUM_CNS) * 4) + 2)));
        if(prvCpssDxchFalconCiderVersionGet() >=  PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_03_13_E )
        {
            numRxSegments = (channel == D2D_RAVEN_CPU_PORT_INDEX_CNS ? 2 : ((bandwidth*32)/D2D_BANDWIDTH_CNS));
        }
        else
        {
            numRxSegments = (channel == D2D_RAVEN_CPU_PORT_INDEX_CNS ? 1 : ((bandwidth*32)/D2D_BANDWIDTH_CNS));
        }
    }
    else /*Eagle*/
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
        fifoPointer = (channel < HWS_D2D_PORTS_NUM_CNS ?  (channel*2) : (channel == 16 ? 16: (((channel%8) * 2) + 1)));
        numRxSegments = (channel == D2D_EAGLE_CPU_PORT_INDEX_CNS ? 1 : ((bandwidth*16)/D2D_BANDWIDTH_CNS));
    }

    if (GT_TRUE == rxMacUseCredits) /* Rx on eagle does not use credits */
    {
        /* Config the channel fifo according to the channel bandwidth. The fifo pointer point to the first fifo segment.
        The last fifo segment point to the first fifo segment*/
        regMask = 0x3F;
        for (ii = 0 ; ii < numRxSegments; ii++)
        {
            index = fifoPointer + ii;
            regAddr = baseAddr + index * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_FIFO_SEG_POINTER;

            if (ii == (numRxSegments - 1))
            {
                data = fifoPointer; /* the last "next segment" point to the head*/
            }
            else
            {
                data = index + 1; /*"next" segment pointer*/
            }
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, regMask));
        }
    }

    /* set channel select bandwdith ( 0: 100G and below, 1: 200G, 2: 400G) */
    regMask = 0x3 << ((channel%HWS_D2D_PORTS_NUM_CNS) * HWS_D2D_MAC_CHANNEL_STEP);
    regAddr = baseAddr + D2D_MAC_CFG_RX_FIFO_BANDWIDTH_SELECT + ((channel/HWS_D2D_PORTS_NUM_CNS) * HWS_D2D_MAC_CHANNEL_STEP);
    data = ((2*bandwidth)/D2D_BANDWIDTH_CNS)<< ((channel%HWS_D2D_PORTS_NUM_CNS) * HWS_D2D_MAC_CHANNEL_STEP);
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, regMask));

    /* set CFG_RX_CHANNEL_2*/
    regMask = 0x3FFFFFFF;
    regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL_2;

    /* calculates flow control level for rate control. xslow > xfast */
    counter = ((2*(16*bandwidth))/D2D_BANDWIDTH_CNS);

    if(prvCpssDxchFalconCiderVersionGet() >=  PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_03_13_E )
    {
        /* disable Xfast, Xslow*/
        xfastLevel = 0x3FF;
        xslowLevel = 0x3FF;

        xonLevel = (counter > 8 ? (6*counter-11) : (counter > 3 ? (5*counter)-1 : (4*counter-1)));
        xoffLevel = (counter > 8 ? (6*counter-10) : (counter > 3 ? (5*counter) : 4*counter));
    }
    else
    {
        /* calculates Xfast, Xslow*/
        xfastLevel = counter;
        xslowLevel = (counter+1);

        /* calculates flow control level for backpressure. xoff > xon */
        xonLevel = ((16/counter)+2);
        xoffLevel = (counter > 4 ? (6*counter) : (7*counter));
    }
    data = ((xfastLevel << 20) | (xslowLevel << 10) | xonLevel);
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, regMask));

    /*
    set initial channel rx credits to 0. rx not use credits since it send massage and not get massage
    set fifo pointer
    set Xoff level
    disable Speculative Flow Control.
    ignore credits in Rx on eagle side
    configure crc definition to 1
    */
    regMask = 0xFFFFFFFF;
    regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL;
    if(prvCpssDxchFalconCiderVersionGet() >=  PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_03_13_E )
    {
        data = ( 0x40000000 |(~rxMacUseCredits << 29)| (0 << 28) | (xoffLevel << 17) | (fifoPointer << 10) | 0 );
    }
    else
    {
        data = ( 0x40000000 |(~rxMacUseCredits << 29)| (1 << 28) | (xoffLevel << 17) | (fifoPointer << 10) | 0 );
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, regMask));

    return GT_OK;
}

/**
* @internal mvHwsD2dConfigChannelTxMacCreditsSet function
* @endinternal
*
* @brief  set Tx mac credits acording too channel index and
*         channel bandwidth
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
* @param[in] bandwidth                - port bandwidth
* @param[in] numChannels             - number of ports in d2d
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dConfigChannelTxMacCreditsSet
(
    GT_U8                  devNum,
    GT_U32                 d2dNum,
    GT_U32                 channel,
    HWS_D2D_BANDWIDTH_ENT  bandwidth,
    GT_U32                 numChannels
)
{

    GT_U32       fifoPointer;        /* fifo pointer that point to the first segment index*/
    GT_U32       numTxSegments;      /* Tx channel segments number, channel fifo size = segments Number * segment size*/
    GT_U32       credits;            /* initial channel tx credits value*/
    GT_U32       d2dIndex;
    GT_U32       unitIndex, unitNum;
    GT_U32       regAddr;
    GT_U32       regMask, baseAddr;
    GT_U32       ii, index, data;

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);

    /* the below calculations are according to :
       ((txSegmentsNum * segmentSize)/ 32B ) = creditsNum*/
    if (D2D_RAVEN_PORTS_NUM_CNS == numChannels)
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));

        fifoPointer = (channel == D2D_RAVEN_CPU_PORT_INDEX_CNS ? 24 : (channel*3));
        numTxSegments = (channel == D2D_RAVEN_CPU_PORT_INDEX_CNS ? 1 : ((bandwidth*24)/D2D_BANDWIDTH_CNS));
        /* tx raven segment size is 128B */
        credits = (4*numTxSegments);
    }
    else /* eagle */
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
        fifoPointer = channel*2;
        numTxSegments = (channel == D2D_EAGLE_CPU_PORT_INDEX_CNS ? 1 : ((bandwidth*16)/D2D_BANDWIDTH_CNS));
        /* tx eagle segment size is 192B */
        credits = (6*numTxSegments);
    }
    /* Config the channel fifo according to the channel bandwidth. The fifo pointer point to the first fifo segment.
       The last fifo segment point to the first fifo segment*/
    regMask = 0x3F;
    for (ii = 0 ; ii < numTxSegments; ii++)
    {
        index = fifoPointer + ii;
        regAddr = baseAddr + index * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_FIFO_SEG_POINTER;

        if (ii == (numTxSegments - 1))
        {
            data = fifoPointer; /* the last "next segment" point to the head*/
        }
        else
        {
            data = index + 1; /*"next" segment pointer*/
        }
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, regMask));
    }
    /*
    set channel select bandwdith ( 0: 100G and below, 1: 200G, 2: 400G)
    set fifo pointer
    set initial credits
    configure crc definition to 1
    */
    regMask = 0x3FFFFFFF;
    regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_CHANNEL;
    data =  ( 0x60000000 | (((2*bandwidth)/D2D_BANDWIDTH_CNS) << 17) | (fifoPointer << 10) | credits);
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, regMask));

    return GT_OK;
}

/**
* @internal mvHwsD2dMacCreditsGet function
* @endinternal
*
* @brief  Get Tx and Rx mac credits for specific channel index
*         and D2D number
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
*
* @param[out] txChannelCreditsArr     - array of tx credits
*       parameters
* @param[out] rxChannelCreditsArr     - array of rx credits
*       parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dMacCreditsGet
(
    GT_U8                  devNum,
    GT_U32                 d2dNum,
    GT_U32                 channel,
    GT_U32                 *txChannelCreditsArr,
    GT_U32                 *rxChannelCreditsArr
)
{

    GT_U32       d2dIndex;
    GT_U32       unitIndex, unitNum;
    GT_U32       regAddr, segMask;
    GT_U32       regMask, baseAddr;
    GT_U32       ii, regData;

    /* tx*/
    GT_U32 numTxSegments;
    GT_U32 txInitCreditMask, txInitCredit;
    GT_U32 txFifoPointerMask, txFifoPointer;
    GT_U32 txFifoBandwidthMask, txFifoBandwidth;
    GT_U32 txChannelSegmentNum, rxChannelSegmentNum;
    /*rx*/
    GT_U32 numRxSegments;
    GT_U32 rxFifoBandwidthMask, rxFifoBandwidth;
    GT_U32 rxFifoPointerMask, rxFifoPointer;
    GT_U32 rxInitCreditMask, rxInitCredit;
    GT_U32 rxXonMask, rxXon;
    GT_U32 rxXoffMask, rxXoff;
    GT_U32 rxXfastMask, rxXfast;
    GT_U32 rxXslowMask, rxXslow;

    segMask = 0x3F;
    regMask = 0x3FFFFFFF;

    /*TX CREDITS */
    txChannelSegmentNum = 0;
    txInitCreditMask = 0x3FF;
    txFifoPointerMask = 0x1FC00;
    txFifoBandwidthMask = 0x60000;

    /*Rx*/
    rxChannelSegmentNum = 0;
    rxInitCreditMask = 0x3FF;
    rxFifoPointerMask = 0x1FC00;
    rxXonMask = 0x3FF;
    rxXoffMask = 0x7FE0000;
    rxXslowMask = 0xFFC00;
    rxXfastMask = 0x3FF00000;


    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
        numTxSegments = 16;
        numRxSegments = 16;
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
        numTxSegments = 24;
        numRxSegments = 32;
    }


    /* Tx channel*/
    regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_CHANNEL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    txInitCredit = regData & txInitCreditMask;
    txFifoPointer = (regData & txFifoPointerMask) >> 10;
    txFifoBandwidth = (regData & txFifoBandwidthMask) >> 17;

    /* Config the channel fifo according to the channel bandwidth. The fifo pointer point to the first fifo segment.
       The last fifo segment point to the first fifo segment*/
    if (txFifoPointer > numTxSegments)
    {
        return GT_FAIL;
    }

    for (ii = txFifoPointer ; ii < numTxSegments; ii++)
    {
        regAddr = baseAddr + ii * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_FIFO_SEG_POINTER;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, segMask));
        txChannelSegmentNum++;
        if (regData == txFifoPointer)
        {
            break;
        }
    }

    /* set tx channel values*/

    /* init credits*/
    txChannelCreditsArr[0] = txInitCredit;
    /*tx fifo pointer */
    txChannelCreditsArr[1] = txFifoPointer;
    /*selected bandwidth*/
    txChannelCreditsArr[2] = txFifoBandwidth;
    /* tx fifo segments number*/
    txChannelCreditsArr[3] = txChannelSegmentNum;


    /* Rx channel*/
    /* rx fifo bandwidth */
    rxFifoBandwidthMask = 0x3 << ((channel%HWS_D2D_PORTS_NUM_CNS) * HWS_D2D_MAC_CHANNEL_STEP);
    regAddr = baseAddr + D2D_MAC_CFG_RX_FIFO_BANDWIDTH_SELECT + ((channel/HWS_D2D_PORTS_NUM_CNS) * HWS_D2D_MAC_CHANNEL_STEP);
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, rxFifoBandwidthMask));

    rxFifoBandwidth = regData >> ((channel%HWS_D2D_PORTS_NUM_CNS) * HWS_D2D_MAC_CHANNEL_STEP);
    /*rx channel 2 */
    regMask = 0x3FFFFFFF;
    regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL_2;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    rxXon = regData & rxXonMask;
    rxXslow = (regData & rxXslowMask) >> 10;
    rxXfast = (regData & rxXfastMask) >> 20;

    /* rx channel*/
    regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    rxInitCredit = regData & rxInitCreditMask;
    rxFifoPointer = (regData & rxFifoPointerMask) >> 10;
    rxXoff = (regData & rxXoffMask) >> 17;

    if (rxFifoPointer > numRxSegments)
    {
        return GT_FAIL;
    }

    for (ii = rxFifoPointer ; ii < numRxSegments; ii++)
    {
        regAddr = baseAddr + ii * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_FIFO_SEG_POINTER;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, segMask));
        rxChannelSegmentNum++;
        if (regData == rxFifoPointer)
        {
            break;
        }
    }


    /* set rx channel values*/
    /* init credits*/
    rxChannelCreditsArr[0] = rxInitCredit;
    /*rx fifo pointer*/
    rxChannelCreditsArr[1] = rxFifoPointer;
    /*selected bandwidth*/
    rxChannelCreditsArr[2] = rxFifoBandwidth;
    /* tx fifo segments number*/
    rxChannelCreditsArr[3] = rxChannelSegmentNum;
    /*xon*/
    rxChannelCreditsArr[4] = rxXon;
    /*xoff*/
    rxChannelCreditsArr[5] = rxXoff;
    /*xfast*/
    rxChannelCreditsArr[6] = rxXfast;
    /*xslow*/
    rxChannelCreditsArr[7] = rxXslow;


   return GT_OK;
}




/**
* @internal mvHwsD2dBandwidthToPortsNumGet function
* @endinternal
*
* @brief  conver bandwidth to number of d2d ports (0 ..7)
*         required for the bandwidth
*
* @param[in] bandwidth                - port bandwidth
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
*
* @param[out] portsNumPtr             - number of ports required
*       for the bandwidth
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dBandwidthToPortsNumGet
(
    HWS_D2D_BANDWIDTH_ENT  bandwidth,
    GT_U32                 d2dNum,
    GT_U32                 channel,
    GT_U32                 *portsNumPtr
)
{
    /* Raven cpu port*/
    if ((channel == D2D_RAVEN_CPU_PORT_INDEX_CNS) && (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum)))
    {
        return GT_BAD_PARAM;
    }

    /* Eagle cpu port*/
    if ((channel == D2D_EAGLE_CPU_PORT_INDEX_CNS) && (!PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum)))
    {
        return GT_BAD_PARAM;
    }

    /* cpu port can support only 25G bandwidth*/
    if (channel == D2D_RAVEN_CPU_PORT_INDEX_CNS || channel == D2D_EAGLE_CPU_PORT_INDEX_CNS)
    {
        if (bandwidth != HWS_D2D_BANDWIDTH_25G_E)
        {
             HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("cpu port support 25G bandwidth only"));
        }
    }

    if ((channel > 7) && ((channel != D2D_EAGLE_CPU_PORT_INDEX_CNS) && (channel != D2D_RAVEN_CPU_PORT_INDEX_CNS)))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("channel number is not supported"));
    }
    /*
    400G requires 8 ports ( 8 * 50), supported only on channel index 0
    200G requires 4 ports (4 * 50), supported only on channel indexes  0, 4
    100G requires 2 ports (2 * 50), supported only on channel indexes  0, 2, 4, 6
    50G  requires 1 ports, supported on all channel indexes  0 .. 7
    25G  requires 1 ports, supported on all channel indexes  0 .. 7 and cpu port 8 or 16

    bandwidth of 50G  R2 (from 2* 25) is equivalent to bandwidth of 50G  R1 (from 1 * 50G) and has the same calendar configuration
    bandwidth of 100G R4 (from 4* 25) is equivalent to bandwidth of 100G R2 (from 2 * 50G) and has the same calendar configuration
    bandwidth of 200G R8 (from 8* 25) is equivalent to bandwidth of 200G R4 (from 4 * 50G) and has the same calendar configuration
    bandwidth of 40G R8  (from 4* 10) is equivalent to bandwidth of 50G  R1 (from 1 * 50G) and has the same calendar configuration

    the calendars configuratins is according to bandwidth and not take in acount number af active lanes
    */
    *portsNumPtr = 0;
    switch (bandwidth)
    {
        case HWS_D2D_BANDWIDTH_400G_E:
            if (channel == 0)
            {
                 *portsNumPtr = 8;
            }
            break;

        case HWS_D2D_BANDWIDTH_200G_E:
            if (channel%4 == 0)
            {
                *portsNumPtr = 4;
            }
            break;

        case HWS_D2D_BANDWIDTH_100G_E:
            if (channel%2 == 0)
            {
              *portsNumPtr = 2;
            }
            break;

        case HWS_D2D_BANDWIDTH_25G_E:
        case HWS_D2D_BANDWIDTH_50G_E:
            *portsNumPtr = 1;
            break;

        default:
           return GT_BAD_PARAM;
    }
    if ((*portsNumPtr) == 0)
    {
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dConfigChannelPcsCalDirSet function
* @endinternal
*
* @brief  configuration of PCS D2D calendar for specific channel
*         and bandwidth. acording to pcs 17 slices calendar. the
*         configuration done twice for RX or TX calendar
*
* @param[in] bandwidth                - port bandwidth
* @param[in] pcs17SlicesCalArr       - array of the first 17
*       slices calander configured according the channel
*       bandwidth that should be duplicated 5 times
* @param[in] pcsCalUnitDirAdrr        - d2d TX or RX PCS
*       calender first addres
* @param[in] channel                  - d2d local channel index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dConfigChannelPcsCalDirSet
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  *pcs17SlicesCalArr,
    GT_U32  pcsCalUnitDirAdrr,
    GT_U32  channel
)
{
    GT_U32     laneIndex;            /* index of first 17 slices pcs calendar*/
    GT_U32     ii;                   /* duplicate index (0 ..4) of lice pcs calendar */
    GT_U32     sliceIndex;           /* slice index in pcs calendar , 0 ..84*/
    GT_U32     sliceIndexInRegister; /* each pcs calendar register include 4 slices. the index is the local slice (0 ..3) */
    GT_U32     pcsCalRegisterIndex;  /* index for pcs calendar registers (0 ..21)*/
    GT_U32     expectedData;         /* expected "reserved" value for slices that not belong to any port*/
    GT_U32     regData;
    GT_U32     d2dIndex;
    GT_U32     unitIndex, unitNum;
    GT_U32     regAddr, baseAddr;
    GT_U32     regMask, data;

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }


    /* Enable RX PCS cal */
    for (laneIndex = 0; laneIndex < D2D_LANES_NUM_CNS; laneIndex++)
    {
        if (1 == pcs17SlicesCalArr[laneIndex])
        {
            for (ii = 0 ; ii < (D2D_PCS_CAL_LEN_CNS/D2D_LANES_NUM_CNS); ii++)
            {
                sliceIndex = laneIndex + ii*D2D_LANES_NUM_CNS;
                sliceIndexInRegister = sliceIndex%D2D_PCS_CAL_SLICES_IN_REGISTER_NUM_CNS;
                pcsCalRegisterIndex = sliceIndex/D2D_PCS_CAL_SLICES_IN_REGISTER_NUM_CNS;
                regMask = 0x3F<<(sliceIndexInRegister * D2D_PCS_CAL_SLICE_OFFSET_CNS);

                /*check that the slice is valid*/
                regAddr = baseAddr + pcsCalUnitDirAdrr + (pcsCalRegisterIndex * 0x4);

                /* if emulator not support the address ... just stop the function (return GT_OK)*/
                /* support mode in which , we bypass ALL Raven access ,
                   in such case the 'read' will return value '0' and not 'expectedData' */
                HWS_RETURN_GT_OK_IF_EMULTOR_NOT_SUPPORT_ADDR_MAC(devNum,regAddr);

                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
                expectedData = D2D_PCS_NON_RSVD_CNS << (sliceIndexInRegister * D2D_PCS_CAL_SLICE_OFFSET_CNS);
                if ( regData != expectedData)
                {
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("PCS RX cal: slice is already belong to other port"));
                }

                /* set the channel index */
                data = channel << (sliceIndexInRegister * D2D_PCS_CAL_SLICE_OFFSET_CNS);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, regMask));
            }
        }
    }

    return GT_OK;
}


/**
* @internal mvHwsD2dConfigChannelPcsCalSet function
* @endinternal
*
* @brief  configuration of PCS D2D calendar for specific channel
*         and bandwidth. the configuration done twice for RX and
*         TX calendar that are the same.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
* @param[in] bandwidth                - port bandwidth
* @param[in] portRangeLen             - number of ports required
*       for the bandwidth
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dConfigChannelPcsCalSet
(
    GT_U8                  devNum,
    GT_U32                 d2dNum,
    GT_U32                 channel,
    HWS_D2D_BANDWIDTH_ENT  bandwidth,
    GT_U32                 portRangeLen
)
{
    GT_STATUS  rc;
    GT_U32     channelIndex;
    GT_U32     pcs17SlicesCalArry[D2D_LANES_NUM_CNS];

    hwsOsMemSetFuncPtr(&pcs17SlicesCalArry,0,(D2D_LANES_NUM_CNS*sizeof(GT_U32)));

    /* pcs calendars incled 85 slice. the channels bandwidth deteremine the first 17 slices (0 .. 16)
       this first 17 slices are duplicate 5 times in the calendar.
    */

    if ((channel == D2D_RAVEN_CPU_PORT_INDEX_CNS) || (channel == D2D_EAGLE_CPU_PORT_INDEX_CNS)) /* cpu port */
    {
        if (portRangeLen != 1)
        {
            return GT_BAD_PARAM;
        }
        else
        {
            pcs17SlicesCalArry[D2D_CPU_CAL_SLICE_INDEX_CNS] = 1;
        }
    }
    else  /* 0..7 ports */
    {
        if (((channel + portRangeLen)> HWS_D2D_PORTS_NUM_CNS))
        {
            return GT_BAD_PARAM;
        }

        /*make array of slice indexes for 17 slices calendar*/
        for (channelIndex = channel; channelIndex < (channel + portRangeLen); channelIndex++)
        {
            pcs17SlicesCalArry[(GT_U32)(pcsCalSliceMap[channelIndex][0])] = 1;
            if (bandwidth > HWS_D2D_BANDWIDTH_25G_E)
            {
                pcs17SlicesCalArry[(GT_U32)(pcsCalSliceMap[channelIndex][1])] = 1;

            }
        }
    }
    /* set pcs Rx calendar configuration*/
    rc = mvHwsD2dConfigChannelPcsCalDirSet(devNum,d2dNum,pcs17SlicesCalArry,D2D_PCS_PCS_RX_CAL_BASE,channel);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set pcs Tx calendar configuration*/
    rc = mvHwsD2dConfigChannelPcsCalDirSet(devNum,d2dNum,pcs17SlicesCalArry,D2D_PCS_PCS_TX_CAL_BASE,channel);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal mvHwsD2dConfigChannelTdmMacRxSet function
* @endinternal
*
* @brief  configuration Mac TDM RX calendar for enable Port, D2D
*         TX and D2D RX. The configuration performs mac tdm rx
*         calendar slices assignment for specific channel and
*         bandwidth. this function should be called twise, from
*         eagle side and from raven side.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
* @param[in] bandwidth                - port bandwidth
* @param[in] numChannels              - channels number in d2d
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dConfigChannelTdmMacRxSet
(
    GT_U8                 devNum,
    GT_U32                d2dNum,
    GT_U32                channel,
    HWS_D2D_BANDWIDTH_ENT bandwidth,
    GT_U32                numChannels
)
{
    GT_U32       macCalRegistersNum;   /* registers number in mac calendar, calendar length is 160. 160/4 = 40*/
    GT_U32       macStride;            /* the gap between channel slices acording to the bandwidth*/
    GT_U32       sliceIndex;           /* slice index in mac calendar , 0 ..159*/
    GT_U32       fieldIndex;           /* each mac calendar register include 4 slices. the index is the local slice (0 ..3) */
    GT_U32       registerIndex;        /* index for mac calendar registers (0 ..39)*/
    GT_U32       d2dIndex;
    GT_U32       unitIndex, unitNum;
    GT_U32       regAddr, regData, data;
    GT_U32       regMask, regMaskValid, baseAddr;

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    macStride = (D2D_BANDWIDTH_CNS/bandwidth);
    sliceIndex = ((channel == (numChannels -1)) ? macCalOffsets[D2D_CPU_CAL_SLICE_INDEX_CNS] : macCalOffsets[channel]);

    /* go over calendar slices*/
    macCalRegistersNum = D2D_MAC_CAL_SLICES_NUM_CNS/D2D_MAC_CAL_SLICES_IN_REGISTER_NUM_CNS;
    for (registerIndex = 0 ; registerIndex < macCalRegistersNum ; registerIndex ++ )
    {
         regAddr = baseAddr + D2D_MAC_CFG_RX_TDM_SCHEDULE + (registerIndex * HWS_D2D_MAC_CHANNEL_STEP);
         for (fieldIndex = 0 ; fieldIndex < D2D_MAC_CAL_SLICES_IN_REGISTER_NUM_CNS ; fieldIndex++)
         {
             if (((D2D_MAC_CAL_SLICES_IN_REGISTER_NUM_CNS*registerIndex) + fieldIndex) == sliceIndex)
             {
                  /* this slice belong to the channel*/
                 if (((sliceIndex%D2D_RAVEN_PORTS_NUM_CNS) != D2D_CPU_CAL_SLICE_INDEX_CNS) || (channel == (numChannels -1)))
                 {
                     regMaskValid = 0x80<<(fieldIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS);
                     regMask = 0x3F<<(fieldIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS);
                     CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMaskValid));

                     /*first check that the slice not belong to other port, its have to be invalid !! ( = 0)*/
                     if (regData == regMaskValid)
                     {
                         HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("MAC TDM RX cal: slice is already belong to other port"));
                     }

                     /* set the channel index in the slice and set the valid bit to 1*/
                     regMask = (regMask | regMaskValid);
                     data = ((channel << (fieldIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS)) | regMaskValid);
                     CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, regMask));
                 }

                 /* calculate the next step offset*/
                 if (macStride == 2)
                 {
                    sliceIndex += (sliceIndex % 2 == 0 ? 1 : 3);
                 }
                 else
                 {
                     sliceIndex += macStride;
                     if (channel == (numChannels-1))
                     {
                         sliceIndex += 1;
                     }
                 }
             }
         }
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dConfigChannelMacSet function
* @endinternal
*
* @brief  configuration Mac resources for enable Port , D2D TX
*         and D2D RX. The configuration performs mac tdm rx
*         calendar slices assignment and Tx and Rx credits for
*         specific channel and bandwidth. this function should
*         be called twise, from eagle side and from raven side.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
* @param[in] bandwidth                - port bandwidth
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dConfigChannelMacSet
(
    GT_U8                 devNum,
    GT_U32                d2dNum,
    GT_U32                channel,
    HWS_D2D_BANDWIDTH_ENT bandwidth
)
{
    GT_STATUS    rc;
    GT_U32       numChannels;
    GT_BOOL      rxMacUseCredits; /* d2d rx on eagle side ignores credits */

    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        rxMacUseCredits = GT_FALSE;
        numChannels = D2D_EAGLE_PORTS_NUM_CNS;
    }
    else
    {
        rxMacUseCredits = GT_TRUE;
        numChannels = D2D_RAVEN_PORTS_NUM_CNS;
    }

    /*set Rx mac credits*/
    rc = mvHwsD2dConfigChannelRxMacCreditsSet(devNum, d2dNum, channel, bandwidth, numChannels, rxMacUseCredits);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*set Rx mac tdm calendar*/
    rc = mvHwsD2dConfigChannelTdmMacRxSet(devNum, d2dNum, channel, bandwidth, numChannels);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*set Tx mac credits*/
    rc = mvHwsD2dConfigChannelTxMacCreditsSet(devNum, d2dNum, channel, bandwidth, numChannels);

    return rc;
}

/**
* @internal mvHwsD2dConfigChannel function
* @endinternal
*
* @brief  Enable Port , D2D TX and D2D RX.
*         The configuration performs enable of Tx and Rx on for
*         specific channel for PCS calendar , MAC TDM RX
*         calendar, and credits. this function should be called
*         twise, from eagle side and from raven side.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
* @param[in] serdesSpeed              - serdes speed
* @param[in] numOfActLanes            - active lanes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dConfigChannel
(
    GT_U8               devNum,
    GT_U32              d2dNum,
    GT_U32              channel,
    MV_HWS_SERDES_SPEED serdesSpeed,
    GT_U8               numOfActLanes
)
{
    /* power up */
    /* 1. set TX pcs calendar and RX channel slices
       2. set mac calendar channel time slots
       3. set channel credits
    */

    GT_STATUS rc;
    HWS_D2D_BANDWIDTH_ENT bandwidth; /* channel bandwidth : 25G, 50G, 100G, 200G, 400Gs*/
    GT_U32  portRangeLen = 0;            /* number of ports that requires for the channel bandwidth */

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsD2dConfigChannel ******\n");
    }
#endif

    /* get port bandwidth*/
    rc = mvHwsD2dSerdesSpeedAndLanesNumToBandwidthGet(serdesSpeed, numOfActLanes,&bandwidth);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* validate the the port support the bandwidth and get number of ports needed for the channel*/
    rc = mvHwsD2dBandwidthToPortsNumGet(bandwidth, d2dNum, channel, &portRangeLen);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* pcs calendar TX and RX configuration */
    rc = mvHwsD2dConfigChannelPcsCalSet(devNum, d2dNum, channel, bandwidth, portRangeLen);
    if (rc != GT_OK)
    {
       return rc;
    }

    /* mac calendar and mac Rx and Tx credits configuration */
    rc = mvHwsD2dConfigChannelMacSet(devNum, d2dNum, channel, bandwidth);

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsD2dConfigChannel ******\n");
    }
#endif
    return rc;
}

/**
* @internal mvHwsD2dDisableChannelPcsCalDir function
* @endinternal
*
* @brief  remove channel pcs calendar slices for pcs tx or rx
*         calendar .
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
* @param[in] pcsUnitDirAddr           -  start addres of pcs tx
*       or rx calendar slices
* @param[in] pcsCalRegNum             - number of pcs calendar
*       registers
*
* @param[out] removedSlicesNum         - pointer to number of
*       slices that found and removed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dDisableChannelPcsCalDir
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channel,
    GT_U32  pcsUnitDirAddr,
    GT_U32  pcsCalRegNum,
    GT_U32  *removedSlicesNum
)
{
    GT_U32       d2dIndex;
    GT_U32       pcsCalSliceCounter = 0;
    GT_U32       registerIndex, fieldIndex;
    GT_U32       unitIndex, unitNum;
    GT_U32       regAddr, regData, data, channelData;
    GT_U32       regMask, baseAddr;


    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    for ( registerIndex = 0 ; registerIndex < pcsCalRegNum ; registerIndex++ )
    {
        regAddr = baseAddr + pcsUnitDirAddr + (registerIndex * HWS_D2D_MAC_CHANNEL_STEP);
        for (fieldIndex = 0 ; fieldIndex < D2D_PCS_CAL_SLICES_IN_REGISTER_NUM_CNS ; fieldIndex++)
        {
            regMask = 0x3F<<(fieldIndex * D2D_PCS_CAL_SLICE_OFFSET_CNS);
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
            channelData = channel <<(fieldIndex * D2D_PCS_CAL_SLICE_OFFSET_CNS);
            if (regData == channelData)
            {
                data = D2D_PCS_NON_RSVD_CNS << (fieldIndex * D2D_PCS_CAL_SLICE_OFFSET_CNS);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, regMask));
                pcsCalSliceCounter++;
            }
         }
    }
    *removedSlicesNum = pcsCalSliceCounter;

    return GT_OK;
}

/**
* @internal mvHwsD2dDisableAll50GChannelsPcsCalDir function
* @endinternal
*
* @brief  remove channel pcs calendar slices for pcs tx or rx
*         calendar.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] pcsUnitDirAddr           -  start addres of pcs tx
*       or rx calendar slices
* @param[in] pcsCalRegNum             - number of pcs calendar
*       registers
*
* @param[out] channelsRemovedSliceCounterArr     - pointer to
*       array of counters. each cell holds the number of slices
*       that found and removed for the channel
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dDisableAll50GChannelsPcsCalDir
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  pcsUnitDirAddr,
    GT_U32  pcsCalRegNum,
    GT_U32  *channelsRemovedSliceCounterArr
)
{
    GT_U32       d2dIndex;
    GT_U32       registerIndex, fieldIndex;
    GT_U32       unitIndex, unitNum;
    GT_U32       regAddr, regData, sliceData;
    GT_U32       regMask = 0xFFFFFF;
    GT_U32       baseAddr;
    GT_U32       disableSlicesRegData = (D2D_PCS_NON_RSVD_CNS | (D2D_PCS_NON_RSVD_CNS << D2D_PCS_CAL_SLICE_OFFSET_CNS) | (D2D_PCS_NON_RSVD_CNS << (2*D2D_PCS_CAL_SLICE_OFFSET_CNS)) |(D2D_PCS_NON_RSVD_CNS << (3*D2D_PCS_CAL_SLICE_OFFSET_CNS)));

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    for ( registerIndex = 0 ; registerIndex < pcsCalRegNum ; registerIndex++ )
    {
        regAddr = baseAddr + pcsUnitDirAddr + (registerIndex * HWS_D2D_MAC_CHANNEL_STEP);
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
        for (fieldIndex = 0 ; fieldIndex < D2D_PCS_CAL_SLICES_IN_REGISTER_NUM_CNS ; fieldIndex++)
        {
            sliceData = (regData & 0x3F);
            if (sliceData < HWS_D2D_PORTS_NUM_CNS)
            {
                channelsRemovedSliceCounterArr[sliceData]++;
            }
            else
            if (((registerIndex*4) + fieldIndex) == (D2D_PCS_CAL_LEN_CNS -1) )
            {
                break;
            }

            regData = (regData >> D2D_PCS_CAL_SLICE_OFFSET_CNS);
         }
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, disableSlicesRegData, regMask));
        if (((registerIndex*4) + fieldIndex) == D2D_PCS_CAL_LEN_CNS )
        {
            break;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dPcsCalDirConfigurationGet function
* @endinternal
*
* @brief  count the slices each channel holds in pcs tx or rx
*         calendar .
*
* @param[in] devNum                   - system device number
* @param[in] baseAddr                 - d2d base addres
* @param[in] dirUnitAddr              -  start addres of pcs tx
*       or rx calendar slices
* @param[in] cpuPort                  - cpu port number
*
* @param[out] removedSlicesDir     - pointer to array of
*       counters. the channel is the cell index. each cell holds
*       the number of the channels slices
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dPcsCalDirConfigurationGet
(
    GT_U8               devNum,
    GT_U32              baseAddr,
    GT_U32              dirUnitAddr,
    GT_U32              cpuPort,
    GT_U32              *removedSlicesDir
)
{
    GT_U32 regData, sliceMask, regAddr,sliceData;
    GT_U32 currSliceIndex = 0;
    GT_U32 currOffsetIndex = 0;
    GT_U32 currRegisterIndex = 0;

    /* go over all pcs calendar Rx or Tx slice and count the number of slice that each channel holds*/
    do {
        sliceMask = 0x3F<< (currOffsetIndex *D2D_PCS_CAL_SLICE_OFFSET_CNS);
        regAddr = baseAddr + dirUnitAddr + (currRegisterIndex * HWS_D2D_MAC_CHANNEL_STEP);
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, sliceMask));
        sliceData = regData >>(currOffsetIndex * D2D_PCS_CAL_SLICE_OFFSET_CNS);
        if (sliceData != D2D_PCS_NON_RSVD_CNS)
        {
            if ((sliceData == cpuPort)&&(cpuPort == D2D_RAVEN_CPU_PORT_INDEX_CNS))
            {
                sliceData = D2D_EAGLE_CPU_PORT_INDEX_CNS;
            }
            if (sliceData <= HWS_D2D_PORTS_NUM_CNS )
            {
                removedSlicesDir[sliceData]++;
            }
            else
            {
                return GT_BAD_PARAM;
            }
        }
        currSliceIndex++;
        if (currOffsetIndex == 3)
        {
            currOffsetIndex = 0;
            currRegisterIndex++;
        }
        else
        {
            currOffsetIndex++;
        }
    }while (currSliceIndex < D2D_PCS_CAL_LEN_CNS);

    return GT_OK;
}

/**
* @internal mvHwsD2dGetChannelsConfigurations function
* @endinternal
*
* @brief  count the slices each channel holds in pcs or mac
*         calendars.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*
* @param[out] channelSlicesArr           - pointer to matrix of
*       counters. the first row contains mac calendar channels
*       slices counters . the second row contains pcs calendars
*       channels slices counters .in each row the channel is the
*       cell index. the cell holds the number of the channel
*       slices
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dGetChannelsConfigurations
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channelSlicesArr[HWS_D2D_PORTS_NUM_CNS+1][2]
)
{
    GT_STATUS    rc;
    GT_U32       d2dIndex, baseAddr;
    GT_U32       unitIndex, unitNum;
    GT_U32       ii;
    GT_U32       regData, sliceMask, sliceMaskValid, mask, regAddr, sliceData, validBit;
    GT_U32       currSliceIndex = 0;
    GT_U32       currOffsetIndex = 0;
    GT_U32       currRegisterIndex = 0;
    GT_U32       pcsIndex = 0, macIndex = 1;
    GT_U32       removedSlicesPcsRx[HWS_D2D_PORTS_NUM_CNS+1]; /* array for rx pcs channels slices */
    GT_U32       removedSlicesPcsTx[HWS_D2D_PORTS_NUM_CNS+1]; /* array for tx pcs channels slices */
    GT_U32       cpuPort;

    hwsOsMemSetFuncPtr(&removedSlicesPcsRx,0,((1/*cpu port*/+HWS_D2D_PORTS_NUM_CNS)*sizeof(GT_U32)));
    hwsOsMemSetFuncPtr(&removedSlicesPcsTx,0,((1/*cpu port*/+HWS_D2D_PORTS_NUM_CNS)*sizeof(GT_U32)));

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
        cpuPort = D2D_EAGLE_CPU_PORT_INDEX_CNS;
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
        cpuPort = D2D_RAVEN_CPU_PORT_INDEX_CNS;
    }

    /* get mac calendar configuration: number of slices for each channel */
    do {
        regAddr = baseAddr + D2D_MAC_CFG_RX_TDM_SCHEDULE + (currRegisterIndex * HWS_D2D_MAC_CHANNEL_STEP);
        sliceMaskValid = 0x80<<(currOffsetIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS);
        sliceMask = 0x3F<<(currOffsetIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS);
        mask = (sliceMask | sliceMaskValid );
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, mask));
        validBit = (regData & sliceMaskValid);
        sliceData = (regData & sliceMask) >> (currOffsetIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS);
        if (validBit)
        {
            if ((sliceData == cpuPort)&&(!PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum)))
            {
                sliceData = D2D_EAGLE_CPU_PORT_INDEX_CNS;
            }
            if (sliceData > HWS_D2D_PORTS_NUM_CNS)
            {
                return GT_BAD_PARAM;
            }
            else
            {
                channelSlicesArr[sliceData][macIndex]++;
            }
        }
        currSliceIndex++;
        if (currOffsetIndex == 3)
        {
            currOffsetIndex = 0;
            currRegisterIndex++;
        }
        else
        {
            currOffsetIndex++;
        }

    }while (currSliceIndex < D2D_MAC_CAL_SLICES_NUM_CNS);

    /* get pcs Rx and pcs Tx configurations:  number of slices for each channel*/
    rc =  mvHwsD2dPcsCalDirConfigurationGet(devNum,baseAddr,D2D_PCS_PCS_RX_CAL_BASE,cpuPort,removedSlicesPcsRx);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc =  mvHwsD2dPcsCalDirConfigurationGet(devNum,baseAddr,D2D_PCS_PCS_TX_CAL_BASE,cpuPort,removedSlicesPcsTx);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check that the pcs calendar configurations are the same for tx and rx calendars*/
    for (ii = 0 ; ii <= HWS_D2D_PORTS_NUM_CNS; ii++)
    {
        if (removedSlicesPcsRx[ii] != removedSlicesPcsTx[ii])
        {
            return GT_FAIL;
        }
        else
        {
            channelSlicesArr[ii][pcsIndex] = removedSlicesPcsTx[ii];
        }
    }
    return GT_OK;
}
/**
* @internal mvHwsD2dDisableChannel function
* @endinternal
*
* @brief  Disable Port , D2D TX and D2D RX.The configuration
*         performs Disable of Tx and Rx on for specific channel
*         for PCS calendar , MAC TDM RX calendar, credits reset.
*         this function should be called twice, from eagle side
*         and from raven side.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dDisableChannel
(
    GT_U8           devNum,
    GT_U32          d2dNum,
    GT_U32          channel
)
{
    /* power down
       1. Disable TX and RX channel ( disable TX, RX channel credits)
       2. Remove channel pcs calendar slices for TX pcs calendar and for RX pcs calendar
       3. Remove channel mac TDM calendar slices

       the function checks:
       1. the pcs calendar RX, TX are in the same size
       2. that the pcs calendar RX and TX stay equal
       3. that the right number of slices was remove.
    */

    GT_STATUS    rc;
    GT_U32       d2dIndex;
    GT_U32       registerIndex, fieldIndex;
    GT_U32       unitIndex, unitNum;
    GT_U32       regAddr, regData, data, macSliceInfo;
    GT_U32       regMask, regMaskValid, baseAddr;
    GT_U32       macCalendarRegistersNum;
    GT_U32       pcsCalendarRegistersNum;
    GT_U32       pcsCalSliceNum;
    GT_U32       pcsCalSliceCounter;      /* count the number of slices that the port holds before the power down*/
    GT_U32       macCalendarSliceCounter; /* count the number of slices that the port holds before the power down*/

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    /*Disable TX Channel*/
    regData = 0<<31;
    regMask = 0x80000000;
    regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_CHANNEL;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    /* if emulator not support the address ... just stop the function (return GT_OK)*/
    /* support mode in which , we bypass ALL Raven access ,
       in such case the 'read' will return value '0' and not 'expectedData' */
    HWS_RETURN_GT_OK_IF_EMULTOR_NOT_SUPPORT_ADDR_MAC(devNum,regAddr);

    /* Get pcs calendar size for TX and RX */
    regMask = 0x1FF;
    regAddr = baseAddr + D2D_PCS_PCS_TX_CAL_CTRL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
    pcsCalSliceNum = regData;
    regAddr = baseAddr + D2D_PCS_PCS_RX_CAL_CTRL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
    if((pcsCalSliceNum != regData) || (pcsCalSliceNum != (D2D_PCS_CAL_LEN_CNS -1)))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("PCS CAL TX length and PCS CALENDAR RX length should be equal"));
    }

    pcsCalSliceNum = D2D_PCS_CAL_LEN_CNS;

    pcsCalendarRegistersNum = ((pcsCalSliceNum/D2D_PCS_CAL_SLICES_IN_REGISTER_NUM_CNS) + (pcsCalSliceNum%D2D_PCS_CAL_SLICES_IN_REGISTER_NUM_CNS));

    /*Disable RX PCS calendar*/
    pcsCalSliceCounter = 0;
    rc = mvHwsD2dDisableChannelPcsCalDir(devNum, d2dNum, channel, D2D_PCS_PCS_RX_CAL_BASE, pcsCalendarRegistersNum,&pcsCalSliceCounter);
    if (rc != GT_OK)
    {
        return rc;
    }


    /*Disable TX PCS calendar*/
    pcsCalSliceCounter = 0;
    rc = mvHwsD2dDisableChannelPcsCalDir(devNum, d2dNum, channel, D2D_PCS_PCS_TX_CAL_BASE, pcsCalendarRegistersNum,&pcsCalSliceCounter);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*Disable RX Channel*/
    regData = 0<<31;
    regMask = 0x80000000;
    regAddr = baseAddr + channel * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    /* Disable RX TDM Schedule */
    macCalendarSliceCounter = 0;
    /* go over calendar slices*/
    macCalendarRegistersNum = D2D_MAC_CAL_SLICES_NUM_CNS/D2D_MAC_CAL_SLICES_IN_REGISTER_NUM_CNS; /* 160/4 = 40*/
    for (registerIndex = 0 ; registerIndex < macCalendarRegistersNum ; registerIndex ++ )
    {
        regAddr = baseAddr + D2D_MAC_CFG_RX_TDM_SCHEDULE + (registerIndex * HWS_D2D_MAC_CHANNEL_STEP);
        for (fieldIndex = 0 ; fieldIndex < D2D_MAC_CAL_SLICES_IN_REGISTER_NUM_CNS ; fieldIndex ++)
        {
            regMaskValid = 0x80<<(fieldIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS);
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMaskValid));
            if (regData == regMaskValid)
            {
                regMask = 0x3F<<(fieldIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS);
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
                macSliceInfo = channel << (fieldIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS);
                if (regData == macSliceInfo)
                {
                    /* this slice belong to the channel and need to be invalid */
                    /* set valid bit to 0 and set field value to 63*/
                    regMask = (regMask | regMaskValid);
                    data = ((0 << (((fieldIndex+1) * (D2D_MAC_CAL_SLICE_OFFSET_CNS)) -1))|(63 << (fieldIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS)));
                    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, regMask));
                    macCalendarSliceCounter++;
                 }
            }
         }
    }

    return GT_OK;
}


/**
* @internal mvHwsD2dDisableAll50GChannel function
* @endinternal
*
* @brief  disable all 50G channels 0 ..7 for specific d2d. this
*         function should be called twice, from eagle side and
*         from raven side.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dDisableAll50GChannel
(
    GT_U8   devNum,
    GT_U32  d2dNum
)
{
   /* The function checks:
       1. the pcs calendar RX, TX are in the same size
       2. that the pcs calendar RX and TX stay equal
       3. that the right number of slices was remove in pcs and mac calendars.
    */

    GT_STATUS    rc;
    GT_U32       d2dIndex;
    GT_U32       channelIndex;
    GT_U32       registerIndex, fieldIndex;
    GT_U32       unitIndex, unitNum;
    GT_U32       regAddr, regData, regDataValid;
    GT_U32       regMask, regMaskValid, baseAddr, sliceValid, sliceData;
    GT_U32       pcsCalSliceNum;
    GT_U32       macCalendarRegistersNum;                           /* mac calendar registers number*/
    GT_U32       pcsCalendarRegistersNum;                           /* pcs calendar registers number*/
    GT_U32       channelsRemovedSliceCounterArr[HWS_D2D_PORTS_NUM_CNS]; /* count the number of slices that the port holds before the power down*/

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    /*Disable TX Channels*/
    regData = 0<<31;
    regMask = 0x80000000;
    for (channelIndex = 0 ; channelIndex < HWS_D2D_PORTS_NUM_CNS; channelIndex++)
    {
        regAddr = baseAddr + channelIndex * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_CHANNEL;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    /* Get pcs calendar size for TX and RX */
    regMask = 0x1FF;
    regAddr = baseAddr + D2D_PCS_PCS_TX_CAL_CTRL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
    pcsCalSliceNum = regData;
    regAddr = baseAddr + D2D_PCS_PCS_RX_CAL_CTRL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
    if(pcsCalSliceNum != regData)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("PCS CALENDAR TX length and PCS CALENDAR RX length should be equal"));
    }
    pcsCalSliceNum = D2D_PCS_CAL_LEN_CNS;

    pcsCalendarRegistersNum = ((pcsCalSliceNum/D2D_PCS_CAL_SLICES_IN_REGISTER_NUM_CNS) + (pcsCalSliceNum%D2D_PCS_CAL_SLICES_IN_REGISTER_NUM_CNS));

    hwsOsMemSetFuncPtr(&channelsRemovedSliceCounterArr, 0, (HWS_D2D_PORTS_NUM_CNS * sizeof(GT_U32)));

    /*Disable RX PCS calendar*/
    rc = mvHwsD2dDisableAll50GChannelsPcsCalDir(devNum,d2dNum, D2D_PCS_PCS_RX_CAL_BASE, pcsCalendarRegistersNum,channelsRemovedSliceCounterArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*Disable TX PCS calendar*/
    hwsOsMemSetFuncPtr(&channelsRemovedSliceCounterArr, 0, (HWS_D2D_PORTS_NUM_CNS * sizeof(GT_U32)));
    rc = mvHwsD2dDisableAll50GChannelsPcsCalDir(devNum,d2dNum,D2D_PCS_PCS_TX_CAL_BASE,pcsCalendarRegistersNum, channelsRemovedSliceCounterArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*Disable RX Channel*/
    regData = 0<<31;
    regMask = 0x80000000;
    for (channelIndex = 0 ; channelIndex < HWS_D2D_PORTS_NUM_CNS; channelIndex++)
    {
        regAddr = baseAddr + channelIndex * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    /* Disable MAC RX TDM Schedule */

    regMaskValid = (0x80 | (0x80 << D2D_MAC_CAL_SLICE_OFFSET_CNS) | (0x80 << (2* D2D_MAC_CAL_SLICE_OFFSET_CNS)) | (0x80 << (3* D2D_MAC_CAL_SLICE_OFFSET_CNS)));
    regMask = (0x3F | (0x3F << D2D_MAC_CAL_SLICE_OFFSET_CNS) | (0x3F << (2*D2D_MAC_CAL_SLICE_OFFSET_CNS)) | (0x3F << (3*D2D_MAC_CAL_SLICE_OFFSET_CNS)));
    hwsOsMemSetFuncPtr(&channelsRemovedSliceCounterArr, 0, (HWS_D2D_PORTS_NUM_CNS * sizeof(GT_U32)));
    macCalendarRegistersNum = D2D_MAC_CAL_SLICES_NUM_CNS/D2D_MAC_CAL_SLICES_IN_REGISTER_NUM_CNS; /* 160/4 = 40*/

    for (registerIndex = 0 ; registerIndex < macCalendarRegistersNum ; registerIndex ++ )
    {
        regAddr = baseAddr + D2D_MAC_CFG_RX_TDM_SCHEDULE + (registerIndex * HWS_D2D_MAC_CHANNEL_STEP);

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xffffffff));
        regDataValid = (regMaskValid & regData); /* valid slices values*/
        regData &= regMask; /* slices data values */
        for (fieldIndex = 0 ; fieldIndex < D2D_MAC_CAL_SLICES_IN_REGISTER_NUM_CNS ; fieldIndex ++)
        {
            sliceValid = (regDataValid & 0x80);
            sliceData = (regData & 0x3F);
            if (sliceValid == 0x80)
            {
                if (sliceData < HWS_D2D_PORTS_NUM_CNS)
                {
                    channelsRemovedSliceCounterArr[sliceData]++;
                }
                else
                if (((registerIndex*4) + fieldIndex) % 17 != 16) /* not cpu slice index*/
                {
                    return GT_BAD_PARAM;
                }
            }
            regData = (regData >> D2D_MAC_CAL_SLICE_OFFSET_CNS);
            regDataValid = (regDataValid >> D2D_MAC_CAL_SLICE_OFFSET_CNS);
         }
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regMask /* 63 is value of idle slice*/, 0xFFFFFFFF));
     }

    return GT_OK;
}


/**
* @internal mvHwsD2dConfigAll50GChannel function
* @endinternal
*
* @brief  ENABLE all channels 0 ..7 to 50G. this function should
*         be called twice, from eagle side and from raven side.
*
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dConfigAll50GChannel
(
    IN  GT_U8   devNum,
    IN  GT_U32  d2dNum
)
{
    GT_STATUS rc;
    GT_U32   channelIndex;

    for (channelIndex = 0 ; channelIndex < HWS_D2D_PORTS_NUM_CNS; channelIndex++)
    {
        rc = mvHwsD2dConfigChannel(devNum, d2dNum, channelIndex, 24, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
        mvHwsD2dPcsCalMacCalAndMacCreditsDump(devNum,d2dNum);
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dMacCreditsDump function
* @endinternal
*
* @brief  print tx and rx mac credits . The configuration
*         performs print of tx channels credits, tx segments, rx
*         channels credits and rx segment. the function is for
*         eagle or raven side
*
* @param[in] devNum                   - system device number
* @param[in] baseAddr                 - d2d base address
* @param[in] isEagle                  - 1: is eagle
*                                       0: is raven
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dMacCreditsDump
(
    GT_U8               devNum,
    GT_U32              baseAddr,
    GT_BOOL             isEagle
)
{
    GT_U32 segId, channelId;
    GT_U32 regMask, regData, regAddr, segMask;
    GT_U32 cpuPort;
    /* tx*/
    GT_U32 numTxSegments;
    GT_U32 txInitCreditMask, txInitCredit;
    GT_U32 txFifoPointerMask, txFifoPointer;
    GT_U32 txFifoBandwidthMask, txFifoBandwidth;
    GT_U32 txEnableMask, txEnable;
    /*rx*/
    GT_U32 numRxSegments;
    GT_U32 rxFifoBandwidthMask, rxFifoBandwidth;
    GT_U32 rxFifoPointerMask, rxFifoPointer;
    GT_U32 rxInitCreditMask, rxInitCredit;
    GT_U32 rxXonMask, rxXon;
    GT_U32 rxXoffMask, rxXoff;
    GT_U32 rxXfastMask, rxXfast;
    GT_U32 rxXslowMask, rxXslow;

    segMask = 0x3F;

    /*TX CREDITS */
    txInitCreditMask = 0x3FF;
    txFifoPointerMask = 0x1FC00;
    txFifoBandwidthMask = 0x60000;
    txEnableMask = 1<<31;

    /*Rx*/
    rxInitCreditMask = 0x3FF;
    rxFifoPointerMask = 0x1FC00;
    rxXonMask = 0x3FF;
    rxXoffMask = 0x7FE0000;
    rxXslowMask = 0xFFC00;
    rxXfastMask = 0x3FF00000;

    if (GT_TRUE == isEagle)
    {
        cpuPort = D2D_EAGLE_CPU_PORT_INDEX_CNS;
        numTxSegments = 16;
        numRxSegments = 16;
    }
    else /* Raven */
    {
        cpuPort = D2D_RAVEN_CPU_PORT_INDEX_CNS;
        numTxSegments = 24;
        numRxSegments = 32;
    }

    hwsOsPrintf ("\n\n    MAC TX CREDITS:");
    hwsOsPrintf ("\n                       :  %9s %9s %9s %8s","Tx Init","Tx Fifo"," Tx Fifo  ","Tx   " );
    hwsOsPrintf ("\n       channel credits :  %9s %9s %9s %8s","Credits","Pointer","Bandwidth","Enable " );
    hwsOsPrintf ("\n                       :-------------------------------------------");
    regMask = 0x3FFFFFFF;
    for (channelId = 0; channelId < HWS_D2D_PORTS_NUM_CNS; channelId++)
    {
        /* tx channel*/
        regAddr = baseAddr + channelId * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_CHANNEL;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

        txInitCredit = regData & txInitCreditMask;
        txFifoPointer = (regData & txFifoPointerMask) >> 10;
        txFifoBandwidth = (regData & txFifoBandwidthMask) >> 17;
        txEnable = regData & txEnableMask;

        hwsOsPrintf ("\n       channel  %2d     :  %4d      %4d      %4d      %4d",channelId, txInitCredit, txFifoPointer, txFifoBandwidth, txEnable );
    }
    /* cpu port */
    regAddr = baseAddr + cpuPort * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_CHANNEL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    txInitCredit = regData & txInitCreditMask;
    txFifoPointer = (regData & txFifoPointerMask) >> 10;
    txFifoBandwidth = (regData & txFifoBandwidthMask) >> 17;
    txEnable = regData & txEnableMask;

    hwsOsPrintf ("\n       channel  %2d     :  %4d      %4d      %4d      %4d",cpuPort, txInitCredit, txFifoPointer, txFifoBandwidth, txEnable );

    hwsOsPrintf ("\n\n   Tx Segments:");
    hwsOsPrintf ("\n         Seg Index :");
    for (segId = 0 ; segId < numTxSegments ; segId++)
    {
        hwsOsPrintf (" %2d",segId);
    }
    hwsOsPrintf ("\n                   :");
    for (segId = 0 ; segId < numTxSegments ; segId++)
    {
        hwsOsPrintf ("---");
    }
    hwsOsPrintf ("\n                   : ");
    for (segId = 0 ; segId < numTxSegments; segId++)
    {
        regAddr = baseAddr +  segId* HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_TX_FIFO_SEG_POINTER;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, segMask));
        hwsOsPrintf ("%2d ",regData);
    }

    /*RX CREDITS */
    hwsOsPrintf ("\n\n    MAC RX CREDITS:");
    hwsOsPrintf ("\n                       :  %9s %9s %9s %6s   %6s    %6s    %6s","Rx Init","Rx Fifo","Rx Fifo ","Rx ","Rx ","Rx ","Rx ");
    hwsOsPrintf ("\n       channel credits :  %9s %9s %9s %6s   %6s    %6s    %6s","Credits","Pointer","Bandwidth","Xon","Xoff","Xfast","Xslow");
    hwsOsPrintf ("\n                       :---------------------------------------------------------------------");

    for (channelId = 0; channelId < HWS_D2D_PORTS_NUM_CNS; channelId++)
    {
        /* rx fifo bandwidth */
        rxFifoBandwidthMask = 0x3 << ((channelId%HWS_D2D_PORTS_NUM_CNS) * HWS_D2D_MAC_CHANNEL_STEP);
        regAddr = baseAddr + D2D_MAC_CFG_RX_FIFO_BANDWIDTH_SELECT + ((channelId/HWS_D2D_PORTS_NUM_CNS) * HWS_D2D_MAC_CHANNEL_STEP);
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

        rxFifoBandwidth = (regData & rxFifoBandwidthMask) >> ((channelId%HWS_D2D_PORTS_NUM_CNS) * HWS_D2D_MAC_CHANNEL_STEP);

        /*rx channel 2 */
        regMask = 0x3FFFFFFF;
        regAddr = baseAddr + channelId * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL_2;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

        rxXon = regData & rxXonMask;
        rxXslow = (regData & rxXslowMask) >> 10;
        rxXfast = (regData & rxXfastMask) >> 20;

        /* rx channel*/
        regAddr = baseAddr + channelId * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

        rxInitCredit = regData & rxInitCreditMask;
        rxFifoPointer = (regData & rxFifoPointerMask) >> 10;
        rxXoff = (regData & rxXoffMask) >> 17;

        hwsOsPrintf ("\n       channel  %2d     :  %4d      %4d      %4d      %4d      %4d      %4d      %4d",channelId, rxInitCredit, rxFifoPointer, rxFifoBandwidth, rxXon, rxXoff, rxXfast, rxXslow );
    }
    /* cpu port */
    channelId = cpuPort;
    /* rx fifo bandwidth */
    rxFifoBandwidthMask = 0x3 << ((channelId%HWS_D2D_PORTS_NUM_CNS) * HWS_D2D_MAC_CHANNEL_STEP);
    regAddr = baseAddr + D2D_MAC_CFG_RX_FIFO_BANDWIDTH_SELECT + ((channelId/HWS_D2D_PORTS_NUM_CNS) * HWS_D2D_MAC_CHANNEL_STEP);
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    rxFifoBandwidth = regData & rxFifoBandwidthMask;

    /*rx channel 2 */
    regMask = 0x3FFFFFFF;
    regAddr = baseAddr + channelId * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL_2;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    rxXon = regData & rxXonMask;
    rxXslow = (regData & rxXslowMask) >> 10;
    rxXfast = (regData & rxXfastMask) >> 20;

    /* rx channel*/
    regAddr = baseAddr + channelId * HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_CHANNEL;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    rxInitCredit = regData & rxInitCreditMask;
    rxFifoPointer = (regData & rxFifoPointerMask) >> 10;
    rxXoff = (regData & rxXoffMask) >> 17;

    hwsOsPrintf ("\n       channel  %2d     :  %4d      %4d      %4d      %4d      %4d      %4d      %4d",channelId, rxInitCredit, rxFifoPointer, rxFifoBandwidth, rxXon, rxXoff, rxXfast, rxXslow );
    hwsOsPrintf ("\n\n   Rx Segments:");
    hwsOsPrintf ("\n         Seg Index :");
    for (segId = 0 ; segId < numRxSegments ; segId++)
    {
        hwsOsPrintf (" %2d",segId);
    }
    hwsOsPrintf ("\n                   :");
    for (segId = 0 ; segId < numRxSegments ; segId++)
    {
        hwsOsPrintf ("---");
    }
    hwsOsPrintf ("\n                   : ");
    for (segId = 0 ; segId < numRxSegments; segId++)
    {
        regAddr = baseAddr +  segId* HWS_D2D_MAC_CHANNEL_STEP + D2D_MAC_CFG_RX_FIFO_SEG_POINTER;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, segMask));
        hwsOsPrintf ("%2d ",regData);
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dMacCalDump function
* @endinternal
*
* @brief  print rx tdm mac calendar slices. the function is for
*         eagle or raven side
*
* @param[in] devNum                   - system device number
* @param[in] baseAddr                 - d2d base address
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dMacCalDump
(
    GT_U8               devNum,
    GT_U32              baseAddr
)
{
    GT_U32 ii, sliceId;
    GT_U32 regData, sliceMask, sliceMaskValid, mask, regAddr, sliceData, validBit;
    GT_U32 currSliceIndex = 0;
    GT_U32 currOffsetIndex = 0;
    GT_U32 currRegisterIndex = 0;
    GT_U32 startIndex, endIndex;
    hwsOsPrintf ("\n\n    TDM MAC RX CALENDAR:");
    hwsOsPrintf ("\n       Slice to port map   : slice :");
    for (sliceId = 0 ; sliceId < (D2D_LANES_NUM_CNS -1) ; sliceId++)
    {
        hwsOsPrintf (" %2d",sliceId);
    }
    hwsOsPrintf ("\n                                   :");
    for (sliceId = 0 ; sliceId < (D2D_LANES_NUM_CNS -1) ; sliceId++)
    {
        hwsOsPrintf ("---");
    }

    for (ii = 0 ; ii < (D2D_MAC_CAL_SLICES_NUM_CNS/(D2D_LANES_NUM_CNS - 1)); ii ++)
    {
        startIndex = ((D2D_LANES_NUM_CNS -1)*ii);
        endIndex = ((D2D_LANES_NUM_CNS -1)*ii + (D2D_LANES_NUM_CNS - 2));
        hwsOsPrintf ("\n       Slice %4d : %4d   : port  :",startIndex, endIndex);
        do {
            regAddr = baseAddr + D2D_MAC_CFG_RX_TDM_SCHEDULE + (currRegisterIndex * HWS_D2D_MAC_CHANNEL_STEP);
            sliceMaskValid = 0x80<<(currOffsetIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS);
            sliceMask = 0x3F<<(currOffsetIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS);
            mask = (sliceMask | sliceMaskValid );
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, mask));
            validBit = (regData & sliceMaskValid);
            sliceData = (regData & sliceMask) >> (currOffsetIndex * D2D_MAC_CAL_SLICE_OFFSET_CNS);
            if (!validBit)
            {
                hwsOsPrintf (" %2s","x");
            }
            else
            {
                hwsOsPrintf (" %2d", sliceData);
            }

            currSliceIndex++;
            if (currOffsetIndex == 3)
            {
                currOffsetIndex = 0;
                currRegisterIndex++;
            }
            else
            {
                currOffsetIndex++;
            }
        }while (currSliceIndex <= endIndex);
    }
    return GT_OK;

}


/**
* @internal mvHwsD2dPcsCalDirDump function
* @endinternal
*
* @brief  print rx or tx pcs calendar slices. the function is
*         for eagle or raven side, for tx or rx calendar (
*         called 4 times)
*
* @param[in] devNum                   - system device number
* @param[in] baseAddr                 - d2d base address
* @param[in] dirUnitAddr              - address of first pcs tx
*       or rx calendar register
* @param[in] portsBandwidthArr        - array to count the ports
*       slices
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dPcsCalDirDump
(
    GT_U8               devNum,
    GT_U32              baseAddr,
    GT_U32              dirUnitAddr,
    GT_U32              *portsBandwidthArr
)
{
    GT_U32 ii;
    GT_U32 regData, sliceMask, regAddr,sliceData;
    GT_U32 currSliceIndex = 0;
    GT_U32 currOffsetIndex = 0;
    GT_U32 currRegisterIndex = 0;
    GT_U32 startIndex, endIndex;

    for (ii = 0 ; ii < (D2D_PCS_CAL_LEN_CNS/D2D_LANES_NUM_CNS); ii ++)
    {
        startIndex = (D2D_LANES_NUM_CNS*ii);
        endIndex = (D2D_LANES_NUM_CNS*ii + (D2D_LANES_NUM_CNS -1));
        hwsOsPrintf ("\n       Slice %4d : %4d   : port  :",startIndex, endIndex);
        do {
            sliceMask = 0x3F<< (currOffsetIndex *D2D_PCS_CAL_SLICE_OFFSET_CNS);
            regAddr = baseAddr + dirUnitAddr + (currRegisterIndex * HWS_D2D_MAC_CHANNEL_STEP);
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, sliceMask));
            sliceData = regData >>(currOffsetIndex * D2D_PCS_CAL_SLICE_OFFSET_CNS);

            if (sliceData == D2D_PCS_NON_RSVD_CNS)
            {
                hwsOsPrintf (" %2s","x");
            }
            else
            {
                hwsOsPrintf (" %2d", sliceData);
                if (sliceData <= HWS_D2D_PORTS_NUM_CNS)
                {
                    portsBandwidthArr[sliceData]++;
                }
            }

            currSliceIndex++;
            if (currOffsetIndex == 3)
            {
                currOffsetIndex = 0;
                currRegisterIndex++;
            }
            else
            {
                currOffsetIndex++;
            }
        }while (currSliceIndex <= endIndex);
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dPcsCalDump function
* @endinternal
*
* @brief  print rx and tx pcs calendar slices. the function is
*         for eagle or raven side
*
* @param[in] devNum                   - system device number
* @param[in] baseAddr                 - d2d base address
* @param[in] portsBandwidthArr        -array to count the ports
*       slices
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dPcsCalDump
(
    GT_U8               devNum,
    GT_U32              baseAddr,
    GT_U32              *portsBandwidthArr
)
{
    GT_STATUS rc;
    GT_U32 sliceId;

    hwsOsPrintf ("\n\n    PCS RX CALENDAR:");
    hwsOsPrintf ("\n       Slice to port map   : slice :");
    for (sliceId = 0 ; sliceId < D2D_LANES_NUM_CNS ; sliceId++)
    {
        hwsOsPrintf (" %2d",sliceId);
    }
    hwsOsPrintf ("\n                                   :");
    for (sliceId = 0 ; sliceId < D2D_LANES_NUM_CNS ; sliceId++)
    {
        hwsOsPrintf ("---");
    }

    rc = mvHwsD2dPcsCalDirDump(devNum,baseAddr,D2D_PCS_PCS_RX_CAL_BASE,portsBandwidthArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    hwsOsPrintf ("\n\n    PCS TX CALENDAR:");
    hwsOsPrintf ("\n       Slice to port map   : slice :");
    for (sliceId = 0 ; sliceId < D2D_LANES_NUM_CNS ; sliceId++)
    {
        hwsOsPrintf (" %2d",sliceId);
    }
    hwsOsPrintf ("\n                                   :");
    for (sliceId = 0 ; sliceId < D2D_LANES_NUM_CNS ; sliceId++)
    {
        hwsOsPrintf ("---");
    }

    rc = mvHwsD2dPcsCalDirDump(devNum,baseAddr,D2D_PCS_PCS_TX_CAL_BASE,portsBandwidthArr);

    return rc;
}


/**
* @internal mvHwsD2dPcsCalMacCalAndMacCreditsPerD2dIndexDump
*           function
* @endinternal
*
* @brief  print rx and tx pcs and mac calendars slices and mac
*         credits. the function is for eagle or raven side
*
* @param[in] devNum                   - system device number
* @param[in] baseAddr                 - d2d base address
* @param[in] isEagle                  - 1: is eagle
*                                       0: is raven
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dPcsCalMacCalAndMacCreditsPerD2dIndexDump
(
    GT_U8               devNum,
    GT_U32              baseAddr,
    GT_BOOL             isEagle
)
{
     GT_STATUS rc;
     GT_U32 portsBandwidthArr[HWS_D2D_PORTS_NUM_CNS];
     GT_U32 ii, portBandwidth;
     isEagle = isEagle;

     hwsOsMemSetFuncPtr(&portsBandwidthArr, 0, (HWS_D2D_PORTS_NUM_CNS * sizeof(GT_U32)));

     rc =  mvHwsD2dPcsCalDump(devNum,baseAddr,portsBandwidthArr);
     if (rc != GT_OK)
     {
         return rc;
     }

     rc =  mvHwsD2dMacCalDump(devNum,baseAddr);
     if (rc != GT_OK)
     {
         return rc;
     }

     rc =  mvHwsD2dMacCreditsDump(devNum,baseAddr,isEagle);

     hwsOsPrintf("\n   Ports Banwidth:\n");
     hwsOsPrintf("       port index  :");
     for (ii = 0 ; ii < HWS_D2D_PORTS_NUM_CNS; ii++ )
     {
         hwsOsPrintf("%4d",ii);
     }
     hwsOsPrintf("\n");
     hwsOsPrintf("                   :---------------------------------\n");
     hwsOsPrintf("       port BW     :");
     for (ii = 0 ; ii < HWS_D2D_PORTS_NUM_CNS; ii++ )
     {
         portBandwidth = ((portsBandwidthArr[ii]/2)*5);
         hwsOsPrintf("%4d",portBandwidth);
     }
     hwsOsPrintf("\n");
     return rc;
}



/**
* @internal mvHwsD2dPcsCalMacCalAndMacCreditsPerD2dDump function
* @endinternal
*
* @brief  print rx and tx pcs and mac calendars slices and mac
*         credits. for both eagle and raven
*
* @param[in] devNum                   - system device number
* @param[in] d2dIndex                 - d2d index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsD2dPcsCalMacCalAndMacCreditsPerD2dDump
(
    GT_U8               devNum,
    GT_U32              d2dIndex
)
{

    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum;
    /* eagle */
    hwsOsPrintf ("\n\n\nD2D NUMBER %d: Eagle side",d2dIndex);
    CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));

    rc = mvHwsD2dPcsCalMacCalAndMacCreditsPerD2dIndexDump(devNum,baseAddr,GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }
    hwsOsPrintf ("\n");

    /* raven */
    hwsOsPrintf ("\n\nD2D NUMBER %d: RAVEN side",d2dIndex);
    CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    rc = mvHwsD2dPcsCalMacCalAndMacCreditsPerD2dIndexDump(devNum,baseAddr,GT_FALSE);

    return rc;
}


/**
* @internal mvHwsD2dPcsCalMacCalAndMacCreditsDump function
* @endinternal
*
* @brief  print rx and tx pcs and mac calendars slices and mac
*         credits. for both eagle and raven. the function print
*         all d2d registers values or specific d2d registers
*         values.
*
* @param[in] devNum                   - system device number
* @param[in] d2dIndex                 - d2d index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dPcsCalMacCalAndMacCreditsDump
(
    GT_U8                       devNum,
    GT_U32                      d2dIdx
)
{

/* go over all d2d , for each eagle <-> raven interface there are 2 d2d in each side.
       d2d in raven side:
        2 PCS calendars:
            Rx PCS calendar
            Tx PCS calendar
        1 MAC TDM Rx calendar
        MAC Rx credits and Mac Tx credits

       d2d on eagle side:
        2 PCS calendars:
            Rx PCS calendar
            Tx PCS calendar
        1 MAC TDM Rx calendars
         MAC Rx credits and Mac Tx credits
*/

    GT_STATUS rc;
    GT_U32 d2dIndx;

    if (d2dIdx != HWS_D2D_ALL)
    {
        rc = mvHwsD2dPcsCalMacCalAndMacCreditsPerD2dDump(devNum,d2dIdx);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        for (d2dIndx = 0; d2dIndx < (hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS); d2dIndx++)
        {
            rc = mvHwsD2dPcsCalMacCalAndMacCreditsPerD2dDump(devNum,d2dIndx);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

