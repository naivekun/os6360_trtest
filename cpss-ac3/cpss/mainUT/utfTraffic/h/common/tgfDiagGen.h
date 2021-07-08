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
* @file tgfDiagGen.h
*
* @brief Diag Descriptor
*
* @version   4
********************************************************************************
*/
#ifndef __tgfDiagGenh
#define __tgfDiagGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

typedef enum
{
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_APPSPECIFICCPUCODE_E,          /* AppSpecificCPUCode */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ARPPTR_E,                      /* ARPPtr */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_BADIPXBC_E,                    /* BadIPxBC */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_BANK_E,                        /* Bank */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_BILLINGEN_E,                   /* BillingEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_BUFREUSE_E,                    /* BufReuse */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_BYPASSBRIDGE_DUP_E,            /* BypassBridge_Dup */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_BYPASSBRIDGE_E,                /* BypassBridge */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_BYPASSINGRESSPIPE_E,           /* BypassIngressPipe */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_BYTECOUNT_E,                   /* ByteCount */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CFI0_E,                        /* CFI0 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CFI1_E,                        /* CFI1 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CFI_E,                         /* CFI */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CFMPKT_E,                      /* CFMPkt */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CONGESTEDPORTNUMBER_E,         /* CongestedPortNumber */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CONGESTEDQUEUENUMBER_E,        /* CongestedQueueNumber */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CONTROLTC_E,                   /* ControlTC */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_COPYTTL_E,                     /* CopyTTL */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CPIDMATCH_E,                   /* CPIDMatch */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CPU2CPUMIRROR_E,               /* CPU2CPUMirror */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CPUCODE_E,                     /* CPUCode */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CPUMAILTONEIGBORCPU_E,         /* CPUMailToNeigborCPU */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CPUTRGDEV_E,                   /* CPUTrgDev */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DECTTL_E,                      /* DecTTL */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DIP_0_31_E,                    /* DIP bits 0..31 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DIP_32_63_E,                   /* DIP bits 32..63 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DIP_64_95_E,                   /* DIP bits 64..95 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DIP_96_127_E,                  /* DIP bits 96..127 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DIP_E,                         /* DIP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DISABLEIPCL1_FOR_ROUTEDPACKETS_E,  /* DisableIPCL1_for_RoutedPackets */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DOROUTEHA_E,                   /* DoRouteHa */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DOROUTE_E,                     /* DoRoute */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DP_E,                          /* DP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DROPCODE_E,                    /* DropCode */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DROPONSOURCE_E,                /* DropOnSource */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DROPPKT_E,                     /* DropPkt */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_DSCP_E,                        /* DSCP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_EGRESSFILTEREN_E,              /* EgressFilterEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_EGRESSFILTERREGISTERED_E,      /* EgressFilterRegistered */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_EGRESSTRGDEV_E,                /* EgressTrgDev */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_EGRESSTRGPORT_DUP_2_0_E,       /* EgressTrgPort_dup_2_0 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_EGRESSTRGPORT_E,               /* EgressTrgPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ENABLEBRIDGELOGFLOW_E,         /* EnableBridgeLogFlow */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ENABLEBRIDGE_E,                /* EnableBridge */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ENABLEDSATAGMANIPULATION_E,    /* EnableDsaTagManipulation */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ENABLEROUTERLOGFLOW_E,         /* EnableRouterLogFlow */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ENABLESOURCELOCALFILTERING_E,  /* EnableSourceLocalFiltering */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ENABLE_IPCL0_LOOKUP_E,         /* Enable_IPCL0_lookup */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ENABLE_POLICY_E,               /* Enable_Policy */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ETHERTYPE_E,                   /* EtherType */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_EXCLUDEDDEVISLOCAL_E,          /* ExcludedDevIsLocal */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_EXCLUDEDPORT_E,                /* ExcludedPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_EXCLUDEDTRUNK_E,               /* ExcludedTrunk */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_EXCLUDEISTRUNK_E,              /* ExcludeIsTrunk */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_EXP_E,                         /* EXP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_FIRSTBUFFER_E,                 /* FirstBuffer */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_FLOODMLLPOINTER_E,             /* FloodMLLPointer */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_FLOWID_E,                      /* FlowID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_FLOWSUBTEMPLATE_E,             /* FlowSubTemplate */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_FORCE_IPCL1_CONFIGURATIONS_E,  /* Force_IPCL1_configurations */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_FRAGMENTED_E,                  /* Fragmented */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_FROMCPUDP_E,                   /* FromCPUDP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_FROMCPUTC_E,                   /* FromCPUTC */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_HASHMASKINDEX_E,               /* HashMaskIndex */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IGMPNONQUERY_E,                /* IGMPNonQuery */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IGMPQUERY_E,                   /* IGMPQuery */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_INLIF_MODE_E,                  /* InLIF_Mode */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_INNERISLLCNONSNAP_E,           /* InnerIsLLCNonSnap */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_INNERL3OFFSET1_DUP_E,          /* InnerL3Offset1_dup */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_INNERL3OFFSET_E,               /* InnerL3Offset */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_INNERPKTTYPE_E,                /* InnerPktType */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPCL_PROFILE_INDEX_E,          /* IPCL_Profile_Index */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPLEGAL_E,                     /* IPLegal */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPMGMNTEN_E,                   /* IPMgmntEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPMODIFYDSCP_E,                /* IPModifyDSCP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPMODIFYUP_E,                  /* IPModifyUP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPM_E,                         /* IPM */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPQOSPROFILE_E,                /* IPQoSProfile */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV4MULTICASTEN_E,             /* Ipv4MulticastEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV4OPTIONFIELD_E,             /* IPv4OptionField */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV4RIPV1_E,                   /* IPv4RIPv1 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV4UNICASTEN_E,               /* Ipv4UnicastEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV4_BAD_GRE_HEADER_E,         /* IPv4_Bad_GRE_Header */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV4_DF_E,                     /* IPv4_DF */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV6FLOWLABEL_E,               /* IPv6FlowLabel */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV6HBHEXT_E,                  /* IPv6HBHExt */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV6HBHOPTIONS_E,              /* IPv6HBHOptions */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV6ICMPTYPE_E,                /* Ipv6ICMPType */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV6ICMP_E,                    /* IPv6ICMP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV6MULTICASTEN_E,             /* Ipv6MulticastEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV6NONHBHEXT_E,               /* IPv6NonHBHExt */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV6SITEID_E,                  /* IPv6SiteID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV6UNICASTEN_E,               /* Ipv6UnicastEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPV6_EH_E,                     /* IPv6_EH */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPXHEADERTOTALLENGTH_E,        /* IPxHeaderTotalLength */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPXMACMCMISMATCH_E,            /* IPxMACMcMismatch */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPXMCLINKLOCALPROT_E,          /* IPxMcLinkLocalProt */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IPXPROTOCOL_E,                 /* IPxProtocol */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IP_FRAGMENTED_E,               /* IP_Fragmented */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_IP_HEADER_INFO_E,              /* IP_Header_Info */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISARPREPLY_E,                  /* IsARPReply */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISARP_E,                       /* IsARP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISBC_E,                        /* IsBC */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISIPV4_E,                      /* IsIPv4 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISIPV4_FORKEY_E,               /* IsIPv4_ForKey */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISIPV6LINKLOCAL_E,             /* IsIPv6LinkLocal */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISIPV6MLD_E,                   /* IsIPv6MLD */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISIPV6_E,                      /* IsIPv6 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISIPV6_FORKEY_E,               /* IsIPv6_ForKey */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISIP_E,                        /* IsIP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISIP_FORKEY_E,                 /* IsIP_ForKey */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISMPLS_E,                      /* IsMPLS */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISND_E,                        /* IsND */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISPTP_E,                       /* IsPTP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ISSYN_E,                       /* IsSYN */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_L2ECHO_E,                      /* L2Echo */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_L2ENCAPSULATION_E,             /* L2Encapsulation */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_L2_VALID_E,                    /* L2_Valid */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_L3OFFSETINVALID_E,             /* L3OffsetInvalid */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_L3TOTALOFFSET_OR_INNERHEADERTOTALOFFSET_E,  /* L3TotalOffset_or_InnerHeaderTotalOffset */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_L4BYTE13_E,                    /* L4Byte13 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_L4BYTES3_0_E,                  /* L4Bytes3_0 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_L4OFFSETINVALID_E,             /* L4OffsetInvalid */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_L4VALID_E,                     /* L4Valid */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LLT_E,                         /* LLT */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LOCALDEVICESRCTRUNKID_E,       /* LocalDeviceSrcTrunkID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LOCALDEVSRCPORT_E,             /* LocalDevSrcPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LOOKUP0_PCL_MODE_E,            /* Lookup0_PCL_mode */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LOOKUP0_USEINDEXFROMDESC_E,    /* Lookup0_UseIndexFromDesc */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LOOKUP1_PCL_MODE_E,            /* Lookup1_PCL_mode */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LOOKUP1_USEINDEXFROMDESC_E,    /* Lookup1_UseIndexFromDesc */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LOOKUP2_PCL_MODE_E,            /* Lookup2_PCL_mode */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LOOKUP2_USEINDEXFROMDESC_E,    /* Lookup2_UseIndexFromDesc */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MAC2ME_E,                      /* mac2me */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MACDATYPE_E,                   /* MacDaType */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MACDA_0_31_E,                  /* MACDA bits 0..31 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MACDA_32_48_E,                 /* MACDA bits 32..48 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MACSAARPSAMISMATCH_E,          /* MacSAArpSAMismatch */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MACSA_0_31_E,                  /* MACSA bits 0..31 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MACSA_32_48_E,                 /* MACSA bits 32..48 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MAILBOXTONEIGHBORCPU_E,        /* MailBoxToNeighborCPU */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MARVELLTAGGEDEXTENDED_E,       /* MarvellTaggedExtended */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MARVELLTAGGED_E,               /* MarvellTagged */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MCRPFFAIL_E,                   /* MCRPFFail */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MDB_E,                         /* MDB */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_METERINGEN_E,                  /* MeteringEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MLLEN_E,                       /* MLLEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MLLLAST_E,                     /* MLLLast */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MLLPTR_E,                      /* MLLPtr */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MLL_E,                         /* MLL */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MLL_POINTER_E,                 /* MLL_Pointer */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MODIFYDP_E,                    /* ModifyDP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MODIFYDSCP_E,                  /* ModifyDSCP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MODIFYDSCP_EXP_E,              /* ModifyDSCP_EXP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MODIFYEXP_E,                   /* ModifyEXP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MODIFYTC_E,                    /* ModifyTC */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MODIFYUP_E,                    /* ModifyUP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MPLSCMD_E,                     /* MPLSCmd */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MPLSLABEL_E,                   /* MPLSLabel */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MPLS_LABEL0_E,                 /* MPLS_Label0 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MPLS_LABEL1_E,                 /* MPLS_Label1 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MPLS_LABEL2_E,                 /* MPLS_Label2 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MPLS_OUTEREXP_E,               /* MPLS_OuterEXP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MPLS_OUTERLABEL_E,             /* MPLS_OuterLabel */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_MPLS_OUTERS_BIT_E,             /* MPLS_OuterS_Bit */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_NESTEDVLANACCESSPORT_E,        /* NestedVlanAccessPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_NHEMCRPFFAILCMD_E,             /* NHEMCRPFFailCmd */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_NUM_OF_INGRESSTAGS_E,          /* Num_of_IngressTags */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ORIGISTRUNK_E,                 /* OrigIsTrunk */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ORIGQCNPRIO_E,                 /* OrigQCNPrio */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ORIGSRCDEV_E,                  /* OrigSrcDev */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ORIGSRCPORT_E,                 /* OrigSrcPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ORIGSRCTRUNK_E,                /* OrigSrcTrunk */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ORIGVID_E,                     /* OrigVID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_OUTERISLLCNONSNAP_E,           /* OuterIsLLCNonSnap */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_OUTERL3TOTALOFFSET_E,          /* OuterL3TotalOffset */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_OUTERSRCTAG_E,                 /* OuterSrcTag */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_OUTGOINGMTAGCMD_E,             /* OutGoingMtagCmd */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_OUTLIF_TYPE_E,                 /* OutLIF_Type */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PACKETCMD_E,                   /* PacketCmd */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PACKETISCNTAGGED_E,            /* PacketIsCNTagged */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PACKETISLLCSNAP_E,             /* PacketIsLLCSnap */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PACKETREJECT_E,                /* PacketReject */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PACKETTOS_E,                   /* PacketTOS */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PACKETTYPE_E,                  /* PacketType */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PACKETUP_E,                    /* PacketUP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PCL1_IPV4_KEY_TYPE_E,          /* PCL1_IPv4_Key_Type */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PCL1_IPV6_KEY_TYPE_E,          /* PCL1_IPv6_Key_Type */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PCL1_LOOKUP_MODE_E,            /* PCL1_Lookup_mode */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PCL1_LOOKUP_TYPE_E,            /* PCL1_Lookup_Type */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PCL1_NONIP_KEY_TYPE_E,         /* PCL1_NonIP_Key_Type */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PCL1_PCL_ID_E,                 /* PCL1_PCL_ID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PCLASSIGNEDSSTID_E,            /* PCLAssignedSSTID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PKTCMD_E,                      /* PktCmd */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PKTHASH_E,                     /* PktHash */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PKTISLOOPED_E,                 /* PktIsLooped */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PKTTYPEFORKEY_E,               /* PktTypeForKey */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_POLICEREN_E,                   /* PolicerEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_POLICERPTR_E,                  /* PolicerPtr */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_POLICYBASEDROUTED_E,           /* PolicyBasedRouted */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_POLICYRTTINDEX_E,              /* PolicyRTTIndex */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PORTSPEEDINDEX_OR_ORIGQCNPRIO_E,  /* PortSpeedIndex_or_OrigQCNPrio */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PRIOTAGGED_E,                  /* PrioTagged */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PSEUDOWIREID_E,                /* PseudoWireID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_PTPTRIGGERTYPE_E,              /* PTPTriggerType */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_QCNPACKET_E,                   /* QCNPacket */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_QCNQDELTA_E,                   /* QCNqDelta */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_QCNQFB_E,                      /* QCNqFb */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_QCNQOFFSET_E,                  /* QCNqOffset */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_QCNQSATURATED_E,               /* QCNqSaturated */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_QCN_E,                         /* QCN */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_QINQ_E,                        /* QinQ */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_QOSPROFILEPRECEDENCE_E,        /* QosProfilePrecedence */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_QOSPROFILE_E,                  /* QosProfile */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RECALCCRC_E,                   /* RecalcCRC */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_REDIRECTCMD_E,                 /* RedirectCmd */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RESERVED_0_31_E,               /* Reserved bits 0..31 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RESERVED_32_63_E,              /* Reserved bits 32..63 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RESERVED_64_65_E,              /* Reserved bits 64..65 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RESERVED_64_90_E,              /* Reserved bits 64..90 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RESERVED_E,                    /* Reserved */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGARPINDEX_E,                  /* RGARPIndex */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGBYPASSTTLANDOPTIONCHECKS_E,  /* RGBypassTTLandOptionChecks */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGDECTTL_E,                    /* RGDecTTL */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGEGRESSINTERFACE_E,           /* RGEgressInterface */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGFWDCMD_E,                    /* RGFwdCmd */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGICMP_REDIRECTCHECKEN_E,      /* RGICMP_RedirectCheckEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGMIRRORTOINGRESSANALYZERPORT_E,  /* RGMirrorToIngressAnalyzerPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGMODIFYDSCP_E,                /* RGModifyDSCP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGMODIFYUP_E,                  /* RGModifyUP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGNHEVID_E,                    /* RGNHEVID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGPCEROUTEENTRY_E,             /* RGPCERouteEntry */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGQOSPROFILE_E,                /* RGQoSProfile */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGQOS_PRECEDENCE_E,            /* RGQoS_Precedence */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RGQOS_PROFILEMARKINGEN_E,      /* RGQoS_ProfileMarkingEn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ROUTED_E,                      /* Routed */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ROUTETOENTRY_E,                /* RouteToEntry */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RSRVDCOPY_E,                   /* RsrvdCopy */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_RXSNIFF_E,                     /* RxSniff */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SIP_0_31_E,                    /* SIP bits 0..31 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SIP_32_63_E,                   /* SIP bits 32..63 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SIP_64_95_E,                   /* SIP bits 64..95 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SIP_96_127_E,                  /* SIP bits 96..127 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SNIFFTRGDEV_E,                 /* SniffTrgDev */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SNIFFTRGPORT_E,                /* SniffTrgPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SNIFFVIDX_E,                   /* SniffVIDX */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SOLICITATIONMULTICASTMESSAGE_E,  /* SolicitationMulticastMessage */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SRCDEVISOWN_E,                 /* SrcDevIsOwn */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SRCDEV_E,                      /* SrcDev */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SRCISLOOPBACK_E,               /* SrcIsLoopback */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SRCPORT_E,                     /* SrcPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SRCTAGGED_DUP_E,               /* SrcTagged_dup */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SRCTAGGED_E,                   /* SrcTagged */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SRCTCPUDPPORTIS0_E,            /* SrcTCPUDPPortIs0 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SRCTRG_E,                      /* SrcTrg */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SRC_IS_PE_E,                   /* Src_is_PE */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SST_ID_E,                      /* SST_ID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SYNWITHDATA_E,                 /* SYNWithData */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TARGETISTRUNK_E,               /* TargetIsTrunk */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TCPFLAGS_E,                    /* TCPFlags */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TCP_UDPDESTPORT_E,             /* TCP_UDPDestPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TCP_UDP_SRCPORT_E,             /* TCP_UDP_SrcPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TC_E,                          /* TC */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TRGDEVICE_E,                   /* TrgDevice */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TRGDEVICE_VIRTUALSRCDEV_E,     /* TrgDevice_VirtualSrcDev */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TRGDEV_E,                      /* TrgDev */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TRGISTRUNK_E,                  /* TrgIsTrunk */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TRGPORT_E,                     /* TrgPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TRGPORT_VIRTUALSRCPORT_E,      /* TrgPort_VirtualSrcPort */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TRGTAGGED_DUP_E,               /* TrgTagged_dup */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TRGTAGGED_E,                   /* TrgTagged */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TRGTRUNKID_E,                  /* TrgTrunkID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TRUNCATED_E,                   /* Truncated */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TS_PASSENGER_TYPE_E,           /* TS_Passenger_Type */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TTL_E,                         /* TTL */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TUNNELPTR_E,                   /* TunnelPtr */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TUNNELSTART_E,                 /* TunnelStart */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TUNNELTERMINATED_E,            /* TunnelTerminated */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TUNNELTYPE_E,                  /* TunnelType */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_TXSAMPLED_E,                   /* TxSampled */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB0_E,                        /* UDB0 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB10_E,                       /* UDB10 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB11_E,                       /* UDB11 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB12_E,                       /* UDB12 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB13_E,                       /* UDB13 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB14_E,                       /* UDB14 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB15_E,                       /* UDB15 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB16_E,                       /* UDB16 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB17_E,                       /* UDB17 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB18_E,                       /* UDB18 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB19_E,                       /* UDB19 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB1_E,                        /* UDB1 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB20_E,                       /* UDB20 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB21_E,                       /* UDB21 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB22_E,                       /* UDB22 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB2_E,                        /* UDB2 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB3_E,                        /* UDB3 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB4_E,                        /* UDB4 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB5_E,                        /* UDB5 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB6_E,                        /* UDB6 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB7_E,                        /* UDB7 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB8_E,                        /* UDB8 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB9_E,                        /* UDB9 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UDB_BYTESVALID_E,              /* UDB_BytesValid */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UP1_E,                         /* UP1 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_UP_E,                          /* UP */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_USEINTERFACEBASEDINDEX_E,      /* UseInterfaceBasedIndex */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_USEVIDX_E,                     /* UseVIDX */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_VID1_E,                        /* VID1 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_VIDMODIFIED_E,                 /* VIDModified */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_VIDX_E,                        /* VIDX */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_VID_E,                         /* VID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_VID_OR_INLIF_ID_E,             /* VID_or_InLIF_ID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_VID_PRECEDENCE_E,              /* VID_Precedence */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_VIRTUALINTERFACE_E,            /* VirtualInterface */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_VLAN1_EXIST_E,                 /* VLAN1_Exist */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_VRF_ID_E,                      /* VRF_ID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_VR_ID_E,                       /* VR_ID */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_SRCOWNDEV0_E,                  /* SrcOwnDev0 */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_CONGESTEDQUEUEFILLLEVEL_E,     /* CongestedQueueFillLevel */
    PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LAST_E

}PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_ENT;

