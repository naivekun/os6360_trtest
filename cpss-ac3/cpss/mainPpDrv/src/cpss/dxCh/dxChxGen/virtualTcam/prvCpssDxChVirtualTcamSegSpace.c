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
* @file prvCpssDxChVirtualTcamSegSpace.c
*
* @brief The CPSS DXCH High Level Virtual TCAM Manager - segments space
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

static GT_BOOL prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnable = GT_FALSE;

GT_VOID prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnableSet(GT_BOOL enable)
{
    prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnable = enable;
}

/**
* @enum PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT
 *
 * @brief Column status of horizontal block row status index.
 * All supported states of part0 and part1.
*/
typedef enum
{
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOOD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOOF_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDFFFF_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDOOOO_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFOOOO_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFDD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOODD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOFF_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDFF_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDOO_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFOO_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFDDDD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OODDDD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOFFFF_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDDF_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDDO_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFO_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDDD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFF_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOF_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDF_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDO_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFO_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFF_E,
    /* enum range values */
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E
} PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT;

static char* prvIndexToHorzBlockRowNames[PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E] =
{
    "BUSY",
    "FFFFFD",
    "OOOOOD",
    "OOOOOF",
    "DDFFFF",
    "DDOOOO",
    "FFOOOO",
    "FFFFDD",
    "OOOODD",
    "OOOOFF",
    "DDDDFF",
    "DDDDOO",
    "FFFFOO",
    "FFDDDD",
    "OODDDD",
    "OOFFFF",
    "DDDDDF",
    "DDDDDO",
    "FFFFFO",
    "DDDDDD",
    "FFFFFF",
    "FFFD",
    "OOOD",
    "OOOF",
    "DDDF",
    "DDDO",
    "FFFO",
    "DDDD",
    "FFFF"
};

static GT_U8 prvIndexToHorzBlockRowContainsFree[PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E] =
{
    /*BUSY*/    0
    /*FFFFFD*/,   1
    /*OOOOOD*/,  0
    /*OOOOOF*/,  1
    /*DDFFFF*/,  1
    /*DDOOOO*/,  0
    /*FFOOOO*/,  1
    /*FFFFDD*/,  1
    /*OOOODD*/,  0
    /*OOOOFF*/,  1
    /*DDDDFF*/,  1
    /*DDDDOO*/,  0
    /*FFFFOO*/,  1
    /*FFDDDD*/,  1
    /*OODDDD*/,  0
    /*OOFFFF*/,  1
    /*DDDDDF*/,  1
    /*DDDDDO*/,  0
    /*FFFFFO*/,  1
    /*DDDDDD*/,  0
    /*FFFFFF*/,  1
    /*FFFD*/,    1
    /*OOOD*/,    0
    /*OOOF*/,    1
    /*DDDF*/,    1
    /*DDDO*/,    0
    /*FFFO*/,    1
    /*DDDD*/,    0
    /*FFFF*/,    1
};

static GT_U8 prvIndexToHorzBlockRowContainsDragged[PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E] =
{
    /*BUSY*/    0
    /*FFFFFD*/,   1
    /*OOOOOD*/,  1
    /*OOOOOF*/,  0
    /*DDFFFF*/,  1
    /*DDOOOO*/,  1
    /*FFOOOO*/,  0
    /*FFFFDD*/,  1
    /*OOOODD*/,  1
    /*OOOOFF*/,  0
    /*DDDDFF*/,  1
    /*DDDDOO*/,  1
    /*FFFFOO*/,  0
    /*FFDDDD*/,  1
    /*OODDDD*/,  1
    /*OOFFFF*/,  0
    /*DDDDDF*/,  1
    /*DDDDDO*/,  1
    /*FFFFFO*/,  0
    /*DDDDDD*/,  1
    /*FFFFFF*/,  0
    /*FFFD*/,    1
    /*OOOD*/,    1
    /*OOOF*/,    0
    /*DDDF*/,    1
    /*DDDO*/,    1
    /*FFFO*/,    0
    /*DDDD*/,    1
    /*FFFF*/,    0
};

static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT prvIndexToHorzBlockRowDraggedToOther[PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E] =
{
    /*BUSY*/      PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /*FFFFFD*/,   PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFO_E
    /*OOOOOD*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /*OOOOOF*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOOF_E
    /*DDFFFF*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDFFFF_E
    /*DDOOOO*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /*FFOOOO*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFOOOO_E
    /*FFFFDD*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFOO_E
    /*OOOODD*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /*OOOOFF*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOFF_E
    /*DDDDFF*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOFF_E
    /*DDDDOO*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /*FFFFOO*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFOO_E
    /*FFDDDD*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFOOOO_E
    /*OODDDD*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /*OOFFFF*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOFFFF_E
    /*DDDDDF*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOOF_E
    /*DDDDDO*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /*FFFFFO*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFO_E
    /*DDDDDD*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /*FFFFFF*/,  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFF_E
    /*FFFD*/,    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFD_E
    /*OOOD*/,    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /*OOOF*/,    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOF_E
    /*DDDF*/,    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOF_E
    /*DDDO*/,    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /*FFFO*/,    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFO_E
    /*DDDD*/,    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /*FFFF*/,    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFF_E
};

static  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT prvIndexToHorzBlockRowFreeColumnsType[PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E] =
{
    /*BUSY*/      PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*FFFFFD*/,   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E
    /*OOOOOD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*OOOOOF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E
    /*DDFFFF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E
    /*DDOOOO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*FFOOOO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E
    /*FFFFDD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E
    /*OOOODD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*OOOOFF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E
    /*DDDDFF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E
    /*DDDDOO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*FFFFOO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E
    /*FFDDDD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E
    /*OODDDD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*OOFFFF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E
    /*DDDDDF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E
    /*DDDDDO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*FFFFFO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E
    /*DDDDDD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*FFFFFF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E
    /*FFFD*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E
    /*OOOD*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*OOOF*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E
    /*DDDF*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E
    /*DDDO*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*FFFO*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E
    /*DDDD*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*FFFF*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E
};

static  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT prvIndexToHorzBlockRowDraggedColumnsType[PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E] =
{
    /*BUSY*/      PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*FFFFFD*/,   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E
    /*OOOOOD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E
    /*OOOOOF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*DDFFFF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E
    /*DDOOOO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E
    /*FFOOOO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*FFFFDD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E
    /*OOOODD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E
    /*OOOOFF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*DDDDFF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E
    /*DDDDOO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E
    /*FFFFOO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*FFDDDD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E
    /*OODDDD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E
    /*OOFFFF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*DDDDDF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E
    /*DDDDDO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E
    /*FFFFFO*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*DDDDDD*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E
    /*FFFFFF*/,  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*FFFD*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E
    /*OOOD*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E
    /*OOOF*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*DDDF*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E
    /*DDDO*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E
    /*FFFO*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
    /*DDDD*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E
    /*FFFF*/,    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E
};

static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT seg_columns_to_row_state_free_other[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E] =
{
    /* _NONE_E       */  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /* _5_E          */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOOF_E
    /* _01_E         */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFOOOO_E
    /* _45_E         */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOFF_E
    /* _0123_E       */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFOO_E
    /* _2345_E       */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOFFFF_E
    /* _01234_E      */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFO_E
    /* _ALL_E        */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFF_E
    /* _LEGACY_3_E   */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOF_E
    /* _LEGACY_012_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFO_E
    /* _LEGACY_ALL_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFF_E
    /* _LEGACY_TTI_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
};

static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT seg_columns_to_row_state_dragged_free[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E] =
{
    /* _NONE_E       */  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /* _5_E          */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFD_E
    /* _01_E         */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDFFFF_E
    /* _45_E         */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFDD_E
    /* _0123_E       */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDFF_E
    /* _2345_E       */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFDDDD_E
    /* _01234_E      */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDDF_E
    /* _ALL_E        */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDDD_E
    /* _LEGACY_3_E   */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFD_E
    /* _LEGACY_012_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDF_E
    /* _LEGACY_ALL_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDD_E
    /* _LEGACY_TTI_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
};

static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT seg_columns_to_row_state_dragged_other[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E] =
{
    /* _NONE_E       */  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /* _5_E          */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOOD_E
    /* _01_E         */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDOOOO_E
    /* _45_E         */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOODD_E
    /* _0123_E       */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDOO_E
    /* _2345_E       */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OODDDD_E
    /* _01234_E      */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDDO_E
    /* _ALL_E        */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDDD_E
    /* _LEGACY_3_E   */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOD_E
    /* _LEGACY_012_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDO_E
    /* _LEGACY_ALL_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDD_E
    /* _LEGACY_TTI_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
};

static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT seg_columns_to_row_state_other_free[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E] =
{
    /* _NONE_E       */  PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /* _5_E          */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFO_E
    /* _01_E         */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOFFFF_E
    /* _45_E         */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFOO_E
    /* _0123_E       */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOFF_E
    /* _2345_E       */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFOOOO_E
    /* _01234_E      */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOOF_E
    /* _ALL_E        */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /* _LEGACY_3_E   */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFO_E
    /* _LEGACY_012_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOF_E
    /* _LEGACY_ALL_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
    /* _LEGACY_TTI_E */, PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E
};

/**
* @struct PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_STC
 *
 * @brief This structure of horizontal block row DRAG status changing.
*/
typedef struct{

    /** @brief amount of such horizontal block rows.
     *  used as repeater when this structure is a part of Dragging Rule.
     */
    GT_U32 repeater;

    /** source status index. */
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT source;

    /** @brief target status index.
     *  Comments:
     */
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT target;

} PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_STC;

/**
* @struct PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC
 *
 * @brief This structure of horizontal block row DRAG status changing.
 * Dragging rule represents moving of some space used by dragged vTcam to
 * another place as changing status
 * status of an old place from "used by dragged vTcam" to "free" and
 * status of a new space from "used by dragged vTcam" to "free"
 * The amount of old space and new space should be the same.
*/
typedef struct{

    /** amount of change status elements. */
    GT_U32 changeArrSize;

    /** @brief (pointer to)array of change status elements.
     *  Comments:
     */
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_STC* changeArr;

} PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC;

typedef PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC
    *PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR;

/* rule definition format:                                             */
/* _H(id) _S(emement1), _S(emement2) ... _R(id, name)                  */
/*_id - part of internal name to be used only once for pair _H and __R */

#ifdef _H
#undef _H
#endif /*_H*/
#ifdef _S
#undef _S
#endif /*_S*/
#ifdef _R
#undef _R
#endif /*_R*/

/* macro for short Rule Change Structure contents                                  */
#define _S(_rep, _src_col, _trg_col)                              \
    {                                                             \
        _rep,                                                     \
        PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_##_src_col##_E,  \
        PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_##_trg_col##_E   \
    }

#define _H(_id)                                                                 \
    static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_STC __drag_rule_arr_##_id[] = {
#define _R(_id,_rule)                                                           \
    };                                                                          \
    static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC _rule =           \
    {                                                                           \
        (sizeof(__drag_rule_arr_##_id) / sizeof(__drag_rule_arr_##_id[0])),     \
        &(__drag_rule_arr_##_id[0])                                             \
    };

/* rule definitions */

/* get 60 without others */
_H(a1) _S(1, FFFFFD, DDDDDD), _S(5, FFFFFD, FFFFFF) _R(a1, rule_6FFFFFD_to_1DDDDDD_5FFFFFF)
_H(a2) _S(1, FFFFDD, DDDDDD), _S(4, FFFFFD, FFFFFF) _R(a2, rule_1FFFFDD_4FFFFFD_to_1DDDDDD_4FFFFFF)
_H(a3) _S(1, FFFFDD, DDDDDD), _S(2, FFFFDD, FFFFFF) _R(a3, rule_3FFFFDD_to_1DDDDDD_2FFFFFF)
_H(a4) _S(1, FFDDDD, DDDDDD), _S(2, FFFFFD, FFFFFF) _R(a4, rule_1FFDDDD_2FFFFFD_to_1DDDDDD_2FFFFFF)
_H(a5) _S(1, FFDDDD, DDDDDD), _S(1, FFFFDD, FFFFFF) _R(a5, rule_1FFDDDD_2FFFFDD_to_1DDDDDD_1FFFFFF)
_H(a6) _S(1, FFFFDD, DDDDDD), _S(1, FFFFDD, FFFFFF), _S(2, FFFFFD, FFFFFF)
    _R(a6, rule_2FFFFDD_2FFFFFD_to_1DDDDDD_3FFFFFF)
/* get 60 with others */
_H(b1) _S(1, OOOOOF, OOOOOD), _S(1, FFFFFD, FFFFFF) _R(b1, rule_1OOOOOF_1FFFFFD_to_1OOOOOD_1FFFFFF)
_H(b2) _S(2, OOOOOF, OOOOOD), _S(1, FFFFDD, FFFFFF) _R(b2, rule_2OOOOOF_1FFFFDD_to_2OOOOOD_1FFFFFF)
_H(b3) _S(1, OOOOFF, OOOODD), _S(1, FFFFDD, FFFFFF) _R(b3, rule_1OOOOFF_1FFFFDD_to_1OOOODD_1FFFFFF)
_H(b4) _S(1, OOOOFF, OOOODD), _S(2, FFFFFD, FFFFFF) _R(b4, rule_1OOOOFF_2FFFFFD_to_1OOOODD_2FFFFFF)
_H(b5) _S(1, OOFFFF, OODDDD), _S(4, FFFFFD, FFFFFF) _R(b5, rule_1OOFFFF_4FFFFFD_to_1OODDDD_4FFFFFF)
_H(b6) _S(1, OOFFFF, OODDDD), _S(2, FFFFFD, FFFFFF), _S(1, FFFFDD, FFFFFF)
    _R(b6, rule_1OOFFFF_2FFFFFD_1FFFFDD_to_1OODDDD_3FFFFFF)
_H(b7) _S(1, OOFFFF, OODDDD), _S(2, FFFFDD, FFFFFF) _R(b7, rule_1OOFFFF_2FFFFDD_to_1OODDDD_2FFFFFF)
_H(b8) _S(1, OOFFFF, OODDDD), _S(1, FFDDDD, FFFFFF) _R(b8, rule_1OOFFFF_1FFDDDD_to_1OODDDD_1FFFFFF)
_H(b9) _S(2, OOOOFF, OOOODD), _S(1, FFDDDD, FFFFFF) _R(b9, rule_2OOOOFF_1FFDDDD_to_2OOOODD_1FFFFFF)
_H(b10) _S(2, OOOOOF, OOOOOD), _S(1, OOOOFF, OOOODD), _S(1, FFDDDD, FFFFFF)
    _R(b10, rule_2OOOOOF_1OOOOFF_1FFDDDD_to_2OOOOOD_1OOOODD_1FFFFFF)
_H(b11) _S(6, OOOOOF, OOOOOD), _S(1, DDDDDD, FFFFFF) _R(b11, rule_6OOOOOF_1DDDDDD_to_6OOOOOD_1FFFFFF)
_H(b12) _S(3, OOOOFF, OOOODD), _S(1, DDDDDD, FFFFFF) _R(b12, rule_3OOOOFF_1DDDDDD_to_3OOOODD_1FFFFFF)
_H(b13) _S(2, OOOOOF, OOOOOD), _S(2, OOOOFF, OOOODD), _S(1, DDDDDD, FFFFFF)
    _R(b13, rule_2OOOOOF_2OOOOFF_1DDDDDD_to_2OOOOOD_2OOOOOD_1FFFFFF)
_H(b14) _S(4, OOOOOF, OOOOOD), _S(1, OOOOFF, OOOODD), _S(1, DDDDDD, FFFFFF)
    _R(b14, rule_4OOOOOF_1OOOOFF_1DDDDDD_to_4OOOOOD_1OOOODD_1FFFFFF)

/* get 50 without others */
_H(c1) _S(1, FFFFDD, DDDDDD), _S(4, FFFFDD, FFFFFD) _R(c1, rule_5FFFFDD_to_1DDDDDD_4FFFFFD)
_H(c2) _S(1, FFDDDD, DDDDDD), _S(2, FFFFDD, FFFFFD) _R(c2, rule_1FFDDDD_2FFFFDD_to_1DDDDDD_2FFFFFD)
_H(c3) _S(1, FFFFFF, FFFFFD), _S(1, FFFFDD, FFFFFD) _R(c3, rule_1FFFFFF_1FFFFDD_to_2FFFFFD)
_H(c4) _S(3, FFDDDD, DDDDDD), _S(2, FFDDDD, FFFFFD) _R(c4, rule_5FFDDDD_to_3DDDDDD_2FFFFFD)
_H(c5) _S(1, FFFFFF, DDDDDD), _S(2, FFDDDD, FFFFFD) _R(c5, rule_1FFFFFF_2FFDDDD_to_1DDDDD_2FFFFFD)
_H(c51) _S(1, DDDDDD, FFFFFD), _S(5, FFFFFF, FFFFFD) _R(c51, rule_5FFFFFF_to_1DDDDDD_6FFFFFD)
/* get 50 with others */
_H(c6) _S(1, OOOOOF, OOOOOD), _S(1, FFFFDD, FFFFFD) _R(c6, rule_1OOOOOF_1FFFFDD_to_1OOOOOD_1FFFFFD)
_H(c7) _S(1, OOOOFF, OOOODD), _S(2, FFFFDD, FFFFFD) _R(c7, rule_1OOOOFF_2FFFFDD_to_1OOOODD_2FFFFFD)
_H(c8) _S(1, OOFFFF, OODDDD), _S(4, FFFFDD, FFFFFD) _R(c8, rule_1OOFFFF_4FFFFDD_to_1OODDDD_4FFFFFD)
_H(c9) _S(3, OOOOOF, OOOOOD), _S(1, FFDDDD, FFFFFD) _R(c9, rule_3OOOOOF_1FFDDDD_to_3OOOOOD_1FFFFFD)
_H(c10) _S(3, OOOOFF, OOOODD), _S(2, FFDDDD, FFFFFD) _R(c10, rule_3OOOOFF_2FFDDDD_to_3OOOODD_2FFFFFD)
_H(c11) _S(3, OOFFFF, OODDDD), _S(4, FFDDDD, FFFFFD) _R(c11, rule_3OOFFFF_4FFDDDD_to_3OODDDD_4FFFFFD)
_H(c12) _S(5, OOOOOF, OOOOOD), _S(1, DDDDDD, FFFFFD) _R(c12, rule_5OOOOOF_1DDDDDD_to_5OOOOOD_1FFFFFD)
_H(c13) _S(1, OOOOOF, OOOOOD), _S(2, OOOOFF, OOOODD), _S(1, DDDDDD, FFFFFD)
    _R(c13, rule_1OOOOOF_2OOOOFF_1DDDDDD_to_1OOOODD_2OOOODD_1FFFFFD)
_H(c14) _S(3, OOOOOF, OOOOOD), _S(1, OOOOFF, OOOODD), _S(1, DDDDDD, FFFFFD)
    _R(c14, rule_3OOOOOF_1OOOOFF_1DDDDDD_to_3OOOODD_1OOOODD_1FFFFFD)
_H(c15) _S(5, OOOOFF, OOOOOD), _S(2, DDDDDD, FFFFFD) _R(c15, rule_5OOOOFF_2DDDDDD_to_5OOOODD_2FFFFFD)
_H(c16) _S(1, OOOOOF, OOOOOD), _S(1, OOFFFF, OODDDD), _S(1, DDDDDD, FFFFFD)
    _R(c16, rule_1OOOOOF_1OOFFFF_1DDDDDD_to_1OOOOOD_1OODDDD_1FFFFFD)
_H(c17) _S(5, OOFFFF, OODDDD), _S(4, DDDDDD, FFFFFD) _R(c17, rule_5OOFFFF_4DDDDDD_to_5OODDDD_4FFFFFD)

/* get 40 without others */
_H(d1) _S(1, FFFFFD, FFFFFF), _S(1, FFFFFD, FFFFDD) _R(d1, rule_2FFFFFD_to_1FFFFFF_1FFFFDD)
_H(d2) _S(1, FFDDDD, DDDDDD), _S(1, FFDDDD, FFFFDD) _R(d2, rule_2FFDDDD_to_1DDDDDD_1FFFFDD)
_H(d3) _S(1, DDDDDD, FFFFDD), _S(2, FFFFFF, FFFFDD) _R(d3, rule_1DDDDDD_2FFFFFF_to_3FFFFDD)
/* get 40 with others */
_H(d4) _S(1, OOFFFF, OODDDD), _S(1, DDDDDD, FFFFDD) _R(d4, rule_1OOFFFF_1DDDDDD_to_1OODDDD_1FFFFDD)
_H(d5) _S(2, OOOOFF, OOOODD), _S(1, DDDDDD, FFFFDD) _R(d5, rule_2OOOOFF_1DDDDDD_to_2OOOODD_1FFFFDD)
_H(d6) _S(4, OOOOOF, OOOOOD), _S(1, DDDDDD, FFFFDD) _R(d6, rule_4OOOOOF_1DDDDDD_to_4OOOOOD_1FFFFDD)
_H(d7) _S(2, OOOOOF, OOOOOD), _S(1, OOOOFF, OOOODD), _S(1, DDDDDD, FFFFDD)
    _R(d7, rule_2OOOOOF_1OOOOFF_1DDDDDD_to_2OOOOOD_1OOOODD_1FFFFDD)
_H(d8) _S(1, OOOOFF, OOOODD), _S(1, FFDDDD, FFFFDD) _R(d8, rule_1OOOOFF_1FFDDDD_to_1OOOODD_1FFFFDD)
_H(d9) _S(2, OOOOOF, OOOOOD), _S(1, FFDDDD, FFFFDD) _R(d9, rule_2OOOOOF_1FFDDDD_to_2OOOOOD_1FFFFDD)

/* special rules for FFDDDD where DDDD contains 30-byte rule */
_H(e1) _S(1, FFDDDD, DDDDDD), _S(1, FFDDDD, FFFFFF) _R(e1, rule_30byte_2FFDDDD_to_1DDDDDD_1FFFFFF)
_H(e2) _S(2, OODDDD, OOFFFF), _S(1, FFFFFF, DDDDDD) _R(e2, rule_30byte_2OODDDD_1FFFFFF_to_2OOFFFF_1DDDDDD)
_H(e3) _S(1, DDDDDD, FFDDDD), _S(1, FFFFFF, FFDDDD) _R(e3, rule_30byte_1DDDDDD_1FFFFFF_to_2FFDDDD)

/* legacy devices - ultra TCAM rule related conversion rules */
/* 1 Full row + 3 Free rows => 4 free places for ultra rules */
_H(f1) _S(1, DDDD, FFFD), _S(3, FFFF, FFFD ) _R(f1, rule_legacy_1DDDD_3FFFF_to_4FFFD)
/* 4 remainders of ultra rules  =>  1 Full row + 3 Free rows */
_H(f2) _S(1, FFFD, DDDD), _S(3, FFFD, FFFF)_R(f2, rule_legacy_4FFFD_to_1DDDD_3FFFF)

/* delete temporary macros */
#undef _H
#undef _S
#undef _R

static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR rules_target_60_dragged_10_arr[] =
{
    &rule_6FFFFFD_to_1DDDDDD_5FFFFFF,
    &rule_1FFFFDD_4FFFFFD_to_1DDDDDD_4FFFFFF,
    &rule_3FFFFDD_to_1DDDDDD_2FFFFFF,
    &rule_1FFDDDD_2FFFFFD_to_1DDDDDD_2FFFFFF,
    &rule_1FFDDDD_2FFFFDD_to_1DDDDDD_1FFFFFF,
    &rule_2FFFFDD_2FFFFFD_to_1DDDDDD_3FFFFFF,
    &rule_1OOOOOF_1FFFFFD_to_1OOOOOD_1FFFFFF,
    &rule_2OOOOOF_1FFFFDD_to_2OOOOOD_1FFFFFF,
    &rule_1OOOOFF_1FFFFDD_to_1OOOODD_1FFFFFF,
    &rule_1OOOOFF_2FFFFFD_to_1OOOODD_2FFFFFF,
    &rule_1OOFFFF_4FFFFFD_to_1OODDDD_4FFFFFF,
    &rule_1OOFFFF_2FFFFFD_1FFFFDD_to_1OODDDD_3FFFFFF,
    &rule_1OOFFFF_2FFFFDD_to_1OODDDD_2FFFFFF,
    &rule_1OOFFFF_1FFDDDD_to_1OODDDD_1FFFFFF,
    &rule_2OOOOFF_1FFDDDD_to_2OOOODD_1FFFFFF,
    &rule_2OOOOOF_1OOOOFF_1FFDDDD_to_2OOOOOD_1OOOODD_1FFFFFF,
    &rule_6OOOOOF_1DDDDDD_to_6OOOOOD_1FFFFFF,
    &rule_3OOOOFF_1DDDDDD_to_3OOOODD_1FFFFFF,
    &rule_2OOOOOF_2OOOOFF_1DDDDDD_to_2OOOOOD_2OOOOOD_1FFFFFF,
    &rule_4OOOOOF_1OOOOFF_1DDDDDD_to_4OOOOOD_1OOOODD_1FFFFFF
};

static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR rules_target_60_dragged_30_arr[] =
{
    &rule_30byte_2FFDDDD_to_1DDDDDD_1FFFFFF
};

static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR rules_target_50_dragged_10_arr[] =
{
    &rule_5FFFFDD_to_1DDDDDD_4FFFFFD,
    &rule_1FFDDDD_2FFFFDD_to_1DDDDDD_2FFFFFD,
    &rule_1FFFFFF_1FFFFDD_to_2FFFFFD,
    &rule_5FFDDDD_to_3DDDDDD_2FFFFFD,
    &rule_1FFFFFF_2FFDDDD_to_1DDDDD_2FFFFFD,
    &rule_5FFFFFF_to_1DDDDDD_6FFFFFD,
    &rule_1OOOOOF_1FFFFDD_to_1OOOOOD_1FFFFFD,
    &rule_1OOOOFF_2FFFFDD_to_1OOOODD_2FFFFFD,
    &rule_1OOFFFF_4FFFFDD_to_1OODDDD_4FFFFFD,
    &rule_3OOOOOF_1FFDDDD_to_3OOOOOD_1FFFFFD,
    &rule_3OOOOFF_2FFDDDD_to_3OOOODD_2FFFFFD,
    &rule_3OOFFFF_4FFDDDD_to_3OODDDD_4FFFFFD,
    &rule_5OOOOOF_1DDDDDD_to_5OOOOOD_1FFFFFD,
    &rule_1OOOOOF_2OOOOFF_1DDDDDD_to_1OOOODD_2OOOODD_1FFFFFD,
    &rule_3OOOOOF_1OOOOFF_1DDDDDD_to_3OOOODD_1OOOODD_1FFFFFD,
    &rule_5OOOOFF_2DDDDDD_to_5OOOODD_2FFFFFD,
    &rule_1OOOOOF_1OOFFFF_1DDDDDD_to_1OOOOOD_1OODDDD_1FFFFFD,
    &rule_5OOFFFF_4DDDDDD_to_5OODDDD_4FFFFFD
};

static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR rules_target_40_dragged_10_arr[] =
{
    &rule_2FFFFFD_to_1FFFFFF_1FFFFDD,
    &rule_2FFDDDD_to_1DDDDDD_1FFFFDD,
    &rule_1DDDDDD_2FFFFFF_to_3FFFFDD,
    &rule_1OOFFFF_1DDDDDD_to_1OODDDD_1FFFFDD,
    &rule_2OOOOFF_1DDDDDD_to_2OOOODD_1FFFFDD,
    &rule_4OOOOOF_1DDDDDD_to_4OOOOOD_1FFFFDD,
    &rule_2OOOOOF_1OOOOFF_1DDDDDD_to_2OOOOOD_1OOOODD_1FFFFDD,
    &rule_1OOOOFF_1FFDDDD_to_1OOOODD_1FFFFDD,
    &rule_2OOOOOF_1FFDDDD_to_2OOOOOD_1FFFFDD
};

static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR rules_miscellanous_arr[] =
{
    &rule_30byte_2OODDDD_1FFFFFF_to_2OOFFFF_1DDDDDD,
    &rule_30byte_1DDDDDD_1FFFFFF_to_2FFDDDD
};

static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR rules_legacy_arr[] =
{
    &rule_legacy_1DDDD_3FFFF_to_4FFFD,
    &rule_legacy_4FFFD_to_1DDDD_3FFFF
};

/* used for dynamic built rule list */
static PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR work_rules_arr[100];
static GT_U32 work_rules_arr_used;

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceRowStatusContainsFree function
* @endinternal
*
* @brief   Checks whether Horizontal Block Row contains free space.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] statusIndex              - Horizontal Block Status index
*
* @retval 1                        - contains, 0 - does not contain (or index out of range).
*/
GT_U8 prvCpssDxChVirtualTcamDbSegSpaceRowStatusContainsFree
(
    IN     PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT      statusIndex
)
{
    if (statusIndex > PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        return 0;
    }
    return prvIndexToHorzBlockRowContainsFree[statusIndex];
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceRowStatusContainsDragged function
* @endinternal
*
* @brief   Checks whether Horizontal Block Row contains derugged space.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] statusIndex              - Horizontal Block Status index
*
* @retval 1                        - contains, 0 - does not contain (or index out of range).
*/
GT_U8 prvCpssDxChVirtualTcamDbSegSpaceRowStatusContainsDragged
(
    IN     PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT      statusIndex
)
{
    if (statusIndex > PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        return 0;
    }
    return prvIndexToHorzBlockRowContainsDragged[statusIndex];
}

/* these macros used to pack/unpack the range of horizontal block rows */
/* to VOID pointer used as data in AVL trees                           */

#define PRV_GT_U32_TO_VOIDPTR_MAC(_gt_u32) \
    (void*)((_gt_u32) + (char*)0)
#define PRV_VOIDPTR_TO_GT_U32_MAC(_void_ptr) \
    (GT_U32)((char*)(_void_ptr) - (char*)0)
/* _freeBlock - 1 bit, _horzBlockIndex - 3 bits, */
/* _rowsBase - 16 bits, _rowsAmount - 12 bits    */
#define PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(             \
    _freeBlock, _horzBlockIndex, _rowsBase, _rowsAmount)      \
    PRV_GT_U32_TO_VOIDPTR_MAC(                                \
        ((_freeBlock) << 31) | ((_horzBlockIndex) << 28)      \
        | ((_rowsBase) << 12) | (_rowsAmount))

#define PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(                 \
    _voidPtr, _freeBlock, _horzBlockIndex, _rowsBase, _rowsAmount)  \
    {                                                               \
        GT_U32 __data = PRV_VOIDPTR_TO_GT_U32_MAC(_voidPtr);        \
        _freeBlock      = ((__data >> 31) & 1);                     \
        _horzBlockIndex = ((__data >> 28) & 0x7);                   \
        _rowsBase       = ((__data >> 12) & 0xFFFF);                \
        _rowsAmount     = (__data & 0xFFF);                         \
    }

/* similar without _freeBlock but with  priority (_RANGE4_) */
/* and without priority (_RANGE3_)                          */

/* _priority - 4 bits, _horzBlockIndex - 3 bits, */
/* _rowsBase - 16 bits, _rowsAmount - 9 bits     */
#define PRV_HORZ_BLOCK_ROWS_RANGE4_TO_VOIDPTR_MAC(                  \
    _priority, _horzBlockIndex, _rowsBase, _rowsAmount)             \
    PRV_GT_U32_TO_VOIDPTR_MAC(                                      \
        ((_priority) << 28) | ((_horzBlockIndex) << 25)             \
        | ((_rowsBase) << 9) | (_rowsAmount))

#define PRV_HORZ_BLOCK_ROWS_RANGE3_TO_VOIDPTR_MAC(                  \
    _horzBlockIndex, _rowsBase, _rowsAmount)                        \
    PRV_GT_U32_TO_VOIDPTR_MAC(                                      \
        ((_horzBlockIndex) << 25)                                   \
        | ((_rowsBase) << 9) | (_rowsAmount))

