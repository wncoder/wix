//-------------------------------------------------------------------------------------------------
// <copyright file="timeutil.cpp" company="Microsoft">
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

#include "precomp.h"

const DWORD THEME_INVALID_ID = 0xFFFFFFFF;
const COLORREF THEME_INVISIBLE_COLORREF = 0xFFFFFFFF;
const DWORD GROW_WINDOW_TEXT = 250;
const LPCWSTR THEME_WC_HYPERLINK = L"ThemeHyperLink";

static Gdiplus::GdiplusStartupInput vgsi;
static Gdiplus::GdiplusStartupOutput vgso = { };
static ULONG_PTR vgdiToken = 0;
static ULONG_PTR vgdiHookToken = 0;
static HMODULE vhHyperlinkRegisteredModule = NULL;
static HMODULE vhModuleRichEd = NULL;

// prototypes
static HRESULT ParseTheme(
    __in_opt HMODULE hModule,
    __in_opt LPCWSTR wzRelativePath,
    __in IXMLDOMDocument* pixd,
    __out THEME** ppTheme
    );
static HRESULT ParseApplication(
    __in_opt HMODULE hModule,
    __in_opt LPCWSTR wzRelativePath,
    __in IXMLDOMElement* pElement,
    __in THEME* pTheme
    );
static HRESULT ParseFonts(
    __in IXMLDOMElement* pElement,
    __in THEME* pTheme
    );
static HRESULT ParsePages(
    __in IXMLDOMElement* pElement,
    __in THEME* pTheme
    );
static HRESULT ParseControls(
    __in IXMLDOMNode* pElement,
    __in THEME* pTheme,
    __in_opt THEME_PAGE* pPage
    );
static HRESULT ParseControl(
    __in IXMLDOMNode* pixn,
    __in THEME_CONTROL_TYPE type,
    __in THEME* pTheme,
    __in DWORD iControl
    );
static HRESULT ParseColumns(
    __in IXMLDOMNode* pixn,
    __in THEME_CONTROL* pControl
    );
static HRESULT ParseTabs(
    __in IXMLDOMNode* pixn,
    __in THEME_CONTROL* pControl
    );
static HRESULT DrawButton(
    __in THEME* pTheme,
    __in DRAWITEMSTRUCT* pdis,
    __in const THEME_CONTROL* pControl
    );
static HRESULT DrawHyperlink(
    __in THEME* pTheme,
    __in DRAWITEMSTRUCT* pdis,
    __in const THEME_CONTROL* pControl
    );
static HRESULT DrawImage(
    __in THEME* pTheme,
    __in DRAWITEMSTRUCT* pdis,
    __in const THEME_CONTROL* pControl
    );
static HRESULT DrawProgressBar(
    __in THEME* pTheme,
    __in DRAWITEMSTRUCT* pdis,
    __in const THEME_CONTROL* pControl
    );
static void DrawHoverControl(
    __in HWND hWnd,
    __in BOOL fHover
    );
static DWORD CALLBACK EditStreamCallback(
    __in DWORD_PTR dwCookie,
    __in LPBYTE pbBuff,
    __in LONG cb,
    __in LONG *pcb
    );
static void FreePage(
    __in THEME_PAGE* pPage
    );
static void FreeControl(
    __in THEME_CONTROL* pControl
    );
static void FreeColumn(
    __in THEME_COLUMN* pColumn
    );
static void FreeFont(
    __in THEME_FONT* pFont
    );
static void FreeTab(
    __in THEME_TAB* pTab
    );


/********************************************************************
 ThemeInitialize - initialized theme management.

*******************************************************************/
extern "C" HRESULT DAPI ThemeInitialize(
    __in HMODULE hModule
    )
{
    HRESULT hr = S_OK;
    INITCOMMONCONTROLSEX icex = { };
    WNDCLASSW wcHyperlink = { };

    hr = XmlInitialize();
    ExitOnFailure(hr, "Failed to initialize XML.");

    // Base the theme hyperlink class on a button but give it the "hand" icon.
    if (!::GetClassInfoW(NULL, WC_BUTTONW, &wcHyperlink))
    {
        ExitWithLastError(hr, "Failed to get button window class.");
    }

    wcHyperlink.lpszClassName = THEME_WC_HYPERLINK;
    wcHyperlink.hCursor = ::LoadCursor(NULL, IDC_HAND);

    if (!::RegisterClassW(&wcHyperlink))
    {
        ExitWithLastError(hr, "Failed to get button window class.");
    }
    vhHyperlinkRegisteredModule = hModule;

    vgsi.SuppressBackgroundThread = TRUE;

    Gdiplus::Status gdiStatus = Gdiplus::GdiplusStartup(&vgdiToken, &vgsi, &vgso);
    ExitOnGdipFailure(gdiStatus, hr, "Failed to initialize GDI+.");

    // Ensure that the common control DLL is loaded.
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES;
    ::InitCommonControlsEx(&icex);

    (*vgso.NotificationHook)(&vgdiHookToken);

LExit:
    return hr;
}


/********************************************************************
 ThemeUninitialize - .unitialize theme management.

*******************************************************************/
extern "C" void DAPI ThemeUninitialize()
{
    if (vhModuleRichEd)
    {
        ::FreeLibrary(vhModuleRichEd);
        vhModuleRichEd = NULL;
    }

    if (vhHyperlinkRegisteredModule)
    {
        ::UnregisterClassW(THEME_WC_HYPERLINK, vhHyperlinkRegisteredModule);
        vhHyperlinkRegisteredModule = NULL;
    }

    if (vgdiToken)
    {
        Gdiplus::GdiplusShutdown(vgdiToken);
        vgdiToken = 0;
    }

    XmlUninitialize();
}


/********************************************************************
 ThemeLoadFromFile - loads a theme from a loose file.

 *******************************************************************/
extern "C" HRESULT DAPI ThemeLoadFromFile(
    __in_z LPCWSTR wzThemeFile,
    __out THEME** ppTheme
    )
{
    HRESULT hr = S_OK;
    IXMLDOMDocument* pixd = NULL;
    LPWSTR sczRelativePath = NULL;

    hr = XmlLoadDocumentFromFile(wzThemeFile, &pixd);
    ExitOnFailure(hr, "Failed to load theme resource as XML document.");

    hr = PathGetDirectory(wzThemeFile, &sczRelativePath);
    ExitOnFailure(hr, "Failed to get relative path from theme file.");

    hr = ParseTheme(NULL, sczRelativePath, pixd, ppTheme);
    ExitOnFailure(hr, "Failed to parse theme.");

LExit:
    ReleaseStr(sczRelativePath);
    ReleaseObject(pixd);
    return hr;
}


/********************************************************************
 ThemeLoadFromResource - loads a theme from a module's data resource.

 NOTE: The resource data must be UTF-8 encoded.
*******************************************************************/
extern "C" HRESULT DAPI ThemeLoadFromResource(
    __in_opt HMODULE hModule,
    __in_z LPCSTR szResource,
    __out THEME** ppTheme
    )
{
    HRESULT hr = S_OK;
    LPVOID pvResource = NULL;
    DWORD cbResource = 0;
    LPVOID pvXml = NULL;
    DWORD cbXml = 0;
    LPWSTR sczXml = NULL;
    IXMLDOMDocument* pixd = NULL;

    hr = ResReadData(hModule, szResource, &pvResource, &cbResource);
    ExitOnFailure(hr, "Failed to read theme from resource.");

    cbXml = cbResource + sizeof(WCHAR); // allocate enough space for the resource data plus a null terminator.

    pvXml = MemAlloc(cbXml, TRUE);
    ExitOnNull(pvXml, hr, E_OUTOFMEMORY, "Failed to allocate memory to duplicate theme resource.");

    memcpy_s(pvXml, cbXml, pvResource, cbResource);

    hr = StrAllocStringAnsi(&sczXml, reinterpret_cast<LPCSTR>(pvXml), 0, CP_UTF8);
    ExitOnFailure(hr, "Failed to convert xml document data to unicode string.");

    hr = XmlLoadDocument(sczXml, &pixd);
    ExitOnFailure(hr, "Failed to load theme resource as XML document.");

    hr = ParseTheme(hModule, NULL, pixd, ppTheme);
    ExitOnFailure(hr, "Failed to parse theme.");

LExit:
    ReleaseObject(pixd);
    ReleaseMem(pvXml);
    ReleaseStr(sczXml);
    return hr;
}


/********************************************************************
 ThemeFree - frees any memory associated with a theme.

*******************************************************************/
extern "C" void DAPI ThemeFree(
    __in THEME* pTheme
    )
{
    if (pTheme)
    {
        for (DWORD i = 0; i < pTheme->cPages; ++i)
        {
            FreePage(pTheme->rgPages + i);
        }

        for (DWORD i = 0; i < pTheme->cControls; ++i)
        {
            FreeControl(pTheme->rgControls + i);
        }

        for (DWORD i = 0; i < pTheme->cFonts; ++i)
        {
            FreeFont(pTheme->rgFonts + i);
        }

        ReleaseMem(pTheme->rgControls);
        ReleaseMem(pTheme->rgPages);
        ReleaseMem(pTheme->rgFonts);

        if (pTheme->hImage)
        {
            ::DeleteBitmap(pTheme->hImage);
        }

        ReleaseStr(pTheme->wzCaption);
        ReleaseMem(pTheme);
    }
}


