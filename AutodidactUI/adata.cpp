#include "autodidact/adata.h"

#ifdef DEBUG
#include <iostream>
#endif // DEBUG

#define TEXT_BUFFER_SIZE 128

namespace Autodidact {
	char		_buffer_char[TEXT_BUFFER_SIZE];
	wchar_t		_buffer_wchar[TEXT_BUFFER_SIZE];

	LPWSTR GetWCharBuffer() {
		return _buffer_wchar;
	}

	LPSTR GetCharBuffer() {
		return _buffer_char;
	}
}
