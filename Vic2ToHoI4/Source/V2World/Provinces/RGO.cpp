#include "RGO.h"
#include "CommonRegexes.h"
#include "ParserHelpers.h"
#include <sstream>
#include "Log.h"



Vic2::RGO::RGO(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

void Vic2::RGO::registerKeys()
{
	registerKeyword("employment", [this](std::istream& theStream) {
		auto rgoEmployment = RGO(theStream);
		employees = rgoEmployment.getEmployees();
	});
	registerKeyword("employees", [this](std::istream& theStream) {
		const auto& employeesStr = commonItems::blobList(theStream).getBlobs();
		for (const auto& employedPop: employeesStr)
		{
			auto newStr = std::stringstream(employedPop);
			auto rgoEmployees = RGO(newStr);

			employees += rgoEmployees.getEmployees();
		}
	});
	registerKeyword("count", [this](std::istream& theStream) {
		employees = commonItems::getInt(theStream);
	});
	registerKeyword("last_income", [this](std::istream& theStream) {
		income = commonItems::getDouble(theStream);
	});
	registerKeyword("goods_type", [this](std::istream& theStream) {
		type = commonItems::getString(theStream);
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}