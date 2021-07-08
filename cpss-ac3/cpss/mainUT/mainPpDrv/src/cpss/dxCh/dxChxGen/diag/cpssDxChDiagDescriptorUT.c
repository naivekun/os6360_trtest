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
* @file cpssDxChDiagDescriptorUT.c
*
* @brief Unit tests for cpssDxChDiagDescriptor
* CPSS DXCH Diagnostic API
*
* @version   7
********************************************************************************
*/
/* includes */

#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDescriptor.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDescriptorRawGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT   descriptorType,
    INOUT GT_U32                            *rawDataLengthPtr,
    OUT GT_U32                              *rawDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagDescriptorRawGet)
{
/*
    ITERATE_DEVICES(xCat2)
    1.1.Call with
        descriptorType [CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_TTI_TO_PCL_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E ]
         rawDataLength [valid value]
         not NULL rawDataPtr
    Expected: GT_OK.
    1.2. Call with wrong descriptorType (invalid enums).
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range value rawDataLength [wrong value],
         and other value from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with wrong rawDataLengthPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call with wrong rawDataPtr [NULL].
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT   descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;
    GT_U32                               rawDataLength  = 0;
        GT_U32                               totalLength    = 0;
    GT_U32                               rawData[40] = {0};
    GT_U32                               descriptorIter = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E  | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (descriptorIter = 0; descriptorIter < CPSS_DXCH_DIAG_DESCRIPTOR_LAST_E; descriptorIter++)
        {
            /* get descriptor length */
            descriptorType = (CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT) descriptorIter;

            st = cpssDxChDiagDescriptorRawSizeGet(dev, descriptorType, &totalLength);
            if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E) &&
               ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)))
            {
                continue;
            }
            if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E) &&
               ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
            {
                continue;
            }

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, descriptorType);

                        /*
                                1.1.Call with
                                          descriptorType [CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E /
                                                                          CPSS_DXCH_DIAG_DESCRIPTOR_TTI_TO_PCL_E /
                                                                          CPSS_DXCH_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E /
                                                                          CPSS_DXCH_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E /
                                      CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E /
                                                                          CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E /
                                                                          CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E /
                                                                          CPSS_DXCH_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E /
                                                                          CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E /
                                                                          CPSS_DXCH_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E ]
                                Expected: GT_OK.
                    */
                        /* call with rawDataLength = totalLength */
                    rawDataLength = totalLength;

                        st = cpssDxChDiagDescriptorRawGet(dev, descriptorType, &rawDataLength, rawData);
            if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E) &&
               ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)))
            {
                continue;
            }
            if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E) &&
               ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
            {
                continue;
            }

            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, descriptorType, rawDataLength, rawData[0]);

                        /* call with rawDataLength = totalLength + 10 */
                    rawDataLength = totalLength + 10;

                        st = cpssDxChDiagDescriptorRawGet(dev, descriptorType, &rawDataLength, rawData);
                        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, descriptorType, rawDataLength, rawData[0]);
                }

        /*
              1.2. Call with wrong descriptorType (invalid enums).
              Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagDescriptorRawGet
                                                        (dev, descriptorType, &rawDataLength, rawData),
                            descriptorType);

        for (descriptorIter = 0; descriptorIter < CPSS_DXCH_DIAG_DESCRIPTOR_LAST_E; descriptorIter++)
        {

            /* get descriptor length */
            descriptorType = (CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT) descriptorIter;

            st = cpssDxChDiagDescriptorRawSizeGet(dev, descriptorType, &totalLength);
            if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E) &&
               ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)))
            {
                continue;
            }
            if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E) &&
               ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
            {
                continue;
            }

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, descriptorType);

                        /*
                                  1.3. Call with out of range value rawDataLength,
                                           and other value from 1.1.
                                  Expected: NOT GT_OK.
                        */
                        /* call with rawDataLength = 0 */
                        rawDataLength = 0;

            st = cpssDxChDiagDescriptorRawGet(dev, descriptorType, &rawDataLength, rawData);
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, descriptorType, rawDataLength, rawData[0]);

                        /* call with rawDataLength = totalLength - 1 */
                        rawDataLength = totalLength - 1;

            st = cpssDxChDiagDescriptorRawGet(dev, descriptorType, &rawDataLength, rawData);
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, descriptorType, rawDataLength, rawData[0]);

                }

        /* restore values */
        descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;
        rawDataLength  = 8;

        /*
              1.4. Call api with wrong rawDataLengthPtr [NULL].
              Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagDescriptorRawGet(dev, descriptorType, NULL, rawData);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rawDataLengthPtr = NULL", dev);

        /*
              1.5. Call api with wrong rawDataPtr [NULL].
              Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagDescriptorRawGet(dev, descriptorType, &rawDataLength, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rawDataPtr = NULL", dev);

    }

        /*        restore default value */
        descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;
    rawDataLength  = 8;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDescriptorRawGet(dev, descriptorType, &rawDataLength, rawData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDescriptorRawGet(dev, descriptorType, &rawDataLength, rawData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDescriptorPortGroupRawGet
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT   descriptorType,
    INOUT GT_U32                            *rawDataLengthPtr,
    OUT GT_U32                              *rawDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagDescriptorPortGroupRawGet)
{
/*
    ITERATE_DEVICES(xCat2)
    1.1.Call with
        descriptorType [CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_TTI_TO_PCL_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E ]
         rawDataLength [valid value]
         not NULL rawDataPtr
    Expected: GT_OK.
    1.2. Call with wrong descriptorType (invalid enums).
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range value rawDataLength [wrong value],
         and other value from 1.1.
    Expected: NOT GT_OK
    1.4. Call with wrong rawDataLengthPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call with wrong rawDataPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                         st = GT_OK;
    GT_U8                             dev;
        GT_U32                            portGroupId;
    GT_PORT_GROUPS_BMP        portGroupsBmp = 1;

    CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT    descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;
    GT_U32                                rawDataLength  = 0;
    GT_U32                                rawData[40] = {0};
        GT_U32                                descriptorIter = 0;
          GT_U32                                totalLength    = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* initialize port group */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);
            for (descriptorIter = 0; descriptorIter < CPSS_DXCH_DIAG_DESCRIPTOR_LAST_E; descriptorIter++)
            {
                /* get descriptor length */
                descriptorType = (CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT) descriptorIter;

                st = cpssDxChDiagDescriptorRawSizeGet(dev, descriptorType, &totalLength);
                if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E) &&
                   ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)))
                {
                    continue;
                }
                if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E) &&
                   ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
                {
                    continue;
                }

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, descriptorType);

                /*
                    1.1.Call with
                        descriptorType [CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_TTI_TO_PCL_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E ]
                    Expected: GT_OK.
                */
                /* call with rawDataLength = totalLength */
                rawDataLength = totalLength;

                st = cpssDxChDiagDescriptorPortGroupRawGet(dev, portGroupsBmp, descriptorType, &rawDataLength, rawData);
                if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E) &&
                   ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)))
                {
                    continue;
                }

                if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E) &&
                   ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
                {
                    continue;
                }

                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, descriptorType, rawDataLength, rawData[0]);

                /* call with rawDataLength = totalLength + 10 */
                rawDataLength = totalLength + 10;

                st = cpssDxChDiagDescriptorPortGroupRawGet(dev, portGroupsBmp, descriptorType, &rawDataLength, rawData);
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, descriptorType, rawDataLength, rawData[0]);
            }

            /*
                  1.2. Call with wrong descriptorType (invalid enums).
                  Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagDescriptorPortGroupRawGet
                                (dev, portGroupsBmp, descriptorType, &rawDataLength, rawData),
                                descriptorType);

            for (descriptorIter = 0; descriptorIter < CPSS_DXCH_DIAG_DESCRIPTOR_LAST_E; descriptorIter++)
            {
                /* get descriptor length */
                descriptorType = (CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT) descriptorIter;

                st = cpssDxChDiagDescriptorRawSizeGet(dev, descriptorType, &totalLength);
                if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E) &&
                   ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)))
                {
                    continue;
                }
                if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E) &&
                   ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
                {
                    continue;
                }

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, descriptorType);

                /*
                      1.3. Call with out of range value rawDataLength,
                           and other value from 1.1.
                      Expected: NOT GT_OK.
                */
                /* call with rawDataLength = 0 */
                rawDataLength = 0;

                st = cpssDxChDiagDescriptorPortGroupRawGet(dev, portGroupsBmp, descriptorType, &rawDataLength, rawData);
                UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, descriptorType, rawDataLength, rawData[0]);

                /* call with rawDataLength = totalLength - 1 */
                rawDataLength = totalLength - 1;

                st = cpssDxChDiagDescriptorPortGroupRawGet(dev, portGroupsBmp, descriptorType, &rawDataLength, rawData);
                UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, descriptorType, rawDataLength, rawData[0]);
            }

            /* restore values */
            descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;
            rawDataLength  = 8;

            /*
                1.4. Call api with wrong rawDataLengthPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagDescriptorPortGroupRawGet(dev, portGroupsBmp, descriptorType, NULL, rawData);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rawDataLengthPtr = NULL", dev);

            /*
                1.5. Call api with wrong rawDataPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagDescriptorPortGroupRawGet(dev, portGroupsBmp, descriptorType, &rawDataLength, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rawDataPtr = NULL", dev);

            /* set next active port */
            portGroupsBmp = (1 << portGroupId);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChDiagDescriptorPortGroupRawGet(dev, portGroupsBmp, descriptorType, &rawDataLength, rawData);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
    }

    /* restore values */
    portGroupsBmp = 1;
    descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;
    rawDataLength  = 8;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDescriptorPortGroupRawGet(dev, portGroupsBmp, descriptorType, &rawDataLength, rawData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDescriptorPortGroupRawGet(dev, portGroupsBmp, descriptorType, &rawDataLength, rawData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDescriptorRawSizeGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT  descriptorType,
    OUT GT_U32                              *rawDataLengthPtr
)

*/
UTF_TEST_CASE_MAC(cpssDxChDiagDescriptorRawSizeGet)
{
/*
    ITERATE_DEVICES(xCat2)
    1.1.Call with
        descriptorType [CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_TTI_TO_PCL_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E ]
         not NULL rawDataLengthPtr
    Expected: GT_OK.
    1.2. Call with wrong descriptorType (invalid enums).
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong rawDataLengthPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT   descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;
    GT_U32                               rawDataLength  = 0;
    GT_U32                               descriptorIter = 0;


    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (descriptorIter = 0; descriptorIter < CPSS_DXCH_DIAG_DESCRIPTOR_LAST_E; descriptorIter++)
        {
                descriptorType = (CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT) descriptorIter;

                /*
                    1.1.Call with
                        descriptorType [CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E /
                                        CPSS_DXCH_DIAG_DESCRIPTOR_TTI_TO_PCL_E /
                                        CPSS_DXCH_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E /
                                        CPSS_DXCH_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E /
                                        CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E /
                                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E /
                                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E /
                                        CPSS_DXCH_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E /
                                        CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E /
                                        CPSS_DXCH_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E ]
                    Expected: GT_OK.
                */
                st = cpssDxChDiagDescriptorRawSizeGet(dev, descriptorType, &rawDataLength);
                if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E) &&
                   ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)))
                {
                    continue;
                }
                if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E) &&
                   ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
                {
                    continue;
                }

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, descriptorType, rawDataLength);
            }

        /*
            1.2. Call with wrong descriptorType (invalid enums).
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagDescriptorRawSizeGet
                            (dev, descriptorType, &rawDataLength),
                            descriptorType);

        /*
            1.3. Call api with wrong rawDataLengthPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagDescriptorRawSizeGet(dev, descriptorType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rawDataLengthPtr = NULL", dev);
    }

        /* restore value*/
        descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDescriptorRawSizeGet(dev, descriptorType, &rawDataLength);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDescriptorRawSizeGet(dev, descriptorType, &rawDataLength);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDescriptorGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT   descriptorType,
    OUT CPSS_DXCH_DIAG_DESCRIPTOR_STC        *descriptorPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagDescriptorGet)
{
/*
    ITERATE_DEVICES(xCat2)
    1.1.Call with
        descriptorType [CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_TTI_TO_PCL_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E ]
         not NULL descriptorPtr
    Expected: GT_OK.
    1.2. Call with wrong descriptorType (invalid enums).
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong descriptorPtr [NULL].
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT   descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;
    CPSS_DXCH_DIAG_DESCRIPTOR_STC        descriptor     = {{0}};
    GT_U32                               descriptorIter = 0;

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
                /* set descriptionType value */
                descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;

        for (descriptorIter = 0; descriptorIter < CPSS_DXCH_DIAG_DESCRIPTOR_LAST_E; descriptorIter++)
            {
                    descriptorType = (CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT) descriptorIter;
                /*
                    1.1.Call with
                          descriptorType [CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_TTI_TO_PCL_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E /
                                          CPSS_DXCH_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E ]
                    Expected: GT_OK.
                */
                st = cpssDxChDiagDescriptorGet(dev, descriptorType, &descriptor);
                if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E) &&
                   ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)))
                {
                    continue;
                }
                if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E) &&
                   ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                    (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
                {
                    continue;
                }

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, descriptorType);
            }

        /*
            1.2. Call with wrong descriptorType (invalid enums).
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagDescriptorGet
                                                        (dev, descriptorType, &descriptor),
                                                           descriptorType);

        /*
            1.3. Call with wrong descriptorPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagDescriptorGet(dev, descriptorType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, descriptorPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDescriptorGet(dev, descriptorType, &descriptor);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDescriptorGet(dev, descriptorType, &descriptor);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDescriptorPortGroupGet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT   descriptorType,
    OUT CPSS_DXCH_DIAG_DESCRIPTOR_STC        *descriptorPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagDescriptorPortGroupGet)
{
/*
    ITERATE_DEVICES(xCat2)
    1.1.Call with
        descriptorType [CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_TTI_TO_PCL_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E /
                        CPSS_DXCH_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E ]
        rawDataLength [valid value]
        not NULL descriptorPtr
    Expected: GT_OK.
    1.2. Call with wrong descriptorType (invalid enums).
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong descriptorPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS          st = GT_OK;
    GT_U32             portGroupId;
    GT_U8              dev;
    GT_PORT_GROUPS_BMP portGroupsBmp = 1;

    CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT    descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;
    CPSS_DXCH_DIAG_DESCRIPTOR_STC                    descriptor     = {{0}};
    GT_U32                                descriptorIter = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
            UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
            | UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* initialize port group */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);
            for (descriptorIter = 0; descriptorIter < CPSS_DXCH_DIAG_DESCRIPTOR_LAST_E; descriptorIter++)
                {
                    descriptorType = (CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT) descriptorIter;
                    /*
                        1.1.Call with
                              descriptorType [CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E /
                                              CPSS_DXCH_DIAG_DESCRIPTOR_TTI_TO_PCL_E /
                                              CPSS_DXCH_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E /
                                              CPSS_DXCH_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E /
                                              CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E /
                                              CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E /
                                              CPSS_DXCH_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E /
                                              CPSS_DXCH_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E /
                                              CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E /
                                              CPSS_DXCH_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E ]
                        Expected: GT_OK.
                    */
                    st = cpssDxChDiagDescriptorPortGroupGet(dev, portGroupsBmp, descriptorType, &descriptor);
                    if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E) &&
                       ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                        (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                        (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)))
                    {
                        continue;
                    }
                    if((descriptorType == CPSS_DXCH_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E) &&
                       ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                        (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
                    {
                        continue;
                    }

                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, descriptorType);
                }

                        /*
                                1.2. Call with wrong descriptorType (invalid enums).
                                Expected: GT_BAD_PARAM.
                        */
                        UTF_ENUMS_CHECK_MAC(cpssDxChDiagDescriptorPortGroupGet
                                                                (dev, portGroupsBmp, descriptorType, &descriptor),
                                                                descriptorType);

                        /*
                                1.3. Call with wrong descriptorPtr [NULL].
                                Expected: GT_BAD_PTR.
                        */
                    st = cpssDxChDiagDescriptorPortGroupGet(dev, portGroupsBmp, descriptorType, NULL);
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, descriptorPtr = NULL", dev);

            /* set next active port */
            portGroupsBmp = (1 << portGroupId);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
                PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChDiagDescriptorPortGroupGet(dev, portGroupsBmp, descriptorType, &descriptor);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);

        }
                PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev, portGroupId)
    }

    /* restore correct values */
    portGroupsBmp = 1;
    descriptorType = CPSS_DXCH_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDescriptorPortGroupGet(dev, portGroupsBmp, descriptorType, &descriptor);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChDiagDescriptorPortGroupGet(dev, portGroupsBmp, descriptorType, &descriptor);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*-----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChDiagDescriptor suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChDiagDescriptor)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDescriptorRawGet)
        UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDescriptorPortGroupRawGet)
        UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDescriptorRawSizeGet)
        UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDescriptorGet)
        UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDescriptorPortGroupGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChDiagDescriptor)

