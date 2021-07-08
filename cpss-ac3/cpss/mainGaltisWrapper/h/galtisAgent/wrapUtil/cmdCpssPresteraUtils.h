/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* cmdPresteraUtils.h
*
* DESCRIPTION:
*       galtis agent interface to prestera specific functionality
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 22 $
*******************************************************************************/

#ifndef __cmdPresteraUtils_h__
#define __cmdPresteraUtils_h__

/***** Include files ***************************************************/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>

#ifdef  __cplusplus
    extern "C" {
#endif

/* free pointer and set to NULL */
#define CMD_OS_FREE_AND_SET_NULL_MAC(ptr) if(ptr) cmdOsFree(ptr); (ptr) = NULL
/* memset 0 element (according to it's size) */
#define MEMSET_0_ELEMENT_MAC(element) cmdOsMemSet(&element,0,sizeof(element))

/**
* @struct GT_BYTE_ARRY
 *
 * @brief This structure contains byte array data and length.
*/
typedef struct{

    GT_U8 *data;

    /** buffer length */
    GT_U32 length;

} GT_BYTE_ARRY;

/* Wrapper related types for union fields*/
typedef GT_U8 GT_U8_UNION;

typedef GT_U16 GT_U16_UNION;

typedef GT_U32 GT_U32_UNION;

typedef GT_BOOL GT_BOOL_UNION;

/* Get the portGroupsBmp for multi port groups device.
   when 'enabled' --> wrappers will use the APIs with portGroupsBmp parameter*/
#define UTIL_MULTI_PORT_GROUPS_BMP_GET(device,enable,portGroupBmp)  \
    utilMultiPortGroupsBmpGet(device,&enable,&portGroupBmp);

/*
 * Typedef: struct GT_UTIL_STC_ELEMENT_PATH_DSC_STC
 *
 * Description:
 *    This record is a description of the structure element.
 *    The structure represented by array of such records -
 *    record for each structure member
 *    An array terminated by record with the namePtr==NULL.
 *    An application can use the larger record (the structure containing
 *    this as a first member) as derived class. The search function has
 *    the special "description record size" parameter to follow the array.
 *
 * Fields:
 *    namePtr    - pointer to the expose name of a structure element
 *    subtreePtr - pointer to araay of substructure elements descriptors
 *                 NULL if the element is not substructure
 *    offset     - offset of the element from the structure origin
 */
typedef struct _stc_GT_UTIL_STC_ELEMENT_DSC_STC
{
    GT_CHAR                                      *namePtr;
    struct _stc_GT_UTIL_STC_ELEMENT_DSC_STC      *subtreePtr;
    GT_U32                                       offset;
} GT_UTIL_STC_ELEMENT_DSC_STC;

/*
*    The regular descriptor record
*  parameters:
*  _struct_type    - the typedef defined strurcture name
*  _element_name   - the name of structure element used
*                    in "C" language definition
*  _element_expose - the name of element used in "path"
*  _subtree_ptr    - pointer to array that describes the substructure elements
*                    for scalar element NULL
*
*/
#define GT_UTIL_STC_ELEMENT_DSC_MAC(                               \
    _struct_type, _element_name, _element_expose, _subtree_ptr)    \
    {                                                              \
        _element_expose,                                           \
        (struct _stc_GT_UTIL_STC_ELEMENT_DSC_STC*)_subtree_ptr,    \
        ((char*)&(((_struct_type*)0)->_element_name) - (char*)0)   \
    }

/* the end of structure element descriptors list */
#define GT_UTIL_STC_ELEMENT_DSC_END_LIST_CNS                       \
    { (GT_CHAR*)0, (struct _stc_GT_UTIL_STC_ELEMENT_DSC_STC*)0,  0}

/**
* @enum GT_UTIL_HEX_DATA_STORE_FORMAT_ENT
 *
 * @brief Hex data store formats
*/
typedef enum{

    /** in network order */
    GT_UTIL_HEX_DATA_STORE_NETWORK_ORDERED_E,

    /** in order reverse to network. */
    GT_UTIL_HEX_DATA_STORE_REVERSE_ORDERED_E,

    /** @brief each byte added by rule
     *  num = (num << 8) next
     */
    GT_UTIL_HEX_DATA_STORE_CPU_INTEGER_E

} GT_UTIL_HEX_DATA_STORE_FORMAT_ENT;


