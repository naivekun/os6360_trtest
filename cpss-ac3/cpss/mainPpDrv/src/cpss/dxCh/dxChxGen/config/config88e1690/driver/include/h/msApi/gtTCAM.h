#include <Copyright.h>
/**
********************************************************************************
* @file gtTCAM.h
*
* @brief API/Structure definitions for Marvell TCAM functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtTCAM.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell TCAM functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtTCAM_h
#define __prvCpssDrvGtTCAM_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* Exported TCAM Types                                                      */
/****************************************************************************/

typedef struct {
    /* Key & Mask */
    GT_U8        frameType;
    GT_U8        frameTypeMask;
    GT_U16       spv;
    GT_U16       spvMask;
    GT_U8        ppri;
    GT_U8        ppriMask;
    GT_U16       pvid;
    GT_U16       pvidMask;

    /* Ethernet Frame Content*/
    GT_U8        frameOctet[48];
    GT_U8        frameOctetMask[48];

    /* Ingress Action */
    GT_U8        continu;
    GT_U8        interrupt;
    GT_U8        IncTcamCtr;
    GT_BOOL      vidOverride;
    GT_U16       vidData;

    GT_U8        nextId;
    GT_BOOL      qpriOverride;
    GT_U8        qpriData;
    GT_BOOL      fpriOverride;
    GT_U8        fpriData;

    GT_U8        dpvSF;
    GT_U32       dpvData;

    GT_U8        dpvMode;
    GT_U8        colorMode;
    GT_BOOL      vtuPageOverride;
    GT_U8        vtuPage;
    GT_U8        unKnownFilter;
    GT_U8        egActPoint;

    GT_BOOL      ldBalanceOverride;
    GT_U8        ldBalanceData;
    GT_BOOL      DSCPOverride;
    GT_U8        DSCP;

    GT_BOOL      factionOverride;
    GT_U16       factionData;
} GT_CPSS_TCAM_DATA;

typedef struct
{
    GT_BOOL      frameModeOverride;
    GT_U8        frameMode;
    GT_BOOL      tagModeOverride;
    GT_U8        tagMode;
    GT_U8        daMode;
    GT_U8        saMode;

    GT_BOOL      egVidModeOverride;
    GT_U8        egVidMode;
    GT_U16       egVidData;

    GT_U8        egDSCPMode;
    GT_U8        egDSCP;
    GT_BOOL      egfpriModeOverride;
    GT_U8        egfpriMode;
    GT_U8        egEC;
    GT_U8        egFPRI;

} GT_CPSS_TCAM_EGR_DATA;


/****************************************************************************/
/* Exported TCAM Functions                                                  */
/****************************************************************************/

/**
* @internal prvCpssDrvGtcamFlushAll function
* @endinternal
*
* @brief   This routine is to flush all entries. A Flush All command will initialize
*         TCAM Pages 0 and 1, offsets 0x02 to 0x1B to 0x0000, and TCAM Page 2 offset
*         0x02 to 0x1B to 0x0000 for all TCAM entries with the exception that TCAM
*         Page 0 offset 0x02 will be initialized to 0x00FF.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGtcamFlushAll
(
    IN  GT_CPSS_QD_DEV     *dev
);

/**
* @internal prvCpssDrvGtcamFlushEntry function
* @endinternal
*
* @brief   This routine is to flush a single entry. A Flush a single TCAM entry command
*         will write the same values to a TCAM entry as a Flush All command, but it is
*         done to the selected single TCAM entry only.
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (0 ~ 255)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamFlushEntry
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U32              tcamPointer
);

/**
* @internal prvCpssDrvGtcamLoadEntry function
* @endinternal
*
* @brief   This routine loads a TCAM entry.
*         The load sequence of TCAM entry is critical. Each TCAM entry is made up of
*         3 pages of data. All 3 pages need to loaded in a particular order for the TCAM
*         to operate correctly while frames are flowing through the switch.
*         If the entry is currently valid, it must first be flushed. Then page 2 needs
*         to be loaded first, followed by page 1 and then finally page 0.
*         Each page load requires its own write TCAMOp with these TCAM page bits set
*         accordingly.
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (0 ~ 255)
* @param[in] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamLoadEntry
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U32              tcamPointer,
    IN  GT_CPSS_TCAM_DATA   *tcamData
);

/**
* @internal prvCpssDrvGtcamGetNextTCAMData function
* @endinternal
*
* @brief   This routine finds the next higher TCAM Entry number that is valid (i.e.,
*         any entry whose Page 0 offset 0x02 is not equal to 0x00FF). The TCAM Entry
*         register (bits 7:0) is used as the TCAM entry to start from. To find
*         the lowest number TCAM Entry that is valid, start the Get Next operation
*         with TCAM Entry set to 0xFF.
* @param[in,out] tcamPointer              - pointer to the desired entry of TCAM (0 ~ 255)
* @param[in,out] tcamPointer              - next pointer entry of TCAM (0 ~ 255)
*
* @param[out] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NO_SUCH               - no more entries.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamGetNextTCAMData
(
    IN    GT_CPSS_QD_DEV        *dev,
    INOUT GT_U32                *tcamPointer,
    OUT   GT_CPSS_TCAM_DATA     *tcamData
);

/**
* @internal prvCpssDrvGtcamReadTCAMData function
* @endinternal
*
* @brief   This routine reads the TCAM offsets 0x02 to 0x1B registers with
*         the data found in the TCAM entry and its TCAM page pointed to by the TCAM
*         entry and TCAM page bits of this register (bits 7:0 and 11:10 respectively.
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (0 ~ 255)
*
* @param[out] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamReadTCAMData
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U32              tcamPointer,
    OUT GT_CPSS_TCAM_DATA   *tcamData
);

/**
* @internal prvCpssDrvGtcamFindEntry function
* @endinternal
*
* @brief   Find the specified valid tcam entry in ingress TCAM Table.
*
* @param[in] tcamPointer              - the tcam entry index to search.
*
* @param[out] found                    - GT_TRUE, if the appropriate entry exists.
* @param[out] tcamData                 - the entry parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error or entry does not exist.
* @retval GT_NO_SUCH               - no more entries.
* @retval GT_BAD_PARAM             - on bad parameter
*/
GT_STATUS prvCpssDrvGtcamFindEntry
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U32              tcamPointer,
    OUT GT_CPSS_TCAM_DATA   *tcamData,
    OUT GT_BOOL             *found
);

