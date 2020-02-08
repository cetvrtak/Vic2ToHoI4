#include "OutModifier.h"

std::ostream& HoI4::operator << (std::ostream& output, const HoI4::Modifier& modifier)
{
	if (!modifier.getIcon().empty())
	{
		output << "\ticon = " << modifier.getIcon() << "\n";
	}
	if (!modifier.getEnable().empty())
	{
		output << "\tenable " << modifier.getEnable() << "\n";
	}
	if (!modifier.getRemoveTrigger().empty())
	{
		output << "\tremove_trigger " << modifier.getRemoveTrigger() << "\n";
	}
	output << "\n";
	for (auto effect: modifier.getEffects())
	{
		output << "\t" << effect.first << " = " << effect.second << "\n";
	}

	return output;
}