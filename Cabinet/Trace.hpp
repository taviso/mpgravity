///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Author: Elmü (www.netcult.ch/elmue)
// Date: 18-03-2008
//
// Trace debug output
//
// User Debugview from www.sysinternals.com to see this output
// This application MUST be started with CTRL + F5 from VisualStudio otherwise you will see nothing
//

#pragma once

#pragma warning(disable: 4793)

namespace Cabinet
{
	// Change FALSE into TRUE to enable tracing
	#define _TraceExtract  (_DEBUG && FALSE) // Extraction
	#define _TraceCompress (_DEBUG && FALSE) // Compression
	#define _TraceInternet (_DEBUG && FALSE) // Internet
	#define _TraceCache    (_DEBUG && FALSE) // Internet Caches
	
#if _DEBUG
	static void TraceW(const WCHAR* u16_Format, ...)
	{
		WCHAR u16_Buf[5001];
		va_list  args;
		va_start(args, u16_Format);
		_vsnwprintf(u16_Buf, 5000, u16_Format, args);
		OutputDebugStringW(u16_Buf);
	}
#endif

} // Namespace Cabinet

