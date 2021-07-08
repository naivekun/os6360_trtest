--
-- vpls packet
--

local uni_mac_da        = "000000240003";
local uni_mac_sa        = "000000240002";
local uni_ctag          = "81000014";
local uni_ctag_2        = "810000DC";
local uni_l2_payload    =
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"

------------Sent packets--------------------------------------------------------
local vpls_tagged_packet =
    uni_mac_da .. uni_mac_sa .. uni_ctag .. uni_l2_payload;
local vpls_untagged_packet =
    uni_mac_da .. uni_mac_sa .. uni_l2_payload;
local vpls_tagged_packet_different_vid =
    uni_mac_da .. uni_mac_sa .. uni_ctag_2 .. uni_l2_payload;


------------Expected packets----------------------------------------------------
local vpls_expected_tagged = ""
    .. uni_mac_da .. uni_mac_sa
    .. uni_ctag .. uni_l2_payload;

local vpls_expected_untagged = ""
    .. uni_mac_da .. uni_mac_sa
    .. uni_l2_payload;

local vpls_expected_dif_tagged = ""
    .. uni_mac_da .. uni_mac_sa
    .. uni_ctag_2 .. uni_l2_payload;

return
{
    tagged_packet               = string.lower(vpls_tagged_packet),
    untagged_packet             = string.lower(vpls_untagged_packet),
    expected_tagged             = string.lower(vpls_expected_tagged),
    expected_untagged           = string.lower(vpls_expected_untagged),
    expected_dif_tagged         = string.lower(vpls_expected_dif_tagged),
    tagged_packet_different_vid = string.lower(vpls_tagged_packet_different_vid)
}