#define PRV_HORZ_BLOCK_ROWS_RANGE4_FROM_VOIDPTR_MAC(                \
    _voidPtr, _priority, _horzBlockIndex, _rowsBase, _rowsAmount)   \
    {                                                               \
        GT_U32 __data = PRV_VOIDPTR_TO_GT_U32_MAC(_voidPtr);        \
        _priority       = ((__data >> 28) & 0xF);                   \
        _horzBlockIndex = ((__data >> 25) & 0x7);                   \
        _rowsBase       = ((__data >> 9) & 0xFFFF);                 \
        _rowsAmount     = (__data & 0x1FF);                         \
    }

#define PRV_HORZ_BLOCK_ROWS_RANGE3_FROM_VOIDPTR_MAC(                \
    _voidPtr,  _horzBlockIndex, _rowsBase, _rowsAmount)             \
    {                                                               \
        GT_U32 __data = PRV_VOIDPTR_TO_GT_U32_MAC(_voidPtr);        \
        _horzBlockIndex = ((__data >> 25) & 0x7);                   \
        _rowsBase       = ((__data >> 9) & 0xFFFF);                 \
        _rowsAmount     = (__data & 0x1FF);                         \
    }


/**
* @struct PRV_SEG_DRAG_WORK_DB_STC
 *
 * @brief This structure of DRAG work database to evaluate positon changes.
*/
typedef struct{

    /** pool for nodes of all trees in rangeTreeArr (below). */
    CPSS_BM_POOL_ID rangeTreeNodesPool;

    PRV_CPSS_AVL_TREE_ID rangeTreeArr[PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E];

    GT_U32 horzBlockRowsCounter[PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E];

} PRV_SEG_DRAG_WORK_DB_STC;

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbCompareFunc function
* @endinternal
*
* @brief   AVL Tree compare function for horizontal block row status ranges.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] data_ptr1                - (pointer to) first horizontal block row status range.
* @param[in] data_ptr2                - (pointer to) second horizontal block row status range.
*                                       GT_EQUAL, GT_GREATER or GT_SMALLER.
*/
GT_COMP_RES prvCpssDxChVirtualTcamDbSegSpaceWorkDbCompareFunc
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    GT_U32 freeBlock1, horzBlockIndex1, rowsBase1, rowsAmount1;
    GT_U32 freeBlock2, horzBlockIndex2, rowsBase2, rowsAmount2;

    PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
        data_ptr1, freeBlock1, horzBlockIndex1, rowsBase1, rowsAmount1);
    PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
        data_ptr2, freeBlock2, horzBlockIndex2, rowsBase2, rowsAmount2);

    /* not used values */
    rowsAmount1 = rowsAmount1;
    rowsAmount2 = rowsAmount2;

    if (freeBlock1 < freeBlock2) return GT_SMALLER;
    if (freeBlock1 > freeBlock2) return GT_GREATER;
    if (horzBlockIndex1 < horzBlockIndex2) return GT_SMALLER;
    if (horzBlockIndex1 > horzBlockIndex2) return GT_GREATER;
    if (rowsBase1 < rowsBase2) return GT_SMALLER;
    if (rowsBase1 > rowsBase2) return GT_GREATER;

    return GT_EQUAL;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbSourceCompareFunc function
* @endinternal
*
* @brief   AVL Tree compare function for horizontal block row status ranges.
*         Ignores priority field.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] data_ptr1                - (pointer to) first horizontal block row status range.
* @param[in] data_ptr2                - (pointer to) second horizontal block row status range.
*                                       GT_EQUAL, GT_GREATER or GT_SMALLER.
*/
GT_COMP_RES prvCpssDxChVirtualTcamDbSegSpaceWorkDbSourceCompareFunc
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    GT_U32 horzBlockIndex1, rowsBase1, rowsAmount1;
    GT_U32 horzBlockIndex2, rowsBase2, rowsAmount2;

    PRV_HORZ_BLOCK_ROWS_RANGE3_FROM_VOIDPTR_MAC(
        data_ptr1, horzBlockIndex1, rowsBase1, rowsAmount1);
    PRV_HORZ_BLOCK_ROWS_RANGE3_FROM_VOIDPTR_MAC(
        data_ptr2, horzBlockIndex2, rowsBase2, rowsAmount2);

    /* not used values */
    rowsAmount1 = rowsAmount1;
    rowsAmount2 = rowsAmount2;

    if (horzBlockIndex1 < horzBlockIndex2) return GT_SMALLER;
    if (horzBlockIndex1 > horzBlockIndex2) return GT_GREATER;
    if (rowsBase1 < rowsBase2) return GT_SMALLER;
    if (rowsBase1 > rowsBase2) return GT_GREATER;

    return GT_EQUAL;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbTargetCompareFunc function
* @endinternal
*
* @brief   AVL Tree compare function for horizontal block row status ranges.
*         Compares priority field.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] data_ptr1                - (pointer to) first horizontal block row status range.
* @param[in] data_ptr2                - (pointer to) second horizontal block row status range.
*                                       GT_EQUAL, GT_GREATER or GT_SMALLER.
*/
GT_COMP_RES prvCpssDxChVirtualTcamDbSegSpaceWorkDbTargetCompareFunc
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    GT_U32 priority1, horzBlockIndex1, rowsBase1, rowsAmount1;
    GT_U32 priority2, horzBlockIndex2, rowsBase2, rowsAmount2;

    PRV_HORZ_BLOCK_ROWS_RANGE4_FROM_VOIDPTR_MAC(
        data_ptr1, priority1, horzBlockIndex1, rowsBase1, rowsAmount1);
    PRV_HORZ_BLOCK_ROWS_RANGE4_FROM_VOIDPTR_MAC(
        data_ptr2, priority2, horzBlockIndex2, rowsBase2, rowsAmount2);

    /* not used values */
    rowsAmount1 = rowsAmount1;
    rowsAmount2 = rowsAmount2;

    if (priority1 < priority2) return GT_SMALLER;
    if (priority1 > priority2) return GT_GREATER;
    if (rowsBase1 < rowsBase2) return GT_SMALLER;
    if (rowsBase1 > rowsBase2) return GT_GREATER;
    if (horzBlockIndex1 < horzBlockIndex2) return GT_SMALLER;
    if (horzBlockIndex1 > horzBlockIndex2) return GT_GREATER;

    return GT_EQUAL;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbRangeInsert function
* @endinternal
*
* @brief   Insert range of Horisontal block rows in the tree of Work DB.
*         Range merged with vertical neighbors if found.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] treeId                   - thee to insert the range
* @param[in] rangeDataPtr             - range compressed to GT_VOID
*
* @retval GT_OK                    - success.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceWorkDbRangeInsert
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                          treeId,
    IN     GT_VOID                                       *rangeDataPtr
)
{
    GT_BOOL   found;
    GT_VOID   *rangeData1Ptr;
    GT_VOID   *rangeDataResultPtr;
    GT_U32    freeBlock, horzBlockIndex, rowsBase, rowsAmount;
    GT_U32    freeBlock1, horzBlockIndex1, rowsBase1, rowsAmount1;
    GT_U32    blockRowBase, blockHighBound;
    GT_STATUS rc;
    PRV_CPSS_AVL_TREE_PATH     path;

    rangeDataResultPtr = rangeDataPtr;
    PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
        rangeDataResultPtr, freeBlock, horzBlockIndex, rowsBase, rowsAmount);

    blockRowBase = rowsBase - (rowsBase % tcamSegCfgPtr->lookupRowAlignment);
    blockHighBound = blockRowBase + tcamSegCfgPtr->lookupRowAlignment;

    if (rowsBase > blockRowBase) /* not equal - check needed */
    {
        rangeData1Ptr = NULL;
        found = prvCpssAvlPathFind(
            treeId, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
            rangeDataResultPtr, path, &rangeData1Ptr);
        PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
            rangeData1Ptr, freeBlock1, horzBlockIndex1, rowsBase1, rowsAmount1);
        if ((found != GT_FALSE)
            && (horzBlockIndex == horzBlockIndex1)
            && (rowsBase1 >= blockRowBase)
            && (rowsBase1 < blockHighBound))
        {
            if ((freeBlock1 != freeBlock)
                || ((rowsBase1 + rowsAmount1) > rowsBase))
            {
                /* BAD_STATE obtained */
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            if ((rowsBase1 + rowsAmount1) == rowsBase)
            {
                rangeData1Ptr = prvCpssAvlItemRemove(treeId, rangeData1Ptr);
                if (rangeData1Ptr == NULL)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                rowsBase = rowsBase1;
                rowsAmount += rowsAmount1;
                rangeDataResultPtr =
                    PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                        freeBlock, horzBlockIndex, rowsBase, rowsAmount);
            }
        }
    }

    if ((rowsBase + rowsAmount) < blockHighBound) /* not equal - check needed */
    {
        found = prvCpssAvlPathFind(
            treeId, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
            rangeDataResultPtr, path, &rangeData1Ptr);
        PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
            rangeData1Ptr, freeBlock1, horzBlockIndex1, rowsBase1, rowsAmount1);
        if ((found != GT_FALSE)
            && (horzBlockIndex == horzBlockIndex1)
            && (rowsBase1 >= blockRowBase)
            && (rowsBase1 < blockHighBound))
        {
            if ((freeBlock != freeBlock1)
                || ((rowsBase + rowsAmount) > rowsBase1))
            {
                /* BAD_STATE obtained */
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            if ((rowsBase + rowsAmount) == rowsBase1)
            {
                rangeData1Ptr = prvCpssAvlItemRemove(treeId, rangeData1Ptr);
                if (rangeData1Ptr == NULL)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                rowsAmount += rowsAmount1;
                rangeDataResultPtr =
                    PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                        freeBlock, horzBlockIndex, rowsBase, rowsAmount);
            }
        }
    }

    rc = prvCpssAvlItemInsert(treeId, rangeDataResultPtr);
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbFree function
* @endinternal
*
* @brief   Free Drag Work DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] workDbPtr                - (pointer to) Work DB
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbSegSpaceWorkDbFree
(
    IN    PRV_SEG_DRAG_WORK_DB_STC                      *workDbPtr
)
{
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT index;

    for (index = 0; (index < PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E); index++)
    {
        if (workDbPtr->rangeTreeArr[index] != NULL)
        {
            prvCpssAvlTreeDelete(
                workDbPtr->rangeTreeArr[index],
                (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
            workDbPtr->rangeTreeArr[index] = NULL;
        }
        workDbPtr->horzBlockRowsCounter[index] = 0;
    }

    if (workDbPtr->rangeTreeNodesPool)
    {
        prvCpssAvlMemPoolDelete(workDbPtr->rangeTreeNodesPool);
        workDbPtr->rangeTreeNodesPool = NULL;
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbInit function
* @endinternal
*
* @brief   Initialize Drag Work DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup id of treated space
*
* @param[out] workDbPtr                - (pointer to) Work DB
*
* @retval GT_OK                    - success.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceWorkDbInit
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                        lookupId,
    OUT    PRV_SEG_DRAG_WORK_DB_STC                      *workDbPtr
)
{
    GT_STATUS                                       rc;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT    index;
    GT_VOID                                         *treeDataPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC segPattern;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   segmentColumns;
    GT_U32                                          blocksInRow;
    GT_U32                                          horzBlockIdx;
    GT_BOOL                                         seeked;
    PRV_CPSS_AVL_TREE_PATH                          path;
    GT_VOID_PTR                                     dbEntryPtr;

    /* clear before initialization */
    cpssOsMemSet(workDbPtr, 0, sizeof(PRV_SEG_DRAG_WORK_DB_STC));

    rc = prvCpssAvlMemPoolCreate(
        (tcamSegCfgPtr->tcamColumnsAmount
            * tcamSegCfgPtr->tcamRowsAmount)/*maxNode*/,
        &(workDbPtr->rangeTreeNodesPool));
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        return rc;
    }

    for (index = 0; (index < PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E); index++)
    {
        rc = prvCpssAvlTreeCreate(
            (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegSpaceWorkDbCompareFunc,
            workDbPtr->rangeTreeNodesPool,
            &(workDbPtr->rangeTreeArr[index]));
        if (rc != GT_OK)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            prvCpssDxChVirtualTcamDbSegSpaceWorkDbFree(workDbPtr);
            return rc;
        }
    }

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    /* mark all free places in given lookup blocks */

    cpssOsMemSet(&segPattern, 0, sizeof(segPattern));
    segPattern.lookupId = lookupId;
    segmentColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);
    for (horzBlockIdx = 0; (horzBlockIdx < blocksInRow); horzBlockIdx++)
    {
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            segPattern.segmentColumnsMap, horzBlockIdx, segmentColumns);
    }

    seeked = prvCpssAvlPathFind(
        tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
        &segPattern, path, &dbEntryPtr);
    segPtr = dbEntryPtr;
    while ((seeked != GT_FALSE) && (segPtr->lookupId == lookupId))
    {
        /* mark any free place as mixed "free" and "other" */
        for (horzBlockIdx = 0; (horzBlockIdx < blocksInRow); horzBlockIdx++)
        {
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segPtr->segmentColumnsMap, horzBlockIdx);

            if (segmentColumns
                == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;

            treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                0 /*freeBlock*/, horzBlockIdx, segPtr->rowsBase, segPtr->rowsAmount);
            index = seg_columns_to_row_state_free_other[segmentColumns];
            rc = prvCpssAvlItemInsert(
                workDbPtr->rangeTreeArr[index],
                treeDataPtr);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                prvCpssDxChVirtualTcamDbSegSpaceWorkDbFree(workDbPtr);
                return rc;
            }
            workDbPtr->horzBlockRowsCounter[index] += segPtr->rowsAmount;
        }

        /* next */
        seeked = prvCpssAvlPathSeek(
            tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
            path, &dbEntryPtr);
        segPtr = dbEntryPtr;
    }

    /* mark all free places in free blocks */

    cpssOsMemSet(&segPattern, 0, sizeof(segPattern));
    segPattern.lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS;
    segmentColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);
    for (horzBlockIdx = 0; (horzBlockIdx < blocksInRow); horzBlockIdx++)
    {
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            segPattern.segmentColumnsMap, horzBlockIdx, segmentColumns);
    }

    seeked = prvCpssAvlPathFind(
        tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
        &segPattern, path, &dbEntryPtr);
    segPtr = dbEntryPtr;
    while ((seeked != GT_FALSE) &&
           (segPtr->lookupId == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS))
    {
        /* mark any free place as mixed "free" and "other" */
        for (horzBlockIdx = 0; (horzBlockIdx < blocksInRow); horzBlockIdx++)
        {
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segPtr->segmentColumnsMap, horzBlockIdx);

            if (segmentColumns
                == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;

            treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                1 /*freeBlock*/, horzBlockIdx, segPtr->rowsBase, segPtr->rowsAmount);
            index = seg_columns_to_row_state_free_other[segmentColumns];
            rc = prvCpssAvlItemInsert(
                workDbPtr->rangeTreeArr[index],
                treeDataPtr);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                prvCpssDxChVirtualTcamDbSegSpaceWorkDbFree(workDbPtr);
                return rc;
            }
            workDbPtr->horzBlockRowsCounter[index] += segPtr->rowsAmount;
        }

        /* next */
        seeked = prvCpssAvlPathSeek(
            tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
            path, &dbEntryPtr);
        segPtr = dbEntryPtr;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbDraggedToOther function
* @endinternal
*
* @brief   Convert All Dragged space to Other Space.
*         Done after one vTcam dragging before another vTcam dragging.
*         (One of changes to do another vTcam Dragged)
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] workDbPtr                - (pointer to) Work DB
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceWorkDbDraggedToOther
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     PRV_SEG_DRAG_WORK_DB_STC                      *workDbPtr
)
{
    GT_STATUS                                       rc;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT    index;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT    newIndex;
    GT_VOID                                         *treeDataPtr;
    PRV_CPSS_AVL_TREE_PATH                          path;

    for (index = 0; (index < PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E); index++)
    {
        if (prvIndexToHorzBlockRowContainsDragged[index] == 0) continue;
        newIndex = prvIndexToHorzBlockRowDraggedToOther[index];

        while (GT_FALSE != prvCpssAvlPathSeek(
            workDbPtr->rangeTreeArr[index], PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &treeDataPtr))
        {
            if (NULL == prvCpssAvlItemRemove(
                workDbPtr->rangeTreeArr[index], treeDataPtr))
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            if (newIndex == PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E) continue;
            rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRangeInsert(
                tcamSegCfgPtr,workDbPtr->rangeTreeArr[newIndex],treeDataPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        if (newIndex != PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E)
        {
            workDbPtr->horzBlockRowsCounter[newIndex] += workDbPtr->horzBlockRowsCounter[index];
        }
        workDbPtr->horzBlockRowsCounter[index] = 0;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbLoadVTcamSpaceRange function
* @endinternal
*
* @brief   Load vTcam space range to Work DB. The space searched and if find
*         converted from FreeAndOther to FreeAndDragged otherwise added as DraggedAndOther.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] workDbPtr                - (pointer to) Work DB
* @param[in] freeBlock                - 0 - old Lookup mapped block, 1 - free block
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceWorkDbLoadVTcamSpaceRange
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC  *tcamSegCfgPtr,
    IN     PRV_SEG_DRAG_WORK_DB_STC                       *workDbPtr,
    IN     GT_U32                                         freeBlock,
    IN     GT_U32                                         horzBlockIdx,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT  columns,
    IN     GT_U32                                         rowsBase,
    IN     GT_U32                                         rowsAmount
)
{
    GT_STATUS                                       rc;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT    draggedFreeIndex;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT    draggedOtherIndex;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT    otherFreeIndex;
    GT_VOID                                         *treeDataPtr;
    PRV_CPSS_AVL_TREE_PATH                          path;
    GT_VOID                                         *patternPtr;
    GT_VOID                                         *foundDataPtr;
    GT_BOOL                                         found;
    GT_U32                                          foundFreeBlock;
    GT_U32                                          foundHorzBlockIdx;
    GT_U32                                          foundRowsBase;
    GT_U32                                          foundRowsAmount;
    GT_U32                                          rowsBase1;
    GT_U32                                          rowsBase2;
    GT_U32                                          rowsAmount1;
    GT_U32                                          reachedRowsBase;
    GT_U32                                          reachedRowsAmount;

    if (rowsAmount == 0)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    if (columns >= PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    draggedFreeIndex  = seg_columns_to_row_state_dragged_free[columns];
    draggedOtherIndex = seg_columns_to_row_state_dragged_other[columns];
    otherFreeIndex    = seg_columns_to_row_state_other_free[columns];

    if (otherFreeIndex == PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E)
    {
        /* all space was BUSY and not kept in DB    */
        /* add the given space to draggedOther tree */
        treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
            freeBlock, horzBlockIdx, rowsBase, rowsAmount);
        rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRangeInsert(
            tcamSegCfgPtr,workDbPtr->rangeTreeArr[draggedOtherIndex],treeDataPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        workDbPtr->horzBlockRowsCounter[draggedOtherIndex] += rowsAmount;
        return GT_OK;
    }

    /* store given space as draggedFree or draggedOther */
    reachedRowsBase   = rowsBase;
    reachedRowsAmount = rowsAmount;
    patternPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
        freeBlock, horzBlockIdx, reachedRowsBase, reachedRowsAmount);

    /* treat range overlapping from low side */
    found = prvCpssAvlPathFind(
        workDbPtr->rangeTreeArr[otherFreeIndex],
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
        patternPtr, path, &foundDataPtr);
    if (found != GT_FALSE)
    {
        PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
            foundDataPtr, foundFreeBlock, foundHorzBlockIdx, foundRowsBase, foundRowsAmount);
        if ((freeBlock == foundFreeBlock)
            && (horzBlockIdx == foundHorzBlockIdx)
            && ((foundRowsBase + foundRowsAmount) >  reachedRowsBase))
        {
            /* add part found as orherFree to draggedFree */
            rowsBase1 = (reachedRowsBase + reachedRowsAmount);
            rowsBase2 = (foundRowsBase + foundRowsAmount);
            rowsAmount1 = reachedRowsAmount;
            if (rowsBase2 < rowsBase1)
            {
                rowsAmount1 = (rowsBase2 - reachedRowsBase);
            }
            treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                freeBlock, horzBlockIdx, reachedRowsBase, rowsAmount1);
            rc = prvCpssAvlItemInsert(
                workDbPtr->rangeTreeArr[draggedFreeIndex], treeDataPtr);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return rc;
            }
            workDbPtr->horzBlockRowsCounter[draggedFreeIndex] += rowsAmount1;
            reachedRowsBase   += rowsAmount1;
            reachedRowsAmount -= rowsAmount1;
        }
    }

    if (reachedRowsAmount > 0)
    {
        /* initialize iteration */
        patternPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
            freeBlock, horzBlockIdx, reachedRowsBase, reachedRowsAmount);
        found = prvCpssAvlPathFind(
            workDbPtr->rangeTreeArr[otherFreeIndex],
            PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
            patternPtr, path, &foundDataPtr);
        if (found == GT_FALSE)
        {
            /* all space was BUSY and not kept in DB    */
            /* add the given space to draggedOther tree */
            treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                freeBlock, horzBlockIdx, reachedRowsBase, reachedRowsAmount);
            rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRangeInsert(
                tcamSegCfgPtr,workDbPtr->rangeTreeArr[draggedOtherIndex],treeDataPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            workDbPtr->horzBlockRowsCounter[draggedOtherIndex] += reachedRowsAmount;
            reachedRowsBase   += reachedRowsAmount;
            reachedRowsAmount = 0;
        }
    }

    while (reachedRowsAmount > 0)
    {
        PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
            foundDataPtr, foundFreeBlock, foundHorzBlockIdx, foundRowsBase, foundRowsAmount);
        if ((freeBlock != foundFreeBlock)
            || (horzBlockIdx != foundHorzBlockIdx)
            || (foundRowsBase >= (reachedRowsBase + reachedRowsAmount)))
        {
            break;
        }
        if (reachedRowsBase < foundRowsBase)
        {
            rowsAmount1 = (foundRowsBase - reachedRowsBase);
            treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                freeBlock, horzBlockIdx, reachedRowsBase, rowsAmount1);
            rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRangeInsert(
                tcamSegCfgPtr,workDbPtr->rangeTreeArr[draggedOtherIndex],treeDataPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            workDbPtr->horzBlockRowsCounter[draggedOtherIndex] += rowsAmount1;
            reachedRowsBase   += rowsAmount1;
            reachedRowsAmount -= rowsAmount1;
        }
        if (reachedRowsAmount == 0) break;

        /* assumed reachedRowsBase == foundRowsBase   */
        /* add part found as orherFree to draggedFree */
        rowsBase1 = (reachedRowsBase + reachedRowsAmount);
        rowsBase2 = (foundRowsBase + foundRowsAmount);
        rowsAmount1 = reachedRowsAmount;
        if (foundRowsAmount < rowsAmount1)
        {
            rowsAmount1 = foundRowsAmount;
        }
        treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
            freeBlock, horzBlockIdx, reachedRowsBase, rowsAmount1);
        rc = prvCpssAvlItemInsert(
            workDbPtr->rangeTreeArr[draggedFreeIndex], treeDataPtr);
        if (rc != GT_OK)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            return rc;
        }
        workDbPtr->horzBlockRowsCounter[draggedFreeIndex] += rowsAmount1;
        reachedRowsBase   += rowsAmount1;
        reachedRowsAmount -= rowsAmount1;

        if (GT_FALSE == prvCpssAvlPathSeek(
                workDbPtr->rangeTreeArr[otherFreeIndex],
                PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &foundDataPtr)) break;
    }

    if (reachedRowsAmount > 0)
    {
        /* add the given space to draggedOther tree */
        treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
            freeBlock, horzBlockIdx, reachedRowsBase, reachedRowsAmount);
        rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRangeInsert(
            tcamSegCfgPtr,workDbPtr->rangeTreeArr[draggedOtherIndex],treeDataPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        workDbPtr->horzBlockRowsCounter[draggedOtherIndex] += reachedRowsAmount;
        reachedRowsBase   += reachedRowsAmount;
        reachedRowsAmount = 0;
    }

    /* clenup range overlapping the given from the low side */
    reachedRowsBase   = rowsBase;
    reachedRowsAmount = rowsAmount;
    patternPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
        freeBlock, horzBlockIdx, reachedRowsBase, reachedRowsAmount);

    found = prvCpssAvlPathFind(
        workDbPtr->rangeTreeArr[otherFreeIndex],
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
        patternPtr, path, &foundDataPtr);
    if (found != GT_FALSE)
    {
        PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
            foundDataPtr, foundFreeBlock, foundHorzBlockIdx, foundRowsBase, foundRowsAmount);
        if ((freeBlock == foundFreeBlock)
            && (horzBlockIdx == foundHorzBlockIdx)
            && ((foundRowsBase + foundRowsAmount) >  reachedRowsBase))
        {
            /* remove overlapping range */
            if (NULL == prvCpssAvlItemRemove(
                workDbPtr->rangeTreeArr[otherFreeIndex], foundDataPtr))
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            workDbPtr->horzBlockRowsCounter[otherFreeIndex] -= foundRowsAmount;

            /* add back part between foundRowsBase and rowsBase */
            if (foundRowsBase < reachedRowsBase)
            {
                rowsAmount1 = (reachedRowsBase - foundRowsBase);
                treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                    freeBlock, horzBlockIdx, foundRowsBase, rowsAmount1);
                rc = prvCpssAvlItemInsert(
                    workDbPtr->rangeTreeArr[otherFreeIndex], treeDataPtr);
                if (rc != GT_OK)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    return rc;
                }
                workDbPtr->horzBlockRowsCounter[otherFreeIndex] += rowsAmount1;
                reachedRowsBase   += rowsAmount1;
                reachedRowsAmount -= rowsAmount1;
            }


            /* add back part between (rowsBase + rowsAmount) */
            /* and (foundRowsBase + foundRowsAmount) */
            rowsBase1 = (reachedRowsBase + reachedRowsAmount);
            rowsBase2 = (foundRowsBase + foundRowsAmount);
            if (rowsBase2 > rowsBase1)
            {
                rowsAmount1 = (rowsBase2 - rowsBase1);
                treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                    freeBlock, horzBlockIdx, rowsBase1, rowsAmount1);
                rc = prvCpssAvlItemInsert(
                    workDbPtr->rangeTreeArr[otherFreeIndex], treeDataPtr);
                if (rc != GT_OK)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    return rc;
                }
                workDbPtr->horzBlockRowsCounter[otherFreeIndex] += rowsAmount1;
                /* cleanup finished - this range already upper then added */
                return GT_OK;
            }

            /* to the end of found range - it is inside the given range */
            rowsBase2 = (foundRowsBase + foundRowsAmount);
            if (rowsBase2 > reachedRowsBase)
            {
                rowsAmount1 = rowsBase2 - reachedRowsBase;
                reachedRowsBase   += rowsAmount1;
                reachedRowsAmount -= rowsAmount1;
            }
        }
    }

    /* cleanup ranges overlapping the given from the middle and high side */
    patternPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
        freeBlock, horzBlockIdx, reachedRowsBase, reachedRowsAmount);
    while (1)
    {
        found = prvCpssAvlPathFind(
            workDbPtr->rangeTreeArr[otherFreeIndex],
            PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
            patternPtr, path, &foundDataPtr);

        if (found == GT_FALSE) break;

        PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
            foundDataPtr, foundFreeBlock, foundHorzBlockIdx, foundRowsBase, foundRowsAmount);
        if ((freeBlock != foundFreeBlock)
            || (horzBlockIdx != foundHorzBlockIdx)
            || ((reachedRowsBase + reachedRowsAmount) <= foundRowsBase))
        {
            break;
        }

        /* remove overlapping range */
        if (NULL == prvCpssAvlItemRemove(
            workDbPtr->rangeTreeArr[otherFreeIndex], foundDataPtr))
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        workDbPtr->horzBlockRowsCounter[otherFreeIndex] -= foundRowsAmount;

        /* assumed that foundRowsBase >= rowsBase                 */
        /* add back part between (rowsBase + rowsAmount) */
        /* and (foundRowsBase + foundRowsAmount)                           */
        rowsBase1 = (reachedRowsBase + reachedRowsAmount);
        rowsBase2 = (foundRowsBase + foundRowsAmount);
        if (rowsBase2 > rowsBase1)
        {
            rowsAmount1 = (rowsBase2 - rowsBase1);
            treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                freeBlock, horzBlockIdx, rowsBase1, rowsAmount1);
            rc = prvCpssAvlItemInsert(
                workDbPtr->rangeTreeArr[otherFreeIndex], treeDataPtr);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return rc;
            }
            workDbPtr->horzBlockRowsCounter[otherFreeIndex] += rowsAmount1;
            /* cleanup finished - this range already upper then added */
            return GT_OK;
        }
    }

    return GT_OK;
}



