#include <exch.h>
//#include <rpcprotocol.h>

//-----------------------------------------------------
ExchBox::ExchBox(const string &retAddr) {
  m_v_exch.push_back(new ExchCoinReform(retAddr));
}

//-----------------------------------------------------
ExchBox::~ExchBox() {
  delete m_v_exch[0];
}

//-----------------------------------------------------
//-----------------------------------------------------

//-----------------------------------------------------
Exch::Exch(const string &retAddr)
  : m_retAddr(retAddr) {
  m_rate = m_limit = m_min = m_minerFee = 0.0;
}

//-----------------------------------------------------
// Get input path within server, like: /api/marketinfo/emc_btc.json
// Called from exchange-specific MarketInfo()
// Fill MarketInfo from exchange.
// Returns empty string if OK, or error message, if error
string Exch::RawMarketInfo(const string &path) {
  m_rate = m_limit = m_min = m_minerFee = 0.0;
  return "Not Implemented";
}










//-----------------------------------------------------
//-----------------------------------------------------


//-----------------------------------------------------
ExchCoinReform::ExchCoinReform(const string &retAddr) 
  : Exch(retAddr) {
}

//-----------------------------------------------------

const string& ExchCoinReform::Name() const { 
  static const string rc("CoinReform");
  return rc;
}

//-----------------------------------------------------
const string& ExchCoinReform::Host() const {
  static const string rc("www.coinreform.com");
  return rc;
}

//-----------------------------------------------------
// Get currency for exchnagge to, like btc, ltc, etc
// Fill MarketInfo from exchange.
// Returns empty string if OK, or error message, if error
string ExchCoinReform::MarketInfo(const string &currency) {
  return RawMarketInfo("/api/marketinfo/emc_" + currency + ".json");
}


//-----------------------------------------------------
