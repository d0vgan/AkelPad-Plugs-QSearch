#include "tests_QSearchFindEx.h"
#include "../../QSearch/QSearchFindEx.h"

#include <assert.h>
#include <conio.h>
#include <stdio.h>
#include <time.h>

// required by "QSearchFindEx.c"
typedef struct sPluginState {
    HWND hMainWnd;
} PluginState;
// required by "QSearchFindEx.c"
typedef struct sQSearchDlgState {
    HWND hDlg;
} QSearchDlgState;
// required by "QSearchFindEx.c"
PluginState     g_Plugin;
QSearchDlgState g_QSearchDlg;

// required by "QSearchFindEx.c"
HWND GetWndEdit(HWND hWnd)
{
    return NULL;
}

#define QSF_WW_DELIM   0x01
#define QSF_WW_SPACE   0x02

static void test_question_mark_wildcard()
{
    wchar_t* last_pos = NULL;

    // Single character match
    const wchar_t* str = L"hello";
    int result = match_maskw(L"h?llo", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello";
    result = match_maskw(L"h?ll?", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello";
    result = match_maskw(L"?e?l?", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Must match entire string
    str = L"hello world";
    result = match_maskw(L"h?llo", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    str = L"hello world";
    result = match_maskw(L"?llo*", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    // With asterisk can match partial
    str = L"hello world";
    result = match_maskw(L"h?llo*", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == wcsstr(str, L" w"));

    str = L"hello world";
    result = match_maskw(L"*?llo*", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == wcsstr(str, L" w"));

    str = L"hello world";
    result = match_maskw(L"h*d", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello world";
    result = match_maskw(L"h?*?d", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"logoutput_frp_mode";
    result = match_maskw(L"logoutput_*_mode", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // No match
    str = L"hello";
    result = match_maskw(L"?hello", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    str = L"hello";
    result = match_maskw(L"hello?", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    // Test with word_wrap
    str = L"hello.";
    result = match_maskw(L"hello?", str, &last_pos, 1);
    assert(result < 0);
    assert(last_pos == NULL);
}

static void test_asterisk_wildcard()
{
    wchar_t* last_pos = NULL;

    // Match anything after
    const wchar_t* str = L"hello world";
    int result = match_maskw(L"hello*", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == wcsstr(str, L" w"));

    // Match anything before
    str = L"hello world";
    result = match_maskw(L"*world", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Match substring with wildcards
    str = L"hello world";
    result = match_maskw(L"*ell*", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == wcsstr(str, L"o w"));

    // Match exact string with optional suffix/prefix
    str = L"hello";
    result = match_maskw(L"hello*", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello";
    result = match_maskw(L"*hello", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello";
    result = match_maskw(L"*hello*", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello.";
    result = match_maskw(L"hel*", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == wcsstr(str, L"lo."));

    str = L".hello";
    result = match_maskw(L"*llo", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // No match with word_wrap
    str = L"hello.";
    result = match_maskw(L"hel*", str, &last_pos, 1);
    assert(result == 0);
    assert(last_pos == NULL);

    str = L".hello";
    result = match_maskw(L"*llo", str, &last_pos, 1);
    assert(result < 0);
    assert(last_pos == NULL);

    // No match without wildcard
    str = L"hello world";
    result = match_maskw(L"hello", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    str = L"hello world";
    result = match_maskw(L"world", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);
}

static void test_double_asterisk()
{
    wchar_t* last_pos = NULL;

    // Match non-space characters when whole_word is set
    const wchar_t* str = L"hello";
    int result = match_maskw(L"h**o", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello";
    result = match_maskw(L"**h**o", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello";
    result = match_maskw(L"h**o**", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello";
    result = match_maskw(L"**h**o**", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"logoutput_frp_mode";
    result = match_maskw(L"logoutput_**_mode", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"logoutput_frp_mode";
    result = match_maskw(L"log**frp**mode", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Should not match across spaces when whole_word is set
    str = L"hello world";
    result = match_maskw(L"h**d", str, &last_pos, 1);
    assert(result < 0);
    assert(last_pos == NULL);

    str = L"hello ";
    result = match_maskw(L"hel**", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == wcsstr(str, L" "));

    str = L" hello";
    result = match_maskw(L"**llo", str, &last_pos, 1);
    assert(result < 0);
    assert(last_pos == NULL);

    // Should match across non-spaces when whole_word is set
    str = L"hello.world";
    result = match_maskw(L"h**d", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello[world";
    result = match_maskw(L"h**d", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Without whole_word, should behave like single asterisk
    str = L"hello world";
    result = match_maskw(L"h**d", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello world";
    result = match_maskw(L"**h**d", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L" hello world";
    result = match_maskw(L"**h**d", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello world";
    result = match_maskw(L"h**d**", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello world ";
    result = match_maskw(L"h**d**", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str) - 1);

    str = L"hello world";
    result = match_maskw(L"**h**d**", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L" hello world";
    result = match_maskw(L"**h**d**", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello world";
    result = match_maskw(L"**h**r**", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == wcsstr(str, L"ld"));

    // whole_word cases
    str = L"hello world";
    result = match_maskw(L"**h** **r**", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello world ";
    result = match_maskw(L"**h** **r**", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str) - 1);

    str = L"hello world,?. ";
    result = match_maskw(L"**h** **r**", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == wcsstr(str, L",?."));

    str = L"hello world,?. ";
    result = match_maskw(L"**h** **r*", str, &last_pos, 1);
    assert(result == 0);
    assert(last_pos == NULL);
}

static void test_escaped_characters()
{
    wchar_t* last_pos = NULL;

    // Test \n (line feed)
    const wchar_t* str = L"hello\nworld";
    int result = match_maskw(L"hello\nworld", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Test \t (tabulation)
    str = L"hello\tworld";
    result = match_maskw(L"hello\tworld", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Test \\ (backslash)
    str = L"hello\\world";
    result = match_maskw(L"hello\\\\world", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Test \* (asterisk)
    str = L"hello*world";
    result = match_maskw(L"hello\\*world", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Test \? (question mark)
    str = L"hello?world";
    result = match_maskw(L"hello\\?world", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Test escaped characters at end of pattern
    str = L"hello*";
    result = match_maskw(L"hello\\*", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Test with word_wrap
    str = L"hello.";
    result = match_maskw(L"hello\\x", str, &last_pos, 1);
    assert(result < 0);
    assert(last_pos == NULL);

    str = L"hello.";
    result = match_maskw(L"h*o\\x", str, &last_pos, 1);
    assert(result < 0);
    assert(last_pos == NULL);

    str = L"hello.o?";
    result = match_maskw(L"h**o\\?", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));
}

static void test_complex_patterns()
{
    wchar_t* last_pos = NULL;

    // Mix of ? and * - full string match
    const wchar_t* str = L"hello world";
    int result = match_maskw(L"h?l*o w*d", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Escaped characters with wildcards
    str = L"*test?123";
    result = match_maskw(L"\\**\\?*", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == wcsstr(str, L"123"));

    // Multiple escaped characters - exact match
    str = L"\\*?";
    result = match_maskw(L"\\\\\\*\\?", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Pattern with multiple wildcards
    str = L"hello world";
    result = match_maskw(L"h*o*d*", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello world";
    result = match_maskw(L"h*o*r*", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == wcsstr(str, L"ld"));

    str = L"abcx abcd abcef abceg";
    result = match_maskw(L"abc*d a?c*g", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));
}

static void test_edge_cases()
{
    wchar_t* last_pos = NULL;

    // Empty pattern and string
    const wchar_t* str = L"";
    int result = match_maskw(L"", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str);

    // Only wildcards
    str = L"abc";
    result = match_maskw(L"***", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str);

    // Only question marks
    str = L"abc";
    result = match_maskw(L"???", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Pattern longer than string
    str = L"abc";
    result = match_maskw(L"abcd", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    str = L"abc";
    result = match_maskw(L"abc*d", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    str = L"abc";
    result = match_maskw(L"a?cd", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    str = L"abc";
    result = match_maskw(L"????", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    // Trailing backslash
    str = L"abc";
    result = match_maskw(L"abc\\", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);
}

static void test_exact_matches()
{
    wchar_t* last_pos = NULL;

    // Exact match - should succeed
    const wchar_t* str = L"hello";
    int result = match_maskw(L"hello", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello world";
    result = match_maskw(L"hello world", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    str = L"hello world";
    result = match_maskw(L"hello world", str, &last_pos, 1);
    assert(result > 0);
    assert(last_pos == str + wcslen(str));

    // Partial match at start - should fail
    str = L"hello world";
    result = match_maskw(L"hello", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    // Partial match at end - should fail
    str = L"hello world";
    result = match_maskw(L"world", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    // Substring match - should fail
    str = L"hello";
    result = match_maskw(L"ell", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);
}

#ifdef _DEBUG
static void test_last_pos_differences()
{
    wchar_t* last_pos = NULL;
    const wchar_t* str;
    int result;

    // Test 1: Simple pattern with asterisk
    str = L"hello world";
    result = match_maskw(L"h*o", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == NULL);

    last_pos = NULL;
    result = match_maskw0(L"h*o", str, &last_pos, 0);
    assert(result == 0);
    assert(last_pos == str + wcslen(str));

    // Test 2: Pattern with multiple asterisks
    str = L"hello world test";
    result = match_maskw(L"h*o*d*t", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str)); // points to end of string

    last_pos = NULL;
    result = match_maskw0(L"h*o*d*t", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str)); // points to somewhere in the middle

    // Test 3: Exact match (no wildcards)
    str = L"hello";
    result = match_maskw(L"hello", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str)); // points to end of string

    last_pos = NULL;
    result = match_maskw0(L"hello", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str)); // should be the same for exact matches

    // Test 4: Pattern with question marks
    str = L"hello world";
    result = match_maskw(L"h?llo w?rld", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str)); // points to end of string

    last_pos = NULL;
    result = match_maskw0(L"h?llo w?rld", str, &last_pos, 0);
    assert(result > 0);
    assert(last_pos == str + wcslen(str)); // should be the same for exact matches
}
#endif

static void test_match_maskw()
{
    test_question_mark_wildcard();
    test_asterisk_wildcard();
    test_double_asterisk();
    test_escaped_characters();
    test_complex_patterns();
    test_edge_cases();
    test_exact_matches();
#ifdef _DEBUG
    test_last_pos_differences();
#endif
}

void tests_qsearch_findex()
{
    /*
    clock_t clock_start;
    clock_t clock_end;
    double d;
    int i;

    clock_start = clock();
    for (i = 0; i < 100000; i++)
    {
    */

    test_match_maskw();

    /*
    }
    clock_end = clock();

    d = ((double)(clock_end - clock_start) * 1000.0) / CLOCKS_PER_SEC;
    printf("%s took %.3f ms\n", __FUNCTION__, d);
    */

    printf("%s() passed!\n", __FUNCTION__);
}