/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbLoadVTcamSpaceTree function
* @endinternal
*
* @brief   Load vTcam space to Work DB. The space searched and if find
*         converted from FreeAndOther to FreeAndDragged otherwise added as DraggedAndOther.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] workDbPtr                - (pointer to) Work DB
* @param[in] segmentTree              - tree of vTcam layout segments
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceWorkDbLoadVTcamSpaceTree
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     PRV_SEG_DRAG_WORK_DB_STC                      *workDbPtr,
    IN     PRV_CPSS_AVL_TREE_ID                          segmentTree
)
{
    GT_STATUS                                       rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   segmentColumns;
    GT_U32                                          blocksInRow;
    GT_U32                                          horzBlockIdx;
    PRV_CPSS_AVL_TREE_PATH                          path;
    GT_VOID_PTR                                     dbEntryPtr;
    PRV_CPSS_AVL_TREE_SEEK_ENT                      seekType;

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
    while (GT_FALSE != prvCpssAvlPathSeek(
            segmentTree, seekType, path, &dbEntryPtr))
    {
        seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
        segPtr = dbEntryPtr;
        /* mark any free place as mixed "free" and "other" */
        for (horzBlockIdx = 0; (horzBlockIdx < blocksInRow); horzBlockIdx++)
        {
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                segPtr->segmentColumnsMap, horzBlockIdx);

            if (segmentColumns
                == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;

            rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbLoadVTcamSpaceRange(
                tcamSegCfgPtr, workDbPtr,
                0 /*freeBlock*/, horzBlockIdx, segmentColumns,
                segPtr->rowsBase, segPtr->rowsAmount);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbDump function
* @endinternal
*
* @brief   Dump work DB.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] workDbPtr                - (pointer to) Work DB
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbSegSpaceWorkDbDump
(
    IN     PRV_SEG_DRAG_WORK_DB_STC                      *workDbPtr
)
{
    GT_U32                                          i;
    GT_VOID                                         *rangeDataPtr;
    GT_U32                                          freeBlock;
    GT_U32                                          horzBlockIndex;
    GT_U32                                          rowsBase;
    GT_U32                                          rowsAmount;
    PRV_CPSS_AVL_TREE_SEEK_ENT                      seekType;
    PRV_CPSS_AVL_TREE_PATH                          path;

    cpssOsPrintf("workDB non-zero counters:\n");
    for (i = 0; (i < PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E); i++)
    {
        if (workDbPtr->horzBlockRowsCounter[i] != 0)
        {
            cpssOsPrintf(
                "%6.6s %d\n", prvIndexToHorzBlockRowNames[i], workDbPtr->horzBlockRowsCounter[i]);
        }
    }
    cpssOsPrintf("workDB space:\n");
    for (i = 0; (i < PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E); i++)
    {
        seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        while (GT_FALSE != prvCpssAvlPathSeek(
                workDbPtr->rangeTreeArr[i], seekType, path, &rangeDataPtr))
        {
            seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
            PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
                rangeDataPtr, freeBlock, horzBlockIndex, rowsBase, rowsAmount);
            cpssOsPrintf(
                "%6.6s freeBlock %d horzBlockIndex %d rowsBase %d rowsAmount %d\n",
                prvIndexToHorzBlockRowNames[i], freeBlock, horzBlockIndex, rowsBase, rowsAmount);
        }
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveVTcamNewSpaceTree function
* @endinternal
*
* @brief   Retrieve vTcam new space from Work DB. The space is all ranges of
*         FreeAndDragged and DraggedAndOther types. Returned tree as a tree
*         after primary allocation yet not merged in same horizontal rows
*         and rule base/amount fields yet not calculated.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] workDbPtr                - (pointer to) Work DB
* @param[in] vTcamId                  - Id of dragged vTcam.
*
* @param[out] segmentTreePtr           - (pointer to) new tree of vTcam layout segments
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveVTcamNewSpaceTree
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_SEG_DRAG_WORK_DB_STC                                   *workDbPtr,
    IN     GT_U32                                                     vTcamId,
    OUT    PRV_CPSS_AVL_TREE_ID                                       *segmentTreePtr
)
{
    GT_STATUS                                       rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC              *pVTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC patternSeg;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;
    PRV_CPSS_AVL_TREE_ID                            segmentTree;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT    index;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   segmentColumns;
    GT_VOID                                         *rangeDataPtr;
    GT_U32                                          freeBlock;
    GT_U32                                          horzBlockIndex;
    GT_U32                                          rowsBase;
    GT_U32                                          rowsAmount;
    PRV_CPSS_AVL_TREE_SEEK_ENT                      seekType;
    PRV_CPSS_AVL_TREE_PATH                          path;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC     *vTcamInfoPtr;
    GT_U32                                          blockIndex;
    GT_U32                                          blocksInRow;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockPtr;
    GT_U32                                          lookupId;

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    pVTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    if (vTcamId >= pVTcamMngPtr->vTcamCfgPtrArrSize)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    vTcamInfoPtr = pVTcamMngPtr->vTcamCfgPtrArr[vTcamId];
    if (vTcamInfoPtr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(
        vTcamInfoPtr->tcamInfo.clientGroup, vTcamInfoPtr->tcamInfo.hitNumber);

    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
        pVTcamMngPtr->segNodesPool, &segmentTree);
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssAvlTreeCreate failed");
    }

    cpssOsMemSet(&patternSeg, 0, sizeof(patternSeg));
    patternSeg.lookupId =
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(
            vTcamInfoPtr->tcamInfo.clientGroup, vTcamInfoPtr->tcamInfo.hitNumber);
    patternSeg.vTcamId  = vTcamId;
    patternSeg.ruleSize =
        prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
            tcamSegCfgPtr, vTcamInfoPtr->tcamInfo.ruleSize);

    for (index = 0; (index < PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E); index++)
    {
        segmentColumns = prvIndexToHorzBlockRowDraggedColumnsType[index];
        if (segmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;

        seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        while (GT_FALSE != prvCpssAvlPathSeek(
                workDbPtr->rangeTreeArr[index], seekType, path, &rangeDataPtr))
        {
            seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;

            segPtr = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
            if (segPtr == NULL)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                prvCpssAvlTreeDelete(
                    segmentTree,
                    (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
                    (GT_VOID*)pVTcamMngPtr->segmentsPool);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }
            cpssOsMemCpy(segPtr, &patternSeg, sizeof(patternSeg));
            PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
                rangeDataPtr, freeBlock, horzBlockIndex, rowsBase, rowsAmount);
            segPtr->rowsBase   = rowsBase;
            segPtr->rowsAmount = rowsAmount;
            segPtr->segmentColumnsMap =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                segPtr->segmentColumnsMap, horzBlockIndex, segmentColumns);
            rc = prvCpssAvlItemInsert(segmentTree, segPtr);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segPtr);
                prvCpssAvlTreeDelete(
                    segmentTree,
                    (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
                    (GT_VOID*)pVTcamMngPtr->segmentsPool);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            if (freeBlock != 0)
            {
                /* memory from block not mapped to lookup before DB init */
                blockIndex =
                    (((rowsBase / tcamSegCfgPtr->lookupRowAlignment)
                     * blocksInRow) + horzBlockIndex);
                blockPtr = &(tcamSegCfgPtr->blockInfArr[blockIndex]);
                if (blockPtr->lookupId != lookupId)
                {
                    if (blockPtr->lookupId !=
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                    }
                    if (tcamSegCfgPtr->lookupMapFuncPtr != NULL)
                    {
                        rc = tcamSegCfgPtr->lookupMapFuncPtr(
                            tcamSegCfgPtr->cookiePtr,
                            blockPtr->rowsBase,
                            blockPtr->columnsBase,
                            lookupId);
                        if (rc != GT_OK)
                        {
                            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(
                                ("lookupMapFuncPtr returned %d", rc));
                            return rc;
                        }
                        blockPtr->lookupId = lookupId;
                    }
                }
            }
        }
    }
    *segmentTreePtr = segmentTree;
    return GT_OK;
}

static int prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpaceTraceEnable = 0;
GT_VOID prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpaceTraceEnableSet(int enable)
{
    prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpaceTraceEnable = enable;
}
/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpace function
* @endinternal
*
* @brief   Retrieve new free space for treated lookup from Work DB. The space is all ranges of
*         FreeAndDragged and FreeAndOther types. Removes all free space of given lookup and
*         adds all free space from Work DB. Merging neighbor segments not done.
*         Per block free space counters updated.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] workDbPtr                - (pointer to) Work DB
* @param[in] lookupId                 - lookup Id
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_SEG_DRAG_WORK_DB_STC                                   *workDbPtr,
    IN     GT_U32                                                     lookupId
)
{
    GT_STATUS                                       rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC patternSeg;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;
    GT_VOID                                         *dataPtr;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT    index;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   segmentColumns;
    GT_VOID                                         *rangeDataPtr;
    GT_U32                                          horzBlockIndex;
    GT_U32                                          rowsBase;
    GT_U32                                          rowsAmount;
    PRV_CPSS_AVL_TREE_SEEK_ENT                      seekType;
    PRV_CPSS_AVL_TREE_PATH                          path;
    GT_U32                                          freeBlock;
    GT_U32                                          blockIndex;
    GT_U32                                          blocksInRow;
    GT_U32                                          horzBlockIdx;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockPtr;

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    /* remove all free memory with given lookupId from free space trees */
    cpssOsMemSet(&patternSeg, 0, sizeof(patternSeg));
    patternSeg.lookupId = lookupId;
    segmentColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);
    for (horzBlockIdx = 0; (horzBlockIdx < blocksInRow); horzBlockIdx++)
    {
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            patternSeg.segmentColumnsMap, horzBlockIdx, segmentColumns);
    }
    while (GT_FALSE != prvCpssAvlPathFind(
            tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
            &patternSeg, path, &dataPtr))
    {
        segPtr = dataPtr;
        if (segPtr->lookupId != lookupId) break;

        if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpaceTraceEnable)
        {
            cpssOsPrintf("remove free segment: ");
            cpssOsPrintf("rowsBase %3.3d ", segPtr->rowsBase);
            cpssOsPrintf("rowsAmount %3.3d ", segPtr->rowsAmount);
            cpssOsPrintf("ColumnsMap 0x%X ", segPtr->segmentColumnsMap);
            cpssOsPrintf("lookupId %3.3d ", segPtr->lookupId);
            cpssOsPrintf("\n");
        }

        if (NULL == prvCpssAvlItemRemove(tcamSegCfgPtr->segFreePrefTree, segPtr))
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if (NULL == prvCpssAvlItemRemove(tcamSegCfgPtr->segFreeLuTree, segPtr))
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace(
            tcamSegCfgPtr, segPtr, GT_TRUE /*decrease*/);
        cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr);
    }

    /* remove all free memory with not-mapped lookupId, but mapped block from free space trees */
    cpssOsMemSet(&patternSeg, 0, sizeof(patternSeg));
    patternSeg.lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS;
    segmentColumns = prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet(tcamSegCfgPtr);
    for (horzBlockIdx = 0; (horzBlockIdx < blocksInRow); horzBlockIdx++)
    {
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
            patternSeg.segmentColumnsMap, horzBlockIdx, segmentColumns);
    }
    while (GT_FALSE != prvCpssAvlPathFind(
            tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
            &patternSeg, path, &dataPtr))
    {
        segPtr = dataPtr;
        if (segPtr->lookupId !=
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS) break;

        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(
            segPtr->segmentColumnsMap, horzBlockIndex, segmentColumns);

        blockIndex =
            (((segPtr->rowsBase / tcamSegCfgPtr->lookupRowAlignment)
             * blocksInRow) + horzBlockIndex);
        blockPtr = &(tcamSegCfgPtr->blockInfArr[blockIndex]);
        if (blockPtr->lookupId ==
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
        {
            /* not mapped block - don't remove free space */
            /* each such segments represent free block    */
            /* restart finding from the next              */
            if (GT_FALSE == prvCpssAvlPathSeek(
                tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dataPtr))
            {
                /* end of tree reached */
                break;
            }
            cpssOsMemCpy(&patternSeg, dataPtr, sizeof(patternSeg));
            continue;
        }

        if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpaceTraceEnable)
        {
            cpssOsPrintf("remove free segment: ");
            cpssOsPrintf("rowsBase %3.3d ", segPtr->rowsBase);
            cpssOsPrintf("rowsAmount %3.3d ", segPtr->rowsAmount);
            cpssOsPrintf("ColumnsMap 0x%X ", segPtr->segmentColumnsMap);
            cpssOsPrintf("lookupId %3.3d ", segPtr->lookupId);
            cpssOsPrintf("\n");
        }

        if (NULL == prvCpssAvlItemRemove(tcamSegCfgPtr->segFreePrefTree, segPtr))
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if (NULL == prvCpssAvlItemRemove(tcamSegCfgPtr->segFreeLuTree, segPtr))
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace(
            tcamSegCfgPtr, segPtr, GT_TRUE /*decrease*/);
        cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr);
    }

    /* add ranges to both trees of free segments */
    cpssOsMemSet(&patternSeg, 0xFF, sizeof(patternSeg));
    patternSeg.lookupId = lookupId;
    for (index = 0; (index < PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E); index++)
    {
        segmentColumns = prvIndexToHorzBlockRowFreeColumnsType[index];
        if (segmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;

        seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        while (GT_FALSE != prvCpssAvlPathSeek(
                workDbPtr->rangeTreeArr[index], seekType, path, &rangeDataPtr))
        {
            seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;

            PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
                rangeDataPtr, freeBlock, horzBlockIndex, rowsBase, rowsAmount);
            freeBlock = freeBlock; /* fix compiler warning */
            blockIndex =
                (((rowsBase / tcamSegCfgPtr->lookupRowAlignment)
                 * blocksInRow) + horzBlockIndex);
            blockPtr = &(tcamSegCfgPtr->blockInfArr[blockIndex]);
            /* free segments of not mapped blocks were not removed */
            /* blocks that become free yet mapped to old lookup    */
            if (blockPtr->lookupId
                == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS) continue;
            segPtr = cpssBmPoolBufGet(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool);
            if (segPtr == NULL)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }
            cpssOsMemCpy(segPtr, &patternSeg, sizeof(patternSeg));
            segPtr->rowsBase   = rowsBase;
            segPtr->rowsAmount = rowsAmount;
            segPtr->segmentColumnsMap =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                segPtr->segmentColumnsMap, horzBlockIndex, segmentColumns);
            segPtr->lookupId = blockPtr->lookupId;

            if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpaceTraceEnable)
            {
                cpssOsPrintf("insert free segment: ");
                cpssOsPrintf("rowsBase %3.3d ", segPtr->rowsBase);
                cpssOsPrintf("rowsAmount %3.3d ", segPtr->rowsAmount);
                cpssOsPrintf("ColumnsMap 0x%X ", segPtr->segmentColumnsMap);
                cpssOsPrintf("lookupId %3.3d ", segPtr->lookupId);
                cpssOsPrintf("\n");
            }
            rc = prvCpssAvlItemInsert(tcamSegCfgPtr->segFreeLuTree, segPtr);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssAvlItemInsert(tcamSegCfgPtr->segFreePrefTree, segPtr);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                prvCpssAvlItemRemove(tcamSegCfgPtr->segFreeLuTree, segPtr);
                cpssBmPoolBufFree(tcamSegCfgPtr->pVTcamMngPtr->segmentsPool, segPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace(
                tcamSegCfgPtr, segPtr, GT_FALSE /*decrease*/);
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbBlockBitmapsGet function
* @endinternal
*
* @brief   Get bitmaps of indexes of free blocks and mapped blocks in work DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] workDbPtr                - (pointer to) Work DB
*
* @param[out] freeBlocksBmpArr[]       - (pointer to)bitmap array of indexes of free blocks
* @param[out] lookupBlocksBmpArr[]     - (pointer to)bitmap array of indexes of blocks mapped
*                                      to lookup of vTcams, Work DB was filled.
*                                      RETURN
*                                      None.
*                                      COMMENTS:
*/
GT_VOID prvCpssDxChVirtualTcamDbSegSpaceWorkDbBlockBitmapsGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_SEG_DRAG_WORK_DB_STC                                   *workDbPtr,
    OUT    GT_U32                                                     freeBlocksBmpArr[],
    OUT    GT_U32                                                     lookupBlocksBmpArr[]
)
{
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT    index;
    GT_VOID                                         *rangeDataPtr;
    GT_U32                                          horzBlockIndex;
    GT_U32                                          freeBlock;
    GT_U32                                          rowsBase;
    GT_U32                                          rowsAmount;
    PRV_CPSS_AVL_TREE_SEEK_ENT                      seekType;
    PRV_CPSS_AVL_TREE_PATH                          path;
    GT_U32                                          blocksPerRow;
    GT_U32                                          blocksRow;
    GT_U32                                          blockIndex;
    GT_U32                                          bitmapMemoSize;

    bitmapMemoSize = (sizeof(GT_U32) * ((tcamSegCfgPtr->blocksAmount + 31) / 32));
    cpssOsMemSet(freeBlocksBmpArr, 0, bitmapMemoSize);
    cpssOsMemSet(lookupBlocksBmpArr, 0, bitmapMemoSize);

    blocksPerRow =
        (tcamSegCfgPtr->tcamColumnsAmount
            / tcamSegCfgPtr->lookupColumnAlignment);

    for (index = 0; (index < PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E); index++)
    {
        seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        while (GT_FALSE != prvCpssAvlPathSeek(
                workDbPtr->rangeTreeArr[index],
                seekType, path, &rangeDataPtr))
        {
            seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;

            PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
                rangeDataPtr, freeBlock, horzBlockIndex, rowsBase, rowsAmount);
            freeBlock      = freeBlock; /* fix compiler warning */
            blocksRow = (rowsBase / tcamSegCfgPtr->lookupRowAlignment);
            blockIndex = ((blocksRow * blocksPerRow) + horzBlockIndex);
            if ((index == PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFF_E)
                && (rowsAmount >= tcamSegCfgPtr->lookupRowAlignment))
            {
                freeBlocksBmpArr[blockIndex / 32] |= (1 << (blockIndex % 32));
            }
            else
            {
                lookupBlocksBmpArr[blockIndex / 32] |= (1 << (blockIndex % 32));
            }
        }
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceAddFullUsedBlocksToBitmap function
* @endinternal
*
* @brief   Add blocks without free space mapped to given lookup to bitmap.
*         Nothing from such blocks loaded to Work DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup Id
* @param[in] lookupBlocksBmpArr[]     - (pointer to)bitmap array of indexes of blocks mapped
*                                      to lookup of vTcams.
*
* @param[out] lookupBlocksBmpArr[]     - (pointer to)bitmap array of indexes of blocks mapped
*                                      to lookup of vTcams.
*                                      RETURN
*                                      None.
*                                      COMMENTS:
*/
static GT_VOID prvCpssDxChVirtualTcamDbSegSpaceAddFullUsedBlocksToBitmap
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     GT_U32                                                     lookupId,
    OUT    GT_U32                                                     lookupBlocksBmpArr[]
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockPtr;
    GT_U32                                               blockIndex;
    GT_BOOL                                              hasFreeSpace;
    GT_U32                                               freeSpaceType;

    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); ++blockIndex)
    {
        blockPtr = &(tcamSegCfgPtr->blockInfArr[blockIndex]);
        if (blockPtr->lookupId != lookupId) continue;
        hasFreeSpace = GT_FALSE;
        for (freeSpaceType = 0;
            (freeSpaceType < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E);
            freeSpaceType++)
        {
            if (blockPtr->freeSpace[freeSpaceType] != 0)
            {
                hasFreeSpace = GT_TRUE;
                break;
            }
        }
        if (hasFreeSpace != GT_FALSE) continue;
        lookupBlocksBmpArr[blockIndex / 32] |= (1 << (blockIndex % 32));
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleMaxUseGet function
* @endinternal
*
* @brief   Get maximal amount of rule usings.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] workDbPtr                - (pointer to) Work Space DB
* @param[in] rulePtr                  - (pointer to) Work Space DB conversion rule
*                                       maximal amount of rule usings.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleMaxUseGet
(
    IN     PRV_SEG_DRAG_WORK_DB_STC                            *workDbPtr,
    IN     PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC  *rulePtr
)
{
    GT_U32  ruleUsedSpaceCounterArr[PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E];
    GT_U32                                          i, w;
    GT_U32                                          maxRuleUseAmount;

    cpssOsMemSet(ruleUsedSpaceCounterArr, 0, sizeof(ruleUsedSpaceCounterArr));
    for (i = 0; (i < rulePtr->changeArrSize); i++)
    {
        ruleUsedSpaceCounterArr[rulePtr->changeArr[i].source] +=
            rulePtr->changeArr[i].repeater;
    }

    /* check that rule does not contain conversions both A=>B and B=>C     */
    /* the same can be defined as A=>C, "both A=>B and B=>C" not supported */
    for (i = 0; (i < rulePtr->changeArrSize); i++)
    {
        if (ruleUsedSpaceCounterArr[rulePtr->changeArr[i].target] != 0)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            return 0;
        }
    }

    maxRuleUseAmount = 0xFFFFFFFF;
    for (i = 0; (i < PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E); i++)
    {
        if (ruleUsedSpaceCounterArr[i] == 0) continue;
        w = workDbPtr->horzBlockRowsCounter[i] / ruleUsedSpaceCounterArr[i];
        if (w < maxRuleUseAmount)
        {
            maxRuleUseAmount = w;
        }
        if (maxRuleUseAmount == 0)
        {
            return 0;
        }
    }
    return maxRuleUseAmount;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbConvertByRule function
* @endinternal
*
* @brief   Convert Work Space DB Using given Rule.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] workDbPtr                - (pointer to) Work Space DB
* @param[in] rulePtr                  - (pointer to) Work Space DB conversion rule
* @param[in] ruleUseLimit             - maximal amount of rule usings
*
* @param[out] ruleUseActualPtr         - (pointer to) actual amount of rule usings.
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceWorkDbConvertByRule
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *tcamSegCfgPtr,
    IN     PRV_SEG_DRAG_WORK_DB_STC                            *workDbPtr,
    IN     PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC  *rulePtr,
    IN     GT_U32                                               ruleUseLimit,
    OUT    GT_U32                                               *ruleUseActualPtr
)
{
    GT_STATUS                                       rc;
    GT_U32                                          i, amount;
    GT_U32                                          maxRuleUseAmount;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT    srcIdx, dstIdx;
    GT_VOID                                         *treeDataPtr;
    PRV_CPSS_AVL_TREE_PATH                          path;
    GT_U32                                          horzBlockIndex, rowsBase, rowsAmount;
    GT_U32                                          dstRowsAmount;
    GT_U32                                          remainedRows;
    GT_U32                                          freeBlock;

    freeBlock = 0;

    maxRuleUseAmount = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleMaxUseGet(
        workDbPtr, rulePtr);
    if (maxRuleUseAmount > ruleUseLimit)
    {
        maxRuleUseAmount = ruleUseLimit;
    }
    *ruleUseActualPtr = maxRuleUseAmount;

    for (i = 0; (i < rulePtr->changeArrSize); i++)
    {
        srcIdx = rulePtr->changeArr[i].source;
        dstIdx = rulePtr->changeArr[i].target;
        amount = rulePtr->changeArr[i].repeater;
        remainedRows = (amount * maxRuleUseAmount);
        while ((GT_FALSE != prvCpssAvlPathSeek(
            workDbPtr->rangeTreeArr[srcIdx], PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &treeDataPtr)) && (remainedRows > 0))
        {
            if (NULL == prvCpssAvlItemRemove(
                workDbPtr->rangeTreeArr[srcIdx], treeDataPtr))
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            PRV_HORZ_BLOCK_ROWS_RANGE_FROM_VOIDPTR_MAC(
                treeDataPtr, freeBlock, horzBlockIndex, rowsBase, rowsAmount);
            workDbPtr->horzBlockRowsCounter[srcIdx] -= rowsAmount;

            if (rowsAmount > remainedRows)
            {
                dstRowsAmount = remainedRows;
                /* put back the redundat part of range */
                treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                    freeBlock, horzBlockIndex, rowsBase, (rowsAmount - dstRowsAmount));
                rc = prvCpssAvlItemInsert(
                    workDbPtr->rangeTreeArr[srcIdx], treeDataPtr);
                if (rc != GT_OK)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    return rc;
                }
                workDbPtr->horzBlockRowsCounter[srcIdx] += (rowsAmount - dstRowsAmount);

                /* new data to move to new tree*/
                treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC(
                    freeBlock, horzBlockIndex, (rowsBase + rowsAmount - dstRowsAmount), dstRowsAmount);
            }
            else
            {
                dstRowsAmount = rowsAmount;
            }
            remainedRows -= dstRowsAmount;
            rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRangeInsert(
                tcamSegCfgPtr,workDbPtr->rangeTreeArr[dstIdx],treeDataPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            workDbPtr->horzBlockRowsCounter[dstIdx] += dstRowsAmount;
        }
        if (remainedRows > 0)
        {
            /* wrong DB state */
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleCapacityGet function
* @endinternal
*
* @brief   Get Conversion Rule capacity for given TCAM rule size.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] rulePtr                  - (pointer to) Work Space DB conversion rule
* @param[in] tcamRuleSize             - size of TCAM rule in minimal rule units
*
* @param[out] sourceCapacityPtr        - (pointer to) capacity of old space elements.
* @param[out] targetCapacityPtr        - (pointer to) capacity of new space elements.
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleCapacityGet
(
    IN     PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC  *rulePtr,
    IN     GT_U32                                              tcamRuleSize,
    OUT    GT_U32                                              *sourceCapacityPtr,
    OUT    GT_U32                                              *targetCapacityPtr
)
{
    GT_U32 i, w;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT columns;

    *sourceCapacityPtr = 0;
    *targetCapacityPtr = 0;

    for (i = 0; (i < rulePtr->changeArrSize); i++)
    {
        columns = prvIndexToHorzBlockRowFreeColumnsType[rulePtr->changeArr[i].source];
        w = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
            columns, tcamRuleSize);
        *sourceCapacityPtr += (w * rulePtr->changeArr[i].repeater);

        columns = prvIndexToHorzBlockRowFreeColumnsType[rulePtr->changeArr[i].target];
        w = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
            columns, tcamRuleSize);
        *targetCapacityPtr += (w * rulePtr->changeArr[i].repeater);
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleIsApplicable function
* @endinternal
*
* @brief   Checks if Conversion Rule applicable for given dragged vTcam rule size.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] rulePtr                  - (pointer to) Work Space DB conversion rule
* @param[in] draggedTcamRuleSize      - size of TCAM rule of dragged vTcam in minimal rule units
*
* @retval GT_TRUE                  - applicable, GT_FALSE - not applicable.
*/
GT_BOOL prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleIsApplicable
(
    IN     PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC  *rulePtr,
    IN     GT_U32                                              draggedTcamRuleSize
)
{
    GT_U32 i, wSrc, wDst, srcDraggedSize, dstDraggedSize;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT columns;

    srcDraggedSize = 0;
    dstDraggedSize = 0;

    for (i = 0; (i < rulePtr->changeArrSize); i++)
    {
        /* free space in the rule */
        columns = prvIndexToHorzBlockRowDraggedColumnsType[rulePtr->changeArr[i].source];
        wSrc = 0;
        if (columns != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
        {
            /* part occuped by dragged vTcam */
            wSrc = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                columns, draggedTcamRuleSize);
            srcDraggedSize += (wSrc * rulePtr->changeArr[i].repeater);
        }

        columns = prvIndexToHorzBlockRowDraggedColumnsType[rulePtr->changeArr[i].target];
        wDst = 0;
        if (columns != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
        {
            /* part occuped by dragged vTcam */
            wDst = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
                columns, draggedTcamRuleSize);
            dstDraggedSize += (wDst * rulePtr->changeArr[i].repeater);
        }

        if ((wSrc + wDst) == 0) return GT_FALSE;
    }
    if (srcDraggedSize != dstDraggedSize) return GT_FALSE;
    return GT_TRUE;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbCapacityGet function
* @endinternal
*
* @brief   Get Work DB capacity for given TCAM rule size.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] workDbPtr                - (pointer to) Work Space DB
* @param[in] tcamRuleSize             - size of TCAM rule in minimal rule units
*                                       capacity of Work DB (amount of rules that can be placed in free space).
*/
GT_U32 prvCpssDxChVirtualTcamDbSegSpaceWorkDbCapacityGet
(
    IN     PRV_SEG_DRAG_WORK_DB_STC                            *workDbPtr,
    IN     GT_U32                                              tcamRuleSize
)
{
    GT_U32                                       w, capacity;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT index;

    capacity = 0;
    for (index = 0; (index < PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E); index++)
    {
        w = prvCpssDxChVirtualTcamDbSegmentTableRowCapacity(
            prvIndexToHorzBlockRowFreeColumnsType[index],
            tcamRuleSize);
        capacity += (w * workDbPtr->horzBlockRowsCounter[index]);
    }
    return capacity;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbConvertByRuleList function
* @endinternal
*
* @brief   Convert Work Space DB Using given Rule List.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] workDbPtr                - (pointer to) Work Space DB
* @param[in] rulePtrArr               - amount of rulers in array
* @param[in] rulePtrArr               - (pointer to) array of pointers to Work Space DB conversion rules
* @param[in] tcamRuleSize             - size of TCAM rule in minimal rule units.
* @param[in] requiredCapacity         - required amount of  TCAM rules.
*
* @param[out] actualCapacityPtr        - (pointer to) actual TCAM Rules amount.
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceWorkDbConvertByRuleList
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *tcamSegCfgPtr,
    IN     PRV_SEG_DRAG_WORK_DB_STC                            *workDbPtr,
    IN     GT_U32                                              rulePtrArrSize,
    IN     PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC  **rulePtrArr,
    IN     GT_U32                                              tcamRuleSize,
    IN     GT_U32                                              requiredCapacity,
    OUT    GT_U32                                              *actualCapacityPtr
)
{
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC  *rulePtr;
    GT_U32    dbCapacity, ruleIndex, useNum, actualUseNum;
    GT_U32    ruleSrcCapacity, ruleDstCapacity, ruleCapacity;
    GT_STATUS rc;

    dbCapacity = prvCpssDxChVirtualTcamDbSegSpaceWorkDbCapacityGet(
        workDbPtr, tcamRuleSize);

    for (ruleIndex = 0; (ruleIndex < rulePtrArrSize); ruleIndex++)
    {
        if (dbCapacity >= requiredCapacity)
        {
            *actualCapacityPtr = dbCapacity;
            return GT_OK;
        }
        rulePtr = rulePtrArr[ruleIndex];
        prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleCapacityGet(
            rulePtr, tcamRuleSize, &ruleSrcCapacity, &ruleDstCapacity);
        if (ruleSrcCapacity >= ruleDstCapacity) continue;
        ruleCapacity = ruleDstCapacity - ruleSrcCapacity;

        useNum = ((requiredCapacity - dbCapacity + ruleCapacity - 1) / ruleCapacity);
        rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbConvertByRule(
            tcamSegCfgPtr, workDbPtr, rulePtr, useNum, &actualUseNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        dbCapacity += (actualUseNum * ruleCapacity);
        *actualCapacityPtr = dbCapacity;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbBuildRuleList function
* @endinternal
*
* @brief   Build list of converting rules relevant to dragged and goal TCAM rule sizes.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] draggedTcamRuleSize      - size of TCAM rule of dragged vTcam in minimal rule units.
* @param[in] goalTcamRuleSize         - size of TCAM rule of allocated vTcam in minimal rule units.
*                                       None.
*
* @note Fills static work_rules_arr variable.
*
*/
static GT_VOID prvCpssDxChVirtualTcamDbSegSpaceWorkDbBuildRuleList
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC      *tcamSegCfgPtr,
    IN     GT_U32                                              draggedTcamRuleSize,
    IN     GT_U32                                              goalTcamRuleSize
)
{
    typedef struct
    {
        PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR *rulesArrPtr;
        GT_U32                                             rulesArrSize;
    } PRV_RULE_LIST_STC;
    static PRV_RULE_LIST_STC ruleListArr[] =
    {
        {rules_target_40_dragged_10_arr,
            (sizeof(rules_target_40_dragged_10_arr) / sizeof(rules_target_40_dragged_10_arr[0]))},
        {rules_target_50_dragged_10_arr,
            (sizeof(rules_target_50_dragged_10_arr) / sizeof(rules_target_50_dragged_10_arr[0]))},
        {rules_target_60_dragged_30_arr,
            (sizeof(rules_target_60_dragged_30_arr) / sizeof(rules_target_60_dragged_30_arr[0]))},
        {rules_target_60_dragged_10_arr,
            (sizeof(rules_target_60_dragged_10_arr) / sizeof(rules_target_60_dragged_10_arr[0]))},
        {rules_miscellanous_arr,
            (sizeof(rules_miscellanous_arr) / sizeof(rules_miscellanous_arr[0]))}
    };
    static GT_U32 ruleListArrsize =
        (sizeof(ruleListArr) / sizeof(ruleListArr[0]));
    static PRV_RULE_LIST_STC ruleLegacyListArr[] =
    {
        {rules_legacy_arr,
            (sizeof(rules_legacy_arr) / sizeof(rules_legacy_arr[0]))}
    };
    static GT_U32 ruleLegacyListArrsize =
        (sizeof(ruleLegacyListArr) / sizeof(ruleLegacyListArr[0]));
    GT_U32 listIdx, ruleIdx;
    PRV_RULE_LIST_STC *listArrPtr;
    GT_U32            listArrSize;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR rulePtr;
    GT_BOOL           applicable;
    GT_U32            sourceCapacity;
    GT_U32            targetCapacity;

    work_rules_arr_used = 0;

    switch (tcamSegCfgPtr->deviceClass)
    {
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        listArrPtr   = ruleListArr;
        listArrSize  = ruleListArrsize;
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        listArrPtr   = ruleLegacyListArr;
        listArrSize  = ruleLegacyListArrsize;
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
    default:
        /* No need to drag */
        return;
    }

    for (listIdx = 0; (listIdx < listArrSize); listIdx++)
    {
        for (ruleIdx = 0; (ruleIdx < listArrPtr[listIdx].rulesArrSize); ruleIdx++)
        {
            rulePtr = listArrPtr[listIdx].rulesArrPtr[ruleIdx];
            applicable = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleIsApplicable(
                rulePtr, draggedTcamRuleSize);
            if (applicable == GT_FALSE) continue;
            prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleCapacityGet(
                rulePtr, goalTcamRuleSize, &sourceCapacity, &targetCapacity);
            if (sourceCapacity >= targetCapacity) continue;
            work_rules_arr[work_rules_arr_used] = rulePtr;
            work_rules_arr_used  ++;

            if (work_rules_arr_used >= (sizeof(work_rules_arr) / sizeof(work_rules_arr[0])))
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return;
            }
        }
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceEstimateFreeSpace function
* @endinternal
*
* @brief   Estimate free space in all TCAM related to request.
*         Free space in other lookups truncated to size of whole blocks
*         that possible can be freed for use by given lookup.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
*
* @retval GT_OK                    - enough free space (maybe can be moved by dragging to be usable).
* @retval GT_FULL                  - not enough free space.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceEstimateFreeSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr
)
{
    GT_U32                                      freeUnitsInLookup;
    GT_U32                                      blockIndex, blockIndex1;
    GT_U32                                      freeBlocksNum;
    GT_U32                                      blockUnitsNum;
    GT_U32                                      requiredSpace;
    GT_U32                                      totalSpace;
    GT_U32                                      lookupId;
    GT_U32                                      freeUnitsInOtherLookup;
    GT_BOOL                                     firstInLookup;

    totalSpace = 0;
    requiredSpace =
        (allocRequestPtr->ruleSize * allocRequestPtr->rulesAmount);
    blockUnitsNum = (tcamSegCfgPtr->lookupColumnAlignment
         * tcamSegCfgPtr->lookupRowAlignment);

    /* given lookup */
    freeUnitsInLookup =
        prvCpssDxChVirtualTcamDbSegmentTableCountFreeSpaceInUnits(
            tcamSegCfgPtr, allocRequestPtr->lookupId);

    totalSpace += freeUnitsInLookup;
    if (totalSpace >= requiredSpace) return GT_OK;

    /* free blocks */
    freeBlocksNum = 0;
    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); blockIndex++)
    {
        if (tcamSegCfgPtr->blockInfArr[blockIndex].lookupId ==
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
        {
            freeBlocksNum ++;
        }
    }

    totalSpace += (freeBlocksNum * blockUnitsNum);
    if (totalSpace >= requiredSpace) return GT_OK;

    /* other lookups */
    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); blockIndex++)
    {
        lookupId = tcamSegCfgPtr->blockInfArr[blockIndex].lookupId;
        if (lookupId ==
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS) continue;
        if (lookupId == allocRequestPtr->lookupId) continue;

        /* check, that block is fist in it's lookup */
        firstInLookup = GT_TRUE;
        for (blockIndex1 = 0; (blockIndex1 < blockIndex); blockIndex1++)
        {
            if (lookupId == tcamSegCfgPtr->blockInfArr[blockIndex1].lookupId)
            {
                firstInLookup = GT_FALSE;
                break;
            }
        }
        if (firstInLookup == GT_FALSE) continue;

        freeUnitsInOtherLookup =
            prvCpssDxChVirtualTcamDbSegmentTableCountFreeSpaceInUnits(
                tcamSegCfgPtr, lookupId);

        totalSpace +=
            (freeUnitsInOtherLookup - (freeUnitsInOtherLookup % blockUnitsNum));
        if (totalSpace >= requiredSpace) return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceDragVTcamToNewSegmentTree function
* @endinternal
*
* @brief   Drag given vTcam to new space given by segment tree.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] vTcamId                  - vTcam id.
* @param[in] segmentsTree             - segment tree with new vTcam space
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceDragVTcamToNewSegmentTree
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                      vTcamId,
    IN     PRV_CPSS_AVL_TREE_ID                        segmentsTree
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr;
    GT_STATUS                                   rc;
    GT_U16                                      *oldRulePhysicalIndexArr;
    GT_U32                                      ruleSize;

    pVTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    vTcamInfoPtr = pVTcamMngPtr->vTcamCfgPtrArr[vTcamId];
    if (vTcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    ruleSize = prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
        tcamSegCfgPtr, vTcamInfoPtr->tcamInfo.ruleSize);

    /* delete old segment tree and substitute the new */
    prvCpssAvlTreeDelete(
        vTcamInfoPtr->segmentsTree,
        (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
        (GT_VOID*)pVTcamMngPtr->segmentsPool);
    vTcamInfoPtr->segmentsTree = segmentsTree;

    /* merge vTCAM new layout segments */
    rc = prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge(
        tcamSegCfgPtr, vTcamInfoPtr->segmentsTree);
    if (rc != GT_OK) return rc;

    /* update rule amounts after splits and merges */
    prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts(
        tcamSegCfgPtr, vTcamInfoPtr->segmentsTree);

    /* update start segment logical indexes */
    segmentTableVTcamBaseLogicalIndexUpdate(vTcamInfoPtr);

    /* save old table logical => physical rule indexes */
    oldRulePhysicalIndexArr = vTcamInfoPtr->rulePhysicalIndexArr;

    /* update Logical => physical table and move rules */
    /* bypassed for dummy vTcam Manager used for checking availability */
    if (oldRulePhysicalIndexArr != NULL)
    {
        /* create new table logical => physical rule indexes */
        vTcamInfoPtr->rulePhysicalIndexArr =
            (GT_U16*)cpssOsMalloc(sizeof(GT_U16) * vTcamInfoPtr->rulesAmount);
        if (vTcamInfoPtr->rulePhysicalIndexArr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate(
            tcamSegCfgPtr,
            vTcamInfoPtr->segmentsTree,
            vTcamInfoPtr->rulePhysicalIndexArr);

        /* move rules from old to new location */
        rc = prvCpssDxChVirtualTcamDbSegmentTableMoveRulesToNewLayout(
            tcamSegCfgPtr, vTcamInfoPtr->rulesAmount, ruleSize,
            vTcamInfoPtr->usedRulesBitmapArr,
            oldRulePhysicalIndexArr /*srcRulePhysicalIndexArr*/,
            vTcamInfoPtr->rulePhysicalIndexArr /*dstRulePhysicalIndexArr*/);

        /* free old bitmap (saved) */
        cpssOsFree(oldRulePhysicalIndexArr);

        if (rc != GT_OK) return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId function
* @endinternal
*
* @brief   Drag vTcams in blocks mapped to required lookup id and free blocks.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] checkWithoutUpdate       - GT_TRUE  - vTcams dragging checked using work DB,
*                                      permanent DB and HW not updated
*                                      GT_FALSE - vTcams dragged in permanent DB and HW
*
* @param[out] actualCapacityPtr        - (pointer to) actual TCAM Rules amount.
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN     GT_BOOL                                                    checkWithoutUpdate,
    OUT    GT_U32                                                     *actualCapacityPtr
)
{
    GT_STATUS                                   rc, totalRc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr;
    PRV_SEG_DRAG_WORK_DB_STC                    workDb;
    GT_U32                                      sip5RuleSizesArr[] = {1,2,3};
    GT_U32                                      legacyRuleSizesArr[] = {1};
    GT_U32                                      ruleSizesArrSize;
    GT_U32                                      *ruleSizesArr;
    GT_U32                                      ruleSizeIdx;
    GT_U32                                      ruleSize;
    GT_U32                                      vTcamId;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr;
    PRV_CPSS_AVL_TREE_ID                        segmentsTree;
    GT_U32                                      actualCapacity;
    GT_U32                                      savedActualCapacity;
    GT_U32                                      draggedVTcamsNum;

    *actualCapacityPtr = 0;

    switch (tcamSegCfgPtr->deviceClass)
    {
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        ruleSizesArr     = sip5RuleSizesArr;
        ruleSizesArrSize = (sizeof(sip5RuleSizesArr) / sizeof(sip5RuleSizesArr[0]));
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        ruleSizesArr     = legacyRuleSizesArr;
        ruleSizesArrSize = (sizeof(legacyRuleSizesArr) / sizeof(legacyRuleSizesArr[0]));
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
    default:
        /* No need to drag */
        return GT_OK;
    }

    rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbInit(
        tcamSegCfgPtr, allocRequestPtr->lookupId, &workDb);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnable != GT_FALSE)
    {
        cpssOsPrintf("WorkDB after Init\n");
        prvCpssDxChVirtualTcamDbSegSpaceWorkDbDump(&workDb);
    }

    pVTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    totalRc = GT_OK;
    savedActualCapacity = 0;
    while (1) /* break conditions at the end */
    {
        draggedVTcamsNum    = 0;
        actualCapacity      = 0;
        for (ruleSizeIdx = 0; (ruleSizeIdx < ruleSizesArrSize); ruleSizeIdx++)
        {
            ruleSize = ruleSizesArr[ruleSizeIdx];
            prvCpssDxChVirtualTcamDbSegSpaceWorkDbBuildRuleList(
                tcamSegCfgPtr, ruleSize, allocRequestPtr->ruleSize);
            for (vTcamId = 0; (vTcamId < pVTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
            {
                /* bypass not existing vTcams and vTcams with other lookupId or orher ruleSize */
                vTcamInfoPtr = pVTcamMngPtr->vTcamCfgPtrArr[vTcamId];
                if (vTcamInfoPtr == NULL) continue;
                if (vTcamInfoPtr->tcamSegCfgPtr != tcamSegCfgPtr)
                {
                    continue;
                }
                if (allocRequestPtr->lookupId !=
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(
                        vTcamInfoPtr->tcamInfo.clientGroup,
                        vTcamInfoPtr->tcamInfo.hitNumber)) continue;
                if (ruleSize != prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
                    tcamSegCfgPtr, vTcamInfoPtr->tcamInfo.ruleSize)) continue;

                /* dragged vTcam treatment */
                draggedVTcamsNum ++;
                /* mark vTcam space as "dragged" */
                rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbLoadVTcamSpaceTree(
                    tcamSegCfgPtr, &workDb, vTcamInfoPtr->segmentsTree);
                if (rc != GT_OK) totalRc = rc;
                if (totalRc != GT_OK) break;

                if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnable != GT_FALSE)
                {
                    cpssOsPrintf("WorkDB after Load vTcam %d\n", vTcamId);
                    prvCpssDxChVirtualTcamDbSegSpaceWorkDbDump(&workDb);
                }
                /* drag space in DB by converting rules */
                rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbConvertByRuleList(
                    tcamSegCfgPtr, &workDb, work_rules_arr_used, work_rules_arr,
                    allocRequestPtr->ruleSize, allocRequestPtr->rulesAmount,
                    &actualCapacity);
                if (rc != GT_OK) totalRc = rc;
                if (totalRc != GT_OK) break;

                if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnable != GT_FALSE)
                {
                    cpssOsPrintf("WorkDB after Convert by rules \n");
                    prvCpssDxChVirtualTcamDbSegSpaceWorkDbDump(&workDb);
                }

                if (checkWithoutUpdate == GT_FALSE)
                {
                    /* Retrieve new vTcam Layout segment tree */
                    rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveVTcamNewSpaceTree(
                        tcamSegCfgPtr, &workDb, vTcamId, &segmentsTree);
                    if (rc != GT_OK) totalRc = rc;
                    if (totalRc != GT_OK) break;

                    rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamToNewSegmentTree(
                        tcamSegCfgPtr, vTcamId, segmentsTree);
                    if (rc != GT_OK) totalRc = rc;
                    if (totalRc != GT_OK) break;
                }
                /* dragged vTcam DB cleanup: Dragged to Other */
                rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbDraggedToOther(
                    tcamSegCfgPtr, &workDb);
                if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnable != GT_FALSE)
                {
                    cpssOsPrintf("WorkDB after cleanup Dragged to Other \n");
                    prvCpssDxChVirtualTcamDbSegSpaceWorkDbDump(&workDb);
                }

                if (actualCapacity >= allocRequestPtr->rulesAmount) break;
            }
            if (totalRc != GT_OK) break;
            if (actualCapacity >= allocRequestPtr->rulesAmount) break;
        }
        if (totalRc != GT_OK) break;
        if (actualCapacity >= allocRequestPtr->rulesAmount) break;
        if (savedActualCapacity >= actualCapacity) break; /* no free space added */
        savedActualCapacity = actualCapacity;
    }

    if ((checkWithoutUpdate == GT_FALSE) && (draggedVTcamsNum > 0))
    {
        rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpace(
            tcamSegCfgPtr, &workDb, allocRequestPtr->lookupId);
        if (rc != GT_OK) totalRc = rc;
        prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeNeighbors(tcamSegCfgPtr);
    }
    prvCpssDxChVirtualTcamDbSegSpaceWorkDbFree(&workDb);
    *actualCapacityPtr = actualCapacity;
    return totalRc;
}