/**
* @enum PRV_TGF_DIAG_DESCRIPTOR_TYPE_ENT
 *
 * @brief This enum defines descriptor types.
*/
typedef enum{

    /** Write DMA to TTI descriptor. */
    PRV_TGF_DIAG_DESCRIPTOR_WR_DMA_TO_TTI_E,

    /** TTI to PCL descriptor. */
    PRV_TGF_DIAG_DESCRIPTOR_TTI_TO_PCL_E,

    /** PCL to bridge descriptor. */
    PRV_TGF_DIAG_DESCRIPTOR_PCL_TO_BRIDGE_E,

    /** Bridge to Router descriptor. */
    PRV_TGF_DIAG_DESCRIPTOR_BRIDGE_TO_ROUTER_E,

    /** Router to ingress policer descriptor. */
    PRV_TGF_DIAG_DESCRIPTOR_ROUTER_TO_INGRESS_POLICER_E,

    /** Ingress policer0 to ingress policer1 descriptor. */
    PRV_TGF_DIAG_DESCRIPTOR_INGRESS_POLICER0_TO_INGRESS_POLICER1_E,

    /** Ingress policer1 to preegress descriptor. */
    PRV_TGF_DIAG_DESCRIPTOR_INGRESS_POLICER1_TO_PRE_EGRESS_E,

    /** Preegress to TxQ descriptor. */
    PRV_TGF_DIAG_DESCRIPTOR_PRE_EGRESS_TO_TXQ_E,

    /** @brief TxQ to
     *  Header Alteration descriptor.
     */
    PRV_TGF_DIAG_DESCRIPTOR_TXQ_TO_HEADER_ALTERATION_E,

    /** @brief egress policer
     *  to egress PCL descriptor.
     */
    PRV_TGF_DIAG_DESCRIPTOR_EGRESS_POLICER_TO_EGRESS_PCL_E,

    /** last descriptor. */
    PRV_TGF_DIAG_DESCRIPTOR_LAST_E

} PRV_TGF_DIAG_DESCRIPTOR_TYPE_ENT;

