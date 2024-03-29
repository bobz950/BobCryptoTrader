#ifndef EXCHANGE_API
#define EXCHANGE_API
#include "../settings.hpp"
#include "../currencies.hpp"
#include "../trade/Order.hpp"
#include "../trade/ProcessedOrder.hpp"
#include "../trade/Position.hpp"
using namespace currencies;

class ExchangeAPI {
public:
	virtual bool marginEnabled() = 0;
	virtual pairVect getCurrencPairs() = 0;
	virtual vector<currency> getCurrencies() = 0;
	virtual float getCurrentUSDPrice(string&) = 0;
	virtual json getTickerInfo(vector<string>&) = 0;

	virtual map<currency, float> getAccountBalance() = 0;
	virtual json getTradeHistory() = 0;
	virtual vector<ProcessedOrder> openOrders() = 0;
	virtual vector<ProcessedOrder> closedOrders() = 0;
	virtual json placeOrder(Order&) = 0;
	virtual json cancelOrder(string&) = 0;
	virtual json withdraw(string&, string&, float) = 0;
	virtual json getDepositAddresses(string&) = 0;
	virtual json newAddress(string&) = 0;
	virtual vector<Position> openPositions() = 0;

	ExchangeAPI() {
		curl_global_init(CURL_GLOBAL_ALL);
		this->curl = curl_easy_init();
		this->ready();
	}
	virtual ~ExchangeAPI() {
		curl_easy_cleanup(this->curl);
		curl_global_cleanup();
	}
	
protected:
	map<string, CurrencyPair> knownPairs; //keep known currencypairs in memory to prevent redudant api calls
	CURL* curl;
	void ready() {
		if (this->curl) {
#if PRINTCURLINFO
			curl_easy_setopt(this->curl, CURLOPT_VERBOSE, 1L);
#endif
			//set the ca cert
			curl_easy_setopt(this->curl, CURLOPT_CAINFO, "cacert.pem");
			curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, &utilities::writer);
			curl_easy_setopt(this->curl, CURLOPT_USERAGENT, "Bob Crypto Trader Client");
		}
	}

};

#endif