/**
* @enum GT_UTIL_PP_FAMILY_TYPE_ENT
 *
 * @brief Device families that may exist in system.
*/
typedef enum{

    /** Twist-C family devices */
    GT_UTIL_PP_FAMILY_TWISTC_E  = 1,

    /** Twist-D family devices */
    GT_UTIL_PP_FAMILY_TWISTD_E,

    /** Samba family devices */
    GT_UTIL_PP_FAMILY_SAMBA_E,

    /** Tiger family devices */
    GT_UTIL_PP_FAMILY_TIGER_E,

    /** Salsa family devices */
    GT_UTIL_PP_FAMILY_SALSA_E   = 7,

    /** Cheetah family devices */
    GT_UTIL_PP_FAMILY_CHEETAH_E = 10,

    /** Cheetah 2 family devices */
    GT_UTIL_PP_FAMILY_CHEETAH2_E,

    /** Cheetah 3 family devices */
    GT_UTIL_PP_FAMILY_CHEETAH3_E,

    /** (DXCH) xCat family devices */
    GT_UTIL_PP_FAMILY_DXCH_XCAT_E,

    /** (DXCH) Lion family devices */
    GT_UTIL_PP_FAMILY_DXCH_LION_E,

    /** (DXCH) xCat2 family devices */
    GT_UTIL_PP_FAMILY_DXCH_XCAT2_E,

    /** (DXCH) Lion 2 family devices */
    GT_UTIL_PP_FAMILY_DXCH_LION2_E,

    /** (DXCH) Lion 3 family devices */
    GT_UTIL_PP_FAMILY_DXCH_LION3_E,

    /** (DXCH) Bobcat 2 family devices */
    GT_UTIL_PP_FAMILY_DXCH_BOBCAT2_E,

    /** Puma family devices (Puma 2) */
    GT_UTIL_PP_FAMILY_PUMA_E    = 20,

    /** Puma 3 family devices */
    GT_UTIL_PP_FAMILY_PUMA3_E,

    /** (DXCH) xCat3 family devices */
    GT_UTIL_PP_FAMILY_DXCH_XCAT3_E = 30,

    /** (DXCH) Bobcat 3 family devices */
    GT_UTIL_PP_FAMILY_DXCH_BOBCAT3_E,

    /** (DXCH) Aldrin family devices */
    GT_UTIL_PP_FAMILY_DXCH_ALDRIN_E,

    /** (DXCH) AC3X family devices */
    GT_UTIL_PP_FAMILY_DXCH_AC3X_E,

    /** (DXCH) Aldrin2 family devices */
    GT_UTIL_PP_FAMILY_DXCH_ALDRIN2_E,

    /** (DXCH) Falcon family devices */
    GT_UTIL_PP_FAMILY_DXCH_FALCON_E,

    /** must be last */
    GT_UTIL_PP_FAMILY_DXCH_LAST_E

} GT_UTIL_PP_FAMILY_TYPE_ENT;

/**
* @enum GT_UTIL_PP_SUB_FAMILY_TYPE_ENT
 *
 * @brief Defines the different device sub families that may exist in system.
*/
typedef enum{

    /** @brief the device hold no special 'sub family' and the
     *  identified without 'sub family'
     */
    GT_UTIL_PP_SUB_FAMILY_NONE_E,

    /** @brief this is bobk sub family within the bobcat2 family.
     *  relevant only when 'family' is 'bobcat2'
     */
    GT_UTIL_PP_SUB_FAMILY_BOBCAT2_BOBK_E,

    GT_UTIL_MAX_SUB_FAMILY = 0x7FFFFFFF

} GT_UTIL_PP_SUB_FAMILY_TYPE_ENT;

/**
* @internal galtisMacAddr function
* @endinternal
*
* @brief   Create Ethernet MAC Address from hexadecimal coded string
*         6 elements, string size = 12 bytes
* @param[in] source                   - hexadecimal coded string reference
*
* @param[out] dest                     - pointer to GT_ETHERADDR structure
*                                       None
*
* @note no assertion is performed on validity of coded string
*       Toolkit:
*
*/
GT_VOID galtisMacAddr
(
    OUT GT_ETHERADDR *dest,
    IN  GT_U8        *source
);

/*******************************************************************************
* galtisByteArrayToString
*
* DESCRIPTION:
*       convert byte array to the string
*
* INPUTS:
*       byteArr - pointer to byte array
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the string contans converted data
*
* COMMENTS:
* Toolkit:
*
*******************************************************************************/
char* galtisByteArrayToString
(
    IN GT_U8  *byteArr,
    IN GT_U32 arraySize
);

/**
* @internal galtisIpAddr function
* @endinternal
*
* @brief   Create IPv4 Address from hexadecimal coded string
*         4 elements, string size = 8 bytes
* @param[in] source                   - hexadecimal coded string reference
*
* @param[out] dest                     - pointer to GT_IPADDR structure
*                                       None
*
* @note no assertion is performed on validity of coded string
*       Toolkit:
*
*/
GT_VOID galtisIpAddr
(
    OUT GT_IPADDR *dest,
    IN  GT_U8     *source
);

