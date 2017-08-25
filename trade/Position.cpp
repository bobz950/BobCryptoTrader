#include "Position.hpp"

Position::Position(float margin, float value, float net, float cost, CurrencyPair pair, string type, string orderType, float price, float quantity, float leverage) 
	: initialMargin(margin), currentValue(value), netProfitLoss(net), totalCost(cost), Order(pair, type, orderType, price, quantity, leverage) {}

void Position::pushPositionId(string id) { this->positionIds.push_back(id); }

float Position::getInitialMargin() {
	return this->initialMargin;
}
float Position::getCurrentValue() {
	return this->currentValue;
}
float Position::getNetProfitLoss() {
	return this->netProfitLoss;
}
float Position::getTotalCost() {
	return this->totalCost;
}

void Position::addMargin(float m) { this->initialMargin += m; }
void Position::addValue(float v) { this->currentValue += v; }
void Position::addNet(float n) { this->netProfitLoss += n; }
void Position::addCost(float c) { this->totalCost += c; }
void Position::addQuantity(float q) { this->_quantity += q; }
void Position::setPair(CurrencyPair p) { this->_pair = p; }

Position& Position::operator += (Position& p) {
	this->addMargin(p.getInitialMargin());
	this->addValue(p.getCurrentValue());
	this->addNet(p.getNetProfitLoss());
	this->addCost(p.getTotalCost());
	this->addQuantity(p.getQuantity());
	for (vector<string>::iterator it = p.positionIds.begin(); it != p.positionIds.end(); it++)
		this->positionIds.push_back(*it);
	return *this;
}