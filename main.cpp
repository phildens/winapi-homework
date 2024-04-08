#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <windowsx.h>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include <cmath>

#define KEY_SHIFTED     0x8000
#define KEY_TOGGLED     0x0001

enum ObjectType {
    NONE,
    CIRCLE,
    CROSS
};


ObjectType** grid = nullptr;




const TCHAR szWinClass[] = _T("Win32SampleApp");
const TCHAR szWinName[] = _T("Win32SampleWindow");
HWND hwnd;               /* This is the handle for our window */
HBRUSH hBrush;  

int rows = 3;
int cols = 3;
COLORREF startColor = RGB(255, 0, 0); // Начальный цвет
COLORREF endColor = RGB(0, 0, 255);   // Конечный цвет
int colorStep = 0;
/* Current brush */
COLORREF InterpolateColor(COLORREF color1, COLORREF color2, float factor) {
    // Интерполируем цвет между color1 и color2 с заданным фактором
    int r = int(GetRValue(color1) * (1.0 - factor) + GetRValue(color2) * factor);
    int g = int(GetGValue(color1) * (1.0 - factor) + GetGValue(color2) * factor);
    int b = int(GetBValue(color1) * (1.0 - factor) + GetBValue(color2) * factor);
    return RGB(r, g, b);
}

void RunNotepad(void)
{
    STARTUPINFO sInfo;
    PROCESS_INFORMATION pInfo;

    ZeroMemory(&sInfo, sizeof(STARTUPINFO));

    puts("Starting Notepad...");
    CreateProcess(_T("C:\\Windows\\Notepad.exe"),
        NULL, NULL, NULL, FALSE, 0, NULL, NULL, &sInfo, &pInfo);
}
	
void DrawCircle(HDC hdc, int x, int y) {
    HPEN hpen = CreatePen(PS_SOLID, 3, InterpolateColor(startColor, endColor, sin(colorStep * 0.1)));
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int cellWidth = clientRect.right / cols;
    int cellHeight = clientRect.bottom / rows;
    // Рисуем круг в ячейке, где было нажатие правой кнопки мыши
    SelectObject(hdc, hpen);
    Ellipse(hdc, x / cellWidth * cellWidth, y / cellHeight * cellHeight,
        (x / cellWidth + 1) * cellWidth, (y / cellHeight + 1) * cellHeight);
    DeleteObject(hpen);
}

void DrawCross(HDC hdc, int x, int y) {
    HPEN hpen = CreatePen(PS_SOLID,3, InterpolateColor(startColor, endColor, sin(colorStep * 0.1)));
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int cellWidth = clientRect.right / cols;
    int cellHeight = clientRect.bottom / rows;
    // Рисуем крестик в ячейке, где было нажатие левой кнопки мыши
    SelectObject(hdc, hpen);
    MoveToEx(hdc, x / cellWidth * cellWidth, y / cellHeight * cellHeight, NULL);
    LineTo(hdc, (x / cellWidth + 1) * cellWidth, (y / cellHeight + 1) * cellHeight);
    MoveToEx(hdc, (x / cellWidth + 1) * cellWidth, y / cellHeight * cellHeight, NULL);
    LineTo(hdc, x / cellWidth * cellWidth, (y / cellHeight + 1) * cellHeight);
    DeleteObject(hpen);
}



void DrawShapes(HDC hdc) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    
    int cellWidth = clientRect.right / cols;
    int cellHeight = clientRect.bottom / rows;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int x = j * cellWidth + cellWidth / 2;
            int y = i * cellHeight + cellHeight / 2;
            if (grid[i][j] == CIRCLE) {
                DrawCircle(hdc, x, y);
            }
            else if (grid[i][j] == CROSS) {
                DrawCross(hdc, x, y);
            }
        }
    }
}

void wpaint(HWND hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    int cellWidth = clientRect.right / cols;
    int cellHeight = clientRect.bottom / rows;
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // Рисуем сетку
    for (int i = 1; i < rows; ++i) {
        MoveToEx(hdc, 0, i * cellHeight, NULL);
        LineTo(hdc, cols * cellWidth, i * cellHeight);
    }
    for (int i = 1; i < cols; ++i) {
        MoveToEx(hdc, i * cellWidth, 0, NULL);
        LineTo(hdc, i * cellWidth, rows * cellHeight);
    }

    // Рисуем круги и крестики
    DrawShapes(hdc);

    EndPaint(hwnd, &ps);
    
}
void UpdateGrid(int x, int y, ObjectType type) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    int cellWidth = clientRect.right / cols;
    int cellHeight = clientRect.bottom / rows;
    grid[y / cellHeight][x / cellWidth] = type;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
    case WM_DESTROY:
        PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
        return 0;
    case WM_KEYDOWN:
        if (wParam == 'Q' && GetAsyncKeyState(VK_CONTROL) < 0) {
            PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
            
        }
        else if (wParam == VK_ESCAPE){
            PostQuitMessage(0);
        }
        else if (wParam == 'C' && GetAsyncKeyState(VK_SHIFT) < 0) {
            RunNotepad();
        }
        else if (wParam == VK_RETURN) {
            COLORREF color = RGB(rand() % 256, rand() % 256, rand() % 256);
            hBrush = CreateSolidBrush(color);
            SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    case WM_PAINT: {
        wpaint(hwnd);
        return 0;
        }
    case WM_SIZE: {
        wpaint(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (delta > 0) {
            colorStep += 5;
        }
        else {
            colorStep -= 5;
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_RBUTTONDOWN: {
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        int cellWidth = clientRect.right / cols;
        int cellHeight = clientRect.bottom / rows;
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        std::cout << x, y;
        if (grid[y / cellHeight][x / cellWidth] == NONE) {
            // Добавляем круг в сетку
            UpdateGrid(x, y, CIRCLE);

            // Перерисовываем окно
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    case WM_LBUTTONDOWN: {
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        int cellWidth = clientRect.right / cols;
        int cellHeight = clientRect.bottom / rows;
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        if (grid[y / cellHeight][x/ cellWidth] == NONE) {
            // Добавляем крестик в сетку
            UpdateGrid(x, y, CROSS);

            // Перерисовываем окно
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }

    } 

    /* for messages that we don't deal with */
    return DefWindowProc(hwnd, message, wParam, lParam);
}



int main(int argc, char** argv)
{
    if (__argc == 2) {
        rows = atoi(__argv[1]);
        cols = atoi(__argv[1]);
        std::cout << atoi(__argv[1]);
    }
    
    grid = new ObjectType * [rows];
    for (int i = 0; i < rows; ++i) {
        grid[i] = new ObjectType[cols];
        memset(grid[i], NONE, cols * sizeof(ObjectType));
    }


    BOOL bMessageOk;
    MSG message;            /* Here message to the application are saved */
    WNDCLASS wincl = { 0 };         /* Data structure for the windowclass */

    /* Harcode show command num when use non-winapi entrypoint */
    int nCmdShow = SW_SHOW;
    /* Get handle */
    HINSTANCE hThisInstance = GetModuleHandle(NULL);

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szWinClass;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by Windows */

    /* Use custom brush to paint the background of the window */
    hBrush = CreateSolidBrush(RGB(0, 30, 255));
    wincl.hbrBackground = hBrush;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClass(&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindow(
        szWinClass,          /* Classname */
        szWinName,       /* Title Text */
        WS_OVERLAPPEDWINDOW, /* default window */
        CW_USEDEFAULT,       /* Windows decides the position */
        CW_USEDEFAULT,       /* where the window ends up on the screen */
        320,                 /* The programs width */
        240,                 /* and height in pixels */
        HWND_DESKTOP,        /* The window is a child-window to desktop */
        NULL,                /* No menu */
        hThisInstance,       /* Program Instance handler */
        NULL                 /* No Window Creation data */
    );

    /* Make the window visible on the screen */
    ShowWindow(hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while ((bMessageOk = GetMessage(&message, NULL, 0, 0)) != 0)
    {
        /* Yep, fuck logic: BOOL mb not only 1 or 0.
         * See msdn at https://msdn.microsoft.com/en-us/library/windows/desktop/ms644936(v=vs.85).aspx
         */
        if (bMessageOk == -1)
        {
            puts("Suddenly, GetMessage failed! You can call GetLastError() to see what happend");
            break;
        }
        /* Translate virtual-key message into character message */
        TranslateMessage(&message);
        /* Send message to WindowProcedure */
        DispatchMessage(&message);
    }
    for (int i = 0; i < rows; ++i) {
        delete[] grid[i];
    }
    delete[] grid;
    /* Cleanup stuff */
    DestroyWindow(hwnd);
    UnregisterClass(szWinClass, hThisInstance);
    DeleteObject(hBrush);



    return 0;
}