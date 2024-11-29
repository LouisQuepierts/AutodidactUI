#pragma once
#ifndef _AUTODIDACT_GBUFFER_H_
#define _AUTODIDACT_GBUFFER_H_

#include <Windows.h>
#include <gdiplus.h>
#include <memory>

namespace Autodidact {
	class GraphicsBuffer {
	public:
		GraphicsBuffer();

		void Reset(
			HWND	hwnd, 
			USHORT	width, 
			USHORT	height,
			DWORD	color	= 0
		);

		void Clear();

		void Release();

		void BitBlt(
			HDC dest, 
			int dx, int dy, 
			int w,  int h, 
			int sx, int sy, 
			DWORD rop
		);
	public:
		std::unique_ptr<Gdiplus::Graphics> graphics;
		HBITMAP bmp;

		USHORT	width;
		USHORT	height;
		DWORD	color;
	};
}

#endif