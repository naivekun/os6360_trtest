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
* @file cpssDxChBrgMc.h
*
* @brief CPSS DXCh Multicast Group facility API.
*
* @version   13
********************************************************************************
*/
#ifndef __cpssDxChBrgMch
#define __cpssDxChBrgMch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal cpssDxChBrgMcInit function
* @endinternal
*
* @brief   Initializes Bridge engine Multicast Library.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcInit
(
    IN GT_U8    devNum
);

/**
* @internal cpssDxChBrgMcIpv6BytesSelectSet function
* @endinternal
*
* @brief   Sets selected 4 bytes from the IPv6 SIP and 4 bytes from the IPv6 DIP
*         are used upon IPv6 MC bridging, based on the packet's IP addresses (due
*         to the limitation of Cheetah FDB Entry, which contains only 32 bits for
*         DIP and 32 bits for SIP, in spite of 128 bits length of IPv6 address).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] dipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      DIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains DIP byte 0,
*                                      respectively the fourth array element contains
*                                      DIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
* @param[in] sipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      SIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains SIP byte 0,
*                                      respectively the fourth array element contains
*                                      SIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcIpv6BytesSelectSet
(
    IN GT_U8  devNum,
    IN GT_U8  dipBytesSelectMapArr[4],
    IN GT_U8  sipBytesSelectMapArr[4]
);

/**
* @internal cpssDxChBrgMcIpv6BytesSelectGet function
* @endinternal
*
* @brief   Gets selected 4 bytes from the IPv6 SIP and 4 bytes from the IPv6 DIP
*         are used upon IPv6 MC bridging, based on the packet's IP addresses (due
*         to the limitation of Cheetah FDB Entry, which contains only 32 bits for
*         DIP and 32 bits for SIP, in spite of 128 bits length of IPv6 address).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
*
* @param[out] dipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      DIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains DIP byte 0,
*                                      respectively the fourth array element contains
*                                      DIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
* @param[out] sipBytesSelectMapArr[4]  array, which contains numbers of 4 bytes from the
*                                      SIP IPv6 address are used upon IPv6 MC bridging.
*                                      The first array element contains SIP byte 0,
*                                      respectively the fourth array element contains
*                                      SIP byte 3.
*                                      The numbers of IPv6 address bytes are in the
*                                      Network format and in the range [0:15], where 0
*                                      is MSB of IP address and 15 is LSB of IP address.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcIpv6BytesSelectGet
(
    IN  GT_U8  devNum,
    OUT GT_U8  dipBytesSelectMapArr[4],
    OUT GT_U8  sipBytesSelectMapArr[4]
);

/**
* @internal cpssDxChBrgMcEntryWrite function
* @endinternal
*
* @brief   Writes Multicast Group entry to the HW.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for configuration.
*                                      The flood VIDX is 0x3FF for xCat2 devices and
*                                      0xFFF for other devices.
* @param[in] vidx                     MC group of specified PP device. 
* @param[in] portBitmapPtr            - pointer to the bitmap of ports are belonged to the
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..26;
*                                      DxCh2, DxCh3, xCat, xCat3, Lion, xCat2 0..27;
*                                      Lion2, Caelum, Aldrin, AC3X, Aldrin2 0..127;
*                                      Bobcat2 0..255;
*                                      Bobcat3 0..511)
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, ports bitmap value or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function doesn't allow rewrite the VIDX 0xFFF Multicast Table Entry.
*       It should allow the Multicast packets to be flooded to all VLAN ports.
*
*/
GT_STATUS cpssDxChBrgMcEntryWrite
(
    IN GT_U8                devNum,
    IN GT_U16               vidx,
    IN CPSS_PORTS_BMP_STC   *portBitmapPtr
);

