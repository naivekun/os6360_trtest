--
-- vpls packet
--

local uni_mac_da_flood  = "009999999999";
local uni_mac_sa_1      = "000000240001";
local uni_mac_sa_2      = "000000240002";
local uni_mac_sa_3      = "000000240003";
local uni_mac_sa_4      = "000000240004";
local uni_mac           = "000000240005";
local uni_mac_da_1      = "000000240001";
local uni_mac_da_2      = "000000240002";
local uni_mac_da_3      = "000000240003";
local uni_mac_da_4      = "000000240004";
local uni_ctag          = "81000014";--20
local uni_ctag_1        = "8100001E";--30
local uni_ctag_2        = "810000DC";--220
local uni_ctag_3        = "81000137";--311
local uni_ctag_4        = "81000139";--313
local uni_l2_payload    =
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"

local nni_mac_da        = "000000B01A13";
local nni_mac_da_2      = "000000B01A14";
local nni_mac_sa        = "000102030405";
local nni_inner_tag     = "81000014";
local nni_outer_tag     = "88A80065";
local nni_outer_tag_2   = "88A80064";
local nni_tag           = "810000DC";
local mpls_ether_type   = "8847";     --MPLS ethertype
local outer_label_1     = "77772000"; --MPLS Header: outer-label=77772
local outer_label_2     = "66662000"; --MPLS Header: outer-label=66662
local outer_label_3     = "22222000"; --MPLS Header: outer-label=22222
local outer_label_4     = "33332000"; --MPLS Header: outer-label=33332
local inner_label_1     = "55552100"; --MPLS Header: inner-label=55552
local inner_label_2     = "44442100"; --MPLS Header: inner-label=44442
local inner_label_3     = "11112100"; --MPLS Header: inner-label=11112

local mpls_header_1     = outer_label_1 .. inner_label_1; 
local mpls_header_2     = outer_label_3 .. inner_label_3; 
local mpls_header_3     = outer_label_2 .. inner_label_1; 
local mpls_header_4     = outer_label_3 .. inner_label_3; 
local mpls_header_5     = outer_label_3 .. inner_label_1; 
local mpls_header_6     = outer_label_2 .. inner_label_2; 
local mpls_header_7     = outer_label_4 .. inner_label_3; 


local nni_payload_case_2_1 =
    uni_mac_da_flood .. uni_mac_sa_1 .. nni_outer_tag_2 .. nni_inner_tag .. uni_l2_payload;

local nni_payload_case_2_2 =
    uni_mac_da_flood .. uni_mac_sa_1 .. nni_outer_tag_2 .. uni_l2_payload;

local nni_payload_case_2_3 =
    uni_mac_da_flood .. uni_mac_sa_1 .. nni_outer_tag_2 .. uni_l2_payload;

local nni_payload_case_2_4 =
    uni_mac_da_flood .. uni_mac_sa_1 .. nni_outer_tag_2 .. uni_l2_payload;

local nni_payload_case_2_5 =
    uni_mac_da_flood .. uni_mac_sa_1 .. uni_l2_payload;

local nni_payload_case_2_7 =
    uni_mac_da_flood .. uni_mac_sa_4 .. uni_l2_payload;

local nni_payload_case_3_4 =
    uni_mac_da_2 .. uni_mac .. nni_outer_tag .. nni_inner_tag .. uni_l2_payload;

local nni_payload_case_3_5 =
    uni_mac_da_3 .. uni_mac .. nni_outer_tag .. uni_l2_payload;

local nni_payload_case_3_6 =
    uni_mac_da_1 .. uni_mac .. nni_outer_tag .. nni_inner_tag .. uni_l2_payload;

local nni_payload_case_3_7 =
    uni_mac_da_2 .. uni_mac .. nni_inner_tag .. uni_l2_payload;

local nni_payload_case_3_8 =
    uni_mac_da_3 .. uni_mac .. uni_l2_payload;

local nni_c_tagged_payload = ""
    .. uni_mac_da_4 .. nni_mac_sa .. uni_l2_payload;

