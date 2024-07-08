#include "WorldWar.h"
#include "src/HOI4World/HoI4Country.h"
using namespace std;

HoI4::WorldWar::WorldWar(const std::string& playerTag_,
	 const std::string& archenemyTag_,
	 const std::map<std::string, std::shared_ptr<HoI4::Country>>& countries_):
	 playerTag(std::move(playerTag_)), archenemyTag(std::move(archenemyTag_)), countries(std::move(countries_))
{
	// Create blocks
	createBlocks();
	// 	Calculate block strenghts && weaknesses
	// 	Rebalance blocks
	// 	Build frontline(s)
	// 	Leaders determine how & where to expand
	// 	Declare war on other block
}

void HoI4::WorldWar::createBlocks()
{
	Log(LogLevel::Info) << "\tCreating blocks";
	const auto& player = countries.at(playerTag);
	const auto& archenemy = countries.at(archenemyTag);

	// 	Other countries choose side || stay neutral
	for (const auto& [tag, country]: countries)
	{
		const auto& master = country->getPuppetMaster();
		if (master && master == player)
		{
			const auto& masterTag = master->getTag();
			blocks[playerTag].insert(tag);
			continue;
		}
		if (master && master == archenemy)
		{
			const auto& masterTag = master->getTag();
			blocks[archenemyTag].insert(tag);
			continue;
		}

		auto faction = country->getFaction();
		if (faction && faction->getLeader() == player)
		{
			blocks[playerTag].insert(tag);
			continue;
		}
		if (faction && faction->getLeader() == archenemy)
		{
			blocks[archenemyTag].insert(tag);
			continue;
		}

		const auto& relations = country->getRelations();
		const auto& playerRelationsItr = relations.find(playerTag);
		if (playerRelationsItr == relations.end())
		{
			continue;
		}
		int playerOpinion = playerRelationsItr->second.getRelations();

		const auto& archenemyRelationsItr = relations.find(archenemyTag);
		if (archenemyRelationsItr == relations.end())
		{
			continue;
		}
		int archenemyOpinion = archenemyRelationsItr->second.getRelations();

		if (playerOpinion - archenemyOpinion > 100)
		{
			blocks[playerTag].insert(tag);
			continue;
		}
		if (playerOpinion - archenemyOpinion < -100)
		{
			blocks[archenemyTag].insert(tag);
		}
	}
}