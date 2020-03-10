
/*Copyright (c) 2018 The Paradox Game Converters Project
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/



#include "HoI4FocusTree.h"
#include "HoI4Country.h"
#include "HoI4Focus.h"
#include "HoI4Localisation.h"
#include "HoI4World.h"
#include "States/HoI4State.h"
#include "../Configuration.h"
#include "../V2World/Party.h"
#include "../V2World/Country.h"
#include "Log.h"
#include "OSCompatibilityLayer.h"
#include "ParserHelpers.h"
#include <fstream>
using namespace std;



//this is used to cache focuses that can be just loaded from a file
static std::map<std::string, HoI4Focus> loadedFocuses;



HoI4FocusTree::HoI4FocusTree(const HoI4::Country& country):
	srcCountryTag(country.getSourceCountry().getTag()),
	dstCountryTag(country.getTag())
{
}


void HoI4FocusTree::addGenericFocusTree(const set<string>& majorIdeologies)
{
	confirmLoadedFocuses();

	int numCollectovistIdeologies = calculateNumCollectovistIdeologies(majorIdeologies);

	if (const auto& originalFocus = loadedFocuses.find("political_effort"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		newFocus->xPos = static_cast<int>((numCollectovistIdeologies * 1.5) + 16);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus political_effort");
	}

	if (numCollectovistIdeologies > 0)
	{
		if (const auto& originalFocus = loadedFocuses.find("collectivist_ethos"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
			newFocus->available = "= {\n";
			newFocus->available += "			OR = {\n";
			for (auto majorIdeology : majorIdeologies)
			{
				if (majorIdeology == "democratic")
				{
					continue;
				}
				newFocus->available += "				has_government = " + majorIdeology + "\n";
			}
			newFocus->available += "			}\n";
			newFocus->available += "		}";
			newFocus->xPos = -(numCollectovistIdeologies / 2) - 1;
			newFocus->completionReward = "= {\n";
			if (majorIdeologies.count("democratic") > 0)
			{
				newFocus->completionReward += "			add_ideas = collectivist_ethos_focus_democratic\n";
			}
			else
			{
				newFocus->completionReward += "			add_ideas = collectivist_ethos_focus_neutral\n";
			}
			newFocus->completionReward += "		}";
			sharedFocuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus collectivist_ethos");
		}

		determineMutualExclusions(majorIdeologies);

		string ideolgicalFanaticsmPrereqs;
		int relativePosition = 1 - numCollectovistIdeologies;
		if (majorIdeologies.count("fascism") > 0)
		{
			addFascistGenericFocuses(relativePosition, majorIdeologies);
			ideolgicalFanaticsmPrereqs += " ";
			ideolgicalFanaticsmPrereqs += "focus = paramilitarism";
			relativePosition += 2;
		}
		if (majorIdeologies.count("communism") > 0)
		{
			addCommunistGenericFocuses(relativePosition);
			if (ideolgicalFanaticsmPrereqs.size() > 0)
			{
				ideolgicalFanaticsmPrereqs += " ";
			}
			ideolgicalFanaticsmPrereqs += "focus = political_commissars";
			relativePosition += 2;
		}
		if (majorIdeologies.count("absolutist") > 0)
		{
			addAbsolutistGenericFocuses(relativePosition);
			if (ideolgicalFanaticsmPrereqs.size() > 0)
			{
				ideolgicalFanaticsmPrereqs += " ";
			}
			ideolgicalFanaticsmPrereqs += "focus = historical_claims_focus";
			relativePosition += 2;
		}
		if (majorIdeologies.count("radical") > 0)
		{
			addRadicalGenericFocuses(relativePosition);
			if (ideolgicalFanaticsmPrereqs.size() > 0)
			{
				ideolgicalFanaticsmPrereqs += " ";
			}
			ideolgicalFanaticsmPrereqs += "focus = army_provides_focus";
		}

		if (const auto& originalFocus = loadedFocuses.find("ideological_fanaticism"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
			newFocus->prerequisites.clear();
			newFocus->prerequisites.push_back("= { " + ideolgicalFanaticsmPrereqs + " }");
			newFocus->xPos = 0;
			newFocus->yPos = 5;
			newFocus->relativePositionId = "collectivist_ethos";
			sharedFocuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus ideological_fanaticism");
		}
	}

	if (const auto& originalFocus = loadedFocuses.find("liberty_ethos"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		if (numCollectovistIdeologies == 0)
		{
			newFocus->mutuallyExclusive.clear();
		}
		if (majorIdeologies.count("democratic") == 0)
		{
			newFocus->available = "= {\n";
			newFocus->available += "			has_government = neutrality\n";
			newFocus->available += "		}";

			newFocus->completionReward = "= {\n";
			newFocus->completionReward += "	add_ideas = liberty_ethos_focus_neutral\n";
			newFocus->completionReward += "}";
		}
		newFocus->xPos = (numCollectovistIdeologies + 1) / 2;
		newFocus->aiWillDo = "= {\n";
		newFocus->aiWillDo += "			factor = 95\n";
		newFocus->aiWillDo += "			modifier = {\n";
		newFocus->aiWillDo += "				factor = 0.1\n";
		newFocus->aiWillDo += "				any_neighbor_country = {\n";
		newFocus->aiWillDo += "					is_major = yes\n";
		newFocus->aiWillDo += "					OR = {\n";
		for (auto majorIdeology : majorIdeologies)
		{
			newFocus->aiWillDo += "						has_government = " + majorIdeology + "\n";
		}
		newFocus->aiWillDo += "					}\n";
		newFocus->aiWillDo += "				}\n";
		if (majorIdeologies.count("democratic") > 0)
		{
			newFocus->aiWillDo += "				NOT = {\n";
			newFocus->aiWillDo += "					any_neighbor_country = {\n";
			newFocus->aiWillDo += "						is_major = yes\n";
			newFocus->aiWillDo += "						has_government = democratic\n";
			newFocus->aiWillDo += "					}\n";
			newFocus->aiWillDo += "				}\n";
		}
		newFocus->aiWillDo += "			}\n";
		newFocus->aiWillDo += "		}";
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus liberty_ethos");
	}

	if (const auto& originalFocus = loadedFocuses.find("neutrality_focus"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		if (majorIdeologies.count("democratic") == 0)
		{
			newFocus->mutuallyExclusive.clear();
		}
		newFocus->aiWillDo = "= {\n";
		newFocus->aiWillDo += "			factor = 1\n";
		newFocus->aiWillDo += "		}";
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus neutrality_focus");
	}

	if (const auto& originalFocus = loadedFocuses.find("deterrence"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus deterrence");
	}

	if (majorIdeologies.count("democratic") != 0)
	{
		if (const auto& originalFocus = loadedFocuses.find("interventionism_focus"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
			sharedFocuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus interventionism_focus");
		}

		if (const auto& originalFocus = loadedFocuses.find("volunteer_corps"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
			sharedFocuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus volunteer_corps");
		}

		if (const auto& originalFocus = loadedFocuses.find("foreign_expeditions"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
			sharedFocuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus foreign_expeditions");
		}
	}

	if (const auto& originalFocus = loadedFocuses.find("why_we_fight"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		if (majorIdeologies.count("democratic") == 0)
		{
			newFocus->prerequisites.clear();
			newFocus->prerequisites.push_back("= { focus = deterrence }");
		}
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus why_we_fight");
	}

	if (const auto& originalFocus = loadedFocuses.find("technology_sharing"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		if (numCollectovistIdeologies == 0)
		{
			newFocus->prerequisites.clear();
			newFocus->prerequisites.push_back("= { focus = why_we_fight }");
		}
		newFocus->xPos = numCollectovistIdeologies;
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus technology_sharing");
	}

	if (const auto& originalFocus = loadedFocuses.find("border_disputes"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		if (!majorIdeologies.count("communism"))
		{
			std::string comm = "";
			comm += "\n\t\t\t\t\tif = {\n";
			comm += "\t\t\t\t\t\tlimit = { ROOT = { has_government = communism } }\n";
			comm += "\t\t\t\t\t\tpotential_communist_puppet_target = yes\n";
			comm += "\t\t\t\t\t}";
			newFocus->updateFocusElement(newFocus->available, comm, "");
			newFocus->updateFocusElement(newFocus->selectEffect, "\n\t\t\t\t\t\tpotential_communist_puppet_target = yes", "");
		}
		if (!majorIdeologies.count("fascism"))
		{
			std::string fasc = "";
			fasc += "\n\t\t\t\t\tif = {\n";
			fasc += "\t\t\t\t\t\tlimit = { ROOT = { has_government = fascism } }\n";
			fasc += "\t\t\t\t\t\tpotential_fascist_annex_target = yes\n";
			fasc += "\t\t\t\t\t}";
			newFocus->updateFocusElement(newFocus->available, fasc, "");
			newFocus->updateFocusElement(newFocus->selectEffect, "\n\t\t\t\t\t\tpotential_fascist_annex_target = yes", "");
		}
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus border_disputes");
	}

	if (const auto& originalFocus = loadedFocuses.find("prepare_for_war"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus prepare_for_war");
	}

	if (const auto& originalFocus = loadedFocuses.find("neighbor_war"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		if (!majorIdeologies.count("communism"))
		{
			std::string comm = "";
			comm += "\n\t\t\telse_if = {\n";
			comm += "\t\t\t\tlimit = { var:neighbor_war_defender = { potential_communist_puppet_target = yes } }\n";
			comm += "\t\t\t\tdeclare_war_on = {\n";
			comm += "\t\t\t\t\ttarget = var:neighbor_war_defender\n";
			comm += "\t\t\t\t\ttype = puppet_wargoal_focus\n";
			comm += "\t\t\t\t}\n";
			comm += "\t\t\t\tadd_named_threat = { threat = 3 name = neighbor_war_NF_threat }\n";
			comm += "\t\t\t}";
			newFocus->updateFocusElement(newFocus->completionReward, comm, "");
		}
		if (!majorIdeologies.count("fascism"))
		{
			std::string fasc = "";
			fasc += "\n\t\t\telse_if = {\n";
			fasc += "\t\t\t\tlimit = { var:neighbor_war_defender = { potential_fascist_annex_target = yes } }\n";
			fasc += "\t\t\t\tdeclare_war_on = {\n";
			fasc += "\t\t\t\t\ttarget = var:neighbor_war_defender\n";
			fasc += "\t\t\t\t\ttype = annex_everything\n";
			fasc += "\t\t\t\t}\n";
			fasc += "\t\t\t\tadd_named_threat = { threat = 3 name = neighbor_war_NF_threat }\n";
			fasc += "\t\t\t}";
			newFocus->updateFocusElement(newFocus->completionReward, fasc, "");
		}
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus neighbor_war");
	}

	nextFreeColumn = static_cast<int>(numCollectovistIdeologies * 1.5) + ((numCollectovistIdeologies + 1) / 2) + 20;
}


void HoI4FocusTree::confirmLoadedFocuses()
{
	if (loadedFocuses.size() == 0)
	{
		registerKeyword("focus_tree", [this](const std::string& unused, std::istream& theStream) {});
		registerKeyword("id", commonItems::ignoreString);
		registerKeyword("country", commonItems::ignoreObject);
		registerKeyword("default", commonItems::ignoreString);
		registerKeyword("reset_on_civilwar", commonItems::ignoreString);
		registerKeyword("focus", [this](const std::string& unused, std::istream& theStream)
		{
			HoI4Focus newFocus(theStream);
			loadedFocuses.insert(make_pair(newFocus.id, newFocus));
		}
		);
		registerKeyword("shared_focus", [this](const std::string& unused, std::istream& theStream)
		{
			HoI4Focus newFocus(theStream);
			loadedFocuses.insert(make_pair(newFocus.id, newFocus));
		}
		);

		parseFile("converterFocuses.txt");
	}
}


int HoI4FocusTree::calculateNumCollectovistIdeologies(const set<string>& majorIdeologies)
{
	int numCollectovistIdeologies = 0;
	numCollectovistIdeologies += majorIdeologies.count("radical");
	numCollectovistIdeologies += majorIdeologies.count("absolutist");
	numCollectovistIdeologies += majorIdeologies.count("communism");
	numCollectovistIdeologies += majorIdeologies.count("fascism");
	return numCollectovistIdeologies;
}


void HoI4FocusTree::determineMutualExclusions(const set<string>& majorIdeologies)
{
	if (majorIdeologies.count("fascism") > 0)
	{
		communistMutualExclusions += "focus = nationalism_focus";
		absolutistMutualExlusions += "focus = nationalism_focus";
		radicalMutualExclusions += "focus = nationalism_focus";
	}
	if (majorIdeologies.count("communism") > 0)
	{
		if (fascistMutualExlusions.size() > 0)
		{
			fascistMutualExlusions += " ";
		}
		if (absolutistMutualExlusions.size() > 0)
		{
			absolutistMutualExlusions += " ";
		}
		if (radicalMutualExclusions.size() > 0)
		{
			radicalMutualExclusions += " ";
		}
		fascistMutualExlusions += "focus = internationalism_focus";
		absolutistMutualExlusions += "focus = internationalism_focus";
		radicalMutualExclusions += "focus = internationalism_focus";
	}
	if (majorIdeologies.count("absolutist") > 0)
	{
		if (fascistMutualExlusions.size() > 0)
		{
			fascistMutualExlusions += " ";
		}
		if (communistMutualExclusions.size() > 0)
		{
			communistMutualExclusions += " ";
		}
		if (radicalMutualExclusions.size() > 0)
		{
			radicalMutualExclusions += " ";
		}
		fascistMutualExlusions += "focus = absolutism_focus";
		communistMutualExclusions += "focus = absolutism_focus";
		radicalMutualExclusions += "focus = absolutism_focus";
	}
	if (majorIdeologies.count("radical") > 0)
	{
		if (fascistMutualExlusions.size() > 0)
		{
			fascistMutualExlusions += " ";
		}
		if (communistMutualExclusions.size() > 0)
		{
			communistMutualExclusions += " ";
		}
		if (absolutistMutualExlusions.size() > 0)
		{
			absolutistMutualExlusions += " ";
		}
		fascistMutualExlusions += "focus = radical_focus";
		communistMutualExclusions += "focus = radical_focus";
		absolutistMutualExlusions += "focus = radical_focus";
	}
}


void HoI4FocusTree::addFascistGenericFocuses(int relativePosition, const std::set<std::string>& majorIdeologies)
{
	if (const auto& originalFocus = loadedFocuses.find("nationalism_focus"); originalFocus != loadedFocuses.end())
	{
		shared_ptr<HoI4Focus> newFocus = make_shared<HoI4Focus>(originalFocus->second);
		newFocus->mutuallyExclusive = "= { " + fascistMutualExlusions + " }";
		newFocus->xPos = relativePosition;
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus nationalism_focus");
	}

	if (const auto& originalFocus = loadedFocuses.find("militarism"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus militarism");
	}

	if (const auto& originalFocus = loadedFocuses.find("military_youth"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		newFocus->completionReward = "= {\n";
		newFocus->completionReward += "			add_ideas = military_youth_focus\n";
		for (auto ideology : majorIdeologies)
		{
			newFocus->completionReward += "			if = {\n";
			newFocus->completionReward += "				limit = { has_government = " + ideology + " }\n";
			newFocus->completionReward += "				add_popularity = {\n";
			newFocus->completionReward += "					ideology = " + ideology + "\n";
			newFocus->completionReward += "					popularity = 0.2\n";
			newFocus->completionReward += "				}\n";
			newFocus->completionReward += "			}\n";
		}
		newFocus->completionReward += "		}";
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus military_youth");
	}

	if (const auto& originalFocus = loadedFocuses.find("paramilitarism"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus paramilitarism");
	}
}


void HoI4FocusTree::addCommunistGenericFocuses(int relativePosition)
{
	if (const auto& originalFocus = loadedFocuses.find("internationalism_focus"); originalFocus != loadedFocuses.end())
	{
		shared_ptr<HoI4Focus> newFocus = make_shared<HoI4Focus>(originalFocus->second);
		newFocus->mutuallyExclusive = "= { " + communistMutualExclusions + " }";
		newFocus->available = "= {\n";
		newFocus->available += "			OR = {\n";
		newFocus->available += "				has_government = communism\n";
		newFocus->available += "				has_government = neutrality\n";
		newFocus->available += "			}\n";
		newFocus->available += "		}";
		newFocus->xPos = relativePosition;
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus internationalism_focus");
	}

	if (const auto& originalFocus = loadedFocuses.find("political_correctness"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus political_correctness");
	}

	if (const auto& originalFocus = loadedFocuses.find("indoctrination_focus"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus indoctrination_focus");
	}

	if (const auto& originalFocus = loadedFocuses.find("political_commissars"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		newFocus->completionReward = "= {\n";
		newFocus->completionReward += "			add_ideas = political_commissars_focus\n";
		newFocus->completionReward += "			if = {\n";
		newFocus->completionReward += "				limit = { has_government = communism }\n";
		newFocus->completionReward += "				add_popularity = {\n";
		newFocus->completionReward += "					ideology = communism\n";
		newFocus->completionReward += "					popularity = 0.2\n";
		newFocus->completionReward += "				}\n";
		newFocus->completionReward += "			}\n";
		newFocus->completionReward += "			add_political_power = 200\n";
		newFocus->completionReward += "		}";
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus political_commissars");
	}
}


void HoI4FocusTree::addAbsolutistGenericFocuses(int relativePosition)
{
	if (const auto& originalFocus = loadedFocuses.find("absolutism_focus"); originalFocus != loadedFocuses.end())
	{
		shared_ptr<HoI4Focus> newFocus = make_shared<HoI4Focus>(originalFocus->second);
		newFocus->mutuallyExclusive = "= { " + absolutistMutualExlusions + " }";
		newFocus->xPos = relativePosition;
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus absolutism_focus");
	}

	if (const auto& originalFocus = loadedFocuses.find("royal_dictatorship_focus"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus royal_dictatorship_focus");
	}

	if (const auto& originalFocus = loadedFocuses.find("royal_army_tradition_focus"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus royal_army_tradition_focus");
	}

	if (const auto& originalFocus = loadedFocuses.find("historical_claims_focus"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus historical_claims_focus");
	}
}


void HoI4FocusTree::addRadicalGenericFocuses(int relativePosition)
{
	if (const auto& originalFocus = loadedFocuses.find("radical_focus"); originalFocus != loadedFocuses.end())
	{
		shared_ptr<HoI4Focus> newFocus = make_shared<HoI4Focus>(originalFocus->second);
		newFocus->mutuallyExclusive = "= { " + radicalMutualExclusions + " }";
		newFocus->xPos = relativePosition;
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus radical_focus");
	}

	if (const auto& originalFocus = loadedFocuses.find("private_channels_focus"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus private_channels_focus");
	}

	if (const auto& originalFocus = loadedFocuses.find("hardfought_market_focus"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus hardfought_market_focus");
	}

	if (const auto& originalFocus = loadedFocuses.find("army_provides_focus"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = make_shared<HoI4Focus>(originalFocus->second);
		sharedFocuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus army_provides_focus");
	}
}


std::unique_ptr<HoI4FocusTree> HoI4FocusTree::makeCustomizedCopy(const HoI4::Country& country) const
{
	HoI4FocusTree newFocusTree(country);

	for (auto focus : focuses)
	{
		auto newFocus = focus->makeCustomizedCopy(newFocusTree.dstCountryTag);
		newFocusTree.addFocus(newFocus);
	}
	newFocusTree.setNextFreeColumn(nextFreeColumn);

	return std::make_unique<HoI4FocusTree>(std::move(newFocusTree));
}


void HoI4FocusTree::addDemocracyNationalFocuses(shared_ptr<HoI4::Country> Home, vector<shared_ptr<HoI4::Country>>& CountriesToContain)
{
	double WTModifier = 1;
	if (Home->getGovernmentIdeology() == "democratic")
	{
		string warPol = Home->getRulingParty().getWarPolicy();
		if (warPol == "jingoism")
		{
			WTModifier = 0;
		}
		if (warPol == "pro_military")
		{
			WTModifier = 0.25;
		}
		if (warPol == "anti_military")
		{
			WTModifier = 0.5;
		}
		if (warPol == "pacifism" || warPol == "pacifist")
		{
			WTModifier = 0.5;
		}
	}

	if (const auto& originalFocus = loadedFocuses.find("WarProp"); originalFocus != loadedFocuses.end())
	{
		shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->available = "= {\n";
		newFocus->available += "			threat > " + to_string(0.20 * WTModifier / 1000) + "\n";
		newFocus->available += "		}";
		newFocus->xPos = nextFreeColumn + CountriesToContain.size() - 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus WarProp");
	}

	if (const auto& originalFocus = loadedFocuses.find("PrepInter"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->available = "= {\n";
		newFocus->available += "			threat > " + to_string(0.30 * WTModifier / 1000) + "\n";
		newFocus->available += "		}";
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus PrepInter");
	}

	if (const auto& originalFocus = loadedFocuses.find("Lim"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->available = "= {\n";
		newFocus->available += "			threat > " + to_string(0.50 * WTModifier / 1000) + "\n";
		newFocus->available += "		}";
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus Lim");
	}

	int relavtivePos = 1 - CountriesToContain.size();
	for (auto country : CountriesToContain)
	{
		auto possibleContainedCountryName = country->getSourceCountry().getName("english");
		string containedCountryName;
		if (possibleContainedCountryName)
		{
			containedCountryName = *possibleContainedCountryName;
		}
		else
		{
			LOG(LogLevel::Warning) << "Could not determine contained country name for democratic focuses";
			containedCountryName.clear();
		}

		if (const auto& originalFocus = loadedFocuses.find("WarPlan"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), country->getTag());
			newFocus->bypass = "= {\n";
			newFocus->bypass += "			has_war_with = " + country->getTag() + "\n";
			newFocus->bypass += "		}";
			newFocus->xPos = relavtivePos;
			newFocus->available = "= {\n";
			newFocus->available += "			any_other_country = {\n";
			newFocus->available += "				original_tag = " + country->getTag() + "\n";
			newFocus->available += "				exists = yes\n";
			newFocus->available += "				NOT = { has_government = democratic } \n";
			newFocus->available += "				NOT = { is_in_faction_with = " + Home->getTag() + " }\n";
			newFocus->available += "				OR = {\n";
			newFocus->available += "					has_offensive_war = yes\n";
			newFocus->available += "					has_added_tension_amount > 30\n";
			newFocus->available += "				}\n";
			newFocus->available += "			}\n";
			newFocus->available += "		}";
			focuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus WarPlan");
		}

		if (const auto& originalFocus = loadedFocuses.find("Embargo"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), country->getTag());
			newFocus->prerequisites.clear();
			newFocus->prerequisites.push_back("= { focus =  WarPlan" + Home->getTag() + country->getTag() + " }");
			newFocus->bypass = "= {\n";
			newFocus->bypass += "			has_war_with = " + country->getTag() + "\n";
			newFocus->bypass += "		}";
			newFocus->relativePositionId += country->getTag();
			newFocus->available = "= {\n";
			newFocus->available += "			any_other_country = {\n";
			newFocus->available += "				original_tag = " + country->getTag() + "\n";
			newFocus->available += "				exists = yes\n";
			newFocus->available += "				NOT = { has_government = democratic } \n";
			newFocus->available += "				NOT = { is_in_faction_with = " + Home->getTag() + " }\n";
			newFocus->available += "				OR = {\n";
			newFocus->available += "					has_offensive_war = yes\n";
			newFocus->available += "					has_added_tension_amount > 30\n";
			newFocus->available += "					threat > 0.6\n";
			newFocus->available += "				}\n";
			newFocus->available += "			}\n";
			newFocus->available += "		}";
			newFocus->completionReward = "= {\n";
			newFocus->completionReward += "			" + country->getTag() + " = {\n";
			newFocus->completionReward += "				add_opinion_modifier = { target = " + Home->getTag() + " modifier = embargo }\n";
			newFocus->completionReward += "			}\n";
			newFocus->completionReward += "		}";
			focuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus Embargo");
		}

		if (const auto& originalFocus = loadedFocuses.find("WAR"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), country->getTag());
			newFocus->prerequisites.clear();
			newFocus->prerequisites.push_back("= { focus =  Embargo" + Home->getTag() + country->getTag() + " }");
			newFocus->bypass = "= {\n";
			newFocus->bypass += "			has_war_with = " + country->getTag() + "\n";
			newFocus->bypass += "		}";
			newFocus->relativePositionId += country->getTag();
			newFocus->available = "= {\n";
			newFocus->available += "			has_war = no\n";
			newFocus->available += "			any_other_country = {\n";
			newFocus->available += "				original_tag = " + country->getTag() + "\n";
			newFocus->available += "				exists = yes\n";
			newFocus->available += "				NOT = { has_government = democratic } \n";
			newFocus->available += "				NOT = { is_in_faction_with = " + Home->getTag() + " }\n";
			newFocus->available += "				OR = {\n";
			newFocus->available += "					has_offensive_war = yes\n";
			newFocus->available += "					has_added_tension_amount > 30\n";
			newFocus->available += "					threat > 0.6\n";
			newFocus->available += "				}\n";
			newFocus->available += "			}\n";
			newFocus->available += "		}";
			newFocus->aiWillDo = "= {\n";
			newFocus->aiWillDo += "			factor = 10\n";
			newFocus->aiWillDo += "		}";
			newFocus->completionReward = "= {\n";
			newFocus->completionReward += "			declare_war_on = {\n";
			newFocus->completionReward += "				type = puppet_wargoal_focus\n";
			newFocus->completionReward += "				target = " + country->getTag() + "\n";
			newFocus->completionReward += "			}\n";
			newFocus->completionReward += "		}";
			focuses.push_back(newFocus);

			relavtivePos += 2;
		}
		else
		{
			throw std::runtime_error("Could not load focus WAR");
		}
	}
}


void HoI4FocusTree::addAbsolutistEmpireNationalFocuses(shared_ptr<HoI4::Country> Home, const vector<shared_ptr<HoI4::Country>>& targetColonies, const vector<shared_ptr<HoI4::Country>>& annexationTargets)
{
	auto possibleHomeCountryAdjective = Home->getSourceCountry().getAdjective("english");
	string homeCountryAdjective;
	if (possibleHomeCountryAdjective)
	{
		homeCountryAdjective = *possibleHomeCountryAdjective;
	}
	else
	{
		LOG(LogLevel::Warning) << "Could not determine home country adjective for absolutist focuses";
		homeCountryAdjective.clear();
	}

	if (const auto& originalFocus = loadedFocuses.find("EmpireGlory"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->xPos = nextFreeColumn + 5;
		newFocus->yPos = 0;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus EmpireGlory");
	}

	if (const auto& originalFocus = loadedFocuses.find("StrengthenColonies"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "EmpireGlory" + Home->getTag();
		newFocus->xPos = -1;
		newFocus->yPos = 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus StrengthenColonies");
	}

	if (const auto& originalFocus = loadedFocuses.find("StrengthenHome"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "EmpireGlory" + Home->getTag();
		newFocus->xPos = 1;
		newFocus->yPos = 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus StrengthenHome");
	}

	if (const auto& originalFocus = loadedFocuses.find("ColonialInd"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "StrengthenColonies" + Home->getTag();
		newFocus->xPos = -2;
		newFocus->yPos = 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus ColonialInd");
	}

	if (const auto& originalFocus = loadedFocuses.find("ColonialHwy"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "ColonialInd" + Home->getTag();
		newFocus->xPos = -2;
		newFocus->yPos = 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus ColonialHwy");
	}

	if (const auto& originalFocus = loadedFocuses.find("ResourceFac"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "ColonialInd" + Home->getTag();
		newFocus->xPos = 0;
		newFocus->yPos = 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus ResourceFac");
	}

	if (const auto& originalFocus = loadedFocuses.find("ColonialArmy"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "StrengthenColonies" + Home->getTag();
		newFocus->xPos = 0;
		newFocus->yPos = 1;
		newFocus->completionReward = "{\n";
		newFocus->completionReward += "		add_ideas = militarism_focus_absolutist\n";
		newFocus->completionReward += "}\n";
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus ColonialArmy");
	}

	//establish protectorate
	if (targetColonies.size() >= 1)
	{
		auto target = targetColonies.front();

		auto possibleProtectorateCountryName = target->getSourceCountry().getName("english");
		string protectorateCountryName;
		if (possibleProtectorateCountryName)
		{
			protectorateCountryName = *possibleProtectorateCountryName;
		}
		else
		{
			LOG(LogLevel::Warning) << "Could not determine protectorate country name for absolutist focuses";
			protectorateCountryName.clear();
		}

		if (const auto& originalFocus = loadedFocuses.find("Protectorate"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), target->getTag());
			newFocus->id = "Protectorate" + Home->getTag() + target->getTag();
			newFocus->available += "= {\n";
			newFocus->available += "			" + target->getTag() + " = { is_in_faction = no }\n";
			newFocus->available += "		}";
			newFocus->prerequisites.push_back("= { focus = ColonialArmy" + Home->getTag() + " }");
			newFocus->relativePositionId = "ColonialArmy" + Home->getTag();
			newFocus->xPos = 0;
			newFocus->yPos = 1;
			newFocus->bypass += "= {\n";
			newFocus->bypass += "			OR = {\n";
			newFocus->bypass += "				" + Home->getTag() + " = {\n";
			newFocus->bypass += "					is_in_faction_with = " + target->getTag() + "\n";
			newFocus->bypass += "					has_war_with = " + target->getTag() + "\n";
			newFocus->bypass += "				}\n";
			newFocus->bypass += "				NOT = { country_exists = " + target->getTag() + " }\n";
			newFocus->bypass += "			}\n";
			newFocus->bypass += "		}";
			newFocus->aiWillDo += "= {\n";
			newFocus->aiWillDo += "			factor = 10\n";
			newFocus->aiWillDo += "			modifier = {\n";
			newFocus->aiWillDo += "				factor = 0\n";
			newFocus->aiWillDo += "				strength_ratio = { tag = " + target->getTag() + " ratio < 1 }\n";
			newFocus->aiWillDo += "			}\n";
			newFocus->aiWillDo += "		}";
			newFocus->selectEffect = "= { add_ai_strategy = { type = prepare_for_war id = \"" + target->getTag() + "\" value = 200 } }";
			newFocus->completionReward += "= {\n";
			newFocus->completionReward += "			create_wargoal = {\n";
			newFocus->completionReward += "				type = annex_everything\n";
			newFocus->completionReward += "				target = " + target->getTag() + "\n";
			newFocus->completionReward += "			}\n";
			newFocus->completionReward += "		}";
			focuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus Protectorate");
		}
	}
	if (targetColonies.size() >= 2)
	{
		auto target = targetColonies.back();

		auto possibleProtectorateCountryName = target->getSourceCountry().getName("english");
		string protectorateCountryName;
		if (possibleProtectorateCountryName)
		{
			protectorateCountryName = *possibleProtectorateCountryName;
		}
		else
		{
			LOG(LogLevel::Warning) << "Could not determine protectorate country name for absolutist focuses";
			protectorateCountryName.clear();
		}

		if (const auto& originalFocus = loadedFocuses.find("Protectorate"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), target->getTag());
			newFocus->id = "Protectorate" + Home->getTag() + target->getTag();
			newFocus->available += "= {\n";
			newFocus->available += "			" + target->getTag() + " = { is_in_faction = no }\n";
			newFocus->available += "		}";
			newFocus->prerequisites.push_back("= { focus = Protectorate" + Home->getTag() + targetColonies.front()->getTag() + " }");
			newFocus->relativePositionId = "Protectorate" + Home->getTag() + targetColonies.front()->getTag();
			newFocus->xPos = 0;
			newFocus->yPos = 1;
			newFocus->bypass += "= {\n";
			newFocus->bypass += "			OR = {\n";
			newFocus->bypass += "				" + Home->getTag() + " = {\n";
			newFocus->bypass += "					is_in_faction_with = " + target->getTag() + "\n";
			newFocus->bypass += "					has_war_with = " + target->getTag() + "\n";
			newFocus->bypass += "				}\n";
			newFocus->bypass += "				NOT = { country_exists = " + target->getTag() + " }\n";
			newFocus->bypass += "			}\n";
			newFocus->bypass += "		}";
			newFocus->aiWillDo += "= {\n";
			newFocus->aiWillDo += "			factor = 5\n";
			newFocus->aiWillDo += "			modifier = {\n";
			newFocus->aiWillDo += "				factor = 0\n";
			newFocus->aiWillDo += "				strength_ratio = { tag = " + target->getTag() + " ratio < 1 }\n";
			newFocus->aiWillDo += "			}\n";
			newFocus->aiWillDo += "		}";
			newFocus->selectEffect = "= { add_ai_strategy = { type = prepare_for_war id = \"" + target->getTag() + "\" value = 200 } }";
			newFocus->completionReward += "= {\n";
			newFocus->completionReward += "			create_wargoal = {\n";
			newFocus->completionReward += "				type = annex_everything\n";
			newFocus->completionReward += "				target = " + target->getTag() + "\n";
			newFocus->completionReward += "			}\n";
			newFocus->completionReward += "		}";
			focuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus Protectorate");
		}
	}

	if (const auto& originalFocus = loadedFocuses.find("TradeEmpire"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "ColonialInd" + Home->getTag();
		newFocus->xPos = -1;
		newFocus->yPos = 2;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus TradeEmpire");
	}

	if (const auto& originalFocus = loadedFocuses.find("IndHome"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "StrengthenHome" + Home->getTag();
		newFocus->xPos = 1;
		newFocus->yPos = 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus IndHome");
	}

	if (const auto& originalFocus = loadedFocuses.find("NationalHwy"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "IndHome" + Home->getTag();
		newFocus->xPos = -1;
		newFocus->yPos = 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus NationalHwy");
	}

	if (const auto& originalFocus = loadedFocuses.find("NatCollege"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "IndHome" + Home->getTag();
		newFocus->xPos = 1;
		newFocus->yPos = 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus NatCollege");
	}

	if (const auto& originalFocus = loadedFocuses.find("MilitaryBuildup"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "IndHome" + Home->getTag();
		newFocus->xPos = 2;
		newFocus->yPos = 2;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus MilitaryBuildup");
	}

	if (const auto& originalFocus = loadedFocuses.find("PrepTheBorder"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "StrengthenHome" + Home->getTag();
		newFocus->xPos = 4;
		newFocus->yPos = 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus PrepTheBorder");
	}

	if (const auto& originalFocus = loadedFocuses.find("NatSpirit"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "PrepTheBorder" + Home->getTag();
		newFocus->xPos = 0;
		newFocus->yPos = 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus NatSpirit");
	}

	//ANNEX
	if (annexationTargets.size() >= 1)
	{
		auto target = annexationTargets.front();

		auto possibleTargetCountryName = target->getSourceCountry().getName("english");
		string targetCountryName;
		if (possibleTargetCountryName)
		{
			targetCountryName = *possibleTargetCountryName;
		}
		else
		{
			LOG(LogLevel::Warning) << "Could not determine target country name for absolutist focuses";
			targetCountryName.clear();
		}

		if (const auto& originalFocus = loadedFocuses.find("Annex"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), target->getTag());
			newFocus->id = "Annex" + Home->getTag() + target->getTag();
			newFocus->available += "= {\n";
			newFocus->available += "			" + target->getTag() + " = { is_in_faction = no }\n";
			newFocus->available += "		}";
			newFocus->prerequisites.push_back("= { focus = PrepTheBorder" + Home->getTag() + " }");
			newFocus->relativePositionId = "PrepTheBorder" + Home->getTag();
			newFocus->xPos = 2;
			newFocus->yPos = 1;
			newFocus->bypass += "= {\n";
			newFocus->bypass += "			OR = {\n";
			newFocus->bypass += "				" + Home->getTag() + "= {\n";
			newFocus->bypass += "					is_in_faction_with = " + target->getTag() + "\n";
			newFocus->bypass += "					has_war_with = " + target->getTag() + "\n";
			newFocus->bypass += "				}\n";
			newFocus->bypass += "				NOT = { country_exists = " + target->getTag() + " }\n";
			newFocus->bypass += "			}\n";
			newFocus->bypass += "		}";
			newFocus->aiWillDo += "= {\n";
			newFocus->aiWillDo += "			factor = 5\n";
			newFocus->aiWillDo += "			modifier = {\n";
			newFocus->aiWillDo += "				factor = 0\n";
			newFocus->aiWillDo += "				strength_ratio = { tag = " + target->getTag() + " ratio < 1 }\n";
			newFocus->aiWillDo += "			}\n";
			newFocus->aiWillDo += "		}";
			newFocus->selectEffect = "= { add_ai_strategy = { type = prepare_for_war id = \"" + target->getTag() + "\" value = 200 } }";
			newFocus->completionReward += "= {\n";
			newFocus->completionReward += "			create_wargoal = {\n";
			newFocus->completionReward += "				type = annex_everything\n";
			newFocus->completionReward += "				target = " + target->getTag() + "\n";
			newFocus->completionReward += "			}\n";
			newFocus->completionReward += "		}";
			focuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus Annex");
		}
	}
	if (annexationTargets.size() >= 2)
	{
		auto target = annexationTargets.back();

		auto possibleTargetCountryName = target->getSourceCountry().getName("english");
		string targetCountryName;
		if (possibleTargetCountryName)
		{
			targetCountryName = *possibleTargetCountryName;
		}
		else
		{
			LOG(LogLevel::Warning) << "Could not determine target country name for absolutist focuses";
			targetCountryName.clear();
		}

		if (const auto& originalFocus = loadedFocuses.find("Annex"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), target->getTag());
			newFocus->id = "Annex" + Home->getTag() + target->getTag();
			newFocus->available += "= {\n";
			newFocus->available += "			\"" + target->getTag() + "\" = { is_in_faction = no }\n";
			newFocus->available += "		}";
			newFocus->prerequisites.push_back("= { focus = NatSpirit" + Home->getTag() + " }");
			newFocus->relativePositionId = "NatSpirit" + Home->getTag();
			newFocus->xPos = 1;
			newFocus->yPos = 1;
			newFocus->bypass += "= {\n";
			newFocus->bypass += "			OR = {\n";
			newFocus->bypass += "				" + Home->getTag() + " = {\n";
			newFocus->bypass += "					is_in_faction_with = " + target->getTag() + "\n";
			newFocus->bypass += "					has_war_with = " + target->getTag() + "\n";
			newFocus->bypass += "				}\n";
			newFocus->bypass += "				NOT = { country_exists = " + target->getTag() + " }\n";
			newFocus->bypass += "			}\n";
			newFocus->bypass += "		}";
			newFocus->aiWillDo += "= {\n";
			newFocus->aiWillDo += "			factor = 5\n";
			newFocus->aiWillDo += "			modifier = {\n";
			newFocus->aiWillDo += "				factor = 0\n";
			newFocus->aiWillDo += "				strength_ratio = { tag = " + target->getTag() + " ratio < 1 }\n";
			newFocus->aiWillDo += "			}\n";
			newFocus->aiWillDo += "		}";
			newFocus->selectEffect = "= { add_ai_strategy = { type = prepare_for_war id = \"" + target->getTag() + "\" value = 200 } }";
			newFocus->completionReward += "= {\n";
			newFocus->completionReward += "			create_wargoal = {\n";
			newFocus->completionReward += "				type = annex_everything\n";
			newFocus->completionReward += "				target = " + target->getTag() + "\n";
			newFocus->completionReward += "			}\n";
			newFocus->completionReward += "		}";
			focuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus Annex");
		}
	}
}

void HoI4FocusTree::addCommunistCoupBranch(shared_ptr<HoI4::Country> Home, const vector<shared_ptr<HoI4::Country>>& coupTargets, const std::set<std::string>& majorIdeologies)
{
	if (coupTargets.size() > 0)
	{
		if (const auto& originalFocus = loadedFocuses.find("Home_of_Revolution"); originalFocus != loadedFocuses.end())
		{
			shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
			newFocus->xPos = nextFreeColumn + coupTargets.size() - 1;
			newFocus->yPos = 0;
			focuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus Home_of_Revolution");
		}

		for (unsigned int i = 0; i < 2; i++)
		{
			if (i < coupTargets.size())
			{
				auto possibleCoupCountryName = coupTargets[i]->getSourceCountry().getName("english");
				string coupCountryName;
				if (possibleCoupCountryName)
				{
					coupCountryName = *possibleCoupCountryName;
				}
				else
				{
					LOG(LogLevel::Warning) << "Could not determine coup country name for communist coup focuses";
					coupCountryName.clear();
				}

				if (const auto& originalFocus = loadedFocuses.find("Influence_"); originalFocus != loadedFocuses.end())
				{
					auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), coupTargets[i]->getTag());
					newFocus->id = "Influence_" + coupTargets[i]->getTag() + "_" + Home->getTag();
					newFocus->xPos = nextFreeColumn + i * 2;
					newFocus->yPos = 1;
					newFocus->completionReward += "= {\n";
					newFocus->completionReward += "			" + coupTargets[i]->getTag() + " = {\n";
					if (majorIdeologies.count("fascism") > 0)
					{
						newFocus->completionReward += "				if = {\n";
						newFocus->completionReward += "					limit = {\n";
						newFocus->completionReward += "						" + Home->getTag() + " = {\n";
						newFocus->completionReward += "							has_government = fascism\n";
						newFocus->completionReward += "						}\n";
						newFocus->completionReward += "					}\n";
						newFocus->completionReward += "					add_ideas = fascist_influence\n";
						newFocus->completionReward += "				}\n";
					}
					if (majorIdeologies.count("communism") > 0)
					{
						newFocus->completionReward += "				if = {\n";
						newFocus->completionReward += "					limit = {\n";
						newFocus->completionReward += "						" + Home->getTag() + " = {\n";
						newFocus->completionReward += "							has_government = communism\n";
						newFocus->completionReward += "						}\n";
						newFocus->completionReward += "					}\n";
						newFocus->completionReward += "					add_ideas = communist_influence\n";
						newFocus->completionReward += "				}\n";
					}
					if (majorIdeologies.count("democratic") > 0)
					{
						newFocus->completionReward += "				if = {\n";
						newFocus->completionReward += "					limit = {\n";
						newFocus->completionReward += "						" + Home->getTag() + " = {\n";
						newFocus->completionReward += "							has_government = democratic\n";
						newFocus->completionReward += "						}\n";
						newFocus->completionReward += "					}\n";
						newFocus->completionReward += "					add_ideas = democratic_influence\n";
						newFocus->completionReward += "				}\n";
					}
					if (majorIdeologies.count("absolutist") > 0)
					{
						newFocus->completionReward += "				if = {\n";
						newFocus->completionReward += "					limit = {\n";
						newFocus->completionReward += "						" + Home->getTag() + " = {\n";
						newFocus->completionReward += "							has_government = absolutist\n";
						newFocus->completionReward += "						}\n";
						newFocus->completionReward += "					}\n";
						newFocus->completionReward += "					add_ideas = absolutist_influence\n";
						newFocus->completionReward += "				}\n";
					}
					if (majorIdeologies.count("radical") > 0)
					{
						newFocus->completionReward += "				if = {\n";
						newFocus->completionReward += "					limit = {\n";
						newFocus->completionReward += "						" + Home->getTag() + " = {\n";
						newFocus->completionReward += "							has_government = radical\n";
						newFocus->completionReward += "						}\n";
						newFocus->completionReward += "					}\n";
						newFocus->completionReward += "					add_ideas = radical_influence\n";
						newFocus->completionReward += "				}\n";
					}
					newFocus->completionReward += "				country_event = { id = generic.1 }\n";
					newFocus->completionReward += "			}\n";
					newFocus->completionReward += "		}";
					focuses.push_back(newFocus);
				}
				else
				{
					throw std::runtime_error("Could not load focus Influence_");
				}

				if (const auto& originalFocus = loadedFocuses.find("Coup_"); originalFocus != loadedFocuses.end())
				{
					auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), coupTargets[i]->getTag());
					newFocus->id = "Coup_" + coupTargets[i]->getTag() + "_" + Home->getTag();
					newFocus->prerequisites.push_back("= { focus = Influence_" + coupTargets[i]->getTag() + "_" + Home->getTag() + " }");
					newFocus->relativePositionId = "Influence_" + coupTargets[i]->getTag() + "_" + Home->getTag();
					newFocus->xPos = 0;
					newFocus->yPos = 1;
					newFocus->available = "= {\n";
					newFocus->available += "			" + coupTargets[i]->getTag() + " = { communism > 0.5 }\n";
					newFocus->available += "		}";
					newFocus->completionReward += "= {\n";
					newFocus->completionReward += "			" + coupTargets[i]->getTag() + " = {\n";
					newFocus->completionReward += "				start_civil_war = {\n";
					newFocus->completionReward += "					ideology = communism\n";
					newFocus->completionReward += "					size = 0.5\n";
					newFocus->completionReward += "				}\n";
					newFocus->completionReward += "			}\n";
					newFocus->completionReward += "		}";
					focuses.push_back(newFocus);
				}
				else
				{
					throw std::runtime_error("Could not load focus Coup_");
				}
			}
		}
		nextFreeColumn += 2 * coupTargets.size();
	}
	return;
}

void HoI4FocusTree::addCommunistWarBranch(shared_ptr<HoI4::Country> Home, const vector<shared_ptr<HoI4::Country>>& warTargets, HoI4::Events* events)
{
	if (warTargets.size() > 0)
	{

		if (const auto& originalFocus = loadedFocuses.find("StrengthCom"); originalFocus != loadedFocuses.end())
		{
			shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
			newFocus->xPos = nextFreeColumn + warTargets.size() - 1;
			newFocus->yPos = 0;
			focuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus StrengthCom");
		}

		if (const auto& originalFocus = loadedFocuses.find("Inter_Com_Pres"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
			newFocus->relativePositionId = "StrengthCom" + Home->getTag();
			newFocus->xPos = 0;
			newFocus->yPos = 1;
			newFocus->completionReward += "= {\n";
			newFocus->completionReward += "			add_named_threat = { threat = 2 name = " + newFocus->text + " }\n";
			newFocus->completionReward += "			add_political_power = 150\n";
			newFocus->completionReward += "		}";
			//FIXME
			//maybe add some claims?
			focuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus Inter_Com_Pres");
		}

		for (unsigned int i = 0; i < 3; i++)
		{
			if (i < warTargets.size())
			{
				auto possibleWarTargetCountryName = warTargets[i]->getSourceCountry().getName("english");
				string warTargetCountryName;
				if (possibleWarTargetCountryName)
				{
					warTargetCountryName = *possibleWarTargetCountryName;
				}
				else
				{
					LOG(LogLevel::Warning) << "Could not determine war target country name for communist war focuses";
					warTargetCountryName.clear();
				}

				int v1 = rand() % 12 + 1;
				int v2 = rand() % 12 + 1;
				if (const auto& originalFocus = loadedFocuses.find("War"); originalFocus != loadedFocuses.end())
				{
					auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), warTargets[i]->getTag());
					newFocus->id = "War" + warTargets[i]->getTag() + Home->getTag();
					newFocus->available = "= {\n";
					newFocus->available += "			date > 1938." + to_string(v1) + "." + to_string(v2) + "\n";
					newFocus->available += "		}";
					newFocus->xPos = nextFreeColumn + i * 2;
					newFocus->yPos = 2;
					newFocus->bypass = "= {\n";
					newFocus->bypass += "					has_war_with = " + warTargets[i]->getTag() + "\n";
					newFocus->bypass += "				}";
					newFocus->aiWillDo = "= {\n";
					newFocus->aiWillDo += "			factor = 5\n";
					newFocus->aiWillDo += "			modifier = {\n";
					newFocus->aiWillDo += "				factor = 0\n";
					newFocus->aiWillDo += "				strength_ratio = { tag = " + warTargets[i]->getTag() + " ratio < 1 }\n";
					newFocus->aiWillDo += "			}";
					if (warTargets.size() > 1)
					{
						newFocus->aiWillDo += "\n";
						newFocus->aiWillDo += "			modifier = {\n";
						newFocus->aiWillDo += "				factor = 0\n";
						newFocus->aiWillDo += "				OR = {\n";
						for (unsigned int i2 = 0; i2 < warTargets.size(); i2++)
						{
							if (i != i2)
								newFocus->aiWillDo += "					has_war_with = " + warTargets[i]->getTag() + "\n";
						}
						newFocus->aiWillDo += "				}\n";
						newFocus->aiWillDo += "			}";
					}
					newFocus->aiWillDo += "\n";
					newFocus->aiWillDo += "		}";

					newFocus->completionReward += "= {\n";
					newFocus->completionReward += "			add_named_threat = { threat = 3 name = \"War with " + warTargetCountryName + "\" }\n";
					newFocus->completionReward += "			create_wargoal = {\n";
					newFocus->completionReward += "				type = puppet_wargoal_focus\n";
					newFocus->completionReward += "				target = " + warTargets[i]->getTag() + "\n";
					newFocus->completionReward += "			}\n";
					newFocus->completionReward += "		}";
					focuses.push_back(newFocus);
				}
				else
				{
					throw std::runtime_error("Could not load focus War");
				}
			}
		}
		nextFreeColumn += warTargets.size() * 2;
	}
}

void HoI4FocusTree::addFascistAnnexationBranch(shared_ptr<HoI4::Country> Home, const vector<shared_ptr<HoI4::Country>>& annexationTargets, HoI4::Events* events)
{
	//The Following 'if' statement prevents converter from generating focuses if annexationTargets.size > 1
	//Keep this 'if' statement off until we figure out how to handle Fascist NF's
	//if (annexationTargets.size() >= 1)
	//{
	if (const auto& originalFocus = loadedFocuses.find("The_third_way"); originalFocus != loadedFocuses.end())
	{
		shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		if (annexationTargets.size() >= 1)
		{
			newFocus->xPos = nextFreeColumn + annexationTargets.size() - 1;
		}

		//'else' statement is there in case annexationTargets.size() is <1.  Need to fix in the future.
		else
		{
			newFocus->xPos = nextFreeColumn;
		}
		newFocus->yPos = 0;
		//FIXME
		//Need to get Drift Defense to work
		//in modified generic focus? (tk)
		//newFocus->completionReward += "			drift_defence_factor = 0.5\n";
		newFocus->completionReward += "= {\n";
		newFocus->completionReward += "			add_named_threat = { threat = 2 name = " + newFocus->text + " }\n";
		newFocus->completionReward += "			add_ideas = fascist_influence\n";
		newFocus->completionReward += "		}";
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus The_third_way");
	}

	if (const auto& originalFocus = loadedFocuses.find("mil_march"); originalFocus != loadedFocuses.end())
	{
		auto newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		newFocus->relativePositionId = "The_third_way" + Home->getTag();
		newFocus->xPos = 0;
		newFocus->yPos = 1;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus mil_march");
	}

	for (unsigned int i = 0; i < annexationTargets.size(); i++)
	{
		auto possibleAnnexationTargetCountryName = annexationTargets[i]->getSourceCountry().getName("english");
		string annexationTargetCountryName;
		if (possibleAnnexationTargetCountryName)
		{
			annexationTargetCountryName = *possibleAnnexationTargetCountryName;
		}
		else
		{
			LOG(LogLevel::Warning) << "Could not determine annexation target country name for fascist annexation focuses";
			annexationTargetCountryName.clear();
		}

		//int x = i * 3;
		//for random date
		int v1 = rand() % 5 + 1;
		int v2 = rand() % 5 + 1;

		if (const auto& originalFocus = loadedFocuses.find("_anschluss_"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), annexationTargets[i]->getTag());
			newFocus->id = Home->getTag() + "_anschluss_" + annexationTargets[i]->getTag();
			newFocus->available += "= {\n";
			newFocus->available += "			" + annexationTargets[i]->getTag() + " = {\n";
			newFocus->available += "				is_in_faction = no\n";
			newFocus->available += "			}\n";
			newFocus->available += "			is_puppet = no\n";
			newFocus->available += "			date > 1937." + to_string(v1 + 5) + "." + to_string(v2 + 5) + "\n";
			newFocus->available += "		}";
			newFocus->xPos = nextFreeColumn + i * 2;
			newFocus->yPos = 2;
			newFocus->completionReward += "= {\n";
			newFocus->completionReward += "			add_named_threat = { threat = 2 name = \"Union with " + annexationTargetCountryName + "\" }\n";
			newFocus->completionReward += "			army_experience = 10\n";
			newFocus->completionReward += "			if = {\n";
			newFocus->completionReward += "				limit = {\n";
			newFocus->completionReward += "					country_exists = " + annexationTargets[i]->getTag() + "\n";
			newFocus->completionReward += "				}\n";
			newFocus->completionReward += "				" + annexationTargets[i]->getTag() + " = {\n";
			newFocus->completionReward += "					country_event = NFEvents." + to_string(events->getCurrentNationFocusEventNum()) + "\n";
			newFocus->completionReward += "				}\n";
			newFocus->completionReward += "			}\n";
			newFocus->completionReward += "		}";
			focuses.push_back(newFocus);

			events->createAnnexEvent(*Home, *annexationTargets[i]);
		}
		else
		{
			throw std::runtime_error("Could not load focus _anschluss_");
		}
	}
	if (annexationTargets.size() >= 1)
	{
		nextFreeColumn += annexationTargets.size() * 2;
	}
	else
	{
		nextFreeColumn += 2;
	}
	//}
}

void HoI4FocusTree::addFascistSudetenBranch(shared_ptr<HoI4::Country> Home, const vector<shared_ptr<HoI4::Country>>& sudetenTargets, const vector<vector<int>>& demandedStates, const HoI4::World* world)
{
	HoI4::Events* events = world->getEvents();

	//if it can easily take these targets as they are not in an alliance, you can get annexation event

	if (const auto& originalFocus = loadedFocuses.find("expand_the_reich"); originalFocus != loadedFocuses.end())
	{
		shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
		if (sudetenTargets.size() == 1 || sudetenTargets.size() >= 2)
		{
			//if there are anschlusses, make this event require at least 1 anschluss, else, its the start of a tree
			for (unsigned int i = 0; i < 2; i++)
			{
				if (i < sudetenTargets.size())
				{
					//					newFocus->prerequisites.push_back("= { focus = " + Home->getTag() + "_anschluss_" + sudetenTargets[i]->getTag() + " }");
				}
			}
			newFocus->xPos = nextFreeColumn + sudetenTargets.size() - 1;
		}

		//'else' statement is there in case sudetenTargets.size() is <1.  Need to fix in the future.
		else
		{
			newFocus->xPos = nextFreeColumn;
		}
		newFocus->yPos = 0;
		newFocus->completionReward += "= {\n";
		newFocus->completionReward += "			add_named_threat = { threat = 3 name = " + newFocus->text + " }\n";//give some claims or cores
		newFocus->completionReward += "			add_political_power = 150\n";
		newFocus->completionReward += "		}";
		addFocus(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus expand_the_reich");
	}

	for (unsigned int i = 0; i < sudetenTargets.size(); i++)
	{
		auto possibleSudetenTargetCountryName = sudetenTargets[i]->getSourceCountry().getName("english");
		string sudetenTargetCountryName;
		if (possibleSudetenTargetCountryName)
		{
			sudetenTargetCountryName = *possibleSudetenTargetCountryName;
		}
		else
		{
			LOG(LogLevel::Warning) << "Could not determine annexation target country name for fascist sudeten focuses";
			sudetenTargetCountryName.clear();
		}

		int v1 = rand() % 8 + 1;
		int v2 = rand() % 8 + 1;

		if (const auto& originalFocus = loadedFocuses.find("_sudeten_"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), sudetenTargets[i]->getTag());
			newFocus->id = Home->getTag() + "_sudeten_" + sudetenTargets[i]->getTag();
			newFocus->available = "= {\n";
			newFocus->available += "			is_puppet = no\n";
			newFocus->available += "			date > 1938." + to_string(v1) + "." + to_string(v2) + "\n";
			newFocus->available += "		}";
			newFocus->xPos = nextFreeColumn + 2 * i;
			newFocus->yPos = 1;
			newFocus->bypass += "= {\n";
			newFocus->bypass += "  has_war_with = " + sudetenTargets[i]->getTag() + "\n";
			newFocus->bypass += "}";
			newFocus->completionReward += "= {\n";
			newFocus->completionReward += "			add_named_threat = { threat = 2 name = \"Demand Territory from " + sudetenTargetCountryName + "\" }\n";
			newFocus->completionReward += "			army_experience = 10\n";
			newFocus->completionReward += "			if = {\n";
			newFocus->completionReward += "				limit = {\n";
			newFocus->completionReward += "					country_exists = " + sudetenTargets[i]->getTag() + "\n";
			newFocus->completionReward += "				}\n";
			newFocus->completionReward += "				" + sudetenTargets[i]->getTag() + " = {\n";
			newFocus->completionReward += "					country_event = NFEvents." + to_string(events->getCurrentNationFocusEventNum()) + "\n";
			newFocus->completionReward += "				}\n";
			newFocus->completionReward += "			}\n";
			newFocus->completionReward += "		}";
			addFocus(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus _sudeten_");
		}

		//FINISH HIM
		if (const auto& originalFocus = loadedFocuses.find("_finish_"); originalFocus != loadedFocuses.end())
		{
			auto newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), sudetenTargets[i]->getTag());
			newFocus->id = Home->getTag() + "_finish_" + sudetenTargets[i]->getTag();
			newFocus->available = "= {\n";
			newFocus->available += "	" + sudetenTargets[i]->getTag() + " = { is_in_faction = no }\n";
			newFocus->available += "		}";
			newFocus->prerequisites.push_back("= { focus =  " + Home->getTag() + "_sudeten_" + sudetenTargets[i]->getTag() + " }");
			newFocus->xPos = nextFreeColumn + 2 * i;
			newFocus->yPos = 2;
			newFocus->bypass += "= {\n";
			newFocus->bypass += "		has_war_with = " + sudetenTargets[i]->getTag() + "\n";
			newFocus->bypass += "	}";
			newFocus->selectEffect = "= { add_ai_strategy = { type = prepare_for_war id = \"" + sudetenTargets[i]->getTag() + "\" value = 200 } }";
			newFocus->completionReward += "= {\n";
			newFocus->completionReward += "			add_named_threat = { threat = 3 name = \"Fate of " + sudetenTargetCountryName + "\" }\n";
			newFocus->completionReward += "			create_wargoal = {\n";
			newFocus->completionReward += "				type = annex_everything\n";
			newFocus->completionReward += "				target = " + sudetenTargets[i]->getTag() + "\n";
			newFocus->completionReward += "			}\n";
			newFocus->completionReward += "		}";
			addFocus(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus _finish_");
		}

		//events
		events->createSudetenEvent(*Home, *sudetenTargets[i], demandedStates[i]);
	}
	nextFreeColumn += 2 * sudetenTargets.size();
}


void HoI4FocusTree::addGPWarBranch(shared_ptr<HoI4::Country> Home, const vector<shared_ptr<HoI4::Country>>& newAllies, const vector<shared_ptr<HoI4::Country>>& GCTargets, const string& ideology, HoI4::Events* events)
{
	int numAllies = newAllies.size();
	string ideologyShort = ideology.substr(0, 3);
	if (newAllies.size() > 0)
	{
		if (const auto& originalFocus = loadedFocuses.find("_Summit"); originalFocus != loadedFocuses.end())
		{
			shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
			newFocus->id = ideologyShort + "_Summit" + Home->getTag();
			newFocus->text = ideology + "_Summit";
			if (numAllies == 0)
			{
				newFocus->xPos = nextFreeColumn + newAllies.size();
			}
			else
			{
				newFocus->xPos = nextFreeColumn + newAllies.size() - 1;
			}
			newFocus->yPos = 0;
			newFocus->completionReward += "= {\n";
			newFocus->completionReward += "			add_named_threat = { threat = 3 name = \"Call for the " + ideology + " Summit\" }\n";
			newFocus->completionReward += "			add_political_power = 150\n";
			newFocus->completionReward += "		}";
			focuses.push_back(newFocus);
			HoI4Localisation::copyFocusLocalisations("_Summit", newFocus->text);
			HoI4Localisation::updateLocalisationText(newFocus->text, "$TARGET", ideology);
			HoI4Localisation::updateLocalisationText(newFocus->text + "_desc", "$TARGET", ideology);
		}
		else
		{
			throw std::runtime_error("Could not load focus _Summit");
		}
	}

	unsigned int i = 0;
	for (auto newAlly : newAllies)
	{
		auto possibleAllyCountryName = newAlly->getSourceCountry().getName("english");
		string allyCountryName;
		if (possibleAllyCountryName)
		{
			allyCountryName = *possibleAllyCountryName;
		}
		else
		{
			LOG(LogLevel::Warning) << "Could not determine ally country name for GP alliance focuses";
			allyCountryName.clear();
		}
		
		if (const auto& originalFocus = loadedFocuses.find("Alliance_"); originalFocus != loadedFocuses.end())
		{
			shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(Home->getTag(), newAlly->getTag());
			newFocus->id = "Alliance_" + newAlly->getTag() + Home->getTag();
			newFocus->prerequisites.push_back("= { focus = " + ideologyShort + "_Summit" + Home->getTag() + " }");
			newFocus->relativePositionId = ideologyShort + "_Summit" + Home->getTag();
			if (numAllies == 1)
			{
				newFocus->xPos = 0;
			}
			else
			{
				newFocus->xPos = (i * 2) - 1;
			}
			newFocus->yPos = 1;
			newFocus->bypass += "= {\n";
			newFocus->bypass += "			OR = {\n";
			newFocus->bypass += "				" + Home->getTag() + " = { is_in_faction_with = " + newAlly->getTag() + "}\n";
			newFocus->bypass += "				has_war_with = " + newAlly->getTag() + "\n";
			newFocus->bypass += "				NOT = { country_exists = " + newAlly->getTag() + " }\n";
			newFocus->bypass += "			}\n";
			newFocus->bypass += "		}";
			newFocus->completionReward += "= {\n";
			newFocus->completionReward += "			" + newAlly->getTag() + " = {\n";
			newFocus->completionReward += "				country_event = { hours = 6 id = NFEvents." + to_string(events->getCurrentNationFocusEventNum()) + " }\n";
			newFocus->completionReward += "				add_opinion_modifier = { target = " + Home->getTag() + " modifier = positive_50 }\n";
			newFocus->completionReward += "			}\n";
			newFocus->completionReward += "		}";
			focuses.push_back(newFocus);

			events->createFactionEvents(*Home, *newAlly);
			i++;
		}
		else
		{
			throw std::runtime_error("Could not load focus Alliance_");
		}
	}

	i = 0;
	for (auto GC : GCTargets)
	{
		auto possibleWarTargetCountryName = GC->getSourceCountry().getName("english");
		string warTargetCountryName;
		if (possibleWarTargetCountryName)
		{
			warTargetCountryName = *possibleWarTargetCountryName;
		}
		else
		{
			LOG(LogLevel::Warning) << "Could not determine war target country name for GP war focuses";
			warTargetCountryName.clear();
		}

		//figuring out location of WG
		if (const auto& originalFocus = loadedFocuses.find("GP_War"); originalFocus != loadedFocuses.end())
		{
			shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeCustomizedCopy(Home->getTag());
			if (newAllies.size() > 0)
			{
				for (unsigned int i2 = 0; i2 < newAllies.size(); i2++)
				{
					newFocus->prerequisites.push_back("= { focus = Alliance_" + newAllies[i2]->getTag() + Home->getTag() + " }");
				}
			}
			int v1 = rand() % 12 + 1;
			int v2 = rand() % 12 + 1;
			newFocus->id = "GP_War" + GC->getTag() + Home->getTag();
			newFocus->text += GC->getTag();
			newFocus->available = "= {\n";
			newFocus->available += "			has_war = no\n";
			newFocus->available += "			date > 1939." + to_string(v1) + "." + to_string(v2) + "\n";
			newFocus->available += "		}";
			if (newAllies.size() > 0)
			{
				newFocus->relativePositionId = ideologyShort + "_Summit" + Home->getTag();
				newFocus->xPos = 0;
				newFocus->yPos = 2;
			}
			else
			{
				newFocus->xPos = nextFreeColumn;
				newFocus->yPos = 0;
			}
			newFocus->bypass = "= {\n";
			newFocus->bypass += "		   has_war_with = " + GC->getTag() + "\n";
			newFocus->bypass += "		}";
			newFocus->aiWillDo = "= {\n";
			newFocus->aiWillDo += "			factor = " + to_string(10 - GCTargets.size() * 5) + "\n";
			newFocus->aiWillDo += "			modifier = {\n";
			newFocus->aiWillDo += "					factor = 0\n";
			newFocus->aiWillDo += "					strength_ratio = { tag = " + GC->getTag() + " ratio < 1 }\n";
			newFocus->aiWillDo += "			}";
			if (GCTargets.size() > 1)
			{
				newFocus->aiWillDo = "\n";

				//make ai have this as a 0 modifier if they are at war
				newFocus->aiWillDo += "			modifier = {\n";
				newFocus->aiWillDo += "				factor = 0\n";
				newFocus->aiWillDo += "				OR = {\n";
				for (unsigned int i2 = 0; i2 < GCTargets.size(); i2++)
				{
					if (GC != GCTargets[i2])
					{
						newFocus->aiWillDo += "					has_war_with = " + GCTargets[i2]->getTag() + "\n";
					}
				}
				newFocus->aiWillDo += "				}\n";
				newFocus->aiWillDo += "			}";
			}
			newFocus->aiWillDo += "\n";
			newFocus->aiWillDo += "		}";
			newFocus->completionReward += "= {\n";
			newFocus->completionReward += "			add_named_threat = { threat = 5 name = \"War with " + warTargetCountryName + "\" }\n";
			newFocus->completionReward += "			declare_war_on = {\n";
			newFocus->completionReward += "				type = puppet_wargoal_focus\n";
			newFocus->completionReward += "				target = " + GC->getTag() + "\n";
			newFocus->completionReward += "			}\n";
			newFocus->completionReward += "		}";
			focuses.push_back(newFocus);
			HoI4Localisation::copyFocusLocalisations("GPWar", newFocus->text);
			HoI4Localisation::updateLocalisationText(newFocus->text, "$TARGET", GC->getTag());
			HoI4Localisation::updateLocalisationText(newFocus->text + "_desc", "$TARGET", GC->getTag());
			i++;
		}
		else
		{
			throw std::runtime_error("Could not load focus GP_War");
		}
	}
	nextFreeColumn += 2 * max(newAllies.size(), GCTargets.size());
}


std::map<std::string, int> HoI4FocusTree::addCoresBranch(
	std::shared_ptr<HoI4::Country> theCountry,
	int& numWarsWithNeighbors,
	const std::set<std::string>& majorIdeologies
){
	std::map<std::string, int> coreHolders;
	int sumUnownedCores = 0;

	for (auto coreState: theCountry->getCoreStates())
	{
		if (auto owner = coreState.second.getOwner(); owner != theCountry->getTag())
		{
			std::set<std::string> allies;
			if (auto faction = theCountry->getFaction(); faction)
			{
				allies = faction->getLeader()->getAllies();
				allies.insert(faction->getLeader()->getTag());
			}
			auto puppets = theCountry->getPuppets();
			auto puppetMaster = theCountry->getPuppetMaster();
			if (allies.find(owner) == allies.end() && puppets.find(owner) == puppets.end() && owner != puppetMaster)
			{
				int numProvinces = std::min(static_cast<int>(coreState.second.getProvinces().size()), 10);
				sumUnownedCores += numProvinces;
				if (auto itr = coreHolders.find(owner); itr == coreHolders.end())
				{
					coreHolders.insert(make_pair(owner, numProvinces));
				}
				else
				{
					itr->second += numProvinces;
				}
			}
		}
	}

	numWarsWithNeighbors = std::min(static_cast<int>(coreHolders.size()), 4);

	if (!coreHolders.empty())
	{
		std::string fascistGovernmentCheck = "";
		fascistGovernmentCheck += "\n\t\t\tmodifier = {\n";
		fascistGovernmentCheck += "\t\t\t\tfactor = 5\n";
		fascistGovernmentCheck += "\t\t\t\thas_government = fascism\n";
		fascistGovernmentCheck += "\t\t\t}";

		if (const auto& originalFocus = loadedFocuses.find("reclaim_cores"); originalFocus != loadedFocuses.end())
		{
			shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeCustomizedCopy(theCountry->getTag());
			newFocus->selectEffect = "= {\n";
			for (auto country: coreHolders)
			{
				newFocus->selectEffect += "\t\t\tset_variable = { unowned_cores_@" + country.first + " = " + std::to_string(country.second) + " }\n";
			}
			newFocus->selectEffect += "\t\t\tset_variable = { revanchism = " + std::to_string(0.00001*sumUnownedCores) + " }\n";
			newFocus->selectEffect += "\t\t\tset_variable = { revanchism_stab = " + std::to_string(-0.000001*sumUnownedCores) + " }\n";
			newFocus->selectEffect += "\t\t\tadd_dynamic_modifier = { modifier = revanchism }\n";
			if (majorIdeologies.count("fascism"))
			{
				newFocus->selectEffect += "\t\t\tadd_dynamic_modifier = { modifier = revanchism_fasc }\n";
			}
			newFocus->selectEffect += "\t\t}\n";
			newFocus->xPos = nextFreeColumn + coreHolders.size() - 1;
			focuses.push_back(newFocus);
		}
		else
		{
			throw std::runtime_error("Could not load focus reclaim_cores");
		}

		for (auto country: coreHolders)
		{
			if (const auto& originalFocus = loadedFocuses.find("raise_matter"); originalFocus != loadedFocuses.end())
			{
				shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(theCountry->getTag(), country.first);
				newFocus->xPos = nextFreeColumn;
				if (!majorIdeologies.count("fascism"))
				{
					newFocus->completionReward = "= {\n";
					newFocus->completionReward += "\t\t\tadd_stability = 0.0001\n";
					newFocus->completionReward += "\t\t\tadd_political_power = 150\n";
					newFocus->completionReward += "\t\t\tadd_timed_idea = { idea = generic_military_industry days = 180 }\n";
					newFocus->completionReward += "\t\t}";

					std::string fascismPopularityCheck = "";
					fascismPopularityCheck += "\n\t\t\tmodifier = {\n";
					fascismPopularityCheck += "\t\t\t\tfactor = 0\n";
					fascismPopularityCheck += "\t\t\t\tNOT = { has_government = fascism }\n";
					fascismPopularityCheck += "\t\t\t\tNOT = { fascism > 0.35 }\n";
					fascismPopularityCheck += "\t\t\t}";
					newFocus->updateFocusElement(newFocus->aiWillDo, fascismPopularityCheck, "");
					newFocus->updateFocusElement(newFocus->aiWillDo, fascistGovernmentCheck, "");
				}
				newFocus->updateFocusElement(newFocus->available, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->completionReward, "$POPULARITY", std::to_string(0.000001*country.second));
				newFocus->updateFocusElement(newFocus->bypass, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->aiWillDo, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->aiWillDo, "$REVANCHISM", std::to_string(std::max(static_cast<int>(0.1*country.second), 1)));
				focuses.push_back(newFocus);
				nextFreeColumn += 2;
			}
			else
			{
				throw std::runtime_error("Could not load focus raise_matter");
			}

			if (const auto& originalFocus = loadedFocuses.find("build_public_support"); originalFocus != loadedFocuses.end())
			{
				shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(theCountry->getTag(), country.first);
				newFocus->prerequisites.clear();
				newFocus->prerequisites.push_back("= { focus = raise_matter" + theCountry->getTag() + country.first + " }");
				newFocus->relativePositionId += country.first;
				if (!majorIdeologies.count("fascism"))
				{
					newFocus->completionReward = " = {\n";
					newFocus->completionReward += "\t\t\tadd_stability = 0.0001\n";
					newFocus->completionReward += "\t\t\tadd_war_support = $WARSUPPORT\n";
					newFocus->completionReward += "\t\t\tadd_timed_idea = { idea = generic_rapid_mobilization days = 180 }\n";
					newFocus->completionReward += "\t\t}";

					std::string fascismPopularityCheck = "";
					fascismPopularityCheck += "\n\t\t\tmodifier = {\n";
					fascismPopularityCheck += "\t\t\t\tfactor = 0\n";
					fascismPopularityCheck += "\t\t\t\tNOT = { has_government = fascism }\n";
					fascismPopularityCheck += "\t\t\t\tNOT = { fascism > 0.4 }\n";
					fascismPopularityCheck += "\t\t\t}";
					newFocus->updateFocusElement(newFocus->aiWillDo, fascismPopularityCheck, "");
					newFocus->updateFocusElement(newFocus->aiWillDo, fascistGovernmentCheck, "");
				}
				newFocus->updateFocusElement(newFocus->available, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->completionReward, "$POPULARITY", std::to_string(0.000001*country.second));
				newFocus->updateFocusElement(newFocus->completionReward, "$WARSUPPORT", std::to_string(0.00001*country.second));
				newFocus->updateFocusElement(newFocus->bypass, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->aiWillDo, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->aiWillDo, "$TAG", theCountry->getTag());
				newFocus->updateFocusElement(newFocus->aiWillDo, "$REVANCHISM", std::to_string(std::max(static_cast<int>(0.1*country.second), 1)));
				focuses.push_back(newFocus);
			}
			else
			{
				throw std::runtime_error("Could not load focus build_public_support");
			}

			if (const auto& originalFocus = loadedFocuses.find("territory_or_war"); originalFocus != loadedFocuses.end())
			{
				shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(theCountry->getTag(), country.first);
				newFocus->prerequisites.clear();
				newFocus->prerequisites.push_back("= { focus = build_public_support" + theCountry->getTag() + country.first + " }");
				newFocus->relativePositionId += country.first;
				if (!majorIdeologies.count("fascism"))
				{
					newFocus->updateFocusElement(newFocus->aiWillDo, fascistGovernmentCheck, "");
				}
				newFocus->updateFocusElement(newFocus->available, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->completionReward, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->bypass, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->aiWillDo, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->aiWillDo, "$TAG", theCountry->getTag());
				newFocus->updateFocusElement(newFocus->aiWillDo, "$REVANCHISM", std::to_string(std::max(static_cast<int>(0.1*country.second), 1)));
				focuses.push_back(newFocus);
			}
			else
			{
				throw std::runtime_error("Could not load focus territory_or_war");
			}

			if (const auto& originalFocus = loadedFocuses.find("war_plan"); originalFocus != loadedFocuses.end())
			{
				shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(theCountry->getTag(), country.first);
				newFocus->prerequisites.clear();
				newFocus->prerequisites.push_back("= { focus = territory_or_war" + theCountry->getTag() + country.first + " }");
				newFocus->relativePositionId += country.first;
				if (!majorIdeologies.count("fascism"))
				{
					newFocus->updateFocusElement(newFocus->aiWillDo, fascistGovernmentCheck, "");
				}
				newFocus->updateFocusElement(newFocus->available, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->completionReward, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->bypass, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->aiWillDo, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->aiWillDo, "$TAG", theCountry->getTag());
				newFocus->updateFocusElement(newFocus->aiWillDo, "$REVANCHISM", std::to_string(std::max(static_cast<int>(0.1*country.second), 1)));
				focuses.push_back(newFocus);
			}
			else
			{
				throw std::runtime_error("Could not load focus war_plan");
			}

			if (const auto& originalFocus = loadedFocuses.find("declare_war"); originalFocus != loadedFocuses.end())
			{
				shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(theCountry->getTag(), country.first);
				newFocus->prerequisites.clear();
				newFocus->prerequisites.push_back("= { focus = war_plan" + theCountry->getTag() + country.first + " }");
				newFocus->relativePositionId += country.first;
				if (!majorIdeologies.count("fascism"))
				{
					newFocus->updateFocusElement(newFocus->aiWillDo, fascistGovernmentCheck, "");
				}
				newFocus->updateFocusElement(newFocus->available, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->completionReward, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->bypass, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->aiWillDo, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->aiWillDo, "$TAG", theCountry->getTag());
				newFocus->updateFocusElement(newFocus->aiWillDo, "$REVANCHISM", std::to_string(std::max(static_cast<int>(country.second), 1)));
				focuses.push_back(newFocus);
			}
			else
			{
				throw std::runtime_error("Could not load focus declare_war");
			}

			if (const auto& originalFocus = loadedFocuses.find("cleanup_revanchism"); originalFocus != loadedFocuses.end())
			{
				shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(theCountry->getTag(), country.first);
				newFocus->prerequisites.clear();
				newFocus->prerequisites.push_back("= { focus = declare_war" + theCountry->getTag() + country.first + " }");
				newFocus->relativePositionId += country.first;
				newFocus->updateFocusElement(newFocus->available, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->completionReward, "$TARGET", country.first);
				newFocus->updateFocusElement(newFocus->completionReward, "$REVANCHISM", std::to_string(0.000005*country.second));
				newFocus->updateFocusElement(newFocus->completionReward, "$STABILITY", std::to_string(0.0000005*country.second));
				focuses.push_back(newFocus);
			}
			else
			{
				throw std::runtime_error("Could not load focus cleanup_revanchism");
			}
		}
	}

	return coreHolders;
}


std::set<std::string> HoI4FocusTree::addConquerBranch(
	std::shared_ptr<HoI4::Country> theCountry,
	int& numWarsWithNeighbors,
	const std::set<std::string>& majorIdeologies,
	std::map<std::string, int> coreHolders
)
{
	std::string tag = theCountry->getTag();
	std::vector<std::pair<std::string, int>> conquerQue;
	std::set<std::string> conquerTags;

	int maxConquerValue = 0;
	for (auto strategy: theCountry->getConquerStrategies())
	{
		if (strategy.second.getValue() > maxConquerValue)
		{
			maxConquerValue = strategy.second.getValue();
		}
	}
	std::vector<std::pair<std::string, int>> sortedConquerStrategies;
	for (int i = maxConquerValue; i > 0; i--)
	{
		for (auto strategy: theCountry->getConquerStrategies())
		{
			if (strategy.second.getValue() == i)
			{
				sortedConquerStrategies.push_back(make_pair(strategy.first, strategy.second.getValue()));
			}
		}
	}

	for (auto target: sortedConquerStrategies)
	{
		if (coreHolders.find(target.first) != coreHolders.end())
		{
			continue;
		}

		std::set<std::string> allies;
		if (auto faction = theCountry->getFaction(); faction)
		{
			allies = faction->getLeader()->getAllies();
			allies.insert(faction->getLeader()->getTag());
		}
		auto puppets = theCountry->getPuppets();
		auto puppetMaster = theCountry->getPuppetMaster();
		if (allies.find(target.first) == allies.end() && puppets.find(target.first) == puppets.end() && target.first != puppetMaster)
		{
			conquerQue.push_back(make_pair(target.first, target.second));
		}
	}

	if (!conquerQue.empty())
	{
		for (auto target: conquerQue)
		{
			int aiChance = 10 * target.second / maxConquerValue;
			if (numWarsWithNeighbors >= 5 || !aiChance)
			{
				break;
			}

			conquerTags.insert(target.first);
			numWarsWithNeighbors++;
			auto relations = theCountry->getRelations(target.first);
			date startDate = date("1936.01.01");
			startDate.increaseByMonths(std::max(0, relations->getRelations() / -4 + 12));

			if (const auto& originalFocus = loadedFocuses.find("border_disputes"); originalFocus != loadedFocuses.end())
			{
				shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(tag, target.first);
				newFocus->relativePositionId = "";
				newFocus->available = "= {\n";
				newFocus->available += "\t\t\thas_war = no\n";
				newFocus->available += "\t\t\tnot_in_border_conflict = yes\n";
				newFocus->available += "\t\t\tany_neighbor_country = {\n";
				newFocus->available += "\t\t\t\texists = yes\n";
				newFocus->available += "\t\t\t\tnot_allies_or_subjects = yes\n";
				newFocus->available += "\t\t\t\tin_weak_faction = yes\n";
				newFocus->available += "\t\t\t\tOR = {\n";
				newFocus->available += "\t\t\t\t\tpotential_take_core_target = yes\n";
				if (majorIdeologies.count("communism"))
				{
					newFocus->available += "\t\t\t\t\tif = {\n";
					newFocus->available += "\t\t\t\t\t\tlimit = { ROOT = { has_government = communism } }\n";
					newFocus->available += "\t\t\t\t\t\tpotential_communist_puppet_target = yes\n";
					newFocus->available += "\t\t\t\t\t}\n";
				}
				if (majorIdeologies.count("fascism"))
				{
					newFocus->available += "\t\t\t\t\tif = {\n";
					newFocus->available += "\t\t\t\t\t\tlimit = { ROOT = { has_government = fascism } }\n";
					newFocus->available += "\t\t\t\t\t\tpotential_fascist_annex_target = yes\n";
					newFocus->available += "\t\t\t\t\t}\n";
				}
				newFocus->available += "\t\t\t\t\tpotential_take_state_target = yes\n";
				newFocus->available += "\t\t\t\t\tpotential_border_war_target = yes\n";
				newFocus->available += "\t\t\t\t}\n";
				newFocus->available += "\t\t\t}\n";
				newFocus->available += "\t\t}\n";
				newFocus->updateFocusElement(newFocus->available, "any_neighbor_country", target.first);
				newFocus->selectEffect  = "";
				newFocus->xPos = nextFreeColumn;
				newFocus->yPos = 0;
				newFocus->aiWillDo = "= {\n";
				newFocus->aiWillDo += "\t\t\tfactor = " + to_string(aiChance) + "\n";
				newFocus->aiWillDo += "\t\t\tmodifier = {\n";
				newFocus->aiWillDo += "\t\t\t\tfactor = 0\n";
				newFocus->aiWillDo += "\t\t\t\thas_opinion = { target = " + target.first + " value > 0 }\n";
				newFocus->aiWillDo += "\t\t\t}\n";
				newFocus->aiWillDo += "\t\t\tmodifier = {\n";
				newFocus->aiWillDo += "\t\t\t\tfactor = 0\n";
				newFocus->aiWillDo += "\t\t\t\t" + target.first + " = { in_weak_faction = no }\n";
				newFocus->aiWillDo += "\t\t\t}\n";
				newFocus->aiWillDo += "\t\t\tmodifier = {\n";
				newFocus->aiWillDo += "\t\t\t\tfactor = 0.1\n";
				newFocus->aiWillDo += "\t\t\t\tthreat > 0.25\n";
				newFocus->aiWillDo += "\t\t\t}\n";
				newFocus->aiWillDo += "\t\t\tmodifier = {\n";
				newFocus->aiWillDo += "\t\t\t\tfactor = 2\n";
				newFocus->aiWillDo += "\t\t\t\t" + target.first + " = { surrender_progress > 0.2 }\n";
				newFocus->aiWillDo += "\t\t\t}\n";
				newFocus->aiWillDo += "\t\t}";
				focuses.push_back(newFocus);
			}
			else
			{
				throw std::runtime_error("Could not load focus border_disputes");
			}

			if (const auto& originalFocus = loadedFocuses.find("prepare_for_war"); originalFocus != loadedFocuses.end())
			{
				shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(tag, target.first);
				newFocus->prerequisites.clear();
				newFocus->prerequisites.push_back("= { focus = border_disputes" + tag + target.first + " }");
				newFocus->relativePositionId += target.first;
				newFocus->available = "= {\n";
				newFocus->available += "\t\t\tdate > " + startDate.toString() + "\n";
				newFocus->available += "\t\t\thas_war = no\n";
				newFocus->available += "\t\t\tnot_in_border_conflict = yes\n";
				newFocus->available += "\t\t\t" + target.first + " = {\n";
				newFocus->available += "\t\t\t\texists = yes\n";
				newFocus->available += "\t\t\t\tnot_in_border_conflict = yes\n";
				newFocus->available += "\t\t\t}\n";
				newFocus->available += "\t\t}";
				newFocus->aiWillDo = "= {\n";
				newFocus->aiWillDo += "\t\t\tfactor = 10\n";
				newFocus->aiWillDo += "\t\t\tmodifier = {\n";
				newFocus->aiWillDo += "\t\t\t\tfactor = 0\n";
				newFocus->aiWillDo += "\t\t\t\thas_opinion = { target = " + target.first + " value > 0 }\n";
				newFocus->aiWillDo += "\t\t\t}\n";
				newFocus->aiWillDo += "\t\t\tmodifier = {\n";
				newFocus->aiWillDo += "\t\t\t\tfactor = 0.1\n";
				newFocus->aiWillDo += "\t\t\t\tthreat > 0.25\n";
				newFocus->aiWillDo += "\t\t\t}\n";
				newFocus->aiWillDo += "\t\t\tmodifier = {\n";
				newFocus->aiWillDo += "\t\t\t\tfactor = 0\n";
				newFocus->aiWillDo += "\t\t\t\t" + target.first + " = { in_weak_faction = no }\n";
				newFocus->aiWillDo += "\t\t\t}\n";
				newFocus->aiWillDo += "\t\t\tmodifier = {\n";
				newFocus->aiWillDo += "\t\t\t\tfactor = 2\n";
				newFocus->aiWillDo += "\t\t\t\t" + target.first + " = { surrender_progress > 0.2 }\n";
				newFocus->aiWillDo += "\t\t\t}\n";
				newFocus->aiWillDo += "\t\t}";
				newFocus->updateFocusElement(newFocus->selectEffect, "var:ROOT.neighbor_war_defender", target.first);
				newFocus->updateFocusElement(newFocus->completionReward, "var:neighbor_war_defender", target.first);
				newFocus->updateFocusElement(newFocus->bypass, "var:neighbor_war_defender", target.first);
				focuses.push_back(newFocus);
			}
			else
			{
				throw std::runtime_error("Could not load focus prepare_for_war");
			}

			if (const auto& originalFocus = loadedFocuses.find("neighbor_war"); originalFocus != loadedFocuses.end())
			{
				shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(tag, target.first);
				newFocus->prerequisites.clear();
				newFocus->prerequisites.push_back("= { focus = prepare_for_war" + tag + target.first + " }");
				newFocus->relativePositionId += target.first;
				newFocus->updateFocusElement(newFocus->available, "var:neighbor_war_defender", target.first);
				newFocus->updateFocusElement(newFocus->available, "var:ROOT.neighbor_war_defender", target.first);
				if (!majorIdeologies.count("communism"))
				{
					std::string comm = "";
					comm += "\n\t\t\telse_if = {\n";
					comm += "\t\t\t\tlimit = { var:neighbor_war_defender = { potential_communist_puppet_target = yes } }\n";
					comm += "\t\t\t\tdeclare_war_on = {\n";
					comm += "\t\t\t\t\ttarget = var:neighbor_war_defender\n";
					comm += "\t\t\t\t\ttype = puppet_wargoal_focus\n";
					comm += "\t\t\t\t}\n";
					comm += "\t\t\t\tadd_named_threat = { threat = 3 name = neighbor_war_NF_threat }\n";
					comm += "\t\t\t}";
					newFocus->updateFocusElement(newFocus->completionReward, comm, "");
				}
				if (!majorIdeologies.count("fascism"))
				{
					std::string fasc = "";
					fasc += "\n\t\t\telse_if = {\n";
					fasc += "\t\t\t\tlimit = { var:neighbor_war_defender = { potential_fascist_annex_target = yes } }\n";
					fasc += "\t\t\t\tdeclare_war_on = {\n";
					fasc += "\t\t\t\t\ttarget = var:neighbor_war_defender\n";
					fasc += "\t\t\t\t\ttype = annex_everything\n";
					fasc += "\t\t\t\t}\n";
					fasc += "\t\t\t\tadd_named_threat = { threat = 3 name = neighbor_war_NF_threat }\n";
					fasc += "\t\t\t}";
					newFocus->updateFocusElement(newFocus->completionReward, fasc, "");
				}
				newFocus->updateFocusElement(newFocus->completionReward, "var:neighbor_war_defender", target.first);
				newFocus->updateFocusElement(newFocus->completionReward, "var:ROOT.neighbor_war_defender", target.first);
				newFocus->updateFocusElement(newFocus->bypass, "var:neighbor_war_defender", target.first);
				focuses.push_back(newFocus);
			}
			else
			{
				throw std::runtime_error("Could not load focus neighbor_war");
			}

			nextFreeColumn += 2;
		}
	}

	return conquerTags;
}

void HoI4FocusTree::addNeighborWarBranch(
	const string& tag,
	const map<string, shared_ptr<HoI4::Country>>& closeNeighbors,
	const shared_ptr<HoI4::Country>& targetNeighbors,
	const string& targetName,
	const date& startDate,
	int numWarsWithNeighbors,
	const std::set<std::string>& majorIdeologies
)
{
	if (const auto& originalFocus = loadedFocuses.find("border_disputes"); originalFocus != loadedFocuses.end())
	{
		shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(tag, targetNeighbors->getTag());
		newFocus->relativePositionId = "";
		newFocus->available = "= {\n";
		newFocus->available += "\t\t\thas_war = no\n";
		newFocus->available += "\t\t\tnot_in_border_conflict = yes\n";
		newFocus->available += "\t\t\thas_opinion = { target = " + targetNeighbors->getTag() + " value < 0 }\n";
		newFocus->available += "\t\t\tany_neighbor_country = {\n";
		newFocus->available += "\t\t\t\texists = yes\n";
		newFocus->available += "\t\t\t\tnot_allies_or_subjects = yes\n";
		newFocus->available += "\t\t\t\tin_weak_faction = yes\n";
		newFocus->available += "\t\t\t\tOR = {\n";
		newFocus->available += "\t\t\t\t\tpotential_take_core_target = yes\n";
		if (majorIdeologies.count("communism"))
		{
			newFocus->available += "\t\t\t\t\tif = {\n";
			newFocus->available += "\t\t\t\t\t\tlimit = { ROOT = { has_government = communism } }\n";
			newFocus->available += "\t\t\t\t\t\tpotential_communist_puppet_target = yes\n";
			newFocus->available += "\t\t\t\t\t}\n";
		}
		if (majorIdeologies.count("fascism"))
		{
			newFocus->available += "\t\t\t\t\tif = {\n";
			newFocus->available += "\t\t\t\t\t\tlimit = { ROOT = { has_government = fascism } }\n";
			newFocus->available += "\t\t\t\t\t\tpotential_fascist_annex_target = yes\n";
			newFocus->available += "\t\t\t\t\t}\n";
		}
		newFocus->available += "\t\t\t\t\tpotential_take_state_target = yes\n";
		newFocus->available += "\t\t\t\t\tpotential_border_war_target = yes\n";
		newFocus->available += "\t\t\t\t}\n";
		newFocus->available += "\t\t\t}\n";
		newFocus->available += "\t\t}\n";
		newFocus->updateFocusElement(newFocus->available, "any_neighbor_country", targetNeighbors->getTag());
		newFocus->selectEffect  = "";
		newFocus->xPos = nextFreeColumn;
		newFocus->yPos = 0;
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus border_disputes");
	}

	if (const auto& originalFocus = loadedFocuses.find("prepare_for_war"); originalFocus != loadedFocuses.end())
	{
		shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(tag, targetNeighbors->getTag());
		newFocus->prerequisites.clear();
		newFocus->prerequisites.push_back("= { focus = border_disputes" + tag + targetNeighbors->getTag() + " }");
		newFocus->relativePositionId += targetNeighbors->getTag();
		newFocus->available = "= {\n";
		newFocus->available += "\t\t\thas_war = no\n";
		newFocus->available += "\t\t\tnot_in_border_conflict = yes\n";
		newFocus->available += "\t\t\thas_opinion = { target = " + targetNeighbors->getTag() + " value < 0 }\n";
		newFocus->available += "\t\t\t" + targetNeighbors->getTag() + " = {\n";
		newFocus->available += "\t\t\t\texists = yes\n";
		newFocus->available += "\t\t\t\tnot_in_border_conflict = yes\n";
		newFocus->available += "\t\t\t}\n";
		newFocus->available += "\t\t}";
		newFocus->updateFocusElement(newFocus->selectEffect, "var:ROOT.neighbor_war_defender", targetNeighbors->getTag());
		newFocus->updateFocusElement(newFocus->completionReward, "var:neighbor_war_defender", targetNeighbors->getTag());
		newFocus->updateFocusElement(newFocus->bypass, "var:neighbor_war_defender", targetNeighbors->getTag());
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus prepare_for_war");
	}

	if (const auto& originalFocus = loadedFocuses.find("neighbor_war"); originalFocus != loadedFocuses.end())
	{
		shared_ptr<HoI4Focus> newFocus = originalFocus->second.makeTargetedCopy(tag, targetNeighbors->getTag());
		newFocus->prerequisites.clear();
		newFocus->prerequisites.push_back("= { focus = prepare_for_war" + tag + targetNeighbors->getTag() + " }");
		newFocus->relativePositionId += targetNeighbors->getTag();
		newFocus->updateFocusElement(newFocus->available, "var:neighbor_war_defender", targetNeighbors->getTag());
		newFocus->updateFocusElement(newFocus->available, "var:ROOT.neighbor_war_defender", targetNeighbors->getTag());
		if (!majorIdeologies.count("communism"))
		{
			std::string comm = "";
			comm += "\n\t\t\telse_if = {\n";
			comm += "\t\t\t\tlimit = { var:neighbor_war_defender = { potential_communist_puppet_target = yes } }\n";
			comm += "\t\t\t\tdeclare_war_on = {\n";
			comm += "\t\t\t\t\ttarget = var:neighbor_war_defender\n";
			comm += "\t\t\t\t\ttype = puppet_wargoal_focus\n";
			comm += "\t\t\t\t}\n";
			comm += "\t\t\t\tadd_named_threat = { threat = 3 name = neighbor_war_NF_threat }\n";
			comm += "\t\t\t}";
			newFocus->updateFocusElement(newFocus->completionReward, comm, "");
		}
		if (!majorIdeologies.count("fascism"))
		{
			std::string fasc = "";
			fasc += "\n\t\t\telse_if = {\n";
			fasc += "\t\t\t\tlimit = { var:neighbor_war_defender = { potential_fascist_annex_target = yes } }\n";
			fasc += "\t\t\t\tdeclare_war_on = {\n";
			fasc += "\t\t\t\t\ttarget = var:neighbor_war_defender\n";
			fasc += "\t\t\t\t\ttype = annex_everything\n";
			fasc += "\t\t\t\t}\n";
			fasc += "\t\t\t\tadd_named_threat = { threat = 3 name = neighbor_war_NF_threat }\n";
			fasc += "\t\t\t}";
			newFocus->updateFocusElement(newFocus->completionReward, fasc, "");
		}
		newFocus->updateFocusElement(newFocus->completionReward, "var:neighbor_war_defender", targetNeighbors->getTag());
		newFocus->updateFocusElement(newFocus->completionReward, "var:ROOT.neighbor_war_defender", targetNeighbors->getTag());
		newFocus->updateFocusElement(newFocus->bypass, "var:neighbor_war_defender", targetNeighbors->getTag());
		focuses.push_back(newFocus);
	}
	else
	{
		throw std::runtime_error("Could not load focus neighbor_war");
	}

	nextFreeColumn += 2;
}

void HoI4FocusTree::removeFocus(const string& id)
{
	focuses.erase(
		std::remove_if(
			focuses.begin(),
			focuses.end(),
			[id](std::shared_ptr<HoI4Focus> focus) {return focus->id == id; }
		),
		focuses.end()
	);
}


void HoI4FocusTree::output(const string& filename) const
{
	ofstream out(filename);
	if (!out.is_open())
	{
		LOG(LogLevel::Error) << "Could not create " << filename;
		exit(-1);
	}

	out << "focus_tree = {\n";
	if (!dstCountryTag.empty())
	{
		out << "	id = " << dstCountryTag + "_focus\n";
		out << "	\n";
		out << "	country = {\n";
		out << "		factor = 0\n";
		out << "		\n";
		out << "		modifier = {\n";
		out << "			add = 10\n";
		out << "			tag = " << dstCountryTag << "\n";
		out << "		}\n";
		out << "	}\n";
		out << "	\n";
		out << "	default = no\n";
		out << "	\n";
		out << "	shared_focus = army_effort\n";
		out << "	shared_focus = aviation_effort\n";
		out << "	shared_focus = naval_effort\n";
		out << "	shared_focus = industrial_effort\n";
		out << "	shared_focus = political_effort\n";
		out << "\n";
	}

	for (auto focus : focuses)
	{
		out << *focus;
		out << "\n";
	}

	out << "}";

	out.close();
}

void outputShared(std::ostream& output, const HoI4Focus& focus)
{
	output << "shared_focus = {\n";
	output << "	id = " << focus.id << "\n";
	output << "	icon = " << focus.icon << "\n";
	if (!focus.text.empty())
	{
		output << "	text = " << focus.text << "\n";
	}
	for (auto prerequisite: focus.prerequisites)
	{
		output << "	prerequisite " << prerequisite << "\n";
	}
	if (!focus.mutuallyExclusive.empty())
	{
		output << "	mutually_exclusive " << focus.mutuallyExclusive << "\n";
	}
	if (!focus.bypass.empty())
	{
		output << "	bypass " << focus.bypass << "\n";
	}
	output << "	x = " << focus.xPos << "\n";
	output << "	y = " << focus.yPos << "\n";
	if (!focus.relativePositionId.empty())
	{
		output << "	relative_position_id = " << focus.relativePositionId << "\n";
	}
	output << "	cost = " << focus.cost << "\n";
	if (focus.availableIfCapitulated)
	{
		output << "	available_if_capitulated = yes\n";
	}
	if (!focus.available.empty())
	{
		output << "	available " << focus.available << "\n";
	}
	if (!focus.cancelIfInvalid.empty())
	{
		output << "	cancel_if_invalid = " << focus.cancelIfInvalid << "\n";
	}
	if (!focus.continueIfInvalid.empty())
	{
		output << "	continue_if_invalid = " << focus.continueIfInvalid << "\n";
	}
	if (!focus.selectEffect.empty())
	{
		output << "	select_effect " << focus.selectEffect << "\n";
	}
	if (!focus.completeTooltip.empty())
	{
		output << "	complete_tooltip " << focus.completeTooltip << "\n";
	}
	output << "	completion_reward " << focus.completionReward << "\n";
	if (!focus.aiWillDo.empty())
	{
		output << "	ai_will_do " << focus.aiWillDo << "\n";
	}

	output << "}\n";
	output << "\n";

	//return output;
}

void HoI4FocusTree::generateSharedFocuses(const string& filename) const
{
	ofstream SharedFoci(filename);
	if (!SharedFoci.is_open())
	{
		LOG(LogLevel::Error) << "Could not create " << filename;
		exit(-1);
	}

	for (auto focus : sharedFocuses)
	{
		outputShared(SharedFoci, *focus);
	}

	SharedFoci.close();
}
