#include "RebelTypeFactory.h"
#include "CommonRegexes.h"
#include "ParserHelpers.h"



Vic2::RebelType::Factory::Factory()
{
	registerKeyword("government", [this](std::istream& theStream) {
		rebelType->governmentMap = commonItems::assignments(theStream).getAssignments();
	});
	registerKeyword("ideology", [this](std::istream& theStream) {
		rebelType->ideology = commonItems::singleString(theStream).getString();
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}


std::unique_ptr<Vic2::RebelType> Vic2::RebelType::Factory::importRebelType(std::istream& theStream)
{
	rebelType = std::make_unique<RebelType>();
	parseStream(theStream);
	return std::move(rebelType);
}