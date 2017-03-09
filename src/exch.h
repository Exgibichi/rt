//#include <string>
//#include <vector>

#include "rpcprotocol.h"

using namespace std;

//-----------------------------------------------------
class Exch {
  public:
  Exch(const string &retAddr); 
  virtual ~Exch() {};

  virtual const string& Name() const;
  virtual const string& Host() const;
  
  // Connect to the server by https, fetch JSON and parse to UniValue
  UniValue httpsFetch(const char *get, const UniValue *post);

  // Get currency for exchnagge to, like btc, ltc, etc
  // Fill MarketInfo from exchange.
  // Returns empty string if OK, or error message, if error
  virtual string MarketInfo(const string &currency);

  // Get input path within server, like: /api/marketinfo/emc_btc.json
  // Called from exchange-specific MarketInfo()
  // Fill MarketInfo from exchange.
  // Throws exception if error
  const UniValue RawMarketInfo(const string &path);

  // Creatse SEND exchange channel for 
  // Send "amount" in external currecny "to" address
  // Fills m_depAddr..m_txKey, and updates m_rate
  // Returns error text, oe rmpty string, if OK
  virtual string Send(const string &to, double amount);

  // Check status of existing transaction.
  // If key is empty, used the last key
  // Returns status, or an empty string, if "not my" key
  virtual string TxStat(const string &txkey, UniValue &details);

  string m_retAddr; // Return EMC Addr

  // MarketInfo fills these params
  string m_pair;
  double m_rate;
  double m_limit;
  double m_min;
  double m_minerFee;

  // Send fills these params _ m_rate above
  string m_depAddr;	// Address to pay EMC
  string m_outAddr;	// Address to pay from exchange
  double m_depAmo;	// amount in EMC
  double m_outAmo;	// Amount transferred to BTC
  string m_txKey;	// TX reference key

}; // class Exch

//-----------------------------------------------------
class ExchCoinReform : public Exch {
  public:
  ExchCoinReform(const string &retAddr);

  virtual ~ExchCoinReform();

  virtual const string& Name() const;
  virtual const string& Host() const;

  // Fill MarketInfo from exchange.
  // Returns empty string if OK, or error message, if erroe
  virtual string MarketInfo(const string &currency);

  // Creatse SEND exchange channel for 
  // Send "amount" in external currecny "to" address
  // Fills m_depAddr..m_txKey, and updates m_rate
  virtual string Send(const string &to, double amount);

  // Check status of existing transaction.
  // If key is empty, used the last key
  // Returns status, or an empty string, if "not my" key
  virtual string TxStat(const string &txkey, UniValue &details);
 

}; // class ExchCoinReform

//-----------------------------------------------------
class ExchBox {
  public:
  ExchBox(const string &retAddr);
  ~ExchBox();

  vector<Exch*> m_v_exch;
}; // class exchbox

//-----------------------------------------------------
