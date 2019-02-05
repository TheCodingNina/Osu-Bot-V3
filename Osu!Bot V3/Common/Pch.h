// Pch.h : include file for standard system include files,
// or project specific include files that are use frequently,
// but are changed infrequently.

#pragma once

#include <Common/Targetver.h>

// Exclude rarely-used stuff from windows headers.
#define WIN32_LEAN_AND_MEAN
// Windows header files:
#include <Windows.h>
#include <wrl/client.h>
#include <wincodec.h>

// C RunTime header files:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>

// TODO: Reference additional header files:
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <DirectXMath.h>

#include <string>
#include <vector>
#include <thread>

#include <Content/Resources/Resource.h>
#include <Common/Size.h>
#include <Common/StepTimer.h>


// Usefull functions for all files to include:
inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw hr;
	}
}
