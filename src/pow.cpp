// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#include "pow.h"

#include "bignum.h"
#include "chain.h"
#include "chainparams.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"

#include <algorithm>

// ppcoin: find last block index up to pindex
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake)
{
    while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
        pindex = pindex->pprev;
    return pindex;
}

unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    using namespace std;
    if (pindexLast == NULL)
        return Params().ProofOfWorkLimit().GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return Params().InitialHashTarget().GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return Params().InitialHashTarget().GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    // ppcoin: target change every block
    // ppcoin: retarget with exponential moving toward target spacing
    CBigNum bnNew;
    bnNew.SetCompact(pindexPrev->nBits);

    // emercoin: first 10 000 blocks are faster to mine.
    int64_t nSpacingRatio = (pindexLast->nHeight <= 10000) ? max((int64_t)10, Params().StakeTargetSpacing() * pindexLast->nHeight / 10000) :
                                                             max((int64_t)10, Params().StakeTargetSpacing());

    int64_t nTargetSpacing = fProofOfStake? Params().StakeTargetSpacing() : min(Params().TargetSpacingMax(), nSpacingRatio * (1 + pindexLast->nHeight - pindexPrev->nHeight));
    int64_t nInterval = Params().TargetTimespan() / nTargetSpacing;

    int n = fProofOfStake ? 1 : ((pindexLast->nHeight < 6666) ? 1 : 3);
    bnNew *= ((nInterval - n) * nTargetSpacing + (n + 1) * nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

    if (bnNew > CBigNum(Params().ProofOfWorkLimit()))
        bnNew = CBigNum(Params().ProofOfWorkLimit());

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits)
{
    bool fNegative;
    bool fOverflow;
    uint256 bnTarget;

    if (Params().SkipProofOfWorkCheck())
       return true;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > Params().ProofOfWorkLimit())
        return error("CheckProofOfWork() : nBits below minimum work");

    // Check proof of work matches claimed amount
    if (hash > bnTarget)
        return error("CheckProofOfWork() : hash doesn't match nBits\nhash = %s\nbnTa = %s", hash.GetHex(), bnTarget.GetHex());

    return true;
}

uint256 GetBlockTrust(const CBlockIndex& block)
{
    uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return block.IsProofOfStake() ? (~bnTarget / (bnTarget + 1)) + 1 : 1;
}
