#include "RebelArmy.h"
#include "CommonRegexes.h"
#include "ParserHelpers.h"



Vic2::Rebellion::Army::Army(std::istream& theStream)
{
	registerKeyword("id", [this](std::istream& theStream) {
		id = commonItems::singleString(theStream).getString();
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);

	parseStream(theStream);
	clearRegisteredKeywords();
}