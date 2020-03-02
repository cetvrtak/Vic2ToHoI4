#include "HoI4Modifier.h"
#include "HoI4Localisation.h"
#include "ParserHelpers.h"



HoI4Modifier::HoI4Modifier(std::istream& theStream)
{
	registerKeyword("icon", [this](const std::string& unused, std::istream& theStream)
		{
			commonItems::singleString iconString(theStream);
			icon = iconString.getString();
		}
	);
	registerKeyword("enable", [this](const std::string& unused, std::istream& theStream)
		{
			commonItems::stringOfObject enableString(theStream);
			enable = enableString.getString();
		}
	);
	registerKeyword("remove_trigger", [this](const std::string& unused, std::istream& theStream)
		{
			commonItems::stringOfObject removeTriggerString(theStream);
			removeTrigger = removeTriggerString.getString();
		}
	);
	registerRegex("[a-zA-Z0-9_]+", [this](const std::string& theEffect, std::istream& theStream)
		{
			commonItems::singleString effectsString(theStream);
			effects.insert(make_pair(theEffect, effectsString.getString()));
		}
	);

	parseStream(theStream);
	clearRegisteredKeywords();
}

std::ostream& operator << (std::ostream& output, const HoI4Modifier& modifier)
{
	if (!modifier.icon.empty())
	{
		output << "\ticon = " << modifier.icon << "\n";
	}
	if (!modifier.enable.empty())
	{
		output << "\tenable " << modifier.enable << "\n";
	}
	if (!modifier.removeTrigger.empty())
	{
		output << "\tremove_trigger " << modifier.removeTrigger << "\n";
	}
	output << "\n";
	for (auto effect: modifier.effects)
	{
		output << "\t" << effect.first << " = " << effect.second << "\n";
	}

	return output;
}