/* 80-byte rules space making                            */
/* two phases: "holes making" and "holes moving"         */
/* making 60-byte (FFFFFF) and 20-byte (FFOOOO) holes    */
/* at any places using reglar workDB                     */
/* moving these horizontal block rows to new places      */
/* capable for 80-byte rules                             */

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleCapacityCalculate function
* @endinternal
*
* @brief   Get Work DB capacity for 80-bytes rules.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] ffffffNum                - amount of _FFFFFF_ horizontal block rows
* @param[in] ffxxxxNum                - amount of _FFDDDD_ and _FFOOOO_ horizontal block rows
*                                       capacity of Work DB (amount of rules that can be placed in free space (after reordering)).
*/
GT_U32 prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleCapacityCalculate
(
    IN    GT_U32 ffffffNum,
    IN    GT_U32 ffxxxxNum
)
{
    if (ffxxxxNum >= ffffffNum) return ffffffNum;

    return ((ffffffNum + ffxxxxNum) / 2);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleAdditionalRowsNumGet function
* @endinternal
*
* @brief   Get amount of additional rows _FFFFFF_ from free blocks
*         to reach given capacity.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] ffffffNum                - amount of _FFFFFF_ horizontal block rows
* @param[in] ffxxxxNum                - amount of _FFDDDD_ and _FFOOOO_ horizontal block rows
* @param[in] capacity                 - amount of 80-byte rules that can be allocated after
*                                      moving _FFFFFF_ and _FFXXXX_ free places
*                                       amount of additional _FFFFFF_ rows that needed.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleAdditionalRowsNumGet
(
    IN    GT_U32 ffffffNum,
    IN    GT_U32 ffxxxxNum,
    IN    GT_U32 capacity
)
{
    GT_U32 usefull_ffxxxxNum;
    GT_U32 required_ffffffNum;

    usefull_ffxxxxNum =
        ((ffxxxxNum >= capacity) ? capacity : ffxxxxNum);
    required_ffffffNum = ((2 * capacity) - usefull_ffxxxxNum);

    return ((required_ffffffNum > ffffffNum) ? (required_ffffffNum - ffffffNum) : 0);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleAdditionalFfxxxxNumGet function
* @endinternal
*
* @brief   Get amount of additional rows _FFXXXX_ from free blocks
*         to reach given capacity.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] ffffffNum                - amount of _FFFFFF_ horizontal block rows
* @param[in] ffxxxxNum                - amount of _FFDDDD_ and _FFOOOO_ horizontal block rows
* @param[in] capacity                 - amount of 80-byte rules that can be allocated after
*                                      moving _FFFFFF_ and _FFXXXX_ free places
*                                       amount of additional _FFXXXX_ rows that needed.
*                                       If given capacity cannot be reached returns 0xFFFFFFFF
*/
GT_U32 prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleAdditionalFfxxxxNumGet
(
    IN    GT_U32 ffffffNum,
    IN    GT_U32 ffxxxxNum,
    IN    GT_U32 capacity
)
{
    /* capacity cannot be reached */
    if (ffffffNum < capacity) return 0xFFFFFFFF;

    /* capacity already reached */
    if ((2 * capacity) <= (ffffffNum + ffxxxxNum)) return 0;

    return ((2 * capacity) - (ffffffNum + ffxxxxNum));
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleRuleUseCountForMaxCapacity function
* @endinternal
*
* @brief   Calculate for wich use count value reached maximal capacity.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] ffffffNum                - DB amount of _FFFFFF_ horizontal block rows
* @param[in] ffxxxxNum                - DB amount of _FFDDDD_ and _FFOOOO_ horizontal block rows
* @param[in] ffffffSrcNumOf           - rule source      number of _FFFFFF_ horozontal block rows.
* @param[in] ffffffDstNumOf           - rule destination number of _FFFFFF_ horozontal block rows.
* @param[in] ffxxxxSrcNumOf           - rule source      number of
*                                      _FFDDDD_ and  _FFOOOO_ horozontal block rows.
* @param[in] ffxxxxDstNumOf           - rule destination number
*                                      _FFDDDD_ and  _FFOOOO_ horozontal block rows.
* @param[in] maxUseCount              - maximal value of use counter.
*                                       Use count value when reached maximal capacity. (after reordering)).
*                                       The least of such values if more then one.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleRuleUseCountForMaxCapacity
(
    IN    GT_U32 ffffffNum,
    IN    GT_U32 ffxxxxNum,
    IN    GT_U32 ffffffSrcNumOf,
    IN    GT_U32 ffffffDstNumOf,
    IN    GT_U32 ffxxxxSrcNumOf,
    IN    GT_U32 ffxxxxDstNumOf,
    IN    GT_U32 maxUseCount
)
{
    GT_U32 ffffffMul, ffxxxxMul;
    GT_U32 count;

    count = 0;

    /* rule is at least unusefull */
    if ((ffffffDstNumOf <= ffffffSrcNumOf)
        && (ffxxxxDstNumOf <= ffxxxxSrcNumOf)) return 0;

    /* capacity grows at any use count increment */
    if ((ffffffDstNumOf >= ffffffSrcNumOf)
        && (ffxxxxDstNumOf >= ffxxxxSrcNumOf)) return maxUseCount;

    /* abstact                                              */
    /* let F6 is current ffffffNum, F2 is current ffxxxxNum */
    /* capacity = ((F6 <= F2) ? F6 : ((F6 + F2) / 2))       */
    /* let K6 is  ffffffMul, K2 is  ffxxxxMul               */
    /* let I6 is  ffffffNum, I2 is  ffxxxxNum               */

    if (ffffffDstNumOf > ffffffSrcNumOf)
    {
        /* assumed ffxxxxDstNumOf < ffxxxxSrcNumOf */
        /* F6 grows, F2 descends                   */
        /* F6 = I6 + (count * K6)                  */
        /* F2 = I2 - (count * K2)                  */

        ffffffMul = ffffffDstNumOf - ffffffSrcNumOf;
        ffxxxxMul = ffxxxxSrcNumOf - ffxxxxDstNumOf;

        /* if K6 > K2 capacity grows at any F6, F2 */
        if (ffffffMul > ffxxxxMul) return maxUseCount;

        /* assumed K6 <= K2, capacity grows only if (F6 <= F2) */
        /* when I6 >= I2 capacity will not grow                */
        if (ffffffNum >= ffxxxxNum) return 0;

        /* find when F6 ~== F2                    */
        /* assumed I6 <= I2, K6 <= K2             */
        /* F6 <= F2                               */
        /* I6 + (count * K6) >= I2 - (count * K2) */
        /* count * (K6 + K2) <= (I2 - I6)         */
        /* count <= (I2 - I6) / (K6 + K2)         */
        count = (ffxxxxNum - ffffffNum) / (ffxxxxMul + ffffffMul);
    }

    if (ffxxxxDstNumOf > ffxxxxSrcNumOf)
    {
        /* assumed ffffffDstNumOf < ffffffSrcNumOf */
        /* F2 grows, F6 descends                   */
        /* F6 = I6 - (count * K6)                  */
        /* F2 = I2 + (count * K2)                  */

        /* if F6 <= F2 capacity will descent at any amount of rule uses */
        if (ffffffNum <= ffxxxxNum) return 0;

        ffffffMul = ffffffSrcNumOf - ffffffDstNumOf;
        ffxxxxMul = ffxxxxDstNumOf - ffxxxxSrcNumOf;

        /* if K6 >= K2 capacity does not grow at any F6, F2 */
        if (ffffffMul >= ffxxxxMul) return 0;

        /* assumed K6 < K2, I6 > I2     */
        /* rule is usefull when F2 < F6 */
        /* I2 + (count * K2) < I6 - (count * K6)  */
        /* count * (K6 + K2) < (I6 - I2)          */
        /* count < (I6 - I2) / (K6 + K2)          */
        count = (ffffffNum - ffxxxxNum) / (ffxxxxMul + ffffffMul);
    }
    if (count > maxUseCount) count = maxUseCount;
    return count;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleCapacityGet function
* @endinternal
*
* @brief   Get Work DB capacity for 80-bytes rules.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] workDbPtr                - (pointer to) Work Space DB
*                                       capacity of Work DB (amount of rules that can be placed in free space (after reordering)).
*/
GT_U32 prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleCapacityGet
(
    IN     PRV_SEG_DRAG_WORK_DB_STC                            *workDbPtr
)
{
    GT_U32 ffffffNum, ffxxxxNum;

    ffffffNum = workDbPtr->horzBlockRowsCounter[
        PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFF_E];
    ffxxxxNum = workDbPtr->horzBlockRowsCounter[
        PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFDDDD_E]
              + workDbPtr->horzBlockRowsCounter[
        PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFOOOO_E];

    return prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleCapacityCalculate(
        ffffffNum, ffxxxxNum);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbRule80byteMakeHoleCapacityGet function
* @endinternal
*
* @brief   Get Conversion Rule capacity for making holes for 80-bytes rule.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] rulePtr                  - (pointer to) Work Space DB conversion rule
*
* @param[out] ffffffSrcNumOfPtr        - (pointer to)source      number of _FFFFFF_ horozontal block rows.
* @param[out] ffffffDstNumOfPtr        - (pointer to)destination number of _FFFFFF_ horozontal block rows.
* @param[out] ffxxxxSrcNumOfPtr        - (pointer to)source      number of
*                                      _FFDDDD_ and  _FFOOOO_ horozontal block rows.
* @param[out] ffxxxxDstNumOfPtr        - (pointer to)destination number
*                                      _FFDDDD_ and  _FFOOOO_ horozontal block rows.
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbSegSpaceWorkDbRule80byteMakeHoleCapacityGet
(
    IN     PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC  *rulePtr,
    OUT    GT_U32                                              *ffffffSrcNumOfPtr,
    OUT    GT_U32                                              *ffffffDstNumOfPtr,
    OUT    GT_U32                                              *ffxxxxSrcNumOfPtr,
    OUT    GT_U32                                              *ffxxxxDstNumOfPtr
)
{
    GT_U32 i;

    *ffffffSrcNumOfPtr = 0;
    *ffffffDstNumOfPtr = 0;
    *ffxxxxSrcNumOfPtr = 0;
    *ffxxxxDstNumOfPtr = 0;

    for (i = 0; (i < rulePtr->changeArrSize); i++)
    {
        switch (rulePtr->changeArr[i].source)
        {
            case PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFF_E:
                *ffffffSrcNumOfPtr += rulePtr->changeArr[i].repeater;
                break;
            case PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFDDDD_E:
            case PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFOOOO_E:
                *ffxxxxSrcNumOfPtr += rulePtr->changeArr[i].repeater;
                break;
            default: break;
        }
        switch (rulePtr->changeArr[i].target)
        {
            case PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFF_E:
                *ffffffDstNumOfPtr += rulePtr->changeArr[i].repeater;
                break;
            case PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFDDDD_E:
            case PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFOOOO_E:
                *ffxxxxDstNumOfPtr += rulePtr->changeArr[i].repeater;
                break;
            default: break;
        }
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleRuleUsingNum function
* @endinternal
*
* @brief   Calculate amount of times of convertion rule using that yet increases capacity.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] workDbPtr                - (pointer to) Work Space DB
* @param[in] rulePtr                  - (pointer to) Work Space DB conversion rule
* @param[in] maxRulesNum              - maximal number of rules that needed to allocate
*                                       amount of times of convertion rule using that yet increases capacity.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleRuleUsingNum
(
    IN     PRV_SEG_DRAG_WORK_DB_STC                            *workDbPtr,
    IN     PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC  *rulePtr,
    IN     GT_U32                                              maxRulesNum
)
{
    GT_U32    ffffffSrcNumOf, ffffffDstNumOf;
    GT_U32    ffxxxxSrcNumOf, ffxxxxDstNumOf;
    GT_U32    ffffffNum, ffxxxxNum;
    GT_U32    ffffffNum1, ffxxxxNum1;
    GT_U32    maxPossible, maxUsefull;
    GT_U32    count, highCount, lowCount;
    GT_U32    capacity;

    maxPossible = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleMaxUseGet(
        workDbPtr, rulePtr);
    if (maxPossible == 0) return 0;

    ffffffNum = workDbPtr->horzBlockRowsCounter[
        PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFF_E];
    ffxxxxNum = workDbPtr->horzBlockRowsCounter[
        PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFDDDD_E]
              + workDbPtr->horzBlockRowsCounter[
        PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFOOOO_E];

    prvCpssDxChVirtualTcamDbSegSpaceWorkDbRule80byteMakeHoleCapacityGet(
      rulePtr, &ffffffSrcNumOf, &ffffffDstNumOf,
      &ffxxxxSrcNumOf, &ffxxxxDstNumOf);

    maxUsefull = prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleRuleUseCountForMaxCapacity(
        ffffffNum, ffxxxxNum,
        ffffffSrcNumOf, ffffffDstNumOf, ffxxxxSrcNumOf, ffxxxxDstNumOf,
        maxPossible);
    if (maxUsefull == 0) return 0;

    capacity = prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleCapacityCalculate(
        ffffffNum, ffxxxxNum);
    if (capacity >= maxRulesNum) return 0; /* already as needed */

    /* check maxUsefull value */
    ffffffNum1 = ffffffNum
        + (ffffffDstNumOf * maxUsefull) - (ffffffSrcNumOf * maxUsefull);
    ffxxxxNum1 = ffxxxxNum
        + (ffxxxxDstNumOf * maxUsefull) - (ffxxxxSrcNumOf * maxUsefull);
    capacity = prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleCapacityCalculate(
      ffffffNum1, ffxxxxNum1);
    if (capacity <= maxRulesNum) return maxUsefull;

    /* binary search of exact result */
    highCount = maxUsefull; /* already checked that enough */
    lowCount  = 0;           /* already checked that not enough */
    while (highCount > (lowCount + 1))
    {
        count = ((highCount + lowCount) / 2);
        ffffffNum1 = ffffffNum
            + (ffffffDstNumOf * count) - (ffffffSrcNumOf * count);
        ffxxxxNum1 = ffxxxxNum
            + (ffxxxxDstNumOf * count) - (ffxxxxSrcNumOf * count);
        capacity = prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleCapacityCalculate(
          ffffffNum1, ffxxxxNum1);
        if (capacity < maxRulesNum)
        {
            lowCount = count;
        }
        else
        {
            highCount = count;
        }
    }
    return ((capacity < maxRulesNum) ? highCount : lowCount);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDb80bytesMakingConvertByRuleList function
* @endinternal
*
* @brief   Convert Work Space DB Making Holes for 80-bytes Rules Using given Rule List.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] workDbPtr                - (pointer to) Work Space DB
* @param[in] rulePtrArr               - amount of rulers in array
* @param[in] rulePtrArr               - (pointer to) array of pointers to Work Space DB conversion rules
* @param[in] requiredCapacity         - required amount of  TCAM rules.
*
* @param[out] actualCapacityPtr        - (pointer to) actual TCAM Rules amount.
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceWorkDb80bytesMakingConvertByRuleList
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *tcamSegCfgPtr,
    IN     PRV_SEG_DRAG_WORK_DB_STC                            *workDbPtr,
    IN     GT_U32                                              rulePtrArrSize,
    IN     PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC  **rulePtrArr,
    IN     GT_U32                                              requiredCapacity,
    OUT    GT_U32                                              *actualCapacityPtr
)
{
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC  *rulePtr;
    GT_U32    dbCapacity, ruleIndex, useNum, actualUseNum;
    GT_STATUS rc;

    dbCapacity = prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleCapacityGet(
        workDbPtr);

    for (ruleIndex = 0; (ruleIndex < rulePtrArrSize); ruleIndex++)
    {
        if (dbCapacity >= requiredCapacity)
        {
            *actualCapacityPtr = dbCapacity;
            return GT_OK;
        }
        rulePtr = rulePtrArr[ruleIndex];
        useNum = prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleRuleUsingNum(
                workDbPtr, rulePtr, requiredCapacity);
        if (useNum == 0) continue;
        rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbConvertByRule(
            tcamSegCfgPtr, workDbPtr, rulePtr, useNum, &actualUseNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        dbCapacity = prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleCapacityGet(
            workDbPtr);
        *actualCapacityPtr = dbCapacity;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceWorkDbBuld80bytesMakingRuleList function
* @endinternal
*
* @brief   Build list of converting rules for making FFFFFF and FFOOOO
*         horizintal block rows.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] draggedTcamRuleSize      - size of TCAM rule of dragged vTcam in minimal rule units.
*                                       None.
*
* @note Fills static work_rules_arr variable.
*
*/
GT_VOID prvCpssDxChVirtualTcamDbSegSpaceWorkDbBuld80bytesMakingRuleList
(
    IN     GT_U32                                              draggedTcamRuleSize
)
{
    typedef struct
    {
        PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR *rulesArrPtr;
        GT_U32                                             rulesArrSize;
    } PRV_RULE_LIST_STC;
    static PRV_RULE_LIST_STC ruleListArr[] =
    {
        {rules_target_40_dragged_10_arr,
            (sizeof(rules_target_40_dragged_10_arr) / sizeof(rules_target_40_dragged_10_arr[0]))},
        {rules_target_50_dragged_10_arr,
            (sizeof(rules_target_50_dragged_10_arr) / sizeof(rules_target_50_dragged_10_arr[0]))},
        {rules_target_60_dragged_30_arr,
            (sizeof(rules_target_60_dragged_30_arr) / sizeof(rules_target_60_dragged_30_arr[0]))},
        {rules_target_60_dragged_10_arr,
            (sizeof(rules_target_60_dragged_10_arr) / sizeof(rules_target_60_dragged_10_arr[0]))},
        {rules_miscellanous_arr,
            (sizeof(rules_miscellanous_arr) / sizeof(rules_miscellanous_arr[0]))}
    };
    static GT_U32 ruleListArrsize =
        (sizeof(ruleListArr) / sizeof(ruleListArr[0]));
    GT_U32 listIdx, ruleIdx;
    PRV_RULE_LIST_STC *listArrPtr;
    GT_U32            listArrSize;
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR rulePtr;
    GT_BOOL           applicable;
    GT_U32            ffffffSrcNumOf;
    GT_U32            ffffffDstNumOf;
    GT_U32            ffxxxxSrcNumOf;
    GT_U32            ffxxxxDstNumOf;

    listArrPtr   = ruleListArr;
    listArrSize  = ruleListArrsize;

    work_rules_arr_used = 0;

    for (listIdx = 0; (listIdx < listArrSize); listIdx++)
    {
        for (ruleIdx = 0; (ruleIdx < listArrPtr[listIdx].rulesArrSize); ruleIdx++)
        {
            rulePtr = listArrPtr[listIdx].rulesArrPtr[ruleIdx];
            applicable = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRuleIsApplicable(
                rulePtr, draggedTcamRuleSize);
            if (applicable == GT_FALSE) continue;

            prvCpssDxChVirtualTcamDbSegSpaceWorkDbRule80byteMakeHoleCapacityGet(
                rulePtr, &ffffffSrcNumOf, &ffffffDstNumOf,
                &ffxxxxSrcNumOf, &ffxxxxDstNumOf);
            if ((ffffffSrcNumOf >= ffffffDstNumOf) && (ffxxxxSrcNumOf >= ffxxxxDstNumOf)) continue;

            work_rules_arr[work_rules_arr_used] = rulePtr;
            work_rules_arr_used  ++;

            if (work_rules_arr_used >= (sizeof(work_rules_arr) / sizeof(work_rules_arr[0])))
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return;
            }
        }
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId80bytesHolesMake function
* @endinternal
*
* @brief   Drag vTcams in blocks mapped to required lookup id and free blocks
*         in order to make 60-byte (FFFFFF) and 20-byte (FFOOOO) holes
*         for future moving to new places making free space ro 80-byte rules.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] checkWithoutUpdate       - GT_TRUE  - vTcams dragging checked using work DB,
*                                      permanent DB and HW not updated
*                                      GT_FALSE - vTcams dragged in permanent DB and HW
*
* @param[out] actualCapacityPtr        - (pointer to) TCAM Rules amount that can be allocated
*                                      after hole containing horizontal block rows moving.
* @param[out] freeBlocksBmpArr[]       - (pointer to)bitmap array of indexes of free blocks
* @param[out] lookupBlocksBmpArr[]     - (pointer to)bitmap array of indexes of blocks mapped
*                                      to lookup of vTcams, Work DB was filled.
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId80bytesHolesMake
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN     GT_BOOL                                                    checkWithoutUpdate,
    OUT    GT_U32                                                     *actualCapacityPtr,
    OUT    GT_U32                                                     freeBlocksBmpArr[],
    OUT    GT_U32                                                     lookupBlocksBmpArr[]
)
{
    GT_STATUS                                   rc, totalRc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr;
    PRV_SEG_DRAG_WORK_DB_STC                    workDb;
    GT_U32                                      sip5RuleSizesArr[] = {1,2,3};
    GT_U32                                      ruleSizesArrSize;
    GT_U32                                      *ruleSizesArr;
    GT_U32                                      ruleSizeIdx;
    GT_U32                                      ruleSize;
    GT_U32                                      vTcamId;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr;
    PRV_CPSS_AVL_TREE_ID                        segmentsTree;
    GT_U32                                      actualCapacity;
    GT_U32                                      savedActualCapacity;
    GT_U32                                      draggedVTcamsNum;

    *actualCapacityPtr = 0;

    if (tcamSegCfgPtr->deviceClass ==
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
    {
        ruleSizesArr     = sip5RuleSizesArr;
        ruleSizesArrSize = (sizeof(sip5RuleSizesArr) / sizeof(sip5RuleSizesArr[0]));
    }
    else
    {
        /* not relevant to legacy devices */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbInit(
        tcamSegCfgPtr, allocRequestPtr->lookupId, &workDb);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnable != GT_FALSE)
    {
        cpssOsPrintf("WorkDB after Init\n");
        prvCpssDxChVirtualTcamDbSegSpaceWorkDbDump(&workDb);
    }

    pVTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    totalRc = GT_OK;
    while (1) /* break conditions at the end */
    {
        draggedVTcamsNum    = 0;
        actualCapacity      =
            prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleCapacityGet(
                &workDb);
        if (actualCapacity >= allocRequestPtr->rulesAmount) break;
        savedActualCapacity = actualCapacity;

        for (ruleSizeIdx = 0; (ruleSizeIdx < ruleSizesArrSize); ruleSizeIdx++)
        {
            ruleSize = ruleSizesArr[ruleSizeIdx];
            prvCpssDxChVirtualTcamDbSegSpaceWorkDbBuld80bytesMakingRuleList(
                ruleSize);
            for (vTcamId = 0; (vTcamId < pVTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
            {
                /* bypass not existing vTcams and vTcams with other lookupId or other ruleSize */
                vTcamInfoPtr = pVTcamMngPtr->vTcamCfgPtrArr[vTcamId];
                if (vTcamInfoPtr == NULL) continue;
                if (vTcamInfoPtr->tcamSegCfgPtr != tcamSegCfgPtr)
                {
                    continue;
                }
                if (allocRequestPtr->lookupId !=
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(
                        vTcamInfoPtr->tcamInfo.clientGroup,
                        vTcamInfoPtr->tcamInfo.hitNumber)) continue;
                if (ruleSize != prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
                    tcamSegCfgPtr, vTcamInfoPtr->tcamInfo.ruleSize)) continue;

                /* dragged vTcam treatment */
                draggedVTcamsNum ++;
                /* mark vTcam space as "dragged" */
                rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbLoadVTcamSpaceTree(
                    tcamSegCfgPtr, &workDb, vTcamInfoPtr->segmentsTree);
                if (rc != GT_OK) totalRc = rc;
                if (totalRc != GT_OK) break;

                if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnable != GT_FALSE)
                {
                    cpssOsPrintf("WorkDB after Load vTcam %d\n", vTcamId);
                    prvCpssDxChVirtualTcamDbSegSpaceWorkDbDump(&workDb);
                }
                /* drag space in DB by converting rules */
                rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDb80bytesMakingConvertByRuleList(
                    tcamSegCfgPtr, &workDb, work_rules_arr_used, work_rules_arr,
                   allocRequestPtr->rulesAmount, &actualCapacity);
                if (rc != GT_OK) totalRc = rc;
                if (totalRc != GT_OK) break;

                if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnable != GT_FALSE)
                {
                    cpssOsPrintf("WorkDB after Convert by rules \n");
                    prvCpssDxChVirtualTcamDbSegSpaceWorkDbDump(&workDb);
                }

                if (checkWithoutUpdate == GT_FALSE)
                {
                    /* Retrieve new vTcam Layout segment tree */
                    rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveVTcamNewSpaceTree(
                        tcamSegCfgPtr, &workDb, vTcamId, &segmentsTree);
                    if (rc != GT_OK) totalRc = rc;
                    if (totalRc != GT_OK) break;

                    rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamToNewSegmentTree(
                        tcamSegCfgPtr, vTcamId, segmentsTree);
                    if (rc != GT_OK) totalRc = rc;
                    if (totalRc != GT_OK) break;
                }
                /* dragged vTcam DB cleanup: Dragged to Other */
                rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbDraggedToOther(
                    tcamSegCfgPtr, &workDb);
                if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnable != GT_FALSE)
                {
                    cpssOsPrintf("WorkDB after cleanup Dragged to Other \n");
                    prvCpssDxChVirtualTcamDbSegSpaceWorkDbDump(&workDb);
                }

                if (actualCapacity >= allocRequestPtr->rulesAmount) break;
            }
            if (totalRc != GT_OK) break;
            if (actualCapacity >= allocRequestPtr->rulesAmount) break;
        }
        if (totalRc != GT_OK) break;
        if (actualCapacity >= allocRequestPtr->rulesAmount) break;
        if (savedActualCapacity >= actualCapacity) break; /* no free space added */
    }

    if ((checkWithoutUpdate == GT_FALSE) && (draggedVTcamsNum > 0))
    {
        rc = prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpace(
            tcamSegCfgPtr, &workDb, allocRequestPtr->lookupId);
        if (rc != GT_OK) totalRc = rc;
        prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeNeighbors(tcamSegCfgPtr);
    }
    prvCpssDxChVirtualTcamDbSegSpaceWorkDbBlockBitmapsGet(
        tcamSegCfgPtr, &workDb, freeBlocksBmpArr, lookupBlocksBmpArr);
    prvCpssDxChVirtualTcamDbSegSpaceWorkDbFree(&workDb);
    /* add to bitmap blocks mapped to lookup and having no free space */
    /* workDb never was aware about them                              */
    prvCpssDxChVirtualTcamDbSegSpaceAddFullUsedBlocksToBitmap(
        tcamSegCfgPtr, allocRequestPtr->lookupId, lookupBlocksBmpArr);

    *actualCapacityPtr = actualCapacity;
    return totalRc;
}