/**
* @internal prvCpssDrvGtcamEgrFlushEntry function
* @endinternal
*
* @brief   This routine is to flush a single egress entry for a particular port. A
*         Flush a single Egress TCAM entry command will write the same values to a
*         Egress TCAM entry as a Flush All command, but it is done to the selected
*         single egress TCAM entry of the selected single port only.
* @param[in] port                     - switch port
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (1 ~ 63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamEgrFlushEntry
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    IN  GT_U32              tcamPointer
);

/**
* @internal prvCpssDrvGtcamEgrFlushEntryAllPorts function
* @endinternal
*
* @brief   This routine is to flush a single egress entry for all switch ports.
*
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (1 ~ 63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamEgrFlushEntryAllPorts
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_U32              tcamPointer
);

/**
* @internal prvCpssDrvGtcamEgrLoadEntry function
* @endinternal
*
* @brief   This routine loads a single egress TCAM entry for a specific port.
*
* @param[in] port                     - switch port
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (1 ~ 63)
* @param[in] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamEgrLoadEntry
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    IN  GT_U32                  tcamPointer,
    IN  GT_CPSS_TCAM_EGR_DATA   *tcamData
);

/**
* @internal prvCpssDrvGtcamEgrGetNextTCAMData function
* @endinternal
*
* @brief   This routine finds the next higher Egress TCAM Entry number that is valid
*         (i.e.,any entry that is non-zero). The TCAM Entry register (bits 5:0) is
*         used as the Egress TCAM entry to start from. To find the lowest number
*         Egress TCAM Entry that is valid, start the Get Next operation with Egress
*         TCAM Entry set to 0x3F.
* @param[in] port                     - switch port
* @param[in,out] tcamPointer              - pointer to start search TCAM
* @param[in,out] tcamPointer              - next pointer entry of TCAM
*
* @param[out] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NO_SUCH               - no more entries.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamEgrGetNextTCAMData
(
    IN    GT_CPSS_QD_DEV            *dev,
    IN    GT_CPSS_LPORT             port,
    INOUT GT_U32                    *tcamPointer,
    OUT   GT_CPSS_TCAM_EGR_DATA     *tcamData
);

/**
* @internal prvCpssDrvGtcamEgrReadTCAMData function
* @endinternal
*
* @brief   This routine reads a single Egress TCAM entry for a specific port.
*
* @param[in] port                     - switch port
* @param[in] tcamPointer              - pointer to the desired entry of TCAM (1 ~ 63)
*
* @param[out] tcamData                 - Tcam entry Data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGtcamEgrReadTCAMData
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    IN  GT_U32                  tcamPointer,
    OUT GT_CPSS_TCAM_EGR_DATA   *tcamData
);


#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtTCAM_h */

