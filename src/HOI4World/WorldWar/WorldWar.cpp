#include "WorldWar.h"
#include "src/HOI4World/HoI4Country.h"
#include <queue>
using namespace std;

HoI4::WorldWar::WorldWar(const std::string& playerTag_,
	 const std::string& archenemyTag_,
	 const std::map<std::string, std::shared_ptr<HoI4::Country>>& countries_,
	 const HoI4::MapUtils& mapUtils_):
	 playerTag(std::move(playerTag_)), archenemyTag(std::move(archenemyTag_)), countries(std::move(countries_)),
	 mapUtils(std::move(mapUtils_))
{
	// Create blocks
	createBlocks();
	// 	Calculate block strenghts && weaknesses
	// 	Rebalance blocks
	// 	Build front(s)
	createFront();

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

std::optional<std::string> HoI4::WorldWar::getBlock(const std::string& tag)
{
	if (blocks.contains(tag))
	{
		return tag;
	}

	const auto& blockItr =
		 std::find_if(blocks.begin(), blocks.end(), [tag](const std::pair<std::string, std::set<std::string>>& block) {
			 return block.second.contains(tag);
		 });

	if (blockItr != blocks.end())
	{
		return blockItr->first;
	}

	return std::nullopt;
}

void HoI4::WorldWar::addToFront(const std::string& tag)
{
	const auto& block = getBlock(tag);
	if (block)
	{
		front[*block].insert(tag);
	}
}

void HoI4::WorldWar::createFront()
{
	Log(LogLevel::Info) << "\tCreating front";
	for (const auto& [leader, members]: blocks)
	{
		const auto& oppositeBlock = std::find_if(blocks.begin(),
			 blocks.end(),
			 [leader](const std::pair<std::string, std::set<std::string>>& block) {
				 return block.first != leader;
			 })->first;

		for (const auto& member: members)
		{
			const auto& path = findShortestPathToTarget(member, oppositeBlock);
			for (auto itr = path.begin(); itr != path.end(); ++itr)
			{
				const auto& tag = *itr;
				const auto& neighbor = *std::next(itr);
				if (getBlock(neighbor) != getBlock(tag))
				{
					addToFront(tag);
					break;
				}
			}
		}
	}
}

std::vector<std::string> HoI4::WorldWar::findShortestPathToTarget(const std::string& source, const std::string& target)
{
	std::queue<std::vector<std::string>> paths;
	std::unordered_set<std::string> visited;
	paths.push({source});
	visited.insert(source);

	while (!paths.empty())
	{
		std::vector<std::string> currentPath = paths.front();
		paths.pop();
		std::string currentTag = currentPath.back();

		if (countries.find(currentTag) == countries.end())
		{
			continue;
		}

		auto referenceBlock = getBlock(currentTag);
		if (!referenceBlock)
		{
			continue;
		}

		std::shared_ptr<HoI4::Country> currentCountry = countries.at(currentTag);
		const auto& neighbors = mapUtils.getNeighbors(currentTag);

		for (const std::string& neighborTag: neighbors)
		{
			if (visited.find(neighborTag) != visited.end())
			{
				continue;
			}

			if (countries.find(neighborTag) == countries.end())
			{
				continue;
			}

			const auto& neighborBlock = getBlock(neighborTag);
			if (!neighborBlock)
			{
				continue;
			}

			// Flag the transition to start checking against target
			if (*neighborBlock == target)
			{
				referenceBlock = target;
			}

			if (*neighborBlock == referenceBlock)
			{
				std::vector<std::string> newPath = currentPath;
				newPath.push_back(neighborTag);

				if (neighborTag == target)
				{
					return newPath;
				}

				paths.push(newPath);
				visited.insert(neighborTag);
			}
		}
	}

	return {};
}