/* lookup moving to other blocks */

/**
* @internal prvCpssDxChVirtualTcamDbSegSpace80bytesLookupRowsStatisticsGet function
* @endinternal
*
* @brief   Get block pairs (even and odd) and row types statistics for lookup.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup Id
* @param[in] freeBlocksBmpArr[]       - (pointer to)bitmap array of indexes of free blocks
* @param[in] lookupBlocksBmpArr[]     - (pointer to)bitmap array of indexes of blocks mapped
*                                      to lookup of vTcams, Work DB was filled.
*
* @param[out] blocksOwnOwnNumPtr       - (pointer to)number of blocks pair both mapped to the given lookup
* @param[out] blocksOwnOtherNumPtr     - (pointer to)number of blocks pair one mapped to the given lookup
*                                      and other mapped to other lookup
* @param[out] blocksOwnFreeNumPtr      - (pointer to)number of blocks pair one mapped to the given lookup
*                                      and other free
* @param[out] blocksOtherFreeNumPtr    - (pointer to)number of blocks pair one mapped to other lookup
*                                      and other free
* @param[out] blocksFreeFreeNumPtr     - (pointer to)number of blocks pair both free
* @param[out] lookup80byteRulesNumPtr  - (pointer to)number of 80-bytes rules in all vTcams
*                                      of the given lookup
*                                       None.
*/
static GT_VOID prvCpssDxChVirtualTcamDbSegSpace80bytesLookupRowsStatisticsGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                       lookupId,
    IN     GT_U32                                       freeBlocksBmpArr[],
    IN     GT_U32                                       lookupBlocksBmpArr[],
    OUT    GT_U32                                       *blocksOwnOwnNumPtr,
    OUT    GT_U32                                       *blocksOwnOtherNumPtr,
    OUT    GT_U32                                       *blocksOwnFreeNumPtr,
    OUT    GT_U32                                       *blocksOtherFreeNumPtr,
    OUT    GT_U32                                       *blocksFreeFreeNumPtr,
    OUT    GT_U32                                       *lookup80byteRulesNumPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC         *vTcamInfoPtr;
    GT_U32                                              blockIndex;
    GT_U32                                              vTcamId;
    GT_U32                                              freeBlock0;
    GT_U32                                              freeBlock1;
    GT_U32                                              lookupBlock0;
    GT_U32                                              lookupBlock1;
    GT_U32                                              vTcamLookupId;
    GT_U32                                              status;

    /* init */
    *blocksOwnOwnNumPtr       = 0;
    *blocksOwnOtherNumPtr     = 0;
    *blocksOwnFreeNumPtr      = 0;
    *blocksOtherFreeNumPtr    = 0;
    *blocksFreeFreeNumPtr     = 0;
    *lookup80byteRulesNumPtr  = 0;

    pVTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    for (vTcamId = 0; (vTcamId < pVTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
    {
        vTcamInfoPtr = pVTcamMngPtr->vTcamCfgPtrArr[vTcamId];
        if (vTcamInfoPtr == NULL) continue;
        if (vTcamInfoPtr->tcamSegCfgPtr != tcamSegCfgPtr)
        {
            continue;
        }
        if (vTcamInfoPtr->tcamInfo.ruleSize !=
            CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E) continue;
        vTcamLookupId =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(
                vTcamInfoPtr->tcamInfo.clientGroup,
                vTcamInfoPtr->tcamInfo.hitNumber);
        if (vTcamLookupId != lookupId) continue;

        lookup80byteRulesNumPtr += vTcamInfoPtr->rulesAmount;
    }

    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); blockIndex += 2)
    {
        freeBlock0    =
            ((freeBlocksBmpArr[blockIndex / 32] >> (blockIndex % 32)) & 1);
        freeBlock1    =
            ((freeBlocksBmpArr[(blockIndex + 1) / 32] >> ((blockIndex + 1) % 32)) & 1);
        lookupBlock0  =
            ((lookupBlocksBmpArr[blockIndex / 32] >> (blockIndex % 32)) & 1);
        lookupBlock1  =
            ((lookupBlocksBmpArr[(blockIndex + 1) / 32] >> ((blockIndex + 1) % 32)) & 1);

        if ((freeBlock0 & lookupBlock0) || (freeBlock1 & lookupBlock1))
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            return;
        }

        status = (lookupBlock1 << 3) | (lookupBlock0 << 2) | (freeBlock1 << 1) | freeBlock0;
        switch (status)
        {
            case 0: /*all busy by other */
                break;
            case 1:
            case 2:
                (*blocksOtherFreeNumPtr) ++;
                break;
            case 3:
                (*blocksFreeFreeNumPtr)  ++;
                break;
            case 4:
            case 8:
                (*blocksOwnOtherNumPtr)  ++;
                break;
            case 9:
            case 6:
                (*blocksOwnFreeNumPtr)   ++;
                break;
            case 12:
                (*blocksOwnOwnNumPtr)    ++;
                break;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                return ;
        }
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherBlock function
* @endinternal
*
* @brief   Drag vTcams from block mapped to some lookup id to free block.
*         Source block becomes free.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] sourceBlockIndex         - source block index (mapped block)
*                                      targetBlocksIndex  - target block index (free block)
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherBlock
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                       sourceBlockIndex,
    IN     GT_U32                                       targetBlockIndex
)
{
    GT_STATUS                                           rc, totalRc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *pVTcamMngPtr;
    GT_U32                                              lookupId;
    GT_U32                                              blockBase;
    GT_U32                                              blocksAmount;
    GT_U32                                              blocksInColumn;
    GT_U32                                              blocksInRow;
    GT_U32                                              draggedSegments;
    GT_U32                                              vTcamId;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC         *vTcamInfoPtr;
    PRV_CPSS_AVL_TREE_ID                                segmentsTree;
    GT_VOID_PTR                                         dbEntryPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC     *segPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC     *segCopyPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC     *segCopy1Ptr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC     segPattern;
    PRV_CPSS_AVL_TREE_SEEK_ENT                          seekType;
    PRV_CPSS_AVL_TREE_PATH                              path;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT       segmentColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP   blockSegmentColumnsMap;
    GT_U32                                              sourceBlockRowsBase;
    GT_U32                                              targetBlockRowsBase;
    GT_U32                                              sourceHorzBlockIndex;
    GT_U32                                              targetHorzBlockIndex;

    totalRc = GT_OK;

    if (tcamSegCfgPtr->deviceClass !=
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    blocksInColumn =
        (tcamSegCfgPtr->tcamRowsAmount
         / tcamSegCfgPtr->lookupRowAlignment);

    blocksAmount = blocksInColumn * blocksInRow;

    if (sourceBlockIndex >= blocksAmount) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (targetBlockIndex >= blocksAmount) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    lookupId = tcamSegCfgPtr->blockInfArr[sourceBlockIndex].lookupId;
    if (lookupId == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    if (tcamSegCfgPtr->blockInfArr[targetBlockIndex].lookupId
        != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    sourceBlockRowsBase =
        ((sourceBlockIndex / blocksInRow) * tcamSegCfgPtr->lookupRowAlignment);
    targetBlockRowsBase =
        ((targetBlockIndex / blocksInRow) * tcamSegCfgPtr->lookupRowAlignment);
    sourceHorzBlockIndex = (sourceBlockIndex % blocksInRow);
    targetHorzBlockIndex = (targetBlockIndex % blocksInRow);

    pVTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    for (vTcamId = 0; (vTcamId < pVTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
    {
        /* bypass not existing vTcams and vTcams with other lookupId or orher ruleSize */
        vTcamInfoPtr = pVTcamMngPtr->vTcamCfgPtrArr[vTcamId];
        if (vTcamInfoPtr == NULL) continue;
        if (lookupId !=
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(
                vTcamInfoPtr->tcamInfo.clientGroup,
                vTcamInfoPtr->tcamInfo.hitNumber)) continue;

        /* check in this vTcam has some space in source block */
        draggedSegments = 0;
        seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        while (GT_FALSE != prvCpssAvlPathSeek(
            vTcamInfoPtr->segmentsTree, seekType, path, &dbEntryPtr))
        {
            seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
            segPtr = dbEntryPtr;
            blockBase = segPtr->rowsBase
                - (segPtr->rowsBase % tcamSegCfgPtr->lookupRowAlignment);
            if (blockBase < sourceBlockRowsBase)
            {
                cpssOsMemCpy(&segPattern, segPtr, sizeof(segPattern));
                segPattern.segmentColumnsMap =
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
                segPattern.rowsBase = sourceBlockRowsBase;
                if (GT_FALSE != prvCpssAvlPathFind(
                    vTcamInfoPtr->segmentsTree,
                    PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
                    &segPattern, path, &dbEntryPtr))
                {
                    break;
                }
                segPtr    = dbEntryPtr;
                blockBase = (segPtr->rowsBase
                    - (segPtr->rowsBase % tcamSegCfgPtr->lookupRowAlignment));
            }
            if (blockBase > sourceBlockRowsBase) break;

            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
               segPtr->segmentColumnsMap, sourceHorzBlockIndex);
            if (segmentColumns ==
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;
            draggedSegments = 1;
            break;
        }
        if (draggedSegments == 0) continue; /* this vTcam has no space in source block */

        /* new segment tree for vTcam */
        rc = prvCpssAvlTreeCreate(
            (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
            pVTcamMngPtr->segNodesPool,
            &segmentsTree);
        if (rc != GT_OK)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            totalRc = rc;
            break;
        }

        seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        while (GT_FALSE != prvCpssAvlPathSeek(
            vTcamInfoPtr->segmentsTree, seekType, path, &dbEntryPtr))
        {
            seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
            segPtr = dbEntryPtr;

            segCopyPtr = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
            if (segCopyPtr == NULL)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                totalRc = GT_NO_RESOURCE;
                break;
            }
            cpssOsMemCpy(
                segCopyPtr, segPtr,
                sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));

            draggedSegments = 0;
            blockBase = segPtr->rowsBase
                - (segPtr->rowsBase % tcamSegCfgPtr->lookupRowAlignment);
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
               segPtr->segmentColumnsMap, sourceHorzBlockIndex);
            if ((blockBase == sourceBlockRowsBase) && (segmentColumns !=
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E))
            {
                draggedSegments = 1;
            }

            if (draggedSegments != 0)
            {
                blockSegmentColumnsMap =
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                    blockSegmentColumnsMap, sourceHorzBlockIndex, segmentColumns);
                if (blockSegmentColumnsMap != segPtr->segmentColumnsMap)
                {
                    /* split horizontal to moved and remainder */
                    segCopy1Ptr = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
                    if (segCopy1Ptr == NULL)
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segCopyPtr);
                        totalRc = GT_NO_RESOURCE;
                        break;
                    }
                    cpssOsMemCpy(
                        segCopy1Ptr, segPtr,
                        sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));

                    /* remove horizontatal block from remainder */
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                        segCopy1Ptr->segmentColumnsMap, sourceHorzBlockIndex,
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E);

                    rc = prvCpssAvlItemInsert(segmentsTree, segCopy1Ptr);
                    if (rc != GT_OK)
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segCopy1Ptr);
                        cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segCopyPtr);
                        totalRc = rc;
                        break;
                    }
                }

                /* new row base in target block */
                segCopyPtr->rowsBase =
                    (segPtr->rowsBase - sourceBlockRowsBase + targetBlockRowsBase);
                /* move only relevant horizontal block of moved segment */
                segCopyPtr->segmentColumnsMap =
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                    segCopyPtr->segmentColumnsMap, targetHorzBlockIndex, segmentColumns);
            }

            rc = prvCpssAvlItemInsert(segmentsTree, segCopyPtr);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segCopyPtr);
                totalRc = rc;
                break;
            }

            if (draggedSegments == 0) continue; /* segment not modified, just copied */

            /* free source of moved space */
            segCopy1Ptr = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
            if (segCopy1Ptr == NULL)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                totalRc = GT_NO_RESOURCE;
                break;
            }
            cpssOsMemCpy(
                segCopy1Ptr, segPtr,
                sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
            /* move only relevant horizontal block of moved segment */
            segCopy1Ptr->segmentColumnsMap =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                segCopy1Ptr->segmentColumnsMap, sourceHorzBlockIndex, segmentColumns);
            rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegment(
                tcamSegCfgPtr, segCopy1Ptr);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segCopy1Ptr);
                totalRc = rc;
                break;
            }

            /* occupate target of moved space */
            rc = prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree(
                tcamSegCfgPtr, lookupId,
                segCopyPtr->rowsBase, segCopyPtr->rowsAmount,
                segCopyPtr->segmentColumnsMap);
            if (rc != GT_OK)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                totalRc = rc;
                break;
            }
        }
        if (totalRc != GT_OK)
        {
            prvCpssAvlTreeDelete(
                segmentsTree,
                (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
                (GT_VOID*)pVTcamMngPtr->segNodesPool);
            break;
        }
        rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamToNewSegmentTree(
            tcamSegCfgPtr, vTcamId, segmentsTree);
        if (rc != GT_OK)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            totalRc = rc;
            break;
        }
    }

    prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeNeighbors(tcamSegCfgPtr);
    return totalRc;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherSetOfBlock function