local nni_p_tagged_payload = ""
    .. uni_mac_da_flood .. uni_mac_sa_2 .. nni_outer_tag .. uni_l2_payload;

local nni_p_tagged_payload_2 = ""
    .. uni_mac_da_flood .. uni_mac_sa_1 .. nni_outer_tag_2 .. uni_l2_payload;

local nni_double_tagged_payload = ""
    .. uni_mac_da_flood .. uni_mac_sa_2 .. nni_outer_tag .. nni_inner_tag .. uni_l2_payload;

local nni_double_tagged_2_payload = ""
    .. uni_mac_da_flood .. uni_mac_sa_2 .. nni_outer_tag .. nni_tag .. uni_l2_payload;

local nni_double_tagged_3_payload = ""
    .. uni_mac_da_1 .. nni_mac_sa .. nni_outer_tag .. nni_inner_tag .. uni_l2_payload;

local nni_double_tagged_4_payload = ""
    .. uni_mac_da_flood .. uni_mac_sa_3 .. nni_outer_tag .. uni_ctag_1 .. uni_l2_payload;

local nni_untagged_payload = ""
    .. uni_mac_da_flood .. uni_mac_sa_2 .. uni_l2_payload;

local nni_untagged_2_payload = ""
    .. uni_mac_da_flood .. uni_mac_sa_3 .. uni_l2_payload;

------------Sent packets--------------------------------------------------------
local vpls_tagged_case_1_1 = ""
    .. uni_mac_da_flood .. uni_mac_sa_2 .. uni_ctag .. uni_l2_payload;
local vpls_untagged_case_1_2 = ""
    .. uni_mac_da_flood .. uni_mac_sa_2 .. uni_l2_payload;
local vpls_tagged_dif_vid_case_1_3 = ""
    .. uni_mac_da_flood .. uni_mac_sa_1 .. uni_ctag .. uni_l2_payload;

local vpls_mpls_c_tag_p_tag_case_2_1 = ""
    .. nni_mac_da .. nni_mac_sa .. mpls_ether_type .. mpls_header_3
    .. nni_payload_case_2_1;
local vpls_mpls_p_tag_case_2_2 = ""
    .. nni_mac_da .. nni_mac_sa .. mpls_ether_type .. mpls_header_3
    .. nni_payload_case_2_2;
local vpls_mpls_c_tag_p_tag_case_2_3 = ""
    .. nni_mac_da .. nni_mac_sa .. mpls_ether_type .. mpls_header_6
    .. nni_payload_case_2_3;
local vpls_mpls_c_tag_p_tag_case_2_4 = ""
    .. nni_mac_da .. nni_mac_sa .. mpls_ether_type .. mpls_header_1
    .. nni_payload_case_2_4;
local vpls_mpls_c_tag_case_2_5 = ""
    .. nni_mac_da .. nni_mac_sa .. mpls_ether_type .. mpls_header_3
    .. nni_payload_case_2_5;
local vpls_tagged_case_2_6 = ""
    .. uni_mac_da_flood .. uni_mac_sa_3 .. uni_ctag_1 .. uni_l2_payload;
local vpls_mpls_untagged_case_2_7 = ""
    .. nni_mac_da .. nni_mac_sa .. mpls_ether_type .. mpls_header_4
    .. nni_payload_case_2_7;
    
local vpls_tagged_case_3_1 = ""
    .. uni_mac_da_3 .. nni_mac_sa .. uni_ctag .. uni_l2_payload;
local vpls_tagged_case_3_2 = ""
    .. uni_mac_da_1 .. nni_mac_sa .. uni_ctag .. uni_l2_payload;
local vpls_tagged_case_3_3 = ""
    .. uni_mac_da_4 .. nni_mac_sa .. uni_ctag .. uni_l2_payload;
local vpls_mpls_c_tag_p_tag_case_3_4 = ""
    .. nni_mac_da .. nni_mac_sa .. mpls_ether_type .. mpls_header_3
    .. nni_payload_case_3_4;
