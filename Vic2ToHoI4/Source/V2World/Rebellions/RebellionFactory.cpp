#include "RebellionFactory.h"
#include "CommonRegexes.h"
#include "ParserHelpers.h"
#include "RebelArmy.h"



Vic2::Rebellion::Factory::Factory()
{
	registerKeyword("country", [this](std::istream& theStream) {
		rebellion->country = commonItems::singleString{theStream}.getString();
	});
	registerKeyword("independence", [this](std::istream& theStream) {
		rebellion->independence = commonItems::singleString{theStream}.getString();
	});
	registerKeyword("type", [this](std::istream& theStream) {
		rebellion->type = commonItems::singleString{theStream}.getString();
	});
	registerKeyword("government", [this](std::istream& theStream) {
		rebellion->government = commonItems::singleString{theStream}.getString();
	});
	registerKeyword("provinces", [this](std::istream& theStream) {
		rebellion->provinces = commonItems::intList(theStream).getInts();
	});
	registerKeyword("army", [this](std::istream& theStream) {
		Army rebelArmy(theStream);
		rebellion->armyIds.insert(rebelArmy.getId());
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}


std::unique_ptr<Vic2::Rebellion> Vic2::Rebellion::Factory::importRebellion(std::istream& theStream)
{
	rebellion = std::make_unique<Rebellion>();
	parseStream(theStream);
	return std::move(rebellion);
}