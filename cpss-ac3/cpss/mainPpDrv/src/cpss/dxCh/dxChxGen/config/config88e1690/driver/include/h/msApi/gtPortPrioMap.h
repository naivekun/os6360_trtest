#include <Copyright.h>
/**
********************************************************************************
* @file gtPortPrioMap.h
*
* @brief API/Structure definitions for Marvell Port Priority Mapping Rules functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtPortPrioMap.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell Port Priority Mapping Rules functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtPortPrioMap_h
#define __prvCpssDrvGtPortPrioMap_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* Exported Port Control Types                                              */
/****************************************************************************/

/**
* @enum GT_CPSS_INITIAL_PRI
 *
 * @brief Defines Initial Priority
*/
typedef enum{

    /** Use Port defauts for Fpri, Qpri&Color. */
    GT_CPSS_INITIAL_PRI_DEFAULT= 0,

    /** Use Tag Priority. */
    GT_CPSS_INITIAL_PRI_TAG,

    /** Use IP Priority. */
    GT_CPSS_INITIAL_PRI_IP,

    /** Use Tag&IP Priority. */
    GT_CPSS_INITIAL_PRI_TAG_IP

} GT_CPSS_INITIAL_PRI;

/* Supported Frame Priority Table set For IEEE priority mapping */
typedef enum
{
  GT_CPSS_IEEE_PRIORITY_TABLE_ING_PCP_2_F_Q_PRI,   /* 0 Ingress PCP to IEEE_FPRI & IEEE_QPRI mapping */
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_PCP, /* 0x1 Egress Green Frame FPRI to PCP mapping (non-AVB) */
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_PCP,   /* 0x2 = Egress Yellow Frame FPRI to PCP mapping (non-AVB) */
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_PCP,   /* 0x3 = Egress AVB Frame FPRI to PCP mapping */
  GT_CPSS_IEEE_PRIORITY_TABLE_RESERVED         ,   /* 0x4 = Reserved for future use */
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_DSCP,/* 0x5 = Egress Green Frame FPRI to DSCP mapping (non-AVB) */
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_DSCP,  /* 0x6 = Egress Yellow Frame FPRI to DSCP mapping (non-AVB) */
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_DSCP,  /* 0x7 = Egress AVB Frame FPRI to DSCP mapping */
} GT_CPSS_IEEE_PRIORITY_TABLE_SET;

/* Supported the format of each Table set For IEEE priority mapping */
typedef struct
{
    GT_U8     ieeeYellow;
    GT_U8     disIeeeQpri;
    GT_U8     ieeeQpri;
    GT_U8     disIeeeFpri;
    GT_U8     ieeeFpri;
} GT_CPSS_IEEE_PRIORITY_TABLE_ING_PCP_2_F_Q_PRI_STRUCT;
typedef struct
{
    GT_U8     disVIDMarking;
    GT_U8     disPCPMarking;
    GT_U8     framePCP;
} GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_PCP_STRUCT;
typedef struct
{
    GT_U8     disVIDMarking;
    GT_U8     disPCPMarking;
    GT_U8     framePCP;
} GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_PCP_STRUCT;
typedef struct
{
    GT_U8     disVIDMarking;
    GT_U8     disPCPMarking;
    GT_U8     framePCP;
} GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_PCP_STRUCT;
typedef struct
{
    GT_U8     enDSCPMarking;
    GT_U8     frameDSCP;
} GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_DSCP_STRUCT;
typedef struct
{
    GT_U8     enDSCPMarking;
    GT_U8     frameDSCP;
} GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_DSCP_STRUCT;
typedef struct
{
    GT_U8     enDSCPMarking;
    GT_U8     frameDSCP;
} GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_DSCP_STRUCT;

typedef union
{
  GT_CPSS_IEEE_PRIORITY_TABLE_ING_PCP_2_F_Q_PRI_STRUCT     ingTbl;
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_PCP_STRUCT   egGreeTblPcp;
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_PCP_STRUCT     egYelTblPcp;
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_PCP_STRUCT     egAvbTblPcp;
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_DSCP_STRUCT  egGreeTblDscp;
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_DSCP_STRUCT    egYelTblDscp;
  GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_DSCP_STRUCT    egAvbTbldscp;
} GT_CPSS_IEEE_PRIORITY_TABLE_STRUCT;

/****************************************************************************/
/* Exported Port Control Functions                                          */
/****************************************************************************/

