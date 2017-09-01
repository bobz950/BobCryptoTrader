#include "KrakenAPI.hpp"
#include <thread>
#include <chrono>

using utilities::RequestMethod;

KrakenAPI::KrakenAPI(string apikey, string secret) : apiKey(apikey), host("api.kraken.com"), apiSecret(secret), pubUrlBase("/0/public/"), privUrlBase("/0/private/") {}

KrakenAPI::~KrakenAPI() {}

bool KrakenAPI::marginEnabled() { return true; }

//returns vector of all currency pairs
pairVect KrakenAPI::getCurrencPairs() {
	json j;
	if (this->sendRequest(RequestMethod::PUBLIC, "AssetPairs", j) != kraken::OK)
		return pairVect();
	
	pairVect pairs;
	try {
		map<string, currency>::iterator mit;
		json res = j["result"];
		for (json::iterator it = res.begin(); it != res.end(); it++) {
			string strPair = it.value()["altname"].get<string>();
			if (*(strPair.rbegin() + 1) == '.') continue; //kraken has duplicate entries ending in .d -- skip these

			mit = krakenCurrencies.find(it.value()["base"].get<string>());
			if (mit == krakenCurrencies.end()) continue; //if base currency not recognized, skip
			currency base = mit->second;

			mit = krakenCurrencies.find(it.value()["quote"].get<string>());
			if (mit == krakenCurrencies.end()) continue; //if quote currency not recognized, skip
			currency quote = mit->second;
			
			pairs.push_back(pair<string, CurrencyPair>(strPair, CurrencyPair(base, quote)));
		}
	}
	catch (exception& e) { printf(e.what()); }
	return pairs;
}
//get all supported currencies
vector<currency> KrakenAPI::getCurrencies() {
	vector<currency> currencyVect;
	json j;
	if (!this->sendRequest(RequestMethod::PUBLIC, "Assets", j) == kraken::OK)
		return currencyVect;
	
	json res = j["result"];
	for (json::iterator it = res.begin(); it != res.end(); it++) {
		map<string, currency>::iterator mit = krakenCurrencies.find(it.key());
		if (mit == krakenCurrencies.end()) continue;
		currencyVect.push_back(mit->second);
	}
	return currencyVect;
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
	catch (out_of_range& e) {
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

map<currency, float> KrakenAPI::getAccountBalance() {
	map<currency, float> balanceMap;
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "Balance", j) != kraken::OK)
		return balanceMap;

	json res = j["result"];
	map<string, currency>::iterator mit;
	for (json::iterator it = res.begin(); it != res.end(); it++) {
		mit = krakenCurrencies.find(it.key());
		if (mit == krakenCurrencies.end()) continue;
		currency cur = mit->second;
		string val = it.value().get<string>();
		balanceMap.insert(pair<currency, float>(cur, stof(val)));
	}
	return balanceMap;
}
json KrakenAPI::getTradeHistory() { 
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "TradesHistory", j) == kraken::OK)
		return j;
	return json();
}

vector<ProcessedOrder> KrakenAPI::openOrders() {
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "OpenOrders", j) != kraken::OK)
		return vector<ProcessedOrder>();
	return this->getOrdersFromJson(j, true);
}
vector<ProcessedOrder> KrakenAPI::closedOrders() {
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "ClosedOrders", j) != kraken::OK)
		return vector<ProcessedOrder>();
	return this->getOrdersFromJson(j, false);
}

