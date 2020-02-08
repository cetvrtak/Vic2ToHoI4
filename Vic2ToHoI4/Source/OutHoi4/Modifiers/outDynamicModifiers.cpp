#include "outDynamicModifiers.h"
#include "../../HOI4World/Modifiers/DynamicModifiers.h"
#include "../../HOI4World/Modifiers/Modifier.h"
#include "OSCompatibilityLayer.h"
#include <fstream>

namespace HoI4
{

class DynamicModifiers;

void outDynamicModifiers(const DynamicModifiers& dynamicModifiers, const Configuration& theConfiguration)
{
	if (!Utils::TryCreateFolder("output/" + theConfiguration.getOutputName() + "/common/dynamic_modifiers"))
	{
		Log(LogLevel::Error) << "Could not create output/" + theConfiguration.getOutputName() +
											 "/common/dynamic_modifiers/";
		exit(-1);
	}

	std::ofstream out(
		 "output/" + theConfiguration.getOutputName() + "/common/dynamic_modifiers/01_converter_modifiers.txt");
	if (!out.is_open())
	{
		LOG(LogLevel::Error) << "Could not create 01_converter_modifiers.txt.";
		exit(-1);
	}

	for (const auto& modifier: dynamicModifiers.getDynamicModifiers())
	{
		out << modifier.first << " = {\n" << modifier.second << "}\n";
		out << "\n";
	}

	out.close();
}

} // namespace HoI4