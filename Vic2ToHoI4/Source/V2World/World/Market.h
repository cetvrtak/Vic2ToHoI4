#ifndef MARKET_H_
#define MARKET_H_


#include "Parser.h"
#include <map>
#include <vector>



namespace Vic2
{

class Market: commonItems::parser
{
  public:
	Market(std::istream& theStream);

	[[nodiscard]] const auto& getMarketPool() const { return marketPool; }
	[[nodiscard]] const auto& getSupplyPool() const { return supplyPool; }
	[[nodiscard]] const auto& getPriceHistory() const { return priceHistory; }

  private:
	void registerKeys();

	std::map<std::string, std::string> marketPool;
	std::map<std::string, std::string> supplyPool;
	std::vector<std::map<std::string, std::string>> priceHistory;
};


} // namespace Vic2


#endif // !MARKET_H_