local vpls_mpls_p_tag_case_3_5 = ""
    .. nni_mac_da .. nni_mac_sa .. mpls_ether_type .. mpls_header_3
    .. nni_payload_case_3_5;
local vpls_mpls_p_tag_case_3_6 = ""
    .. nni_mac_da_2 .. nni_mac_sa .. mpls_ether_type .. mpls_header_4
    .. nni_payload_case_3_6;
local vpls_mpls_c_tag_case_3_7 = ""
    .. nni_mac_da_2 .. nni_mac_sa .. mpls_ether_type .. mpls_header_4
    .. nni_payload_case_3_7;
local vpls_mpls_c_tag_case_3_8 = ""
    .. nni_mac_da_2 .. nni_mac_sa .. mpls_ether_type .. mpls_header_4
    .. nni_payload_case_3_8;
    
------------Expected packets----------------------------------------------------
local vpls_expected_tagged = ""
    .. uni_mac_da_flood .. uni_mac_sa_2 .. uni_ctag .. uni_l2_payload;
    
local vpls_expected_tagged_1 = ""
    .. uni_mac_da_flood .. uni_mac_sa_2 .. uni_ctag_2 .. uni_l2_payload;
    
local vpls_expected_tagged_2 = ""
    .. uni_mac_da_flood .. uni_mac_sa_1 .. uni_ctag_3 .. uni_l2_payload;
    
local vpls_expected_tagged_3 = ""
    .. uni_mac_da_flood .. uni_mac_sa_1 .. uni_ctag .. uni_l2_payload;
    
local vpls_expected_tagged_4 = ""
    .. uni_mac_da_3 .. nni_mac_sa .. uni_ctag .. uni_l2_payload;
    
local vpls_expected_tagged_5 = ""
    .. uni_mac_da_3 .. uni_mac .. uni_ctag_3 .. uni_l2_payload;
    
local vpls_expected_tagged_6 = ""
    .. uni_mac_da_3 .. uni_mac .. uni_ctag_4 .. uni_l2_payload;
    
local vpls_expected_tagged_7 = ""
    .. uni_mac_da_flood .. uni_mac_sa_4 .. uni_ctag_4 .. uni_l2_payload;
 
local vpls_expected_untagged = ""
    .. uni_mac_da_2 .. uni_mac .. uni_l2_payload;
 
local vpls_expected_untagged_1 = ""
    .. uni_mac_da_flood .. uni_mac_sa_1 .. uni_l2_payload;
 
local vpls_expected_untagged_2 = ""
    .. uni_mac_da_2 .. uni_mac .. uni_l2_payload;
 
local vpls_expected_untagged_3 = ""
    .. uni_mac_da_flood .. uni_mac_sa_3 .. uni_l2_payload;
 
local vpls_expected_untagged_4 = ""
    .. uni_mac_da_flood .. uni_mac_sa_4 .. uni_l2_payload;

local vpls_expected_mpls_c_tag_p_tag = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header_1
    .. nni_double_tagged_payload;

local vpls_expected_mpls_c_tag_p_tag_2 = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header_1
    .. nni_double_tagged_2_payload;

local vpls_expected_mpls_c_tag_p_tag_3 = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header_1
    .. nni_double_tagged_3_payload;

local vpls_expected_mpls_c_tag_p_tag_4 = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header_1
    .. nni_double_tagged_4_payload;

local vpls_expected_mpls_p_tag = ""
    .. nni_mac_da_2 .. nni_mac_sa
    .. mpls_ether_type .. mpls_header_2
    .. nni_p_tagged_payload;

local vpls_expected_mpls_p_tag_2 = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header_1
    .. nni_p_tagged_payload_2;

local vpls_expected_mpls_c_tag = ""
    .. nni_mac_da_2 .. nni_mac_sa
    .. mpls_ether_type .. mpls_header_7
    .. nni_c_tagged_payload;

local vpls_expected_mpls_untagged = ""
    .. nni_mac_da_2 .. nni_mac_sa
    .. mpls_ether_type .. mpls_header_7
    .. nni_untagged_payload;