/**
* @internal galtisIpv6Addr function
* @endinternal
*
* @brief   Create IPv6 Address from hexadecimal coded string of
*         16 elements, string size = 32 bytes
* @param[in] source                   - hexadecimal coded string reference
*
* @param[out] dest                     - pointer to GT_IPV6ADDR structure
*                                       None
*
* @note no assertion is performed on validity of coded string
*
*/
GT_VOID galtisIpv6Addr
(
    OUT GT_IPV6ADDR *dest,
    IN  GT_U8     *source
);

/**
* @internal galtisFcoeAddr function
* @endinternal
*
* @brief   Create FCoE Address from hexadecimal coded string of
*         3 elements, string size = 24 bytes
* @param[in] source                   - hexadecimal coded string reference
*
* @param[out] dest                     - pointer to GT_FCID structure
*                                       None
*
* @note no assertion is performed on validity of coded string
*
*/
GT_VOID galtisFcoeAddr
(
    OUT GT_FCID   *dest,
    IN  GT_U8     *source
);

/**
* @internal galtisU64 function
* @endinternal
*
* @brief   Create GT_U64 structure from two elements
*
* @param[in] lo                       hi - GT_U64 elements
*
* @param[out] big                      - initialized GT_U64 structure
*                                       None
*
* @note Toolkit:
*
*/
GT_VOID galtisU64
(
    OUT GT_U64 *big,
    IN  GT_U32 lo,
    IN  GT_U32 hi
);

/*******************************************************************************
* galtisU64COut
*
* DESCRIPTION:
*       Create Galtis compatible string output from GT_U64 for OUTPUT
*
* INPUTS:
*       big - 64-bit integer (GT_U64)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       galtis output string that represent the GT_U64 value
*
* COMMENTS:
*
* Toolkit:
*
*******************************************************************************/
GT_U8* galtisU64COut
(
    IN  GT_U64 *big
);

/*******************************************************************************
* galtisU64FOut
*
* DESCRIPTION:
*       Create Galtis compatible string output from GT_U64 for FIELDS
*
* INPUTS:
*       big - 64-bit integer (GT_U64)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       galtis output string that represent the GT_U64 value as table fileds
*
* COMMENTS:
*
* Toolkit:
*
*******************************************************************************/
GT_U8* galtisU64FOut
(
    IN GT_U64 *big
);

/*******************************************************************************
* galtisBArrayOut
*
* DESCRIPTION:
*       Create Galtis compatible string output from char buffer
*
* INPUTS:
*       bArray - the buffer data and length in a struct
*
* OUTPUTS:
*       None
*
* RETURNS:
*       galtis output string that represent the buffer data
*
* COMMENTS:
*
* Toolkit:
*
*******************************************************************************/
GT_U8* galtisBArrayOut
(
    IN GT_BYTE_ARRY* bArray
);

/**
* @internal galtisBArray function
* @endinternal
*
* @brief   build GT_BYTE_ARRY from string
*
* @param[in] source                   - byte array buffer
*
* @param[out] byteArray                - byte array data and size
*                                       None
*
* @note Toolkit:
*
*/
GT_VOID galtisBArray
(
    OUT GT_BYTE_ARRY *byteArray,
    IN  GT_U8* source
);

/**
* @internal galtisBArraySized function
* @endinternal
*
* @brief   build GT_BYTE_ARRY from string expanding or trunkating to size
*
* @param[in] source                   - byte array buffer
* @param[in] size                     - exact  in bytes
* @param[in] defData                  - size of default data
* @param[in] defData                  - default data for bytes, not specified in source
*
* @param[out] byteArray                - byte array data and size
*                                       None
*/
GT_VOID galtisBArraySized
(
    OUT GT_BYTE_ARRY *byteArray ,
    IN GT_U8* source,
    IN GT_U32 size,
    IN GT_U32 defDataSize,
    IN GT_U8* defData
) ;

/**
* @internal galtisBArrayFree function
* @endinternal
*
* @brief   Free GT_BYTE_ARRY from string
*
* @note Toolkit:
*
*/
GT_VOID galtisBArrayFree
(
    IN GT_BYTE_ARRY *byteArray
);

/**
* @internal galtisBytesArrayDirect function
* @endinternal
*
* @brief   build Byte Array from string in the given buffer
*
* @param[in] source                   - byte array buffer (hexadecimal string)
* @param[in] size                     - exact  in bytes
*
* @param[out] byteArray                - byte array data
*                                       None
*/
GT_VOID galtisBytesArrayDirect
(
    OUT GT_U8  *byteArray,
    IN  GT_U8  *source,
    IN  GT_U32 size
);

/**
* @internal galtisIsMacGreater function
* @endinternal
*
* @brief   return true if one mac address is greater than the other.
*
* @param[in] firstMac                 - first MAC address.
* @param[in] secondMac                - second MAC address.
*                                       true if greater.
*/
GT_BOOL galtisIsMacGreater
(
    IN GT_ETHERADDR firstMac,
    IN GT_ETHERADDR secondMac
);

/**
* @internal gtPortsMapDbEnable function
* @endinternal
*
* @brief   Enables ports mapping DB lookup
*/
GT_VOID gtPortsMapDbEnable(GT_VOID);


/**
* @internal gtPortsMapDbDisable function
* @endinternal
*
* @brief   Disables ports mapping DB lookup
*/
GT_VOID gtPortsMapDbDisable(GT_VOID);

/**
* @internal gtIsPortsMapDbEnable function
* @endinternal
*
* @brief   Enables ports mapping DB lookup
*/
GT_BOOL gtIsPortsMapDbEnable(GT_VOID);



/**
* @internal gtPortsDevConvert function
* @endinternal
*
* @brief   Do ports mapping.
*/
void gtPortsDevConvert
(
    INOUT GT_U8              *devNum,
    INOUT GT_U8              *portNum
);

/**
* @internal gtPhysicalPortsDevConvert function
* @endinternal
*
* @brief   Do ports mapping.
*/
GT_VOID gtPhysicalPortsDevConvert
(
   INOUT GT_U8 *devNum,
   INOUT GT_PHYSICAL_PORT_NUM *portNum
);

/**
* @internal gtPhysicalPortsSwDev_32_Convert function
* @endinternal
*
* @brief   Do ports mapping.
*/
GT_VOID gtPhysicalPortsSwDev_32_Convert
(
   INOUT GT_SW_DEV_NUM *devNum,
   INOUT GT_PHYSICAL_PORT_NUM *portNum
);

/**
* @internal gtPhysicalPortsDevConvertBack function
* @endinternal
*
* @brief   Do ports mapping.
*/
GT_VOID gtPhysicalPortsDevConvertBack
(
   INOUT GT_U8                  *devNum,
   INOUT GT_PHYSICAL_PORT_NUM   *portNum
);

/**
* @internal gt_U32_PortsDevConvert function
* @endinternal
*
* @brief   Do ports mapping.
*/
GT_VOID gt_U32_PortsDevConvert
(
   INOUT GT_U8 *devNum,
   INOUT GT_U32 *portNum
);

void gtPortsSwDevConvert
(
    INOUT GT_HW_DEV_NUM     *hwDevNum,
    INOUT GT_PORT_NUM       *portNum
);

void gtPortsDevConvertDataBack
(
   INOUT GT_HW_DEV_NUM  *hwDevNum,
   INOUT GT_PORT_NUM    *portNum
);

void gtPhysicalPortsSwDevConvert
(
   INOUT GT_HW_DEV_NUM            *hwDevNum,
   INOUT GT_PHYSICAL_PORT_NUM     *portNum
);

void gtPhysicalPortsDevConvertDataBack
(
   INOUT GT_HW_DEV_NUM           *hwDevNum,
   INOUT GT_PHYSICAL_PORT_NUM    *portNum
);

void gtPhysicalPortsBitmapDevConvert
(
   INOUT GT_U8                *devNum,
   INOUT CPSS_PORTS_BMP_STC   *portBitMap
);

void gtPhysicalPortsBitmapDevConvertDataBack
(
   INOUT GT_U8                  *devNum,
   INOUT CPSS_PORTS_BMP_STC     *portBitMap
);

/* flag for debug to remove the hwDevNum convert*/
extern GT_BOOL gtHwDevNumConvert;
/* enable/disable the hwDevNum convert .
function sets the flag of gtHwDevNumConvert */
GT_STATUS gtHwDevNumConvertSet
(
   IN GT_BOOL enableConvert
);

/* Don't do casting to GT_U8 for devNum/portNum. They must be of this type already*/
/* CONVERT_DEV_PORT_MAC  :
    convert dev,port when 'dev' is the first input parameter for the CPSS API's.
    so dev is not used as 'Field in HW' and not as 'index in HW table'

    meaning when calling
     cpssYyyFunc(devNum,PortNum,enable); --> need to use CONVERT_DEV_PORT_MAC

     but for :
     fdbEntry.interface.type = 'port';
     the pair of :
     fdbEntry.interface.devPort.hwDevNum,fdbEntry.interface.devPort.portNum -->
     need convert by CONVERT_DEV_PORT_DATA_MAC

     cpssZzzFunc(devNum,&fdbEntry); --> need to use CONVERT_DEV_PORT_KEY_MAC

     and also for function that uses 'interface' as index into HW like

     cpssMmmFunc(devNum , &interface , data) and for :
     fdbEntry.interface.type = 'port';
     fdbEntry.interface.devPort.hwDevNum,fdbEntry.interface.devPort.portNum -->
     used as INDEX in HW --> need convert by CONVERT_DEV_PORT_DATA_MAC

     NOTE: this macro NOT need 'Convert back' because this is 'key' that always
           given from caller and never returned from wrapper

*/
#define CONVERT_DEV_PORT_MAC(devNum, portNum)   \
{                                               \
   gtPortsDevConvert(&devNum, &portNum);        \
}


