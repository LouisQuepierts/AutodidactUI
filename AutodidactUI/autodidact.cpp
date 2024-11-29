#include "autodidact/autodidact.h"
#include <iostream>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "gdiplus.lib")

#include "autodidact/ui/window.h"
#include "resource.h"

#define MIN_INITIALIZE_MILIS 1000ul

namespace Autodidact {
	LRESULT hitTests[3][3] = {
		{ HTTOPLEFT,    HTCAPTION,    HTTOPRIGHT },
		{ HTLEFT,       HTNOWHERE ,     HTRIGHT },
		{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
	};

	HINSTANCE			DLLInstance = NULL;
	HICON				iCursor;
	HICON				iIcon;

	HWND				hInit;

	Resource*			pResources;

	Window*				window_main;

	enum class AppState {
		UNENABLED,
		INITIELIZE,
		ENABLED
	};
	
	AppState	_initState = AppState::UNENABLED;

	DWORD		_initTime;
	LPCWSTR		appName;
	ULONG_PTR	gdiplusToken;
	WCHAR		fontName[32];


	LRESULT CALLBACK WindowInitProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	void Initialize(
		Gdiplus::Color colorTheme,
		Gdiplus::Color colorTitleBg, 
		HINSTANCE hInst, 
		LPCWSTR name, 
		LPCWSTR iconPathICO, 
		LPCWSTR iconPathPNG
	) {
		if (_initState != AppState::UNENABLED)
			return;

		SetProcessDPIAware();

		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

		_initTime = GetTickCount();

		printf("Initialize\n");
		_initState = AppState::INITIELIZE;

		pResources = new Resource(colorTheme, colorTitleBg, hInst, iconPathICO, iconPathPNG);

		appName		= name;

		iCursor		= LoadCursor(NULL, IDC_ARROW);
		iIcon		= LoadIcon(hInst, iconPathICO);

		int width	= GetSystemMetrics(SM_CXSCREEN);
		int height	= GetSystemMetrics(SM_CYSCREEN);

		NONCLIENTMETRICS info = {};
		info.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &info, 0);
		wcscpy_s(fontName, info.lfMessageFont.lfFaceName);

		const wchar_t CLASS_NAME[] = L"WindowInitialization";

		WNDCLASSEX wnd = {};
		wnd.cbSize			= sizeof(WNDCLASSEX);
		wnd.style			= CS_HREDRAW | CS_VREDRAW;
		wnd.cbClsExtra		= 0;
		wnd.cbWndExtra		= 0;
		wnd.hInstance		= DLLInstance;
		wnd.hIcon			= iIcon;
		wnd.hIconSm			= NULL;
		wnd.hbrBackground	= NULL;
		wnd.hCursor			= NULL;
		wnd.lpfnWndProc		= WindowInitProc;
		wnd.lpszMenuName	= NULL;
		wnd.lpszClassName	= CLASS_NAME;

		RegisterClassEx(&wnd);

		hInit = CreateWindowEx(
			WS_EX_TOPMOST, CLASS_NAME, appName, WS_POPUP | WS_CAPTION | WS_VISIBLE | DS_CENTER,
			(width - 500) / 2, (height - 310) / 2,
			500, 310,
			NULL, NULL, DLLInstance, NULL
		);

		window_main = new Window(hInst, name);
		window_main->SetupCaption(60);
	}

	void PostInitialize() {
		if (_initState != AppState::INITIELIZE)
			return;

		DWORD time = GetTickCount() - _initTime;

		if (time < MIN_INITIALIZE_MILIS) {
			Sleep(MIN_INITIALIZE_MILIS - time);
		}

		_initState = AppState::ENABLED;

		DestroyWindow(hInit);
		window_main->SetShow();
	}

	void CleanUp() {
		delete window_main;

		delete pResources;

		DeleteObject(iCursor);
		DeleteObject(iIcon);

		Gdiplus::GdiplusShutdown(gdiplusToken);
	}

	LPCWCHAR GetFontName() {
		return fontName;
	}

	LRESULT CALLBACK WindowInitProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		switch (msg) {
		case WM_CREATE: {
			MARGINS margin{ 0, 0, 0, 1 };
			DwmExtendFrameIntoClientArea(hwnd, &margin);

			SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
				SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

			COLORREF color = 0xffffff;
			DwmSetWindowAttribute(hwnd, 34, &color, sizeof(COLORREF));
			break;
		}
		case WM_NCCALCSIZE: {
			if (wparam == TRUE) {
				SetWindowLong(hwnd, 0, 0);
				return TRUE;
			}
			return FALSE;
		}
		case WM_SETCURSOR: {
			SetCursor(iCursor);
			return TRUE;
		}
		case WM_ERASEBKGND:
			return 1;
		case WM_NCPAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			LRESULT ret = DefWindowProcW(hwnd, msg, wparam, lparam);

			Gdiplus::Graphics graphics{ hdc };
			Gdiplus::SolidBrush brush(Gdiplus::ARGB(0xffffffff));
			int x = ps.rcPaint.left;
			int y = ps.rcPaint.top;
			int width = ps.rcPaint.right - ps.rcPaint.left;
			int height = ps.rcPaint.bottom - ps.rcPaint.top;
			graphics.FillRectangle(&brush, x, y, width, height);

			Gdiplus::Region rgn(CreateRoundRectRgn(10, 10, 136, 40, 5, 5));
			brush.SetColor(pResources->colorTitle);

			graphics.FillRegion(&brush, &rgn);

			{
				Gdiplus::Font font(fontName, 14, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);

				brush.SetColor(pResources->colorTheme);

				graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
				graphics.DrawString(TEXT(AUTODIDACT_VERSION), 13, &font, Gdiplus::PointF{21, 15}, &brush);
			}

			{
				Gdiplus::Font font(fontName, 17, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

				brush.SetColor(Gdiplus::ARGB(0xff757575));

				int length = wsprintf(GetWCharBuffer(), L"ÕýÔÚÆô¶¯ Autodidact %ls...", appName);
				graphics.DrawString(GetWCharBuffer(), length, &font, Gdiplus::PointF{10, 278}, &brush);
			}

			graphics.DrawImage(pResources->bIcon.get(), 190, 95, 110, 110);

			EndPaint(hwnd, &ps);
			return ret;
		}
		case WM_DESTROY: {
			break;
		}
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}

