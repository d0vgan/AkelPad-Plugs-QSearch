#ifndef _akel_qsearch_defs_h_
#define _akel_qsearch_defs_h_
//---------------------------------------------------------------------------

#define  OPTF_SRCH_ONTHEFLY_MODE       0
#define  OPTF_SRCH_FROM_BEGINNING      1
#define  OPTF_SRCH_USE_SPECIALCHARS    2
#define  OPTF_SRCH_USE_REGEXP          3
#define  OPTF_SRCH_REGEXP_DOT_NEWLINE  4
#define  OPTF_SRCH_PICKUP_SELECTION    5
#define  OPTF_SRCH_SELFIND_PICKUP      6
#define  OPTF_SRCH_STOP_EOF            7
#define  OPTF_SRCH_WND_DOCKEDTOP       8
#define  OPTF_EDIT_FOCUS_SELECTALL     9
#define  OPTF_CATCH_MAIN_F3           10
#define  OPTF_CATCH_MAIN_ESC          11
#define  OPTF_HOTKEY_HIDES_PANEL      12
#define  OPTF_EDITOR_AUTOFOCUS_MOUSE  13
#define  OPTF_QSEARCH_AUTOFOCUS_MOUSE 14
#define  OPTF_QSEARCH_AUTOFOCUS_FILE  15
#define  OPTF_DOCK_RECT_DISABLED      16
#define  OPTF_COUNT                   17

#define  OPT_DOCK_RECT                17
#define  OPT_COLOR_NOTFOUND           18
#define  OPT_COLOR_NOTREGEXP          19
#define  OPT_COLOR_EOF                20
#define  OPT_COLOR_HIGHLIGHT          21
#define  OPT_HIGHLIGHT_MARK_ID        22
#define  OPT_HIGHLIGHT_STATE          23
#define  OPT_USE_ALT_HOTKEYS          24
#define  OPT_ALT_MATCHCASE            25
#define  OPT_ALT_WHOLEWORD            26
#define  OPT_ALT_HIGHLIGHTALL         27
#define  OPT_FIND_HISTORY_ITEMS       28
#define  OPT_HISTORY_SAVE             29
#define  OPT_NEW_UI                   30
#define  OPT_SELECT_BY_F3             31
#define  OPT_ADJ_INCOMPL_REGEXP       32

#define  OPT_TOTALCOUNT               33

#define  PICKUP_SEL_ALWAYS             1
#define  PICKUP_SEL_IF_NOT_QSBTN      11

#define  STOP_EOF_WITH_MSG             1
#define  STOP_EOF_WITHOUT_MSG         11

#define  HLS_IS_CHECKED                1
#define  HLS_SET_ALWAYS               10

#define  ESC_CATCH_FROM_MAIN           1
#define  ESC_THROW_TO_MAIN            10

#define  AUTOFOCUS_EDITOR              1
#define  AUTOFOCUS_QSEARCH            10
#define  AUTOFOCUS_EDITOR_ALWAYS     100

#define  QS_UI_ORIGINAL                0
#define  QS_UI_NEW_01                  1
#define  QS_UI_NEW_02                  2

//---------------------------------------------------------------------------
#endif
