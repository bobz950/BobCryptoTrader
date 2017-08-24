#include "KrakenAPI.hpp"
#include <thread>
#include <chrono>

using utilities::RequestMethod;

KrakenAPI::KrakenAPI(string apikey, string secret) : apiKey(apikey), host("api.kraken.com"), apiSecret(secret), pubUrlBase("/0/public/"), privUrlBase("/0/private/") {}

KrakenAPI::~KrakenAPI() {}

json KrakenAPI::getCurrencPairs() {
	json j;
	if (this->sendRequest(RequestMethod::PUBLIC, "AssetPairs", j) == kraken::OK)
		return j;
	return json();
}
json KrakenAPI::getCurrencyInfo(vector<string>& currencies) { 
	if (currencies.size() == 0)
		return json();
	string currencyList = currencies.front();

	for (vector<string>::iterator it = currencies.begin() + 1; it != currencies.end(); it++)
		currencyList += "," + *it;

	paramVect params{ pair<string, string>("asset", currencyList) };
	json j;
	if (this->sendRequest(RequestMethod::PUBLIC, "Assets", j, &params) == kraken::OK)
		return j;
	
	return json();
}

float KrakenAPI::getCurrentUSDPrice(string& currency) { 
	paramVect p = { { "pair", currency + "USD" } };
	json j;
	if (this->sendRequest(RequestMethod::PUBLIC, "Ticker", j, &p) != kraken::OK)
		return json();
	string s;
	try {
		s = j.at("result").front().at("c").at(0).get<string>();
	}
	catch (out_of_range e) {
		printf("%s \n", e.what());
		return 0.0f;
	}
	return stof(s);
}

json KrakenAPI::getTickerInfo(vector<string>& pairs) { 
	if (pairs.size() == 0)
		return json();

	string pairList = pairs.front();
	for (vector<string>::iterator it = pairs.begin() + 1; it != pairs.end(); it++)
		pairList += "," + *it;
	paramVect params{ pair<string, string>("pair", pairList) };
	json j;
	if (this->sendRequest(RequestMethod::PUBLIC, "Ticker", j, &params) != kraken::OK)
		return json();
	return j;
}

json KrakenAPI::getAccountBalance() { 
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "Balance", j) == kraken::OK)
		return j;
	return json();
}
json KrakenAPI::getTradeHistory() { 
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "TradesHistory", j) == kraken::OK)
		return j;
	return json();
}

json KrakenAPI::openOrders() {
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "OpenOrders", j) == kraken::OK)
		return j;
	return json();
}
json KrakenAPI::closedOrders() {
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "ClosedOrders", j) == kraken::OK)
		return j;
	return json();
}

json KrakenAPI::placeOrder(Order& order) {
	paramVect params;
	params.push_back(pair<string, string>("pair", order.getPair()));
	params.push_back(pair<string, string>("type", order.getType()));
	params.push_back(pair<string, string>("ordertype", order.getOrderType()));

	params.push_back(pair<string, string>("price", to_string(order.getPrice())));
	params.push_back(pair<string, string>("volume", to_string(order.getQuantity())));
	if (order.getLeverage() != 1.0f)
		params.push_back(pair<string, string>("leverage", to_string(order.getLeverage())));

	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "AddOrder", j, &params) == kraken::OK)
		return j;
	return json();
}

json KrakenAPI::cancelOrder(string& id) {
	paramVect params{ {"txid", id} };
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "CancelOrder", j, &params) == kraken::OK)
		return j;
	return json();
}


//note: open positions sometimes have more than 1 trade made to open at full amount. In this case, each trade will show up as its own position. Match the ordertxid of each position
json KrakenAPI::openPositions() {
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "OpenPositions", j) == kraken::OK)
		return j;
	return json();
}

json KrakenAPI::withdraw(string& currency, string& address, float amount) {
	json j;
	string method = this->getWithdrawalMethod(currency, address, amount);
	if (method.size() == 0)
		return j;
	this_thread::sleep_for(chrono::seconds(1));

	paramVect params{ {"asset", currency}, {"key", address}, {"amount", to_string(amount)} };
	if (this->sendRequest(RequestMethod::PRIVATE, "Withdraw", j, &params) == kraken::OK)
		return j;
	return json();
}

string KrakenAPI::getWithdrawalMethod(string& currency, string& key, float amount) {
	paramVect v{ { "asset", currency },{ "key", key },{ "amount", to_string(amount) } };
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "WithdrawInfo", j, &v) != kraken::OK)
		return "";
	try {
		return j["result"]["method"].get<string>();
	}
	catch (out_of_range) {
		return "";
	}
}

json KrakenAPI::getDepositAddresses(string& currency) {
	json j;
	string method = this->getDepositMethod(currency);
	if (method.size() == 0)
		return j;
	this_thread::sleep_for(chrono::seconds(1));
	paramVect params{ {"asset", currency}, {"method", method} };
	if (this->sendRequest(RequestMethod::PRIVATE, "DepositAddresses", j, &params) == kraken::OK)
		return j;
	return json();
}

