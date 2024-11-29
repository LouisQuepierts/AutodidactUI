#pragma once
#ifndef _AUTODIDACT_UI_COMPONENT_H_
#define _AUTODIDACT_UI_COMPONENT_H_

typedef unsigned int RESPONSE;

#define RESPONSE_MOUSE	1
#define RESPONSE_CLICK	2
#define RESPONSE_RESIZE	4
#define RESPONSE_FOCUS	8

#define RESPONSE_INTERACTABLE	RESPONSE_MOUSE | RESPONSE_CLICK

#define CheckResponse(_iR, _pR) (_iR) & (_pR)

#define PRESSED_CONRTOL(wparam) (wparam & MK_CONTROL)
#define PRESSED_LBUTTON(wparam) (wparam & MK_LBUTTON)
#define PRESSED_RBUTTON(wparam) (wparam & MK_RBUTTON)
#define PRESSED_SHIFT(wparam)	(wparam & MK_SHIFT)

#include "collider.h"
#include "repaintstruct.h"

namespace Autodidact {
	typedef bool (*Filter)(Component*);

	class AUIDLL Component : public Collider {
	public:
		typedef void (*PaintProcess)(Component*, Gdiplus::Graphics&);

	public:
		enum State {
			NORMAL,
			HOVERED,
			PRESSED
		};

	public:
		Component(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			DSIZE		height,
			Component&	parent,
			RESPONSE	response	= RESPONSE_MOUSE | RESPONSE_RESIZE,
			int			hit			= HTCLIENT
		);

		Component(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			DSIZE		height,
			Window&		window,
			RESPONSE	response	= RESPONSE_MOUSE | RESPONSE_RESIZE,
			int			hit			= HTCLIENT
		);

		Component(
			const DRECT&	drect,
			Component&		parent,
			RESPONSE		response	= RESPONSE_MOUSE | RESPONSE_RESIZE,
			int				hit			= HTCLIENT
		);

		Component(
			const DRECT&	drect,
			Window&			window,
			RESPONSE		response	= RESPONSE_MOUSE | RESPONSE_RESIZE,
			int				hit			= HTCLIENT
		);
		
		virtual ~Component();

		virtual bool GetHittedComponents(const POINT& pt, std::queue<Component*>& hitted);

		virtual void LButtonDown(WPARAM wparam, const POINT& pt);

		virtual void LButtonUp(WPARAM wparam, const POINT& pt);

		virtual void MButtonDown(WPARAM wparam, const POINT& pt);

		virtual void MButtonUp(WPARAM wparam, const POINT& pt);

		virtual void RButtonDown(WPARAM wparam, const POINT& pt);

		virtual void RButtonUp(WPARAM wparam, const POINT& pt);

		virtual void MouseMove(WPARAM wparam, const POINT& pt);

		virtual void MouseEnter();

		virtual void MouseLeave();

		virtual void MouseWheelH(WPARAM wparam);

		virtual void MouseWheelV(WPARAM wparam);

		virtual void CharInput(wchar_t c);

		virtual void KeyDown(WPARAM wparam, LPARAM lparam);

		virtual void KeyUp(WPARAM wparam, LPARAM lparam);

		virtual void OnResize(int nWidth, int nHeight, Gdiplus::ARGB eraseColor, std::queue<REPAINTSTRUCT>* repaint);

		virtual void GetUpdateList(OUT std::queue<Component*>& update);

		virtual void GetUpdateList(Filter filter, OUT std::queue<Component*>& update);

		virtual void OnTimer();

		virtual void OnAnimate(Gdiplus::Graphics& graphics);

		virtual void OnSetFocus();

		virtual void OnKillFocus();

		void UpdateBuffer();

		virtual void UpdateBufferChained();

		int GetHitResult();

		bool IsClient();

		State GetState();

	protected:
		virtual void OnPaint(Gdiplus::Graphics& graphics);

	protected:
		Window& window;
		State	state = NORMAL;

	public:
		const int		hittest;

		const RESPONSE	response;

		const UINT_PTR	parent;

		bool			static_ = false;
		bool			enable  = true;
	};

	class ListComponent : public Component {
	public:
		using Component::Component;

		AUIDLL void AddComponent(Component* component, bool update = true);

		AUIDLL virtual bool GetHittedComponents(const POINT& pt, std::queue<Component*>& hitted) override;

		AUIDLL virtual void OnResize(int nWidth, int nHeight, Gdiplus::ARGB eraseColor, std::queue<REPAINTSTRUCT>* repaint) override;

		AUIDLL virtual void GetUpdateList(OUT std::queue<Component*>& update) override;

		AUIDLL virtual void GetUpdateList(Filter filter, OUT std::queue<Component*>& update) override;

		AUIDLL virtual void MouseEnter() override;

		AUIDLL virtual void MouseLeave() override;

		AUIDLL virtual void LButtonDown(WPARAM wparam, const POINT& pt) override;

		AUIDLL virtual void LButtonUp(WPARAM wparam, const POINT& pt) override;

		AUIDLL Component& operator[] (const UINT index);

	protected:
		AUIDLL void OnPaint(Gdiplus::Graphics& graphics) override;

	public:
		DWORD	color = NULL;

	protected:
		std::vector<std::unique_ptr<Component>> components = std::vector<std::unique_ptr<Component>>();
	};

	class PaintComponent : public Component {
	public:
		AUIDLL PaintComponent(
			DPOS		x,
			DPOS		y,
			DSIZE		width,
			DSIZE		height,
			Component&	parent,
			PaintProcess	paint,
			RESPONSE	response = RESPONSE_MOUSE | RESPONSE_RESIZE,
			int			hit = HTCLIENT
		);

	protected:
		void OnPaint(Gdiplus::Graphics& graphics) override;

	private:
		PaintProcess	paint;
	};
}

#endif // !_AUTODIDACT_UI_COMPONENT_H_