json KrakenAPI::placeOrder(Order& order) {
	paramVect params;
	CurrencyPair cp = order.getPair();
	cp._exchange = exchange::KRAKEN; //set currencypair exchange to kraken
	params.push_back(pair<string, string>("pair", cp));
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
vector<Position> KrakenAPI::openPositions() {
	paramVect params{ {"docalcs", "true"} };
	vector<Position> positionVect;
	json j;
	if (this->sendRequest(RequestMethod::PRIVATE, "OpenPositions", j, &params) != kraken::OK)
		return positionVect;
	json result = j["result"];
	map<string, Position*> positions;
	for (json::iterator it = result.begin(); it != result.end(); it++) {
		json jpos = it.value();
		Position* pos = this->getPositionFromJson(jpos);
		string positionid = it.key(); //get unique position id and push to new position
		pos->pushPositionId(positionid); 
		string txId = jpos["ordertxid"].get<string>();
		map<string, Position*>::iterator mit = positions.find(txId);
		if (mit == positions.end()) { //if no current position with matching ordertxid exists, add position to map
			CurrencyPair cpair = this->makePair(jpos["pair"].get<string>());
			pos->setPair(cpair);
			positions.insert(pair<string, Position*>(txId, pos));
		}
		else { //otherwise add positions together and free new position
			Position* p = mit->second;
			*p += *pos;
			delete pos;
		}
	} //copy positions to vector and free objects
	for (map<string, Position*>::iterator mit = positions.begin(); mit != positions.end(); mit++) {
		positionVect.push_back(*mit->second);
		delete mit->second;
	}

	return positionVect;
}

json KrakenAPI::getPairInfo(string& pair) {
	paramVect params{ {"pair", pair} };
	json j;
	if (!this->sendRequest(RequestMethod::PUBLIC, "AssetPairs", j, &params) == kraken::OK)
		return json();
	return j["result"].begin().value();
}

CurrencyPair KrakenAPI::makePair(string& pair) {
	map<string, CurrencyPair>::iterator mit = this->knownPairs.find(pair);
	if (mit == this->knownPairs.end()) {
		json jpair = this->getPairInfo(pair);
		CurrencyPair cpair(krakenCurrencies[jpair["base"].get<string>()], krakenCurrencies[jpair["quote"].get<string>()]);
		this->knownPairs.insert({ pair, cpair });
		return cpair;
	}
	return mit->second;
}

Position* KrakenAPI::getPositionFromJson(json& j) {
	float margin = stof(j["margin"].get<string>()); //margin numeric
	float value = stof(j["value"].get<string>()); //value numeric
	string snet = j["net"].get<string>(); //net string with + or - char
	float net = stof(snet.substr(1, snet.size() - 1)); //chop off the +/- char at begin of string
	float cost = stof(j["cost"].get<string>());
	if (snet[0] == '-') net *= -1; //make negative of sign was -

	string type = j["type"].get<string>();
	string ordertype = j["ordertype"].get<string>();
	float quantity = stof(j["vol"].get<string>());
	float price = cost / quantity;
	float leverage = margin / cost;
	string txId = j["ordertxid"].get<string>();
	Order order(CurrencyPair(currency::NIL, currency::NIL), type, ordertype, price, quantity, leverage);
	return new Position (margin, value, net, cost, order);
}

vector<ProcessedOrder> KrakenAPI::getOrdersFromJson(json& j, bool open) {
	vector<ProcessedOrder> orderVect;
	json res = j["result"];
	if (open) res = res["open"];
	else res = res["closed"];
	for (json::iterator it = res.begin(); it != res.end(); it++) {
		json jpos = it.value();
		CurrencyPair cpair = this->makePair(jpos["descr"]["pair"].get<string>());
		string type = jpos["descr"]["type"].get<string>();
		string orderType = jpos["descr"]["ordertype"].get<string>();
		float price = stof(jpos["descr"]["price"].get<string>());
		float quantity = stof(jpos["vol"].get<string>());
		float leverage = 1;
		string lvg = jpos["descr"]["leverage"].get<string>();
		if (isdigit(lvg[0])) leverage = stof(lvg);
		Order order(cpair, type, orderType, price, quantity, leverage);

		string id = it.key();
		string time;
		if (open) time = to_string(jpos["opentm"].get<float>());
		else time = to_string(jpos["closetm"].get<float>());
		string status = jpos["status"].get<string>();
		float fee = stof(jpos["fee"].get<string>());
		ProcessedOrder p(id, time, status, fee, order);
		orderVect.push_back(p);
	}
	return orderVect;
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
