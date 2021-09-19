#include "RebelTypesFactory.h"
#include "CommonRegexes.h"
#include "Log.h"
#include "OSCompatibilityLayer.h"
#include "ParserHelpers.h"
#include "RebelTypeFactory.h"



Vic2::RebelTypes::Factory::Factory()
{
	registerRegex(commonItems::catchallRegex, [this](const std::string& rebelType, std::istream& theStream) {
		rebelTypes->rebelTypesMap.insert(std::make_pair(rebelType, (*RebelType::Factory().importRebelType(theStream))));
	});
}


std::unique_ptr<Vic2::RebelTypes> Vic2::RebelTypes::Factory::importRebelTypes(const Configuration& theConfiguration)
{
	Log(LogLevel::Info) << "Importing rebel types";

	rebelTypes = std::make_unique<RebelTypes>();
	parseFile(theConfiguration.getVic2Path() + "/common/rebel_types.txt");
	for (const auto& mod: theConfiguration.getVic2Mods())
	{
		if (commonItems::DoesFileExist(mod.path + "/common/rebel_types.txt"))
		{
			Log(LogLevel::Info) << "\tReading mod rebels from " << mod.name;
			rebelTypes->rebelTypesMap.clear();
			parseFile(mod.path + "/common/rebel_types.txt");
		}
	}

	return std::move(rebelTypes);
}