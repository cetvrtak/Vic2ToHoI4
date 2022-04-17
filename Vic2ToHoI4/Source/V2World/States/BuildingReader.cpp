#include "BuildingReader.h"
#include "../Provinces/RGO.h"
#include "CommonRegexes.h"
#include "ParserHelpers.h"



Vic2::BuildingReader::BuildingReader()
{
	registerKeyword("level", [this](std::istream& theStream) {
		level = commonItems::singleInt{theStream}.getInt();
	});
	registerKeyword("employment", [this](std::istream& theStream) {
		const auto rgo = RGO(theStream);
		employees = rgo.getEmployees();
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}


int Vic2::BuildingReader::getLevel(std::istream& theStream)
{
	level = 0;
	parseStream(theStream);
	return level;
}
