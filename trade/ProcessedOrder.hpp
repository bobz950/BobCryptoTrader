#ifndef PROCESSEDORDER_H
#define PROCESSEDORDER_H
#include "../settings.hpp"
#include "Order.hpp"

class ProcessedOrder : public Order {
	string _orderId;
	string _time;
	string _status;
	float _fee;
public:
	ProcessedOrder(string id, string time, string status, float fee, Order& order);
	string getId();
	string getTime();
	string getStatus();
	float getFee();
};

#endif
