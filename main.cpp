#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <windowsx.h>
#include <iostream>
#include <cmath>
#include <fstream>
#define KEY_SHIFTED     0x8000
#define KEY_TOGGLED     0x0001
#define MODE_MMAP           0
#define MODE_POSIX          1
#define MODE_FSTREAM        2
#define MODE_WINAPI         3

#define CIRCLE  1
#define CROSS   2

enum ObjectType {
    NONE,
    CIRCLES,
    CROSSES
};

struct Color {
    int r = 0;
    int g = 0;
    int b = 0;
};

ObjectType** grid = nullptr;


struct Configuration {
    int gridSize;
    int windowWidth;
    int windowHeight;
    COLORREF backgroundColor;
    COLORREF gridColor;
    COLORREF circleColor;
    COLORREF crossColor;
};

// Глобальная переменная для хранения конфигурации
Configuration config;
bool config_exist = false;

// Функция для чтения конфигурационного файла
void ReadConfigFile() {
    std::ifstream file("config.ini");
    if (file.is_open()) {
        // Чтение значений из файла и запись в структуру config
        file >> config.gridSize;
        file >> config.windowWidth;
        file >> config.windowHeight;
        int r, g, b;
        file >> r >> g >> b;
        config.backgroundColor = RGB((BYTE)r, (BYTE)g, (BYTE)b);
        file >> r >> g >> b;
        config.gridColor = RGB(r, g, b);
        file >> r >> g >> b;
        config.circleColor = RGB(r, g, b);
        file >> r >> g >> b;
        config.crossColor = RGB(r, g, b);
        file.close();
        config_exist = true;
    } else {
        // Если файл не найден, используем значения по умолчанию
        config.gridSize = 3;
        config.windowWidth = 320;
        config.windowHeight = 240;
        config.backgroundColor = RGB(0, 0, 255);
        config.gridColor = RGB(255, 0, 0);
        config.circleColor = RGB(0, 0, 255);
        config.crossColor = RGB(0, 255, 0);
    }
}

// Функция для записи конфигурационного файла
void WriteConfigFile() {
    std::ofstream file("config.ini");
    if (file.is_open()) {
        // Запись значений из структуры config в файл
        file << config.gridSize << "\n";
        file << config.windowWidth << " " << config.windowHeight << "\n";
        file << (int)GetRValue(config.backgroundColor) << " " << (int)GetGValue(config.backgroundColor) << " " << (int)GetBValue(config.backgroundColor) << "\n";
        file << GetRValue(config.gridColor) << " " << GetGValue(config.gridColor) << " " << GetBValue(config.gridColor) << "\n";
        file << GetRValue(config.circleColor) << " " << GetGValue(config.circleColor) << " " << GetBValue(config.circleColor) << "\n";
        file << GetRValue(config.crossColor) << " " << GetGValue(config.crossColor) << " " << GetBValue(config.crossColor) << "\n";
        file.close();
    }
}
//j
const TCHAR szWinClass[] = _T("Win32SampleApp");
const TCHAR szWinName[] = _T("Win32SampleWindow");
HWND hwnd;               /* This is the handle for our window */
HBRUSH hBrush;


COLORREF startColor = RGB(255, 0, 0); //
COLORREF endColor = RGB(0, 200, 255);   //
int colorStep = 0;
/* Current brush */
COLORREF InterpolateColor(COLORREF color1, COLORREF color2, float factor) {
    //                          color1   color2
    int r = int(GetRValue(color1) * (1.0 - factor) + GetRValue(color2) * factor);
    int g = int(GetGValue(color1) * (1.0 - factor) + GetGValue(color2) * factor);
    int b = int(GetBValue(color1) * (1.0 - factor) + GetBValue(color2) * factor);
    return RGB(r, g, b);
}
void Line(HDC hdc, UINT x1, UINT y1, UINT x2, UINT y2) {
    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
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
    HPEN hpen = CreatePen(PS_SOLID, 3, RGB(0, 200, 0));
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int xi = x;
    int x_rate = clientRect.right % config.gridSize;
    double cellWidth = clientRect.right / config.gridSize + x_rate;
    //x *= cellWidth;

    double cellHeight = clientRect.bottom / config.gridSize;
    //y *= cellHeight;
    SelectObject(hdc, hBrush);
    //                     ,
    SelectObject(hdc, hpen);
    Ellipse(hdc, x * clientRect.right / config.gridSize, y * clientRect.bottom/ config.gridSize,
            (x + 1) * clientRect.right / config.gridSize, (y + 1) * clientRect.bottom / config.gridSize);
    DeleteObject(hpen);
}

void DrawCross(HDC hdc, int x, int y) {
    HPEN hpen = CreatePen(PS_SOLID, 2, RGB(0,200,0));
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    //                        ,
    SelectObject(hdc, hpen);
    Line(hdc, x * clientRect.right / config.gridSize, y * clientRect.bottom / config.gridSize, x * clientRect.right / config.gridSize + clientRect.right / config.gridSize, y * clientRect.bottom / config.gridSize + clientRect.bottom / config.gridSize);
    Line(hdc, x * clientRect.right / config.gridSize + clientRect.right / config.gridSize, y * clientRect.bottom / config.gridSize, x * clientRect.right / config.gridSize, y * clientRect.bottom / config.gridSize + clientRect.bottom / config.gridSize);

    DeleteObject(hpen);
}



void DrawShapes(HDC hdc) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    int cellWidth = clientRect.right / config.gridSize;
    int cellHeight = clientRect.bottom / config.gridSize;
    for (int i = 0; i < config.gridSize; i++) {
        for (int j = 0; j < config.gridSize; j++) {

            if (grid[i][j] == CIRCLE) {
                DrawCircle(hdc, j, i);
            }
            else if (grid[i][j] == CROSS) {
                DrawCross(hdc, j, i);
            }
        }
    }
}



void wpaint(HWND hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    int cellWidth = clientRect.right;
    int cellHeight = clientRect.bottom;

    WriteConfigFile();
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    HPEN hpen = CreatePen(PS_SOLID, 2, InterpolateColor(startColor, endColor, sin(colorStep * 0.1)));
    SelectObject(hdc, hpen);
    //
    for (int i = 1; i < config.gridSize; ++i) {
        Line(hdc, 0, i * cellHeight / config.gridSize, cellWidth, i * cellHeight/ config.gridSize);
        Line(hdc, i * cellWidth / config.gridSize,  0,  i * cellWidth / config.gridSize, cellHeight);

    }
    DeleteObject(hpen);

    //
    DrawShapes(hdc);

    EndPaint(hwnd, &ps);

}
//void UpdateGrid(int x, int y, ObjectType type) {
//    RECT clientRect;
//    GetClientRect(hwnd, &clientRect);
//
//    int cellWidth = clientRect.right / config.gridSize;
//    int cellHeight = clientRect.bottom / config.gridSize;
//    grid[y / cellHeight][x / cellWidth] = type;
//}
unsigned long seed = 1;  // это то самое стартовое значение,
// о котором мы говорили ранее

int my_rand() {
    seed = seed * 1103515245 + 12345;  // Линейный конгруэнтный метод
    return (seed / 65536) % 256;     // Вернуть значение в диапазоне 0-32767
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
            else if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            else if (wParam == 'C' && GetAsyncKeyState(VK_SHIFT) < 0) {
                RunNotepad();
            }
            else if (wParam == VK_RETURN) {
               

                srand(time(0)*100);
                
                
                COLORREF color = RGB(my_rand(), my_rand(), my_rand());
                config.backgroundColor = color;
                WriteConfigFile();
                std::cout << color;

                hBrush = CreateSolidBrush(color);

                DeleteObject((HBRUSH) SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush));

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
            RECT rect;
            GetWindowRect(hwnd, &rect);
            config.windowHeight = rect.bottom - rect.top;
            config.windowWidth = rect.right - rect.left;
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

            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            std::cout << x, y;
            if (grid[y * config.gridSize / clientRect.bottom][x * config.gridSize / clientRect.right] == NONE) {
                //
                grid[y * config.gridSize / clientRect.bottom][x * config.gridSize / clientRect.right] = CIRCLES;

                //
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        }
        case WM_LBUTTONDOWN: {
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            if (grid[y * config.gridSize / clientRect.bottom][x * config.gridSize / clientRect.right] == NONE) {
                //
                grid[y * config.gridSize / clientRect.bottom][x * config.gridSize / clientRect.right] = CROSSES;

                //
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
    ReadConfigFile();
    
    if (__argc == 2) {
        config.gridSize = atoi(__argv[1]);
        std::cout << atoi(__argv[1]);
        WriteConfigFile();
    }

    
    grid = new ObjectType * [config.gridSize];
    for (int i = 0; i < config.gridSize; ++i) {
        grid[i] = new ObjectType[config.gridSize];
        memset(grid[i], NONE, config.gridSize * sizeof(ObjectType));
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
    hBrush = CreateSolidBrush(config.backgroundColor);
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
            config.windowWidth,                 /* The programs width */
            config.windowHeight,                 /* and height in pixels */
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
    for (int i = 0; i < config.gridSize; ++i) {
        delete[] grid[i];
    }
    delete[] grid;
    /* Cleanup stuff */
    DestroyWindow(hwnd);
    UnregisterClass(szWinClass, hThisInstance);
    DeleteObject(hBrush);



    return 0;
}