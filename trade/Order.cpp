#include "Order.hpp"

Order::Order(const Order& o)
	: _pair(o._pair), _type(o._type), _orderType(o._orderType), _price(o._price), _quantity(o._quantity), _leverage(o._leverage) {}

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