/**
* @internal cpssDxChBrgMcEntryRead function
* @endinternal
*
* @brief   Reads the Multicast Group entry from the HW.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is allowed for reading.
*                                      The flood VIDX is 0x3FF for xCat2 devices and
*                                      0xFFF for other devices.
*
* @param[out] portBitmapPtr            - pointer to the bitmap of ports are belonged to the
* @param[out] vidx                     MC group of specified PP device.
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or
*                                       vidx is larger than the allowed value.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcEntryRead
(
    IN  GT_U8                devNum,
    IN  GT_U16               vidx,
    OUT CPSS_PORTS_BMP_STC   *portBitmapPtr
);

/**
* @internal cpssDxChBrgMcGroupDelete function
* @endinternal
*
* @brief   Deletes the Multicast Group entry from the HW.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for configuration.
*                                      The flood VIDX is 0x3FF for xCat2 devices and
*                                      0xFFF for other devices.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcGroupDelete
(
    IN GT_U8    devNum,
    IN GT_U16   vidx
);

/**
* @internal cpssDxChBrgMcMemberAdd function
* @endinternal
*
* @brief   Add new port member to the Multicast Group entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for non CPU port
*                                      configuration. CPU port may be added to the flood
*                                      VIDX entry.
*                                      The flood VIDX is 0x3FF for xCat2 devices and
*                                      0xFFF for other devices.
* @param[in] portNum                  - physical port number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..26;
*                                      DxCh2, DxCh3, xCat, xCat3, Lion, xCat2 0..27;
*                                      Lion2, Caelum, Aldrin, AC3X, Aldrin2 0..127;
*                                      Bobcat2 0..255;
*                                      Bobcat3 0..511)
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_U16                   vidx,
    IN GT_PHYSICAL_PORT_NUM     portNum
);

/**
* @internal cpssDxChBrgMcMemberDelete function
* @endinternal
*
* @brief   Delete port member from the Multicast Group entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] vidx                     - multicast group index.
*                                      valid range 0..(Layer-2 Multicast groups number - 1).
*                                      see datasheet of specific device to get number of
*                                      Layer-2 Multicast groups.
*                                      The flood VIDX entry is not allowed for non CPU port
*                                      configuration. CPU port may be deleted from the flood
*                                      VIDX entry.
*                                      The flood VIDX is 0x3FF for xCat2 devices and
*                                      0xFFF for other devices.
* @param[in] portNum                  - physical port number.
*                                      (APPLICABLE RANGES: DxCh1, DxCh1_Diamond 0..26;
*                                      DxCh2, DxCh3, xCat, xCat3, Lion, xCat2 0..27;
*                                      Lion2, Caelum, Aldrin, AC3X, Aldrin2 0..127;
*                                      Bobcat2 0..255;
*                                      Bobcat3 0..511)
*                                      CPU port supported
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, portNum or
*                                       vidx is larger than the allowed value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcMemberDelete
(
    IN GT_U8                        devNum,
    IN GT_U16                       vidx,
    IN GT_PHYSICAL_PORT_NUM         portNum
);


/**
* @internal cpssDxChBrgMcPhysicalSourceFilteringEnableSet function
* @endinternal
*
* @brief   Enable/disable source physical port/trunk filtering for packets that are
*         forwarded to a VIDX target.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable filtering
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcPhysicalSourceFilteringEnableSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgMcPhysicalSourceFilteringEnableGet function
* @endinternal
*
* @brief   Get the enabling status of source physical port/trunk filtering for
*         packets that are forwarded to a VIDX target.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - filtering enabling status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcPhysicalSourceFilteringEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
);


/**
* @internal cpssDxChBrgMcPhysicalSourceFilteringIgnoreSet function
* @endinternal
*
* @brief   Set indication that the physical port can 'ignore' MC src physical port filtering.
*         Enable/Disable 'ignoring' MC src physical port filtering .
*         Filter relate to sending Multicast packets back to its source physical port
*         by ignoring the 'SrcMcFilter' that may be set by ingress eport
*         see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...)
*         NOTE: by default ALL physical ports are set 'IgnorePhySrcMcFilter = false'
*         except for cascade port that are set as 'IgnorePhySrcMcFilter = true'
*         (done by API cpssDxChCscdPortTypeSet(...))
*         BUT it may be required for some cases to override those 'default' settings.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ignoreFilter             - Boolean value:
*                                      GT_TRUE  - 'ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      regardless to filter set by the 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...).
*                                      GT_FALSE - 'do not ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      only if allowed by 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcPhysicalSourceFilteringIgnoreSet
(
    IN GT_U8            devNum,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    IN GT_BOOL          ignoreFilter
);

/**
* @internal cpssDxChBrgMcPhysicalSourceFilteringIgnoreGet function
* @endinternal
*
* @brief   Get indication if the physical port 'ignoring' MC src physical port filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ignoreFilterPtr          - (pointer to) Boolean value:
*                                      GT_TRUE  - 'ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      regardless to filter set by the 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...).
*                                      GT_FALSE - 'do not ignore' the 'per eport' filter :
*                                      the physical port will allow MC back to it ,
*                                      only if allowed by 'src eport'
*                                      see API cpssDxChBrgMcPhysicalSourceFilteringEnableSet(...)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong dev
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgMcPhysicalSourceFilteringIgnoreGet
(
    IN GT_U8            devNum,
    IN GT_PHYSICAL_PORT_NUM      portNum,
    OUT GT_BOOL          *ignoreFilterPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgMch */

