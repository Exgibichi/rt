#include <exch.h>
#include "clientversion.h"
#include "util.h"

using namespace std;
using namespace boost;
using namespace boost::asio;

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
const string& Exch::Name() const { 
  static const string rc("ERROR: Used base class Exch");
  return rc;
}

//-----------------------------------------------------
const string& Exch::Host() const {
  return Name();
}

//-----------------------------------------------------
// Get currency for exchnagge to, like btc, ltc, etc
// Fill MarketInfo from exchange.
// Returns empty string if OK, or error message, if error
string  Exch::MarketInfo(const string &currency) {
  return Name();
}

//-----------------------------------------------------
// Get input path within server, like: /api/marketinfo/emc_btc.json
// Called from exchange-specific MarketInfo()
// Fill MarketInfo from exchange.
// Throws exception if error
const UniValue Exch::RawMarketInfo(const string &path) {
  m_rate = m_limit = m_min = m_minerFee = 0.0;
  m_pair.erase();
  return httpsFetch(path.c_str(), NULL);
} //  Exch::RawMarketInfo

//-----------------------------------------------------
string Exch::Send(const string &to, double amount) {
  return "ERR";
}

//-----------------------------------------------------
// Connect to the server by https, fetch JSON and parse to UniValue
UniValue Exch::httpsFetch(const char *get, const UniValue *post) {
  // Connect to exchange
  asio::io_service io_service;
  ssl::context context(io_service, ssl::context::sslv23);
  context.set_options(ssl::context::no_sslv2 | ssl::context::no_sslv3);
  asio::ssl::stream<asio::ip::tcp::socket> sslStream(io_service, context);
  SSL_set_tlsext_host_name(sslStream.native_handle(), Host().c_str());
  SSLIOStreamDevice<asio::ip::tcp> d(sslStream, true); // use SSL
  iostreams::stream< SSLIOStreamDevice<asio::ip::tcp> > stream(d);


  if(!d.connect(Host(), "443"))
    throw runtime_error("Couldn't connect to server");

  string postBody;
  const char *reqType = "GET ";

  if(post != NULL) {
    // This is POST request - prepare postBody
    reqType = "POST ";
    postBody = post->write(0, 0, 0) + '\n';
  } 

printf("DBG: Exch::httpsFetch: Req: method=[%s] path=[%s] H=[%s]\n", reqType, get, Host().c_str());

  // Send request
  stream << reqType << (get? get : "/") << " HTTP/1.1\r\n"
         << "Host: " << Host() << "\r\n"
         << "User-Agent: emercoin-json-rpc/" << FormatFullVersion() << "\r\n";

  if(postBody.size()) {
    stream << "Content-Type: application/json\r\n"
           << "Content-Length: " << postBody.size() << "\r\n";
  }

  stream << "Connection: close\r\n"
         << "Accept: application/json\r\n\r\n" 
	 << postBody << std::flush;

  // Receive HTTP reply status
  int nProto = 0;
  int nStatus = ReadHTTPStatus(stream, nProto);

  if(nStatus >= 400)
    throw runtime_error(strprintf("Server returned HTTP error %d", nStatus));

  // Receive HTTP reply message headers and body
  map<string, string> mapHeaders;
  string strReply;
  ReadHTTPMessage(stream, mapHeaders, strReply, nProto, 4 * 1024);

printf("DBG: Exch::httpsFetch: Server returned HTTP: %d\n", nStatus);

  if(strReply.empty())
    throw runtime_error("No response from server");

printf("DBG: Exch::httpsFetch: Reply from server: [%s]\n", strReply.c_str());

  size_t json_beg = strReply.find('{');
  size_t json_end = strReply.rfind('}');
  if(json_beg == string::npos || json_end == string::npos)
    throw runtime_error("Reply is not JSON");

   // Parse reply
  UniValue valReply(UniValue::VSTR);

  if(!valReply.read(strReply.substr(json_beg, json_end - json_beg + 1), 0))
    throw runtime_error("Couldn't parse reply from server");

  const UniValue& reply = valReply.get_obj();

  if(reply.empty())
    throw runtime_error("Empty JSON reply");

  return reply;

} // UniValue Exch::httpsFetch







//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

//=====================================================

//-----------------------------------------------------
ExchCoinReform::ExchCoinReform(const string &retAddr)
: Exch::Exch(retAddr) {
}

//-----------------------------------------------------
ExchCoinReform::~ExchCoinReform() {}

//-----------------------------------------------------
const string& ExchCoinReform::Name() const { 
  static const string rc("CoinReform");
  return rc;
}

//-----------------------------------------------------
const string& ExchCoinReform::Host() const {
  //static const string rc("shapeshift.io");
  static const string rc("www.coinreform.com");
  return rc;
}

//-----------------------------------------------------
// Get currency for exchnagge to, like btc, ltc, etc
// Fill MarketInfo from exchange.
// Returns empty string if OK, or error message, if error
string ExchCoinReform::MarketInfo(const string &currency) {
  try {
    //const UniValue mi(RawMarketInfo("/marketinfo/ltc_" + currency));
    const UniValue mi(RawMarketInfo("/api/marketinfo/emc_" + currency + ".json"));
    printf("DBG: ExchCoinReform::MarketInfo(%s|%s) returns <%s>\n\n", Host().c_str(), currency.c_str(), mi.write(0, 0, 0).c_str());
    m_pair     = mi["pair"].get_str();
    m_rate     = atof(mi["rate"].get_str().c_str());
    m_limit    = atof(mi["limit"].get_str().c_str());
    m_min      = atof(mi["min"].get_str().c_str());
    m_minerFee = atof(mi["minerFee"].get_str().c_str());
    return "";
  } catch(std::exception &e) {
    return e.what();
  }
} // ExchCoinReform::MarketInfo
//coinReform
//{"pair":"EMC_BTC","rate":"0.00016236","limit":"0.01623600","min":"0.00030000","minerFee":"0.00050000"}








// ShapeShift
// <{"pair":"ltc_btc","rate":0.00320953,"minerFee":0.0012,"limit":256.45898362,"minimum":0.74136083,"maxLimit":641.14745904}
/*
    m_rate     = mi["rate"].get_real();
    m_limit    = mi["limit"].get_real();
    m_min      = mi["min"].get_real();
    m_minerFee = mi["minerFee"].get_real();
*/

//-----------------------------------------------------
//=====================================================
void exch_test() {
    ExchBox  eBox("ESgQZ4oU5TN6BRK3DqZX3qDSrQjPwWHP7t");
      Exch    *exch = eBox.m_v_exch[0];
      printf("exch_test()\nwork with exch=0, Name=%s URL=%s\n", exch->Name().c_str(), exch->Host().c_str());
      string err(exch->MarketInfo("btc"));
      printf("MarketInfo returned: [%s]\n", err.c_str());
      printf("Values from exch: m_rate=%lf; m_limit=%lf; m_min=%lf; m_minerFee=%lf\n", exch->m_rate, exch->m_limit, exch->m_min, exch->m_minerFee);
      printf("Quit from test\n");
      exit(0);
}

//-----------------------------------------------------
