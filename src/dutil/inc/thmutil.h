#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="thmutil.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
//
// <summary>
//  Theme helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

enum THEME_CONTROL_DATA
{
    THEME_CONTROL_DATA_HOVER = 1,
};

enum THEME_CONTROL_TYPE
{
    THEME_CONTROL_TYPE_UNKNOWN,
    THEME_CONTROL_TYPE_BUTTON,
    THEME_CONTROL_TYPE_CHECKBOX,
    THEME_CONTROL_TYPE_EDITBOX,
    THEME_CONTROL_TYPE_HYPERLINK,
    THEME_CONTROL_TYPE_IMAGE,
    THEME_CONTROL_TYPE_PROGRESSBAR,
    THEME_CONTROL_TYPE_RICHEDIT,
    THEME_CONTROL_TYPE_STATIC,
    THEME_CONTROL_TYPE_TEXT,
    THEME_CONTROL_TYPE_LISTVIEW,
    THEME_CONTROL_TYPE_TREEVIEW,
    THEME_CONTROL_TYPE_TAB,
};


struct THEME_COLUMN
{
    LPWSTR pszName;
    UINT uStringId;
    int nWidth;
};


struct THEME_TAB
{
    LPWSTR pszName;
    UINT uStringId;
};

// THEME_ASSIGN_CONTROL_ID - Used to apply a specific id to a named control (usually
//                           to set the WM_COMMAND).
struct THEME_ASSIGN_CONTROL_ID
{
    WORD wId;       // id to apply to control
    LPCWSTR wzName; // name of control to match
};

const DWORD THEME_FIRST_ASSIGN_CONTROL_ID = 1024; // Recommended first control id to be assigned.

struct THEME_CONTROL
{
    THEME_CONTROL_TYPE type;

    WORD wId;
    WORD wPageId;

    LPWSTR wzName; // optional name for control, only used to apply control id.
    LPWSTR wzText;
    int nX;
    int nY;
    int nHeight;
    int nWidth;
    int nSourceX;
    int nSourceY;
    UINT uStringId;

    // Used by listview controls
    THEME_COLUMN *ptcColumns;
    DWORD cColumns;

    // Used by tab controls
    THEME_TAB *pttTabs;
    DWORD cTabs;

    BOOL fHideWhenDisabled;
    DWORD dwStyle;
    DWORD dwExtendedStyle;
    DWORD dwFontId;
    DWORD dwFontHoverId;
    DWORD dwFontSelectedId;

    // state variables that should be ignored
    HWND hWnd;
    DWORD dwData; // type specific data
};


struct THEME_PAGE
{
    WORD wId;
    LPWSTR wzName;

    DWORD cControlIndices;
    DWORD* rgdwControlIndices;
};

struct THEME_FONT
{
    HFONT hFont;
    COLORREF crForeground;
    HBRUSH hForeground;
    COLORREF crBackground;
    HBRUSH hBackground;
};


struct THEME
{
    WORD wId;

    DWORD dwStyle;
    DWORD dwFontId;
    HANDLE hIcon;
    LPWSTR wzCaption;
    int nHeight;
    int nWidth;
    int nSourceX;
    int nSourceY;
    UINT uStringId;

    HBITMAP hImage;

    DWORD cFonts;
    THEME_FONT* rgFonts;

    DWORD cPages;
    THEME_PAGE* rgPages;

    DWORD cControls;
    THEME_CONTROL* rgControls;

    // state variables that should be ignored
    HWND hwndParent; // parent for loaded controls
    HACCEL hActiveAcceleratorTable; // currently active accelerator table
    HWND hwndHover; // current hwnd hovered over
};


HRESULT DAPI ThemeInitialize(
    __in HMODULE hModule
    );

void DAPI ThemeUninitialize();

HRESULT DAPI ThemeLoadFromFile(
    __in_z LPCWSTR wzThemeFile,
    __out THEME** ppTheme
    );

HRESULT DAPI ThemeLoadFromResource(
    __in_opt HMODULE hModule,
    __in_z LPCSTR szResource,
    __out THEME** ppTheme
    );

void DAPI ThemeFree(
    __in THEME* pTheme
    );

HRESULT DAPI ThemeLoadControls(
    __in THEME* pTheme,
    __in HWND hwndParent,
    __in_ecount_opt(cAssignControlIds) THEME_ASSIGN_CONTROL_ID* rgAssignControlIds,
    __in DWORD cAssignControlIds
    );

HRESULT DAPI ThemeLoadLocFromFile(
    __in THEME* pTheme,
    __in_z LPCWSTR wzFileName,
    __in HMODULE hModule
    );

HRESULT DAPI ThemeLoadStrings(
    __in THEME* pTheme,
    __in HMODULE hResModule
    );

HRESULT DAPI ThemeLoadRichEditFromFile(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in_z LPCWSTR wzFileName,
    __in HMODULE hModule
    );

BOOL DAPI ThemeTranslateAccelerator(
    __in_opt THEME* pTheme,
    __in HWND hWnd,
    __in MSG* pMsg
    );

LRESULT CALLBACK ThemeDefWindowProc(
    __in_opt THEME* pTheme,
    __in HWND hWnd,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    );

void DAPI ThemeGetPageIds(
    __in THEME* pTheme,
    __in_ecount(cGetPages) LPCWSTR* rgwzFindNames,
    __in_ecount(cGetPages) DWORD* rgdwPageIds,
    __in DWORD cGetPages
    );

void DAPI ThemeShowPage(
    __in THEME* pTheme,
    __in DWORD dwPage,
    __in int nCmdShow
    );

BOOL DAPI ThemeControlExists(
    __in THEME* pTheme,
    __in DWORD dwControl
    );

void DAPI ThemeControlEnable(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in BOOL fEnable
    );

void DAPI ThemeShowControl(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in int nCmdShow
    );

BOOL DAPI ThemePostControlMessage(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in UINT Msg,
    __in WPARAM wParam,
    __in LPARAM lParam
    );

LRESULT DAPI ThemeSendControlMessage(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in UINT Msg,
    __in WPARAM wParam,
    __in LPARAM lParam
    );

HRESULT DAPI ThemeDrawBackground(
    __in THEME* pTheme,
    __in PAINTSTRUCT* pps
    );

HRESULT DAPI ThemeDrawControl(
    __in THEME* pTheme,
    __in DRAWITEMSTRUCT* pdis
    );

void DAPI ThemeHoverControl(
    __in THEME* pTheme,
    __in HWND hwndParent,
    __in HWND hwndControl
    );

BOOL DAPI ThemeIsControlChecked(
    __in THEME* pTheme,
    __in DWORD dwControl
    );

BOOL DAPI ThemeSetControlColor(
    __in THEME* pTheme,
    __in HDC hdc,
    __in HWND hWnd,
    __out HBRUSH* phBackgroundBrush
    );

HRESULT DAPI ThemeSetProgressControl(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in DWORD dwProgressPercentage
    );

HRESULT DAPI ThemeSetProgressControlColor(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in DWORD dwColorIndex
    );

HRESULT DAPI ThemeSetTextControl(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in_z LPCWSTR wzText
    );

HRESULT DAPI ThemeGetTextControl(
    __in const THEME* pTheme,
    __in DWORD dwControl,
    __out LPWSTR* psczText
    );

#ifdef __cplusplus
}
#endif

