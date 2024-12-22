#include "WorldWar.h"
#include "src/HOI4World/HoI4Country.h"
#include <queue>
#include <ranges>
using namespace std;

HoI4::WorldWar::WorldWar(const std::string& playerTag_,
	 const std::string& archenemyTag_,
	 const std::map<std::string, std::shared_ptr<HoI4::Country>>& countries_,
	 const HoI4::MapUtils& mapUtils_):
	 playerTag(std::move(playerTag_)), archenemyTag(std::move(archenemyTag_)), countries(std::move(countries_)),
	 mapUtils(std::move(mapUtils_))
{
	CreateBlocks();
	Log(LogLevel::Debug) << "WorldWar blocks size: " << blocks.size();

	// 	Calculate block strenghts && weaknesses
	// 	Rebalance blocks
	// 	Build front(s)
	CreateFront();

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

std::optional<std::string> HoI4::WorldWar::GetBlock(const std::string& tag)
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

void HoI4::WorldWar::AddToFront(const std::string& tag)
{
	const auto& block = GetBlock(tag);
	if (block)
	{
		front[*block].insert(tag);
	}
}

void HoI4::WorldWar::CreateFront()
{
	Log(LogLevel::Info) << "\tCreating front";
	for (const auto& [leader, members]: blocks)
	{
		const auto& oppositeBlock = std::find_if(blocks.begin(),
			 blocks.end(),
			 [leader](const std::pair<std::string, std::set<std::string>>& block) {
				 return block.first != leader;
			 })->first;
		Log(LogLevel::Debug) << "Blocks: " << leader << " | " << oppositeBlock;

		for (const auto& member: members)
		{
			Log(LogLevel::Debug) << "\t" << member;
			const auto& path = FindShortestPathToTarget(member, oppositeBlock);
			for (auto itr = path.begin(); itr != path.end(); ++itr)
			{
				const auto& tag = *itr;
				const auto& neighbor = *std::next(itr);
				if (GetBlock(neighbor) != GetBlock(tag))
				{
					Log(LogLevel::Debug) << "\t\tAdding: " << tag << " -> " << neighbor;
					AddToFront(tag);
					break;
				}
			}
		}
	}
}

std::vector<std::string> HoI4::WorldWar::FindShortestPathToTarget(const std::string& source, const std::string& target)
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

		auto referenceBlock = GetBlock(currentTag);
		if (!referenceBlock)
		{
			continue;
		}
		Log(LogLevel::Debug) << "Popping new path from: " << currentTag << " (" << *referenceBlock << ")";

		std::shared_ptr<HoI4::Country> currentCountry = countries.at(currentTag);
		const auto& neighbors = mapUtils.GetCapitalAreaNeighbors(currentCountry);

		for (const std::string& neighborTag: neighbors)
		{
			Log(LogLevel::Debug) << "Checking neighbor: " << neighborTag;
			if (visited.find(neighborTag) != visited.end())
			{
				Log(LogLevel::Debug) << "\tAlready visited -> skipping";
				continue;
			}

			if (countries.find(neighborTag) == countries.end())
			{
				Log(LogLevel::Debug) << "\tNo such country -> skipping";
				continue;
			}

			const auto& neighborBlock = GetBlock(neighborTag);
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
				Log(LogLevel::Debug) << " -> Adding new path with: " << neighborTag;
				newPath.push_back(neighborTag);

				if (neighborTag == target)
				{
					Log(LogLevel::Debug) << "Found archenemy. Returning path";
					return newPath;
				}

				paths.push(newPath);
				visited.insert(neighborTag);
			}
		}
	}

	return {};
}
