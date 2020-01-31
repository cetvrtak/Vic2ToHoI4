#include "SphereDecisions.h"
#include "..\HoI4Country.h"
#include "Log.h"


const std::string othersInfluencingBlockBuilder(const std::vector<std::shared_ptr<HoI4::Country>>& greatPowers);
const std::string commonTargetTriggerBlockBuilder(const std::vector<std::shared_ptr<HoI4::Country>>& greatPowers);
HoI4::decision&& updateDecreaseAutonomy(
	HoI4::decision&& decision,
	const std::string commonTargetTriggerBlock,
	const std::string othersInfluencingBlock
);
HoI4::decision&& updateAddToSphere(HoI4::decision&& decision,const std::string commonTargetTriggerBlock);
HoI4::decision customizeIncreaseAutonomy(
	HoI4::decision& decision,
	const std::shared_ptr<HoI4::Country>& GP,
	const std::string commonTargetTriggerBlock,
	const std::string othersInfluencingBlock
);
HoI4::decision&& updateRemoveFromSphere(HoI4::decision&& decision,const std::string commonTargetTriggerBlock);
HoI4::decision&& updateGuarantee(HoI4::decision&& decision,const std::string commonTargetTriggerBlock);

void HoI4::SphereDecisions::updateSphereDecisions(const std::vector<std::shared_ptr<HoI4::Country>>& greatPowers)
{
	for (auto& category: decisions)
	{
		for (auto& decision: category.getDecisions())
		{
			const std::string oIBlock = othersInfluencingBlockBuilder(greatPowers);
			const std::string cTTBlock = commonTargetTriggerBlockBuilder(greatPowers);

			if (decision.getName() == "decrease_autonomy")
			{
				category.replaceDecision(updateDecreaseAutonomy(std::move(decision),cTTBlock,oIBlock));
			}

			if (decision.getName() == "add_to_sphere")
			{
				category.replaceDecision(updateAddToSphere(std::move(decision),cTTBlock));
			}

			if (decision.getName() == "increase_autonomy_")
			{
				auto customizedDecision = decision;
				for (const auto& GP: greatPowers)
				{
					category.addDecision(customizeIncreaseAutonomy(customizedDecision,GP,cTTBlock,oIBlock));
				}
			}

			if (decision.getName() == "remove_from_sphere")
			{
				category.replaceDecision(updateRemoveFromSphere(std::move(decision),cTTBlock));
			}

			if (decision.getName() == "guarantee")
			{
				category.replaceDecision(updateGuarantee(std::move(decision),cTTBlock));
			}
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

HoI4::decision&& updateDecreaseAutonomy(
	HoI4::decision&& decision,
	const std::string commonTargetTriggerBlock,
	const std::string othersInfluencingBlock
) {
	decision.setTargetTrigger(commonTargetTriggerBlock);

	std::string available;
	available  = "= {\n";
	available += "\t\t\tcustom_trigger_tooltip = { tooltip = autonomy_at_18_or_not_influence_by_another_gp_tt }\n";
	available += "\t\t\thidden_trigger = {\n";
	available += "\t\t\t\tFROM = {\n";
	available += "\t\t\t\t\tOR = {\n";
	available += "\t\t\t\t\t\tcompare_autonomy_progress_ratio > 0.18\n";
	available += othersInfluencingBlock;
	available += "\t\t\t\t\t}\n";
	available += "\t\t\t\t}\n";
	available += "\t\t\t}\n";
	available += "\t\t\tNOT = { has_war_with = FROM }\n";
	available += "\t\t\tNOT = { has_truce_with = FROM }\n";
	available += "\t\t}\n";
	decision.setAvailable(available);

	std::string aiWillDo;
	aiWillDo  = "= {\n";
	aiWillDo += "\t\t\tfactor = 2\n";
	aiWillDo += "\t\t\tmodifier = {\n";
	aiWillDo += "\t\t\t\tfactor = 0.1\n";
	aiWillDo += "\t\t\t\tFROM = {\n";
	aiWillDo += "\t\t\t\t\t#compare_autonomy_progress_ratio < 0.5\n";
	aiWillDo += "\t\t\t\t\tNOT = {\n";
	aiWillDo += othersInfluencingBlock;
	aiWillDo += "\t\t\t\t\t}\n";
	aiWillDo += "\t\t\t\t}\n";
	aiWillDo += "\t\t\t}\n";
	aiWillDo += "\t\t\tmodifier = {\n";
	aiWillDo += "\t\t\t\tfactor = 2\n";
	aiWillDo += "\t\t\t\tFROM = {\n";
	aiWillDo += "\t\t\t\t\tOR = {\n";
	aiWillDo += othersInfluencingBlock;
	aiWillDo += "\t\t\t\t\t}\n";
	aiWillDo += "\t\t\t\t}\n";
	aiWillDo += "\t\t\t}\n";
	aiWillDo += "\t\t\tmodifier = {\n";
	aiWillDo += "\t\t\t\tfactor = 2\n";
	aiWillDo += "\t\t\t\tFROM = {\n";
	aiWillDo += "\t\t\t\t\tcount_triggers = {\n";
	aiWillDo += "\t\t\t\t\t\tamount = 2\n";
	aiWillDo += othersInfluencingBlock;
	aiWillDo += "\t\t\t\t\t}\n";
	aiWillDo += "\t\t\t\t}\n";
	aiWillDo += "\t\t\t}\n";
	aiWillDo += "\t\t}\n";
	decision.setAiWillDo(aiWillDo);

	return std::move(decision);
}

HoI4::decision&& updateAddToSphere(
	HoI4::decision&& decision,
	const std::string commonTargetTriggerBlock
) {
	decision.setTargetTrigger(commonTargetTriggerBlock);

	return std::move(decision);
}

HoI4::decision customizeIncreaseAutonomy(
	HoI4::decision& decision,
	const std::shared_ptr<HoI4::Country>& GP,
	const std::string commonTargetTriggerBlock,
	const std::string othersInfluencingBlock
)
{
	const std::string decisionName = "increase_autonomy_" + GP->getTag();
	decision.setName(decisionName);

	const std::string allowed = "= { tag = " + GP->getTag() + " }\n";
	decision.setAllowed(allowed);

	std::string targetTrigger;
	targetTrigger  = "= {\n";
	targetTrigger += "\t\t\tFROM = {\n";
	targetTrigger += "\t\t\t\tOR = {";
	for (const auto& sphereling: GP->getPotentialSpherelings())
	{
		targetTrigger += " tag=" + sphereling;
	}
	targetTrigger += " }\n";
	targetTrigger += "\t\t\t}\n";
	targetTrigger += "\t\t}\n";
	decision.setTargetTrigger(targetTrigger);

	std::string visible;
	visible  = "= {\n";
	visible += "\t\t\ttag = " + GP->getTag() + "\n";
	visible += "\t\t\tFROM = {\n";
	visible += "\t\t\t\tNOT = { is_subject_of = ROOT }\n";
	visible += "\t\t\t\thas_autonomy_state = autonomy_sphereling\n";
	visible += "\t\t\t\tcompare_autonomy_progress_ratio < 1\n";
	visible += "\t\t\t}\n";
	visible += "\t\t}\n";
	decision.setVisible(visible);

	std::string completeEffect;
	completeEffect  = "= {\n";
	completeEffect += "\t\t\tFROM = {\n";
	completeEffect += "\t\t\t\tadd_timed_idea = {\n";
	completeEffect += "\t\t\t\t\tidea = gp_influence_" + GP->getTag() + "\n";
	completeEffect += "\t\t\t\t\tdays = 180\n";
	completeEffect += "\t\t\t\t}\n";
	completeEffect += "\t\t\t}\n";
	completeEffect += "\t\t}\n";
	decision.setCompleteEffect(completeEffect);

	std::string aiWillDo;
	aiWillDo  = "= {\n";
	aiWillDo += "\t\t\tfactor = 1\n";
	aiWillDo += "\t\t\tmodifier = {\n";
	aiWillDo += "\t\t\t\tfactor = 0\n";
	aiWillDo += "\t\t\t\tAND = {\n";
	aiWillDo += "\t\t\t\t\tcompare_autonomy_progress_ratio > 0.865\n";
	aiWillDo += "\t\t\t\t\tNOT = { has_idea = gp_influence_overlord }\n";
	aiWillDo += "\t\t\t\t\tOR = {\n";
	aiWillDo += othersInfluencingBlock;
	aiWillDo += "\t\t\t\t\t}\n";
	aiWillDo += "\t\t\t\t}\n";
	aiWillDo += "\t\t\t}\n";
	aiWillDo += "\t\t}";
	decision.setAiWillDo(aiWillDo);

	return decision;
}

HoI4::decision&& updateRemoveFromSphere(
	HoI4::decision&& decision,
	const std::string commonTargetTriggerBlock
) {
	decision.setTargetTrigger(commonTargetTriggerBlock);

	return std::move(decision);
}

HoI4::decision&& updateGuarantee(
	HoI4::decision&& decision,
	const std::string commonTargetTriggerBlock
) {
	decision.setTargetTrigger(commonTargetTriggerBlock);

	return std::move(decision);
}

