#ifndef POSITION_H
#define POSITION_H
#include "../settings.hpp"
#include "Order.hpp"

class Position : public Order {
	vector<string> positionIds;
	float initialMargin;
	float currentValue;
	float netProfitLoss;
	float totalCost;
public:
	Position(float margin, float value, float net, float cost, Order& order);
	void pushPositionId(string id);
	float getInitialMargin();
	float getCurrentValue();
	float getNetProfitLoss();
	float getTotalCost();

	void addMargin(float m);
	void addValue(float v);
	void addNet(float n);
	void addCost(float c);
	void addQuantity(float q);

	void setPair(CurrencyPair p);
	Position& operator += (Position& p);
};

#endif
