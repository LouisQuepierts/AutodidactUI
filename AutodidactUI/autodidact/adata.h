#ifndef _AUTODIDACT_DATA_H_
#define _AUTODIDACT_DATA_H_

#ifndef __EXPORT
#define AUIDLL __declspec(dllexport)
#else
#define AUIDLL __declspec(dllimport)
#endif

#include <Windows.h>

namespace Autodidact {
	enum class AUIDLL WidgetState {
		NORMAL,
		HOVERED,
		PRESSED,
		DISABLED
	};

	AUIDLL inline LPWSTR GetWCharBuffer();

	AUIDLL inline LPSTR GetCharBuffer();
}

#endif // !_AUTODIDACT_DATA_H_
