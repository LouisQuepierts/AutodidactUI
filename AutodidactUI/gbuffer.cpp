#include "gbuffer.h"

namespace Autodidact {
	GraphicsBuffer::GraphicsBuffer()
	:	width(0),
		height(0),
		bmp(0),
		graphics() {
	}

	void GraphicsBuffer::Reset(
		HWND	hwnd,
		USHORT	width,
		USHORT	height,
		DWORD	color
	) {
		this->width = width;
		this->height = height;

		if (bmp)
			DeleteObject(bmp);

		HDC hdc = GetDC(hwnd);
		bmp = CreateCompatibleBitmap(hdc, width, height);
		ReleaseDC(hwnd, hdc);

		hdc = CreateCompatibleDC(NULL);
		SelectObject(hdc, bmp);
		graphics.reset(new Gdiplus::Graphics(hdc));

		Gdiplus::SolidBrush brush(color);
		graphics->FillRectangle(&brush, 0, 0, width, height);

		this->color = color;
	}

	void GraphicsBuffer::Clear() {
		Gdiplus::SolidBrush brush(color);
		graphics->FillRectangle(&brush, 0, 0, width, height);
	}

	void GraphicsBuffer::Release() {
		if (bmp)
			DeleteObject(bmp);

		graphics.reset();
		bmp = 0;
	}

	void GraphicsBuffer::BitBlt(HDC dest, int dx, int dy, int w, int h, int sx, int sy, DWORD rop) {
		HDC hdc = graphics->GetHDC();

		if (color != 0xff000000) {
			::BitBlt(dest, dx, dy, w, h, hdc, sx, sy, rop);
		}
		else {
			::TransparentBlt(dest, dx, dy, w, h, hdc, dx, dy, w, h, 0);
		}
		
		graphics->ReleaseHDC(hdc);
	}
}