/* convert device and port like CONVERT_DEV_PORT_MAC but for GT_PORT_PHYSICAL_NUM */
#define CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum)       \
{                                                            \
   gtPhysicalPortsDevConvert(&devNum, &portNum);             \
}

/* convert device and port like CONVERT_DEV_PORT_MAC but for GT_PORT_PHYSICAL_NUM */
#define CONVERT_SW_DEV_PHYSICAL_PORT_MAC(devNum, portNum)       \
{                                                               \
   gtPhysicalPortsSwDev_32_Convert(&devNum, &portNum);             \
}


/* convert device and port after doing CONVERT_DEV_PHYSICAL_PORT_MAC for GT_PORT_PHYSICAL_NUM */
#define CONVERT_BACK_DEV_PHYSICAL_PORT_MAC(devNum, portNum)       \
{                                                            \
   gtPhysicalPortsDevConvertBack(&devNum, &portNum);             \
}

/* convert device and port like CONVERT_DEV_PORT_MAC but for GT_PORT_NUM or
   GT_PHYSICAL_PORT portNum type */
#define CONVERT_DEV_PORT_U32_MAC(devNum, _portNum)   \
{                                                    \
   gt_U32_PortsDevConvert(&devNum, &_portNum);       \
}


/* CONVERT_DEV_PORT_DATA_MAC :
    convert dev,port when those are used as 'Field in HW' or as 'index in HW table'
    and dev is not the first input parameter for the CPSS API's

    meaning when calling
     fdbEntry.interface.type = 'port';
     the pair of :
     fdbEntry.interface.devPort.hwDevNum,fdbEntry.interface.devPort.portNum -->
     need convert by CONVERT_DEV_PORT_DATA_MAC

     cpssZzzFunc(devNum,&fdbEntry); --> need to use CONVERT_DEV_PORT_KEY_MAC

     and also for function that uses 'interface' as index into HW like

     cpssMmmFunc(devNum , &interface , data) and for :
     fdbEntry.interface.type = 'port';
     fdbEntry.interface.devPort.hwDevNum,fdbEntry.interface.devPort.portNum -->
     used as INDEX in HW --> need convert by CONVERT_DEV_PORT_DATA_MAC


     but for :
     cpssYyyFunc(devNum,PortNum,enable); --> need to use CONVERT_DEV_PORT_MAC

*/
#define CONVERT_DEV_PORT_DATA_MAC(devNum, portNum) \
{                                               \
    gtPortsSwDevConvert(&devNum, &portNum);       \
    /* do convert to HW id only after SW convert */ \
    if(gtHwDevNumConvert == GT_TRUE && PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))      \
       devNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);\
}

/* same as CONVERT_DEV_PORT_DATA_MAC just for physical ports */
#define CONVERT_DEV_PHYSICAL_PORT_DATA_MAC(devNum, portNum) \
{                                                           \
    gtPhysicalPortsSwDevConvert(&devNum, &portNum);            \
    /* do convert to HW id only after SW convert */ \
    if(gtHwDevNumConvert == GT_TRUE && PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))      \
       devNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);\
}

/* same as CONVERT_BACK_DEV_PORT_DATA_MAC just for physical ports */
#define CONVERT_BACK_DEV_PHYSICAL_PORT_DATA_MAC(devNum, portNum)  \
{                                                        \
   GT_HW_DEV_NUM        _tmpHwDevNum = devNum;                    \
   GT_PHYSICAL_PORT_NUM _tmpPortNum = portNum;                    \
   gtPhysicalPortsDevConvertDataBack(&_tmpHwDevNum, &_tmpPortNum); \
   devNum = _tmpHwDevNum;                                  \
   portNum = _tmpPortNum;                                \
}

/* CONVERT_BACK_DEV_PORT_DATA_MAC :
    convert back from dev,port of CPSS to this of RDE test.
    the convert back needed ONLY for the cases that used the 'CONVERT_DEV_PORT_DATA_MAC'
*/
#define CONVERT_BACK_DEV_PORT_DATA_MAC(devNum, portNum)  \
{                                                        \
   GT_HW_DEV_NUM _tmpHwDevNum = devNum;                    \
   GT_PORT_NUM _tmpPortNum = portNum;                    \
   gtPortsDevConvertDataBack(&_tmpHwDevNum, &_tmpPortNum); \
   devNum = _tmpHwDevNum;                                  \
   portNum = _tmpPortNum;                                \
}

/* same MACRO as CONVERT_BACK_DEV_PORT_DATA_MAC but use GT_U8 devNum, portNum */
#define CONVERT_BACK_DEV_PORT_DATA_U8_MAC(devNum, portNum)  \
{                                                        \
   GT_HW_DEV_NUM _tmpHwDevNum = devNum;                    \
   GT_PORT_NUM _tmpPortNum = portNum;                    \
   gtPortsDevConvertDataBack(&_tmpHwDevNum, &_tmpPortNum); \
   devNum = (GT_U8)_tmpHwDevNum;                           \
   portNum = (GT_U8)_tmpPortNum;                         \
}