local vpls_expected_mpls_untagged_2 = ""
    .. nni_mac_da_2 .. nni_mac_sa
    .. mpls_ether_type .. mpls_header_7
    .. nni_untagged_2_payload;


return
{
    tagged_case_1_1               = string.lower(vpls_tagged_case_1_1),       
    untagged_case_1_2             = string.lower(vpls_untagged_case_1_2),      
    tagged_dif_vid_case_1_3       = string.lower(vpls_tagged_dif_vid_case_1_3),
    mpls_c_tag_p_tag_case_2_1     = string.lower(vpls_mpls_c_tag_p_tag_case_2_1),
    mpls_p_tag_case_2_2           = string.lower(vpls_mpls_p_tag_case_2_2),
    mpls_c_tag_p_tag_case_2_3     = string.lower(vpls_mpls_c_tag_p_tag_case_2_3),
    mpls_c_tag_p_tag_case_2_4     = string.lower(vpls_mpls_c_tag_p_tag_case_2_4),
    mpls_c_tag_case_2_5           = string.lower(vpls_mpls_c_tag_case_2_5),
    tagged_case_2_6               = string.lower(vpls_tagged_case_2_6),
    mpls_untagged_case_2_7        = string.lower(vpls_mpls_untagged_case_2_7),
    tagged_case_3_1               = string.lower(vpls_tagged_case_3_1),
    tagged_case_3_2               = string.lower(vpls_tagged_case_3_2),
    tagged_case_3_3               = string.lower(vpls_tagged_case_3_3),
    mpls_c_tag_p_tag_case_3_4     = string.lower(vpls_mpls_c_tag_p_tag_case_3_4),
    mpls_p_tag_case_3_5           = string.lower(vpls_mpls_p_tag_case_3_5),
    mpls_p_tag_case_3_6           = string.lower(vpls_mpls_p_tag_case_3_6),
    mpls_c_tag_case_3_7           = string.lower(vpls_mpls_c_tag_case_3_7),
    mpls_c_tag_case_3_8           = string.lower(vpls_mpls_c_tag_case_3_8),
    expected_tagged               = string.lower(vpls_expected_tagged),   
    expected_tagged_1             = string.lower(vpls_expected_tagged_1),      
    expected_tagged_2             = string.lower(vpls_expected_tagged_2),      
    expected_tagged_3             = string.lower(vpls_expected_tagged_3),      
    expected_tagged_4             = string.lower(vpls_expected_tagged_4),      
    expected_tagged_5             = string.lower(vpls_expected_tagged_5),       
    expected_tagged_6             = string.lower(vpls_expected_tagged_6),        
    expected_tagged_7             = string.lower(vpls_expected_tagged_7),        
    expected_untagged             = string.lower(vpls_expected_untagged),    
    expected_untagged_1           = string.lower(vpls_expected_untagged_1),    
    expected_untagged_2           = string.lower(vpls_expected_untagged_2),    
    expected_untagged_3           = string.lower(vpls_expected_untagged_3),    
    expected_untagged_4           = string.lower(vpls_expected_untagged_4),    
    expected_mpls_c_tag           = string.lower(vpls_expected_mpls_c_tag),
    expected_mpls_p_tag           = string.lower(vpls_expected_mpls_p_tag),
    expected_mpls_p_tag_2         = string.lower(vpls_expected_mpls_p_tag_2),
    expected_mpls_c_tag_p_tag     = string.lower(vpls_expected_mpls_c_tag_p_tag),
    expected_mpls_c_tag_p_tag_2   = string.lower(vpls_expected_mpls_c_tag_p_tag_2),
    expected_mpls_c_tag_p_tag_3   = string.lower(vpls_expected_mpls_c_tag_p_tag_3),
    expected_mpls_c_tag_p_tag_4   = string.lower(vpls_expected_mpls_c_tag_p_tag_4),
    expected_mpls_untagged        = string.lower(vpls_expected_mpls_untagged),
    expected_mpls_untagged_2      = string.lower(vpls_expected_mpls_untagged_2)
}
