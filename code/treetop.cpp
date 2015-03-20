 /* 
	$File: $
	$Date: $
	$Revision: $
	$Creator: Ashley Pringle $
	$(C) Copyright 2015 by Ashley Pringle. All Rights Reserved. $ */

#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

global_variable bool Running; // global for now

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;

int BytesPerPixel = 4;

internal void
RenderWeirdGradient(int XOffset, int YOffset)
{
	int Width = BitmapWidth;
	int Height = BitmapHeight;
	
	int Pitch = Width*BytesPerPixel;
	uint8 *Row = (uint8 *)BitmapMemory;
	for(int Y = 0; Y < BitmapHeight; Y++)
	{
		uint32 *Pixel = (uint32 *)Row;
		for(int X = 0; X < BitmapWidth; X++)
		{
			uint8 Blue = (X + XOffset);
			uint8 Green = (Y + YOffset);

			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Pitch;
	}
}

internal void 
Win32ResizeDIBSection(int Width, int Height)
{
	// TODO: Bulletproof this
	// Maybe don't free first, free after, then free first if that fails

	if(BitmapMemory)
	{
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
	}

	BitmapWidth = Width;
	BitmapHeight = Height;

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	// NOTE: No more DC for us
	int BitmapMemorySize = BytesPerPixel*(BitmapWidth * BitmapHeight);
	BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	// TODO clrear this to black
	//RenderWeirdGradient(120, 0);
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height)
{
	int WindowWidth = ClientRect->right - ClientRect->left;
	int WindowHeight = ClientRect->bottom - ClientRect->top;
	StretchDIBits(DeviceContext,
						/*
						X, Y, Width, Height,
						X, Y, Width, Height,
						*/
						0, 0, BitmapWidth, BitmapHeight,
						0, 0, WindowWidth, WindowHeight,
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

			RECT ClientRect;
			GetClientRect(Window, &ClientRect);

			Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
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
		HWND Window = 
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
		if(Window)
		{
			int XOffset = 0;
			int YOffset = 0;

			Running = true;
			while(Running) // infinite loop. without this, window will open, run to end of code, and finish.
					// the for(;;) method is standard now
					// the while(1) method is another way, but is not accepted standard any more. higher warning levels will give warnings about constant in while.
			{

				MSG Message;
				while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if(Message.message == WM_QUIT)
					{
						Running = false;
					}
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}

				RenderWeirdGradient(XOffset, YOffset);

				HDC DeviceContext = GetDC(Window);
				RECT ClientRect;
				GetClientRect(Window, &ClientRect);
				int WindowWidth = ClientRect.right - ClientRect.left;
				int WindowHeight = ClientRect.bottom - ClientRect.top;
				Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
				ReleaseDC(Window, DeviceContext);

				++XOffset;

				/*
				BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
				if (MessageResult > 0)
				{
					
				}
				else
				{
					break;
				}
				*/
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