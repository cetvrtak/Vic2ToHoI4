#include "SphereDecisions.h"
#include "..\HoI4Country.h"
#include "Log.h"


const std::string othersInfluencingBlockBuilder(const std::vector<std::shared_ptr<HoI4::Country>>& greatPowers);
const std::string commonTargetTriggerBlockBuilder(const std::vector<std::shared_ptr<HoI4::Country>>& greatPowers);
HoI4::decision updateDecreaseAutonomy(HoI4::decision decision,const std::string commonTargetTriggerBlock);
HoI4::decision updateAddToSphere(HoI4::decision decision,const std::string commonTargetTriggerBlock);
HoI4::decision updateIncreaseAutonomy(HoI4::decision decision,const std::string commonTargetTriggerBlock);
HoI4::decision updateRemoveFromSphere(HoI4::decision decision,const std::string commonTargetTriggerBlock);
HoI4::decision updateRemoveFromSphereFreebie(HoI4::decision decision,const std::string commonTargetTriggerBlock);
HoI4::decision updateGuarantee(HoI4::decision decision,const std::string commonTargetTriggerBlock);

void HoI4::SphereDecisions::updateSphereDecisions(const std::vector<std::shared_ptr<HoI4::Country>>& greatPowers)
{
	for (auto& category: decisions)
	{
		if (category.getName() == "sphere_decisions")
		{
			//auto copyCat = category;
			//copyCat.clear();
			for (auto& decision: category.getDecisions())
			{
				//const std::string oIBlock = othersInfluencingBlockBuilder(greatPowers);
				const std::string cTTBlock = commonTargetTriggerBlockBuilder(greatPowers);
				decision.setTargetTrigger(cTTBlock);
				category.replaceDecision(decision);

				/*if (decision.getName() == "decrease_autonomy")
				{
					copyCat.addDecision(updateDecreaseAutonomy(decision,cTTBlock));
				}

				if (decision.getName() == "add_to_sphere")
				{
					copyCat.addDecision(updateAddToSphere(decision,cTTBlock));
				}

				if (decision.getName() == "increase_autonomy")
				{
					copyCat.addDecision(updateIncreaseAutonomy(decision,cTTBlock));
				}

				if (decision.getName() == "remove_from_sphere")
				{
					copyCat.addDecision(updateRemoveFromSphere(decision,cTTBlock));
				}

				if (decision.getName() == "remove_from_sphere_freebie")
				{
					copyCat.addDecision(updateRemoveFromSphereFreebie(decision,cTTBlock));
				}

				if (decision.getName() == "guarantee")
				{
					copyCat.addDecision(updateGuarantee(decision,cTTBlock));
				}*/
			}

			//category = copyCat;
		}
	}
}

const std::string othersInfluencingBlockBuilder(const std::vector<std::shared_ptr<HoI4::Country>>& greatPowers)
{
	std::string othersInfluencingBlock;
	for (const auto& GP: greatPowers)
	{
		othersInfluencingBlock += "\t\t\t\t\t\thas_idea = gp_influence_" + GP->getTag() + "\n";
	}

	return othersInfluencingBlock;
}

const std::string commonTargetTriggerBlockBuilder(const std::vector<std::shared_ptr<HoI4::Country>>& greatPowers)
{
	std::string block;
	block += "= {\n";
	block += "\t\t\tFROM = {\n";
	for (const auto& GP: greatPowers)
	{
		block += "\t\t\t\tif = { limit = { ROOT = { tag = " + GP->getTag() + " } }\n";
		block += "\t\t\t\tOR = {";
		for (const auto& sphereling: GP->getPotentialSpherelings())
		{
			block += " tag=" + sphereling;
		}
		block += " } }\n";
	}
	block += "\t\t\t}\n";
	block += "\t\t}\n";

	return block;
}

HoI4::decision updateDecreaseAutonomy(
	HoI4::decision decision,
	const std::string commonTargetTriggerBlock
) {
	decision.setTargetTrigger(commonTargetTriggerBlock);

	return decision;
}

HoI4::decision updateAddToSphere(
	HoI4::decision decision,
	const std::string commonTargetTriggerBlock
) {
	decision.setTargetTrigger(commonTargetTriggerBlock);

	return decision;
}

HoI4::decision updateIncreaseAutonomy(
	HoI4::decision decision,
	const std::string commonTargetTriggerBlock
) {
	decision.setTargetTrigger(commonTargetTriggerBlock);

	return decision;
}

HoI4::decision updateRemoveFromSphere(
	HoI4::decision decision,
	const std::string commonTargetTriggerBlock
) {
	decision.setTargetTrigger(commonTargetTriggerBlock);

	return decision;
}

HoI4::decision updateRemoveFromSphereFreebie(
	HoI4::decision decision,
	const std::string commonTargetTriggerBlock
) {
	decision.setTargetTrigger(commonTargetTriggerBlock);

	return decision;
}

HoI4::decision updateGuarantee(
	HoI4::decision decision,
	const std::string commonTargetTriggerBlock
) {
	decision.setTargetTrigger(commonTargetTriggerBlock);

	return decision;
}

