#include "ArmyId.h"
#include "CommonRegexes.h"
#include "ParserHelpers.h"


Vic2::ArmyId::ArmyId(std::istream& theStream)
{
	registerKeyword("id", [this](std::istream& theStream) {
		id = commonItems::singleString(theStream).getString();
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}