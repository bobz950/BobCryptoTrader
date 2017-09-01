#include "ProcessedOrder.hpp"

ProcessedOrder::ProcessedOrder(string id, string time, string status, float fee, Order& order)
	: _orderId(id), _time(time), _status(status), _fee(fee), Order(order) {}

string ProcessedOrder::getId() {
	return this->_orderId;
}
string ProcessedOrder::getTime() {
	return this->_time;
}
string ProcessedOrder::getStatus() {
	return this->_status;
}
float ProcessedOrder::getFee() {
	return this->_fee;
}