 /* 
	$File: $
	$Date: $
	$Revision: $
	$Creator: Ashley Pringle $
	$(C) Copyright 2015 by Ashley Pringle. All Rights Reserved. $ */

#include <windows.h>

LRESULT CALLBACK
MainWindowCallback(	HWND Window,
					UINT Message,
					WPARAM WParam,
					LPARAM LParam)
{
	LRESULT Result = 0;

	switch(Message)
	{
		case WM_SIZE:
		{
			OutputDebugStringA("WM_SIZE\n");
		} break;

		case WM_DESTROY:
		{
			OutputDebugStringA("WM_DESTROY\n");
		} break;

		case WM_CLOSE:
		{
			OutputDebugStringA("WM_CLOSE\n");
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			static DWORD Operation = WHITENESS;
			PatBlt(DeviceContext, X, Y, Width, Height, Operation);
			if (Operation == WHITENESS)
			{
				Operation = BLACKNESS;
			}
			else
			{
				Operation = WHITENESS;
			}
			EndPaint(Window, &Paint);
		}break;
		//SetPixel(DeviceContext, 100, 100, RGB(255, 0, 255)); // in theory puts a pixel on the screen at x == 100 y == 100, in the window DeviceContext(?) but can't see it on my screen
		default:
		{
			//OutputDebugStringA("default\n");
			Result = DefWindowProc(Window, Message, WParam, LParam);
		} break;
	}

	return(Result);
}

int CALLBACK 
WinMain(HINSTANCE Instance,
		HINSTANCE PrevInstance,
		LPSTR CommandLine,
		int ShowCode)
{
	//MessageBox (0, "pls answer truthfully", "ARE U A BUTTFACE?", MB_YESNO|MB_ICONEXCLAMATION);
	WNDCLASS WindowClass = {};

	//WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW; //do REDRAWS still matter? // removing this line glitches window when resizing it
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "TreetopWindowClass";
	//WindowClass.hIcon = 0;

	if(RegisterClass(&WindowClass))
	{
		HWND WindowHandle = 
			CreateWindowEx(
				0,
		  		WindowClass.lpszClassName,
				"Treetop Fighter",
				WS_OVERLAPPEDWINDOW|WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				Instance,
				0);
		if(WindowHandle)
		{
			for(;;) // infinite loop. without this, window will open, run to end of code, and finish.
			{
				MSG Message;
				BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
				if (MessageResult > 0)
				{
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			// TODO
		}		
	}
	else
	{
		// TODO
	}

	return(0);
}