/* CONVERT_DEV_PHYSICAL_PORTS_BMP_MAC :
    convert to dev,portBmp (CPSS_PORTS_BMP_STC) of CPSS from that of RDE test.
    the convert back is done by 'CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC'
*/
#define CONVERT_DEV_PHYSICAL_PORTS_BMP_MAC(devNum, portsBmp) \
{                                                            \
   gtPhysicalPortsBitmapDevConvert(&devNum, &portsBmp);      \
}

/* CONVERT_BACK_DEV_PORTS_BMP_MAC :
    convert back from dev,portBmp (CPSS_PORTS_BMP_STC) of CPSS to that of RDE test.
    the convert back needed ONLY for the cases that used the 'CONVERT_DEV_PHYSICAL_PORTS_BMP_MAC'
*/
#define CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum, portsBmp)    \
{                                                                    \
   gtPhysicalPortsBitmapDevConvertDataBack(&devNum, &portsBmp);      \
}

/**
* @internal gtTrunkConvertFromTestToCpss function
* @endinternal
*
* @brief   convert trunkId from test value (caller of gatlis wrapper to SET value)
*         to value that the CPSS expect
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
*                                       none
*/
void gtTrunkConvertFromTestToCpss
(
   INOUT GT_TRUNK_ID *trunkIdPtr
);

/**
* @internal gtTrunkConvertFromCpssToTest function
* @endinternal
*
* @brief   convert trunkId from CPSS value to value that the TEST expect (refresh)
*
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
*                                       none
*/
void gtTrunkConvertFromCpssToTest
(
   INOUT GT_TRUNK_ID *trunkIdPtr
);

/* Don't do casting to GT_TRUNK_ID for trunkId. it must be of this type already */
#define CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(_trunkId) \
   gtTrunkConvertFromTestToCpss(&_trunkId)

#define CONVERT_TRUNK_ID_CPSS_TO_TEST_MAC(_trunkId) \
   gtTrunkConvertFromCpssToTest(&_trunkId)

/* flag to state that the DXCH (xcat) devices need the WA of trunk-Id conversions */
extern GT_BOOL dxChTrunkWaNeeded;

/**
* @internal gtPclMaskTrunkConvertFromTestToCpss function
* @endinternal
*
* @brief   for PCL MASK trunkId field -- when <isTrunk> has 'exact match' mask and pattern is 1
*         convert trunkId from test value (caller of gatlis wrapper to SET value)
*         to value that the CPSS expect
* @param[in] isTrunkMask              - <is trunk> in the mask
* @param[in] isTrunkPattern           - <is trunk> in the pattern
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
*                                       none
*/
void gtPclMaskTrunkConvertFromTestToCpss
(
    IN    GT_U32        isTrunkMask,
    IN    GT_U32        isTrunkPattern,
    INOUT GT_TRUNK_ID   *trunkIdPtr
);

/**
* @internal gtPclMaskTrunkConvertFromCpssToTest function
* @endinternal
*
* @brief   for PCL MASK trunkId field -- when <isTrunk> has 'exact match' mask and pattern is 1
*         convert trunkId from CPSS value to value that the TEST expect (refresh)
* @param[in] isTrunkMask              - <is trunk> in the mask
* @param[in] isTrunkPattern           - <is trunk> in the pattern
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
*                                       none
*/
void gtPclMaskTrunkConvertFromCpssToTest
(
    IN    GT_U32        isTrunkMask,
    IN    GT_U32        isTrunkPattern,
    INOUT GT_TRUNK_ID *trunkIdPtr
);

/**
* @internal gtPclPatternTrunkConvertFromTestToCpss function
* @endinternal
*
* @brief   for PCL Pattern trunkId field -- when <isTrunk> has 'exact match' mask and pattern is 1
*         convert trunkId from test value (caller of gatlis wrapper to SET value)
*         to value that the CPSS expect
* @param[in] isTrunkMask              - <is trunk> in the mask
* @param[in] isTrunkPattern           - <is trunk> in the pattern
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
*                                       none
*/
void gtPclPatternTrunkConvertFromTestToCpss
(
    IN    GT_U32        isTrunkMask,
    IN    GT_U32        isTrunkPattern,
    INOUT GT_TRUNK_ID   *trunkIdPtr
);

/**
* @internal gtPclPatternTrunkConvertFromCpssToTest function
* @endinternal
*
* @brief   for PCL Pattern trunkId field -- when <isTrunk> has 'exact match' mask and pattern is 1
*         convert trunkId from CPSS value to value that the TEST expect (refresh)
* @param[in] isTrunkMask              - <is trunk> in the mask
* @param[in] isTrunkPattern           - <is trunk> in the pattern
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the CPSS
* @param[in,out] trunkIdPtr               - (pointer to) the trunkId of the test
*                                       none
*/
void gtPclPatternTrunkConvertFromCpssToTest
(
    IN    GT_U32        isTrunkMask,
    IN    GT_U32        isTrunkPattern,
    INOUT GT_TRUNK_ID *trunkIdPtr
);


