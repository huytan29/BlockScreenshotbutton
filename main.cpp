#include <windows.h>
#include <iostream>
#include <ctime>
#include <string>

// Forward declarations
HBITMAP CreateOverlayBitmap();
void SaveOverlayToFile();
void CreateErrorWindow();

// Global variables
HWND g_hErrorWindow = NULL;

// General keyboard hook function
// Inside your KeyboardProc function
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        // Check for key press events
        if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT* keyInfo = (KBDLLHOOKSTRUCT*)lParam;

            // Check if the user has pressed any key
            std::cout << "Disable Screenshot!" << std::endl;

            // Save the overlay image to the clipboard or a folder
            if (OpenClipboard(NULL)) {
                EmptyClipboard();

                // Create an overlay image and save it to the clipboard
                HBITMAP hBitmap = CreateOverlayBitmap(); // CreateOverlayBitmap() function needs to be implemented
                SetClipboardData(CF_BITMAP, hBitmap);

                CloseClipboard();
            }

            // Save the overlay image to a folder
            SaveOverlayToFile();

            // Show the error window automatically
            if (g_hErrorWindow != NULL) {
                ShowWindow(g_hErrorWindow, SW_SHOW);
                UpdateWindow(g_hErrorWindow);
            }

            return 1; // Block the event
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


// Function to create an overlay image (e.g., a black bitmap)
HBITMAP CreateOverlayBitmap() {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);

    SelectObject(hdcMem, hBitmap);

    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0)); // Black color
    RECT rect = {0, 0, screenWidth, screenHeight};
    FillRect(hdcMem, &rect, hBrush);

    DeleteObject(hBrush);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return hBitmap;
}

// Function to save the overlay image to a folder
void SaveOverlayToFile() {
    // Set the file name and storage folder as needed
    std::string folderPath = "C:\\Screenshots\\";
    std::time_t now = std::time(nullptr);
    std::string fileName = folderPath + "overlay_" + std::to_string(now) + ".bmp";

    HBITMAP hBitmap = CreateOverlayBitmap();

    // Save the image to a BMP file
    PBITMAPINFO pbmi = (PBITMAPINFO)malloc(sizeof(BITMAPINFOHEADER));
    // ... Perform image save operations, create folders if necessary

    DeleteObject(hBitmap);
    free(pbmi);
}

// Function to handle the error window
LRESULT CALLBACK ErrorWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

void CreateErrorWindow() {
    const wchar_t* className = L"ErrorWindowClass";

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = ErrorWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = className;

    if (!RegisterClassExW(&wc)) {
        std::cerr << "Error registering window class" << std::endl;
        return;
    }

    g_hErrorWindow = CreateWindowExW(
        0,
        className,
        L"Warning",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (g_hErrorWindow == NULL) {
        std::cerr << "Error creating window" << std::endl;
        return;
    }

    // Set the default color and font size for the notification window
    SendMessage(g_hErrorWindow, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

    // Set the window dimensions
    RECT rc;
    GetClientRect(g_hErrorWindow, &rc);
    SetWindowPos(g_hErrorWindow, NULL, rc.left, rc.top, 400, 200, SWP_NOMOVE | SWP_NOZORDER);

    // Use the default icon for a warning message
    SendMessage(g_hErrorWindow, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(NULL, IDI_WARNING));

    // Show the message box inside the error window
    MessageBoxW(g_hErrorWindow, L"Screenshot functionality has been disabled.", L"Warning", MB_ICONWARNING | MB_OK);

    ShowWindow(g_hErrorWindow, SW_HIDE);
    UpdateWindow(g_hErrorWindow);
}

int main() {
    // Set up the global keyboard hook
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

    if (hook == NULL) {
        std::cerr << "Error setting up keyboard hook" << std::endl;
        return 1;
    }

    // Create the error notification window
    CreateErrorWindow();

    // Run the message loop to maintain the hook
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook the keyboard before exiting
    UnhookWindowsHookEx(hook);

    return 0;
}
