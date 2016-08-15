// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#ifndef BITCOIN_ECWRAPPER_H
#define BITCOIN_ECWRAPPER_H

#include <cstddef>
#include <vector>

#include <openssl/ec.h>

class uint256;

/** RAII Wrapper around OpenSSL's EC_KEY */
class CECKey {
private:
    EC_KEY *pkey;

public:
    CECKey();
    ~CECKey();

    void GetPubKey(std::vector<unsigned char>& pubkey, bool fCompressed);
    bool SetPubKey(const unsigned char* pubkey, size_t size);
    bool Verify(const uint256 &hash, const std::vector<unsigned char>& vchSigParam);

    /**
     * reconstruct public key from a compact signature
     * This is only slightly more CPU intensive than just verifying it.
     * If this function succeeds, the recovered public key is guaranteed to be valid
     * (the signature is a valid signature of the given data for that key)
     */
    bool Recover(const uint256 &hash, const unsigned char *p64, int rec);

    bool TweakPublic(const unsigned char vchTweak[32]);
    static bool SanityCheck();
};

#endif // BITCOIN_ECWRAPPER_H
