#ifndef _akel_qsearch_defs_h_
#define _akel_qsearch_defs_h_
//---------------------------------------------------------------------------

#define  OPTF_SRCH_ONTHEFLY_MODE     0
#define  OPTF_SRCH_FROM_BEGINNING    1
#define  OPTF_SRCH_USE_SPECIALCHARS  2
#define  OPTF_SRCH_USE_REGEXP        3
#define  OPTF_SRCH_PICKUP_SELECTION  4
#define  OPTF_SRCH_SELFIND_PICKUP    5
#define  OPTF_SRCH_STOP_EOF          6
#define  OPTF_SRCH_WND_DOCKEDTOP     7
#define  OPTF_EDIT_FOCUS_SELECTALL   8
#define  OPTF_CATCH_MAIN_F3          9
#define  OPTF_CATCH_MAIN_ESC        10
#define  OPTF_HOTKEY_HIDES_PANEL    11
#define  OPTF_EDITOR_AUTOFOCUS      12
#define  OPTF_QSEARCH_AUTOFOCUS     13
#define  OPTF_DOCK_RECT_DISABLED    14
#define  OPTF_COUNT                 15

#define  OPT_DOCK_RECT              15
#define  OPT_COLOR_NOTFOUND         16
#define  OPT_COLOR_NOTREGEXP        17
#define  OPT_COLOR_EOF              18
#define  OPT_COLOR_HIGHLIGHT        19
#define  OPT_HIGHLIGHT_MARK_ID      20
#define  OPT_HIGHLIGHT_STATE        21
#define  OPT_USE_ALT_HOTKEYS        22
#define  OPT_ALT_MATCHCASE          23
#define  OPT_ALT_WHOLEWORD          24
#define  OPT_ALT_HIGHLIGHTALL       25
#define  OPT_FIND_HISTORY_ITEMS     26
#define  OPT_HISTORY_SAVE           27
#define  OPT_NEW_UI                 28
#define  OPT_SELECT_BY_F3           29

#define  OPT_TOTALCOUNT             30

#define  PICKUP_SEL_ALWAYS           1
#define  PICKUP_SEL_IF_NOT_QSBTN    11

#define  STOP_EOF_WITH_MSG           1
#define  STOP_EOF_WITHOUT_MSG       11

#define  HLS_IS_CHECKED              1
#define  HLS_SET_ALWAYS             10

#define  ESC_CATCH_FROM_MAIN         1
#define  ESC_THROW_TO_MAIN          10

#define  AUTOFOCUS_EDITOR            1
#define  AUTOFOCUS_QSEARCH          10
#define  AUTOFOCUS_EDITOR_ALWAYS   100

#define  QS_UI_ORIGINAL              0
#define  QS_UI_NEW_01                1
#define  QS_UI_NEW_02                2

//---------------------------------------------------------------------------
#endif
