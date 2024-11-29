#pragma once
#ifndef _AUTODIDACT_UI_WINDOW_H_
#define _AUTODIDACT_UI_WINDOW_H_

#include <Windows.h>
#include <unordered_map>

#include "gbuffer.h"
#include "architect.h"
#include "collider.h"
#include "repaintstruct.h"

namespace Autodidact {
	class Component;
	class Division;

	typedef unsigned int RESPONSE;
	typedef void (Component::* PAINTTIMERPROC)(Gdiplus::Graphics&);

	class Window : public Collider {
	public:
		AUIDLL Window(
			HINSTANCE	hInstance,
			LPCWSTR		name
		);

		AUIDLL ~Window();

		AUIDLL void SetShow(bool show = true);

		AUIDLL void SetupCaption(int height);

		AUIDLL void UpdateRect(const Gdiplus::Rect& rc);

		AUIDLL void ForceUpdate();

		AUIDLL void ForceUpdate(const Gdiplus::Rect& rc);

		AUIDLL void TryUpdate();

		AUIDLL void PushPos(const SPOINT& pos);

		AUIDLL void TranslatePos(const SPOINT& pos);

		AUIDLL void PopPos();

		AUIDLL void SetTimer(Component* component, UINT delay);

		AUIDLL void KillTimer(Component* component);

		AUIDLL void SetPaintTimer(Component* component, PAINTTIMERPROC process);

		AUIDLL void KillPaintTimer(Component* component);

		AUIDLL Division& Divide(
			const DRECT&	rc, 
			RESPONSE		response	= 5, 
			int				hit			= HTCLIENT
		);

		AUIDLL SPOINT CurrentPos();

		AUIDLL Gdiplus::ARGB GetBgColor();

		AUIDLL Gdiplus::Graphics* GetGrpahics();

		AUIDLL bool Focused();

		AUIDLL bool Maximized();

		AUIDLL void SetFocus(Component* component);

		AUIDLL void KillFocus();

		AUIDLL void TryKillFocus(Component* component);

		AUIDLL void SetMinSize(
			int width	= 600, 
			int height	= 500
		);

		AUIDLL void SetMaxSize(
			int width	= 1942, 
			int height	= 1102
		);

		AUIDLL void SetSize(
			int width,
			int height
		);

		AUIDLL void ClearHoverStack(Component* dest);

		AUIDLL void ClearPosStack();
	private:

		static LRESULT CALLBACK Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		static void CALLBACK ComponentTimerProcess(HWND, UINT, UINT_PTR, DWORD);

		static void CALLBACK PaintTimerProcess(HWND, UINT, UINT_PTR, DWORD);

	private:
		std::vector<std::unique_ptr<Division>>	divisions;

		std::queue<Gdiplus::Rect>	updateQueue;
		std::stack<SPOINT>			posStack;
		std::stack<Component*>		hoverStack;

		std::unordered_map<UINT_PTR, PAINTTIMERPROC> paint;

		Component* focus = nullptr;

		HWND	hwnd;
		HMENU	hmenu;

		GraphicsBuffer bufferStatic;
		GraphicsBuffer bufferDynamic;

		Gdiplus::ARGB	bgColor = 0xfff0f0f0;

		int		offset = 0;

		int		minWidth	= 600;
		int		minHeight	= 500;
		int		maxWidth	= 1942;
		int		maxHeight	= 1102;

		bool	tracking	= false;
		bool	maximized	= false;
		bool	focused		= true;

		enum UpdateMode {
			INVALID,
			FULLSCREEN,
			SINGLE,
			BATCHED
		} updateMode = FULLSCREEN;
	};
}

#endif // !_AUTODIDACT_UI_WINDOW_H_