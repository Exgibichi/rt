// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#ifndef BITCOIN_ECCRYPTOVERIFY_H
#define BITCOIN_ECCRYPTOVERIFY_H

#include <vector>
#include <cstdlib>

class uint256;

namespace eccrypto {

bool Check(const unsigned char *vch);
bool CheckSignatureElement(const unsigned char *vch, int len, bool half);

} // eccrypto namespace

#endif // BITCOIN_ECCRYPTOVERIFY_H
