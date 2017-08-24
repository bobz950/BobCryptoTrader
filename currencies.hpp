#ifndef CURRENCIES_H
#define CURRENCIES_H
#include <map>
using namespace std;

namespace currencies {
	//supported currencies
	enum class currency {
		BTC = 'BTC',
		ETH = 'ETH',
		LTC = 'LTC',
		BCH = 'BCH',
		DASH = 'DASH',
		EOS = 'EOS',
		GNO = 'GNO',
		FEE = 'FEE',
		USDT = 'USDT',
		DAO = 'DAO',
		ETC = 'ETC',
		ICN = 'ICN',
		MLN = 'MLN',
		NMC = 'NMC',
		REP = 'REP',
		XDG = 'XDG',
		XLM = 'XLM',
		XMR = 'XMR',
		XRP = 'XRP',
		XVN = 'XVN',
		ZEC = 'ZEC',
		CAD = 'CAD',
		EUR = 'EUR',
		GBP = 'GBP',
		JPY = 'JPY',
		KRW = 'KRW',
		USD = 'USD',
		NIL = 'NIL'
	};

	struct CurrencyPair {
		CurrencyPair(currency base, currency quote) : _base(base), _quote(quote) {}
		currency _base;
		currency _quote;
	};

	//constructs string representation of currency
	static string currencyToString(currency c) {
		string r = "";
		char* p = ((char*)&c) + 3;
		for (int i = 0; i < 4; i++) {
			if (*p != '\0') r.push_back(*p);
			p--;
		}
		return r;
	}

	//gets the currency represented by a string
	static currency stringToCurrency(string& s) {
		const char* c = s.c_str();
		int size = s.size();
		switch (size) {
		case 3:
			return (currency)((((c[0] << 8) + c[1]) << 8) + c[2]);
		case 4:
			return (currency)((((((c[0] << 8) + c[1]) << 8) + c[2]) << 8) + c[3]);

		case 5:
			return (currency)((((((((c[0] << 8) + c[1]) << 8) + c[2]) << 8) + c[3]) << 8) + c[4]);
		}
		return currency::NIL;
	}


	typedef vector<pair<string, CurrencyPair>> pairVect;
	
	//mapping of kraken currency strings to currency types
	static map<string, currency> krakenCurrencies { {"BCH", currency::BCH},{ "DASH", currency::DASH },{ "EOS", currency::EOS },{ "GNO", currency::GNO },
	{ "KFEE", currency::FEE },{ "USDT", currency::USDT },{ "XDAO", currency::DAO },{ "XETC", currency::ETC },{ "XETH", currency::ETH },{ "XICN", currency::ICN },
	{ "XLTC", currency::LTC },{ "XMLN", currency::MLN },{ "XNMC", currency::NMC },{ "XREP", currency::REP },{ "XXBT", currency::BTC },{ "XXDG", currency::XDG },
	{ "XXLM", currency::XLM },{ "XXMR", currency::XMR },{ "XXRP", currency::XRP },{ "XXVN", currency::XVN },{ "XZEC", currency::ZEC },{ "ZCAD",currency::CAD },
	{ "ZEUR", currency::EUR },{ "ZGBP", currency::GBP },{ "ZJPY", currency::JPY },{ "ZKRW", currency::KRW },{ "ZUSD", currency::USD } };
}

#endif
