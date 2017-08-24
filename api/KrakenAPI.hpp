#ifndef KRAKEN_API
#define KRAKEN_API
#include "../settings.hpp"
#include "../errors.hpp"
#include "ExchangeAPI.hpp"
using error::kraken;

class KrakenAPI : public ExchangeAPI {
public:
	KrakenAPI(string apikey, string secret);
	~KrakenAPI();
	json getCurrencPairs();
	json getCurrencyInfo(vector<string>&);
	float getCurrentUSDPrice(string&);
	json getTickerInfo(vector<string>&);

	json getAccountBalance();
	json getTradeHistory();
	json openOrders();
	json closedOrders();
	json placeOrder(Order& order);
	json cancelOrder(string&);
	json withdraw(string&, string&, float);
	json getDepositAddresses(string&);
	json newAddress(string&);

	//non exchangeAPI methods
	json openPositions();

	string testy();
private:
	string getWithdrawalMethod(string& currency, string& key, float amount);
	string getDepositMethod(string& currency);
	string getNonce();
	void resetHeaders();
	void setHeaders(string, paramVect*);
	string getSignature(string& path, string& nonce, string& postdata);

	kraken sendRequest(utilities::RequestMethod method, string loc, json& result, paramVect* queryVars = 0);
	kraken handleError(string& e);
	
	string host;
	string apiKey;
	string apiSecret;
	string privUrlBase;
	string pubUrlBase;

	string query;
	paramVect httpHeaders;
	paramVect requestParams;
};

#endif