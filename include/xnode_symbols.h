#pragma once

#include "xbase/symbols.h"

#if defined(XNODE_STATIC_DEFINE)
    #define XNODE_API
#elif defined(XNODE_BUILDING_DLL)
    #define XNODE_API XSDK_SYMBOL_EXPORT
#else
    #define XNODE_API XSDK_SYMBOL_IMPORT
#endif

#define XNODE_PRIVATE XSDK_SYMBOL_LOCAL