/********************************************************************
 ThemeLoadControls - creates the windows for all the theme controls.

*******************************************************************/
extern "C" HRESULT DAPI ThemeLoadControls(
    __in THEME* pTheme,
    __in HWND hwndParent,
    __in_ecount_opt(cAssignControlIds) THEME_ASSIGN_CONTROL_ID* rgAssignControlIds,
    __in DWORD cAssignControlIds
    )
{
    AssertSz(!pTheme->hwndParent, "Theme already loaded controls because it has a parent window.");

    HRESULT hr = S_OK;
    RECT rcParent = { };

    pTheme->hwndParent = hwndParent;

    ::GetClientRect(pTheme->hwndParent, &rcParent);

    for (DWORD i = 0; i < pTheme->cControls; ++i)
    {
        THEME_CONTROL* pControl = pTheme->rgControls + i;
        LPCWSTR wzWindowClass = NULL;
        DWORD dwWindowBits = WS_CHILD;

        switch (pControl->type)
        {
        case THEME_CONTROL_TYPE_CHECKBOX:
            dwWindowBits |= BS_AUTOCHECKBOX; // checkbox is basically a button with an extra bit tossed in.
            __fallthrough;
        case THEME_CONTROL_TYPE_BUTTON:
            wzWindowClass = WC_BUTTONW;
            if (pTheme->hImage && 0 <= pControl->nSourceX && 0 <= pControl->nSourceY)
            {
                dwWindowBits |= BS_OWNERDRAW;
            }
            break;

        case THEME_CONTROL_TYPE_LISTVIEW:
            wzWindowClass = WC_LISTVIEWW;
            break;

        case THEME_CONTROL_TYPE_TREEVIEW:
            wzWindowClass = WC_TREEVIEWW;
            break;

        case THEME_CONTROL_TYPE_TAB:
            wzWindowClass = WC_TABCONTROLW;
            break;

        case THEME_CONTROL_TYPE_EDITBOX:
            wzWindowClass = WC_EDITW;
            dwWindowBits |= WS_BORDER;
            break;

        case THEME_CONTROL_TYPE_HYPERLINK: // hyperlinks are basically just owner drawn buttons.
            wzWindowClass = THEME_WC_HYPERLINK;
            dwWindowBits |= BS_OWNERDRAW;
            break;

        case THEME_CONTROL_TYPE_IMAGE: // images are basically just owner drawn static controls (so we can draw .jpgs and .pngs instead of just bitmaps).
            if (pTheme->hImage && 0 <= pControl->nSourceX && 0 <= pControl->nSourceY)
            {
                wzWindowClass = WC_STATICW;
                dwWindowBits |= SS_OWNERDRAW;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }
            break;

        case THEME_CONTROL_TYPE_PROGRESSBAR:
            if (pTheme->hImage && 0 <= pControl->nSourceX && 0 <= pControl->nSourceY)
            {
                wzWindowClass = WC_STATICW; // no such thing as an owner drawn progress bar so we'll make our own out of a static control.
                dwWindowBits |= SS_OWNERDRAW;
            }
            else
            {
                wzWindowClass = PROGRESS_CLASSW;
            }
            break;

        case THEME_CONTROL_TYPE_RICHEDIT:
            if (NULL == vhModuleRichEd)
            {
                hr = LoadSystemLibrary(L"Riched20.dll", &vhModuleRichEd);
                ExitOnFailure(hr, "Failed to load Rich Edit control library.");
            }
            wzWindowClass = RICHEDIT_CLASSW;
            dwWindowBits |= ES_AUTOVSCROLL | ES_MULTILINE | WS_VSCROLL | ES_READONLY;
            break;

        case THEME_CONTROL_TYPE_STATIC:
            wzWindowClass = WC_STATICW;
            dwWindowBits |= WS_VISIBLE | SS_ETCHEDHORZ;
            break;

        case THEME_CONTROL_TYPE_TEXT:
            wzWindowClass = WC_STATICW;
            break;
        }
        ExitOnFailure1(hr, "Failed to configure control %u.", i);

        // If the control has a window, set the other information.
        if (wzWindowClass)
        {
            // Default control ids to the theme id and its index in the control array, unless there
            // is a specific id to assign to a named control.
            WORD wControlId = MAKEWORD(i, pTheme->wId);
            for (DWORD iAssignControl = 0; pControl->wzName && iAssignControl < cAssignControlIds; ++iAssignControl)
            {
                if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, pControl->wzName, -1, rgAssignControlIds[iAssignControl].wzName, -1))
                {
                    wControlId = rgAssignControlIds[iAssignControl].wId;
                    break;
                }
            }

            pControl->wId = wControlId;

            int w = pControl->nWidth < 1 ? pControl->nX < 0 ? rcParent.right + pControl->nWidth : rcParent.right + pControl->nWidth - pControl->nX : pControl->nWidth;
            int h = pControl->nHeight < 1 ? pControl->nY < 0 ? rcParent.bottom + pControl->nHeight : rcParent.bottom + pControl->nHeight - pControl->nY : pControl->nHeight;
            int x = pControl->nX < 0 ? rcParent.right + pControl->nX - w : pControl->nX;
            int y = pControl->nY < 0 ? rcParent.bottom + pControl->nY - h : pControl->nY;

            pControl->hWnd = ::CreateWindowW(wzWindowClass, pControl->wzText, pControl->dwStyle | dwWindowBits, x, y, w, h, pTheme->hwndParent, reinterpret_cast<HMENU>(wControlId), NULL, NULL);
            ExitOnNullWithLastError(pControl->hWnd, hr, "Failed to create window.");

            ::SetWindowLongPtrW(pControl->hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pControl));

            if (THEME_CONTROL_TYPE_LISTVIEW == pControl->type)
            {
                ::SendMessageW(pControl->hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, pControl->dwExtendedStyle);

                for (DWORD j = 0; j < pControl->cColumns; j++)
                {
                    LVCOLUMNW lvc = { };
                    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                    lvc.cx = pControl->ptcColumns[j].nWidth;
                    lvc.iSubItem = j;
                    lvc.pszText = pControl->ptcColumns[j].pszName;
                    lvc.fmt = LVCFMT_LEFT;
                    lvc.cchTextMax = 4;

                    if (-1 == ::SendMessageW(pControl->hWnd, LVM_INSERTCOLUMNW, (WPARAM)(int)(j), (LPARAM)(const LV_COLUMNW *)(&lvc)))
                    {
                        ExitWithLastError1(hr, "Failed to insert listview column %u into tab control", j);
                    }
                }
            }
            else if (THEME_CONTROL_TYPE_TAB == pControl->type)
            {
                for (DWORD j = 0; j < pControl->cTabs; j++)
                {
                    TCITEMW tci = { };
                    tci.mask = TCIF_TEXT | TCIF_IMAGE;
                    tci.iImage = -1;
                    tci.pszText = pControl->pttTabs[j].pszName;

                    if (-1 == ::SendMessageW(pControl->hWnd, TCM_INSERTITEMW, (WPARAM)(int)(j), (LPARAM)(const TC_ITEMW *)(&tci)))
                    {
                        ExitWithLastError1(hr, "Failed to insert tab %u into tab control", j);
                    }
                }
            }

            if (pTheme->cFonts > pControl->dwFontId)
            {
                THEME_FONT* pFont = pTheme->rgFonts + pControl->dwFontId;
                ::SendMessageW(pControl->hWnd, WM_SETFONT, (WPARAM)pFont->hFont, FALSE);
            }
        }
    }

LExit:
    return hr;
}


/********************************************************************
 ThemeLoadLocFromFile - Loads a wxl file and localizes strings.
 Must be called after loading a theme.

*******************************************************************/
extern "C" HRESULT DAPI ThemeLoadLocFromFile(
    __in THEME* pTheme,
    __in_z LPCWSTR wzFileName,
    __in HMODULE hModule
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczFileFullPath = NULL;
    LOC_STRINGSET* pLocStringSet = NULL;

    ExitOnNull(pTheme, hr, S_FALSE, "Theme must be loaded first.");

    hr = PathRelativeToModule(&sczFileFullPath, wzFileName, hModule);
    ExitOnFailure(hr, "Failed to create WXL file path.");

    hr = LocLoadFromFile(sczFileFullPath, &pLocStringSet);
    ExitOnFailure(hr, "Failed to load WXL file.");

    hr = LocLocalizeString(pLocStringSet, &pTheme->wzCaption);
    ExitOnFailure(hr, "Failed to localize theme caption.");

    for (DWORD i = 0; i < pTheme->cControls; ++i)
    {
        THEME_CONTROL* pControl = pTheme->rgControls + i;

        hr = LocLocalizeString(pLocStringSet, &pControl->wzText);
        ExitOnFailure(hr, "Failed to localize control text.");

        for (DWORD j = 0; j < pControl->cColumns; ++j)
        {
            hr = LocLocalizeString(pLocStringSet, &pControl->ptcColumns[j].pszName);
            ExitOnFailure(hr, "Failed to localize column text.");
        }

        for (DWORD j = 0; j < pControl->cTabs; ++j)
        {
            hr = LocLocalizeString(pLocStringSet, &pControl->pttTabs[j].pszName);
            ExitOnFailure(hr, "Failed to localize tab text.");
        }
    }

LExit:
    LocFree(pLocStringSet);
    ReleaseStr(sczFileFullPath);

    return hr;
}


/********************************************************************
 ThemeLoadStrings - Loads string resources.
 Must be called after loading a theme and before calling
 ThemeLoadControls.
*******************************************************************/
extern "C" HRESULT DAPI ThemeLoadStrings(
    __in THEME* pTheme,
    __in HMODULE hResModule
    )
{
    HRESULT hr = S_OK;
    ExitOnNull(pTheme, hr, S_FALSE, "Theme must be loaded first.");

    if (UINT_MAX != pTheme->uStringId)
    {
        hr = ResReadString(hResModule, pTheme->uStringId, &pTheme->wzCaption);
        ExitOnFailure(hr, "Failed to load theme caption.");
    }

    for (DWORD i = 0; i < pTheme->cControls; ++i)
    {
        THEME_CONTROL* pControl = pTheme->rgControls + i;

        if (UINT_MAX != pControl->uStringId)
        {
            hr = ResReadString(hResModule, pControl->uStringId, &pControl->wzText);
            ExitOnFailure(hr, "Failed to load control text.");

            for (DWORD j = 0; j < pControl->cColumns; ++j)
            {
                if (UINT_MAX != pControl->ptcColumns[j].uStringId)
                {
                    hr = ResReadString(hResModule, pControl->ptcColumns[j].uStringId, &pControl->ptcColumns[j].pszName);
                    ExitOnFailure(hr, "Failed to load column text.");
                }
            }

            for (DWORD j = 0; j < pControl->cTabs; ++j)
            {
                if (UINT_MAX != pControl->pttTabs[j].uStringId)
                {
                    hr = ResReadString(hResModule, pControl->pttTabs[j].uStringId, &pControl->pttTabs[j].pszName);
                    ExitOnFailure(hr, "Failed to load tab text.");
                }
            }
        }
    }

LExit:
    return hr;
}


