/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <appDemo/boardConfig/gtDbDxBobcat2UtilsSimClock.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>

#ifdef ASIC_SIMULATION
    #include <asicSimulation/SCIB/scib.h>
#endif


/*-------------------------------------------------------------------------*
 *  BC2 / BOBK sim core slock via SAR(samle and reset) PLL0 settings       *
 *-------------------------------------------------------------------------*/
#define BAD_VALUE (GT_U32)~0

const static CORECOLOCK_PLL0_VALUE_STC bc2_coreClockPllArr[] =
{      /* clock ,  SAR2 PLL0 value */
     {        362,           0 }
    ,{        221,           1 }
    ,{        250,           2 }
    ,{        400,           3 }
    ,{        175,           4 }
    ,{        521,           5 }
    ,{        450,           6 }
    ,{  BAD_VALUE,   BAD_VALUE }
};

const static CORECOLOCK_PLL0_VALUE_STC bobk_coreClockPllArr[] =
{      /* clock ,  SAR2 PLL0 value */
     {        365,           0 }              /* 0 = 365.625 MHz           */
 /* ,{        220,           1 } */           /* 1 = 220.833 MHz Reserved  */
    ,{        250,           2 }              /* 2 = 250.000 MHz           */
    ,{        200,           3 }              /* 3 = 200.000 MHz           */
    ,{        167,           4 }              /* 4 = 167.188 MHz           */
 /* ,{        133,           5 } */           /* 5 = 133.333 Mhz Reserved  */
 /* ,{        225,           6 } */           /* 6 = 225.000 Mhz Reserved  */
    ,{  BAD_VALUE,   BAD_VALUE }
};

const static CORECOLOCK_PLL0_VALUE_STC aldrin_coreClockPllArr[] =
{      /* clock ,  SAR2 PLL0 value */
     {        365,           0 }              /* 0 = 365.625 MHz           */
 /* ,{        220,           1 } */           /* 1 = 220.833 MHz Reserved  */
    ,{        250,           2 }              /* 2 = 250.000 MHz           */
    ,{        200,           3 }              /* 3 = 200.000 MHz           */
 /* ,{        167,           4 } */           /* 4 = 167.188 MHz           */
 /* ,{        133,           5 } */           /* 5 = 133.333 Mhz Reserved  */
 /* ,{        225,           6 } */           /* 6 = 225.000 Mhz Reserved  */
    ,{  BAD_VALUE,   BAD_VALUE }
};

const static CORECOLOCK_PLL0_VALUE_STC bc3_coreClockPllArr[] =
{      /* clock ,  SAR2 PLL0 value */
     {       450,            0   }     /*   0 = 450        Reserved     */
    ,{       487,            1   }     /*   1 = 487.5      Reserved     */
    ,{       525,            2   }     /*   2 = 525                     */
    ,{       583,            3   }     /*   3 = 583.334    Reserved     */
    ,{       600,            4   }     /*   4 = 600                     */
    ,{       625,            5   }     /*   5 = 625        Reserved     */
    ,{       556,            6   }     /*   6 = 556.250    Reserved     */
    ,{       250,            7   }     /*   7 = 250        Reserved     */
    ,{       572,            8   }     /*   8 = 572.917                 */
    ,{       577,            9   }     /*   9 = 577.083    Reserved     */
    ,{       585,           10   }     /*  10 = 585.417    Reserved     */
    ,{       400,           11   }     /*  11 = 400        Reserved     */
    ,{       535,           12   }     /*  12 = 535.416    Reserved     */
    ,{       545,           13   }     /*  13 = 545.8335   Reserved     */
    ,{       593,           14   }     /*  14 = 593.75     Reserved     */
    ,{  BAD_VALUE,     BAD_VALUE }
};



GT_STATUS appDemoBcatBobKSimCoreClockSet
(
    IN GT_U8  devNum,
    IN GT_U32 coreClock,
    IN const CORECOLOCK_PLL0_VALUE_STC *coreClockPllArr
)
{
    #ifdef ASIC_SIMULATION

        #define BC2_DFX_SAR2_ADDR_CNS  0x000F8204
        #define BC2_DFX_SAR2_ADDR_PLL0_OFFS_CNS 21
        #define BC2_DFX_SAR2_ADDR_PLL0_LEN_CNS   3


        GT_U32 i;

        if (devNum == 255)
        {
            cpssOsPrintf("\n+------------+----------------+");
            cpssOsPrintf("\n| Core Clock | SAR PLL0 value |");
            cpssOsPrintf("\n+------------+----------------+");
            for (i = 0 ; coreClockPllArr[i].coreClock != BAD_VALUE; i++)
            {
                cpssOsPrintf("\n| %10d | %10d     |",coreClockPllArr[i].coreClock,
                                                coreClockPllArr[i].pll0Value);
            }
            cpssOsPrintf("\n+------------+----------------+");
            cpssOsPrintf("\n");
            return GT_OK;
        }

        /* simulation will fatal error if the core clock is not found ! */
        scibCoreClockRegisterUpdate(devNum/*simulation devId*/,coreClock,0/*ignored*/);

        return GT_OK;
    #else
        #define UNUSED_PARAM_MAC(x) x = x
        UNUSED_PARAM_MAC(devNum);
        UNUSED_PARAM_MAC(coreClock);
        return GT_NOT_SUPPORTED;
    #endif
}



GT_STATUS appDemoBcat2B0SimCoreClockSet
(
    IN GT_U8  devNum,
    IN GT_U32 coreClock
)
{
    return appDemoBcatBobKSimCoreClockSet(devNum,coreClock,&bc2_coreClockPllArr[0]);
}


GT_STATUS appDemoBobKSimCoreClockSet
(
    IN GT_U8  devNum,
    IN GT_U32 coreClock
)
{
    return appDemoBcatBobKSimCoreClockSet(devNum,coreClock,&bobk_coreClockPllArr[0]);
}


GT_STATUS appDemoAldrinSimCoreClockSet
(
    IN GT_U8  devNum,
    IN GT_U32 coreClock
)
{
    return appDemoBcatBobKSimCoreClockSet(devNum,coreClock,&aldrin_coreClockPllArr[0]);
}

GT_STATUS appDemoBc3SimCoreClockSet
(
    IN GT_U8  devNum,
    IN GT_U32 coreClock
)
{
    return appDemoBcatBobKSimCoreClockSet(devNum,coreClock,&bc3_coreClockPllArr[0]);
}



