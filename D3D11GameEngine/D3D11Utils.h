#pragma once

#include <Windows.h>


#if defined(DEBUG) | defined(DEBUG)
#ifndef HR
#define HR(x)												\
	{														\
		HRESULT hr = (x);									\
		if(FAILED(hr))										\
		{													\
			LPWSTR output;									\
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |		\
				FORMAT_MESSAGE_IGNORE_INSERTS 	 |			\
				FORMAT_MESSAGE_ALLOCATE_BUFFER,				\
				NULL,										\
				hr,											\
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	\
				(LPTSTR) &output,							\
				0,											\
				NULL);										\
			MessageBox(NULL, output, "Error!", MB_OK);		\
		}													\
	}
#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif 

#define ReleaseCom(x)	{ if(x) { x->Release(); x = nullptr; } }
#define Delete(x)		{ delete x; x = nullptr; }