/********************************************************************
 ThemeLoadRichEditFromFile - Attach a richedit control to a RTF file.

 *******************************************************************/
extern "C" HRESULT DAPI ThemeLoadRichEditFromFile(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in_z LPCWSTR wzFileName,
    __in HMODULE hModule
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczFile = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HWND hWnd = ::GetDlgItem(pTheme->hwndParent, dwControl);

    hr = PathRelativeToModule(&sczFile, wzFileName, hModule);
    ExitOnFailure(hr, "Failed to create RTF path.");

    hFile = ::CreateFileW(sczFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        ExitWithLastError(hr, "Failed to open RTF file.");
    }
    else
    {
        EDITSTREAM es = { };

        es.pfnCallback = EditStreamCallback;
        es.dwCookie = reinterpret_cast<DWORD_PTR>(hFile);

        ::SendMessageW(hWnd, EM_STREAMIN, SF_RTF, reinterpret_cast<LPARAM>(&es));
        hr = es.dwError;
        ExitOnFailure(hr, "Failed to update RTF stream");
    }

LExit:
    ReleaseStr(sczFile);
    ReleaseFile(hFile);
    return hr;
}


/********************************************************************
 ThemeTranslateAccelerator - will translate the message using the active
                             accelerator table.

*******************************************************************/
extern "C" BOOL DAPI ThemeTranslateAccelerator(
    __in_opt THEME* pTheme,
    __in HWND hWnd,
    __in MSG* pMsg
    )
{
    BOOL fProcessed = FALSE;

    if (pTheme && pTheme->hActiveAcceleratorTable)
    {
        fProcessed = ::TranslateAcceleratorW(hWnd, pTheme->hActiveAcceleratorTable, pMsg);
    }

    return fProcessed;
}


/********************************************************************
 ThemeDefWindowProc - replacement for DefWindowProc() when using theme.

*******************************************************************/
extern "C" LRESULT CALLBACK ThemeDefWindowProc(
    __in_opt THEME* pTheme,
    __in HWND hWnd,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    )
{
    if (pTheme)
    {
        switch (uMsg)
        {
        case WM_NCHITTEST:
            if (pTheme->dwStyle & WS_POPUP)
            {
                return HTCAPTION; // allow pop-up windows to be moved by grabbing any non-control.
            }
            break;

        case WM_WINDOWPOSCHANGED:
            {
                //WINDOWPOS* pos = reinterpret_cast<LPWINDOWPOS>(lParam);
                //ThemeWindowPositionChanged(pTheme, pos);
            }
            break;

        case WM_DRAWITEM:
            ThemeDrawControl(pTheme, reinterpret_cast<LPDRAWITEMSTRUCT>(lParam));
            return TRUE;

        case WM_CTLCOLORSTATIC:
            {
            HBRUSH hBrush = NULL;
            if (ThemeSetControlColor(pTheme, reinterpret_cast<HDC>(wParam), reinterpret_cast<HWND>(hWnd), &hBrush))
            {
                return reinterpret_cast<LRESULT>(hBrush);
            }
            }
            break;

        case WM_SETCURSOR:
            ThemeHoverControl(pTheme, hWnd, reinterpret_cast<HWND>(wParam));
            break;

        case WM_PAINT:
            if (::GetUpdateRect(hWnd, NULL, FALSE))
            {
                PAINTSTRUCT ps;
                ::BeginPaint(hWnd, &ps);
                ThemeDrawBackground(pTheme, &ps);
                ::EndPaint(hWnd, &ps);
            }
            return 0;
        }
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}


/********************************************************************
 ThemeGetPageIds - gets the page ids for the theme via page names.

*******************************************************************/
extern "C" void DAPI ThemeGetPageIds(
    __in THEME* pTheme,
    __in_ecount(cGetPages) LPCWSTR* rgwzFindNames,
    __inout_ecount(cGetPages) DWORD* rgdwPageIds,
    __in DWORD cGetPages
    )
{
    for (DWORD i = 0; i < cGetPages; ++i)
    {
        LPCWSTR wzFindName = rgwzFindNames[i];
        for (DWORD j = 0; j < pTheme->cPages; ++j)
        {
            LPCWSTR wzPageName = pTheme->rgPages[j].wzName;
            if (wzPageName && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzPageName, -1, wzFindName, -1))
            {
                rgdwPageIds[i] = j + 1; // add one to make the page ids 1-based (so zero is invalid).
                break;
            }
        }
    }
}


/********************************************************************
 ThemeShowPage - shows or hides all of the controls in the page at one time.

 *******************************************************************/
extern "C" void DAPI ThemeShowPage(
    __in THEME* pTheme,
    __in DWORD dwPage,
    __in int nCmdShow
    )
{
    DWORD iPage = dwPage - 1;
    if (iPage < pTheme->cPages)
    {
        THEME_PAGE* pPage = pTheme->rgPages + iPage;
        for (DWORD i = 0; i < pPage->cControlIndices; ++i)
        {
            THEME_CONTROL* pControl = pTheme->rgControls + pPage->rgdwControlIndices[i];
            HWND hWnd = pControl->hWnd;

            if (pControl->fHideWhenDisabled && !::IsWindowEnabled(hWnd))
            {
                ::ShowWindow(hWnd, SW_HIDE);
            }
            else
            {
                ::ShowWindow(hWnd, nCmdShow);
            }
        }
    }
}


/********************************************************************
 ThemeControlExists - check if a control with the specified id exists.

 *******************************************************************/
extern "C" BOOL DAPI ThemeControlExists(
    __in THEME* pTheme,
    __in DWORD dwControl
    )
{
    BOOL fExists = FALSE;
    HWND hWnd = ::GetDlgItem(pTheme->hwndParent, dwControl);
    if (hWnd)
    {
        THEME_CONTROL* pControl = reinterpret_cast<THEME_CONTROL*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        fExists = (hWnd == pControl->hWnd);
    }

    return fExists;
}


/********************************************************************
 ThemeControlEnable - enables/disables a control.

 *******************************************************************/
extern "C" void DAPI ThemeControlEnable(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in BOOL fEnable
    )
{
    HWND hWnd = ::GetDlgItem(pTheme->hwndParent, dwControl);
    ::EnableWindow(hWnd, fEnable);
}


/********************************************************************
 ThemeShowControl - shows/hides a control.

 *******************************************************************/
extern "C" void DAPI ThemeShowControl(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in int nCmdShow
    )
{
    HWND hWnd = ::GetDlgItem(pTheme->hwndParent, dwControl);
    ::ShowWindow(hWnd, nCmdShow);
}


extern "C" BOOL DAPI ThemePostControlMessage(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in UINT Msg,
    __in WPARAM wParam,
    __in LPARAM lParam
    )
{
    HRESULT hr = S_OK;
    HWND hWnd = ::GetDlgItem(pTheme->hwndParent, dwControl);

    if (!::PostMessageW(hWnd, Msg, wParam, lParam))
    {
        hr = ::GetLastError();
        hr = HRESULT_FROM_WIN32(hr);
    }

    return hr;
}


extern "C" LRESULT DAPI ThemeSendControlMessage(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in UINT Msg,
    __in WPARAM wParam,
    __in LPARAM lParam
    )
{
    HWND hWnd = ::GetDlgItem(pTheme->hwndParent, dwControl);
    return ::SendMessageW(hWnd, Msg, wParam, lParam);
}


/********************************************************************
 ThemeDrawBackground - draws the theme background.

*******************************************************************/
extern "C" HRESULT DAPI ThemeDrawBackground(
    __in THEME* pTheme,
    __in PAINTSTRUCT* pps
    )
{
    HRESULT hr = S_FALSE;

    if (pTheme->hImage && 0 <= pTheme->nSourceX && 0 <= pTheme->nSourceY && pps->fErase)
    {
        HDC hdcMem = ::CreateCompatibleDC(pps->hdc);
        HBITMAP hDefaultBitmap = static_cast<HBITMAP>(::SelectObject(hdcMem, pTheme->hImage));

        ::StretchBlt(pps->hdc, 0, 0, pTheme->nWidth, pTheme->nHeight, hdcMem, pTheme->nSourceX, pTheme->nSourceY, pTheme->nWidth, pTheme->nHeight, SRCCOPY);

        ::SelectObject(hdcMem, hDefaultBitmap);
        ::DeleteDC(hdcMem);

        hr = S_OK;
    }

    return hr;
}


/********************************************************************
 ThemeDrawControl - draw an owner drawn control.

*******************************************************************/
extern "C" HRESULT DAPI ThemeDrawControl(
    __in THEME* pTheme,
    __in DRAWITEMSTRUCT* pdis
    )
{
    HRESULT hr = S_OK;
    const THEME_CONTROL* pControl = reinterpret_cast<THEME_CONTROL*>(::GetWindowLongPtrW(pdis->hwndItem, GWLP_USERDATA));

    AssertSz(pControl->hWnd == pdis->hwndItem, "Expected control window to match owner draw window.");
    AssertSz(pControl->nWidth < 1 || pControl->nWidth == pdis->rcItem.right - pdis->rcItem.left, "Expected control window width to match owner draw window width.");
    AssertSz(pControl->nHeight < 1 || pControl->nHeight == pdis->rcItem.bottom - pdis->rcItem.top, "Expected control window height to match owner draw window height.");

    switch (pControl->type)
    {
    case THEME_CONTROL_TYPE_BUTTON:
        hr = DrawButton(pTheme, pdis, pControl);
        ExitOnFailure(hr, "Failed to draw button");
        break;

    case THEME_CONTROL_TYPE_HYPERLINK:
        hr = DrawHyperlink(pTheme, pdis, pControl);
        ExitOnFailure(hr, "Failed to draw hyperlink.");
        break;

    case THEME_CONTROL_TYPE_IMAGE:
        hr = DrawImage(pTheme, pdis, pControl);
        ExitOnFailure(hr, "Failed to draw image.");
        break;

    case THEME_CONTROL_TYPE_PROGRESSBAR:
        hr = DrawProgressBar(pTheme, pdis, pControl);
        ExitOnFailure(hr, "Failed to draw progress bar.");
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnRootFailure(hr, "Did not specify an owner draw control to draw.");
    }

LExit:
    return hr;
}


/********************************************************************
 ThemeHoverControl - mark a control as hover.

*******************************************************************/
extern "C" void DAPI ThemeHoverControl(
    __in THEME* pTheme,
    __in HWND hwndParent,
    __in HWND hwndControl
    )
{
    if (hwndControl != pTheme->hwndHover)
    {
        if (pTheme->hwndHover && pTheme->hwndHover != hwndParent)
        {
            DrawHoverControl(pTheme->hwndHover, FALSE);
        }

        pTheme->hwndHover = hwndControl;

        if (pTheme->hwndHover && pTheme->hwndHover != hwndParent)
        {
            DrawHoverControl(pTheme->hwndHover, TRUE);
        }
    }
}


/********************************************************************
 ThemeIsControlChecked - gets whether a control is checked. Only
                         really useful for checkbox controls.

*******************************************************************/
extern "C" BOOL DAPI ThemeIsControlChecked(
    __in THEME* pTheme,
    __in DWORD dwControl
    )
{
    HWND hWnd = ::GetDlgItem(pTheme->hwndParent, dwControl);
    return BST_CHECKED == ::SendMessageW(hWnd, BM_GETCHECK, 0, 0);
}


/********************************************************************
 ThemeSetControlColor - sets the color of text for a control.

*******************************************************************/
extern "C" BOOL DAPI ThemeSetControlColor(
    __in THEME* pTheme,
    __in HDC hdc,
    __in HWND hWnd,
    __out HBRUSH* phBackgroundBrush
    )
{
    THEME_FONT* pFont = NULL;
    BOOL fHasBackground = FALSE;

    if (hWnd == pTheme->hwndParent)
    {
        pFont = (THEME_INVALID_ID == pTheme->dwFontId) ? NULL : pTheme->rgFonts + pTheme->dwFontId;
    }
    else
    {
        const THEME_CONTROL* pControl = reinterpret_cast<THEME_CONTROL*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        pFont = (THEME_INVALID_ID == pControl->dwFontId) ? NULL : pTheme->rgFonts + pControl->dwFontId;
    }

    if (pFont)
    {
        if (pFont->hForeground)
        {
            ::SetTextColor(hdc, pFont->crForeground);
        }

        if (pFont->hBackground)
        {
            ::SetBkColor(hdc, pFont->crBackground);

            *phBackgroundBrush = pFont->hBackground;
            fHasBackground = TRUE;
        }
    }

    return fHasBackground;
}


/********************************************************************
 ThemeSetProgressControl - sets the current percentage complete in a
                           progress bar control.

*******************************************************************/
extern "C" HRESULT DAPI ThemeSetProgressControl(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in DWORD dwProgressPercentage
    )
{
    HRESULT hr = S_FALSE;
    HWND hWnd = ::GetDlgItem(pTheme->hwndParent, dwControl);

    if (hWnd)
    {
        THEME_CONTROL* pControl = reinterpret_cast<THEME_CONTROL*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        DWORD dwCurrentProgress = LOWORD(pControl->dwData);

        if (dwCurrentProgress != dwProgressPercentage)
        {
            DWORD dwColor = HIWORD(pControl->dwData);
            pControl->dwData = MAKEDWORD(dwProgressPercentage, dwColor);

            if (pTheme->hImage && 0 <= pControl->nSourceX && 0 <= pControl->nSourceY)
            {
                if (!::InvalidateRect(hWnd, NULL, FALSE))
                {
                    ExitWithLastError(hr, "Failed to invalidate progress bar window.");
                }
            }
            else
            {
                ::SendMessageW(hWnd, PBM_SETPOS, dwProgressPercentage, 0);
            }

            hr = S_OK;
        }
    }

LExit:
    return hr;
}


/********************************************************************
 ThemeSetProgressControlColor - sets the current color of a
                                progress bar control.

*******************************************************************/
extern "C" HRESULT DAPI ThemeSetProgressControlColor(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in DWORD dwColorIndex
    )
{
    HRESULT hr = S_FALSE;
    HWND hWnd = ::GetDlgItem(pTheme->hwndParent, dwControl);
    if (hWnd)
    {
        THEME_CONTROL* pControl = reinterpret_cast<THEME_CONTROL*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));

        // Only set color on owner draw progress bars.
        if (pTheme->hImage && 0 <= pControl->nSourceX && 0 <= pControl->nSourceY)
        {
            DWORD dwCurrentColor = HIWORD(pControl->dwData);

            if (dwCurrentColor != dwColorIndex)
            {
                DWORD dwCurrentProgress =  LOWORD(pControl->dwData);
                pControl->dwData = MAKEDWORD(dwCurrentProgress, dwColorIndex);

                if (!::InvalidateRect(hWnd, NULL, FALSE))
                {
                    ExitWithLastError(hr, "Failed to invalidate progress bar window.");
                }

                hr = S_OK;
            }
        }
    }

LExit:
    return hr;
}


/********************************************************************
 ThemeSetTextControl - sets the text of a control.

*******************************************************************/
extern "C" HRESULT DAPI ThemeSetTextControl(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in_z LPCWSTR wzText
    )
{
    HRESULT hr = S_OK;
    HWND hWnd = ::GetDlgItem(pTheme->hwndParent, dwControl);

    if (hWnd && !::SetWindowTextW(hWnd, wzText))
    {
        ExitWithLastError(hr, "Failed to set control text.");
    }

LExit:
    return hr;
}


/********************************************************************
 ThemeGetTextControl - gets the text of a control.

*******************************************************************/
extern "C" HRESULT DAPI ThemeGetTextControl(
    __in const THEME* pTheme,
    __in DWORD dwControl,
    __out LPWSTR* psczText
    )
{
    HRESULT hr = S_OK;
    HWND hWnd = ::GetDlgItem(pTheme->hwndParent, dwControl);
    DWORD cchText = 0;
    DWORD cchTextRead = 0;

    // Ensure the string has room for at least one character.
    hr = StrMaxLength(psczText, reinterpret_cast<DWORD_PTR*>(&cchText));
    ExitOnFailure(hr, "Failed to get text buffer length.");

    if (0 == cchText)
    {
        cchText = GROW_WINDOW_TEXT;

        hr = StrAlloc(psczText, cchText);
        ExitOnFailure(hr, "Failed to grow text buffer.");
    }

    // Read (and keep growing buffer) until we finally read less than there
    // is room in the buffer.
    for (;;)
    {
        cchTextRead = ::GetWindowTextW(hWnd, *psczText, cchText);
        if (cchTextRead < cchText)
        {
            break;
        }
        else
        {
            cchText = cchTextRead + GROW_WINDOW_TEXT;

            hr = StrAlloc(psczText, cchText);
            ExitOnFailure(hr, "Failed to grow text buffer again.");
        }
    }

LExit:
    return hr;
}


// Internal functions.

static HRESULT ParseTheme(
    __in_opt HMODULE hModule,
    __in_opt LPCWSTR wzRelativePath,
    __in IXMLDOMDocument* pixd,
    __out THEME** ppTheme
    )
{
    static WORD wThemeId = 0;

    HRESULT hr = S_OK;
    THEME* pTheme = NULL;
    IXMLDOMElement *pThemeElement = NULL;
    Gdiplus::Bitmap* pBitmap = NULL;
    BSTR bstr = NULL;
    LPSTR pszId = NULL;
    LPWSTR sczImageFile = NULL;

    hr = pixd->get_documentElement(&pThemeElement);
    ExitOnFailure(hr, "Failed to get theme element.");

    pTheme = static_cast<THEME*>(MemAlloc(sizeof(THEME), TRUE));
    ExitOnNull(pTheme, hr, E_OUTOFMEMORY, "Failed to allocate memory for theme.");

    pTheme->wId = ++wThemeId;

    // Parse the optional background resource image.
    hr = XmlGetAttribute(pThemeElement, L"IconResource", &bstr);
    if (S_FALSE == hr)
    {
        hr = XmlGetAttribute(pThemeElement, L"i", &bstr);
    }
    ExitOnFailure(hr, "Failed to get theme image (t@i) attribute.");

    if (S_OK == hr)
    {
        hr = StrAnsiAllocString(&pszId, bstr, 0, CP_UTF8);
        ExitOnFailure(hr, "Failed to convert theme image attribute to ANSI.");

        ReleaseNullBSTR(bstr);

        hr = GdipBitmapFromResource(hModule, pszId, &pBitmap);
        //// don't fail
    }

    // Parse the optional background image from a given file.
    if (!pBitmap)
    {
        hr = XmlGetAttribute(pThemeElement, L"ImageFile", &bstr);
        if (S_FALSE == hr)
        {
            hr = XmlGetAttribute(pThemeElement, L"if", &bstr);
        }
        ExitOnFailure(hr, "Failed to get theme image (t@if) attribute.");

        if (S_OK == hr)
        {
            if (wzRelativePath)
            {
                hr = PathConcat(wzRelativePath, bstr, &sczImageFile);
                ExitOnFailure(hr, "Failed to combine image file path.");
            }
            else
            {
                hr = PathRelativeToModule(&sczImageFile, bstr, hModule);
                ExitOnFailure(hr, "Failed to get image filename.");
            }
            ReleaseNullBSTR(bstr);

            hr = GdipBitmapFromFile(sczImageFile, &pBitmap);
            // don't fail
        }
    }

    // If there is an image, convert it into a bitmap handle.
    if (pBitmap)
    {
        Gdiplus::Color black;
        Gdiplus::Status gs = pBitmap->GetHBITMAP(black, &pTheme->hImage);
        ExitOnGdipFailure(gs, hr, "Failed to convert GDI+ bitmap into HBITMAP.");
    }

    // Parse the application element
    hr = ParseApplication(hModule, wzRelativePath, pThemeElement, pTheme);
    ExitOnFailure(hr, "Failed to parse theme application element.");

    // Parse the optional window style.
    hr = XmlGetAttributeNumberBase(pThemeElement, L"HexStyle", 16, &pTheme->dwStyle);
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumberBase(pThemeElement, L"s", 16, &pTheme->dwStyle);
    }
    ExitOnFailure(hr, "Failed to get theme window style (t@s) attribute.");

    if (S_FALSE == hr)
    {
        pTheme->dwStyle = WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU;
        pTheme->dwStyle |= (0 <= pTheme->nSourceX && 0 <= pTheme->nSourceY) ? WS_POPUP : WS_OVERLAPPED;
    }

    // Parse the fonts.
    hr = ParseFonts(pThemeElement, pTheme);
    ExitOnFailure(hr, "Failed to parse theme fonts.");

    // Parse the pages.
    hr = ParsePages(pThemeElement, pTheme);
    ExitOnFailure(hr, "Failed to parse theme pages.");

    // Parse the non-paged controls.
    hr = ParseControls(pThemeElement, pTheme, NULL);
    ExitOnFailure(hr, "Failed to parse theme controls.");

    *ppTheme = pTheme;
    pTheme = NULL;

LExit:
    ReleaseStr(sczImageFile);
    ReleaseStr(pszId);
    ReleaseBSTR(bstr);
    ReleaseObject(pThemeElement);

    if (pBitmap)
    {
        delete pBitmap;
    }

    if (pTheme)
    {
        ThemeFree(pTheme);
    }

    return hr;
}


static HRESULT ParseApplication(
    __in_opt HMODULE hModule,
    __in_opt LPCWSTR wzRelativePath,
    __in IXMLDOMElement* pElement,
    __in THEME* pTheme
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNode* pixn = NULL;
    BSTR bstr = NULL;
    LPWSTR sczIconFile = NULL;

    hr = XmlSelectSingleNode(pElement, L"Application|App|a", &pixn);
    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    ExitOnFailure(hr, "Failed to find application element.");

    hr = XmlGetAttributeNumber(pixn, L"Width", reinterpret_cast<DWORD*>(&pTheme->nWidth));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"w", reinterpret_cast<DWORD*>(&pTheme->nWidth));
        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            ExitOnRootFailure(hr, "Failed to find application width attribute.");
        }
    }
    ExitOnFailure(hr, "Failed to get application width attribute.");

    hr = XmlGetAttributeNumber(pixn, L"Height", reinterpret_cast<DWORD*>(&pTheme->nHeight));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"h", reinterpret_cast<DWORD*>(&pTheme->nHeight));
        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            ExitOnFailure(hr, "Failed to find application height attribute.");
        }
    }
    ExitOnFailure(hr, "Failed to get application height attribute.");

    hr = XmlGetAttributeNumber(pixn, L"FontId", &pTheme->dwFontId);
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"f", &pTheme->dwFontId);
        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            ExitOnRootFailure(hr, "Failed to find application font attribute.");
        }
    }
    ExitOnFailure(hr, "Failed to get application font attribute.");

    // Get the optional application icon from a resource.
    hr = XmlGetAttribute(pixn, L"IconResource", &bstr);
    if (S_FALSE == hr)
    {
        hr = XmlGetAttribute(pixn, L"i", &bstr);
    }
    ExitOnFailure(hr, "Failed to get application icon resource attribute.");

    if (S_OK == hr)
    {
        pTheme->hIcon = ::LoadIconW(hModule, bstr);
        ExitOnNullWithLastError(pTheme->hIcon, hr, "Failed to load application icon.");

        ReleaseNullBSTR(bstr);
    }

    // Get the optional application icon from a file.
    hr = XmlGetAttribute(pixn, L"IconFile", &bstr);
    if (S_FALSE == hr)
    {
        hr = XmlGetAttribute(pixn, L"if", &bstr);
    }
    ExitOnFailure(hr, "Failed to get application icon file attribute.");

    if (S_OK == hr)
    {
        if (wzRelativePath)
        {
            hr = PathConcat(wzRelativePath, bstr, &sczIconFile);
            ExitOnFailure(hr, "Failed to combine icon file path.");
        }
        else
        {
            hr = PathRelativeToModule(&sczIconFile, bstr, hModule);
            ExitOnFailure(hr, "Failed to get icon filename.");
        }
        ReleaseNullBSTR(bstr);

        pTheme->hIcon = ::LoadImageW(NULL, sczIconFile, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
        ExitOnNullWithLastError1(pTheme->hIcon, hr, "Failed to load application icon: %ls.", bstr);
    }

    hr = XmlGetAttributeNumber(pixn, L"SourceX", reinterpret_cast<DWORD*>(&pTheme->nSourceX));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"sx", reinterpret_cast<DWORD*>(&pTheme->nSourceX));
        if (S_FALSE == hr)
        {
            pTheme->nSourceX = -1;
        }
    }
    ExitOnFailure(hr, "Failed to get application source X attribute.");

    hr = XmlGetAttributeNumber(pixn, L"SourceY", reinterpret_cast<DWORD*>(&pTheme->nSourceY));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"sy", reinterpret_cast<DWORD*>(&pTheme->nSourceY));
        if (S_FALSE == hr)
        {
            pTheme->nSourceY = -1;
        }
    }
    ExitOnFailure(hr, "Failed to get application source Y attribute.");

    hr = XmlGetAttributeNumber(pixn, L"StringId", reinterpret_cast<DWORD*>(&pTheme->uStringId));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"sid", reinterpret_cast<DWORD*>(&pTheme->uStringId));
    }
    ExitOnFailure(hr, "Failed to get application caption id attribute.");

    if (S_FALSE == hr)
    {
        pTheme->uStringId = UINT_MAX;

        hr = XmlGetText(pixn, &bstr);
        ExitOnFailure(hr, "Failed to get application caption.");

        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            ExitOnRootFailure(hr, "Failed to find application caption.");
        }

        hr = StrAllocString(&pTheme->wzCaption, bstr, 0);
        ExitOnFailure(hr, "Failed to copy application caption.");
    }

LExit:
    ReleaseStr(sczIconFile);
    ReleaseBSTR(bstr);
    ReleaseObject(pixn);
    return hr;
}


static HRESULT ParseFonts(
    __in IXMLDOMElement* pElement,
    __in THEME* pTheme
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnl = NULL;
    IXMLDOMNode* pixn = NULL;
    BSTR bstrName = NULL;
    DWORD dwId = 0;
    LOGFONTW lf = { };
    LOGBRUSH lb = { };
    COLORREF crForeground = THEME_INVISIBLE_COLORREF;
    COLORREF crBackground = THEME_INVISIBLE_COLORREF;

    hr = XmlSelectNodes(pElement, L"Font|f", &pixnl);
    if (S_FALSE == hr)
    {
        hr = S_OK;
        ExitFunction();
    }
    ExitOnFailure(hr, "Failed to find font elements.");

    hr = pixnl->get_length(reinterpret_cast<long*>(&pTheme->cFonts));
    ExitOnFailure(hr, "Failed to count the number of theme fonts.");

    pTheme->rgFonts = static_cast<THEME_FONT*>(MemAlloc(sizeof(THEME_FONT) * pTheme->cFonts, TRUE));
    ExitOnNull(pTheme->rgFonts, hr, E_OUTOFMEMORY, "Failed to allocate theme fonts.");

    lf.lfQuality = ANTIALIASED_QUALITY;
    lb.lbStyle = BS_SOLID;

    while (S_OK == (hr = XmlNextElement(pixnl, &pixn, NULL)))
    {
        hr = XmlGetAttributeNumber(pixn, L"id", &dwId);
        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
        ExitOnFailure(hr, "Failed to find font foreground color.");

        if (pTheme->cFonts <= dwId)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            ExitOnRootFailure(hr, "Invalid theme font id.");
        }

        hr = XmlGetText(pixn, &bstrName);
        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
        ExitOnFailure(hr, "Failed to get application caption.");

        hr = ::StringCchCopyW(lf.lfFaceName, countof(lf.lfFaceName), bstrName);
        ExitOnFailure(hr, "Failed to copy font name.");

        hr = XmlGetAttributeNumber(pixn, L"Height", reinterpret_cast<DWORD*>(&lf.lfHeight));
        if (S_FALSE == hr)
        {
            hr = XmlGetAttributeNumber(pixn, L"h", reinterpret_cast<DWORD*>(&lf.lfHeight));
            if (S_FALSE == hr)
            {
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }
        }
        ExitOnFailure(hr, "Failed to find font height attribute.");

        hr = XmlGetAttributeNumber(pixn, L"Weight", reinterpret_cast<DWORD*>(&lf.lfWeight));
        if (S_FALSE == hr)
        {
            hr = XmlGetAttributeNumber(pixn, L"w", reinterpret_cast<DWORD*>(&lf.lfWeight));
            if (S_FALSE == hr)
            {
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }
        }
        ExitOnFailure(hr, "Failed to find font weight attribute.");

        hr = XmlGetYesNoAttribute(pixn, L"Underline", reinterpret_cast<BOOL*>(&lf.lfUnderline));
        if (E_NOTFOUND == hr)
        {
            hr = XmlGetYesNoAttribute(pixn, L"u", reinterpret_cast<BOOL*>(&lf.lfUnderline));
            if (E_NOTFOUND == hr)
            {
                lf.lfUnderline = FALSE;
                hr = S_OK;
            }
        }
        ExitOnFailure(hr, "Failed to find font underline attribute.");

        hr = XmlGetAttributeNumberBase(pixn, L"Foreground", 16, &crForeground);
        if (S_FALSE == hr)
        {
            hr = XmlGetAttributeNumberBase(pixn, L"f", 16, &crForeground);
            if (S_FALSE == hr)
            {
                crForeground = THEME_INVISIBLE_COLORREF;
                hr = S_OK;
            }
        }
        ExitOnFailure(hr, "Failed to find font foreground color.");

        hr = XmlGetAttributeNumberBase(pixn, L"Background", 16, &crBackground);
        if (S_FALSE == hr)
        {
            hr = XmlGetAttributeNumberBase(pixn, L"b", 16, &crBackground);
            if (S_FALSE == hr)
            {
                crBackground = THEME_INVISIBLE_COLORREF;
                hr = S_OK;
            }
        }
        ExitOnFailure(hr, "Failed to find font background color.");

        THEME_FONT* pFont = pTheme->rgFonts + dwId;
        if (NULL != pFont->hFont)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            ExitOnRootFailure(hr, "Theme font id duplicated.");
        }

        pFont->hFont = ::CreateFontIndirectW(&lf);
        ExitOnNullWithLastError(pFont->hFont, hr, "Failed to create product title font.");

        pFont->crForeground = crForeground;
        if (THEME_INVISIBLE_COLORREF != pFont->crForeground)
        {
            lb.lbColor = pFont->crForeground;
            pFont->hForeground = ::CreateBrushIndirect(&lb);
            ExitOnNullWithLastError(pFont->hForeground, hr, "Failed to create text foreground brush.");
        }

        pFont->crBackground = crBackground;
        if (THEME_INVISIBLE_COLORREF != pFont->crBackground)
        {
            lb.lbColor = pFont->crBackground;
            pFont->hBackground = ::CreateBrushIndirect(&lb);
            ExitOnNullWithLastError(pFont->hBackground, hr, "Failed to create text background brush.");
        }

        ReleaseNullBSTR(bstrName);
        ReleaseNullObject(pixn);
    }
    ExitOnFailure(hr, "Failed to enumerate all fonts.");

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

LExit:
    ReleaseBSTR(bstrName);
    ReleaseObject(pixn);
    ReleaseObject(pixnl);
    return hr;
}


static HRESULT ParsePages(
    __in IXMLDOMElement* pElement,
    __in THEME* pTheme
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnl = NULL;
    IXMLDOMNode* pixn = NULL;
    BSTR bstrType = NULL;
    THEME_PAGE* pPage = NULL;
    DWORD iPage = 0;

    hr = XmlSelectNodes(pElement, L"Page", &pixnl);
    if (S_FALSE == hr)
    {
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to find page elements.");

    hr = pixnl->get_length(reinterpret_cast<long*>(&pTheme->cPages));
    ExitOnFailure(hr, "Failed to count the number of theme pages.");

    if (0 == pTheme->cPages)
    {
        ExitFunction();
    }

    pTheme->rgPages = static_cast<THEME_PAGE*>(MemAlloc(sizeof(THEME_PAGE) * pTheme->cPages, TRUE));
    ExitOnNull(pTheme->rgPages, hr, E_OUTOFMEMORY, "Failed to allocate theme pages.");

    while (S_OK == (hr = XmlNextElement(pixnl, &pixn, &bstrType)))
    {
        pPage = pTheme->rgPages + iPage;

        pPage->wId = static_cast<WORD>(iPage + 1);

        hr = XmlGetAttributeEx(pixn, L"Name", &pPage->wzName);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        ExitOnFailure(hr, "Failed when querying page Name.");

        hr = ParseControls(pixn, pTheme, pPage);
        ExitOnFailure(hr, "Failed to parse page controls.");

        ++iPage;

        ReleaseNullBSTR(bstrType);
    }

LExit:
    ReleaseBSTR(bstrType);
    return hr;
}


static HRESULT ParseControls(
    __in IXMLDOMNode* pElement,
    __in THEME* pTheme,
    __in_opt THEME_PAGE* pPage
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnl = NULL;
    IXMLDOMNode* pixn = NULL;
    BSTR bstrType = NULL;
    DWORD cNewControls = 0;
    DWORD iControl = 0;
    DWORD iPageControl = 0;

    hr = XmlSelectNodes(pElement, L"*", &pixnl);
    if (S_FALSE == hr)
    {
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to find control elements.");

    hr = pixnl->get_length(reinterpret_cast<long*>(&cNewControls));
    ExitOnFailure(hr, "Failed to count the number of theme controls.");

    // If we are creating top level controls (no page provided), subtract the font and
    // page elements and "application" element since they are all siblings and inflate
    // the count.
    if (!pPage)
    {
        cNewControls = cNewControls - pTheme->cFonts - pTheme->cPages - 1;
    }

    if (0 == cNewControls)
    {
        ExitFunction1(hr = S_OK);
    }

    if (pPage)
    {
        pPage->rgdwControlIndices = static_cast<DWORD*>(MemAlloc(sizeof(DWORD) * cNewControls, TRUE));
        ExitOnNull(pPage->rgdwControlIndices, hr, E_OUTOFMEMORY, "Failed to allocate theme page controls.");

        pPage->cControlIndices = cNewControls;
    }

    hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pTheme->rgControls), pTheme->cControls, sizeof(THEME_CONTROL), cNewControls);
    ExitOnFailure(hr, "Failed to allocate theme controls.");

    iControl = pTheme->cControls;
    pTheme->cControls += cNewControls;

    while (S_OK == (hr = XmlNextElement(pixnl, &pixn, &bstrType)))
    {
        THEME_CONTROL_TYPE type = THEME_CONTROL_TYPE_UNKNOWN;

        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Button", -1) ||
            CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"b", 1))
        {
            type = THEME_CONTROL_TYPE_BUTTON;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Checkbox", -1) ||
                 CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"cb", 2))
        {
            type = THEME_CONTROL_TYPE_CHECKBOX;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Editbox", -1) ||
                 CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"eb", 2))
        {
            type = THEME_CONTROL_TYPE_EDITBOX;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Hyperlink", -1) ||
                 CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"l", 1))
        {
            type = THEME_CONTROL_TYPE_HYPERLINK;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Image", -1) ||
                 CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"i", 1))
        {
            type = THEME_CONTROL_TYPE_IMAGE;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Progressbar", -1) ||
                 CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"pb", 2))
        {
            type = THEME_CONTROL_TYPE_PROGRESSBAR;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Richedit", -1) ||
                 CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"rt", 2))
        {
            type = THEME_CONTROL_TYPE_RICHEDIT;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Static", -1) ||
                 CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"s", 1))
        {
            type = THEME_CONTROL_TYPE_STATIC;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Text", -1) ||
                 CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"t", 1))
        {
            type = THEME_CONTROL_TYPE_TEXT;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Listview", -1) ||
                 CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"lv", 2))
        {
            type = THEME_CONTROL_TYPE_LISTVIEW;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Treeview", -1) ||
                 CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"tv", 2))
        {
            type = THEME_CONTROL_TYPE_TREEVIEW;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"Tab", -1) ||
                 CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"tb", 2))
        {
            type = THEME_CONTROL_TYPE_TAB;
        }

        if (THEME_CONTROL_TYPE_UNKNOWN != type)
        {
            hr = ParseControl(pixn, type, pTheme, iControl);
            ExitOnFailure(hr, "Failed to parse control.");

            if (pPage)
            {
                pTheme->rgControls[iControl].wPageId = pPage->wId;
                pPage->rgdwControlIndices[iPageControl] = iControl;
                ++iPageControl;
            }

            ++iControl;
        }

        ReleaseNullBSTR(bstrType);
        ReleaseNullObject(pixn);
    }
    ExitOnFailure(hr, "Failed to enumerate all controls.");

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

LExit:
    ReleaseBSTR(bstrType);
    ReleaseObject(pixn);
    ReleaseObject(pixnl);
    return hr;
}


static HRESULT ParseControl(
    __in IXMLDOMNode* pixn,
    __in THEME_CONTROL_TYPE type,
    __in THEME* pTheme,
    __in DWORD iControl
    )
{
    HRESULT hr = S_OK;
    DWORD dwId = 0;
    THEME_CONTROL* pControl = NULL;
    DWORD dwValue = 0;
    BOOL fValue = FALSE;
    BSTR bstrText = NULL;

    hr = XmlGetAttributeNumber(pixn, L"id", &dwId);
    if (S_FALSE == hr)
    {
        dwId = iControl;
    }
    ExitOnFailure(hr, "Failed to find control id.");

    if (pTheme->cControls <= dwId)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnRootFailure(hr, "Invalid theme control id.");
    }

    pControl = pTheme->rgControls + dwId;
    if (THEME_CONTROL_TYPE_UNKNOWN != pControl->type)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnRootFailure(hr, "Theme control id duplicated.");
    }

    pControl->type = type;

    hr = XmlGetAttributeEx(pixn, L"Name", &pControl->wzName);
    if (E_NOTFOUND == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed when querying control Name.");

    hr = XmlGetAttributeNumber(pixn, L"X", reinterpret_cast<DWORD*>(&pControl->nX));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"x", reinterpret_cast<DWORD*>(&pControl->nX));
        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
    }
    ExitOnFailure(hr, "Failed to find control X attribute.");

    hr = XmlGetAttributeNumber(pixn, L"Y", reinterpret_cast<DWORD*>(&pControl->nY));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"y", reinterpret_cast<DWORD*>(&pControl->nY));
        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
    }
    ExitOnFailure(hr, "Failed to find control Y attribute.");

    hr = XmlGetAttributeNumber(pixn, L"Height", reinterpret_cast<DWORD*>(&pControl->nHeight));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"h", reinterpret_cast<DWORD*>(&pControl->nHeight));
        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
    }
    ExitOnFailure(hr, "Failed to find control height attribute.");

    hr = XmlGetAttributeNumber(pixn, L"Width", reinterpret_cast<DWORD*>(&pControl->nWidth));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"w", reinterpret_cast<DWORD*>(&pControl->nWidth));
        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
    }
    ExitOnFailure(hr, "Failed to find control weight attribute.");

    hr = XmlGetAttributeNumber(pixn, L"SourceX", reinterpret_cast<DWORD*>(&pControl->nSourceX));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"sx", reinterpret_cast<DWORD*>(&pControl->nSourceX));
        if (S_FALSE == hr)
        {
            pControl->nSourceX = -1;
        }
    }
    ExitOnFailure(hr, "Failed to find control source X attribute.");

    hr = XmlGetAttributeNumber(pixn, L"SourceY", reinterpret_cast<DWORD*>(&pControl->nSourceY));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"sy", reinterpret_cast<DWORD*>(&pControl->nSourceY));
        if (S_FALSE == hr)
        {
            pControl->nSourceY = -1;
        }
    }
    ExitOnFailure(hr, "Failed to find control source Y attribute.");

    hr = XmlGetAttributeNumber(pixn, L"FontId", &pControl->dwFontId);
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"f", &pControl->dwFontId);
        if (S_FALSE == hr)
        {
            pControl->dwFontId = THEME_INVALID_ID;
        }
    }
    ExitOnFailure(hr, "Failed to find font for control.");

    hr = XmlGetAttributeNumber(pixn, L"HoverFontId", &pControl->dwFontHoverId);
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"fh", &pControl->dwFontHoverId);
        if (S_FALSE == hr)
        {
            pControl->dwFontHoverId = THEME_INVALID_ID;
        }
    }
    ExitOnFailure(hr, "Failed to find hover font for control.");

    hr = XmlGetAttributeNumber(pixn, L"SelectedFontId", &pControl->dwFontSelectedId);
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"fs", &pControl->dwFontSelectedId);
        if (S_FALSE == hr)
        {
            pControl->dwFontSelectedId = THEME_INVALID_ID;
        }
    }
    ExitOnFailure(hr, "Failed to find selected font for control.");

    // Parse the optional window style.
    hr = XmlGetAttributeNumberBase(pixn, L"HexStyle", 16, &pControl->dwStyle);
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumberBase(pixn, L"s", 16, &pControl->dwStyle);
        ExitOnFailure(hr, "Failed to get control window style (@s) attribute.");
    }

    // Parse the tabstop bit "shortcut nomenclature", this could have been set with the style above.
    hr = XmlGetAttributeNumber(pixn, L"TabStop", &dwValue);
    if (S_OK == hr && dwValue)
    {
        hr = XmlGetAttributeNumber(pixn, L"t", &dwValue);
        if (S_OK == hr && dwValue)
        {
            pControl->dwStyle |= WS_TABSTOP;
        }
    }
    ExitOnFailure(hr, "Failed to tell if the control is a tab stop.");

    hr = XmlGetYesNoAttribute(pixn, L"Visible", &fValue);
    if (E_NOTFOUND == hr)
    {
        hr = S_OK;
    }
    else if (fValue)
    {
        pControl->dwStyle |= WS_VISIBLE;
    }
    ExitOnFailure(hr, "Failed to tell if the control is visible.");

    hr = XmlGetYesNoAttribute(pixn, L"HideWhenDisabled", &pControl->fHideWhenDisabled);
    if (E_NOTFOUND != hr)
    {
        ExitOnFailure(hr, "Failed to parse if the control should be hidden when disabled.");
    }

    hr = XmlGetAttributeNumber(pixn, L"StringId", reinterpret_cast<DWORD*>(&pControl->uStringId));
    if (S_FALSE == hr)
    {
        hr = XmlGetAttributeNumber(pixn, L"sid", reinterpret_cast<DWORD*>(&pControl->uStringId));
    }
    ExitOnFailure(hr, "Failed to get control text id attribute.");

    if (S_FALSE == hr)
    {
        pControl->uStringId = UINT_MAX;

        hr = XmlGetText(pixn, &bstrText);
        ExitOnFailure(hr, "Failed to get control text.");

        if (S_OK == hr)
        {
            hr = StrAllocString(&pControl->wzText, bstrText, 0);
            ExitOnFailure(hr, "Failed to copy control text.");
        }
        else if (S_FALSE == hr)
        {
            hr = S_OK;
        }
    }

    if (THEME_CONTROL_TYPE_LISTVIEW == type)
    {
        // Parse the optional extended window style.
        hr = XmlGetAttributeNumberBase(pixn, L"HexExtendedStyle", 16, &pControl->dwExtendedStyle);
        if (S_FALSE == hr)
        {
            hr = XmlGetAttributeNumberBase(pixn, L"xs", 16, &pControl->dwExtendedStyle);
        }
        ExitOnFailure(hr, "Failed to get theme window extended style (t@xs) attribute.");

        hr = ParseColumns(pixn, pControl);
        ExitOnFailure(hr, "Failed to parse columns");
    }
    else if (THEME_CONTROL_TYPE_TREEVIEW == type)
    {
        pControl->dwStyle |= TVS_DISABLEDRAGDROP;

        hr = XmlGetYesNoAttribute(pixn, L"EnableDragDrop", &fValue);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        else if (fValue)
        {
            pControl->dwStyle |= ~TVS_DISABLEDRAGDROP;
        }
        ExitOnFailure(hr, "Failed to tell if the tree control control enables drag and drop..");

        hr = XmlGetYesNoAttribute(pixn, L"FullRowSelect", &fValue);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        else if (fValue)
        {
            pControl->dwStyle |= TVS_FULLROWSELECT;
        }
        ExitOnFailure(hr, "Failed to tell if the tree control enables full row select.");

        hr = XmlGetYesNoAttribute(pixn, L"HasButtons", &fValue);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        else if (fValue)
        {
            pControl->dwStyle |= TVS_HASBUTTONS;
        }
        ExitOnFailure(hr, "Failed to tell if the tree control show buttons.");

        hr = XmlGetYesNoAttribute(pixn, L"AlwaysShowSelect", &fValue);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        else if (fValue)
        {
            pControl->dwStyle |= TVS_SHOWSELALWAYS;
        }
        ExitOnFailure(hr, "Failed to tell if the tree control always displays the selection.");

        hr = XmlGetYesNoAttribute(pixn, L"LinesAtRoot", &fValue);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        else if (fValue)
        {
            pControl->dwStyle |= TVS_LINESATROOT;
        }
        ExitOnFailure(hr, "Failed to tell if the tree control shows lines at the root.");

        hr = XmlGetYesNoAttribute(pixn, L"HasLines", &fValue);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        else if (fValue)
        {
            pControl->dwStyle |= TVS_HASLINES;
        }
        ExitOnFailure(hr, "Failed to tell if the tree control shows lines.");
    }
    else if (THEME_CONTROL_TYPE_TAB == type)
    {
        hr = ParseTabs(pixn, pControl);
        ExitOnFailure(hr, "Failed to parse tabs");
    }

LExit:
    ReleaseBSTR(bstrText);

    return hr;
}


static HRESULT ParseColumns(
    __in IXMLDOMNode* pixn,
    __in THEME_CONTROL* pControl
    )
{
    HRESULT hr = S_OK;
    DWORD i = 0;
    IXMLDOMNodeList* pixnl = NULL;
    IXMLDOMNode* pixnChild = NULL;
    BSTR bstrText = NULL;

    hr = XmlSelectNodes(pixn, L"Column|c", &pixnl);
    ExitOnFailure(hr, "Failed to select child column nodes");

    if (S_FALSE != hr)
    {
        hr = pixnl->get_length(reinterpret_cast<long*>(&pControl->cColumns));
        ExitOnFailure(hr, "Failed to count the number of control columns.");

        pControl->ptcColumns = static_cast<THEME_COLUMN*>(MemAlloc(sizeof(THEME_COLUMN) * pControl->cColumns, TRUE));
        ExitOnNull(pControl->ptcColumns, hr, E_OUTOFMEMORY, "Failed to allocate column structs.");

        i = 0;
        while (S_OK == (hr = XmlNextElement(pixnl, &pixnChild, NULL)))
        {
            hr = XmlGetText(pixnChild, &bstrText);
            ExitOnFailure(hr, "Failed to get inner text of column element");

            hr = XmlGetAttributeNumber(pixnChild, L"Width", reinterpret_cast<DWORD*>(&pControl->ptcColumns[i].nWidth));
            if (S_FALSE == hr)
            {
                hr = XmlGetAttributeNumber(pixnChild, L"w", reinterpret_cast<DWORD*>(&pControl->ptcColumns[i].nWidth));
                if (S_FALSE == hr)
                {
                    pControl->ptcColumns[i].nWidth = 100; // Default to 100
                }
            }
            ExitOnFailure(hr, "Failed to get column width attribute.");

            hr = StrAllocString(&(pControl->ptcColumns[i].pszName), bstrText, 0);
            ExitOnFailure(hr, "Failed to copy column name");

            i++;
        }
    }

LExit:
    ReleaseObject(pixnl);
    ReleaseObject(pixnChild);
    ReleaseBSTR(bstrText);

    return hr;
}


static HRESULT ParseTabs(
    __in IXMLDOMNode* pixn,
    __in THEME_CONTROL* pControl
    )
{
    HRESULT hr = S_OK;
    DWORD i = 0;
    IXMLDOMNodeList* pixnl = NULL;
    IXMLDOMNode* pixnChild = NULL;
    BSTR bstrText = NULL;

    hr = XmlSelectNodes(pixn, L"Tab|t", &pixnl);
    ExitOnFailure(hr, "Failed to select child tab nodes");

    if (S_FALSE != hr)
    {
        hr = pixnl->get_length(reinterpret_cast<long*>(&pControl->cTabs));
        ExitOnFailure(hr, "Failed to count the number of tabs.");

        pControl->pttTabs = static_cast<THEME_TAB*>(MemAlloc(sizeof(THEME_TAB) * pControl->cTabs, TRUE));
        ExitOnNull(pControl->pttTabs, hr, E_OUTOFMEMORY, "Failed to allocate tab structs.");

        i = 0;
        while (S_OK == (hr = XmlNextElement(pixnl, &pixnChild, NULL)))
        {
            hr = XmlGetText(pixnChild, &bstrText);
            ExitOnFailure(hr, "Failed to get inner text of tab element");

            hr = StrAllocString(&(pControl->pttTabs[i].pszName), bstrText, 0);
            ExitOnFailure(hr, "Failed to copy tab name");

            i++;
        }
    }

LExit:
    ReleaseObject(pixnl);
    ReleaseObject(pixnChild);
    ReleaseBSTR(bstrText);

    return hr;
}


static HRESULT DrawButton(
    __in THEME* pTheme,
    __in DRAWITEMSTRUCT* pdis,
    __in const THEME_CONTROL* pControl
    )
{
    HRESULT hr = S_OK;
    DWORD_PTR dwStyle = ::GetWindowLongPtrW(pdis->hwndItem, GWL_STYLE);
    int nSourceY = pControl->nSourceY;

    HDC hdcMem = ::CreateCompatibleDC(pdis->hDC);
    HBITMAP hDefaultBitmap = static_cast<HBITMAP>(::SelectObject(hdcMem, pTheme->hImage));

    if (ODS_SELECTED & pdis->itemState)
    {
        nSourceY += pControl->nHeight * 2;
    }
    else if (pControl->dwData & THEME_CONTROL_DATA_HOVER)
    {
        nSourceY += pControl->nHeight;
    }

    ::StretchBlt(pdis->hDC, 0, 0, pControl->nWidth, pControl->nHeight, hdcMem, pControl->nSourceX, nSourceY, pControl->nWidth, pControl->nHeight, SRCCOPY);

    if (WS_TABSTOP & dwStyle && ODS_FOCUS & pdis->itemState)
    {
        ::DrawFocusRect(pdis->hDC, &pdis->rcItem);
    }

    ::SelectObject(hdcMem, hDefaultBitmap);
    ::DeleteDC(hdcMem);
    return hr;
}


static HRESULT DrawHyperlink(
    __in THEME* pTheme,
    __in DRAWITEMSTRUCT* pdis,
    __in const THEME_CONTROL* pControl
    )
{
    HRESULT hr = S_OK;
    WCHAR wzText[256] = { };
    DWORD cchText = 0;
    THEME_FONT* pFont = NULL;
    HFONT hfPrev = NULL;
    COLORREF clrForePrev;
    COLORREF clrBackPrev;

    if (0 == (cchText = ::GetWindowTextW(pdis->hwndItem, wzText, countof(wzText))))
    {
        ExitWithLastError(hr, "Failed to get text of link.");
    }

    if (ODS_SELECTED & pdis->itemState)
    {
        pFont = pTheme->rgFonts + pControl->dwFontSelectedId;
    }
    else if (pControl->dwData & THEME_CONTROL_DATA_HOVER)
    {
        pFont = pTheme->rgFonts + pControl->dwFontHoverId;
    }
    else
    {
        pFont = pTheme->rgFonts + pControl->dwFontId;
    }

    hfPrev = SelectFont(pdis->hDC, pFont->hFont);

    clrForePrev = ::SetTextColor(pdis->hDC, pFont->crForeground);
    clrBackPrev = ::SetBkColor(pdis->hDC, pFont->crBackground);

    ::ExtTextOutW(pdis->hDC, 0, 0, ETO_CLIPPED | ETO_OPAQUE, &pdis->rcItem, wzText, cchText, NULL);

    if (ODS_FOCUS & pdis->itemState)
    {
        ::DrawFocusRect(pdis->hDC, &pdis->rcItem);
    }

    ::SetBkColor(pdis->hDC, clrBackPrev);
    ::SetTextColor(pdis->hDC, clrForePrev);

    SelectFont(pdis->hDC, hfPrev);

LExit:
    return hr;
}


static HRESULT DrawImage(
    __in THEME* pTheme,
    __in DRAWITEMSTRUCT* pdis,
    __in const THEME_CONTROL* pControl
    )
{
    DWORD dwHeight = pdis->rcItem.bottom - pdis->rcItem.top;
    DWORD dwWidth = pdis->rcItem.right - pdis->rcItem.left;

    HDC hdcMem = ::CreateCompatibleDC(pdis->hDC);
    HBITMAP hDefaultBitmap = static_cast<HBITMAP>(::SelectObject(hdcMem, pTheme->hImage));

    // Draw the image.
    ::StretchBlt(pdis->hDC, 0, 0, dwWidth, dwHeight, hdcMem, pControl->nSourceX, pControl->nSourceY, dwWidth, dwHeight, SRCCOPY);

    ::SelectObject(hdcMem, hDefaultBitmap);
    ::DeleteDC(hdcMem);
    return S_OK;
}


static HRESULT DrawProgressBar(
    __in THEME* pTheme,
    __in DRAWITEMSTRUCT* pdis,
    __in const THEME_CONTROL* pControl
    )
{
    DWORD dwProgressColor = HIWORD(pControl->dwData);
    DWORD dwProgressPercentage = LOWORD(pControl->dwData);
    DWORD dwHeight = pdis->rcItem.bottom - pdis->rcItem.top;
    DWORD dwCenter = (pdis->rcItem.right - 2) * dwProgressPercentage / 100;
    int nSourceY = pControl->nSourceY + (dwProgressColor * pControl->nHeight);

    HDC hdcMem = ::CreateCompatibleDC(pdis->hDC);
    HBITMAP hDefaultBitmap = static_cast<HBITMAP>(::SelectObject(hdcMem, pTheme->hImage));

    // Draw the left side of the progress bar.
    ::StretchBlt(pdis->hDC, 0, 0, 1, dwHeight, hdcMem, pControl->nSourceX, nSourceY, 1, dwHeight, SRCCOPY);

    // Draw the filled side of the progress bar, if there is any.
    if (0 < dwCenter)
    {
        ::StretchBlt(pdis->hDC, 1, 0, dwCenter, dwHeight, hdcMem, pControl->nSourceX + 1, nSourceY, 1, dwHeight, SRCCOPY);
    }

    // Draw the unfilled side of the progress bar, if there is any.
    if (dwCenter < static_cast<DWORD>(pdis->rcItem.right - 2))
    {
        ::StretchBlt(pdis->hDC, 1 + dwCenter, 0, pdis->rcItem.right - dwCenter - 1, dwHeight, hdcMem, pControl->nSourceX + 2, nSourceY, 1, dwHeight, SRCCOPY);
    }

    // Draw the right side of the progress bar.
    ::StretchBlt(pdis->hDC, pdis->rcItem.right - 1, 0, 1, dwHeight, hdcMem, pControl->nSourceX, nSourceY, 1, dwHeight, SRCCOPY);

    ::SelectObject(hdcMem, hDefaultBitmap);
    ::DeleteDC(hdcMem);
    return S_OK;
}


static void DrawHoverControl(
    __in HWND hWnd,
    __in BOOL fHover
    )
{
    THEME_CONTROL* pControl = reinterpret_cast<THEME_CONTROL*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    AssertSz(pControl->hWnd == hWnd, "Expected control's window to be the same as the window's user data.");

    if (fHover)
    {
        pControl->dwData |= THEME_CONTROL_DATA_HOVER;
    }
    else
    {
        pControl->dwData &= ~THEME_CONTROL_DATA_HOVER;
    }

    ::InvalidateRect(pControl->hWnd, NULL, FALSE);
}


static void FreePage(
    __in THEME_PAGE* pPage
    )
{
    if (pPage)
    {
        ReleaseStr(pPage->wzName);
    }
}


static void FreeControl(
    __in THEME_CONTROL* pControl
    )
{
    if (pControl)
    {
        if (::IsWindow(pControl->hWnd))
        {
            ::CloseWindow(pControl->hWnd);
            pControl->hWnd = NULL;
        }

        ReleaseStr(pControl->wzName);
        ReleaseStr(pControl->wzText);

        for (DWORD i = 0; i < pControl->cColumns; i++)
        {
            FreeColumn(&(pControl->ptcColumns[i]));
        }

        for (DWORD i = 0; i < pControl->cTabs; i++)
        {
            FreeTab(&(pControl->pttTabs[i]));
        }

        ReleaseMem(pControl->ptcColumns);
        ReleaseMem(pControl->pttTabs);
    }
}


static void FreeColumn(
    __in THEME_COLUMN* pColumn
    )
{
    ReleaseStr(pColumn->pszName);
}


static void FreeTab(
    __in THEME_TAB* pTab
    )
{
    ReleaseStr(pTab->pszName);
}


static void FreeFont(
    __in THEME_FONT* pFont
    )
{
    if (pFont)
    {
        if (pFont->hBackground)
        {
            ::DeleteObject(pFont->hBackground);
            pFont->hBackground = NULL;
        }

        if (pFont->hForeground)
        {
            ::DeleteObject(pFont->hForeground);
            pFont->hForeground = NULL;
        }

        if (pFont->hFont)
        {
            ::DeleteObject(pFont->hFont);
            pFont->hFont = NULL;
        }
    }
}


static DWORD CALLBACK EditStreamCallback(
    __in DWORD_PTR dwCookie,
    __in LPBYTE pbBuff,
    __in LONG cb,
    __in LONG* pcb
    )
{
    HRESULT hr = S_OK;
    HANDLE hFile = reinterpret_cast<HANDLE>(dwCookie);

    if (!::ReadFile(hFile, pbBuff, cb, reinterpret_cast<DWORD*>(pcb), NULL))
    {
        ExitWithLastError(hr, "Failed to read file");
    }

LExit:
    return hr;
}
