#include <string>
#include <vector>

using namespace std;

//-----------------------------------------------------
class Exch {
  public:
  Exch(const string &retAddr); 
  virtual ~Exch() {};

  virtual const string& Name() const = 0;
  virtual const string& Host() const = 0;
  
  protected:
  // Get currency for exchnagge to, like btc, ltc, etc
  // Fill MarketInfo from exchange.
  // Returns empty string if OK, or error message, if error
  virtual string MarketInfo(const string &currency) = 0;

  // Get input path within server, like: /api/marketinfo/emc_btc.json
  // Called from exchange-specific MarketInfo()
  // Fill MarketInfo from exchange.
  // Returns empty string if OK, or error message, if error
  string RawMarketInfo(const string &path);

  string m_retAddr; // Return EMC Addr

  // MarketInfo fills these params
  string m_pair;
  double m_rate;
  double m_limit;
  double m_min;
  double m_minerFee;

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

}; // class ExchCoinReform

//-----------------------------------------------------
class ExchBox {
  public:
  ExchBox(const string &retAddr);
  ~ExchBox();

  vector<Exch*> m_v_exch;
}; // class exchbox

//-----------------------------------------------------
