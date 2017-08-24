#ifndef ORDER_H
#define ORDER_H
#include "../settings.hpp"

class Order {
	string _pair;
	string _type;
	string _orderType;
	float _price;
	float _quantity;
	float _leverage;
public:
	Order(string pair, string type, string orderType, float price, float quantity, float leverage = 1.0f);
	string getPair();
	string getType();
	string getOrderType();
	float getPrice();
	float getQuantity();
	float getLeverage();
};

#endif