		return 0;
	}

	HINSTANCE GetInstance() {
		return DLLInstance;
	}

	void DrawButton(
		Gdiplus::Graphics& graphics, 
		Autodidact::ButtonType type, 
		int x, 
		int y
	) {
		Gdiplus::Image* bmp;

		switch (type) {
		default:
		case Autodidact::ButtonType::MINIMIZE:
			bmp = pResources->bMinimize.get();
			break;
		case Autodidact::ButtonType::MAXIMIZE:
			bmp = pResources->bMaximize.get();
			break;
		case Autodidact::ButtonType::RESTORE:
			bmp = pResources->bRestore.get();
			break;
		case Autodidact::ButtonType::CLOSE:
			bmp = pResources->bClose.get();
			break;
		}

		graphics.DrawImage(bmp, x, y, 16, 16);
	}

	void DrawButton(
		Gdiplus::Graphics& graphics, 
		Autodidact::ButtonType type, 
		Gdiplus::ColorMatrix& matrix, 
		int x, 
		int y
	) {
		Gdiplus::ImageAttributes imageAttribute;
		imageAttribute.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

		Gdiplus::Image* bmp;

		switch (type) {
		default:
		case Autodidact::ButtonType::MINIMIZE:
			bmp = pResources->bMinimize.get();
			break;
		case Autodidact::ButtonType::MAXIMIZE:
			bmp = pResources->bMaximize.get();
			break;
		case Autodidact::ButtonType::RESTORE:
			bmp = pResources->bRestore.get();
			break;
		case Autodidact::ButtonType::CLOSE:
			bmp = pResources->bClose.get();
			break;
		}

		graphics.DrawImage(bmp, Gdiplus::Rect{ x, y, 16, 16 }, 0, 0, 16, 16, Gdiplus::UnitPixel, &imageAttribute);
	}

	Window& GetAPPWindow() {
		return *window_main;
	}

	Resource::Resource(
		Gdiplus::Color colorTheme,
		Gdiplus::Color colorTitle,
		HINSTANCE hInst,
		LPCWSTR iconPathICO,
		LPCWSTR iconPathPNG
	) :
		bMinimize(LoadPNGFromResource(DLLInstance, MAKEINTRESOURCE(IDB_MINIMIZE))),
		bMaximize(LoadPNGFromResource(DLLInstance, MAKEINTRESOURCE(IDB_MAXIMIZE))),
		bRestore(LoadPNGFromResource(DLLInstance, MAKEINTRESOURCE(IDB_RESTORE))),
		bClose(LoadPNGFromResource(DLLInstance, MAKEINTRESOURCE(IDB_CLOSE))),
		bIcon(LoadPNGFromResource(hInst, iconPathPNG)),
		colorTheme(colorTheme),
		colorTitle(colorTitle) {}

	Gdiplus::Bitmap* LoadPNGFromResource(HINSTANCE hInst, LPCWSTR id) {
		if (id == nullptr) {
			return nullptr;
		}
		static const LPCWSTR type = L"PNG";

		using Gdiplus::Bitmap;

		IStream* pStream = nullptr;
		Bitmap* pBmp = nullptr;
		HGLOBAL		hGlobal = nullptr;

		HRSRC hrsrc = FindResourceW(hInst, id, type);

		if (!hrsrc) {
			return nullptr;
		}

		DWORD dResourceSize = SizeofResource(hInst, hrsrc);

		if (dResourceSize == 0) {
			return nullptr;
		}

		HGLOBAL hGlobalResource = LoadResource(hInst, hrsrc);

		if (!hGlobalResource) {
			return nullptr;
		}

		void* imagebytes = LockResource(hGlobalResource);
		hGlobal = ::GlobalAlloc(GHND, dResourceSize);

		if (!hGlobal) {
			return nullptr;
		}

		void* pBuffer = ::GlobalLock(hGlobal);

		if (pBuffer) {
			memcpy(pBuffer, imagebytes, dResourceSize);
			HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);

			if (SUCCEEDED(hr)) {
				hGlobal = nullptr;
				pBmp = new Gdiplus::Bitmap(pStream);
			}
		}

		if (pStream) {
			pStream->Release();
			pStream = nullptr;
		}

		if (hGlobal) {
			::GlobalFree(hGlobal);
		}

		return pBmp;
	}

	const Resource& GetResource() {
		return *pResources;
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		Autodidact::DLLInstance = hModule;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}