/**
* @internal prvCpssDrvGqosSetPrioMapRule function
* @endinternal
*
* @brief   This routine sets priority mapping rule(TagIfBoth bit).
*         If the current frame is both IEEE 802.3ac tagged and an IPv4 or IPv6,
*         and UserPrioMap (for IEEE 802.3ac) and IPPrioMap (for IP frame) are
*         enabled, then priority selection is made based on this setup.
*         If PrioMapRule is set to GT_TRUE, UserPrioMap is used.
*         If PrioMapRule is reset to GT_FALSE, IPPrioMap is used.
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_TRUE for user prio rule, GT_FALSE for otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGqosSetPrioMapRule
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL    mode
);

/**
* @internal prvCpssDrvGqosGetPrioMapRule function
* @endinternal
*
* @brief   This routine gets priority mapping rule (TagIfBoth bit).
*         If the current frame is both IEEE 802.3ac tagged and an IPv4 or IPv6,
*         and UserPrioMap (for IEEE 802.3ac) and IPPrioMap (for IP frame) are
*         enabled, then priority selection is made based on this setup.
*         If PrioMapRule is set to GT_TRUE, UserPrioMap is used.
*         If PrioMapRule is reset to GT_FALSE, IPPrioMap is used.
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_TRUE for user prio rule, GT_FALSE for otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGqosGetPrioMapRule
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *mode
);

/**
* @internal prvCpssDrvGprtSetInitialPri function
* @endinternal
*
* @brief   This routine set initial Priority assignment.
*         Each frame entering a port is assigned a Frame Priority(FPri) and
*         a Queue Priority (QPri). This mode can be assigned as follows:
*         GT_CPSS_INITIAL_PRI_DEFAULT
*         GT_CPSS_INITIAL_PRI_TAG
*         GT_CPSS_INITIAL_PRI_IP
*         GT_CPSS_INITIAL_PRI_TAG_IP
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_CPSS_INITIAL_PRI type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetInitialPri
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    IN  GT_CPSS_INITIAL_PRI     mode
);

/**
* @internal prvCpssDrvGprtGetInitialPri function
* @endinternal
*
* @brief   This routine gets initial Priority assignment.
*         Each frame entering a port is assigned a Frame Priority(FPri) and
*         a Queue Priority (QPri). This mode can be assigned as follows:
*         GT_CPSS_INITIAL_PRI_DEFAULT
*         GT_CPSS_INITIAL_PRI_TAG
*         GT_CPSS_INITIAL_PRI_IP
*         GT_CPSS_INITIAL_PRI_TAG_IP
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_CPSS_INITIAL_PRI type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetInitialPri
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    OUT GT_CPSS_INITIAL_PRI     *mode
);

/**
* @internal prvCpssDrvGprtSetDefQpri function
* @endinternal
*
* @brief   This routine Set Default queue priority.
*
* @param[in] port                     - the logical  number
* @param[in] defque                   - default queue priority (0~7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetDefQpri
(
  IN  GT_CPSS_QD_DEV    *dev,
  IN  GT_CPSS_LPORT     port,
  IN  GT_U8             defque
);

/**
* @internal prvCpssDrvGprtGetDefQpri function
* @endinternal
*
* @brief   This routine Get Default queue priority.
*
* @param[in] port                     - the logical  number
*
* @param[out] defque                   - default queue priority structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetDefQpri
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    OUT GT_U8            *defque
);

/**
* @internal prvCpssDrvGprtSetIEEEPrioMap function
* @endinternal
*
* @brief   This API set port IEEE Priority Mapping.
*
* @param[in] port                     - the logical  number
* @param[in] table                    - Frame Priority Table set
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_ING_PCP_2_F_Q_PRI
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_PCP
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_PCP
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_PCP
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_DSCP
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_DSCP
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_DSCP
* @param[in] entryPtr                 - Pointer to the desired entry of the IEEE Priority
*                                      Mapping table selected above
* @param[in] entryStruct              - Mapping data structure for the above selected entry.
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_ING_PCP_2_F_Q_PRI_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_PCP_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_PCP_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_PCP_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_DSCP_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_DSCP_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_DSCP_STRUCT;
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetIEEEPrioMap
(
    IN  GT_CPSS_QD_DEV                      *dev,
    IN  GT_CPSS_LPORT                       port,
    IN  GT_CPSS_IEEE_PRIORITY_TABLE_SET     table,
    IN  GT_U8                               entryPtr,
    IN  GT_CPSS_IEEE_PRIORITY_TABLE_STRUCT  *entryStruct
);

/**
* @internal prvCpssDrvGprtGetIEEEPrioMap function
* @endinternal
*
* @brief   This API ge tport IEEE Priority Mapping.
*
* @param[in] port                     - the logical  number
* @param[in] table                    - Frame Priority Table set
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_ING_PCP_2_F_Q_PRI
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_PCP
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_PCP
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_PCP
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_DSCP
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_DSCP
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_DSCP
* @param[in] entryPtr                 - Pointer to the desired entry of the IEEE Priority
*                                      Mapping table selected above
*
* @param[out] entryStruct              - Mapping data structure for the above selected entry.
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_ING_PCP_2_F_Q_PRI_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_PCP_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_PCP_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_PCP_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_GREEN_FPRI_2_DSCP_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_YEL_FPRI_2_DSCP_STRUCT;
*                                      GT_CPSS_IEEE_PRIORITY_TABLE_EG_AVB_FPRI_2_DSCP_STRUCT;
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetIEEEPrioMap
(
    IN  GT_CPSS_QD_DEV                      *dev,
    IN  GT_CPSS_LPORT                       port,
    IN  GT_CPSS_IEEE_PRIORITY_TABLE_SET     table,
    IN  GT_U8                               entryPtr,
    OUT GT_CPSS_IEEE_PRIORITY_TABLE_STRUCT  *entryStruct
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtPortPrioMap_h */

