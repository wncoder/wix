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
    __in IXMLDOMElement* pElement,
    __in THEME* pTheme
    );
static HRESULT ParseFonts(
    __in IXMLDOMElement* pElement,
    __in THEME* pTheme
    );
static HRESULT ParseControls(
    __in IXMLDOMElement* pElement,
    __in THEME* pTheme
    );
static HRESULT ParseControl(
    __in IXMLDOMNode* pixn,
    __in THEME_CONTROL_TYPE type,
    __in THEME* pTheme
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
static void FreeControl(
    __in THEME_CONTROL* pControl
    );
static void FreeFont(
    __in THEME_FONT* pFont
    );


/********************************************************************
 ThemeInitialize - initialized theme management.

*******************************************************************/
extern "C" HRESULT DAPI ThemeInitialize(
    __in HMODULE hModule
    )
{
    HRESULT hr = S_OK;
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

    (*vgso.NotificationHook)(&vgdiHookToken);

LExit:
    return hr;
}


/********************************************************************
 ThemeUninitialize - .unitialize theme management.

*******************************************************************/
void DAPI ThemeUninitialize()
{
    if (vhModuleRichEd)
    {
        ::FreeLibrary(vhModuleRichEd);
        vhModuleRichEd = NULL;
    }

    if (vhHyperlinkRegisteredModule)
    {
        ::UnregisterClassW(THEME_WC_HYPERLINK, vhHyperlinkRegisteredModule);
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
        for (DWORD i = 0; i < pTheme->cControls; ++i)
        {
            FreeControl(pTheme->rgControls + i);
        }

        for (DWORD i = 0; i < pTheme->cFonts; ++i)
        {
            FreeFont(pTheme->rgFonts + i);
        }

        ReleaseMem(pTheme->rgControls);
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
    __in HWND hwndParent
    )
{
    HRESULT hr = S_OK;

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
                vhModuleRichEd = ::LoadLibraryW(L"Riched20.dll");
                ExitOnNullWithLastError(vhModuleRichEd, hr, "Failed to load Rich Edit control library.");
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
            pControl->hWnd = ::CreateWindowW(wzWindowClass, pControl->wzText, pControl->dwStyle | dwWindowBits, pControl->nX, pControl->nY, pControl->nWidth, pControl->nHeight, hwndParent, NULL, NULL, NULL);
            ExitOnNullWithLastError(pControl->hWnd, hr, "Failed to create window.");

            ::SetWindowLongW(pControl->hWnd, GWL_ID, i);

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
    THEME_CONTROL* pControl = pTheme->rgControls + dwControl;
    HWND hWnd = pControl->hWnd;
    HANDLE hFile = INVALID_HANDLE_VALUE;

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
    }

LExit:
    LocFree(pLocStringSet);
    ReleaseStr(sczFileFullPath);
    return hr;
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
    const THEME_CONTROL* pControl = pTheme->rgControls + pdis->CtlID;

    AssertSz(pControl->hWnd == pdis->hwndItem, "Expected control window to match owner draw window.");
    AssertSz(pControl->nWidth == pdis->rcItem.right - pdis->rcItem.left, "Expected control window width to match owner draw window width.");
    AssertSz(pControl->nHeight == pdis->rcItem.bottom - pdis->rcItem.top, "Expected control window height to match owner draw window height.");

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
BOOL DAPI ThemeIsControlChecked(
    __in THEME* pTheme,
    __in DWORD dwControl
    )
{
    THEME_CONTROL* pControl = pTheme->rgControls + dwControl;
    return BST_CHECKED == ::SendMessageW(pControl->hWnd, BM_GETCHECK, 0, 0);
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
    BOOL fHasBackground = FALSE;
    DWORD dwControl = ::GetWindowLongW(hWnd, GWL_ID);
    const THEME_CONTROL* pControl = pTheme->rgControls + dwControl;
    THEME_FONT* pFont = (THEME_INVALID_ID == pControl->dwFontId) ? NULL : pTheme->rgFonts + pControl->dwFontId;

    if (pFont && pFont->hForeground)
    {
        ::SetTextColor(hdc, pFont->crForeground);
    }

    if (pFont && pFont->hBackground)
    {
        ::SetBkColor(hdc, pFont->crBackground);

        *phBackgroundBrush = pFont->hBackground;
        fHasBackground = TRUE;
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
    THEME_CONTROL* pControl = pTheme->rgControls + dwControl;
    HWND hWnd = pControl->hWnd;
    LONG_PTR dwData = ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
    DWORD dwCurrentProgress =  LOWORD(dwData);

    if (dwCurrentProgress != dwProgressPercentage)
    {
        DWORD dwColor = HIWORD(dwData);
        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, MAKELONG(dwProgressPercentage, dwColor));

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
    THEME_CONTROL* pControl = pTheme->rgControls + dwControl;

    // Only set color on owner draw progress bars.
    if (pTheme->hImage && 0 <= pControl->nSourceX && 0 <= pControl->nSourceY)
    {
        HWND hWnd = pControl->hWnd;
        LONG_PTR dwData = ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
        DWORD dwCurrentColor = HIWORD(dwData);

        if (dwCurrentColor != dwColorIndex)
        {
            DWORD dwCurrentProgress =  LOWORD(dwData);
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, MAKELONG(dwCurrentProgress, dwColorIndex));

            if (!::InvalidateRect(hWnd, NULL, FALSE))
            {
                ExitWithLastError(hr, "Failed to invalidate progress bar window.");
            }

            hr = S_OK;
        }
    }

LExit:
    return hr;
}


/********************************************************************
 ThemeSetTextControl - sets the text of a control.

*******************************************************************/
HRESULT DAPI ThemeSetTextControl(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in_z LPCWSTR wzText
    )
{
    HRESULT hr = S_OK;
    HWND hWnd = pTheme->rgControls[dwControl].hWnd;

    if (!::SetWindowTextW(hWnd, wzText))
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

    if (pTheme->cControls <= dwControl)
    {
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid theme control id.");
    }

    hr = StrAllocString(psczText, pTheme->rgControls[dwControl].wzText, 0);
    ExitOnFailure(hr, "Unable to copy control text.");

LExit:
    return hr;
}


static HRESULT ParseTheme(
    __in_opt HMODULE hModule,
    __in_opt LPCWSTR wzRelativePath,
    __in IXMLDOMDocument* pixd,
    __out THEME** ppTheme
    )
{
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

    // Parse the optional background resource image.
    hr = XmlGetAttribute(pThemeElement, L"i", &bstr);
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
        hr = XmlGetAttribute(pThemeElement, L"if", &bstr);
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
    hr = ParseApplication(hModule, pThemeElement, pTheme);
    ExitOnFailure(hr, "Failed to parse theme application element.");

    // Parse the optional window style.
    hr = XmlGetAttributeNumberBase(pThemeElement, L"s", 16, &pTheme->dwStyle);
    ExitOnFailure(hr, "Failed to get theme window style (t@s) attribute.");

    if (S_FALSE == hr)
    {
        pTheme->dwStyle = WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU;
        pTheme->dwStyle |= (0 <= pTheme->nSourceX && 0 <= pTheme->nSourceY) ? WS_POPUP : WS_OVERLAPPED;
    }

    // Parse the fonts.
    hr = ParseFonts(pThemeElement, pTheme);
    ExitOnFailure(hr, "Failed to parse theme fonts.");

    // Parse the controls.
    hr = ParseControls(pThemeElement, pTheme);
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
    __in IXMLDOMElement* pElement,
    __in THEME* pTheme
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNode* pixn = NULL;
    BSTR bstr = NULL;

    hr = XmlSelectSingleNode(pElement, L"a", &pixn);
    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    ExitOnFailure(hr, "Failed to find application element.");

    hr = XmlGetAttributeNumber(pixn, L"w", reinterpret_cast<DWORD*>(&pTheme->nWidth));
    ExitOnFailure(hr, "Failed to get application width attribute.");

    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnRootFailure(hr, "Failed to find application width attribute.");
    }

    hr = XmlGetAttributeNumber(pixn, L"h", reinterpret_cast<DWORD*>(&pTheme->nHeight));
    ExitOnFailure(hr, "Failed to get application width attribute.");

    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnFailure(hr, "Failed to find application width attribute.");
    }

    hr = XmlGetAttributeNumber(pixn, L"f", &pTheme->dwFontId);
    ExitOnFailure(hr, "Failed to get application font attribute.");

    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnRootFailure(hr, "Failed to find application font attribute.");
    }

    // Get the optional application icon from a resource.
    hr = XmlGetAttribute(pixn, L"i", &bstr);
    ExitOnFailure(hr, "Failed to get application icon resource attribute.");

    if (S_OK == hr)
    {
        pTheme->hIcon = ::LoadIconW(hModule, bstr);
        ExitOnNullWithLastError(pTheme->hIcon, hr, "Failed to load application icon.");

        ReleaseNullBSTR(bstr);
    }

    // Get the optional application icon from a file.
    hr = XmlGetAttribute(pixn, L"if", &bstr);
    ExitOnFailure(hr, "Failed to get application icon file attribute.");

    if (S_OK == hr)
    {
        pTheme->hIcon = ::LoadImageW(NULL, bstr, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
        ExitOnNullWithLastError1(pTheme->hIcon, hr, "Failed to load application icon: %ls.", bstr);

        ReleaseNullBSTR(bstr);
    }

    hr = XmlGetAttributeNumber(pixn, L"sx", reinterpret_cast<DWORD*>(&pTheme->nSourceX));
    ExitOnFailure(hr, "Failed to get application source X attribute.");

    if (S_FALSE == hr)
    {
        pTheme->nSourceX = -1;
    }

    hr = XmlGetAttributeNumber(pixn, L"sy", reinterpret_cast<DWORD*>(&pTheme->nSourceY));
    ExitOnFailure(hr, "Failed to get application source Y attribute.");

    if (S_FALSE == hr)
    {
        pTheme->nSourceY = -1;
    }

    hr = XmlGetText(pixn, &bstr);
    ExitOnFailure(hr, "Failed to get application caption.");

    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnRootFailure(hr, "Failed to find application caption.");
    }

    hr = StrAllocString(&pTheme->wzCaption, bstr, 0);
    ExitOnFailure(hr, "Failed to copy application caption.");

LExit:
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

    hr = XmlSelectNodes(pElement, L"f", &pixnl);
    if (S_FALSE == hr)
    {
        hr = S_OK;
        ExitFunction();
    }
    ExitOnFailure(hr, "Failed to find application element.");

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

        hr = XmlGetAttributeNumber(pixn, L"h", reinterpret_cast<DWORD*>(&lf.lfHeight));
        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
        ExitOnFailure(hr, "Failed to find font height attribute.");

        hr = XmlGetAttributeNumber(pixn, L"w", reinterpret_cast<DWORD*>(&lf.lfWeight));
        if (S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
        ExitOnFailure(hr, "Failed to find font weight attribute.");

        hr = XmlGetAttributeNumber(pixn, L"u", reinterpret_cast<DWORD*>(&lf.lfUnderline));
        if (S_FALSE == hr)
        {
            lf.lfUnderline = FALSE;
        }
        ExitOnFailure(hr, "Failed to find font weight attribute.");

        hr = XmlGetAttributeNumberBase(pixn, L"f", 16, &crForeground);
        if (S_FALSE == hr)
        {
            crForeground = THEME_INVISIBLE_COLORREF;
            hr = S_OK;
        }
        ExitOnFailure(hr, "Failed to find font foreground color.");

        hr = XmlGetAttributeNumberBase(pixn, L"b", 16, &crBackground);
        if (S_FALSE == hr)
        {
            crBackground = THEME_INVISIBLE_COLORREF;
            hr = S_OK;
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


static HRESULT ParseControls(
    __in IXMLDOMElement* pElement,
    __in THEME* pTheme
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnl = NULL;
    IXMLDOMNode* pixn = NULL;
    BSTR bstrType = NULL;

    hr = pElement->get_childNodes(&pixnl);
    if (S_FALSE == hr)
    {
        hr = S_OK;
        ExitFunction();
    }
    ExitOnFailure(hr, "Failed to find control elements.");

    hr = pixnl->get_length(reinterpret_cast<long*>(&pTheme->cControls));
    ExitOnFailure(hr, "Failed to count the number of theme controls.");

    if (pTheme->cControls <= pTheme->cFonts)
    {
        hr = E_INVALIDARG;
        ExitOnFailure(hr, "Number of controls should be greater than number of fonts");
    }

    pTheme->cControls = pTheme->cControls - pTheme->cFonts - 1; // subtract the font elements and "application" element since they are siblings and inflate the count.

    if (0 == pTheme->cControls)
    {
        ExitFunction1(hr = S_OK);
    }

    pTheme->rgControls = static_cast<THEME_CONTROL*>(MemAlloc(sizeof(THEME_CONTROL) * pTheme->cControls, TRUE));
    ExitOnNull(pTheme->rgControls, hr, E_OUTOFMEMORY, "Failed to allocate theme controls.");

    while (S_OK == (hr = XmlNextElement(pixnl, &pixn, &bstrType)))
    {
        THEME_CONTROL_TYPE type = THEME_CONTROL_TYPE_UNKNOWN;

        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"b", 1))
        {
            type = THEME_CONTROL_TYPE_BUTTON;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"cb", 2))
        {
            type = THEME_CONTROL_TYPE_CHECKBOX;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"eb", 2))
        {
            type = THEME_CONTROL_TYPE_EDITBOX;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"l", 1))
        {
            type = THEME_CONTROL_TYPE_HYPERLINK;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"i", 1))
        {
            type = THEME_CONTROL_TYPE_IMAGE;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"pb", 2))
        {
            type = THEME_CONTROL_TYPE_PROGRESSBAR;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"rt", 2))
        {
            type = THEME_CONTROL_TYPE_RICHEDIT;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"s", 1))
        {
            type = THEME_CONTROL_TYPE_STATIC;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrType, -1, L"t", 1))
        {
            type = THEME_CONTROL_TYPE_TEXT;
        }

        if (THEME_CONTROL_TYPE_UNKNOWN != type)
        {
            hr = ParseControl(pixn, type, pTheme);
            ExitOnFailure(hr, "Failed to parse control.");
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
    __in THEME* pTheme
    )
{
    HRESULT hr = S_OK;
    DWORD dwId = 0;
    THEME_CONTROL* pControl = NULL;
    DWORD dwValue = 0;
    BSTR bstrText = NULL;

    hr = XmlGetAttributeNumber(pixn, L"id", &dwId);
    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
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

    hr = XmlGetAttributeNumber(pixn, L"x", reinterpret_cast<DWORD*>(&pControl->nX));
    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    ExitOnFailure(hr, "Failed to find control X attribute.");

    hr = XmlGetAttributeNumber(pixn, L"y", reinterpret_cast<DWORD*>(&pControl->nY));
    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    ExitOnFailure(hr, "Failed to find control Y attribute.");

    hr = XmlGetAttributeNumber(pixn, L"h", reinterpret_cast<DWORD*>(&pControl->nHeight));
    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    ExitOnFailure(hr, "Failed to find control height attribute.");

    hr = XmlGetAttributeNumber(pixn, L"w", reinterpret_cast<DWORD*>(&pControl->nWidth));
    if (S_FALSE == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    ExitOnFailure(hr, "Failed to find control weight attribute.");

    hr = XmlGetAttributeNumber(pixn, L"sx", reinterpret_cast<DWORD*>(&pControl->nSourceX));
    if (S_FALSE == hr)
    {
        pControl->nSourceX = -1;
    }
    ExitOnFailure(hr, "Failed to find control source X attribute.");

    hr = XmlGetAttributeNumber(pixn, L"sy", reinterpret_cast<DWORD*>(&pControl->nSourceY));
    if (S_FALSE == hr)
    {
        pControl->nSourceY = -1;
    }
    ExitOnFailure(hr, "Failed to find control source Y attribute.");

    hr = XmlGetAttributeNumber(pixn, L"f", &pControl->dwFontId);
    if (S_FALSE == hr)
    {
        pControl->dwFontId = THEME_INVALID_ID;
    }
    ExitOnFailure(hr, "Failed to find font for control.");

    hr = XmlGetAttributeNumber(pixn, L"fh", &pControl->dwFontHoverId);
    if (S_FALSE == hr)
    {
        pControl->dwFontHoverId = THEME_INVALID_ID;
    }
    ExitOnFailure(hr, "Failed to find hover font for control.");

    hr = XmlGetAttributeNumber(pixn, L"fs", &pControl->dwFontSelectedId);
    if (S_FALSE == hr)
    {
        pControl->dwFontSelectedId = THEME_INVALID_ID;
    }
    ExitOnFailure(hr, "Failed to find selected font for control.");

    // Parse the optional window style.
    hr = XmlGetAttributeNumberBase(pixn, L"s", 16, &pControl->dwStyle);
    ExitOnFailure(hr, "Failed to get control window style (@s) attribute.");

    // Parse the tabstop bit "shortcut nomenclature", this could have been set with the style above.
    hr = XmlGetAttributeNumber(pixn, L"t", &dwValue);
    if (S_OK == hr && dwValue)
    {
        pControl->dwStyle |= WS_TABSTOP;
    }
    ExitOnFailure(hr, "Failed to tell if the control is a tab stop.");

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

LExit:
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
    DWORD_PTR dwData = ::GetWindowLongPtrW(pdis->hwndItem, GWLP_USERDATA);
    int nSourceY = pControl->nSourceY;

    HDC hdcMem = ::CreateCompatibleDC(pdis->hDC);
    HBITMAP hDefaultBitmap = static_cast<HBITMAP>(::SelectObject(hdcMem, pTheme->hImage));

    if (ODS_SELECTED & pdis->itemState)
    {
        nSourceY += pControl->nHeight * 2;
    }
    else if (dwData & THEME_CONTROL_DATA_HOVER)
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
    DWORD_PTR dwData = ::GetWindowLongPtrW(pdis->hwndItem, GWLP_USERDATA);
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
    else if (dwData & THEME_CONTROL_DATA_HOVER)
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
    DWORD_PTR dwData = ::GetWindowLongPtrW(pdis->hwndItem, GWLP_USERDATA);
    DWORD dwProgressColor = HIWORD(dwData);
    DWORD dwProgressPercentage = LOWORD(dwData);
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
    DWORD_PTR dwData = ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

    if (fHover)
    {
        dwData |= THEME_CONTROL_DATA_HOVER;
    }
    else
    {
        dwData &= ~THEME_CONTROL_DATA_HOVER;
    }

    ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, dwData);
    ::InvalidateRect(hWnd, NULL, FALSE);
}


static void FreeControl(
    __in THEME_CONTROL* pControl
    )
{
    if (pControl)
    {
        if (pControl->hWnd)
        {
            ::CloseWindow(pControl->hWnd);
            pControl->hWnd = NULL;
        }

        ReleaseStr(pControl->wzText);
    }
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
    __in LONG *pcb
    )
{
    HRESULT hr = S_OK;
    HANDLE hFile = (HANDLE)dwCookie;
 
    if (!::ReadFile(hFile, pbBuff, cb, reinterpret_cast<DWORD *>(pcb), NULL))
    {
        ExitWithLastError(hr, "Failed to read file");
    }

LExit:
    return hr;
}