/**
* @internal gtPclMaskPclIdConvertFromTestToCpss function
* @endinternal
*
* @brief   convert pclId of the pcl mask from test value (caller of gatlis wrapper to SET value)
*         to value that the CPSS expect
*         NOTE: call this only for short keys (24B),not needed for larger keys.
* @param[in,out] pclIdPtr                 - (pointer to) the pclId of the test
* @param[in,out] pclIdPtr                 - (pointer to) the pclId of the CPSS
*                                       none
*/
void gtPclMaskPclIdConvertFromTestToCpss
(
   INOUT GT_U32 *pclIdPtr
);

/* Don't do casting to GT_U16 for pclId. it must be of this type already */
#define CONVERT_PCL_MASK_PCL_ID_TEST_TO_CPSS_MAC(_pclId) \
   gtPclMaskPclIdConvertFromTestToCpss(&_pclId)

/**
* @internal gtTrunkWaForceDownSet function
* @endinternal
*
* @brief   set if the trunk WA need to be forced down (disabled)
*
* @param[in] forceDown                - do we want to force down the WA
*                                      GT_TRUE - force down (disable) the WA
*                                      GT_FALSE - don't force down (allow to enable) the WA
*
* @retval GT_OK                    - success
*
* @note this function is to be called from the terminal -->
*       AFTER cpssInitSystem(...)
*
*/
GT_STATUS gtTrunkWaForceDownSet
(
    IN GT_U32    forceDown
);


/**
* @internal utilFindStructureElenentDescriptorByPath function
* @endinternal
*
* @brief   searches the structure element descriptor by path
*
* @param[in] startDscArr[]            - (pointer to) array of descriptors for root structure
* @param[in] dscEntrySize             - size of descriptor entry in bytes
*                                      dedicated to application definded descrioprs that
*                                      contain the GT_UTIL_STC_ELEMENT_DSC_STC as first member
*                                      0 means sizeof(GT_UTIL_STC_ELEMENT_DSC_STC)
* @param[in] nameSeparator            - character used as separator in the path
* @param[in] pathStr                  - string that contains the path of the searched field
*
* @param[out] foundDscPtrPtr           - (pointer to) pointer to found descriptor entry
* @param[out] summaryOffsetPtr         - (pointer to) summary offset of the root structure origin
*
* @retval GT_OK                    - found
* @retval GT_NO_SUCH               - not found
*/
GT_STATUS utilFindStructureElenentDescriptorByPath
(
    IN  GT_UTIL_STC_ELEMENT_DSC_STC   startDscArr[],
    IN  GT_U32                        dscEntrySize,
    IN  GT_U8                         nameSeparator,
    IN  GT_U8                         *pathStr,
    OUT GT_UTIL_STC_ELEMENT_DSC_STC   **foundDscPtrPtr,
    OUT GT_U32                        *summaryOffsetPtr
);

/**
* @internal utilListStructureElenentNamesByPath function
* @endinternal
*
* @brief   prints the structure element names by path
*
* @param[in] startDscArr[]            - (pointer to) array of descriptors for root structure
* @param[in] dscEntrySize             - size of descriptor entry in bytes
*                                      dedicated to application definded descrioprs that
*                                      contain the GT_UTIL_STC_ELEMENT_DSC_STC as first member
*                                      0 means sizeof(GT_UTIL_STC_ELEMENT_DSC_STC)
* @param[in] nameSeparator            - character used as separator in the path
* @param[in] pathStr                  - string that contains the path of the searched field
*
* @retval GT_OK                    - found
* @retval GT_NO_SUCH               - not found
*/
GT_STATUS utilListStructureElenentNamesByPath
(
    IN  GT_UTIL_STC_ELEMENT_DSC_STC   startDscArr[],
    IN  GT_U32                        dscEntrySize,
    IN  GT_U8                         nameSeparator,
    IN  GT_U8                         *pathStr
);

/**
* @internal utilHexStringToStcField function
* @endinternal
*
* @brief   Convers the string that contains pairs of hex numbers to the
*         contents of the "C" language defined data
* @param[in] dataStr                  - (pointer to) string with pairs of hex numbers
* @param[in] dataBytesSize            - amount of bytes to store
* @param[in] storeFormat              - the format conversion rule used during the strore
*
* @param[out] fieldPtr                 - (pointer to) the field tat the data stored to
*
* @retval GT_OK                    - OK
* @retval GT_BAD_PARAM             - wrong hex data in the string
*/
GT_STATUS utilHexStringToStcField
(
    IN  GT_U8                              *dataStr,
    IN  GT_U32                             dataBytesSize,
    IN  GT_UTIL_HEX_DATA_STORE_FORMAT_ENT  storeFormat,
    OUT GT_VOID                            *fieldPtr
);

/**
* @internal utilMultiPortGroupsBmpSet function
* @endinternal
*
* @brief   set the portGroupsBmp for multi port groups device.
*         when 'enabled' --> wrappers will use the APIs with portGroupsBmp parameter
* @param[in] devNum                   - device number
* @param[in] enable                   -  / disable the use of APIs with portGroupsBmp parameter.
* @param[in] portGroupsBmp            - port groups bmp , relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - OK
* @retval GT_BAD_PARAM             - on bad device number
*/
GT_STATUS utilMultiPortGroupsBmpSet
(
    IN  GT_U8               devNum,
    IN  GT_BOOL             enable,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp
);

/**
* @internal utilMultiPortGroupsBmpGet function
* @endinternal
*
* @brief   Get the portGroupsBmp for multi port groups device.
*         when 'enabled' --> wrappers will use the APIs with portGroupsBmp parameter
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable / disable the use of APIs with portGroupsBmp parameter.
* @param[out] portGroupsBmpPtr         - (pointer to)port groups bmp , relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - OK
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS utilMultiPortGroupsBmpGet
(
    IN  GT_U8               devNum,
    OUT  GT_BOOL             *enablePtr,
    OUT  GT_PORT_GROUPS_BMP  *portGroupsBmpPtr
);


/* convert from
    devNum,portGroupsBmp    of test values  (received from Galtis/RDE test)

    to

    devNum,portGroupsBmp    of CPSS values
*/
void gtPortGroupDevConvert
(
   INOUT GT_U8 *devNum,
   INOUT GT_PORT_GROUPS_BMP *portGroupsBmpPtr
);

/* convert from
    devNum,portGroupsBmp    of CPSS values

    to

    devNum,portGroupsBmp    of test values (to send to Galtis/RDE test)
*/
void gtPortGroupDevConvertBack
(
   INOUT GT_U8 *devNum,
   INOUT GT_PORT_GROUPS_BMP *portGroupsBmpPtr
);

/* Don't do casting to GT_U8 for devNum. They must be of this type already*/
#define CONVERT_DEV_PORT_GROUPS_BMP_MAC(devNum, portGroupsBmp) \
   gtPortGroupDevConvert(&devNum, &portGroupsBmp)

#define CONVERT_BACK_DEV_PORT_GROUPS_BMP_MAC(devNum, portGroupsBmp) \
   gtPortGroupDevConvertBack(&devNum, &portGroupsBmp)

/* allow exmxpm devices to imitate puma2 24 physical ports mode */
GT_U32 wrCpssExMxPmImitationPhysicalPortMode_puma2_24ports_mode_Get(void);

typedef GT_VOID (*wrCpssResetLibFuncPtr)(GT_VOID);

/**
* @internal wrCpssRegisterResetCb function
* @endinternal
*
* @brief   Register reset callback.
*
* @retval GT_OK                    - OK
* @retval GT_FULL                  - on full list
*/
GT_STATUS wrCpssRegisterResetCb
(
    wrCpssResetLibFuncPtr funcPtr
);

/**
* @internal utilCpssToWrFamilyConvert function
* @endinternal
*
* @brief   Convert CPSS family type enumeration value to wrapper used enumeration.
*
* @param[in] cpssFamily               - CPSS family type
*
* @param[out] wrFamilyPtrPtr           - (pointer to) pointer to wrapper family type
*
* @retval GT_OK                    - convert done
* @retval GT_FAIL                  - wrong family type, no conversion
*/
GT_STATUS utilCpssToWrFamilyConvert
(
    IN  CPSS_PP_FAMILY_TYPE_ENT      cpssFamily,
    OUT GT_UTIL_PP_FAMILY_TYPE_ENT  *wrFamilyPtrPtr
);

/**
* @internal utilCpssToWrSubFamilyConvert function
* @endinternal
*
* @brief   Convert CPSS sub-family type enumeration value to wrapper used enumeration.
*
* @param[in] cpssSubFamily            - CPSS family type
*
* @param[out] wrSubFamilyPtr           - (pointer to) wrapper family type
*
* @retval GT_OK                    - convert done
* @retval GT_FAIL                  - wrong sub-family type, no conversion
*/
GT_STATUS utilCpssToWrSubFamilyConvert
(
    IN  CPSS_PP_SUB_FAMILY_TYPE_ENT      cpssSubFamily,
    OUT GT_UTIL_PP_SUB_FAMILY_TYPE_ENT  *wrSubFamilyPtr
);

#ifdef  __cplusplus
}
#endif  /* __cplusplus      */
#endif /* __cmdPresteraUtils_h__ */


