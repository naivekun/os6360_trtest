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
* @file prvCpssDxCh_ppa_fw_imem_addr_data.h
*
* @brief hold the FW code to support the PHA unit.
*
*   NOTE: the content is copied from the FW file : ppa_fw_imem_addr_data.h
*
*   APPLICABLE DEVICES:      Falcon.
*
* @version   1
********************************************************************************
*/
#ifndef PRV_CPSS_DXCH_PPA_FW_IMEM_ADDR_DATA_H_
#define PRV_CPSS_DXCH_PPA_FW_IMEM_ADDR_DATA_H_

/******************************************************************************/
/* FW release version : FALCON_FW_18_06_00 (based on ppa_fw_imem_addr_data.h) */
/******************************************************************************/

/* Falcon : the number of words of data in prv_cpss_dxch_falcon_ppa_fw_imem_data */
#define PRV_CPSS_DXCH_FALCON_PPA_FW_IMAGE_SIZE (1796)

/* Falcon : the FW code to load into the PHA unit  */
const GT_U32 prv_cpss_dxch_falcon_ppa_fw_imem_data[PRV_CPSS_DXCH_FALCON_PPA_FW_IMAGE_SIZE] = {
0x02004400,
0x03e63101,
0xc000c230,
0x60000000,
0x00500050,
0x00004105,
0x04d13141,
0x00440000,
0x93f494f5,
0x91f292f3,
0x052090f1,
0x4f00004f,
0x9bfc9cfd,
0x99fa9afb,
0x97f898f9,
0x95f696f7,
0x1005d130,
0xe83025f6,
0x3095ff05,
0x9dfe05b1,
0x25f61305,
0x1205eb30,
0xee3025f6,
0x25f61105,
0x00004f05,
0x25f6154f,
0x14050330,
0xe63025f6,
0xd30fd30f,
0xf0500193,
0x4edbad9e,
0x204e00ba,
0xffffffff,
0xffffffff,
0xf6000000,
0xd30f63ff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x63fffc00,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x00000000,
0x00200af0,
0x4f00404f,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0x63fffc00,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xffffffff,
0xfffc5001,
0x50015e63,
0x10500005,
0x211cf090,
0x42660021,
0xd00f0000,
0x20211c10,
0x638010c0,
0xc0209022,
0xeb30a200,
0x42000000,
0x1cf09010,
0x50012a50,
0x0550012a,
0x001e5000,
0x50001450,
0x42900d43,
0x0f000000,
0x211c10d0,
0x00078010,
0xc026c148,
0x9234d00f,
0x1042900d,
0x00004305,
0x06233407,
0xc0302334,
0xc47cc486,
0xc45ec463,
0x24340422,
0x02253403,
0x34012634,
0x28340027,
0x004f0364,
0x05104f00,
0x4e00004e,
0x3405d00f,
0x0d42cafe,
0x84304290,
0x00430510,
0x9fe04300,
0xc03047c8,
0x9230d00f,
0x463f8330,
0x244dff22,
0x01450821,
0x140f4408,
0x429c0f4b,
0x0f46c00f,
0x4808004a,
0x0a490801,
0x540a495c,
0x4a440748,
0x0c08460c,
0x4b0c0842,
0x0c44000c,
0x0c884500,
0xbe2b36bd,
0x36c12236,
0x2736c226,
0x00470c00,
0x2936bb48,
0xbc2836ba,
0x36c02a36,
0x2436bf25,
0x00054b85,
0x45460344,
0x074a0801,
0x08214954,
0x01460877,
0x440a4708,
0x46447a47,
0x2d42040f,
0x4a00f949,
0xa045080a,
0x8c74449a,
0x4204124b,
0x36b32536,
0x2936b22a,
0xf92836b1,
0xf9004800,
0xb82736b9,
0x36b52636,
0x2b36b422,
0xb72436b6,
0x46a00026,
0x6c22bd6c,
0x044747a0,
0x46000347,
0x6de0257d,
0x2bacec24,
0xac2736b0,
0x36af2236,
0xec2a36aa,
0x36ab2a9d,
0x2436ad2b,
0xe02536ae,
0x2536a528,
0xec256d4c,
0x0003298c,
0x4b440044,
0x36a22436,
0x4bb4002b,
0xa944b874,
0x7dec2936,
0xa8224c98,
0x86032836,
0x276dec45,
0xa4490749,
0x45995846,
0xa3480739,
0x36a12236,
0x2abdfc2a,
0xbdf046ec,
0x2936a027,
0xa749ae48,
0x04472736,
0x00229dd8,
0x369f4544,
0x48aa4428,
0x6c25369c,
0x47000346,
0xe024369b,
0x369e24bd,
0x2636a627,
0x36934207,
0x45a00025,
0x0322369a,
0x00034900,
0x74273695,
0xa00447a4,
0x49fc0046,
0x492b8dda,
0x2b369924,
0x032a6de0,
0x36974800,
0x26369d29,
0x07392a36,
0x2436924b,
0x48223691,
0x5dfc429a,
0x44296dec,
0x36964b96,
0x2a5df028,
0x9848f808,
0x242d584a,
0xe82a368f,
0x3690285d,
0x29368e2b,
0x36894808,
0x28368d24,
0x9822bd5a,
0x0003267c,
0x034ae800,
0x36944786,
0x22368826,
0x0144001f,
0x44e0e02b,
0xe042001f,
0x8558265d,
0x2a368647,
0x36804900,
0x2b368724,
0xe4283684,
0x9d604800,
0x8b47000d,
0x368c2636,
0x45450227,
0x0342c0c0,
0x44080344,
0x7f49e408,
0x60022236,
0x46540745,
0xe70147de,
0x25368a46,
0x85420807,
0x00e42936,
0xe4298cf4,
0x08214200,
0x26368245,
0xde273683,
0x48540f29,
0x7e440739,
0x6d142b36,
0x2436772b,
0x04074608,
0x25368142,
0x764500e6,
0x367c2236,
0xf244f244,
0x5c14275c,
0x48d8002a,
0x23490407,
0x28367b49,
0xf42a367d,
0x367a2b5c,
0x24366e27,
0x36792936,
0x22366d26,
0xd842ef00,
0x8b124600,
0x494b0407,
0x36754707,
0x4a44002b,
0x73488663,
0x46852d45,
0x5c490005,
0xec014885,
0x4782484a,
0x36742736,
0x2836722a,
0x7449dede,
0xc80f46fc,
0xbc48c00f,
0x08214a05,
0x2b366c47,
0x714be803,
0x25367849,
0x00460801,
0x367045da,
0x29366a26,
0x36682a36,
0x4700ca27,
0x004ae501,
0x082348ca,
0xf84600c2,
0x8603247c,
0x4900ec45,
0x6b283669,
0x29366148,
0x2a45f958,
0x3665226c,
0x26366724,
0x366f2236,
0x2a7c2225,
0x0d489648,
0x07494985,
0x072a3662,
0x08034747,
0x246c3642,
0x6445540f,
0x4a852d47,
0x63293658,
0x80744886,
0x28366049,
0xc2002536,
0x24365c45,
0xf846c80f,
0x61004200,
0x5b27365f,
0xee002236,
0x2b5d2a46,
0x66444680,
0x4986442a,
0x5c283657,
0x07394899,
0x4a907449,
0x36634460,
0x26365e2b,
0x03424603,
0x36594786,
0x80425f05,
0x08074644,
0x2436534b,
0x01450821,
0x49001348,
0x4f484400,
0x45802936,
0x478d584a,
0x60014b00,
0x22365246,
0x01273656,
0x80014a48,
0x032b365a,
0x365d44fc,
0x4500ee25,
0xf8440407,
0x28364e49,
0x072a3650,
0x05bc4704,
0x26365542,
0x9f1242f9,
0x4a001d47,
0x46480013,
0xb4b42936,
0x02445c07,
0x364a4545,
0x4b478224,
0x01460407,
0x27364d4a,
0x00223649,
0x45004683,
0x4560014b,
0x5c0f4200,
0x2a364747,
0xa0283645,
0xc0c048a0,
0x01243641,
0x365444cb,
0x2b365125,
0x0f26364c,
0x47a40027,
0x2342b8d0,
0x0019266d,
0x45540f4b,
0xc4002536,
0x4bdede45,
0x40263643,
0x36442236,
0x420c0200,
0x00c82536,
0x45040745,
0x4b2b3648,
0x43000042,
0xc340c060,
0xd00f0000,
0xd00f0000,
0x50935294,
0x253cf096,
0x88430590,
0x88884288,
0x00004905,
0x47888849,
0x0f000000,
0x519230d0,
0xaa0a92a0,
0x080a420b,
0x91478888,
0x8088908b,
0x46000046,
0x0b02074f,
0x98907761,
0x8694b188,
0xa29292c8,
0x8092a0b4,
0x2e8a92c0,
0x331f776b,
0x927489f0,
0x33b42292,
0x309b20b4,
0x4db1888b,
0x55d30fd3,
0x223684c8,
0x00432e00,
0xc0204300,
0xc050211c,
0xc030c040,
0xd00f0000,
0x0f63fff8,
0x8010211c,
0x1053ffe3,
0x42332090,
0xf042000f,
0x2326800c,
0x00423e00,
0xc0304200,
0x10d00f00,
0x211cf090,
0xd00f0000,
0x810a2000,
0x02002222,
0x5d5000d4,
0x00325000,
0x50001250,
0x1050000b,
0x5003a850,
0x5a500301,
0x01d35002,
0x50014f50,
0x10d00f00,
0x8010211c,
0xf750050e,
0x04505004,
0x02002222,
0x2326800c,
0x00423e00,
0xc0304200,
0x00000000,
0x00000000,
0xd00f0000,
0x810a2000,
0x80827705,
0x857b280a,
0x00472100,
0xc0604700,
0x404b02a2,
0x245d2224,
0x11050547,
0xc5530d55,
0xa454234c,
0x470c4411,
0xb0440404,
0x5502820c,
0x09880149,
0xf849ffff,
0x887f49ff,
0x40732b18,
0x1d224c50,
0x987f6000,
0x00098802,
0x00034900,
0x2b5c4b26,
0x02228483,
0x0e000822,
0x4a00004a,
0x0000433e,
0x26a6c143,
0x022b8486,
0xa6c008bb,
0x00d00f00,
0x32810a30,
0x0c020023,
0x00263680,
0x804b268d,
0x48220024,
0x80480000,
0xc09027fa,
0xaa550a77,
0x0404470a,
0x8049b044,
0xff8a6728,
0x07074772,
0x85222c40,
0x0d8811b8,
0x0c0c4211,
0x0a804200,
0x75a34e27,
0x542a7cd8,
0x73576540,
0xc199b029,
0x0e002926,
0x2bbd1142,
0x000e4b11,
0x26c407aa,
0x2a8c2829,
0xc3257493,
0x26c22926,
0x062926c6,
0x7496050b,
0x23749a2a,
0x0223ac14,
0x2926800c,
0x00423e00,
0x643b4200,
0x2926c52b,
0x44fff844,
0xd00f836f,
0x810a2000,
0x02002222,
0x3302936f,
0x44000004,
0x01440003,
0xffff0433,
0x00000000,
0x00000000,
0x00000000,
0x63ffd200,
0x13026540,
0x211ce090,
0x20008290,
0x49000049,
0x5a400248,
0x40a87702,
0x18400207,
0x02264002,
0x66020254,
0x027a40a8,
0x3a40aa66,
0x40aa8802,
0x11c0709a,
0x81370e5a,
0x289d0228,
0xa766aa55,
0x800e5511,
0x9512260a,
0x5502520c,
0x11c3a8a6,
0x215b4700,
0x40d30065,
0xaa0220ac,
0x05aa0c06,
0x7dedc0b0,
0x47210024,
0x102264af,
0x00b85292,
0xc0b02b46,
0xa22a649e,
0xac1e2b64,
0x2b46cb2b,
0xaa27ac39,
0x743b2b64,
0x2bac3c22,
0xc72b46c8,
0x46c92b46,
0xb282122b,
0x14c070c0,
0x2764a698,
0x64b22b46,
0x78870922,
0x8890b302,
0xca6420c0,
0xbe2b46cf,
0xb2382864,
0x0379870b,
0xcc8890b4,
0x00828d46,
0x00004821,
0x7a874848,
0x8890b433,
0x0646266d,
0x02020603,
0xff062201,
0x3fff46ff,
0x2746d327,
0xd12746d2,
0x46d02746,
0x22926027,
0xa866280a,
0x11087846,
0xd5888986,
0x46d42746,
0x33260a80,
0xdf8890b4,
0x84ff2746,
0x80b86626,
0x90b4337c,
0x2b46d688,
0x382864da,
0x7b870bb1,
0x7d870e28,
0x8890b433,
0xe62b46d9,
0x87092364,
0xb4337e87,
0x46e08890,
0x2284022b,
0x1a80b232,
0x337f8709,
0xd78890b4,
0x64de2b46,
0x0bb13222,
0x293cfc29,
0x46cd8814,
0xb6b4332b,
0xb3382864,
0x110b9902,
0xac2c083b,
0xce891023,
0x64ba2746,
0x2746e127,
0x0723840b,
0x1a802384,
0x64804028,
0xac382746,
0x2746e322,
0xc7238413,
0x46e22975,
0x64efb400,
0xdac85b20,
0xc0802746,
0xe42264eb,
0x64fa2746,
0x29ac2e29,
0xdb7583f2,
0xb4882746,
0x004a3e00,
0x46e34a00,
0x2975c727,
0xde600005,
0x1c200aa0,
0x81801321,
0x02002aa2,
0x27a6800c,
0x00000000,
0x00000000,
0x00000000,
0x00d00f00,
0x90124920,
0x1ce09c13,
0x49000021,
0xc070c3a8,
0x056c40a0,
0x2d057040,
0x9d022881,
0x00859028,
0x32400540,
0x11a0cc05,
0x0ec21192,
0xcc055040,
0x40050040,
0xa0cc0512,
0x2040a200,
0x40a0cc05,
0x1105c50c,
0x0c470ecb,
0x00a0cc0c,
0x050554a2,
0x23ac2c65,
0x0224ac24,
0x0a800baa,
0x0baa0c2b,
0xbded2706,
0x4b210020,
0x804b0000,
0x516d261a,
0x641b2706,
0x47921022,
0xb8520202,
0xe50ec511,
0x802a749e,
0x06e9270a,
0x2706e827,
0xe62706e7,
0x06ea27ac,
0x2706c727,
0x642ac070,
0x27ac1e27,
0x329814c0,
0xac282b64,
0x27642e2b,
0x2522b43b,
0x88907887,
0xec64c0be,
0x06eb2b06,
0x70c0b22b,
0x3379870b,
0xed8890b4,
0x64362b06,
0x0bb33828,
0x7a874948,
0x8890b433,
0x3a2b06ee,
0xb2322264,
0xff072201,
0x3fff47ff,
0x00828d47,
0x00004821,
0x202206d0,
0x229270c0,
0x0206277d,
0x03074602,
0x2206d588,
0xd32206d4,
0x06d22206,
0x2206d122,
0x772784ff,
0x280a80b8,
0x7846a877,
0x89871108,
0xb1382864,
0x707b870b,
0x90b433c0,
0x2206df88,
0x64422b06,
0x7c870923,
0x8890b433,
0x3e2b06ef,
0x462b06f1,
0xb2322264,
0x337d870b,
0xf08890b4,
0x2b06f288,
0x3828644a,
0x7e870bb1,
0x8890b433,
0x332b06f3,
0x29644eb4,
0x8709b339,
0x90b4337f,
0x1029ac18,
0x2706f483,
0xfc2b6452,
0x88142b3c,
0x29646727,
0x4029646b,
0x33026480,
0x089b110b,
0x06fb2706,
0x29647327,
0xf92375f7,
0x06fa2706,
0x62c0806a,
0x06f62264,
0x22ac1a27,
0xfc24645b,
0x06f70808,
0x5fb48827,
0xb8992964,
0xc111a849,
0xf72706fb,
0x00052375,
0x2706f860,
0x477582ed,
0x2aa2818c,
0x800c0200,
0x3e0027a6,
0x4a00004a,
0x00000000,
0x00d00f00,
0x1c200aa0,
0x13801221,
0x211ce090,
0xc0490000,
0x80c4a0d2,
0xc070260a,
0x20921e0c,
0x921f9213,
0x2891372c,
0x12492200,
0x330ca450,
0x0cb550a4,
0x500c8350,
0xeb500c94,
0x500cdc50,
0xa5bb0cc5,
0x540cf550,
0xa5440002,
0x0c4702c2,
0xcca3cc0c,
0x11a5cca4,
0xacbc0ebb,
0x3823ac40,
0xaa0224ac,
0x05aa0c06,
0x0c0ec511,
0x220c8315,
0x00034703,
0x15008247,
0x65218c93,
0x80050347,
0x9315200a,
0x1a809416,
0x64217c26,
0x0d132706,
0x400e0024,
0x10400000,
0x2304af93,
0xff260d04,
0x647e2706,
0x2264822a,
0xcb22ac20,
0x83158416,
0x1123443b,
0x260a809b,
0x97609814,
0x0a2264b2,
0xb3327086,
0x921fc0b2,
0x64c0cb28,
0x382864be,
0x71860cb2,
0x921fb433,
0x2b06cc28,
0x00004821,
0x72864a48,
0x921fb433,
0x2b06cf28,
0x02020603,
0xff062201,
0x3fff46ff,
0x00828d46,
0xd12706d2,
0x06d02706,
0x22926027,
0x0646266d,
0x11080846,
0xd5888986,
0x06d42706,
0x2706d327,
0x06df2892,
0x2684ff27,
0x80266c18,
0xa866280a,
0x64da2b06,
0x0cb13828,
0x0a807386,
0x1fb43326,
0x2b06d928,
0x0a2364e6,
0xb4337486,
0xd628921f,
0x2284022b,
0x1a80b232,
0x75860f28,
0x921fb433,
0x2264de2b,
0x860cb132,
0x1fb43376,
0x06e02892,
0x2864b6b4,
0x8609b338,
0x1fb43377,
0x06d72892,
0x2706ce83,
0xfc2964ba,
0x8814293c,
0x332b06cd,
0x64805122,
0x3302b349,
0x08b91109,
0x102bac2c,
0x06e22375,
0x2706e127,
0x072b240b,
0x1a802b24,
0x28247727,
0x482706e4,
0x2706e3b2,
0xc72b2413,
0x06dc22ac,
0x2864f327,
0x7a2706fe,
0x06fd2924,
0x992964ef,
0x11a849b8,
0xc0806ac1,
0x2e2264fa,
0x06de6000,
0x7582ed27,
0xdb080847,
0xb4882706,
0x7a2706fe,
0x06e329a4,
0x2a1a8027,
0x0e2375c7,
0x2bb2818c,
0x800c0200,
0x3e0027b6,
0x4b00004b,
0x00000000,
0x00d00f00,
0x1c200ab0,
0x13801221,
0x90134922,
0x00211ce0,
0xd2c04900,
0xc070c4a0,
0x0c94500c,
0x140ce050,
0x28912d92,
0x002c921a,
0x0cf550a5,
0xa450a544,
0x0cb5500c,
0x8350a433,
0x00112592,
0x0ca5cc0e,
0x0cdc50ac,
0x000cc550,
0xcb110baa,
0x0502540e,
0xcc0c0c47,
0x19a4cca3,
0x2423ac2c,
0xaa0224ac,
0x2b0a800b,
0x0c02c20c,
0x660c6461,
0x05864702,
0x12050247,
0x65219290,
0x21002b4d,
0x44000044,
0x17261a80,
0x84931694,
0x47931023,
0xc5110503,
0x2706e50e,
0xf1204ded,
0x0a802706,
0x2706e722,
0xe62706e8,
0x641b2706,
0xc797b398,
0x648e2706,
0x22ac1e22,
0xe92a249e,
0x921ac0b2,
0x64c0cf28,
0x83168417,
0x1523443b,
0x921ab433,
0x2b06ed28,
0x38286436,
0x70860cb3,
0x921ab433,
0x2b06ee28,
0x3222643a,
0x71860cb2,
0x3fff47ff,
0x00828d47,
0x00004821,
0x72864b48,
0x229270c0,
0x0206277d,
0x03074602,
0xff072201,
0xd32206d4,
0x06d22206,
0x2206d122,
0x202206d0,
0x280a8027,
0x0846a877,
0x89871208,
0x2206d588,
0x33c07073,
0x28921ab4,
0xff2206df,
0x7c182784,
0x1ab43374,
0x06ef2892,
0x28643e2b,
0x860cb138,
0x3375860c,
0x28921ab4,
0x422b06f0,
0x860a2364,
0x3376860c,
0x28921ab4,
0x462b06f1,
0xb2322264,
0x33778609,
0x28921ab4,
0x4a2b06f2,
0xb1382864,
0x223cfc4b,
0x06f38815,
0x4eb4332b,
0xb3392964,
0xac188310,
0x2706f429,
0x00226452,
0x00004b12,
0x4d821122,
0xb3496480,
0x11093302,
0x99110899,
0x75f72264,
0x2706fa23,
0x6b2706f9,
0x64672264,
0x97b12964,
0xfc286487,
0xb2482706,
0x732706fb,
0x6462c080,
0x22ac1a22,
0x649397b4,
0x8a97b228,
0x2706f708,
0x645fb488,
0x49b89929,
0x6ac111a8,
0x75f72706,
0x60000a23,
0xed2706f8,
0x08477582,
0x27a6800c,
0x004a3e00,
0x97b24a00,
0xfb29648a,
0x0aa000d0,
0x13211c20,
0x818c1480,
0x02002aa2,
0x00000000,
0x00000000,
0x00000000,
0x0f000000,
0x48000021,
0xc048c060,
0x0a80c4b0,
0x251a802a,
0x48200029,
0x9c169015,
0x9d179e18,
0x1cd09f19,
0x50007d50,
0xffff006e,
0x4c3fff4c,
0x8df28080,
0xd350a3ff,
0x50a32200,
0xe25000cf,
0x00f35000,
0x00a25000,
0x9312a2ff,
0x500ef311,
0xa2ff00b2,
0x52500fdd,
0x22a2ff00,
0x008250a3,
0x9350a2ff,
0x092ff12d,
0x221102ee,
0x2f8d020f,
0x09208dee,
0x472f8247,
0x8d010d0d,
0xdd9f132e,
0x228245ae,
0xd304330a,
0x0f5f40a3,
0xf3509310,
0x2606e502,
0xbc2c24bc,
0x0abb0227,
0x1103bb0c,
0x03034793,
0xe72606e9,
0x06e82606,
0x2606e626,
0x2423541b,
0x2606ea0c,
0x2a2606c7,
0xbc1e2a54,
0x2ba49e2a,
0xda0a23ac,
0x7026310b,
0x3b2ffd2e,
0xff1123e4,
0x15269614,
0x54d22696,
0x2aac2c2a,
0x342354d6,
0x7af13842,
0x012ff295,
0xf2950c2c,
0x22f2942a,
0x2a83ed2a,
0xdaf06000,
0xff7c232f,
0x1dcd4264,
0x1473f117,
0x03e35193,
0x010a0f41,
0xf2950c3c,
0x02f20c23,
0x000db232,
0x04b0aa60,
0xb13272f9,
0x4323549a,
0x245496b4,
0xfe023a39,
0xacfd2aac,
0x96978380,
0x0827549e,
0x83807036,
0x96969695,
0x2254a69f,
0x54a2b272,
0x71360f27,
0xb477c0f2,
0x46233d22,
0x360b0703,
0x80b47772,
0x98969983,
0x0246222d,
0x73360b07,
0xb4778a13,
0x9a308380,
0x06070246,
0x36330c03,
0x80b47774,
0x229c2083,
0x06d22606,
0x2606d126,
0x202606d0,
0x222d2293,
0x46a23322,
0x83120272,
0x06d582e9,
0xd3969a26,
0xb4778c11,
0x06df8380,
0x2324ff26,
0x0a80b833,
0x76360ab1,
0x8380b477,
0x54ae9f9b,
0x75360827,
0x082754b6,
0xb4777736,
0x9f9c8380,
0x732354b2,
0xb47e2e54,
0x0f2754ba,
0xb4777836,
0x9f9d8380,
0xb673b272,
0x77793613,
0x9f8380b8,
0xbe969e9f,
0x8380b877,
0x102f9611,
0x54c62f96,
0x2254c223,
0x96122f96,
0x2854ce2f,
0x54cab478,
0x7a360d27,
0x4a23546b,
0xc70264a0,
0x08371107,
0x1323bc18,
0x23547326,
0xf92765f7,
0x06fa2606,
0x23546726,
0x06f62754,
0x27bc1a26,
0xfc24545b,
0x06fb2606,
0x140e3811,
0xa3d36a31,
0x70033309,
0x628310c0,
0x07074778,
0x772606f7,
0x29545fb4,
0xa749b899,
0x06fb4a00,
0x2765f726,
0xf8600005,
0x72ed2606,
0x8d178e18,
0x02008c16,
0x26a6800c,
0x004a3e00,
0x00211c30,
0x80150aa0,
0xa2818f19,
0x211c302a,
0x00000000,
0x00000000,
0x00000000,
0xd00f0000,
0xd090159f,
0x0000211c,
0x80c04848,
0xc050260a,
0x01298df2,
0x20002a8d,
0x189c1648,
0x199d179e,
0x500ef350,
0xffff0e6f,
0x4d3fff4d,
0xc4c08e80,
0x00a2000e,
0x0ed350a3,
0xc050a322,
0x0ee2500e,
0x0e9350a2,
0x000ea250,
0x119312a2,
0xb2500e03,
0x22110e7e,
0x0e52500f,
0xa322a200,
0x000e8250,
0x45afee0e,
0x8dee2282,
0x02ff0920,
0x5000ee09,
0x1393102f,
0x2ff1379f,
0x0202f350,
0x0e472f8d,
0x0f5f4023,
0x03479311,
0x04330a03,
0x8247a3e3,
0xcc382506,
0x27cc4024,
0x0c06cc02,
0x64af03cc,
0xc72506c8,
0x649e2506,
0x2b64a22c,
0xcb2bcc1e,
0x222a800c,
0x2e702634,
0xa43b2ffd,
0x0cff1123,
0x240e2596,
0x233c4023,
0x482b2412,
0xe30a2b3c,
0x012af295,
0xf2950dad,
0x2af2942b,
0x24259623,
0xdba06000,
0xff7df32f,
0x1dcd4f64,
0x7ba1384f,
0x02e25192,
0x010b0f41,
0xf2950d2d,
0x2a82ad2b,
0x000db223,
0x04b0bb60,
0xb12373f9,
0x1472f117,
0xb44f2a1a,
0xfe032b39,
0xbcfd2bbc,
0x03f30c22,
0x16838070,
0x96172596,
0x2fa4de25,
0x8024a4da,
0xc0f27136,
0x8380b477,
0xe2259618,
0x360927a4,
0x25961a83,
0xea2f9619,
0xb27222a4,
0x1127a4e6,
0x9b308380,
0x46233d22,
0x360b0703,
0x80b47772,
0x229d2083,
0x0246222d,
0x73360b07,
0xb4778b13,
0x222d2293,
0x06070246,
0x36360d03,
0x80b47774,
0xd3420000,
0x06d22506,
0x2506d125,
0x202506d0,
0x027246a2,
0x82298312,
0x9a2506d5,
0x42210095,
0x778d1175,
0xdf8380b4,
0x64ff2506,
0x33b83323,
0x76360bb1,
0x8380b477,
0xee2f961b,
0x360927a4,
0x360927a4,
0x80b47777,
0x2f961c83,
0x7323a4f2,
0xa4f6b472,
0x78361127,
0x8380b477,
0xca2f9612,
0x77793616,
0x1e8380b8,
0x961d2f96,
0x22a4fa25,
0x022f961f,
0xb6732324,
0x7323a4fe,
0x222a80b2,
0x222a8027,
0x3610b478,
0x80b8777a,
0x2f962083,
0xcc2c0873,
0x2f962227,
0x0a2f9621,
0x24062824,
0x072506e2,
0xa40b27a4,
0x64b04a27,
0x1103d302,
0x2506e424,
0x132506e3,
0x55c727a4,
0x2506e123,
0x10c07003,
0x2764fa83,
0x2e2506da,
0x64eb27cc,
0xb8992964,
0x3811a749,
0x6a31140e,
0x3309a3e3,
0x2506de60,
0x477872ed,
0x06db0707,
0xefb47725,
0x3e0025a6,
0x4a00004a,
0xc72506e3,
0x00052355,
0x302aa281,
0x8e18211c,
0x8c168d17,
0x800c0200,
0x00000000,
0x1c30d00f,
0x0aa00021,
0x8f198015,
0x90159f19,
0x00211cd0,
0xd2c04900,
0xc060c550,
0x2890650c,
0x921a9216,
0x4922002c,
0x9e189d17,
0xef50a3ff,
0x0cf3500c,
0x500e8811,
0x6d500cce,
0x500ee311,
0xafee0cbf,
0xd350a3ee,
0x0808470c,
0x0c8f50a3,
0x9350afee,
0x0caf500c,
0x9311afee,
0x0ecc090f,
0x110c7c50,
0x5f500fff,
0xffafee0c,
0x0c470ec4,
0xe7adcc0c,
0xff87e98b,
0xdd092e9d,
0x775a0260,
0x7707c70a,
0x070746a8,
0x110b5b40,
0x2d040247,
0x00002f91,
0x80c4d04a,
0x01f9271a,
0x0c20adfc,
0x0a8002dd,
0x4a12002f,
0x92109f13,
0x22741b26,
0x2c25dc24,
0xdd0228dc,
0x2606e50f,
0x06e92df4,
0x23dc1e26,
0xe82606e7,
0x06e62606,
0x4f3fff4f,
0xa30cbb11,
0x2606c796,
0x9e23748e,
0x1503e251,
0xbd302bb2,
0x22e43b2b,
0xffff83ed,
0x1b8214b1,
0x0f4172f1,
0x0193120b,
0x92140f33,
0xbcfd2bbc,
0x0f821423,
0xb0bb6000,
0x2272f904,
0x2f2a8070,
0x3923921a,
0xf20c023b,
0xfeb22202,
0xa6262392,
0x26a62526,
0x1a28f416,
0x361028f4,
0x832374a6,
0x2874a2b2,
0xf2713610,
0x1ab488c0,
0x08024622,
0x8872360c,
0x23921ab4,
0x9fa896a9,
0x360d232a,
0x888b1373,
0x23921ab4,
0x2d229b20,
0x74363782,
0x921ab488,
0x2fa62c23,
0x80283432,
0x2606d026,
0x02069230,
0x233d2202,
0x12080346,
0xd583e982,
0x96aa2606,
0xd22606d3,
0x06d12606,
0x2234ff26,
0x80222c18,
0xa322230a,
0x11030346,
0x2874ae9f,
0x10753609,
0x1ab4888e,
0x06df2392,
0x74b29fac,
0x0bb18323,
0xb4887636,
0xab23921a,
0xad23921a,
0x2874b69f,
0x88773609,
0x23921ab4,
0x1e283422,
0x2a802834,
0x1ab48223,
0xb4887836,
0x921ab888,
0x289faf23,
0xa62726a6,
0x2274be26,
0xc62fa610,
0x74c22374,
0x83b28222,
0x793614b6,
0x74cab489,
0x7a360d28,
0x921ab888,
0x2fa61123,
0x2f1a0008,
0x1329dc18,
0xa6122fa6,
0x2974ce2f,
0x0264b04f,
0xe8110c88,
0x02b15e08,
0x93110e33,
0x26a62a2e,
0x2928f5d5,
0xb42726a6,
0x2b2a802e,
0x06fa2606,
0x29746726,
0x2b29746b,
0xb42f26a6,
0x1a2606fc,
0x06fb28dc,
0x29747326,
0xf92365f7,
0x29745fb4,
0xa859b899,
0x806ac111,
0x287462c0,
0xf860000b,
0x82ed2606,
0x08084774,
0x882606f7,
0x4a00004a,
0xf72606fb,
0xa62a2365,
0x28f5d526,
0x8e18211c,
0x8c168d17,
0x800c0200,
0x3e0026a6,
0x1c30d00f,
0x0aa00021,
0x8f198015,
0x302aa281,
0x90169f1a,
0x1cd09c17,
0x49000021,
0xc060c550,
0xff87b98d,
0x90792b9d,
0x49220028,
0x9e199d18,
0x500ec250,
0x5d400e6f,
0x0e88110d,
0xb72e921f,
0xa433a877,
0x500ee350,
0x07460ef4,
0x08084707,
0x119412a3,
0xb3500e24,
0x22a3220e,
0x0ed450a4,
0xa433a322,
0x500e8350,
0xa3220e94,
0x220ea350,
0x02ee09af,
0x1103ff09,
0x7e500f33,
0x0e53500e,
0x026001f6,
0xe70a775a,
0x0ee41107,
0xee0e0e47,
0x02479210,
0x20913704,
0x0a80c4f0,
0x2c1a8027,
0x0e0007ff,
0x2274af40,
0x0c400000,
0x901102ff,
0x23fc2026,
0x142a0d04,
0x25fc389f,
0x0228fc40,
0xa00cdd11,
0x2606ff96,
0x822fc47e,
0x06cb23c4,
0xdd302dd2,
0x22b43b2d,
0xffff83bd,
0x4f3fff4f,
0x0f4172f1,
0x0193130d,
0x92150f33,
0x1503e251,
0x0d23dcfd,
0xb0dd6000,
0xb272f904,
0x198b15b1,
0x921f2f2a,
0x023d3923,
0x0c2ddcfe,
0xb2b202f2,
0x2d26a62e,
0xf43626a6,
0x28f43a28,
0x80703610,
0xe6b28323,
0x361228c4,
0x88c0b271,
0x23921fb4,
0x8872360c,
0x23921fb4,
0x1926a61a,
0xc4ea2ba6,
0x888d1473,
0x23921fb4,
0x2d229d20,
0x08024622,
0x8874363a,
0x23921fb4,
0x522ba634,
0x360a28f4,
0x202606d0,
0x03030693,
0x46222d22,
0x83130802,
0x42210096,
0xd3420000,
0x06d22606,
0x2606d126,
0x33233c18,
0x020246a2,
0x82298312,
0xaa2606d5,
0x360a28c4,
0x1fb48875,
0x06df2392,
0x2374ff26,
0xb18222c4,
0x8876360c,
0x23921fb4,
0xee2ba61b,
0x28c4ca2b,
0x8877360a,
0x23921fb4,
0xf22ba61c,
0x3e28f442,
0x361828f4,
0x1fb48878,
0xa6122392,
0xa61e2392,
0x26a6302b,
0xfa26a62f,
0xb48323c4,
0xfe22f402,
0xb28323c4,
0x3614b682,
0x1fb88879,
0x117a360d,
0x1fb88889,
0xa6202392,
0x2ba61f2b,
0xa6228810,
0x2ba6212b,
0x8222f40a,
0x28f406b4,
0x11088811,
0x02b15898,
0xb3110833,
0x2bdc2c08,
0xa631291a,
0x22f44726,
0x90518211,
0x0e880264,
0x332bc40b,
0xf44f26a6,
0x26a63222,
0x002895e5,
0x2bc41326,
0xe12365c7,
0x06e22606,
0x2bc40726,
0x806ae111,
0x2974fac0,
0x2e2606e4,
0x06e329dc,
0x08084774,
0x882606db,
0x2974efb4,
0xa859b899,
0xb5e526a6,
0x2b1a0028,
0xde60000e,
0x82ed2606,
0x0026c680,
0x00004c3e,
0x2606e34c,
0x322365c7,
0x818e198f,
0x1c302cc2,
0x168d1821,
0x0c020080,
0x00000000,
0x30d00f00,
0x8c17211c,
0x1a0ac000,
0x70d470c8,
0x0e8660c0,
0x1200265d,
0x45000045,
0xc0b14800,
0xf7275636,
0xb1447469,
0x66275635,
0xc0932956,
0xa22a563c,
0x2b5637c0,
0x00483e00,
0x40275641,
0x563f2756,
0x27563f27,
0x3d27563e,
0x0a8000d0,
0x00288281,
0x86800c02,
0x27564227,
0x00000000,
0x00000000,
0x00000000,
0x0f000000,
0x03240173,
0xc080c73e,
0x20008250,
0x45000045,
0x0b4780a0,
0x00004700,
0x49000046,
0x20538a51,
0xa074a301,
0x50c2a0d5,
0x210069a1,
0x46120049,
0x86c3c22c,
0x07220222,
0x0c082211,
0xd54005a2,
0xbb1102bb,
0x08221100,
0x4405220c,
0xb05b0b0b,
0xc4286643,
0xbb022b86,
0x42800802,
0x02420000,
0x2836800c,
0x00433e00,
0x943b4300,
0x28664425,
0x80745301,
0xd00f250a,
0x810a3000,
0x02002332,
0x86c30505,
0x07440224,
0x20084411,
0xd540245c,
0x15b33303,
0x03b13082,
0x63ffcb00,
0x06286643,
0xffffffff,
0x01000300,
0xc72f211d,
0xb1318316
};

#endif  /* PRV_CPSS_DXCH_PPA_FW_IMEM_ADDR_DATA_H_ */