/**
* @struct PRV_TGF_DIAG_DESCRIPTOR_STC
 *
 * @brief used to describe descriptor information.
*/
typedef struct{

    GT_U32 fieldValue[PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LAST_E];

    GT_BOOL fieldValueValid[PRV_TGF_DIAG_DESCRIPTOR_FIELD_TYPE_LAST_E];

} PRV_TGF_DIAG_DESCRIPTOR_STC;



/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfDiagDescriptorGet function
* @endinternal
*
* @brief   Get the descriptor information.
*
* @note   APPLICABLE DEVICES:      xCat2, Lion, Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] descriptorType           - descriptor type
*
* @param[out] descriptorPtr            - pointer to descriptor information.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number, portGroupsBmp,
*                                       descriptorType.
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDiagDescriptorGet
(
    IN  GT_U8                                devNum,
    IN  PRV_TGF_DIAG_DESCRIPTOR_TYPE_ENT   descriptorType,
    OUT PRV_TGF_DIAG_DESCRIPTOR_STC        *descriptorPtr
);

/**
* @internal prvTgfDiagDescriptorPortGroupGet function
* @endinternal
*
* @brief   Get the descriptor information.
*
* @note   APPLICABLE DEVICES:      xCat2, Lion, Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] descriptorType           - descriptor type
*
* @param[out] descriptorPtr            - pointer to descriptor information.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number, portGroupsBmp,
*                                       descriptorType.
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDiagDescriptorPortGroupGet
(
    IN  GT_U8                                devNum,
    IN  GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN  PRV_TGF_DIAG_DESCRIPTOR_TYPE_ENT     descriptorType,
    OUT PRV_TGF_DIAG_DESCRIPTOR_STC          *descriptorPtr
);

/**
* @internal prvTgfDiagDescriptorRawGet function
* @endinternal
*
* @brief   Get the descriptor information in raw (HW) format
*
* @note   APPLICABLE DEVICES:      xCat2, Lion, Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] descriptorType           - descriptor type
* @param[in,out] rawDataLengthPtr         - pointer to descriptor raw (HW) data length (bytes).
* @param[in,out] rawDataLengthPtr         - pointer to actual descriptor raw (HW) data length (bytes).
*
* @param[out] rawDataPtr               - pointer to descriptor data in raw (HW) format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number, portGroupsBmp,
*                                       descriptorType.
* @retval GT_BAD_SIZE              - on insufficient descriptor raw (HW) data size.
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDiagDescriptorRawGet 
(
    IN GT_U8                                devNum,
    IN PRV_TGF_DIAG_DESCRIPTOR_TYPE_ENT   descriptorType,
    INOUT GT_U32                            *rawDataLengthPtr,  
    OUT GT_U32                              *rawDataPtr  
);

/**
* @internal prvTgfDiagDescriptorRawSizeGet function
* @endinternal
*
* @brief   Get the descriptor raw (HW) size.
*
* @note   APPLICABLE DEVICES:      xCat2, Lion, Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] descriptorType           - descriptor type
*
* @param[out] rawDataLengthPtr         - pointer to actual descriptor raw (HW) data length (bytes).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number, portGroupsBmp,
*                                       descriptorType.
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDiagDescriptorRawSizeGet 
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_DIAG_DESCRIPTOR_TYPE_ENT  descriptorType,
    OUT GT_U32                              *rawDataLengthPtr 
);

/**
* @internal prvTgfDiagDescriptorPortGroupRawGet function
* @endinternal
*
* @brief   Get the descriptor information in raw (HW) format
*
* @note   APPLICABLE DEVICES:      xCat2, Lion, Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] descriptorType           - descriptor type
* @param[in,out] rawDataLengthPtr         - pointer to descriptor raw (HW) data length (bytes).
* @param[in,out] rawDataLengthPtr         - pointer to actual descriptor raw (HW) data length (bytes).
*
* @param[out] rawDataPtr               - pointer to descriptor data in raw (HW) format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad device number, portGroupsBmp,
*                                       descriptorType.
* @retval GT_BAD_SIZE              - on insufficient descriptor raw (HW) data size.
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfDiagDescriptorPortGroupRawGet 
(
    IN GT_U8                                devNum,
    IN GT_PORT_GROUPS_BMP                   portGroupsBmp,
    IN PRV_TGF_DIAG_DESCRIPTOR_TYPE_ENT     descriptorType,
    INOUT GT_U32                            *rawDataLengthPtr,  
    OUT GT_U32                              *rawDataPtr  
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfDiagGenh */


