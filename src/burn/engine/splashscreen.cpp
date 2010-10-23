//-------------------------------------------------------------------------------------------------
// <copyright file="splashscreen.cpp" company="Microsoft">
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
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

#define BURN_SPLASHSCREEN_CLASS_WINDOW L"WixBurnSplashScreen"
#define IDB_SPLASHSCREEN 1

// struct

struct SPLASHSCREEN_IMAGE
{
    HBITMAP hBitmap;
    SIZE size;
};

struct SPLASHSCREEN_CONTEXT
{
    HANDLE hIntializedEvent;
    HINSTANCE hInstance;
    LPCWSTR wzCaption;

    HWND* pHwnd;
};

// internal function definitions

static DWORD WINAPI ThreadProc(
    __in LPVOID pvContext
    );
static LRESULT CALLBACK WndProc(
    __in HWND hWnd,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    );


// function definitions

extern "C" void SplashScreenCreate(
    __in HINSTANCE hInstance,
    __in_z_opt LPCWSTR wzCaption,
    __out HWND* pHwnd
    )
{
    HRESULT hr = S_OK;
    SPLASHSCREEN_CONTEXT context = { };
    HANDLE rgSplashScreenEvents[2] = { };
    DWORD dwSplashScreenThreadId = 0;

    rgSplashScreenEvents[0] = ::CreateEventW(NULL, TRUE, FALSE, NULL);
    ExitOnNullWithLastError(rgSplashScreenEvents[0], hr, "Failed to create modal event.");

    // create splash screen thread.
    context.hIntializedEvent = rgSplashScreenEvents[0];
    context.hInstance = hInstance;
    context.wzCaption = wzCaption;
    context.pHwnd = pHwnd;

    rgSplashScreenEvents[1] = ::CreateThread(NULL, 0, ThreadProc, &context, 0, &dwSplashScreenThreadId);
    ExitOnNullWithLastError(rgSplashScreenEvents[1], hr, "Failed to create UI thread.");

    // It doesn't really matter if the thread gets initialized (WAIT_OBJECT_0) or fails and exits
    // prematurely (WAIT_OBJECT_0 + 1), we just want to wait long enough for one of those two
    // events to happen.
    ::WaitForMultipleObjects(countof(rgSplashScreenEvents), rgSplashScreenEvents, FALSE, INFINITE);

LExit:
    ReleaseHandle(rgSplashScreenEvents[1]);
    ReleaseHandle(rgSplashScreenEvents[0]);
}


static DWORD WINAPI ThreadProc(
    __in LPVOID pvContext
    )
{
    HRESULT hr = S_OK;
    SPLASHSCREEN_CONTEXT* pContext = static_cast<SPLASHSCREEN_CONTEXT*>(pvContext);

    SPLASHSCREEN_IMAGE image = { };
    BITMAP bmp = { };

    WNDCLASSW wc = { };
    BOOL fRegistered = TRUE;
    HWND hWnd = NULL;

    BOOL fRet = FALSE;
    MSG msg = { };

    image.hBitmap = ::LoadBitmapW(pContext->hInstance, MAKEINTRESOURCEW(IDB_SPLASHSCREEN));
    ExitOnNullWithLastError(image.hBitmap, hr, "Failed to load splash screen bitmap.");

    ::GetObject(image.hBitmap, sizeof(bmp), static_cast<void*>(&bmp));
    image.size.cx = bmp.bmWidth;
    image.size.cy = bmp.bmHeight;

    // Register the window class and create the window.
    wc.lpfnWndProc = WndProc;
    wc.hInstance = pContext->hInstance;
    wc.hCursor = ::LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    wc.lpszClassName = BURN_SPLASHSCREEN_CLASS_WINDOW;
    if (!::RegisterClassW(&wc))
    {
        ExitWithLastError(hr, "Failed to register window.");
    }

    fRegistered = TRUE;

    hWnd = ::CreateWindowExW(0, wc.lpszClassName, pContext->wzCaption, WS_POPUP | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, image.size.cx, image.size.cy, HWND_DESKTOP, NULL, pContext->hInstance, &image);
    ExitOnNullWithLastError(hWnd, hr, "Failed to create window.");

    // Return the splash screen window and free the main thread waiting for us to be initialized.
    *pContext->pHwnd = hWnd;
    ::SetEvent(pContext->hIntializedEvent);

    // Pump messages until the bootstrapper application destroys the window.
    while (0 != (fRet = ::GetMessageW(&msg, NULL, 0, 0)))
    {
        if (-1 == fRet)
        {
            hr = E_UNEXPECTED;
            ExitOnFailure(hr, "Unexpected return value from message pump.");
        }
        else if (!::IsDialogMessageW(hWnd, &msg))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
    }

LExit:
    if (fRegistered)
    {
        ::UnregisterClassW(BURN_SPLASHSCREEN_CLASS_WINDOW, pContext->hInstance);
    }

    if (image.hBitmap)
    {
        ::DeleteObject(image.hBitmap);
    }

    return hr;
}

static LRESULT CALLBACK WndProc(
    __in HWND hWnd,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    )
{
    LRESULT lres = 0;
    SPLASHSCREEN_IMAGE* pImage = reinterpret_cast<SPLASHSCREEN_IMAGE*>(::GetWindowLongW(hWnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_NCCREATE:
        {
        LPCREATESTRUCTW lpcs = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(lpcs->lpCreateParams));
        }
        break;

    case WM_NCDESTROY:
        lres = ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
        return lres;

    case WM_NCHITTEST:
        return HTCAPTION; // allow window to be moved by grabbing any pixel.

    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;

    case WM_ERASEBKGND:
        {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        HDC hdcMem = ::CreateCompatibleDC(hdc);
        HBITMAP hDefaultBitmap = static_cast<HBITMAP>(::SelectObject(hdcMem, pImage->hBitmap));
        ::StretchBlt(hdc, 0, 0, pImage->size.cx, pImage->size.cy, hdcMem, 0, 0, pImage->size.cx, pImage->size.cy, SRCCOPY);
        ::SelectObject(hdcMem, hDefaultBitmap);
        ::DeleteDC(hdcMem);
        }
        return 1;
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
