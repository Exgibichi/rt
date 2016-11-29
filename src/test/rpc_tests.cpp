// Copyright (c) 2012-2013 The Bitcoin Core developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.
#if 0

#include "rpcserver.h"
#include "rpcclient.h"

#include "base58.h"
#include "netbase.h"

#include <boost/algorithm/string.hpp>
#include <boost/test/unit_test.hpp>

using namespace std;

Array
createArgs(int nRequired, const char* address1=NULL, const char* address2=NULL)
{
    Array result;
    result.push_back(nRequired);
    Array addresses;
    if (address1) addresses.push_back(address1);
    if (address2) addresses.push_back(address2);
    result.push_back(addresses);
    return result;
}

Value CallRPC(string args)
{
    vector<string> vArgs;
    boost::split(vArgs, args, boost::is_any_of(" \t"));
    string strMethod = vArgs[0];
    vArgs.erase(vArgs.begin());
    Array params = RPCConvertValues(strMethod, vArgs);

    rpcfn_type method = tableRPC[strMethod]->actor;
    try {
        Value result = (*method)(params, false);
        return result;
    }
    catch (Object& objError)
    {
        throw runtime_error(find_value(objError, "message").get_str());
    }
}


BOOST_AUTO_TEST_SUITE(rpc_tests)

BOOST_AUTO_TEST_CASE(rpc_rawparams)
{
    // Test raw transaction API argument handling
    Value r;

    BOOST_CHECK_THROW(CallRPC("getrawtransaction"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("getrawtransaction not_hex"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("getrawtransaction a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff150ed not_int"), runtime_error);

    BOOST_CHECK_THROW(CallRPC("createrawtransaction"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("createrawtransaction null null"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("createrawtransaction not_array"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("createrawtransaction [] []"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("createrawtransaction {} {}"), runtime_error);
    BOOST_CHECK_NO_THROW(CallRPC("createrawtransaction [] {}"));
    BOOST_CHECK_THROW(CallRPC("createrawtransaction [] {} extra"), runtime_error);

    BOOST_CHECK_THROW(CallRPC("decoderawtransaction"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("decoderawtransaction null"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("decoderawtransaction DEADBEEF"), runtime_error);
    string rawtx = "01000000fdbbd05501ca38f3453fc002bec2621b4a12c8ebd966410af11d5d37f76c7b920c57f05f5e010000004a4930460221009d2002e6991bd30e1aec818b468ddf72eb4963e4b508cc625e596dbe0371b159022100d6ceebe4419de90ae7d79aa27cb068712f4073966d23c1e579679d07aa78da6c01ffffffff020000000000000000006585cd06000000002321025a5288bb579d0bd332269fd7239cdbc19a731cc94afa143b057f071b0e8d9b26ac00000000";
    BOOST_CHECK_NO_THROW(r = CallRPC(string("decoderawtransaction ")+rawtx));
    BOOST_CHECK_EQUAL(find_value(r.get_obj(), "version").get_int(), 1);
    BOOST_CHECK_EQUAL(find_value(r.get_obj(), "locktime").get_int(), 0);
    BOOST_CHECK_THROW(r = CallRPC(string("decoderawtransaction ")+rawtx+" extra"), runtime_error);

    BOOST_CHECK_THROW(CallRPC("signrawtransaction"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("signrawtransaction null"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("signrawtransaction ff00"), runtime_error);
    BOOST_CHECK_NO_THROW(CallRPC(string("signrawtransaction ")+rawtx));
    BOOST_CHECK_NO_THROW(CallRPC(string("signrawtransaction ")+rawtx+" null null NONE|ANYONECANPAY"));
    BOOST_CHECK_NO_THROW(CallRPC(string("signrawtransaction ")+rawtx+" [] [] NONE|ANYONECANPAY"));
    BOOST_CHECK_THROW(CallRPC(string("signrawtransaction ")+rawtx+" null null badenum"), runtime_error);

    // Only check failure cases for sendrawtransaction, there's no network to send to...
    BOOST_CHECK_THROW(CallRPC("sendrawtransaction"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("sendrawtransaction null"), runtime_error);
    BOOST_CHECK_THROW(CallRPC("sendrawtransaction DEADBEEF"), runtime_error);
    BOOST_CHECK_THROW(CallRPC(string("sendrawtransaction ")+rawtx+" false extra"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_rawsign)
{
    Value r;
    // input is a 1-of-2 multisig (so is output):
    string prevout =
      "[{\"txid\":\"b4cc287e58f87cdae59417329f710f3ecd75a4ee1d2872b7248f50977c8493f3\","
      "\"vout\":1,\"scriptPubKey\":\"a914b10c9df5f7edf436c697f02f1efdba4cf399615187\","
      "\"redeemScript\":\"512103debedc17b3df2badbcdd86d5feb4562b86fe182e5998abd8bcd4f122c6155b1b21027e940bb73ab8732bfdf7f9216ecefca5b94d6df834e77e108f68e66f126044c052ae\"}]";
    r = CallRPC(string("createrawtransaction ")+prevout+" "+
      "{\"ELPLvENw9BCCeNCvbYiBctq4TfjaQyuu1k\":11}");
    string notsigned = r.get_str();
    string privkey1 = "\"KzsXybp9jX64P5ekX1KUxRQ79Jht9uzW7LorgwE65i5rWACL6LQe\"";
    string privkey2 = "\"Kyhdf5LuKTRx4ge69ybABsiUAWjVRK4XGxAKk2FQLp2HjGMy87Z4\"";
    r = CallRPC(string("signrawtransaction ")+notsigned+" "+prevout+" "+"[]");
    BOOST_CHECK(find_value(r.get_obj(), "complete").get_bool() == false);
    r = CallRPC(string("signrawtransaction ")+notsigned+" "+prevout+" "+"["+privkey1+","+privkey2+"]");
    BOOST_CHECK(find_value(r.get_obj(), "complete").get_bool() == true);
}

// emercoin: currently we have different COIN value. maybe match it with bitcoin COIN value?
BOOST_AUTO_TEST_CASE(rpc_format_monetary_values)
{
//    BOOST_CHECK_EQUAL(write_string(ValueFromAmount(0LL), false), "0.00000000");
//    BOOST_CHECK_EQUAL(write_string(ValueFromAmount(1LL), false), "0.00000001");
//    BOOST_CHECK_EQUAL(write_string(ValueFromAmount(17622195LL), false), "0.17622195");
//    BOOST_CHECK_EQUAL(write_string(ValueFromAmount(50000000LL), false), "0.50000000");
//    BOOST_CHECK_EQUAL(write_string(ValueFromAmount(89898989LL), false), "0.89898989");
//    BOOST_CHECK_EQUAL(write_string(ValueFromAmount(100000000LL), false), "1.00000000");
//    BOOST_CHECK_EQUAL(write_string(ValueFromAmount(2099999999999990LL), false), "20999999.99999990");
//    BOOST_CHECK_EQUAL(write_string(ValueFromAmount(2099999999999999LL), false), "20999999.99999999");
}

static Value ValueFromString(const std::string &str)
{
    Value value;
    BOOST_CHECK(read_string(str, value));
    return value;
}

// emercoin: currently we have different COIN value. maybe match it with bitcoin COIN value?
BOOST_AUTO_TEST_CASE(rpc_parse_monetary_values)
{
//    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.00000001")), 1LL);
//    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.17622195")), 17622195LL);
//    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.5")), 50000000LL);
//    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.50000000")), 50000000LL);
//    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("0.89898989")), 89898989LL);
//    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("1.00000000")), 100000000LL);
//    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("20999999.9999999")), 2099999999999990LL);
//    BOOST_CHECK_EQUAL(AmountFromValue(ValueFromString("20999999.99999999")), 2099999999999999LL);
}

BOOST_AUTO_TEST_CASE(json_parse_errors)
{
    Value value;
    // Valid
    BOOST_CHECK_EQUAL(read_string(std::string("1.0"), value), true);
    // Valid, with trailing whitespace
    BOOST_CHECK_EQUAL(read_string(std::string("1.0 "), value), true);
    // Invalid, initial garbage
    BOOST_CHECK_EQUAL(read_string(std::string("[1.0"), value), false);
    BOOST_CHECK_EQUAL(read_string(std::string("a1.0"), value), false);
    // Invalid, trailing garbage
    BOOST_CHECK_EQUAL(read_string(std::string("1.0sds"), value), false);
    BOOST_CHECK_EQUAL(read_string(std::string("1.0]"), value), false);
    // BTC addresses should fail parsing
    BOOST_CHECK_EQUAL(read_string(std::string("175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"), value), false);
    BOOST_CHECK_EQUAL(read_string(std::string("3J98t1WpEZ73CNmQviecrnyiWrnqRhWNL"), value), false);
}

BOOST_AUTO_TEST_CASE(rpc_boostasiotocnetaddr)
{
    // Check IPv4 addresses
    BOOST_CHECK_EQUAL(BoostAsioToCNetAddr(boost::asio::ip::address::from_string("1.2.3.4")).ToString(), "1.2.3.4");
    BOOST_CHECK_EQUAL(BoostAsioToCNetAddr(boost::asio::ip::address::from_string("127.0.0.1")).ToString(), "127.0.0.1");
    // Check IPv6 addresses
    BOOST_CHECK_EQUAL(BoostAsioToCNetAddr(boost::asio::ip::address::from_string("::1")).ToString(), "::1");
    BOOST_CHECK_EQUAL(BoostAsioToCNetAddr(boost::asio::ip::address::from_string("123:4567:89ab:cdef:123:4567:89ab:cdef")).ToString(),
                                         "123:4567:89ab:cdef:123:4567:89ab:cdef");
    // v4 compatible must be interpreted as IPv4
    BOOST_CHECK_EQUAL(BoostAsioToCNetAddr(boost::asio::ip::address::from_string("::0:127.0.0.1")).ToString(), "127.0.0.1");
    // v4 mapped must be interpreted as IPv4
    BOOST_CHECK_EQUAL(BoostAsioToCNetAddr(boost::asio::ip::address::from_string("::ffff:127.0.0.1")).ToString(), "127.0.0.1");
}

BOOST_AUTO_TEST_SUITE_END()
#endif
