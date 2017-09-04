// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "auxpow.h"
#include "bignum.h"
#include "chain.h"
#include "primitives/block.h"
#include "util.h"

#include <algorithm>

// ppcoin: find last block index up to pindex
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake)
{
    while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
        pindex = pindex->pprev;
    return pindex;
}

unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake, const Consensus::Params& params)
{
    using namespace std;

    if (pindexLast == NULL)
        return UintToArith256(params.powLimit).GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return UintToArith256(params.bnInitialHashTarget).GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return UintToArith256(params.bnInitialHashTarget).GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    // ppcoin: target change every block
    // ppcoin: retarget with exponential moving toward target spacing
    CBigNum bnNew;
    bnNew.SetCompact(pindexPrev->nBits);

    // emercoin: first 10 000 blocks are faster to mine.
    int64_t nSpacingRatio = (pindexLast->nHeight <= 10000) ? max((int64_t)10, params.nStakeTargetSpacing * pindexLast->nHeight / 10000) :
                                                             max((int64_t)10, params.nStakeTargetSpacing);

    int64_t nTargetSpacing = fProofOfStake? params.nStakeTargetSpacing : min(params.nTargetSpacingMax, nSpacingRatio * (1 + pindexLast->nHeight - pindexPrev->nHeight));
    int64_t nInterval = params.nTargetTimespan / nTargetSpacing;

    int n = fProofOfStake ? 1 : ((pindexLast->nHeight < 6666) ? 1 : 3);
    bnNew *= ((nInterval - n) * nTargetSpacing + (n + 1) * nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

    if (bnNew > CBigNum(params.powLimit))
        bnNew = CBigNum(params.powLimit);

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}

bool CheckBlockProofOfWork(const CBlockHeader *pblock, const Consensus::Params& params)
{
    // There's an issue with blocks prior to the auxpow fork reporting an invalid chain ID.
    // As no version earlier than the 0.10 client a) has version 5+ blocks and b)
    //	has auxpow, anything that isn't a version 5+ block can be checked normally.

    if (pblock->GetBlockVersion() > 4)
    {
        if (!params.fPowAllowMinDifficultyBlocks && (pblock->nVersion & BLOCK_VERSION_AUXPOW && pblock->GetChainID() != AUXPOW_CHAIN_ID))
            return error("CheckBlockProofOfWork() : block does not have our chain ID");

        if (pblock->auxpow.get() != NULL)
        {
            if (!pblock->auxpow->Check(pblock->GetHash(), pblock->GetChainID(), params))
                return error("CheckBlockProofOfWork() : AUX POW is not valid");
            // Check proof of work matches claimed amount
            if (!CheckProofOfWork(pblock->auxpow->GetParentBlockHash(), pblock->nBits, params))
                return error("CheckBlockProofOfWork() : AUX proof of work failed");
        }
        else
        {
            // Check proof of work matches claimed amount
            if (!CheckProofOfWork(pblock->GetHash(), pblock->nBits, params))
                return error("CheckBlockProofOfWork() : proof of work failed");
        }
    }
    else
    {
        // Check proof of work matches claimed amount
        if (!CheckProofOfWork(pblock->GetHash(), pblock->nBits, params))
            return error("CheckBlockProofOfWork() : proof of work failed");
    }
    return true;
}
