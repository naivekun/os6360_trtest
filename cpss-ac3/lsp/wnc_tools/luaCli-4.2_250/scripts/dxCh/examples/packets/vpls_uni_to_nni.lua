--
-- vpls packet
--

local uni_mac_da        = "000000240004";
local uni_mac_sa        = "000000240002";
local uni_ctag          = "81000014";
local uni_ctag_2        = "810000C8";
local uni_l2_payload    =
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"

local vpls_tagged_packet =
    uni_mac_da .. uni_mac_sa .. uni_ctag .. uni_l2_payload;
local vpls_untagged_packet =
    uni_mac_da .. uni_mac_sa .. uni_l2_payload;
local vpls_tagged_packet_dif_vid =
    uni_mac_da .. uni_mac_sa .. uni_ctag_2 .. uni_l2_payload;

local nni_mac_da        = "000000B01A11";
local nni_mac_sa        = "000102030405";
local nni_inner_tag     = "81000014";
local nni_outer_tag     = "88A80065";
local nni_tag           = "810000C8";
local mpls_ether_type   = "8847";             --MPLS ethertype
local mpls_header       = "7777200055552100"; --MPLS Header: outer-label=77772, inner-label=55552


local nni_untagged_payload =
    uni_mac_da .. uni_mac_sa .. uni_l2_payload;

local nni_single_tagged_payload =
    uni_mac_da .. uni_mac_sa .. nni_inner_tag .. uni_l2_payload;

local nni_single_tagged_2_payload =
    uni_mac_da .. uni_mac_sa .. nni_tag .. uni_l2_payload;

local nni_single_pw_tagged_payload =
    uni_mac_da .. uni_mac_sa .. nni_outer_tag .. uni_l2_payload;

local nni_double_tagged_payload =
    uni_mac_da .. uni_mac_sa .. nni_outer_tag .. nni_inner_tag .. uni_l2_payload;

local nni_double_tagged_2_payload =
    uni_mac_da .. uni_mac_sa .. nni_outer_tag .. nni_tag .. uni_l2_payload;


--------------------------------------------------------------
local vpls_expected_double_tagged = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header
    .. nni_double_tagged_payload;
    
local vpls_expected_double_tagged_2 = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header
    .. nni_double_tagged_2_payload;

local vpls_expected_single_tagged = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header
    .. nni_single_tagged_payload;

local vpls_expected_single_tagged_2 = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header
    .. nni_single_tagged_2_payload;

local vpls_expected_untagged = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header
    .. nni_untagged_payload;

local vpls_expected_single_pw_tagged = ""
    .. nni_mac_da .. nni_mac_sa
    .. mpls_ether_type .. mpls_header
    .. nni_single_pw_tagged_payload;


return
{
    tagged_packet               = string.lower(vpls_tagged_packet),
    untagged_packet             = string.lower(vpls_untagged_packet),
    tagged_packet_dif_vid       = string.lower(vpls_tagged_packet_dif_vid),
    expected_double_tagged      = string.lower(vpls_expected_double_tagged),
    expected_double_tagged_2    = string.lower(vpls_expected_double_tagged_2),
    expected_single_tagged      = string.lower(vpls_expected_single_tagged),
    expected_single_tagged_2    = string.lower(vpls_expected_single_tagged_2),
    expected_untagged           = string.lower(vpls_expected_untagged),
    expected_single_pw_tagged   = string.lower(vpls_expected_single_pw_tagged)
}


