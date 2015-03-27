 /* 
	$File: $
	$Date: $
	$Revision: $
	$Creator: Ashley Pringle $
	$(C) Copyright 2015 by Ashley Pringle. All Rights Reserved. $ */

#include <windows.h>
#include <stdint.h>


// static variables can only be accessed by THIS translation unit (program?), not others
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

struct win32_offscreen_buffer
{
	// NOTE pixels are always 32 bit wide Memory order BB GG RR XX
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int Pitch;
};

struct win32_window_dimension
{
	int Width;
	int Height;
};

global_variable bool Running; // global for now
global_variable win32_offscreen_buffer GlobalBackbuffer;

win32_window_dimension 
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Height = ClientRect.bottom - ClientRect.top;
	Result.Width = ClientRect.right - ClientRect.left;

	return(Result);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset)
{
	// TODO see what the optimizer does
	//int Width = Buffer.Width;
	//int Height = Buffer.Height;

	uint8 *Row = (uint8 *)Buffer.Memory;
	for(int Y = 0; Y < Buffer.Height; Y++)
	{
		uint32 *Pixel = (uint32 *)Row;
		for(int X = 0; X < Buffer.Width; X++)
		{
			uint8 Blue = (X + XOffset);
			uint8 Green = (Y + YOffset);

			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Buffer.Pitch;
	}
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
	// TODO: Bulletproof this
	// Maybe don't free first, free after, then free first if that fails

	if(Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;
	int BytesPerPixel = 4;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	// NOTE: No more DC for us
	int BitmapMemorySize = BytesPerPixel*(Buffer->Width * Buffer->Height);
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	// TODO clrear this to black
	//RenderWeirdGradient(120, 0);
	Buffer->Pitch = Width*BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer Buffer)
{
	// TODO: aspect ratio correction
	StretchDIBits(DeviceContext,
						/*
						X, Y, Width, Height,
						X, Y, Width, Height,
						*/
						0, 0, WindowWidth, WindowHeight,
						0, 0, Buffer.Width, Buffer.Height,
						Buffer.Memory,
						&Buffer.Info,
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
			win32_window_dimension Dimension = Win32GetWindowDimension(Window);
			Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackbuffer);
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

	Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

	WindowClass.style = CS_HREDRAW|CS_VREDRAW; //do REDRAWS still matter? // if you're going to stretch the window, these matter
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

				RenderWeirdGradient(GlobalBackbuffer, XOffset, YOffset);

				HDC DeviceContext = GetDC(Window);

				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackbuffer);
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