* @endinternal
*
* @brief   Drag vTcams from set of blocks mapped to other lookups to free blocks.
*         Source blocks become free.
*         Blocks mapped to other lookups (not that the lookupBlocksBmpArr related to)
*         that have pair-neighbors of the treated (own) lookup blocks swapped with
*         free blocks pair-neighbors of the other lookup blocks.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] swapBlocksNeeded         - amount of blocks needed to swap
* @param[in] checkOnly                - GT_TRUE - do not drag vTcams, check space only
*                                      GT_FALSE - drag vTcams
*                                      In both cases bitmaps below updated.
* @param[in,out] freeBlocksBmpArr[]       - (pointer to)bitmap array of indexes of free blocks
* @param[in,out] lookupBlocksBmpArr[]     - (pointer to)bitmap array of indexes of blocks mapped
*                                      to lookup of vTcams, Work DB was filled.
* @param[in,out] freeBlocksBmpArr[]       - (pointer to)updated bitmap array of indexes of free blocks
* @param[in,out] lookupBlocksBmpArr[]     - (pointer to)updated bitmap array of indexes of blocks mapped
*
* @retval GT_OK                    - success.
* @retval GT_FULL                  - no enuogh free place, actual when checkOnly == TRUE
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherSetOfBlock
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                       swapBlocksNeeded,
    IN     GT_BOOL                                      checkOnly,
    INOUT  GT_U32                                       freeBlocksBmpArr[],
    INOUT  GT_U32                                       lookupBlocksBmpArr[]
)
{
    GT_STATUS rc;
    GT_U32    sourceBlockIdx;
    GT_U32    sourceBlockIdx1;
    GT_U32    targetBlockIdx;
    GT_U32    pairBlockIdx;
    GT_U32    swapBlocksCount;

    /* Blocks mapped to other lookups (not that the lookupBlocksBmpArr related to) */
    /* that have pair-neighbors of the treated (own) lookup blocks swapped with    */
    /* free blocks pair-neighbors of the other lookup blocks.                      */
    sourceBlockIdx  = 0;
    targetBlockIdx  = 0;
    for (swapBlocksCount = 0; (swapBlocksCount < swapBlocksNeeded); swapBlocksCount++)
    {
        for (; (sourceBlockIdx < tcamSegCfgPtr->blocksAmount); sourceBlockIdx ++)
        {
            /* pare block - other from the pair */
            pairBlockIdx = (sourceBlockIdx ^ 1); /* swap bit0 only*/
            /* source block should be mapped to other lookup */
            /* pair block should be mapped to given lookup   */
            if (freeBlocksBmpArr[sourceBlockIdx / 32] & (1 << (sourceBlockIdx % 32)))
            {
                /* source block is free */
                continue;
            }
            if (lookupBlocksBmpArr[sourceBlockIdx / 32] & (1 << (sourceBlockIdx % 32)))
            {
                /* source block mapped to given lookup */
                continue;
            }
            if ((lookupBlocksBmpArr[pairBlockIdx / 32] & (1 << (pairBlockIdx % 32))) == 0)
            {
                /* pair block not mapped to given lookup */
                continue;
            }
            break;
        }
        if (sourceBlockIdx >= tcamSegCfgPtr->blocksAmount) break;
        for (; (targetBlockIdx < tcamSegCfgPtr->blocksAmount); targetBlockIdx ++)
        {
            /* pare block - other from the pair */
            pairBlockIdx = (targetBlockIdx ^ 1); /* swap bit0 only*/
            /* target block should be free                  */
            /* pair block should be mapped to other lookup  */
            if ((freeBlocksBmpArr[targetBlockIdx / 32] & (1 << (targetBlockIdx % 32))) == 0)
            {
                /* target block is not free */
                continue;
            }
            if (freeBlocksBmpArr[pairBlockIdx / 32] & (1 << (pairBlockIdx % 32)))
            {
                /* pair block is free */
                continue;
            }
            if (lookupBlocksBmpArr[pairBlockIdx / 32] & (1 << (pairBlockIdx % 32)))
            {
                /* pair block mapped to given lookup */
                continue;
            }
            break;
        }
        if (targetBlockIdx >= tcamSegCfgPtr->blocksAmount) break;

        if (checkOnly == GT_FALSE)
        {
            rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherBlock(
                tcamSegCfgPtr, sourceBlockIdx, targetBlockIdx);
            if (rc != GT_OK) return rc;
        }

        /* source became free, target became busy                                   */
        /* both source and target were not mapped to own lookup and stay not mapped */
        freeBlocksBmpArr[sourceBlockIdx / 32] |= (1 << (sourceBlockIdx % 32));
        freeBlocksBmpArr[targetBlockIdx / 32] &= (~ (1 << (targetBlockIdx % 32)));

        /* start search from the next pair */
        sourceBlockIdx = ((sourceBlockIdx + 2) & (~ 1));
        targetBlockIdx = ((targetBlockIdx + 2) & (~ 1));
    }

    /* move two of other-blocks from own-other pairs to both blocks in pair free-free */
    /* counted as (1 + 1 - 1) == 1 block swapping                                     */
    sourceBlockIdx   = 0;
    sourceBlockIdx1  = 0;
    targetBlockIdx   = 0;
    for (/*none*/; (swapBlocksCount < swapBlocksNeeded); swapBlocksCount++)
    {
        for (; (sourceBlockIdx < tcamSegCfgPtr->blocksAmount); sourceBlockIdx ++)
        {
            /* pare block - other from the pair */
            pairBlockIdx = (sourceBlockIdx ^ 1); /* swap bit0 only*/
            /* source block should be mapped to other lookup */
            /* pair block should be mapped to given lookup   */
            if (freeBlocksBmpArr[sourceBlockIdx / 32] & (1 << (sourceBlockIdx % 32)))
            {
                /* source block is free */
                continue;
            }
            if (lookupBlocksBmpArr[sourceBlockIdx / 32] & (1 << (sourceBlockIdx % 32)))
            {
                /* source block mapped to given lookup */
                continue;
            }
            if ((lookupBlocksBmpArr[pairBlockIdx / 32] & (1 << (pairBlockIdx % 32))) == 0)
            {
                /* pair block not mapped to given lookup */
                continue;
            }
            break;
        }
        if (sourceBlockIdx >= tcamSegCfgPtr->blocksAmount) break;

        sourceBlockIdx1 = ((sourceBlockIdx + 2) & (~ 1)); /* first in next pair */
        for (; (sourceBlockIdx1 < tcamSegCfgPtr->blocksAmount); sourceBlockIdx1 ++)
        {
            /* pare block - other from the pair */
            pairBlockIdx = (sourceBlockIdx1 ^ 1); /* swap bit0 only*/
            /* source block should be mapped to other lookup */
            /* pair block should be mapped to given lookup   */
            if (freeBlocksBmpArr[sourceBlockIdx1 / 32] & (1 << (sourceBlockIdx1 % 32)))
            {
                /* source block is free */
                continue;
            }
            if (lookupBlocksBmpArr[sourceBlockIdx1 / 32] & (1 << (sourceBlockIdx1 % 32)))
            {
                /* source block mapped to given lookup */
                continue;
            }
            if ((lookupBlocksBmpArr[pairBlockIdx / 32] & (1 << (pairBlockIdx % 32))) == 0)
            {
                /* pair block not mapped to given lookup */
                continue;
            }
            break;
        }
        if (sourceBlockIdx1 >= tcamSegCfgPtr->blocksAmount) break;

        for (; (targetBlockIdx < tcamSegCfgPtr->blocksAmount); targetBlockIdx ++)
        {
            /* pare block - other from the pair */
            pairBlockIdx = (targetBlockIdx + 1);
            /* target block should be free */
            /* pair block should be free   */
            if ((freeBlocksBmpArr[targetBlockIdx / 32] & (1 << (targetBlockIdx % 32))) == 0)
            {
                /* target block is not free */
                continue;
            }
            if ((freeBlocksBmpArr[pairBlockIdx / 32] & (1 << (pairBlockIdx % 32))) == 0)
            {
                /* pair block is not free */
                continue;
            }
            break;
        }

        if (checkOnly == GT_FALSE)
        {
            rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherBlock(
                tcamSegCfgPtr, sourceBlockIdx, targetBlockIdx);
            if (rc != GT_OK) return rc;
            rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherBlock(
                tcamSegCfgPtr, sourceBlockIdx1, (targetBlockIdx + 1));
            if (rc != GT_OK) return rc;
        }

        /* both sources became free, target pair became busy                        */
        /* both source and target were not mapped to own lookup and stay not mapped */
        freeBlocksBmpArr[sourceBlockIdx / 32] |= (1 << (sourceBlockIdx % 32));
        freeBlocksBmpArr[sourceBlockIdx1 / 32] |= (1 << (sourceBlockIdx1 % 32));
        freeBlocksBmpArr[targetBlockIdx / 32] &= (~ (1 << (targetBlockIdx % 32)));
        freeBlocksBmpArr[(targetBlockIdx + 1) / 32] &= (~ (1 << ((targetBlockIdx + 1) % 32)));

        sourceBlockIdx = ((sourceBlockIdx1 + 2) & (~ 1)); /* first in next pair */
        targetBlockIdx += 2; /* next pair */
    }

    /* if needed uniting block pairs other-free + other-free => other-other + free-free */
    sourceBlockIdx = 0;
    targetBlockIdx = 0;
    for (/*none*/; (swapBlocksCount < swapBlocksNeeded); swapBlocksCount++)
    {
        for (; (targetBlockIdx < tcamSegCfgPtr->blocksAmount); targetBlockIdx ++)
        {
            /* pare block - other from the pair */
            pairBlockIdx = (targetBlockIdx ^ 1); /* swap bit0 only*/
            /* target block should be free                  */
            /* pair block should be mapped to other lookup  */
            if ((freeBlocksBmpArr[targetBlockIdx / 32] & (1 << (targetBlockIdx % 32))) == 0)
            {
                /* target block is not free */
                continue;
            }
            if (freeBlocksBmpArr[pairBlockIdx / 32] & (1 << (pairBlockIdx % 32)))
            {
                /* pair block is free */
                continue;
            }
            if (lookupBlocksBmpArr[pairBlockIdx / 32] & (1 << (pairBlockIdx % 32)))
            {
                /* pair block mapped to given lookup */
                continue;
            }
            break;
        }
        if (targetBlockIdx >= tcamSegCfgPtr->blocksAmount) break;

        /* start search from the next pair */
        sourceBlockIdx = ((targetBlockIdx + 2) & (~ 1));
        for (; (sourceBlockIdx < tcamSegCfgPtr->blocksAmount); sourceBlockIdx ++)
        {
            /* pare block - other from the pair */
            pairBlockIdx = (sourceBlockIdx ^ 1); /* swap bit0 only*/
            /* source block should be mapped to other lookup      */
            /* pair block should be free                          */
            if (freeBlocksBmpArr[sourceBlockIdx / 32] & (1 << (sourceBlockIdx % 32)))
            {
                /* source block is free */
                continue;
            }
            if (lookupBlocksBmpArr[sourceBlockIdx / 32] & (1 << (sourceBlockIdx % 32)))
            {
                /* source block mapped to given lookup */
                continue;
            }
            if ((freeBlocksBmpArr[pairBlockIdx / 32] & (1 << (pairBlockIdx % 32))) == 0)
            {
                /* pair block not free */
                continue;
            }
            break;
        }
        if (sourceBlockIdx >= tcamSegCfgPtr->blocksAmount) break;

        if (checkOnly == GT_FALSE)
        {
            rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherBlock(
                tcamSegCfgPtr, sourceBlockIdx, targetBlockIdx);
            if (rc != GT_OK) return rc;
        }

        /* source became free, target became busy                                   */
        /* both source and target were not mapped to own lookup and stay not mapped */
        freeBlocksBmpArr[sourceBlockIdx / 32] |= (1 << (sourceBlockIdx % 32));
        freeBlocksBmpArr[targetBlockIdx / 32] &= (~ (1 << (targetBlockIdx % 32)));

        /* start search from the next pair */
        targetBlockIdx = ((sourceBlockIdx + 2) & (~ 1));
    }

    if (swapBlocksCount < swapBlocksNeeded)
    {
        if (checkOnly == GT_FALSE)
        {
            /* this function should be called with checkOnly and success before */
            prvCpssDxChVirtualTcamDbSegmentTableException();
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherSpaceTree function
* @endinternal
*
* @brief   Drag vTcams from some space to new space.
*         Source space becomes free.
*         Space defined in horizontal block resolution
*         in PRV_HORZ_BLOCK_ROWS_RANGE_TO_VOIDPTR_MAC format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup Id
* @param[in] sourceSpaceTree          - source space tree (space used by vTcams of given lookup)
* @param[in] targetSpaceTree          - target space tree (free space)
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherSpaceTree
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                               lookupId,
    IN     PRV_CPSS_AVL_TREE_ID                 sourceSpaceTree,
    IN     PRV_CPSS_AVL_TREE_ID                 targetSpaceTree
)
{
    GT_STATUS                                           rc, totalRc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *pVTcamMngPtr;
    GT_U32                                              draggedSegments;
    GT_U32                                              vTcamId;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC         *vTcamInfoPtr;
    PRV_CPSS_AVL_TREE_SEEK_ENT                          seekType;
    PRV_CPSS_AVL_TREE_PATH                              path;
    GT_BOOL                                             rangeSeeked;
    PRV_CPSS_AVL_TREE_PATH                              rangePath;
    GT_VOID_PTR                                         dbEntryPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC     *segPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC     *segCopyPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC     *segCopy1Ptr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT       segmentColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP   segmentColumnsMap;
    PRV_CPSS_AVL_TREE_ID                                segmentsTree;
    GT_VOID*                                            rangeKeyDataPtr;
    GT_VOID*                                            rangeFoundDataPtr;
    GT_U32                                              blocksInRow;
    GT_U32                                              horzBlockIdx;
    GT_U32                                              horzBlockIndex;
    GT_U32                                              rowsBase;
    GT_U32                                              rowsAmount;
    GT_U32                                              commonRowsBase;
    GT_U32                                              commonRowsBound;
    GT_U32                                              remainderRowsBase;
    GT_U32                                              remainderRowsBound;
    PRV_CPSS_AVL_TREE_PATH                              targetRangePath;
    GT_VOID*                                            targetRangeDataPtr;
    GT_U32                                              targetRangeRowsUsed;
    GT_U32                                              targetRangeHorzBlockIndex;
    GT_U32                                              targetRangeRowsBase;
    GT_U32                                              targetRangeRowsAmount;
    GT_BOOL                                             targetEndOfTreeReached;
    GT_U32                                              workRowsNum;
    GT_U32                                              workRowsNum1;

    totalRc = GT_OK;
    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);
    segmentsTree = NULL;

    /* initialize destination space getting */
    if (GT_FALSE == prvCpssAvlPathSeek(
        targetSpaceTree, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
        targetRangePath, &targetRangeDataPtr))
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    targetEndOfTreeReached = GT_FALSE;
    targetRangeRowsUsed = 0;
    PRV_HORZ_BLOCK_ROWS_RANGE3_FROM_VOIDPTR_MAC(
        targetRangeDataPtr, targetRangeHorzBlockIndex,
        targetRangeRowsBase, targetRangeRowsAmount);

    pVTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    for (vTcamId = 0; (vTcamId < pVTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
    {
        /* bypass not existing vTcams and vTcams with other lookupId or other ruleSize */
        vTcamInfoPtr = pVTcamMngPtr->vTcamCfgPtrArr[vTcamId];
        if (vTcamInfoPtr == NULL) continue;
        if (vTcamInfoPtr->tcamSegCfgPtr != tcamSegCfgPtr)
        {
            continue;
        }
        if (lookupId !=
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(
                vTcamInfoPtr->tcamInfo.clientGroup,
                vTcamInfoPtr->tcamInfo.hitNumber)) continue;

        /* check in this vTcam has some space in source block */
        draggedSegments = 0;
        seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        while (GT_FALSE != prvCpssAvlPathSeek(
            vTcamInfoPtr->segmentsTree, seekType, path, &dbEntryPtr))
        {
            seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
            segPtr = dbEntryPtr;

            for (horzBlockIdx = 0; (horzBlockIdx < blocksInRow); horzBlockIdx++)
            {
                segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                    segPtr->segmentColumnsMap, horzBlockIdx);

                if (segmentColumns
                    == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;

                rangeKeyDataPtr =
                    PRV_HORZ_BLOCK_ROWS_RANGE3_TO_VOIDPTR_MAC(
                        horzBlockIdx, segPtr->rowsBase, 0/*rowsAmount*/);
                if (GT_FALSE != prvCpssAvlPathFind(
                    sourceSpaceTree,
                    PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
                    rangeKeyDataPtr, rangePath, &rangeFoundDataPtr))
                {
                    PRV_HORZ_BLOCK_ROWS_RANGE3_FROM_VOIDPTR_MAC(
                        rangeFoundDataPtr, horzBlockIndex, rowsBase, rowsAmount);
                    if ((horzBlockIdx == horzBlockIndex)
                        && ((rowsBase + rowsAmount) > segPtr->rowsBase))
                    {
                        draggedSegments = 1;
                        break;
                    }
                }
                if (GT_FALSE != prvCpssAvlPathFind(
                    sourceSpaceTree,
                    PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
                    rangeKeyDataPtr, rangePath, &rangeFoundDataPtr))
                {
                    PRV_HORZ_BLOCK_ROWS_RANGE3_FROM_VOIDPTR_MAC(
                        rangeFoundDataPtr, horzBlockIndex, rowsBase, rowsAmount);
                    if ((horzBlockIdx == horzBlockIndex)
                        && (rowsBase < (segPtr->rowsBase + segPtr->rowsAmount)))
                    {
                        draggedSegments = 1;
                        break;
                    }
                }
            }
            if (draggedSegments) break;
        }

        if (draggedSegments == 0) continue; /* this vTcam has no space in source tree */

        /* new segment tree for vTcam */
        rc = prvCpssAvlTreeCreate(
            (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
            pVTcamMngPtr->segNodesPool,
            &segmentsTree);
        if (rc != GT_OK)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            totalRc = rc;
            break;
        }

        seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        while (GT_FALSE != prvCpssAvlPathSeek(
            vTcamInfoPtr->segmentsTree, seekType, path, &dbEntryPtr))
        {
            seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
            segPtr = dbEntryPtr;

            for (horzBlockIdx = 0; (horzBlockIdx < blocksInRow); horzBlockIdx++)
            {
                segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                    segPtr->segmentColumnsMap, horzBlockIdx);

                if (segmentColumns
                    == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;

                segmentColumnsMap =
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                    segmentColumnsMap, horzBlockIdx, segmentColumns);

                remainderRowsBase = segPtr->rowsBase;
                remainderRowsBound = (segPtr->rowsBase + segPtr->rowsAmount);
                rangeKeyDataPtr =
                    PRV_HORZ_BLOCK_ROWS_RANGE3_TO_VOIDPTR_MAC(
                        horzBlockIdx, segPtr->rowsBase, 0/*rowsAmount*/);
                rangeSeeked = prvCpssAvlPathFind(
                    sourceSpaceTree,
                    PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
                    rangeKeyDataPtr, rangePath, &rangeFoundDataPtr);
                if (rangeSeeked != GT_FALSE)
                {
                    PRV_HORZ_BLOCK_ROWS_RANGE3_FROM_VOIDPTR_MAC(
                        rangeFoundDataPtr, horzBlockIndex, rowsBase, rowsAmount);
                    if (horzBlockIdx != horzBlockIndex) rangeSeeked = GT_FALSE;
                    if ((rowsBase + rowsAmount) <= segPtr->rowsBase) rangeSeeked = GT_FALSE;
                }
                if (rangeSeeked == GT_FALSE)
                {
                    rangeSeeked = prvCpssAvlPathFind(
                        sourceSpaceTree,
                        PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
                        rangeKeyDataPtr, rangePath, &rangeFoundDataPtr);
                }
                for (;
                    (rangeSeeked != GT_FALSE);
                    rangeSeeked = prvCpssAvlPathSeek(
                        sourceSpaceTree,
                        PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                        rangePath, &rangeFoundDataPtr))
                {
                    PRV_HORZ_BLOCK_ROWS_RANGE3_FROM_VOIDPTR_MAC(
                        rangeFoundDataPtr, horzBlockIndex, rowsBase, rowsAmount);

                    if (horzBlockIdx != horzBlockIndex) break;

                    commonRowsBase  = (segPtr->rowsBase > rowsBase)
                        ? segPtr->rowsBase : rowsBase;
                    commonRowsBound = (segPtr->rowsBase + segPtr->rowsAmount);
                    if (commonRowsBound > (rowsBase + rowsAmount))
                        commonRowsBound = (rowsBase + rowsAmount);

                    if (commonRowsBase >= commonRowsBound) break;

                    if (remainderRowsBase < commonRowsBase)
                    {
                        /* keep this space */
                        segCopyPtr = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
                        if (segCopyPtr == NULL)
                        {
                            prvCpssDxChVirtualTcamDbSegmentTableException();
                            totalRc = GT_NO_RESOURCE;
                            break;
                        }
                        cpssOsMemCpy(
                            segCopyPtr, segPtr,
                            sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
                        segCopyPtr->segmentColumnsMap = segmentColumnsMap;
                        segCopyPtr->rowsBase = remainderRowsBase;
                        segCopyPtr->rowsAmount = (remainderRowsBase - commonRowsBase);
                        rc = prvCpssAvlItemInsert(segmentsTree, segCopyPtr);
                        if (rc != GT_OK)
                        {
                            cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segCopyPtr);
                            prvCpssDxChVirtualTcamDbSegmentTableException();
                            totalRc = rc;
                            break;
                        }
                        remainderRowsBase = commonRowsBase;
                    }
                    if (totalRc != GT_OK) break;

                    /* free common space moving to destination ranges tree */
                    while (remainderRowsBase < commonRowsBound)
                    {
                        if (targetEndOfTreeReached != GT_FALSE)
                        {
                            prvCpssDxChVirtualTcamDbSegmentTableException();
                            totalRc = GT_FAIL;
                            break;
                        }
                        workRowsNum = (targetRangeRowsAmount - targetRangeRowsUsed);
                        workRowsNum1 = (commonRowsBound - remainderRowsBase);
                        if (workRowsNum > workRowsNum1) workRowsNum = workRowsNum1;

                        segCopyPtr = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
                        if (segCopyPtr == NULL)
                        {
                            prvCpssDxChVirtualTcamDbSegmentTableException();
                            totalRc = GT_NO_RESOURCE;
                            break;
                        }
                        /* insert new space to new vTcam segmentsTree */
                        cpssOsMemCpy(
                            segCopyPtr, segPtr,
                            sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
                        segCopyPtr->segmentColumnsMap =
                            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segCopyPtr->segmentColumnsMap, targetRangeHorzBlockIndex, segmentColumns);
                        segCopyPtr->rowsBase = (targetRangeRowsBase + targetRangeRowsUsed);
                        segCopyPtr->rowsAmount = workRowsNum;
                        rc = prvCpssAvlItemInsert(segmentsTree, segCopyPtr);
                        if (rc != GT_OK)
                        {
                            cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segCopyPtr);
                            prvCpssDxChVirtualTcamDbSegmentTableException();
                            totalRc = rc;
                            break;
                        }

                        /* free source vTcam space */
                        segCopy1Ptr = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
                        if (segCopy1Ptr == NULL)
                        {
                            prvCpssDxChVirtualTcamDbSegmentTableException();
                            totalRc = GT_NO_RESOURCE;
                            break;
                        }
                        cpssOsMemCpy(
                            segCopy1Ptr, segPtr,
                            sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
                        segCopy1Ptr->segmentColumnsMap = segmentColumnsMap;
                        segCopy1Ptr->rowsBase          = remainderRowsBase;
                        segCopy1Ptr->rowsAmount        = workRowsNum;
                        rc = prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit(
                            tcamSegCfgPtr, segCopy1Ptr);
                        if (rc != GT_OK)
                        {
                            cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segCopy1Ptr);
                            totalRc = rc;
                            break;
                        }

                        /* occupy destination vTcam space */
                        rc = prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree(
                            tcamSegCfgPtr, lookupId,
                            segCopyPtr->rowsBase, segCopyPtr->rowsAmount,
                            segCopyPtr->segmentColumnsMap);
                        if (rc != GT_OK)
                        {
                            totalRc = rc;
                            break;
                        }

                        remainderRowsBase   += workRowsNum;
                        targetRangeRowsUsed += workRowsNum;

                        /* retrieve new destination space */
                        if (targetRangeRowsUsed >= targetRangeRowsAmount)
                        {
                            if (GT_FALSE != prvCpssAvlPathSeek(
                                targetSpaceTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                                targetRangePath, &targetRangeDataPtr))
                            {
                                targetRangeRowsUsed = 0;
                                PRV_HORZ_BLOCK_ROWS_RANGE3_FROM_VOIDPTR_MAC(
                                    targetRangeDataPtr, targetRangeHorzBlockIndex,
                                    targetRangeRowsBase, targetRangeRowsAmount);
                            }
                            else
                            {
                                targetEndOfTreeReached = GT_TRUE;
                            }
                        }
                        if (totalRc != GT_OK) break;
                    }
                    if (totalRc != GT_OK) break;
                }
                if (remainderRowsBase < remainderRowsBound)
                {
                    /* keep this space (remainder) */
                    segCopyPtr = cpssBmPoolBufGet(pVTcamMngPtr->segmentsPool);
                    if (segCopyPtr == NULL)
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        totalRc = GT_NO_RESOURCE;
                        break;
                    }
                    cpssOsMemCpy(
                        segCopyPtr, segPtr,
                        sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
                    segCopyPtr->segmentColumnsMap = segmentColumnsMap;
                    segCopyPtr->rowsBase = remainderRowsBase;
                    segCopyPtr->rowsAmount = (remainderRowsBound - remainderRowsBase);
                    rc = prvCpssAvlItemInsert(segmentsTree, segCopyPtr);
                    if (rc != GT_OK)
                    {
                        cpssBmPoolBufFree(pVTcamMngPtr->segmentsPool, segCopyPtr);
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        totalRc = rc;
                        break;
                    }
                    remainderRowsBase = remainderRowsBound;
                }
                if (totalRc != GT_OK) break;
            }
            if (totalRc != GT_OK) break;
        }
        if (totalRc != GT_OK) break;

        rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamToNewSegmentTree(
            tcamSegCfgPtr, vTcamId, segmentsTree);
        if (rc != GT_OK)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            totalRc = rc;
            break;
        }
        segmentsTree = NULL; /* used and should not be removed */
    }

    if (segmentsTree != NULL)
    {
        /* loop upper was broken due to error */
        prvCpssAvlTreeDelete(
            segmentsTree,
            (GT_VOIDFUNCPTR)cpssBmPoolBufFree,
            (GT_VOID*)pVTcamMngPtr->segmentsPool);
    }

    prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeNeighbors(tcamSegCfgPtr);
    return totalRc;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet function
* @endinternal
*
* @brief   Get bitmap of horizontal block rows containg free space
*         of given shape.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] blockIndex               - block index
* @param[in] freeColumns              - shape of free part of horizontal row.
*
* @param[out] rowsBitmapArr[]          - (pointer to) bitmap for relative rows 0..255
*                                      0 - shape of free part of horizontal row is other
*                                      1 - shape of free part of horizontal row is as given
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC  *tcamSegCfgPtr,
    IN     GT_U32                                          blockIndex,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT   freeColumns,
    OUT    GT_U32                                          rowsBitmapArr[]
)
{
    GT_U32                                              blocksInRow;
    GT_U32                                              rowBase;
    GT_U32                                              horzBlockRowIndex;
    GT_U32                                              lookupId;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT       rowColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC     *segPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC     segPattern;
    GT_BOOL                                             seeked;
    PRV_CPSS_AVL_TREE_PATH                              path;
    GT_VOID                                             *dbDataPtr;
    GT_U32                                              base, num, mask, idx, w0, w1, w2;

    if (blockIndex >= tcamSegCfgPtr->blocksAmount)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(
        rowsBitmapArr, 0,
        (((tcamSegCfgPtr->lookupRowAlignment + 31)
          / 32) * sizeof(GT_U32)));

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    rowBase =
        ((blockIndex / blocksInRow) * tcamSegCfgPtr->lookupRowAlignment);
    horzBlockRowIndex = (blockIndex % blocksInRow);
    lookupId = tcamSegCfgPtr->blockInfArr[blockIndex].lookupId;

    cpssOsMemSet(&segPattern, 0, sizeof(segPattern));
    segPattern.lookupId = lookupId;
    segPattern.rowsBase = rowBase;
    segPattern.segmentColumnsMap =
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        segPattern.segmentColumnsMap, horzBlockRowIndex,  freeColumns);

    for (seeked = prvCpssAvlPathFind(
            tcamSegCfgPtr->segFreeLuTree,
            PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
            &segPattern, path, &dbDataPtr);
        (seeked != GT_FALSE);
        seeked = prvCpssAvlPathSeek(
            tcamSegCfgPtr->segFreeLuTree,
            PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
            path, &dbDataPtr))
    {
        segPtr = dbDataPtr;
        if (segPtr->lookupId != lookupId) break;
        if (segPtr->rowsBase >=
            (rowBase + tcamSegCfgPtr->lookupRowAlignment)) break;
        rowColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
            segPtr->segmentColumnsMap, horzBlockRowIndex);
        if (rowColumns != freeColumns) continue;

        /* found free space as was specified */
        /* mark it by ones in bitmap         */
        base = (segPtr->rowsBase % tcamSegCfgPtr->lookupRowAlignment);
        num  = segPtr->rowsAmount;
        while (num)
        {
            w0   = (base % 32);
            w1   = (((w0 + num) >= 32) ? (32 - w0) : num);
            w2   = (32 - w0 - w1);
            mask = ((0xFFFFFFFFU >> (w0 + w2)) << w2);
            idx  = (base / 32);
            rowsBitmapArr[idx] |= mask;
            base += w1;
            num  -= w1;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceBlockBitmapsGet function
* @endinternal
*
* @brief   Get bitmaps of indexes of free blocks and mapped blocks to the given lookup.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup id
*
* @param[out] freeBlocksBmpArr[]       - (pointer to)bitmap array of indexes of free blocks
* @param[out] lookupBlocksBmpArr[]     - (pointer to)bitmap array of indexes of blocks mapped
*                                      to given lookup.
*                                      RETURN
*                                      None.
*                                      COMMENTS:
*/
static GT_VOID prvCpssDxChVirtualTcamDbSegSpaceBlockBitmapsGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                 lookupId,
    OUT    GT_U32                                 freeBlocksBmpArr[],
    OUT    GT_U32                                 lookupBlocksBmpArr[]
)
{
    GT_U32  blockIndex;
    GT_U32  bitmapMemoSize;

    bitmapMemoSize = (sizeof(GT_U32) * ((tcamSegCfgPtr->blocksAmount + 31) / 32));
    cpssOsMemSet(freeBlocksBmpArr, 0, bitmapMemoSize);
    cpssOsMemSet(lookupBlocksBmpArr, 0, bitmapMemoSize);

    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); blockIndex++)
    {
        if (tcamSegCfgPtr->blockInfArr[blockIndex].lookupId
                == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
        {
            freeBlocksBmpArr[blockIndex / 32] |= (1 << (blockIndex % 32));
            continue;
        }
        if (tcamSegCfgPtr->blockInfArr[blockIndex].lookupId == lookupId)
        {
            lookupBlocksBmpArr[blockIndex / 32] |= (1 << (blockIndex % 32));
            continue;
        }
    }
}

/* types of blocks mapped to the given lookup in block pairs */
typedef   enum
{
    PRV_OWN_BLOCK_TYPE_LEFT_OWN_OWN_E,
    PRV_OWN_BLOCK_TYPE_RIGHT_OWN_OWN_E,
    PRV_OWN_BLOCK_TYPE_LEFT_OWN_FREE_E,
    PRV_OWN_BLOCK_TYPE_RIGHT_FREE_OWN_E,
    PRV_OWN_BLOCK_TYPE_LEFT_OWN_OTHER_E,
    PRV_OWN_BLOCK_TYPE_RIGHT_OTHER_OWN_E,
    PRV_OWN_BLOCK_TYPE_LAST_E
} PRV_OWN_BLOCK_TYPE_ENT;

/* types of free blocks in block pairs               */
/* members of pairs where both are free not included */
typedef   enum
{
    PRV_FREE_BLOCK_TYPE_RIGHT_OWN_FREE_E,
    PRV_FREE_BLOCK_TYPE_LEFT_FREE_OWN_E,
    PRV_FREE_BLOCK_TYPE_ANY_FREE_WITH_OTHER_E,
    PRV_FREE_BLOCK_TYPE_LAST_E
} PRV_FREE_BLOCK_TYPE_ENT;

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceBloksInPairsStatusGet function
* @endinternal
*
* @brief   Get statistics about blocks in pairs.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup id
*
* @param[out] freeBlocksBmpArr[]       - (pointer to)bitmap array of indexes of free blocks
* @param[out] lookupBlocksBmpArr[]     - (pointer to)bitmap array of indexes of blocks mapped
*                                      to given lookup.
* @param[out] ffooooRowsNumArrPtr[]    - (pointer to) array of numbers of _FFOOOO_ cells per
*                                      own lookup mapped block type (PRV_OWN_BLOCK_TYPE_ENT).
* @param[out] ffffffRowsNumArrPtr[]    - (pointer to) array of numbers of _FFFFFF_ cells per
*                                      own lookup mapped block type (PRV_OWN_BLOCK_TYPE_ENT).
* @param[out] freeBlocksNumArrPtr[]    - (pointer to) array of numbers of free blocks per
*                                      free block type (PRV_FREE_BLOCK_TYPE_ENT).
* @param[out] freeBlockPairsNumPtr     - (pointer to) number of free blocks pairs.
* @param[out] ownBlockPairsNumPtr      - (pointer to) number of own-lookup mapped blocks pairs.
*                                       None.
*/
static GT_VOID prvCpssDxChVirtualTcamDbSegSpaceBloksInPairsStatusGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                lookupId,
    OUT    GT_U32                                freeBlocksBmpArr[],
    OUT    GT_U32                                lookupBlocksBmpArr[],
    OUT    GT_U32                                ffooooRowsNumArrPtr[],
    OUT    GT_U32                                ffffffRowsNumArrPtr[],
    OUT    GT_U32                                freeBlocksNumArrPtr[],
    OUT    GT_U32                                *freeBlockPairsNumPtr,
    OUT    GT_U32                                *ownBlockPairsNumPtr
)
{
    GT_U32 blockIndex, w, pairState;

    cpssOsMemSet(ffooooRowsNumArrPtr, 0, (PRV_OWN_BLOCK_TYPE_LAST_E * sizeof(GT_U32)));
    cpssOsMemSet(ffffffRowsNumArrPtr, 0, (PRV_OWN_BLOCK_TYPE_LAST_E * sizeof(GT_U32)));
    cpssOsMemSet(freeBlocksNumArrPtr, 0, (PRV_FREE_BLOCK_TYPE_LAST_E * sizeof(GT_U32)));
    *freeBlockPairsNumPtr = 0;
    *ownBlockPairsNumPtr  = 0;

    prvCpssDxChVirtualTcamDbSegSpaceBlockBitmapsGet(
        tcamSegCfgPtr, lookupId, freeBlocksBmpArr, lookupBlocksBmpArr);

    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); blockIndex +=2)
    {
        pairState = 0;
        w = ((lookupBlocksBmpArr[blockIndex / 32] >> (blockIndex % 32)) & 1);
        pairState |= w;
        w = ((freeBlocksBmpArr[blockIndex / 32] >> (blockIndex % 32)) & 1);
        pairState |= (w << 1);
        w = ((lookupBlocksBmpArr[(blockIndex + 1)/ 32] >> ((blockIndex + 1) % 32)) & 1);
        pairState |= (w << 2);
        w = ((freeBlocksBmpArr[(blockIndex + 1) / 32] >> ((blockIndex + 1) % 32)) & 1);
        pairState |= (w << 3);
        switch (pairState)
        {
            default: prvCpssDxChVirtualTcamDbSegmentTableException(); break;
            case 0: /* OTHER OTHER */
                break;
            case 1: /* OWN OTHER */
                ffooooRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_LEFT_OWN_OTHER_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E];
                ffffffRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_LEFT_OWN_OTHER_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E];
                break;
            case 2: /* FREE OTHER */
                freeBlocksNumArrPtr[PRV_FREE_BLOCK_TYPE_ANY_FREE_WITH_OTHER_E] ++;
                break;
            case 4: /* OTHER OWN */
                ffooooRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_RIGHT_OTHER_OWN_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex + 1].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E];
                ffffffRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_RIGHT_OTHER_OWN_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex + 1].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E];
                break;
            case 5: /* OWN OWN */
                ffooooRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_LEFT_OWN_OWN_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E];
                ffffffRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_LEFT_OWN_OWN_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E];
                ffooooRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_RIGHT_OWN_OWN_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex + 1].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E];
                ffffffRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_RIGHT_OWN_OWN_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex + 1].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E];
                (*ownBlockPairsNumPtr) ++;
                break;
            case 6: /* FREE OWN */
                ffooooRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_RIGHT_FREE_OWN_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex + 1].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E];
                ffffffRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_RIGHT_FREE_OWN_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex + 1].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E];
                freeBlocksNumArrPtr[PRV_FREE_BLOCK_TYPE_LEFT_FREE_OWN_E] ++;
                break;
            case 8: /* OTHER FREE */
                freeBlocksNumArrPtr[PRV_FREE_BLOCK_TYPE_ANY_FREE_WITH_OTHER_E] ++;
                break;
            case 9: /* OWN FREE */
                ffooooRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_LEFT_OWN_FREE_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E];
                ffffffRowsNumArrPtr[PRV_OWN_BLOCK_TYPE_LEFT_OWN_FREE_E]
                    += tcamSegCfgPtr->blockInfArr[blockIndex].freeSpace[
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E];
                freeBlocksNumArrPtr[PRV_FREE_BLOCK_TYPE_RIGHT_OWN_FREE_E] ++;
                break;
            case 10: /* FREE, FREE */
                (*freeBlockPairsNumPtr) ++;
                break;
        }
    }
}

/* 80-byte rules space making  (part2)                   */
/* "holes moving"                                        */
/* moving 60-byte (FFFFFF) and 20-byte (FFOOOO) holes    */
/* moving full horizontal block rows to new places       */
/* capable for 80-byte rules                             */

/*=========================================================================================*/

/* amount of words in bitmap of TCAM blocks - more then 1 to prevent compiler warnings */
#define PRV_WORDS_IN_BLOCK_BITMAP_CNS 2
/* amount of words in bitmap of TCAM block 256 / 32 */
#define PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS 8

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceNewLookupBlocksBitmapGet function
* @endinternal
*
* @brief   Get new bitmap of lookup blocks..
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] freeBlocksBmpArr[]       - (pointer to)bitmap array of indexes of free blocks
* @param[in] lookupBlocksBmpArr[]     - (pointer to)bitmap array of indexes of blocks mapped
*                                      to given lookup.
* @param[in] addedFreeBlockPairsToLookup - number of free block pairs added to lookup
* @param[in] addedFreeNeighborBlocksToLookup - number of free neighbor blocks added to to lookup
* @param[in] addedAnyBlocksForSpace   - number of any blocks added to to lookup (after counted upper)
*
* @param[out] usedBlocksBmpArr[]       - (pointer to)bitmap array of indexes of blocks that
*                                      will be mapped to lookup after moving _FFFFFF_ and _FFOOOO_.
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceNewLookupBlocksBitmapGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                     freeBlocksBmpArr[],
    IN     GT_U32                                     lookupBlocksBmpArr[],
    IN     GT_U32                                     addedFreeBlockPairsToLookup,
    IN     GT_U32                                     addedFreeNeighborBlocksToLookup,
    IN     GT_U32                                     addedAnyBlocksForSpace,
    OUT    GT_U32                                     usedBlocksBmpArr[]
)
{
    GT_U32    blockIndex;
    GT_U32    blockNeighbor;
    GT_U32    blocksNum;

    /* bit map of all blocks that will be mapped to own lookup */
    /* copy already mapped                                     */
    cpssOsMemCpy(
        usedBlocksBmpArr, lookupBlocksBmpArr,
        (sizeof(GT_U32) * PRV_WORDS_IN_BLOCK_BITMAP_CNS));

    /* add addedFreeNeighborBlocksToLookup blocks */
    blocksNum = 0;
    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); blockIndex++)
    {
        if (blocksNum >= addedFreeNeighborBlocksToLookup) break;
        blockNeighbor = (blockIndex ^ 1);
        if ((freeBlocksBmpArr[blockIndex / 32] & (1 << (blockIndex % 32))) == 0)
        {
            continue; /* not free */
        }
        if ((lookupBlocksBmpArr[blockNeighbor / 32] & (1 << (blockNeighbor % 32))) == 0)
        {
            continue; /* not mapped to own lookup */
        }
        usedBlocksBmpArr[blockIndex / 32] |= (1 << (blockIndex % 32));
        blocksNum ++;
    }
    if (blocksNum < addedFreeNeighborBlocksToLookup)
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* add addedFreeBlockPairsToLookup blocks  */
    blocksNum = 0;
    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); blockIndex += 2)
    {
        if (blocksNum >= addedFreeBlockPairsToLookup) break;
        blockNeighbor = blockIndex + 1;
        if ((freeBlocksBmpArr[blockIndex / 32] & (1 << (blockIndex % 32))) == 0)
        {
            continue; /* not free */
        }
        if ((freeBlocksBmpArr[blockNeighbor / 32] & (1 << (blockNeighbor % 32))) == 0)
        {
            continue; /* not free */
        }
        /* pair of blocks */
        usedBlocksBmpArr[blockIndex / 32] |= (3 << (blockIndex % 32));
        blocksNum ++;
    }
    if (blocksNum < addedFreeBlockPairsToLookup)
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* add addedAnyBlocksForSpace blocks */
    blocksNum = 0;
    /* add remained addedFreeNeighborBlocksToLookup as addedAnyBlocksForSpace */
    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); blockIndex++)
    {
        if (blocksNum >= addedAnyBlocksForSpace) break;
        if (usedBlocksBmpArr[blockIndex / 32] & (1 << (blockIndex % 32)))
        {
            continue; /* already added */
        }
        blockNeighbor = (blockIndex ^ 1);
        if ((freeBlocksBmpArr[blockIndex / 32] & (1 << (blockIndex % 32))) == 0)
        {
            continue; /* not free */
        }
        if ((lookupBlocksBmpArr[blockNeighbor / 32] & (1 << (blockNeighbor % 32))) == 0)
        {
            continue; /* not mapped to own lookup */
        }
        usedBlocksBmpArr[blockIndex / 32] |= (1 << (blockIndex % 32));
        blocksNum ++;
    }
    /* add remained addedFreeBlockPairsToLookup as addedAnyBlocksForSpace */
    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); blockIndex++)
    {
        if (blocksNum >= addedAnyBlocksForSpace) break;
        if (usedBlocksBmpArr[blockIndex / 32] & (1 << (blockIndex % 32)))
        {
            continue; /* already added */
        }
        blockNeighbor = (blockIndex ^ 1);
        if ((freeBlocksBmpArr[blockIndex / 32] & (1 << (blockIndex % 32))) == 0)
        {
            continue; /* not free */
        }
        if ((freeBlocksBmpArr[blockNeighbor / 32] & (1 << (blockNeighbor % 32))) == 0)
        {
            continue; /* not free */
        }
        usedBlocksBmpArr[blockIndex / 32] |= (1 << (blockIndex % 32));
        blocksNum ++;
    }
    /* add remained any free blocks as addedAnyBlocksForSpace */
    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); blockIndex++)
    {
        if (blocksNum >= addedAnyBlocksForSpace) break;
        if (usedBlocksBmpArr[blockIndex / 32] & (1 << (blockIndex % 32)))
        {
            continue; /* already added */
        }
        if ((freeBlocksBmpArr[blockIndex / 32] & (1 << (blockIndex % 32))) == 0)
        {
            continue; /* not free */
        }
        usedBlocksBmpArr[blockIndex / 32] |= (1 << (blockIndex % 32));
        blocksNum ++;
    }
    if (blocksNum < addedAnyBlocksForSpace)
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/* types of _FFFFFF_ and _FFOOOO_ spaces in blocks that mapped */
/* or will be mapped to lookup                                 */
typedef enum
{
    PRV_ROW_SPACE_SINGLE_FFFFFF_E,
    PRV_ROW_SPACE_SINGLE_FFOOOO_E,
    PRV_ROW_SPACE_FFFFFF_FFFFFF_E,
    PRV_ROW_SPACE_FFFFFF_FFOOOO_E,
    PRV_ROW_SPACE_FFOOOO_FFFFFF_E,
    PRV_ROW_SPACE_FFOOOO_FFOOOO_E,
    PRV_ROW_SPACE_FFFFFF_OTHER_E,
    PRV_ROW_SPACE_OTHER_FFFFFF_E,
    PRV_ROW_SPACE_FFOOOO_OTHER_E,
    PRV_ROW_SPACE_OTHER_FFOOOO_E,
    PRV_ROW_SPACE_OTHER_OTHER_E,
    PRV_ROW_SPACE_TYPE_LAST_E
} PRV_ROW_SPACE_TYPE_ENT;

