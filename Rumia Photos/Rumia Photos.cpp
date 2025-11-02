// Rumia Photos.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Rumia Photos.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HBITMAP bmp = NULL; // ensure initialized
int x = 0;
int y = 0;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
void OpenPhoto(HWND hWnd);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_RUMIAPHOTOS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RUMIAPHOTOS));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

bool IsProgramOnPathW(const wchar_t* program, wchar_t* fullPath) {
    DWORD result = SearchPath(
        NULL,       // search in PATH
        program,    // program to search
        NULL,       // extension, optional
        MAX_PATH,   // size of buffer
        fullPath,   // buffer to receive full path
        NULL        // optional file part
    );

    return (result > 0 && result < MAX_PATH);
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RUMIAPHOTOS));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_RUMIAPHOTOS);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// Open a photo file and load it as a bitmap
void OpenPhoto(HWND hWnd)
{
    OPENFILENAME ofn;
    wchar_t szFile[260];

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;

    ofn.lpstrFile[0] = L'\0';
    ofn.nMaxFile = _countof(szFile);
    ofn.lpstrFilter = L"Images\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        wchar_t targetFile[260];
        wchar_t* filename = ofn.lpstrFile;
        wchar_t fullPath[260];
        if (IsProgramOnPathW(L"magick.exe", fullPath)) {
            wchar_t command[1024];
            swprintf_s(targetFile, L"%s_converted.bmp", filename);
            swprintf_s(command, L"magick \"%s\" -define bmp:format=bmp3 \"%s\"", filename, targetFile);
            _wsystem(command);
        }
        else {
            swprintf_s(targetFile, L"%s", filename);
        }
        if (bmp) {
            DeleteObject(bmp);
            bmp = NULL;
        }
        bmp = (HBITMAP)LoadImageW(NULL, targetFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        if (!bmp) {
            DWORD err = GetLastError();
            wchar_t msg[512];
            swprintf_s(msg, L"Failed to load bitmap. GetLastError() = 0x%08X", err);
            MessageBoxW(hWnd, msg, L"LoadImage Failed", MB_OK | MB_ICONERROR);
        }
        InvalidateRect(hWnd, NULL, TRUE);
    }
}

// Resize a bitmap and draw it to the destination device context
void ResizeBitmap(HDC hdcDest, HBITMAP hBitmap, int destWidth, int destHeight) {
    HDC hdcSrc = CreateCompatibleDC(hdcDest);
    SelectObject(hdcSrc, hBitmap);
    // Set high-quality stretch mode
    SetStretchBltMode(hdcDest, HALFTONE);
    // Get original bitmap dimensions
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);
    // Scale and draw the bitmap
    StretchBlt(
        hdcDest,
        0, 0, destWidth, destHeight,
        hdcSrc,
        0, 0, bmp.bmWidth, bmp.bmHeight,
        SRCCOPY
    );
    DeleteDC(hdcSrc);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_FILE_OPENPHOTO:
            {
            OpenPhoto(hWnd);
            }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (bmp) {
            BITMAP bm;
            if (GetObject(bmp, sizeof(bm), &bm) != 0) {
				ResizeBitmap(hdc, bmp, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
            }
        }
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        if (bmp) {
            DeleteObject(bmp);
            bmp = NULL;
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
