#include "Order.hpp"

Order::Order(CurrencyPair pair, string type, string orderType, float price, float quantity, float leverage)
	: _pair(pair), _type(type), _orderType(orderType), _price(price), _quantity(quantity), _leverage(leverage) {}

CurrencyPair Order::getPair() {
	return this->_pair;
}
string Order::getType() {
	return this->_type;
}
string Order::getOrderType() {
	return this->_orderType;
}
float Order::getPrice() {
	return this->_price;
}
float Order::getQuantity() {
	return this->_quantity;
}
float Order::getLeverage() {
	return this->_leverage;
}