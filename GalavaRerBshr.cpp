#include "framework.h"
#include <commctrl.h> // For creating edit controls (text fields)
#include "GalavaRerBshr.h"
#include <shellapi.h> // For system tray icons

#define MAX_LOADSTRING 100
#define IDC_EDIT 101   // Identifier for the edit control
#define IDC_BUTTON 102 // Identifier for the button
#define WM_TRAYICON (WM_USER + 1)  // Custom message for tray icon
#define IDI_CUSTOM_ICON 103 // Custom icon resource identifier
#define HOTKEY_ID 104 // Identifier for the hotkey

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWnd;                                      // handle to main window
NOTIFYICONDATA nid;                             // tray icon data
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
wchar_t cachedText[100];                        // Global variable to store cached text

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Function to add the tray icon
void AddTrayIcon(HWND hWnd) {
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;

    // Load the custom icon instead of the default application icon
    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CUSTOM_ICON));

    wcscpy_s(nid.szTip, L"Galava RerBshr");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

// Function to remove the tray icon
void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

// Function to minimize the window to the tray
void MinimizeToTray(HWND hWnd) {
    ShowWindow(hWnd, SW_HIDE);  // Hide the window
    AddTrayIcon(hWnd);          // Add the tray icon
}

// Function to restore the window from the tray
void RestoreFromTray(HWND hWnd) {
    RemoveTrayIcon();           // Remove the tray icon
    ShowWindow(hWnd, SW_SHOW);  // Show the window
    SetForegroundWindow(hWnd);  // Bring the window to the foreground
}

// Function to copy text to clipboard
void CopyTextToClipboard(const wchar_t* text) {
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        size_t size = (wcslen(text) + 1) * sizeof(wchar_t);
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
        if (hGlobal) {
            memcpy(GlobalLock(hGlobal), text, size);
            GlobalUnlock(hGlobal);
            SetClipboardData(CF_UNICODETEXT, hGlobal);
        }
        CloseClipboard();
    }
}

// Function to register the hotkey
void RegisterHotkey(HWND hWnd) {
    // Register Ctrl + P + I as a hotkey (VK_P = 0x50, VK_I = 0x49)
    RegisterHotKey(hWnd, HOTKEY_ID, MOD_CONTROL, 'P');
}

// Function to handle hotkey
void HandleHotkey() {
    // Check if the cached text exists and paste it
    if (wcslen(cachedText) > 0) {
        CopyTextToClipboard(cachedText);
        // Optionally, you can simulate a paste action here if needed
        MessageBox(NULL, L"Text pasted to clipboard!", L"Hotkey Activated", MB_OK);
    }
    else {
        MessageBox(NULL, L"No text cached.", L"Hotkey Activated", MB_OK);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GALAVARERBSHR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GALAVARERBSHR));

    MSG msg;

    // Register the hotkey
    RegisterHotkey(hWnd);

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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GALAVARERBSHR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GALAVARERBSHR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 400, 300, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hEdit, hButton;  // Declare static handles for the controls

    switch (message)
    {
    case WM_CREATE:
    {
        // Create edit control (text field)
        hEdit = CreateWindowEx(0, WC_EDIT, L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
            10, 10, 200, 20, hWnd, (HMENU)IDC_EDIT, hInst, NULL);

        // Create button
        hButton = CreateWindowEx(0, WC_BUTTON, L"SAVE", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            220, 10, 80, 20, hWnd, (HMENU)IDC_BUTTON, hInst, NULL);
    }
    break;
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
        case IDC_BUTTON:  // Handle button click
        {
            GetWindowText(hEdit, cachedText, 100);  // Cache the text from the edit control

            // Minimize to tray
            MinimizeToTray(hWnd);
        }
        break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_HOTKEY:
        if (wParam == HOTKEY_ID) {
            HandleHotkey();  // Handle the hotkey press
        }
        break;
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP) {
            RestoreFromTray(hWnd);
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        RemoveTrayIcon();
        UnregisterHotKey(hWnd, HOTKEY_ID);  // Unregister the hotkey
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
