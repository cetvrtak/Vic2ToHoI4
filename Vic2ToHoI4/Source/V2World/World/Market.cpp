#include "Market.h"
#include "CommonRegexes.h"
#include "OSCompatibilityLayer.h"
#include "ParserHelpers.h"


Vic2::Market::Market(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}


void Vic2::Market::registerKeys()
{
	registerKeyword("worldmarket_pool", [this](std::istream& theStream) {
		marketPool = commonItems::assignments(theStream).getAssignments();
	});
	registerKeyword("supply_pool", [this](std::istream& theStream) {
		supplyPool = commonItems::assignments(theStream).getAssignments();
	});
	registerKeyword("price_history", [this](std::istream& theStream) {
		priceHistory.push_back(commonItems::assignments(theStream).getAssignments());
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}