// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include "uint256.h"
#include <map>
#include <string>

namespace Consensus {

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;
    /** Block height and hash at which BIP34 becomes active */
    int BIP34Height;
    uint256 BIP34Hash;
    /** Block height at which BIP65 becomes active */
    int BIP65Height;
    /** Block height at which BIP66 becomes active */
    int BIP66Height;
    /** Block height at which merged mining becomes active */
    int MMHeight;
    /** Block height at which min tx fee was changed */
    int MinFeeHeight;
    /** Proof of work parameters */
    uint256 powLimit;
    bool fPowAllowMinDifficultyBlocks;
    int64_t nTargetSpacing;
    int64_t nTargetTimespan;
    uint256 nMinimumChainWork;
    uint256 defaultAssumeValid;

    // emercoin stuff:
    uint256 bnInitialHashTarget;
    int64_t nStakeTargetSpacing;
    int64_t nTargetSpacingMax;
    int64_t nStakeMinAge;
    int64_t nStakeMaxAge;
    int64_t nStakeModifierInterval;
};
} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
