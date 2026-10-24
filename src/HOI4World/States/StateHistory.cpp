#include "src/HOI4World/States/StateHistory.h"
#include "external/common_items/CommonRegexes.h"
#include "external/common_items/ParserHelpers.h"
#include "src/HOI4World/States/StateBuildings.h"



HoI4::StateHistory::StateHistory(std::istream& theStream)
{
	registerKeyword("owner", [this](std::istream& theStream) {
		commonItems::singleString ownerString(theStream);
		owner = ownerString.getString();
	});
	registerKeyword("buildings", [this](std::istream& theStream) {
		HoI4::StateBuildings theBuildings(theStream);
		civFactories = theBuildings.getCivFactories();
		milFactories = theBuildings.getMilFactories();
		dockyards = theBuildings.getDockyards();
	});
	registerKeyword("victory_points", [this](std::istream& theStream) {
		const auto& vps = commonItems::getDoubles(theStream);
		int province = static_cast<int>(vps[0]);
		int vp = static_cast<int>(vps[1]);
		victoryPoints[province] = vp;
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);

	parseStream(theStream);
}