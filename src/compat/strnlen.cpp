// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif

#include <cstring>
// OSX 10.6 is missing strnlen at runtime, but builds targetting it will still
// succeed. Define our own version here to avoid a crash.
size_t strnlen_int( const char *start, size_t max_len)
{
    const char *end = (const char *)memchr(start, '\0', max_len);

    return end ? (size_t)(end - start) : max_len;
}