/**
* @struct PRV_ROW_SPACE_REQUEST_ENTRY_STC
 *
 * @brief Array of such records used as request
 * of fillng source ant target space trees
 * for moving used space by dragging vTcams.
*/
typedef struct{

    /** type of block or block */
    PRV_ROW_SPACE_TYPE_ENT rowsType;

    /** @brief space from block0 or block1 of pair.
     *  1 - block0, 2 - block1, 3 - both
     */
    GT_U32 blockInPairBmp;

    /** @brief space tree priority
     *  (APPLICABLE RANGES: 0..15)
     *  Comments:
     */
    GT_U32 priority;

} PRV_ROW_SPACE_REQUEST_ENTRY_STC;


/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceLookupRowsCountByType function
* @endinternal
*
* @brief   Get new bitmap of lookup blocks..
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] usedBlocksBmpArr[]       - (pointer to)bitmap array of indexes of blocks that
*                                      will be mapped to lookup after moving _FFFFFF_ and _FFOOOO_.
* @param[in] freeBlocksBmpArr[]       - (pointer to)bitmap array of indexes of free blocks
*
* @param[out] rowSpaceCountArr[]       -  (pointer to)array of counters of counters of free spaces
*                                      of different types from PRV_ROW_SPACE_TYPE_ENT
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceLookupRowsCountByType
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                     usedBlocksBmpArr[],
    IN     GT_U32                                     freeBlocksBmpArr[],
    OUT    GT_U32                                     rowSpaceCountArr[]
)
{
    GT_STATUS     rc;
    GT_U32        rowIdx;
    GT_U32        blockIdx;
    GT_U32        block0Idx;
    GT_U32        block1Idx;
    GT_U32        block0Used;
    GT_U32        block1Used;
    GT_U32        block0Free;
    GT_U32        block1Free;
    GT_U32        count0_FFFFFF, count1_FFFFFF;
    GT_U32        count0_FFOOOO, count1_FFOOOO;
    GT_U32        count_FFFFFF_FFFFFF, count_FFFFFF_FFOOOO;
    GT_U32        count_FFOOOO_FFFFFF, count_FFOOOO_FFOOOO;
    GT_U32        count_OTHER_FFFFFF, count_OTHER_FFOOOO;
    GT_U32        count_FFFFFF_OTHER, count_FFOOOO_OTHER;
    static GT_U32 rowsBitmapArr0[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];
    static GT_U32 rowsBitmapArr1[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];
    static GT_U32 rowsBitmapArr0_1[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];
    static GT_U32 rowsBitmapArr1_1[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];
    static GT_U32 rowsBitmapArrRes[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];

    cpssOsMemSet(rowSpaceCountArr, 0, (sizeof(GT_U32) * PRV_ROW_SPACE_TYPE_LAST_E));

    if (tcamSegCfgPtr->blocksAmount > (32 * PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS))
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for (block0Idx = 0; (block0Idx < tcamSegCfgPtr->blocksAmount); block0Idx += 2)
    {
        block1Idx  = block0Idx + 1;
        block0Used = (usedBlocksBmpArr[block0Idx / 32] & (1 << (block0Idx % 32)));
        block1Used = (usedBlocksBmpArr[block1Idx / 32] & (1 << (block1Idx % 32)));
        /* both blocks not used */
        if ((block0Used | block1Used) == 0) continue;
        block0Free = (freeBlocksBmpArr[block0Idx / 32] & (1 << (block0Idx % 32)));
        block1Free = (freeBlocksBmpArr[block1Idx / 32] & (1 << (block1Idx % 32)));
        if (block0Used && block1Used && block0Free && block1Free)
        {
            /* both used and both free */
            rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFFFFF_E]
                += tcamSegCfgPtr->lookupRowAlignment;
            continue;
        }

        if ((block0Used == 0) || (block1Used == 0))
        {
            /* used one block only in pair */
            blockIdx = ((block1Used == 0) ? block0Idx : block1Idx);

            rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
                tcamSegCfgPtr, blockIdx,
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E,
                rowsBitmapArr0);
            if (rc != GT_OK) return rc;
            rowSpaceCountArr[PRV_ROW_SPACE_SINGLE_FFFFFF_E] +=
                (GT_U32)prvCpssBitmapOneBitsCount(
                    rowsBitmapArr0, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);

            rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
                tcamSegCfgPtr, blockIdx,
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E,
                rowsBitmapArr0);
            if (rc != GT_OK) return rc;
            rowSpaceCountArr[PRV_ROW_SPACE_SINGLE_FFOOOO_E] +=
                (GT_U32)prvCpssBitmapOneBitsCount(
                    rowsBitmapArr0, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);

            continue;
        }

        /* assumed both blocks used and at least one of them not free */
        if (block1Free)
        {
            /* block0 not free and  block1Free */
            blockIdx = block0Idx;
            rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
                tcamSegCfgPtr, blockIdx,
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E,
                rowsBitmapArr0);
            if (rc != GT_OK) return rc;
            count0_FFFFFF = (GT_U32)prvCpssBitmapOneBitsCount(
                rowsBitmapArr0, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);
            rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFFFFF_E] += count0_FFFFFF;

            rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
                tcamSegCfgPtr, blockIdx,
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E,
                rowsBitmapArr0_1);
            if (rc != GT_OK) return rc;
            count0_FFOOOO = (GT_U32)prvCpssBitmapOneBitsCount(
                rowsBitmapArr0_1, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);
            rowSpaceCountArr[PRV_ROW_SPACE_FFOOOO_FFFFFF_E] += count0_FFOOOO;

            rowSpaceCountArr[PRV_ROW_SPACE_OTHER_FFFFFF_E] +=
                (tcamSegCfgPtr->lookupRowAlignment
                 - count0_FFFFFF - count0_FFOOOO);

            continue;
        }

        if (block0Free)
        {
            /* block1 not free and  block0Free */
            blockIdx = block1Idx;
            rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
                tcamSegCfgPtr, blockIdx,
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E,
                rowsBitmapArr0);
            if (rc != GT_OK) return rc;
            count1_FFFFFF = (GT_U32)prvCpssBitmapOneBitsCount(
                rowsBitmapArr0, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);
            rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFFFFF_E] += count1_FFFFFF;

            rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
                tcamSegCfgPtr, blockIdx,
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E,
                rowsBitmapArr0_1);
            if (rc != GT_OK) return rc;
            rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFOOOO_E] +=
            count1_FFOOOO = (GT_U32)prvCpssBitmapOneBitsCount(
                rowsBitmapArr0_1, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);
            rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFOOOO_E] += count1_FFOOOO;

            rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_OTHER_E] +=
                (tcamSegCfgPtr->lookupRowAlignment
                 - count1_FFFFFF - count1_FFOOOO);

            continue;
        }

        /* both blocks not free */

        rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
            tcamSegCfgPtr, block0Idx,
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E,
            rowsBitmapArr0);
        if (rc != GT_OK) return rc;

        rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
            tcamSegCfgPtr, block1Idx,
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E,
            rowsBitmapArr1);
        if (rc != GT_OK) return rc;

        rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
            tcamSegCfgPtr, block0Idx,
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E,
            rowsBitmapArr0_1);
        if (rc != GT_OK) return rc;

        rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
            tcamSegCfgPtr, block1Idx,
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E,
            rowsBitmapArr1_1);
        if (rc != GT_OK) return rc;

        count0_FFFFFF = (GT_U32)prvCpssBitmapOneBitsCount(
            rowsBitmapArr0, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);

        count1_FFFFFF = (GT_U32)prvCpssBitmapOneBitsCount(
            rowsBitmapArr1, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);

        count0_FFOOOO = (GT_U32)prvCpssBitmapOneBitsCount(
            rowsBitmapArr0_1, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);

        count1_FFOOOO = (GT_U32)prvCpssBitmapOneBitsCount(
            rowsBitmapArr1_1, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);

        for (rowIdx = 0; (rowIdx < PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS); rowIdx++)
        {
            rowsBitmapArrRes[rowIdx] = (rowsBitmapArr0[rowIdx] & rowsBitmapArr1[rowIdx]);
        }

        count_FFFFFF_FFFFFF = (GT_U32)prvCpssBitmapOneBitsCount(
            rowsBitmapArrRes, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);
        rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFFFFF_E] += count_FFFFFF_FFFFFF;

        for (rowIdx = 0; (rowIdx < PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS); rowIdx++)
        {
            rowsBitmapArrRes[rowIdx] = (rowsBitmapArr0[rowIdx] & rowsBitmapArr1_1[rowIdx]);
        }
        count_FFFFFF_FFOOOO = (GT_U32)prvCpssBitmapOneBitsCount(
            rowsBitmapArrRes, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);
        rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFOOOO_E] += count_FFFFFF_FFOOOO;

        count_FFFFFF_OTHER =
            (count0_FFFFFF - count_FFFFFF_FFFFFF - count_FFFFFF_FFOOOO);
        rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_OTHER_E] += count_FFFFFF_OTHER;

        for (rowIdx = 0; (rowIdx < PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS); rowIdx++)
        {
            rowsBitmapArrRes[rowIdx] = (rowsBitmapArr0_1[rowIdx] & rowsBitmapArr1[rowIdx]);
        }
        count_FFOOOO_FFFFFF = (GT_U32)prvCpssBitmapOneBitsCount(
            rowsBitmapArrRes, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);
        rowSpaceCountArr[PRV_ROW_SPACE_FFOOOO_FFFFFF_E] += count_FFOOOO_FFFFFF;

        count_OTHER_FFFFFF =
            (count1_FFFFFF - count_FFFFFF_FFFFFF - count_FFOOOO_FFFFFF);
        rowSpaceCountArr[PRV_ROW_SPACE_OTHER_FFFFFF_E] += count_OTHER_FFFFFF;

        for (rowIdx = 0; (rowIdx < PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS); rowIdx++)
        {
            rowsBitmapArrRes[rowIdx] = (rowsBitmapArr0_1[rowIdx] & rowsBitmapArr1_1[rowIdx]);
        }
        count_FFOOOO_FFOOOO = (GT_U32)prvCpssBitmapOneBitsCount(
            rowsBitmapArrRes, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);
        rowSpaceCountArr[PRV_ROW_SPACE_FFOOOO_FFOOOO_E] += count_FFOOOO_FFOOOO;

        count_OTHER_FFOOOO =
            (count1_FFOOOO - count_FFFFFF_FFOOOO - count_FFOOOO_FFOOOO);
        rowSpaceCountArr[PRV_ROW_SPACE_OTHER_FFOOOO_E] += count_OTHER_FFOOOO;

        count_FFOOOO_OTHER =
            (count0_FFOOOO - count_FFOOOO_FFFFFF - count_FFOOOO_FFOOOO);
        rowSpaceCountArr[PRV_ROW_SPACE_FFOOOO_OTHER_E] += count_FFOOOO_OTHER;

        rowSpaceCountArr[PRV_ROW_SPACE_OTHER_OTHER_E] +=
            (tcamSegCfgPtr->lookupRowAlignment
             - count_FFFFFF_FFFFFF
             - count_FFFFFF_FFOOOO
             - count_FFFFFF_OTHER
             - count_FFOOOO_FFFFFF
             - count_FFOOOO_FFOOOO
             - count_FFOOOO_OTHER
             - count_OTHER_FFFFFF
             - count_OTHER_FFOOOO);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceBlockFreeRowsBitmapGet function
* @endinternal
*
* @brief   Get bitmap of block or block-pair rows with free space of required type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] blockIndex               - (index of block or first block in pair.
* @param[in] rowsType                 - type of block or block-pair rows with free space.
*
* @param[out] rowBitmapArr[]           - (pointer to)array of PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS
*                                      words for required rows bitmap.
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceBlockFreeRowsBitmapGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC  *tcamSegCfgPtr,
    IN     GT_U32                                     blockIndex,
    IN     PRV_ROW_SPACE_TYPE_ENT                     rowsType,
    OUT    GT_U32                                     rowBitmapArr[]
)
{
    GT_U32 rowBitmapWorkArr[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];
    GT_STATUS     rc;
    GT_U32        rowIdx, blockIdx0, blockIdx1;
    GT_U32        blockIdxArr[2],i,j;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumnsArr[2];

    if (tcamSegCfgPtr->blocksAmount > (32 * PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS))
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(rowBitmapArr, 0, (sizeof(GT_U32) * PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS));

    /* single block cases */
    segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
    switch (rowsType)
    {
        case PRV_ROW_SPACE_SINGLE_FFFFFF_E:
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
            break;
        case PRV_ROW_SPACE_SINGLE_FFOOOO_E:
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E;
            break;
        default: break;
    }
    if (segmentColumns != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
    {
        return prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
            tcamSegCfgPtr, blockIndex, segmentColumns, rowBitmapArr);
    }

    /* block pair check */
    if (blockIndex & 1)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* cases of block pair without _OTHER_ */
    segmentColumnsArr[0] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
    segmentColumnsArr[1] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
    switch (rowsType)
    {
        case PRV_ROW_SPACE_FFFFFF_FFFFFF_E:
            segmentColumnsArr[0] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
            segmentColumnsArr[1] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
            break;
        case PRV_ROW_SPACE_FFFFFF_FFOOOO_E:
            segmentColumnsArr[0] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
            segmentColumnsArr[1] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E;
            break;
        case PRV_ROW_SPACE_FFOOOO_FFFFFF_E:
            segmentColumnsArr[0] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E;
            segmentColumnsArr[1] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
            break;
        case PRV_ROW_SPACE_FFOOOO_FFOOOO_E:
            segmentColumnsArr[0] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E;
            segmentColumnsArr[1] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E;
            break;
        default: break;
    }
    if ((segmentColumnsArr[0] != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
        && (segmentColumnsArr[1] != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E))
    {
        rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
            tcamSegCfgPtr, blockIndex, segmentColumnsArr[0], rowBitmapArr);
        if (rc != GT_OK) return rc;
        rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
            tcamSegCfgPtr, (blockIndex + 1), segmentColumnsArr[1], rowBitmapWorkArr);
        if (rc != GT_OK) return rc;
        for (rowIdx = 0; (rowIdx < PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS); rowIdx++)
        {
            rowBitmapArr[rowIdx] &= rowBitmapWorkArr[rowIdx];
        }
        return GT_OK;
    }

    /* cases of block pair with _OTHER_ */
    segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
    switch (rowsType)
    {
        case PRV_ROW_SPACE_FFFFFF_OTHER_E:
            blockIdx0 = blockIndex;
            blockIdx1 = blockIndex + 1;
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
            break;
        case PRV_ROW_SPACE_OTHER_FFFFFF_E:
            blockIdx0 = blockIndex + 1;
            blockIdx1 = blockIndex;
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
            break;
        case PRV_ROW_SPACE_FFOOOO_OTHER_E:
            blockIdx0 = blockIndex;
            blockIdx1 = blockIndex + 1;
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E;
            break;
        case PRV_ROW_SPACE_OTHER_FFOOOO_E:
            blockIdx0 = blockIndex + 1;
            blockIdx1 = blockIndex;
            segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E;
            break;
        default: break;
    }
    if (segmentColumns != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
    {
        rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
            tcamSegCfgPtr, blockIdx0, segmentColumns, rowBitmapArr);
        if (rc != GT_OK) return rc;
        segmentColumnsArr[0] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
        segmentColumnsArr[1] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E;
        for (i = 0; (i < 2); i++)
        {
            rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
                tcamSegCfgPtr, blockIdx1, segmentColumnsArr[i], rowBitmapWorkArr);
            if (rc != GT_OK) return rc;
            for (rowIdx = 0; (rowIdx < PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS); rowIdx++)
            {
                rowBitmapArr[rowIdx] &= (~ rowBitmapWorkArr[rowIdx]);
            }
        }
        return GT_OK;
    }

    if (rowsType == PRV_ROW_SPACE_OTHER_OTHER_E)
    {
        blockIdxArr[0] = blockIndex;
        blockIdxArr[1] = blockIndex + 1;
        segmentColumnsArr[0] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E;
        segmentColumnsArr[1] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E;
        cpssOsMemSet(rowBitmapArr, 0xFF, (sizeof(GT_U32) * PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS));
        for (i = 0; (i < 2); i++)
        {
            for (j = 0; (j < 2); j++)
            {
                rc = prvCpssDxChVirtualTcamDbSegSpaceFreeCellsInBlockBitmapGet(
                    tcamSegCfgPtr, blockIdxArr[j], segmentColumnsArr[i], rowBitmapWorkArr);
                if (rc != GT_OK) return rc;
                for (rowIdx = 0; (rowIdx < PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS); rowIdx++)
                {
                    rowBitmapArr[rowIdx] &= (~ rowBitmapWorkArr[rowIdx]);
                }
            }
        }
        return GT_OK;
    }

    /* not supported cases */
    prvCpssDxChVirtualTcamDbSegmentTableException();
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceBlockBypassedRowsBitmapGet function
* @endinternal
*
* @brief   Get bitmap of block or block-pair rows with free space of required type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] blockRowsBase            - block Rows Base.
* @param[in] blockRowsSize            - amount of rows in block
* @param[in] horzBlockIndexBmp        - horizontal BlockIndex bitmap
* @param[in] bypassedRowsTree         - tree of add bypassed rows ranges
*
* @param[out] rowBitmapArr[]           - (pointer to)array of PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS
*                                      words for required rows bitmap.
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbSegSpaceBlockBypassedRowsBitmapGet
(
    IN     GT_U32                                     blockRowsBase,
    IN     GT_U32                                     blockRowsSize,
    IN     GT_U32                                     horzBlockIndexBmp,
    IN     PRV_CPSS_AVL_TREE_ID                       bypassedRowsTree,
    OUT    GT_U32                                     rowBitmapArr[]
)
{
    GT_U32    horzBlockIdx;
    GT_U32    low, rangeHigh, high, lowBit, numOfBits;
    GT_U32    blockRowHigh;
    GT_U32    foundHorzBlockIndex, foundRowsBase, foundRowsAmount;
    GT_VOID   *treeKeyPtr;
    GT_VOID   *treeDataPtr;
    PRV_CPSS_AVL_TREE_PATH     path;
    GT_BOOL   found;

    blockRowHigh = blockRowsBase + blockRowsSize;
    cpssOsMemSet(rowBitmapArr, 0, (sizeof(GT_U32) * PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS));

    for (horzBlockIdx = 0; (horzBlockIdx < 32); horzBlockIdx++)
    {
        if ((horzBlockIndexBmp & (1 << horzBlockIdx)) == 0) continue;

        treeKeyPtr = PRV_HORZ_BLOCK_ROWS_RANGE3_TO_VOIDPTR_MAC(
            horzBlockIdx, blockRowsBase, 0 /*rowsSize*/);

        if (GT_FALSE != prvCpssAvlPathFind(
            bypassedRowsTree, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
            treeKeyPtr, path, &treeDataPtr))
        {
            PRV_HORZ_BLOCK_ROWS_RANGE3_FROM_VOIDPTR_MAC(
                treeDataPtr,  foundHorzBlockIndex, foundRowsBase, foundRowsAmount);
            rangeHigh = foundRowsBase + foundRowsAmount;
            if (foundHorzBlockIndex != horzBlockIdx) continue;
            if (foundRowsBase >= blockRowHigh) continue;
            if (rangeHigh > blockRowsBase)
            {
                low  = (foundRowsBase > blockRowsBase) ? foundRowsBase : blockRowsBase;
                high = (rangeHigh < blockRowHigh) ? rangeHigh : blockRowHigh;
                if (low >= high)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    return;
                }
                lowBit = (low - blockRowsBase);
                numOfBits = (high - low);
                prvCpssBitmapRangeSet(
                    rowBitmapArr, lowBit, numOfBits, 1);
            }
        }

        for (found = prvCpssAvlPathFind(
                bypassedRowsTree, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
                treeKeyPtr, path, &treeDataPtr);
              (found != GT_FALSE);
              found = prvCpssAvlPathSeek(
                bypassedRowsTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &treeDataPtr))
        {
            PRV_HORZ_BLOCK_ROWS_RANGE3_FROM_VOIDPTR_MAC(
                treeDataPtr,  foundHorzBlockIndex, foundRowsBase, foundRowsAmount);
            rangeHigh = foundRowsBase + foundRowsAmount;
            if (foundHorzBlockIndex != horzBlockIdx) break;
            if (foundRowsBase >= blockRowHigh) break;
            if (rangeHigh > blockRowsBase)
            {
                low  = (foundRowsBase > blockRowsBase) ? foundRowsBase : blockRowsBase;
                high = (rangeHigh < blockRowHigh) ? rangeHigh : blockRowHigh;
                if (low >= high)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    return;
                }
                lowBit = (low - blockRowsBase);
                numOfBits = (high - low);
                prvCpssBitmapRangeSet(
                    rowBitmapArr, lowBit, numOfBits, 1);
            }
        }
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceAddBlockRowsToTree function
* @endinternal
*
* @brief   Add to tree the given number of block rows with free space of the given type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] usedBlocksBmpArr[]       - (pointer to)bitmap array of indexes of blocks that
*                                      will be mapped to lookup after moving _FFFFFF_ and _FFOOOO_.
* @param[in] rowsType                 - type of block or block-pair rows with free space.
* @param[in] blockInPairBmp           - 1 - space from left block, 2 - space from right block,
*                                      3 - space from both blocks
*                                      Relevant only when rowsType specifies pair of blocks
* @param[in] rowsAmount               - amount of rows to add, if blockInPairBmp==3 it is the summary
*                                      of rows amounts placed in both blocks of pair.
* @param[in] priority                 -  in the tree
* @param[in] spaceTree                - tree to add found rows ranges
* @param[in] bypassedRowsTree         - tree of add bypassed rows ranges
*                                      If specified NULL - no bypassed rows ranges.
*
* @param[out] addedRowsPtr             - (pointer to)amount of added rows.
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceAddBlockRowsToTree
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                     usedBlocksBmpArr[],
    IN     PRV_ROW_SPACE_TYPE_ENT                     rowsType,
    IN     GT_U32                                     blockInPairBmp,
    IN     GT_U32                                     rowsAmount,
    IN     GT_U32                                     priority,
    IN     PRV_CPSS_AVL_TREE_ID                       spaceTree,
    IN     PRV_CPSS_AVL_TREE_ID                       bypassedRowsTree,
    OUT    GT_U32                                     *addedRowsPtr
)
{
    GT_STATUS rc;
    GT_U32    blockIndex, blockIndexInc, blockIndexMsk, blockPairIndex;
    GT_U32    bypassHorsBlockBmp;
    GT_U32    rowsRemainder, rangeSize;
    GT_32     startIdx, endIdx;
    GT_VOID   *treeDataPtr;
    GT_U32    rowBitmapArr[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];
    GT_U32    rowBypassBitmapArr[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];
    GT_U32    fixBlockIndex, fixBlockIndexMask;
    GT_U32    horzBlockIdx, rowsBase;
    GT_U32    blocksInRow, blockRowsBase;
    GT_U32    rangeSizeArr[2];
    GT_U32    rowIdx;

    blocksInRow =
        (tcamSegCfgPtr->tcamColumnsAmount
         / tcamSegCfgPtr->lookupColumnAlignment);

    *addedRowsPtr = 0;
    switch (rowsType)
    {
        case PRV_ROW_SPACE_SINGLE_FFFFFF_E:
        case PRV_ROW_SPACE_SINGLE_FFOOOO_E:
            blockIndexInc = 1;
            bypassHorsBlockBmp = 1;
            break;
        default:
            blockIndexInc = 2;
            bypassHorsBlockBmp = 3;
            if (((blockInPairBmp & 3) == 0) || ((blockInPairBmp & (~3)) != 0))
            {
                /* valid 1,2,3 only */
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            break;
    }

    rowsRemainder  = rowsAmount;
    if (rowsRemainder == 0) return GT_OK;

    for (blockIndex = 0; (blockIndex < tcamSegCfgPtr->blocksAmount); blockIndex += blockIndexInc)
    {
        blockPairIndex = (blockIndex & (~ 1));
        blockIndexMsk = ((usedBlocksBmpArr[blockPairIndex / 32] >> (blockPairIndex % 32)) & 3);
        switch (rowsType)
        {
            /* single blocks */
            case PRV_ROW_SPACE_SINGLE_FFFFFF_E:
            case PRV_ROW_SPACE_SINGLE_FFOOOO_E:
                if ((blockIndexMsk != 1) && (blockIndexMsk != 2)) continue;
                fixBlockIndexMask = 1;
                break;
            default:
                if (blockIndexMsk != 3) continue; /* block pair */
                /* for block pairs */
                fixBlockIndexMask = blockInPairBmp;
                break;
        }


        blockRowsBase =
            ((blockIndex / blocksInRow) * tcamSegCfgPtr->lookupRowAlignment);

        rc = prvCpssDxChVirtualTcamDbSegSpaceBlockFreeRowsBitmapGet(
            tcamSegCfgPtr, blockIndex, rowsType, rowBitmapArr);
        if (rc != GT_OK) return rc;

        if (bypassedRowsTree)
        {
            prvCpssDxChVirtualTcamDbSegSpaceBlockBypassedRowsBitmapGet(
                blockRowsBase, tcamSegCfgPtr->lookupRowAlignment,
                (bypassHorsBlockBmp << (blockIndex % blocksInRow)) /*horzBlockIndexBmp*/,
                bypassedRowsTree, rowBypassBitmapArr);
            for (rowIdx = 0; (rowIdx < PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS); rowIdx++)
            {
                rowBitmapArr[rowIdx] &= (~ rowBypassBitmapArr[rowIdx]);
            }
        }

        startIdx = -1;
        while (1)
        {
            startIdx = prvCpssBitmapNextOneBitIndexFind(
                rowBitmapArr, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS,
                startIdx);
            if (startIdx == -1) break;
            endIdx = prvCpssBitmapNextZeroBitIndexFind(
                rowBitmapArr, PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS,
                startIdx);
            if (endIdx == -1)
            {
                /* bit after bitmap */
                endIdx = (32 * PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS);
            }
            rangeSize = (GT_U32)(endIdx - startIdx);
            if (rangeSize == 0)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            rangeSizeArr[0] = rangeSize;
            if (fixBlockIndexMask != 3)
            {
                /* using one only block of pair    */
                /* only one of values will be used */
                if (rangeSizeArr[0] > rowsRemainder)
                {
                    rangeSizeArr[0] = rowsRemainder;
                }
                rangeSizeArr[1] = rangeSizeArr[0];
            }
            else
            {
                /* using both blocks of pair */
                if ((rangeSizeArr[0] * 2) <= rowsRemainder)
                {
                    rangeSizeArr[1] = rangeSizeArr[0];
                }
                else
                {
                    rangeSizeArr[1] = (rowsRemainder / 2);
                    rangeSizeArr[0] = (rowsRemainder - rangeSizeArr[1]);
                }

            }
            rowsBase = (blockRowsBase + startIdx);
            for (fixBlockIndex = 0; (fixBlockIndex < 2); fixBlockIndex++)
            {
                if (((1 << fixBlockIndex) & fixBlockIndexMask) == 0) continue;

                horzBlockIdx = ((blockIndex % blocksInRow) + fixBlockIndex);
                treeDataPtr = PRV_HORZ_BLOCK_ROWS_RANGE4_TO_VOIDPTR_MAC(
                    priority, horzBlockIdx, rowsBase, rangeSizeArr[fixBlockIndex]);
                rc = prvCpssAvlItemInsert(spaceTree, treeDataPtr);
                if (rc != GT_OK)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    return rc;
                }
                rowsRemainder -= rangeSizeArr[fixBlockIndex];
                if (rowsRemainder == 0) break;
            }
            if (rowsRemainder == 0) break;
        }
        if (rowsRemainder == 0) break;
    }

    *addedRowsPtr = (rowsAmount - rowsRemainder);
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceCreateBlockRowsTree function
* @endinternal
*
* @brief   Create space tree and fill it by the given number of block rows
*         with free spaces of the given types.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] usedBlocksBmpArr[]       - (pointer to)bitmap array of indexes of blocks that
*                                      will be mapped to lookup after moving _FFFFFF_ and _FFOOOO_.
* @param[in] isTargetTree             - GT_TRUE - target tree with priority
*                                      GT_FALSE - source tree
* @param[in] rowsAmount               - amount of rows to add
* @param[in] spaceRequestArr[]        - amount of entries in space request array
* @param[in] spaceRequestArr[]        - (pointer to)space request array contains
*                                      entries with {rowsType, blockInPair, priority}
* @param[in] bypassedRowsTree         - tree of add bypassed rows ranges
*                                      If specified NULL - no bypassed rows ranges.
*
* @param[out] spaceTreePoolPtr         - (pointer to)Id of new Pool for tree nodes.
*                                      addedRowsPtr          - (pointer to)new Tree Id.
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceCreateBlockRowsTree
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC  *tcamSegCfgPtr,
    IN     GT_U32                                     usedBlocksBmpArr[],
    IN     GT_BOOL                                    isTargetTree,
    IN     GT_U32                                     rowsAmount,
    IN     GT_U32                                     spaceRequestArrSize,
    IN     PRV_ROW_SPACE_REQUEST_ENTRY_STC            spaceRequestArr[],
    IN     PRV_CPSS_AVL_TREE_ID                       bypassedRowsTree,
    OUT    CPSS_BM_POOL_ID                            *spaceTreePoolPtr,
    OUT    PRV_CPSS_AVL_TREE_ID                       *spaceTreePtr
)
{
    GT_STATUS               rc;
    GT_U32                  addedRows;
    CPSS_BM_POOL_ID         poolId;
    PRV_CPSS_AVL_TREE_ID    spaceTree;
    GT_U32                  spaceRequestIndex;
    GT_U32                  remaiderRowsAmount;

    rc = prvCpssAvlMemPoolCreate(
        rowsAmount/*maxNode*/,&poolId);
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        return rc;
    }

    if (isTargetTree == GT_FALSE)
    {
        rc = prvCpssAvlTreeCreate(
            (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegSpaceWorkDbSourceCompareFunc,
            poolId,
            &spaceTree);
    }
    else
    {
        rc = prvCpssAvlTreeCreate(
            (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegSpaceWorkDbTargetCompareFunc,
            poolId,
            &spaceTree);
    }
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        prvCpssAvlMemPoolDelete(poolId);
        return rc;
    }

    remaiderRowsAmount = rowsAmount;

    for (spaceRequestIndex = 0; (spaceRequestIndex < spaceRequestArrSize); spaceRequestIndex++)
    {
        rc = prvCpssDxChVirtualTcamDbSegSpaceAddBlockRowsToTree(
            tcamSegCfgPtr, usedBlocksBmpArr,
            spaceRequestArr[spaceRequestIndex].rowsType,
            spaceRequestArr[spaceRequestIndex].blockInPairBmp,
            remaiderRowsAmount,
            spaceRequestArr[spaceRequestIndex].priority,
            spaceTree,
            bypassedRowsTree, &addedRows);
        if (rc != GT_OK)
        {
            prvCpssAvlTreeDelete(
                spaceTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
            prvCpssAvlMemPoolDelete(poolId);
            return rc;
        }

        if (remaiderRowsAmount >= addedRows)
            {remaiderRowsAmount -= addedRows;}
        else
            {remaiderRowsAmount = 0;}
        if (remaiderRowsAmount == 0) break;
    }

    if (remaiderRowsAmount > 0)
    {
        /* failed: not found enough of space */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        prvCpssAvlTreeDelete(
            spaceTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        prvCpssAvlMemPoolDelete(poolId);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    *spaceTreePoolPtr = poolId;
    *spaceTreePtr     = spaceTree;
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceMoveBlockRows function
* @endinternal
*
* @brief   Move given number of block rows using given source and target
*         types of block rows.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] usedBlocksBmpArr[]       - (pointer to)bitmap array of indexes of blocks that
*                                      will be mapped to lookup after moving _FFFFFF_ and _FFOOOO_.
* @param[in] bypassSourceRows         - GT_TRUE - bypass Source space building Target space
*                                      GT_FALSE - use any space
* @param[in] lookupId                 - lookup Id. (needed for mapping used blocks thay yet free).
* @param[in] rowsAmount               - amount of rows to add
* @param[in] sourceSpaceRequestArr[]  - amount of entries in space request array
*                                      spaceRequestArr            - (pointer to)space request array contains
*                                      entries with {rowsType, blockInPair, priority}
* @param[in] targetSpaceRequestArr[]  - amount of entries in space request array
* @param[in] targetSpaceRequestArr[]  - (pointer to)space request array contains
*                                      entries with {rowsType, blockInPair, priority}
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceMoveBlockRows
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                     usedBlocksBmpArr[],
    IN     GT_BOOL                                    bypassSourceRows,
    IN     GT_U32                                     lookupId,
    IN     GT_U32                                     rowsAmount,
    IN     GT_U32                                     sourceSpaceRequestArrSize,
    IN     PRV_ROW_SPACE_REQUEST_ENTRY_STC            sourceSpaceRequestArr[],
    IN     GT_U32                                     targetSpaceRequestArrSize,
    IN     PRV_ROW_SPACE_REQUEST_ENTRY_STC            targetSpaceRequestArr[]
)
{
    GT_STATUS                   rc;
    CPSS_BM_POOL_ID             sourceSpaceTreePool;
    PRV_CPSS_AVL_TREE_ID        sourceSpaceTree;
    CPSS_BM_POOL_ID             targetSpaceTreePool;
    PRV_CPSS_AVL_TREE_ID        targetSpaceTree;
    PRV_CPSS_AVL_TREE_ID        rowsBypassTree;

    rc = prvCpssDxChVirtualTcamDbSegSpaceCreateBlockRowsTree(
        tcamSegCfgPtr, usedBlocksBmpArr, GT_FALSE/*isTargetTree*/, rowsAmount,
        sourceSpaceRequestArrSize, sourceSpaceRequestArr,
        NULL /*bypassedRowsTree*/,
        &sourceSpaceTreePool, &sourceSpaceTree);
    if (rc != GT_OK) return rc;

    rowsBypassTree = (bypassSourceRows == GT_FALSE) ? NULL : sourceSpaceTree;

    rc = prvCpssDxChVirtualTcamDbSegSpaceCreateBlockRowsTree(
        tcamSegCfgPtr, usedBlocksBmpArr, GT_TRUE/*isTargetTree*/, rowsAmount,
        targetSpaceRequestArrSize, targetSpaceRequestArr,
        rowsBypassTree /*bypassedRowsTree*/,
        &targetSpaceTreePool, &targetSpaceTree);
    if (rc != GT_OK)
    {
        prvCpssAvlTreeDelete(
            sourceSpaceTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
        prvCpssAvlMemPoolDelete(sourceSpaceTreePool);
        return rc;
    }

    rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherSpaceTree(
        tcamSegCfgPtr, lookupId,
        sourceSpaceTree, targetSpaceTree);

    prvCpssAvlTreeDelete(
        sourceSpaceTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
    prvCpssAvlMemPoolDelete(sourceSpaceTreePool);
    prvCpssAvlTreeDelete(
        targetSpaceTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
    prvCpssAvlMemPoolDelete(targetSpaceTreePool);
    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceFreePlaceForRightFfxxxx function
* @endinternal
*
* @brief   Free place for move to it _FFOOOO_ to the right block of a pair.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] usedBlocksBmpArr[]       - (pointer to)bitmap array of indexes of blocks that
*                                      will be mapped to lookup after moving _FFFFFF_ and _FFOOOO_.
* @param[in] freeBlocksBmpArr[]       - (pointer to)bitmap array of indexes of free blocks
* @param[in,out] rowSpaceCountArr[]       - (pointer to)array of counters of counters of free spaces
*                                      of different types from PRV_ROW_SPACE_TYPE_ENT
* @param[in,out] rowSpaceCountArr[]       - (pointer to)array of counters of counters of free spaces
*                                      of different types from PRV_ROW_SPACE_TYPE_ENT
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceFreePlaceForRightFfxxxx
(
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN       GT_U32                                                     usedBlocksBmpArr[],
    IN       GT_U32                                                     freeBlocksBmpArr[],
    INOUT    GT_U32                                                     rowSpaceCountArr[]
)
{
    GT_STATUS   rc;
    GT_U32      rows80BytesRemainder;
    GT_U32      ready_FFFFFF_FFOOOO;
    GT_U32      ready_right_FFOOOO;
    GT_U32      ready_right_FFFFFF;
    GT_U32      ready_left_FFFFFF;
    GT_U32      ready_single_FFFFFF;
    GT_U32      needed_right_FFFFFF;
    GT_U32      needed_right_FFOOOO;
    /* rowsType, blockInPairBmp, priority - for target only  */
    static PRV_ROW_SPACE_REQUEST_ENTRY_STC   sourceSpaceRequestArr[] =
    {
        {PRV_ROW_SPACE_FFFFFF_OTHER_E, 2, 0},
        {PRV_ROW_SPACE_FFOOOO_OTHER_E, 2, 0},
        {PRV_ROW_SPACE_OTHER_OTHER_E,  2, 0}
    };
    static GT_U32                            sourceSpaceRequestArrSize =
        (sizeof(sourceSpaceRequestArr) / sizeof(sourceSpaceRequestArr[0]));
    static PRV_ROW_SPACE_REQUEST_ENTRY_STC   targetSpaceRequestArr[] =
    {
        {PRV_ROW_SPACE_SINGLE_FFFFFF_E, 0, 0},
        {PRV_ROW_SPACE_FFFFFF_OTHER_E,  1, 1},
        {PRV_ROW_SPACE_FFFFFF_FFFFFF_E, 1, 3}
    };
    static GT_U32                            targetSpaceRequestArrSize =
        (sizeof(targetSpaceRequestArr) / sizeof(targetSpaceRequestArr[0]));

    ready_FFFFFF_FFOOOO = rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFOOOO_E];
    /* assumed allocRequestPtr->rulesAmount > ready_FFFFFF_FFOOOO */
    rows80BytesRemainder = (allocRequestPtr->rulesAmount - ready_FFFFFF_FFOOOO);

    ready_right_FFOOOO =
        rowSpaceCountArr[PRV_ROW_SPACE_FFOOOO_FFOOOO_E] +
        rowSpaceCountArr[PRV_ROW_SPACE_OTHER_FFOOOO_E ];
    ready_right_FFFFFF =
        rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFFFFF_E] +
        rowSpaceCountArr[PRV_ROW_SPACE_OTHER_FFFFFF_E ] +
        rowSpaceCountArr[PRV_ROW_SPACE_FFOOOO_FFFFFF_E];
    ready_left_FFFFFF =
        rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFFFFF_E] +
        rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_OTHER_E ];
    ready_single_FFFFFF =
        rowSpaceCountArr[PRV_ROW_SPACE_SINGLE_FFFFFF_E];

    needed_right_FFOOOO =
        prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleAdditionalFfxxxxNumGet(
            (ready_right_FFFFFF + ready_left_FFFFFF + ready_single_FFFFFF),
            ready_right_FFOOOO,
            rows80BytesRemainder);
    if (needed_right_FFOOOO == 0xFFFFFFFF)
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    if (needed_right_FFOOOO <= ready_right_FFFFFF)
    {
        /* found enough place for moving _FFOOOO_ to right column */
        return GT_OK;
    }

    /* free space for _FFOOOO_ by swapping      */
    /* right_OTHER_ and single or left _FFFFFF_ */
    /* move source priority:                    */
    /* 0 - PRV_ROW_SPACE_FFFFFF_OTHER_E         */

    needed_right_FFFFFF = (needed_right_FFOOOO - ready_right_FFFFFF);

    rc = prvCpssDxChVirtualTcamDbSegSpaceMoveBlockRows(
        tcamSegCfgPtr, usedBlocksBmpArr, GT_FALSE /*bypassSourceRows*/,
        allocRequestPtr->lookupId, needed_right_FFFFFF,
        sourceSpaceRequestArrSize, sourceSpaceRequestArr,
        targetSpaceRequestArrSize, targetSpaceRequestArr);
    if (rc != GT_OK) return rc;

    /* recalculate counters */
    rc =  prvCpssDxChVirtualTcamDbSegSpaceLookupRowsCountByType(
        tcamSegCfgPtr, usedBlocksBmpArr, freeBlocksBmpArr, rowSpaceCountArr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceMoveFfxxxxToRight function
* @endinternal
*
* @brief   Move _FFOOOO_ to the right block of a pair.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] usedBlocksBmpArr[]       - (pointer to)bitmap array of indexes of blocks that
*                                      will be mapped to lookup after moving _FFFFFF_ and _FFOOOO_.
* @param[in] freeBlocksBmpArr[]       - (pointer to)bitmap array of indexes of free blocks
* @param[in,out] rowSpaceCountArr[]       - (pointer to)array of counters of counters of free spaces
*                                      of different types from PRV_ROW_SPACE_TYPE_ENT
* @param[in,out] rowSpaceCountArr[]       - (pointer to)array of counters of counters of free spaces
*                                      of different types from PRV_ROW_SPACE_TYPE_ENT
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceMoveFfxxxxToRight
(
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN       GT_U32                                                     usedBlocksBmpArr[],
    IN       GT_U32                                                     freeBlocksBmpArr[],
    INOUT    GT_U32                                                     rowSpaceCountArr[]
)
{
    GT_STATUS   rc;
    GT_U32      rows80BytesRemainder;
    GT_U32      ready_FFFFFF_FFOOOO;
    GT_U32      ready_right_FFOOOO;
    GT_U32      ready_right_FFFFFF;
    GT_U32      ready_left_FFFFFF;
    GT_U32      ready_single_FFFFFF;
    GT_U32      needed_right_FFOOOO;
    /* rowsType, blockInPairBmp, priority - for target only  */
    static PRV_ROW_SPACE_REQUEST_ENTRY_STC   sourceSpaceRequestArr[] =
    {
        {PRV_ROW_SPACE_FFOOOO_FFOOOO_E, 1, 0},
        {PRV_ROW_SPACE_FFOOOO_FFFFFF_E, 1, 0},
        {PRV_ROW_SPACE_FFOOOO_OTHER_E,  1, 0},
        {PRV_ROW_SPACE_SINGLE_FFOOOO_E, 0, 0}
    };
    static GT_U32                            sourceSpaceRequestArrSize =
        (sizeof(sourceSpaceRequestArr) / sizeof(sourceSpaceRequestArr[0]));
    static PRV_ROW_SPACE_REQUEST_ENTRY_STC   targetSpaceRequestArr[] =
    {
        {PRV_ROW_SPACE_FFOOOO_FFFFFF_E, 2, 0},
        {PRV_ROW_SPACE_OTHER_FFFFFF_E,  2, 1},
        {PRV_ROW_SPACE_FFFFFF_FFFFFF_E, 2, 2}
    };
    static GT_U32                            targetSpaceRequestArrSize =
        (sizeof(targetSpaceRequestArr) / sizeof(targetSpaceRequestArr[0]));

    ready_FFFFFF_FFOOOO = rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFOOOO_E];
    /* assumed allocRequestPtr->rulesAmount > ready_FFFFFF_FFOOOO */
    rows80BytesRemainder = (allocRequestPtr->rulesAmount - ready_FFFFFF_FFOOOO);

    ready_right_FFOOOO =
        rowSpaceCountArr[PRV_ROW_SPACE_FFOOOO_FFOOOO_E] +
        rowSpaceCountArr[PRV_ROW_SPACE_OTHER_FFOOOO_E ];
    ready_right_FFFFFF =
        rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFFFFF_E] +
        rowSpaceCountArr[PRV_ROW_SPACE_OTHER_FFFFFF_E ] +
        rowSpaceCountArr[PRV_ROW_SPACE_FFOOOO_FFFFFF_E];
    ready_left_FFFFFF =
        rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFFFFF_E] +
        rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_OTHER_E ];
    ready_single_FFFFFF =
        rowSpaceCountArr[PRV_ROW_SPACE_SINGLE_FFFFFF_E];

    needed_right_FFOOOO =
        prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleAdditionalFfxxxxNumGet(
            (ready_right_FFFFFF + ready_left_FFFFFF + ready_single_FFFFFF),
            ready_right_FFOOOO,
            rows80BytesRemainder);
    if (needed_right_FFOOOO == 0xFFFFFFFF)
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    if (needed_right_FFOOOO == 0)
    {
        /* found enough _FFOOOO_ in the right column */
        return GT_OK;
    }

    rc = prvCpssDxChVirtualTcamDbSegSpaceMoveBlockRows(
        tcamSegCfgPtr, usedBlocksBmpArr, GT_FALSE /*bypassSourceRows*/,
        allocRequestPtr->lookupId, needed_right_FFOOOO,
        sourceSpaceRequestArrSize, sourceSpaceRequestArr,
        targetSpaceRequestArrSize, targetSpaceRequestArr);
    if (rc != GT_OK) return rc;

    /* recalculate counters */
    rc =  prvCpssDxChVirtualTcamDbSegSpaceLookupRowsCountByType(
        tcamSegCfgPtr, usedBlocksBmpArr, freeBlocksBmpArr, rowSpaceCountArr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceMoveFfffffToRelevantPlaces function
* @endinternal
*
* @brief   Move _FFFFFF_ to make _FFFFFF_FFOOOO_ and _FFFFFF_FFFFFF_ pairs.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] usedBlocksBmpArr[]       - (pointer to)bitmap array of indexes of blocks that
*                                      will be mapped to lookup after moving _FFFFFF_ and _FFOOOO_.
* @param[in] freeBlocksBmpArr[]       - (pointer to)bitmap array of indexes of free blocks
* @param[in,out] rowSpaceCountArr[]       - (pointer to)array of counters of counters of free spaces
*                                      of different types from PRV_ROW_SPACE_TYPE_ENT
* @param[in,out] rowSpaceCountArr[]       - (pointer to)array of counters of counters of free spaces
*                                      of different types from PRV_ROW_SPACE_TYPE_ENT
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceMoveFfffffToRelevantPlaces
(
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN       GT_U32                                                     usedBlocksBmpArr[],
    IN       GT_U32                                                     freeBlocksBmpArr[],
    INOUT    GT_U32                                                     rowSpaceCountArr[]
)
{
    GT_STATUS   rc;
    GT_U32      rows80BytesRemainder;
    GT_U32      ready_FFFFFF_FFOOOO;
    GT_U32      ready_FFFFFF_FFFFFF;
    GT_U32      ready_rows;
    /* rowsType, blockInPairBmp, priority - for target only  */
    static PRV_ROW_SPACE_REQUEST_ENTRY_STC   sourceSpaceRequestArr[] =
    {
        {PRV_ROW_SPACE_FFOOOO_FFOOOO_E, 1, 0},
        {PRV_ROW_SPACE_OTHER_FFOOOO_E,  1, 0},
        {PRV_ROW_SPACE_FFOOOO_FFFFFF_E, 1, 0},
        {PRV_ROW_SPACE_OTHER_FFFFFF_E,  1, 0},
        {PRV_ROW_SPACE_FFFFFF_OTHER_E,  2, 0},
        {PRV_ROW_SPACE_FFOOOO_OTHER_E,  3, 0},
        {PRV_ROW_SPACE_OTHER_OTHER_E,   3, 0}
    };
    static GT_U32                            sourceSpaceRequestArrSize =
        (sizeof(sourceSpaceRequestArr) / sizeof(sourceSpaceRequestArr[0]));
    static PRV_ROW_SPACE_REQUEST_ENTRY_STC   targetSpaceRequestArr[] =
    {
        {PRV_ROW_SPACE_SINGLE_FFFFFF_E, 0, 0},
        {PRV_ROW_SPACE_FFOOOO_FFFFFF_E, 2, 0},
        {PRV_ROW_SPACE_OTHER_FFFFFF_E,  2, 1}
    };
    static GT_U32                            targetSpaceRequestArrSize =
        (sizeof(targetSpaceRequestArr) / sizeof(targetSpaceRequestArr[0]));

    ready_FFFFFF_FFOOOO = rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFOOOO_E];
    ready_FFFFFF_FFFFFF = rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFFFFF_E];
    ready_rows =  ready_FFFFFF_FFOOOO + ready_FFFFFF_FFFFFF;
    if (allocRequestPtr->rulesAmount <= ready_rows)
    {
        /* found enough ready rows */
        return GT_OK;
    }
    /* assumed allocRequestPtr->rulesAmount > ready_FFFFFF_FFOOOO */
    rows80BytesRemainder = (allocRequestPtr->rulesAmount - ready_rows);

    rc = prvCpssDxChVirtualTcamDbSegSpaceMoveBlockRows(
        tcamSegCfgPtr, usedBlocksBmpArr, GT_TRUE /*bypassSourceRows*/,
        allocRequestPtr->lookupId, rows80BytesRemainder,
        sourceSpaceRequestArrSize, sourceSpaceRequestArr,
        targetSpaceRequestArrSize, targetSpaceRequestArr);
    if (rc != GT_OK) return rc;

    /* recalculate counters */
    rc =  prvCpssDxChVirtualTcamDbSegSpaceLookupRowsCountByType(
        tcamSegCfgPtr, usedBlocksBmpArr, freeBlocksBmpArr, rowSpaceCountArr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbSegSpace80bytesSpaceMake function
* @endinternal
*
* @brief   Make free space ro 80-byte rules.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegSpace80bytesSpaceMake
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr
)
{
    GT_STATUS rc;
    GT_U32    actualCapacity;
    GT_U32    freeBlocksBmpArr[PRV_WORDS_IN_BLOCK_BITMAP_CNS];
    GT_U32    lookupBlocksBmpArr[PRV_WORDS_IN_BLOCK_BITMAP_CNS];
    GT_U32    freeBlocksBmpCopyArr[PRV_WORDS_IN_BLOCK_BITMAP_CNS];
    GT_U32    lookupBlocksBmpCopyArr[PRV_WORDS_IN_BLOCK_BITMAP_CNS];
    GT_U32    blocksOwnOwnNum;
    GT_U32    blocksOwnOtherNum;
    GT_U32    blocksOwnFreeNum;
    GT_U32    blocksOtherFreeNum;
    GT_U32    blocksFreeFreeNum;
    GT_U32    lookup80byteRulesNum;
    GT_U32    blockPairsWithoutOther;
    GT_U32    totalRowsWithoutOther;
    GT_U32    swapBlocksNeeded;
    GT_U32    ffooooRowsNumArr[PRV_OWN_BLOCK_TYPE_LAST_E];
    GT_U32    ffffffRowsNumArr[PRV_OWN_BLOCK_TYPE_LAST_E];
    GT_U32    freeBlocksNumArr[PRV_FREE_BLOCK_TYPE_LAST_E];
    GT_U32    freeBlockPairsNum, ownBlockPairsNum;
    GT_U32    freableRowsTotal;
    GT_U32    freableRowsInLookup;
    GT_U32    addedBlockPairsRemainder;
    GT_U32    addedFreeBlockPairsToLookup;
    GT_U32    addedFreeNeighborBlocksToLookup;
    GT_U32    addedFreeBlocksForRowsToLookup;
    GT_U32    addedAnyBlocksForSpace;
    GT_U32    i;
    GT_U32    ffooooInLookupTotal;
    GT_U32    ffffffInLookupTotal;
    GT_U32    notInLookupRowsNeeded;
    GT_U32    usedBlocksBmpArr[PRV_WORDS_IN_BLOCK_BITMAP_CNS];
    GT_U32    rowSpaceCountArr[PRV_ROW_SPACE_TYPE_LAST_E];

    if (tcamSegCfgPtr->deviceClass !=
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (((sizeof(freeBlocksBmpArr) / sizeof(freeBlocksBmpArr[0])) * 32) < tcamSegCfgPtr->blocksAmount)
    {
        /* device has more blocks than currently supported */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    if (allocRequestPtr->ruleSize != 8) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* check free block neighbor pairs */
    prvCpssDxChVirtualTcamDbSegSpaceBlockBitmapsGet(
        tcamSegCfgPtr, allocRequestPtr->lookupId,
        freeBlocksBmpArr, lookupBlocksBmpArr);
    blocksFreeFreeNum = 0;
    for (i = 0; (i < tcamSegCfgPtr->blocksAmount); i += 2)
    {
        if (((freeBlocksBmpArr[i / 32] >> (i % 32)) & 3) == 3)
        {
            blocksFreeFreeNum ++;
        }
    }
    if ((blocksFreeFreeNum * tcamSegCfgPtr->lookupRowAlignment)
        >= allocRequestPtr->rulesAmount)
    {
        return GT_OK;
    }

    /* check hole making capacity */
    rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId80bytesHolesMake(
        tcamSegCfgPtr, allocRequestPtr, GT_TRUE /*checkWithoutUpdate*/,
        &actualCapacity, freeBlocksBmpArr, lookupBlocksBmpArr);
    if (rc != GT_OK) return rc;
    if (actualCapacity < allocRequestPtr->rulesAmount)
    {
        rc = prvCpssDxChVirtualTcamDbBlocksMakeSpace(
            tcamSegCfgPtr, allocRequestPtr,
            (allocRequestPtr->rulesAmount - actualCapacity));
        if (rc != GT_OK) return rc;

        /* additional attempt after lookup compressing */
        rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId80bytesHolesMake(
            tcamSegCfgPtr, allocRequestPtr, GT_TRUE /*checkWithoutUpdate*/,
            &actualCapacity, freeBlocksBmpArr, lookupBlocksBmpArr);
        if (actualCapacity < allocRequestPtr->rulesAmount) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
    }

    /* check blocks swaping needs and capacity */
    prvCpssDxChVirtualTcamDbSegSpace80bytesLookupRowsStatisticsGet(
        tcamSegCfgPtr, allocRequestPtr->lookupId,
        freeBlocksBmpArr, lookupBlocksBmpArr,
        &blocksOwnOwnNum, &blocksOwnOtherNum,
        &blocksOwnFreeNum, &blocksOtherFreeNum,
        &blocksFreeFreeNum, &lookup80byteRulesNum);

    if ((blocksOwnOwnNum * tcamSegCfgPtr->lookupRowAlignment)
        < lookup80byteRulesNum)
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* rows in pairs of blocks not occupated by other lookups */
    blockPairsWithoutOther =
        (blocksOwnOwnNum + blocksOwnFreeNum + blocksFreeFreeNum);
    totalRowsWithoutOther =
        (blockPairsWithoutOther * tcamSegCfgPtr->lookupRowAlignment);

    swapBlocksNeeded = 0; /* init */
    if (totalRowsWithoutOther < (lookup80byteRulesNum + allocRequestPtr->rulesAmount))
    {
        /* needed full-block swapping */
        swapBlocksNeeded =
            ((allocRequestPtr->rulesAmount - (totalRowsWithoutOther - lookup80byteRulesNum)
              + (tcamSegCfgPtr->lookupRowAlignment - 1))
             / tcamSegCfgPtr->lookupRowAlignment);
    }

    if (swapBlocksNeeded > 0)
    {
        cpssOsMemCpy(freeBlocksBmpCopyArr, freeBlocksBmpArr, sizeof(freeBlocksBmpArr));
        cpssOsMemCpy(lookupBlocksBmpCopyArr, lookupBlocksBmpArr, sizeof(lookupBlocksBmpArr));

        rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherSetOfBlock(
            tcamSegCfgPtr, swapBlocksNeeded, GT_TRUE /*checkOnly*/,
            freeBlocksBmpCopyArr, lookupBlocksBmpCopyArr);
        if (rc != GT_OK) return rc;
    }

    /* make holes */
    rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId80bytesHolesMake(
        tcamSegCfgPtr, allocRequestPtr, GT_FALSE /*checkWithoutUpdate*/,
        &actualCapacity, freeBlocksBmpArr, lookupBlocksBmpArr);
    if (rc != GT_OK) return rc;

    /* swap blocks if needed */
    if (swapBlocksNeeded > 0)
    {
        rc = prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsToOtherSetOfBlock(
            tcamSegCfgPtr, swapBlocksNeeded, GT_FALSE /*checkOnly*/,
            freeBlocksBmpArr, lookupBlocksBmpArr);
        if (rc != GT_OK)
        {
            /* check only call passed with GT_OK */
            prvCpssDxChVirtualTcamDbSegmentTableException();
            return rc;
        }
    }

    /* move holes for 80bytes                                     */
    /* calculation                                                */
    /* phase0: prepare places for _FFOOOO_ swapping _FFFFFF_ with */
    /*         any content at these places                        */
    /* phase1: swap _FFOOOO_ with _FFFFFF_ at prepared places     */
    /* phase2: swap _FFFFFF_ with any content at needed places    */


    prvCpssDxChVirtualTcamDbSegSpaceBloksInPairsStatusGet(
        tcamSegCfgPtr, allocRequestPtr->lookupId,
        freeBlocksBmpArr, lookupBlocksBmpArr,
        ffooooRowsNumArr, ffffffRowsNumArr, freeBlocksNumArr,
        &freeBlockPairsNum, &ownBlockPairsNum);

    /* first stage of calculations based of block pair rows   */
    /* that reside in blocks not occupied by other lookups   */
    /* this stage calculates amount of blocks                 */
    /* regardless of  the contents of blocks mapped to lookup */
    /* beside existing 80-byte rules                          */

    blockPairsWithoutOther =
        (freeBlockPairsNum + ownBlockPairsNum
        + freeBlocksNumArr[PRV_FREE_BLOCK_TYPE_RIGHT_OWN_FREE_E]
        + freeBlocksNumArr[PRV_FREE_BLOCK_TYPE_LEFT_FREE_OWN_E]);
    if ((allocRequestPtr->rulesAmount + lookup80byteRulesNum) >
            (blockPairsWithoutOther * tcamSegCfgPtr->lookupRowAlignment))
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    freableRowsTotal =
        ((blockPairsWithoutOther * tcamSegCfgPtr->lookupRowAlignment)
        - lookup80byteRulesNum);
    if ((allocRequestPtr->rulesAmount + lookup80byteRulesNum) > freableRowsTotal)
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
    }

    freableRowsInLookup =
        ((ownBlockPairsNum * tcamSegCfgPtr->lookupRowAlignment)
        - lookup80byteRulesNum);

    /* adding to lookup or one free block of pair own-free or free-own */
    /* or all pair free-free                                           */
    addedBlockPairsRemainder = 0;
    if (allocRequestPtr->rulesAmount > freableRowsInLookup)
    {
        addedBlockPairsRemainder =
            ((allocRequestPtr->rulesAmount - freableRowsInLookup
             + tcamSegCfgPtr->lookupRowAlignment - 1)
             / tcamSegCfgPtr->lookupRowAlignment);
    }

    addedFreeNeighborBlocksToLookup =
        (freeBlocksNumArr[PRV_FREE_BLOCK_TYPE_RIGHT_OWN_FREE_E]
         + freeBlocksNumArr[PRV_FREE_BLOCK_TYPE_LEFT_FREE_OWN_E]);
    if (addedFreeNeighborBlocksToLookup > addedBlockPairsRemainder)
    {
        addedFreeNeighborBlocksToLookup = addedBlockPairsRemainder;
    }
    addedBlockPairsRemainder -= addedFreeNeighborBlocksToLookup;

    addedFreeBlockPairsToLookup = freeBlockPairsNum;
    if (addedFreeBlockPairsToLookup > addedBlockPairsRemainder)
    {
        addedFreeBlockPairsToLookup = addedBlockPairsRemainder;
    }
    addedBlockPairsRemainder -= addedFreeBlockPairsToLookup;

    if (addedBlockPairsRemainder > 0)
    {
        /* should never occur */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    addedFreeBlocksForRowsToLookup =
        ((2 * addedFreeBlockPairsToLookup) + addedFreeNeighborBlocksToLookup);

    /* addedSingleNeighborBlocks and addedFreeBlockPairs calculated */
    /* regardless of  the contents of blocks mapped to lookup       */
    /* beside existing 80-byte rules.                               */
    /* Now calculate addedAnyBlocks                                 */
    /* dependent of the contents of blocks mapped to lookup.        */

    ffooooInLookupTotal = 0;
    ffffffInLookupTotal = 0;
    for (i = 0; (i < PRV_OWN_BLOCK_TYPE_LAST_E); i++)
    {
        ffooooInLookupTotal += ffooooRowsNumArr[i];
        ffffffInLookupTotal += ffffffRowsNumArr[i];
    }

    notInLookupRowsNeeded =
        prvCpssDxChVirtualTcamDbSegSpaceWorkDb80byteMakeHoleAdditionalRowsNumGet(
            ffffffInLookupTotal, ffooooInLookupTotal, allocRequestPtr->rulesAmount);

    addedAnyBlocksForSpace = 0;
    if (notInLookupRowsNeeded >
        (addedFreeBlocksForRowsToLookup * tcamSegCfgPtr->lookupRowAlignment))
    {
        addedAnyBlocksForSpace =
            ((notInLookupRowsNeeded -
                (addedFreeBlocksForRowsToLookup * tcamSegCfgPtr->lookupRowAlignment)
                + tcamSegCfgPtr->lookupRowAlignment - 1)
                / tcamSegCfgPtr->lookupRowAlignment);
    }

    /* bit map of all blocks that will be mapped to own lookup */
    rc = prvCpssDxChVirtualTcamDbSegSpaceNewLookupBlocksBitmapGet(
        tcamSegCfgPtr, freeBlocksBmpArr, lookupBlocksBmpArr,
        addedFreeBlockPairsToLookup, addedFreeNeighborBlocksToLookup,
        addedAnyBlocksForSpace, usedBlocksBmpArr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    rc =  prvCpssDxChVirtualTcamDbSegSpaceLookupRowsCountByType(
        tcamSegCfgPtr, usedBlocksBmpArr, freeBlocksBmpArr, rowSpaceCountArr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if ((rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFFFFF_E]
          + rowSpaceCountArr[PRV_ROW_SPACE_FFFFFF_FFOOOO_E])
        >= allocRequestPtr->rulesAmount)
    {
        /* already found needed amount of ready rows */
        return GT_OK;
    }

    rc = prvCpssDxChVirtualTcamDbSegSpaceFreePlaceForRightFfxxxx(
        tcamSegCfgPtr, allocRequestPtr,
        usedBlocksBmpArr, freeBlocksBmpArr,
        rowSpaceCountArr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChVirtualTcamDbSegSpaceMoveFfxxxxToRight(
        tcamSegCfgPtr, allocRequestPtr,
        usedBlocksBmpArr, freeBlocksBmpArr,
        rowSpaceCountArr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChVirtualTcamDbSegSpaceMoveFfffffToRelevantPlaces(
        tcamSegCfgPtr, allocRequestPtr,
        usedBlocksBmpArr, freeBlocksBmpArr,
        rowSpaceCountArr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpaceTraceEnable)
    {
        cpssOsPrintf("after prvCpssDxChVirtualTcamDbSegSpace80bytesSpaceMake \n");
        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayout(tcamSegCfgPtr->pVTcamMngPtr->vTcamMngId);
        prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpace(tcamSegCfgPtr->pVTcamMngPtr->vTcamMngId, GT_TRUE);
    }

    return GT_OK;
}




