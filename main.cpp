#include <windows.h>

#include <math.h>

//Создаем макет функции окна, которая будет определена ниже

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

char szProgName[] = "3pnt";

int points[3][3] = {};

int check = 0;

int xView, yView, max, min, length;

double y, a, b, c;
HWND hWnd;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)

{


	MSG lpMsg;

	WNDCLASS w; 

	w.lpszClassName = szProgName;

	w.hInstance = hInstance;

	w.lpfnWndProc = WndProc;

	w.hCursor = LoadCursor(NULL, IDC_ARROW);

	w.hIcon = 0;

	w.lpszMenuName = 0;

	w.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	w.style = CS_HREDRAW | CS_VREDRAW;

	w.cbClsExtra = 0;

	w.cbWndExtra = 0;

	if (!RegisterClass(&w)) return 0;


                HMENU hMainMenu = CreateMenu();
                HMENU hPopMenuFile = CreatePopupMenu();
 
                AppendMenu(hMainMenu, MF_STRING | MF_POPUP, (UINT)hPopMenuFile, "Файл");
				AppendMenu(hMainMenu, MF_STRING, 1004, "Новое задание");
                AppendMenu(hMainMenu, MF_STRING, 1001, "О программе");
                AppendMenu(hPopMenuFile, MF_STRING , 1002, "Сохранить");
				AppendMenu(hPopMenuFile, MF_STRING , 1003, "Выход");

                SetMenu(hWnd, hMainMenu);
                SetMenu(hWnd, hPopMenuFile);


	hWnd = CreateWindow(szProgName, "Треугольник", WS_OVERLAPPEDWINDOW, 100, 100, 500, 400, NULL, hMainMenu, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);

	UpdateWindow(hWnd);

	while (GetMessage(&lpMsg, 0, 0, 0)) 
	{

		TranslateMessage(&lpMsg);

		DispatchMessage(&lpMsg);

	}

	return(lpMsg.wParam);

}

int CaptureAnImage(HWND hWnd)
					{
						HDC hdcWindow;
						HDC hdcMemDC = NULL;
						HBITMAP hbmScreen = NULL;
						BITMAP bmpScreen;
					 
						// Retrieve the handle to a display device context for the client 
						// area of the window. 
						hdcWindow = GetDC(hWnd);
					 
						// Create a compatible DC which is used in a BitBlt from the window DC
						hdcMemDC = CreateCompatibleDC(hdcWindow);
					 
						if (!hdcMemDC)
						{
							MessageBox(hWnd, "CreateCompatibleDC has failed", "Failed", MB_OK);
							goto done;
						}
					 
						// Get the client area for size calculation
						RECT rcClient;
						GetClientRect(hWnd, &rcClient);
					 
						//This is the best stretch mode
						SetStretchBltMode(hdcWindow, HALFTONE);
					 
					 
						// Create a compatible bitmap from the Window DC
						hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
					 
						if (!hbmScreen)
						{
							MessageBox(hWnd, "CreateCompatibleBitmap Failed", "Failed", MB_OK);
							goto done;
						}
					 
						// Select the compatible bitmap into the compatible memory DC.
						SelectObject(hdcMemDC, hbmScreen);
					 
						// Bit block transfer into our compatible memory DC.
						if (!BitBlt(hdcMemDC,
							0, 0,
							rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
							hdcWindow,
							0, 0,
							SRCCOPY))
						{
							MessageBox(hWnd, "BitBlt has failed", "Failed", MB_OK);
							goto done;
						}
					 
						// Get the BITMAP from the HBITMAP
						GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);
					 
						BITMAPFILEHEADER   bmfHeader;
						BITMAPINFOHEADER   bi;
					 
						bi.biSize = sizeof(BITMAPINFOHEADER);
						bi.biWidth = bmpScreen.bmWidth;
						bi.biHeight = bmpScreen.bmHeight;
						bi.biPlanes = 1;
						bi.biBitCount = 32;
						bi.biCompression = BI_RGB;
						bi.biSizeImage = 0;
						bi.biXPelsPerMeter = 0;
						bi.biYPelsPerMeter = 0;
						bi.biClrUsed = 0;
						bi.biClrImportant = 0;
					 
						DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;
					 
						// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
						// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
						// have greater overhead than HeapAlloc.
						HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
						char *lpbitmap = (char *)GlobalLock(hDIB);
					 
						// Gets the "bits" from the bitmap and copies them into a buffer 
						// which is pointed to by lpbitmap.
						GetDIBits(hdcWindow, hbmScreen, 0,
							(UINT)bmpScreen.bmHeight,
							lpbitmap,
							(BITMAPINFO *)&bi, DIB_RGB_COLORS);
					 
						// A file is created, this is where we will save the screen capture.
						HANDLE hFile = CreateFile("save.bmp",
							GENERIC_WRITE,
							0,
							NULL,
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL, NULL);
					 
						// Add the size of the headers to the size of the bitmap to get the total file size
						DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
					 
						//Offset to where the actual bitmap bits start.
						bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
					 
						//Size of the file
						bmfHeader.bfSize = dwSizeofDIB;
					 
						//bfType must always be BM for Bitmaps
						bmfHeader.bfType = 0x4D42; //BM   
					 
						DWORD dwBytesWritten = 0;
						WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
						WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
						WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
					 
						//Unlock and Free the DIB from the heap
						GlobalUnlock(hDIB);
						GlobalFree(hDIB);
					 
						//Close the handle for the file that was created
						CloseHandle(hFile);
					 
						//Clean up
					done:
						DeleteObject(hbmScreen);
						DeleteObject(hdcMemDC);
						ReleaseDC(hWnd, hdcWindow);
					 
						return 0;
					}



