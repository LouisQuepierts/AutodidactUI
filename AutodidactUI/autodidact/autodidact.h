#ifndef AUTODIDACT_H
#define AUTODIDACT_H

#define LOADPNG

#include "auidll.h"
#include "ui/window.h"

#ifndef AUTODIDACT_VERSION
#define AUTODIDACT_VERSION "Autodidact 24"
#endif // !AUTODIDACT_VERSION

namespace Autodidact {
	enum class AUIDLL ButtonType {
		MINIMIZE,
		MAXIMIZE,
		RESTORE,
		CLOSE
	};

	AUIDLL inline HINSTANCE GetInstance();

	AUIDLL void DrawButton(
		Gdiplus::Graphics& graphics, 
		Autodidact::ButtonType type, 
		int x, 
		int y)
		;

	AUIDLL void DrawButton(
		Gdiplus::Graphics& graphics, 
		Autodidact::ButtonType type, 
		Gdiplus::ColorMatrix& matrix, 
		int x, 
		int y
	);

	AUIDLL void Initialize(
		Gdiplus::Color colorTheme, 
		Gdiplus::Color colorTitleBg, 
		HINSTANCE hInst, 
		LPCWSTR name, 
		LPCWSTR iconPathICO,
		LPCWSTR iconPathPNG
	);

	AUIDLL void PostInitialize();

	AUIDLL void CleanUp();

	AUIDLL Window& GetAPPWindow();

	AUIDLL const Resource& GetResource();

	AUIDLL LPCWCHAR GetFontName();

	AUIDLL Gdiplus::Bitmap* LoadPNGFromResource(HINSTANCE hInst, LPCWSTR id);
}

#endif // !AUTODIDACT_H