json KrakenAPI::newAddress(string& currency) {
	json j;
	string method = this->getDepositMethod(currency);
	if (method.size() == 0)
		return j;
	this_thread::sleep_for(chrono::seconds(1));
	paramVect params{ { "asset", currency },{ "method", method }, {"new", "true"} };
	if(this->sendRequest(RequestMethod::PRIVATE, "DepositAddresses", j, &params) == kraken::OK)
		return j;
	return json();
}


string KrakenAPI::getDepositMethod(string& currency) {
	paramVect v{ { "asset", currency } };
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "DepositMethods", j, &v) != kraken::OK)
		return "";
	try {
		return j["result"].at(0)["method"].get<string>();
	}
	catch (exception e) {
		printf(j.dump().c_str());
		return "";
	}
}

kraken KrakenAPI::sendRequest(RequestMethod method, string loc, json& result, paramVect* queryVars) {
	this->resetHeaders(); //clear the object data for headers so api sign can be changed
	string uri = this->host + (method == RequestMethod::PUBLIC ? this->pubUrlBase : this->privUrlBase) + loc;
	string url = "https://" + uri;

	struct curl_slist* head = 0;
	if (method == RequestMethod::PRIVATE) { //if private request, call setHeaders() to set headers and query string
		this->setHeaders(loc, queryVars);
		curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, this->query.c_str());
		curl_easy_setopt(this->curl, CURLOPT_POSTFIELDSIZE, this->query.size());
		//add http header fields (for api key and api sign)
		for (int i = 0; i < this->httpHeaders.size(); i++) 
			head = curl_slist_append(head, (this->httpHeaders.at(i).first + ": " + this->httpHeaders.at(i).second).c_str());
		curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, head);
	}
	else if (method == RequestMethod::PUBLIC && queryVars) { //if public request, add query string to end of url
		string qry = utilities::getQueryString(*queryVars);
		url.append(qry);
	}
	curl_easy_setopt(this->curl, CURLOPT_URL, url.c_str()); //set the url
	curl_easy_perform(this->curl); //perform the request
	//get the response and clear the response buffer
	istringstream is(utilities::res);
	is >> result;
	utilities::res.clear(); //clear response string for next request
	//free added header list
	if (head)
		curl_slist_free_all(head);
	//if api returned an error, return false
	if (result["error"].size() > 0) 
		return this->handleError(result["error"].at(0).get<string>());
	return kraken::OK;
}

kraken KrakenAPI::handleError(string& e) {
	if (!e.compare("EAPI:Invalid signature"))
		return kraken::INVALID_SIGNATURE;
	else if (!e.compare("EGeneral:Invalid arguments"))
		return kraken::INVALID_ARGUMENTS;
	else return kraken::OTHER;
}

string KrakenAPI::testy() {
	//paramVect p = { { "pair", "ETHUSD" } };
	//return this->sendRequest(utilities::RequestMethod::GET, "Ticker", &p);
	paramVect v{ {"asset", "XBT"}, {"key", "bittrex-btc"}, {"amount", "1"} };
	//return this->sendRequest(RequestMethod::PRIVATE, "WithdrawInfo", &v);
}

string KrakenAPI::getNonce() {
	time_t n = time(0);
	return to_string(n * 1000);
}

void KrakenAPI::resetHeaders() {
	curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, 0);
	curl_easy_setopt(this->curl, CURLOPT_POSTFIELDSIZE, 0);
	curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, 0);
	this->httpHeaders.clear();
	this->requestParams.clear();
	this->query.clear();
}

void KrakenAPI::setHeaders(string loc, paramVect* queryVars) {
	string nonce = this->getNonce();
	this->requestParams.push_back(pair<string, string>("nonce", nonce));
	//add additional params to post data
	if (queryVars)
		this->requestParams.insert(this->requestParams.end(), queryVars->begin(), queryVars->end());
	this->query = utilities::getQueryString(this->requestParams, false); //create the query string for post data from vector
	string signature = this->getSignature(this->privUrlBase + loc, nonce, this->query);
	this->httpHeaders.push_back(pair<string, string>("API-Key", this->apiKey));
	this->httpHeaders.push_back(pair<string, string>("API-Sign", signature));
}

string KrakenAPI::getSignature(string& path, string& nonce, string& postdata) {
	string s256 = utilities::sha256(nonce + postdata);  //hash the nonce + post data query
	const char* secret = this->apiSecret.c_str();
	string pathSha = path + s256;
	string decodedSecret = utilities::Base64Decode(secret);
	unsigned int hmacSize;
	unsigned char* hmac = utilities::sha512hmac(decodedSecret, pathSha, hmacSize);
	return utilities::Base64Encode(hmac, hmacSize);
}