BOOL Line(HDC hdc, int x1, int y1, int x2, int y2);

BOOL Circle(HDC hdc, int x0, int y0, int r);

void InscribedCircle(HDC hdc);

void СircumscribedCircle(HDC hdc);

bool CrossPoint(double &x, double &y, double x1, double y1, double ax, double ay, double x2, double y2, double bx, double by);

LRESULT CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)

{

HDC hdc;

PAINTSTRUCT ps;

static HPEN hPenRed;

static HPEN hPenWhite;

HPEN hOldPen;

hPenRed = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

hPenWhite = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

int lineSize = 4;







switch (messg)

{

case WM_PAINT:

	hdc = BeginPaint(hWnd, &ps);

	hOldPen = (HPEN)SelectObject(hdc, hPenRed);

	for (int i(0); i < check; i++)

	{

		Line(hdc, points[i][0] - lineSize + 1, points[i][1] - lineSize + 1, points[i][0] + lineSize, points[i][1] + lineSize);

		Line(hdc, points[i][0] + lineSize - 1, points[i][1] - lineSize + 1, points[i][0] - lineSize, points[i][1] + lineSize);

	}

	if (check > 1)

	{

		Line(hdc, points[0][0], points[0][1], points[1][0], points[1][1]);

		if (check > 2)

		{

			Line(hdc, points[1][0], points[1][1], points[2][0], points[2][1]);

			Line(hdc, points[2][0], points[2][1], points[0][0], points[0][1]);

			InscribedCircle(hdc);

			СircumscribedCircle(hdc);

		}

	}

SelectObject(hdc, hOldPen);

EndPaint(hWnd, &ps);

break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
		{
			case 1003:
				exit(0);
            break;
			case 1001:
				MessageBox(hWnd, TEXT("Треугольник"), TEXT("О программе"), 0);
            break;
			case 1004:

				InvalidateRect(hWnd, 0, TRUE);
				//тут должна быть процедура очистки окна


            break;
			case 1002:
				CaptureAnImage(hWnd);


        }
        break;



case WM_LBUTTONDOWN:

if (check < 3)

{

points[check][0] = LOWORD(lParam);

points[check][1] = HIWORD(lParam);

check++;

InvalidateRect(hWnd, 0, TRUE);

}

break;

case WM_RBUTTONDOWN:

if (check > 0)

{

check=0;

InvalidateRect(hWnd, 0, TRUE);

}

break;

//сообщение выхода - разрушение окна

case WM_DESTROY:

PostQuitMessage(0); //Посылаем сообщение выхода с кодом 0 - нормальное завершение

break;

default:

return(DefWindowProc(hWnd, messg, wParam, lParam)); //освобождаем очередь приложения от нераспознаных

}



return 0;

}

