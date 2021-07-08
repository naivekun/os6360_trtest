--
-- vpls packet
--

local uni_mac_da        = "000000240004";
local uni_mac_sa        = "000000240002";
local uni_ctag          = "81000100";
local uni_ctag_2        = "8100012C";
local uni_l2_payload    =
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"

------------Expected packets----------------------------------------------------
local vpls_expected_tagged = 
    uni_mac_da .. uni_mac_sa .. uni_ctag .. uni_l2_payload;

local vpls_expected_untagged = 
    uni_mac_da .. uni_mac_sa .. uni_l2_payload;

local vpls_expected_dif_tagged = 
    uni_mac_da .. uni_mac_sa .. uni_ctag_2 .. uni_l2_payload;

local nni_mac_da        = "000000B01A11";
local nni_mac_sa        = "000102030405";
local nni_outer_tag     = "88A80065";
local nni_tag           = "810000C8";
local mpls_ether_type   = "8847";             --MPLS ethertype
local mpls_header       = "6666200055552100"; --MPLS Header: outer-label=66662, inner-label=55552

local nni_inner_tag     = uni_ctag;

local nni_untagged_payload =
    uni_mac_da .. uni_mac_sa .. uni_l2_payload;

local nni_single_tagged_payload =
    uni_mac_da .. uni_mac_sa .. nni_outer_tag .. uni_l2_payload;

local nni_single_dif_tagged_payload =
    uni_mac_da .. uni_mac_sa .. nni_tag .. uni_l2_payload;

local nni_double_tagged_payload =
    uni_mac_da .. uni_mac_sa .. nni_outer_tag .. nni_inner_tag .. uni_l2_payload;


------------Sent packets--------------------------------------------------------
local vpls_ctagged_ptagged_packet = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header
    .. nni_double_tagged_payload;

local vpls_ptagged_packet = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header
    .. nni_single_tagged_payload;

local vpls_untagged_packet = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header
    .. nni_untagged_payload;


return
{
    ctagged_ptagged_packet      = string.lower(vpls_ctagged_ptagged_packet),
    ptagged_packet              = string.lower(vpls_ptagged_packet),
    untagged_packet             = string.lower(vpls_untagged_packet),
    expected_tagged             = string.lower(vpls_expected_tagged),
    expected_untagged           = string.lower(vpls_expected_untagged),
    expected_dif_tagged         = string.lower(vpls_expected_dif_tagged)
}
