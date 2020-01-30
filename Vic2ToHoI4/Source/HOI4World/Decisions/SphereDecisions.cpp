#include "SphereDecisions.h"
#include "..\HoI4Country.h"
#include "Log.h"


const std::string othersInfluencingBlockBuilder(const std::vector<std::shared_ptr<HoI4::Country>>& greatPowers);
HoI4::decision customizeIncreaseAutonomy(HoI4::decision&, const std::shared_ptr<HoI4::Country>&, const std::string);

void HoI4::SphereDecisions::updateSphereDecisions(const std::vector<std::shared_ptr<HoI4::Country>>& greatPowers)
{
	for (auto& category: decisions)
	{
		if (!category.getName().empty()) {
			LOG(LogLevel::Debug) << "Category: " << category.getName();
		}
		else {
			LOG(LogLevel::Debug) << "No category found";
		}
		for (auto& decision: category.getDecisions())
		{
			const std::string othersInfluencingBlock = othersInfluencingBlockBuilder(greatPowers);

			if (!decision.getName().empty()) {
				LOG(LogLevel::Debug) << "Decision: " << decision.getName();
				if (decision.getName() == "increase_autonomy_")
				{
					LOG(LogLevel::Info) << "Decision IS increase_autonomy_";
					auto customizedDecision = decision;
					LOG(LogLevel::Info) << "After copying - Customized Decision: " << customizedDecision.getName();
					for (const auto& GP: greatPowers)
					{
						category.addDecision(customizeIncreaseAutonomy(customizedDecision, GP, othersInfluencingBlock));
						LOG(LogLevel::Info) << "Customized decision for " << GP->getTag() << "added to category";
					}
				}
			}
			else {
				LOG(LogLevel::Debug) << "No decision found";
			}
			LOG(LogLevel::Info) << "Decision is now known as: " << decision.getName();
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

HoI4::decision customizeIncreaseAutonomy(
	HoI4::decision& decision,
	const std::shared_ptr<HoI4::Country>& GP,
	const std::string othersInfluencingBlock
)
{
	const std::string decisionName = "increase_autonomy_" + GP->getTag();
	LOG(LogLevel::Info)	<< "decisionName: " << decisionName;
	decision.setName(decisionName);
	LOG(LogLevel::Info) << "After updating - Customized Decision: " << decision.getName();

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
	completeEffect += "= {\n";
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
	aiWillDo += "\t\t}\n";
	decision.setAiWillDo(aiWillDo);

	return decision;
}
