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
#define  OPTF_EDITOR_AUTOFOCUS        13
#define  OPTF_QSEARCH_AUTOFOCUS       14
#define  OPTF_DOCK_RECT_DISABLED      15
#define  OPTF_COUNT                   16

#define  OPT_DOCK_RECT                16
#define  OPT_COLOR_NOTFOUND           17
#define  OPT_COLOR_NOTREGEXP          18
#define  OPT_COLOR_EOF                19
#define  OPT_COLOR_HIGHLIGHT          20
#define  OPT_HIGHLIGHT_MARK_ID        21
#define  OPT_HIGHLIGHT_STATE          22
#define  OPT_USE_ALT_HOTKEYS          23
#define  OPT_ALT_MATCHCASE            24
#define  OPT_ALT_WHOLEWORD            25
#define  OPT_ALT_HIGHLIGHTALL         26
#define  OPT_FIND_HISTORY_ITEMS       27
#define  OPT_HISTORY_SAVE             28
#define  OPT_NEW_UI                   29
#define  OPT_SELECT_BY_F3             30
#define  OPT_ADJ_INCOMPL_REGEXP       31

#define  OPT_TOTALCOUNT               32

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
