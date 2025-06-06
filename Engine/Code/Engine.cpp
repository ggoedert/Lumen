//==============================================================================================================================================================================
/// \file
/// \brief     engine extra code
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#ifdef _DEBUG
#include "lDefs.h"

using namespace Lumen;

/// hash (FNV-1a) class name from current function name, debug version
Type Lumen::ClassNameType(const char *currentFunction)
{
    size_t end = std::string_view(currentFunction).find_last_of('(');
    end = std::string_view(currentFunction, end).find_last_of(':') - 1;
    size_t begin = std::string_view(currentFunction, end).find_last_of(' ') + 1;
    Hash hash = HASH_OFFSET;
    size_t hashPos = begin;
    while (hashPos < end)
    {
        hash ^= *(currentFunction + hashPos);
        hash *= HASH_PRIME;
        hashPos++;
    }
    return Type(hash, std::string_view(currentFunction + begin, end - begin));
}
#endif
