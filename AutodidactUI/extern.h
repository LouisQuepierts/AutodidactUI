#ifndef EXTERN_H
#define EXTERN_H

#include <memory>

namespace Autodidact {
	extern HINSTANCE			DLLInstance;
	extern HICON				iCursor;
	extern HICON				iIcon;

	extern DWORD				_initTime;
	extern LPCWSTR				appName;
	extern WCHAR				fontName[32];

	extern LRESULT				hitTests[3][3];

	struct Resource {
		std::unique_ptr<Gdiplus::Bitmap>	bMinimize;
		std::unique_ptr<Gdiplus::Bitmap>	bMaximize;
		std::unique_ptr<Gdiplus::Bitmap>	bRestore;
		std::unique_ptr<Gdiplus::Bitmap>	bClose;

		std::unique_ptr<Gdiplus::Bitmap>	bIcon;

		const Gdiplus::Color				colorTheme;
		const Gdiplus::Color				colorTitle;

		Resource(
			Gdiplus::Color,
			Gdiplus::Color,
			HINSTANCE,
			LPCWSTR,
			LPCWSTR
		);
	};

	extern Resource* pResources;

#ifndef LOADPNG
#define LOADPNG
	Gdiplus::Bitmap* LoadPNGFromResource(HINSTANCE hInst, LPCWSTR id);
#endif // !LOADPNG
}

#endif // !EXTERN_H
