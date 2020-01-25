#include "SphereDecisions.h"
#include "HoI4World.h"
#include "HoI4Country.h"


void HoI4::SphereDecisions::outputTargetTrigger(
	std::vector<std::shared_ptr<HoI4::Country>> greatPowers,
	std::vector<std::string> potentialSpherelings
)
{
	for (auto GP: getGreatPowers())
	{
		output << "\t\t\t\tif = {\n";
		output << "\t\t\t\t\tlimit = { ROOT = { tag = " << GP->getTag() << " } }\n";
		output << "\t\t\t\t\tOR = {\n";

		for (auto spherelingTag: GP->getPotentialSpherelings())
		{
		output << "\t\t\t\t\t\ttag = " << spherelingTag->getTag() << "\n";
		}

		output << "\t\t\t\t\t}\n";
		output << "\t\t\t\t}\n";
	}
}