#ifndef __appDemoWncUtilsTrtesth
#define __appDemoWncUtilsTrtesth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200112L
#endif

/* For common cpss api include */
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cmdShell/common/cmdExtServices.h>
#include <cpss/common/private/prvCpssGlobalMutex.h>
#include <cpss/generic/version/cpssGenStream.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <extUtils/trafficEngine/tgfTrafficGenerator.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <dirent.h>

/* For task include */
#include <gtOs/gtOsSharedUtil.h>
#include <gtOs/gtOsSharedIPC.h>
#include <gtOs/gtOsSharedData.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsInit.h>

/* currently on-going projects max number of unit and port */
#define MAX_UNIT_CNS    2
#define MAX_PORTS_CNS   64

/* default vlan definition */
#define DEFAULT_VLAN	100

/* loopbacke mode definition */
typedef enum
{
    APPDEMO_WNC_UTILS_TRTEST_LOOPBACK_NONE = 0,	/* None loopback */
    APPDEMO_WNC_UTILS_TRTEST_LOOPBACK_PHYEXT	/* External phy loopback */

}APPDEMO_WNC_UTILS_TRTEST_LOOPBACK_MODE;

/* phy type definition  */
typedef enum
{
    APPDEMO_WNC_UTILS_TRTEST_PHY_NONE = 0,	/* None */
    APPDEMO_WNC_UTILS_TRTEST_PHY_88E1680,	/* 88E1680 */
    APPDEMO_WNC_UTILS_TRTEST_PHY_88E1543,	/* 88E1543 */
    APPDEMO_WNC_UTILS_TRTEST_PHY_88E2010,	/* 88E2010 */
    APPDEMO_WNC_UTILS_TRTEST_PHY_INTPHY		/* INTPHY */

}APPDEMO_WNC_UTILS_TRTEST_PHY_TYPE;

typedef struct{
    GT_U32  numCount;
    GT_U8   devNumArr[MAX_PORTS_CNS];
    GT_U32  portNumArr[MAX_PORTS_CNS];
} WNC_CW_INR_ETH_PORTS_STC;

typedef struct{
    GT_CHAR lport[8];
    GT_U8   devNum;
    GT_U32  portNum;
} WNC_CW_INR_ETH_PORTMAPPING_STC;

typedef struct{
    GT_CHAR lport[8];
    GT_U8   devNum;
    GT_U32  portNum;
	GT_U32  vlan;
	GT_U8   vlanMemberType;
	GT_U8   loopBackMode;
	GT_U32  phyType;
} WNC_CW_INR_ETH_MFG_PORT_INFO_STC;

/* MAC and port counter */
static GT_U32 portmapCount = 0;
static GT_U32 devMacNum = 0;
/* port mapping info */
static WNC_CW_INR_ETH_PORTMAPPING_STC portMapInfo[MAX_PORTS_CNS];
/* MFG mode port info */
static WNC_CW_INR_ETH_MFG_PORT_INFO_STC mfgPortInfo[MAX_PORTS_CNS];

/* port info file path */
#define TF_SYSINFO_CONFIG "/etc/opt/asic/sysinfo"
#define TF_PORTINFO_MFG_CONFIG "/etc/opt/asic/portinfo_mfg"

/* counter wather thread */
static GT_U32 cw_tpps[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{0}};
static GT_U32 cw_rpps[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{0}};
/* initialize the first element in the array */
/* typedef struct                            */
/* {                                         */
/*     GT_U32  l[2];                         */
/* }GT_U64;                                  */
static GT_U64 cw_tpok[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 cw_tpng[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 cw_rpok[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 cw_rpng[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
/* high resolution counter wather thread */
static GT_U32 hr_tpps[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{0}};
static GT_U32 hr_rpps[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{0}};
static GT_U64 hr_tpok[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_tpng[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_rpok[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_rpng[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};

static GT_U64 hr_txbyte[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_rxbyte[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_txerr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_rxerr[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_txdisc[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_rxdisc[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_txpaus[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_rxpaus[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};

/* To record counters port cntr in cntrNameArrRxGood[] and cntrNameArrTxGood[], update bit 32~63 manually */
static GT_U64 hr_tpok_bc[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_tpok_mc[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_tpok_uc[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_rpok_bc[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_rpok_mc[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};
static GT_U64 hr_rpok_uc[MAX_UNIT_CNS][MAX_PORTS_CNS] = {{{{0,0}}}};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoWncUtilsTrtesth */
