#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <windowsx.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#define KEY_SHIFTED     0x8000
#define KEY_TOGGLED     0x0001
#define MODE_MMAP           0
#define MODE_POSIX          1
#define MODE_FSTREAM        2
#define MODE_WINAPI         3

#define CIRCLE  1
#define CROSS   2
std::atomic<bool> is_render(true);
std::atomic<bool> is_pause(true);
const UINT WM_FIELD_UPDATE = RegisterWindowMessage("WM_FIELD_UPDATE");
enum ObjectType {
    NONE,
    CIRCLES,
    CROSSES
};
HANDLE back_t;
HANDLE sem1;
HANDLE sem2;
HANDLE mutex;
struct Color {
    UINT r = 0;
    UINT g = 0;
    UINT b = 0;

    [[nodiscard]] unsigned int as_uint() const {
        return RGB(r, g, b);
    }
};
#define VK_0           0x30
#define VK_9           0x39
ObjectType** grid = nullptr;


struct color_change {
    UINT h = 0;
    UINT s = 0;
    UINT v = 0;

    [[nodiscard]] unsigned int as_uint() const {
        int h1, vmin, vinc, vdec, a, r, g, b;
        h1 = (h / 60) % 6;
        vmin = (100 - s) * v / 100;
        a = (v - vmin) * (h % 60) / 60;
        vinc = vmin + a;
        vdec = v - a;
        switch (h1) {
        case 0:
            r = v;
            g = vinc;
            b = vmin;
            break;
        case 1:
            r = vdec;
            g = v;
            b = vmin;
            break;
        case 2:
            r = vmin;
            g = v;
            b = vinc;
            break;
        case 3:
            r = vmin;
            g = vdec;
            b = v;
            break;
        case 4:
            r = vinc;
            g = vmin;
            b = v;
            break;
        case 5:
            r = v;
            g = vmin;
            b = vdec;
            break;
        default:
            break;
        }
        return RGB(r * 255 / 100, g * 255 / 100, b * 255 / 100);
    }
};

struct Configuration {
    int gridSize;
    int windowWidth;
    int windowHeight;
    COLORREF backgroundColor;
    COLORREF gridColor;

    void ViewConfigs() {
        std::cout << " grid   " << gridSize << '\n' <<
            windowHeight << "   height   " << windowWidth << '\n';
    }
};
int method = 3;
// ���������� ���������� ��� �������� ������������
Configuration config, viver;
bool config_exist = false;
int take_start_color = 0;

bool hasFile() {
    DWORD fileAttributes = GetFileAttributes("CONFIG");

    return fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}
// ������� ��� ������ ����������������� �����
void ReadConfigFile() {
    std::cout << method << " method in switch" << '\n';
    switch (method) {
    case (0): {
        std::cout << "stream" << '\n';
        std::fstream output_file("CONFIG");
        output_file.read((char*)&config, sizeof(Configuration));
        output_file.close();
        break;
    };
    case (1): {
        std::cout << "windows" << '\n';
        HANDLE hFile = CreateFile(TEXT("CONFIG"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD dwBytesRead;
            if (ReadFile(hFile, &config, sizeof(Configuration), &dwBytesRead, NULL) && dwBytesRead == sizeof(Configuration)) {}
            // ��������� ������������ �������
            CloseHandle(hFile);
        }
        break;
    };
    case (2): {
        std::cout << "fopen" << '\n';
        auto fd = fopen("CONFIG", "rb");
        fread(&config, sizeof(Configuration), 1, fd);
        fclose(fd);

        break;
    };
    case (3): {
        std::cout << "mmap" << '\n';
        auto h_file = CreateFile(
            "CONFIG",
            GENERIC_READ,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        auto mapping = CreateFileMapping(
            h_file,
            nullptr,
            PAGE_READONLY,
            0,
            sizeof(Configuration),
            nullptr);

        auto data = (Configuration*)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, sizeof(Configuration));

        memcpy((void*)&config, data, sizeof(Configuration));

        UnmapViewOfFile(data);
        CloseHandle(mapping);
        CloseHandle(h_file);
        break;
    };

    default:
        break;
    }
}





// ������� ��� ������ ����������������� �����
void WriteConfigFile() {

    switch (method)
    {
    case 0: {
        std::cout << "stream" << '\n';
        std::fstream output_file("CONFIG", std::ios::ate);
        output_file.write((char*)&config, sizeof(Configuration));
        output_file.close();
        
        break;
    }

    case 1: {
        std::cout << "windows" << '\n';
        HANDLE hFile = CreateFile(TEXT("CONFIG"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD dwBytesWritten;
            if (WriteFile(hFile, &config, sizeof(Configuration), &dwBytesWritten, NULL) && dwBytesWritten == sizeof(Configuration)) {
                // �������� ������������ �������
            }


        }
        CloseHandle(hFile);
        break;
    }
    case 2: {
        std::cout << "fopen" << '\n';
        auto fd = fopen("CONFIG", "wb");
        fwrite(&config, sizeof(Configuration), 1, fd);
        fclose(fd);
    }
    case (3): {
        std::cout << "mmap" << '\n';
       auto h_file = CreateFile("CONFIG", GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr);

        auto mapping = CreateFileMapping(h_file, nullptr, PAGE_READWRITE, 0, sizeof(Configuration), nullptr);

        auto data = (Configuration*)MapViewOfFile(mapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0,
            sizeof(Configuration));

        memcpy(data, (void*) & config, sizeof(Configuration));

        UnmapViewOfFile(data);
        CloseHandle(mapping);
        CloseHandle(h_file);
        break;
    }
    default:
        break;
    }
    
    auto h_file = CreateFile(
        "CONFIG",
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    auto mapping = CreateFileMapping(
        h_file,
        nullptr,
        PAGE_READONLY,
        0,
        sizeof(Configuration),
        nullptr);

    auto data = (Configuration*)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, sizeof(Configuration));

    memcpy((void*)&viver, data, sizeof(Configuration));

    UnmapViewOfFile(data);
    CloseHandle(mapping);
    CloseHandle(h_file);
    std::cout << "checker " << '\n';
    viver.ViewConfigs();
}



struct Board {
    RECT rect{ 0 };
    HANDLE h_map_file;
    std::vector<std::vector<UINT>> data;
    const wchar_t* shared_memory_name = L"MySharedMemory";
    int* lp_map_address;
    int shared_memory_size;
    HBRUSH h_brush;
    HPEN hpen_t1;
    HPEN hpen_t2;
    void pullData() {
        for (int i = 0; i < config.gridSize; ++i)
            memcpy(data[i].data(), lp_map_address + i * config.gridSize, sizeof(int) * config.gridSize);
    }

    void pushData() {
        for (int i = 0; i < config.gridSize; ++i)
            memcpy(lp_map_address + i * config.gridSize, data[i].data(), sizeof(int) * config.gridSize);
        PostMessage(HWND_BROADCAST, WM_FIELD_UPDATE, 0, 0);

    }

    void initData() {
        for (int i = 0; i < config.gridSize; ++i) { data.emplace_back(config.gridSize); }
        shared_memory_size = sizeof(UINT) * config.gridSize * config.gridSize;
        h_map_file = CreateFileMappingW(INVALID_HANDLE_VALUE,
            nullptr,
            PAGE_READWRITE, 0, shared_memory_size,
            shared_memory_name);
        lp_map_address = (int*)MapViewOfFile(h_map_file, FILE_MAP_ALL_ACCESS, 0, 0, shared_memory_size);
        pullData();
    }

    void close() {
        UnmapViewOfFile(lp_map_address);
        CloseHandle(h_map_file);

        //DeleteObject(board.h_brush);
        DeleteObject(hpen_t1);
        DeleteObject(hpen_t2);
    }
};
//j
const TCHAR szWinClass[] = _T("Win32SampleApp");
const TCHAR szWinName[] = _T("Win32SampleWindow");
HWND hwnd;               /* This is the handle for our window */
HBRUSH hBrush;
Board board;

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
    Ellipse(hdc, x * clientRect.right / config.gridSize, y * clientRect.bottom / config.gridSize,
        (x + 1) * clientRect.right / config.gridSize, (y + 1) * clientRect.bottom / config.gridSize);
    DeleteObject(hpen);
}

void DrawCross(HDC hdc, int x, int y) {
    HPEN hpen = CreatePen(PS_SOLID, 2, RGB(0, 200, 0));
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

            if (board.data[i][j] == CIRCLE) {
                DrawCircle(hdc, j, i);
            }
            else if (board.data[i][j] == CROSS) {
                DrawCross(hdc, j, i);
            }
        }
    }
}



void wpaint(HDC hdc) {
    
    GetClientRect(hwnd, &board.rect);

    int cellWidth = board.rect.right;
    int cellHeight = board.rect.bottom;


   
    //HDC hdc = BeginPaint(hwnd, &ps);
    COLORREF gridcolor = RGB(0, 200, 255);
    

    //std::cout << config.gridColor << " " << gridcolor << '\n';


    HPEN hpen = CreatePen(PS_SOLID, 2, gridcolor);
    SelectObject(hdc, board.h_brush);
    SelectObject(hdc, hpen);
    //
    for (int i = 1; i < config.gridSize; ++i) {
        Line(hdc, 0, i * cellHeight / config.gridSize, cellWidth, i * cellHeight / config.gridSize);
        Line(hdc, i * cellWidth / config.gridSize, 0, i * cellWidth / config.gridSize, cellHeight);

    }
    DeleteObject(hpen);

    //
    DrawShapes(hdc);

    

}
//void UpdateGrid(int x, int y, ObjectType type) {
//    RECT clientRect;
//    GetClientRect(hwnd, &clientRect);
//
//    int cellWidth = clientRect.right / config.gridSize;
//    int cellHeight = clientRect.bottom / config.gridSize;
//    grid[y / cellHeight][x / cellWidth] = type;
//}
unsigned long seed = 1;  // ��� �� ����� ��������� ��������,
// � ������� �� �������� �����

int my_rand() {
    seed = seed * 1103515245 + 12345;  // �������� ������������ �����
    return (seed / 65536) % 256;     // ������� �������� � ��������� 0-32767
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //RECT rect = { 0 };
    if (message == WM_FIELD_UPDATE) {
        board.pullData();
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    }
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
        else if (wParam == VK_SPACE) {
            std::cout << "thread_paused";
            if (is_pause) {
                is_pause = false;
            }
            else
            {
                is_pause = true;
            }
            
            (is_pause ? ResumeThread : SuspendThread)(back_t);
        }
        else if (VK_0 <= wParam && wParam <= VK_9) {
            //std::cout << "old priority: " << GetThreadPriority(back_t) << std::endl;
            //std::cout << "to set: " << wParam - VK_0 << " highest: " << THREAD_PRIORITY_HIGHEST << std::endl;
            SetThreadPriority(back_t, wParam - VK_0);
            std::cout << "change priority to : " << GetThreadPriority(back_t) << std::endl;
        }
        else if (wParam == 'C' && GetAsyncKeyState(VK_SHIFT) < 0) {
            RunNotepad();
        }
        else if (wParam == VK_RETURN) {


            srand(time(0) * 100);


            COLORREF color = RGB(my_rand(), my_rand(), my_rand());
            config.backgroundColor = color;
            WriteConfigFile();


            hBrush = CreateSolidBrush(color);

            DeleteObject((HBRUSH)SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush));

            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;
    /*case WM_PAINT: {

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
    }*/
    /*case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);

        if (delta > 0) {
            colorStep += 5;
        }
        else {
            colorStep -= 5;
        }
        wpaint(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }*/
    //case WM_RBUTTONDOWN: {
    //    RECT clientRect;
    //    GetClientRect(hwnd, &clientRect);

    //    int x = GET_X_LPARAM(lParam);
    //    int y = GET_Y_LPARAM(lParam);
    //    //std::cout << x, y;
    //    if (board.data[y * config.gridSize / clientRect.bottom][x * config.gridSize / clientRect.right] == NONE) {
    //        //
    //        board.data[y * config.gridSize / clientRect.bottom][x * config.gridSize / clientRect.right] = CIRCLES;
    //        board.pushData();
    //        //
    //        InvalidateRect(hwnd, NULL, TRUE);
    //    }
    //    return 0;
    //}
    case WM_LBUTTONDOWN: {

        /*if (WaitForSingleObject( (team == CIRCLE ? sem1 : sem2),1)== WAIT_TIMEOUT) {
            std::cout << "not your turn";
            return;
        }*/

        WaitForSingleObject(mutex, INFINITE);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        if (board.data[y * config.gridSize / clientRect.bottom][x * config.gridSize / clientRect.right] == NONE) {

            board.data[y * config.gridSize / clientRect.bottom][x * config.gridSize / clientRect.right] = CROSSES;
            board.pushData();

            InvalidateRect(hwnd, NULL, TRUE);
        }
        ReleaseMutex(mutex);
        //ReleaseSemaphore((team != CIRCLE ? sem1 : sem2),1 ,NULL);
        return 0;
    }

    }

    /* for messages that we don't deal with */
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int team = CIRCLE;

UINT change_delta = 0;
UINT delay = 50;
color_change field_color = { 0, 100, 100 };
DWORD WINAPI window_render(LPVOID lpparam) {
    HWND hwnd = *reinterpret_cast<HWND*>(lpparam);
    PAINTSTRUCT ps;
    HDC hdc, hdc_mem;
    HBITMAP hbm_mem, hbm_old;
    RECT rect = { 0 };

    hdc = GetDC(hwnd);
    GetClientRect(hwnd, &rect);
    hdc_mem = CreateCompatibleDC(hdc);
    hbm_mem = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
    hbm_old = (HBITMAP)SelectObject(hdc_mem, hbm_mem);
    ReleaseDC(hwnd, hdc);

    change_delta = field_color.h;
    while (is_render) {
        if (board.rect.right != rect.right || board.rect.bottom != rect.bottom) {
            rect = board.rect;
            SelectObject(hdc_mem, hbm_old);
            DeleteObject(hbm_mem);
            hdc = GetDC(hwnd);
            hbm_mem = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            hbm_old = (HBITMAP)SelectObject(hdc_mem, hbm_mem);
            ReleaseDC(hwnd, hdc);
        }

        field_color.h = (++change_delta / delay) % 360;
        board.h_brush = CreateSolidBrush(field_color.as_uint());
        DeleteBrush((HBRUSH)SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)board.h_brush));

        FillRect(hdc_mem, &board.rect, board.h_brush);

        wpaint(hdc_mem);

        InvalidateRect(hwnd, &board.rect, FALSE);
        hdc = BeginPaint(hwnd, &ps);
        BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdc_mem, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);

    }
    SelectObject(hdc_mem, hbm_old);
    DeleteObject(hbm_mem);
    DeleteDC(hdc_mem);
    return 0;
}

const TCHAR SZ_WIN_CLASS[] = _T("lab_5");
const TCHAR SZ_WIN_NAME[] = _T("lab_5");


int main(int argc, char** argv)
{
    
    
    
    if (__argc == 2) {
        if (atoi(__argv[1]) < 1) {
            method = method + atoi(__argv[1]);
            std::cout << method << " method ___" << '\n';
            if (!hasFile()) {
                config.gridSize = 3;
                config.windowWidth = 320;
                config.windowHeight = 240;
                config.backgroundColor = RGB(0, 0, 255);
                config.gridColor = RGB(255, 0, 0);
               
            }
            else
            {
                ReadConfigFile();
                config.ViewConfigs();
            }
        }
        else
        {
            config.gridSize = atoi(__argv[1]);
            std::cout << atoi(__argv[1]);
            if (!hasFile()) {
                config.windowWidth = 320;
                config.windowHeight = 240;
                config.backgroundColor = RGB(0, 0, 255);
                config.gridColor = RGB(255, 0, 0);
            }
            else
            {
                ReadConfigFile();
                config.gridSize = atoi(__argv[1]);
            }
            WriteConfigFile();
        }
    }
    else if (__argc == 3) {
        if (!hasFile()) {
            config.gridSize = 3;
            config.windowWidth = 320;
            config.windowHeight = 240;
            config.backgroundColor = RGB(0, 0, 255);
            config.gridColor = RGB(255, 0, 0);

        }
        else
        {
            ReadConfigFile();
            
        }
        method = method + atoi(__argv[1]);
        config.gridSize = atoi(__argv[2]);
        std::cout << atoi(__argv[2]);
        WriteConfigFile();
       
    }
    else
    {
        if (!hasFile()) {
            config.gridSize = 3;
            config.windowWidth = 320;
            config.windowHeight = 240;
            config.backgroundColor = RGB(0, 0, 255);
            config.gridColor = RGB(255, 0, 0);

        }
        else
        {
            ReadConfigFile();
        }
        WriteConfigFile();
    }
    
    config.ViewConfigs();


    board.initData();
   /* grid = new ObjectType * [config.gridSize];
    for (int i = 0; i < config.gridSize; ++i) {
        grid[i] = new ObjectType[config.gridSize];
        memset(grid[i], NONE, config.gridSize * sizeof(ObjectType));
    }*/
    mutex = CreateMutex(NULL, FALSE, "BoardMutex");
    sem1 = CreateSemaphore(NULL, 1, 1, "1sem");
    sem2 = CreateSemaphore(NULL, 0, 1, "2sem");
   

    BOOL bMessageOk;
    MSG message;            /* Here message to the application are saved */
    WNDCLASS wincl = { 0 };         /* Data structure for the windowclass */

    /* Harcode show command num when use non-winapi entrypoint */
    int nCmdShow = SW_SHOW;
    /* Get handle */
    HINSTANCE hThisInstance = GetModuleHandle(NULL);


    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = SZ_WIN_CLASS;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by Windows */

    /* Use custom brush to paint the background of the window */
    hBrush = CreateSolidBrush(config.backgroundColor);
    wincl.hbrBackground = hBrush;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClass(&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindow(
        SZ_WIN_CLASS,          /* Classname */
        SZ_WIN_NAME,       /* Title Text */
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
    back_t = CreateThread(NULL, 64 * 1024, window_render, &hwnd, 0, NULL);
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
    /*for (int i = 0; i < config.gridSize; ++i) {
        delete[] grid[i];
    }
    delete[] grid;*/
    /* Cleanup stuff */
    CloseHandle(back_t);
    DestroyWindow(hwnd);
    UnregisterClass(SZ_WIN_CLASS, hThisInstance);
    DeleteObject(hBrush);
    board.close();


    return 0;
}