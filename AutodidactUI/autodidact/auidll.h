#ifndef AUIDLL
#include <memory>
#include <vector>
#include <dwmapi.h>
#include <gdiplus.h>
#include <WinGDI.h>

#include "adata.h"
#include "ui/dint.h"
#include "extern.h"

#ifndef __EXPORT
#define AUIDLL __declspec(dllexport)
#else
#define AUIDLL __declspec(dllimport)
#pragma comment(lib, "Autodidact.lib")
#endif
#endif // !AUIDLL