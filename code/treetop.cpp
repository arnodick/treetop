 /* 
	$File: $
	$Date: $
	$Revision: $
	$Creator: Ashley Pringle $
	$(C) Copyright 2015 by Ashley Pringle. All Rights Reserved. $ */

#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool Running; // global for now

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void 
Win32ResizeDIBSection(int Width, int Height)
{
	// TODO: Bulletproof this
	// Maybe don't free first, free after, then free first if that fails

	if(BitmapHandle)
	{
		DeleteObject(BitmapHandle);
	}
	
	if(!BitmapDeviceContext)
	{
		// TODO should we recreate these under certain special circumstances?
		BitmapDeviceContext = CreateCompatibleDC(0);
	}


	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	BitmapHandle = CreateDIBSection(
		BitmapDeviceContext,
		&BitmapInfo,
		DIB_RGB_COLORS,
		&BitmapMemory,
		0,
		0);
}

internal void
Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
	StretchDIBits(DeviceContext,
						X, Y, Width, Height,
						X, Y, Width, Height,
						BitmapMemory,
						&BitmapInfo,
						DIB_RGB_COLORS, // change this to DIB_PAL_COLORS for palette based colour
						SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
						UINT Message,
						WPARAM WParam,
						LPARAM LParam)
{
	LRESULT Result = 0;

	switch(Message)
	{
		case WM_SIZE:
		{
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			int Height = ClientRect.bottom - ClientRect.top;
			int Width = ClientRect.right - ClientRect.left;
			Win32ResizeDIBSection(Width, Height);
			//OutputDebugStringA("WM_SIZE\n");
		} break;

		case WM_CLOSE:
		{
			// handle this with message to the user
			Running = false;
			//OutputDebugStringA("WM_CLOSE\n");
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

		case WM_DESTROY:
		{
			// handle this as an error, and recreate window
			Running = false;
			//OutputDebugStringA("WM_DESTROY\n");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
			/*
			local_persist DWORD Operation = WHITENESS;
			PatBlt(DeviceContext, X, Y, Width, Height, Operation);
			if (Operation == WHITENESS)
			{
				Operation = BLACKNESS;
			}
			else
			{
				Operation = WHITENESS;
			}
			*/
			EndPaint(Window, &Paint);
		} break;
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
	WNDCLASS WindowClass = {}; // {} is null, right? nothing input

	//WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW; //do REDRAWS still matter? // removing this line glitches window when resizing it
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "TreetopWindowClass";
	//WindowClass.hIcon = 0;

	if(RegisterClass(&WindowClass))
	{
		HWND WindowHandle = 
			CreateWindowExA( // why ExA?
				0, // change this to 0 for normal behaviour
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
			Running = true;
			while(Running) // infinite loop. without this, window will open, run to end of code, and finish.
					// the for(;;) method is standard now
					// the while(1) method is another way, but is not accepted standard any more. higher warning levels will give warnings about constant in while.
			{
				MSG Message;
				BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
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