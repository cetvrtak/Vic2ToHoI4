#include "WorldWar.h"
#include "src/HOI4World/HoI4Country.h"
#include <ranges>
using namespace std;

HoI4::WorldWar::WorldWar(const std::string& playerTag_,
	 const std::string& archenemyTag_,
	 const std::map<std::string, std::shared_ptr<HoI4::Country>>& countries_):
	 playerTag(std::move(playerTag_)), archenemyTag(std::move(archenemyTag_)), countries(std::move(countries_))
{
	CreateBlocks();
	// 	Calculate block strenghts && weaknesses
	// 	Rebalance blocks
	// 	Build frontline(s)
	// 	Leaders determine how & where to expand
	// 	Declare war on other block
}

void HoI4::WorldWar::CreateBlocks()
{
	Log(LogLevel::Info) << "\tCreating blocks";
	AddToBlock(playerTag, playerTag);
	AddToBlock(archenemyTag, archenemyTag);

	// 	Other countries choose side || stay neutral
	for (const auto& [tag, country]: countries)
	{
		if (const auto& master = country->getPuppetMaster(); master && blocks.contains(master->getTag()))
		{
			AddToBlock(tag, master->getTag());
			continue;
		}

		if (auto faction = country->getFaction();
			 faction && faction->getLeader() && blocks.contains(faction->getLeader()->getTag()))
		{
			AddToBlock(tag, faction->getLeader()->getTag());
			continue;
		}

		AddGuaranteedToBlock(tag);
		AddFriendlyToBlock(country);
	}
}

void HoI4::WorldWar::AddGuaranteedToBlock(const std::string& tag)
{
	// Check all blocks since a country can have multiple guarantees
	for (const auto& leaderTag: blocks | std::views::keys)
	{
		const auto& relations = countries.at(leaderTag)->getRelations();
		const auto& relationsItr = relations.find(tag);
		if (relationsItr == relations.end())
		{
			continue;
		}

		if (relationsItr->second.getGuarantee())
		{
			AddToBlock(tag, leaderTag);
		}
	}
}

void HoI4::WorldWar::AddFriendlyToBlock(const std::shared_ptr<HoI4::Country>& country)
{
	const auto& tag = country->getTag();
	const auto& relations = country->getRelations();
	const auto& playerRelationsItr = relations.find(playerTag);
	if (playerRelationsItr == relations.end())
	{
		return;
	}
	int playerOpinion = playerRelationsItr->second.getRelations();

	const auto& archenemyRelationsItr = relations.find(archenemyTag);
	if (archenemyRelationsItr == relations.end())
	{
		return;
	}
	int archenemyOpinion = archenemyRelationsItr->second.getRelations();

	if (playerOpinion - archenemyOpinion > 100)
	{
		AddToBlock(tag, playerTag);
		return;
	}
	if (playerOpinion - archenemyOpinion < -100)
	{
		AddToBlock(tag, archenemyTag);
	}
}