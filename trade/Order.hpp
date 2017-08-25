#ifndef ORDER_H
#define ORDER_H
#include "../settings.hpp"
#include "../currencies.hpp"
using namespace currencies;

class Order {
protected:
	CurrencyPair _pair;
	string _type; //buy or sell
	string _orderType; //market, limit, stop loss, etc..
	float _price;
	float _quantity;
	float _leverage;
public:
	Order(CurrencyPair pair, string type, string orderType, float price, float quantity, float leverage = 1.0f);
	CurrencyPair getPair();
	string getType();
	string getOrderType();
	float getPrice();
	float getQuantity();
	float getLeverage();
};

#endif