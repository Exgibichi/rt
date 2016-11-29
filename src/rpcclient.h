// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#ifndef BITCOIN_RPCCLIENT_H
#define BITCOIN_RPCCLIENT_H

#include "univalue.h"

UniValue RPCConvertValues(const std::string& strMethod, const std::vector<std::string>& strParams);

#endif // BITCOIN_RPCCLIENT_H
