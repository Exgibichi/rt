// Copyright (c) 2012-2013 The Bitcoin Core developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#include "compat/sanity.h"
#include "key.h"

#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_SUITE(sanity_tests)

BOOST_AUTO_TEST_CASE(basic_sanity)
{
  BOOST_CHECK_MESSAGE(glibc_sanity_test() == true, "libc sanity test");
  BOOST_CHECK_MESSAGE(glibcxx_sanity_test() == true, "stdlib sanity test");
  BOOST_CHECK_MESSAGE(ECC_InitSanityCheck() == true, "openssl ECC test");
}

BOOST_AUTO_TEST_SUITE_END()