BOOL Line(HDC hdc, int x1, int y1, int x2, int y2)

{

MoveToEx(hdc, x1, y1, NULL); //сделать текущими координаты x1, y1

return LineTo(hdc, x2, y2); //нарисовать линию

}

BOOL Circle(HDC hdc, int x0, int y0, int r)

{

return Arc(hdc, x0-r, y0-r, x0+r, y0+r, 0, 0, 0, 0);

}

// посчитать координаты центра и радиус вписанной окружности

void InscribedCircle(HDC hdc)

{

double x,w,z,t,h,f;

x = points[0][0] - points[1][0];w = points[0][1] - points[1][1];

z = points[0][0] - points[2][0]; t = points[0][1] - points[2][1];

h = points[1][0] - points[2][0];f = points[1][1] - points[2][1];

// стороны треугольника

double a = sqrt(pow(x, 2) + pow(w, 2));

double b = sqrt(pow(z, 2) + pow(t, 2));

double c = sqrt(pow(h, 2) + pow(f, 2));

// вектор биссектрисы точки 1 (вектор)

double dx1 = (points[1][0] - points[0][0]) / a + (points[2][0] - points[0][0]) / b;

double dy1 = (points[1][1] - points[0][1]) / a + (points[2][1] - points[0][1]) / b;

// вектор биссектрисы точки 2 (вектор)

double dx2 = (points[0][0] - points[1][0]) / a + (points[2][0] - points[1][0]) / c;

double dy2 = (points[0][1] - points[1][1]) / a + (points[2][1] - points[1][1]) / c;

// точка пересечения двух биссектрис определяет центр

double x0, y0;

CrossPoint(x0, y0, points[0][0], points[0][1], dx1, dy1, points[1][0], points[1][1], dx2, dy2);

// полупериметр

double p = (a + b + c) / 2;

// площадь

double s = sqrt(p*(p - a)*(p - b)*(p - c));

// радиус

double r = s/p;

Circle(hdc, (int)x0,(int)y0, (int)r);

}

// посчитать координаты центра и радиус описанной окружности

void СircumscribedCircle(HDC hdc)

{

// центр стороны 1 (точка)

double cx1 = (points[0][0] + points[1][0]) / 2.0;

double cy1 = (points[0][1] + points[1][1]) / 2.0;

// перпендикуляр к стороне 1 (вектор)

int dx1 = (points[0][1] - points[1][1]);

int dy1 = -(points[0][0] - points[1][0]);

// центр стороны 2 (точка)

double cx2 = (points[0][0] + points[2][0]) / 2.0;

double cy2 = (points[0][1] + points[2][1]) / 2.0;

// перпендикуляр к стороне 2 (вектор)

int dx2 = (points[0][1] - points[2][1]);

int dy2 = -(points[0][0] - points[2][0]);

// точка пересечения двух перпендикуляров определяет центр

double x0, y0;

CrossPoint(x0, y0, cx1, cy1, dx1, dy1, cx2, cy2, dx2, dy2);

// стороны треугольника

double a = sqrt(pow((double)dx1, 2) + pow((double)dy1, 2));

double b = sqrt(pow((double)dx2, 2) + pow((double)dy2, 2));

double c = sqrt(pow((double)points[1][0] - (double)points[2][0], 2) + pow((double)points[1][1] - (double)points[2][1], 2));

// полупериметр

double p = (a + b + c) / 2;

// площадь

double s = sqrt(p*(p - a)*(p - b)*(p - c));

// радиус

double r = a*b*c / (4 * s);

Circle(hdc, (int)x0, (int)y0, (int)r);

}

bool CrossPoint(double &x, double &y, double x1, double y1, double ax, double ay, double x2, double y2, double bx, double by)

{

double den = by*ax - ay*bx; // -a2*b1 + a1*b2

if ((den > -0.00001) && (den < 0.00001))

return false; // прямые параллельны

double c1 = ay*x1 - ax*y1; // -c = a*x0 + b*y0

double c2 = by*x2 - bx*y2; // -c = a*x0 + b*y0

x = (c2*ax - c1*bx) / den; // -(-c2*b1 + c1*b2) / den

y = (c2*ay - c1*by) / den; // -(c2*a1 - c1*a2) / den

return true;

}
