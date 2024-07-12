#ifndef WORLD_WAR_H_
#define WORLD_WAR_H_



#include "src/HOI4World/HoI4Country.h"
#include "src/HOI4World/WarCreator/MapUtils.h"
#include <map>
#include <memory>
#include <set>
#include <string>


namespace HoI4
{

class WorldWar
{
  public:
	WorldWar(const std::string& playerTag_,
		 const std::string& archenemyTag_,
		 const std::map<std::string, std::shared_ptr<HoI4::Country>>& countries_,
		 const HoI4::MapUtils& mapUtils);

	void CreateBlocks();
	void CreateFront();

	void AddToBlock(const std::string& tag, const std::string& blockLeader) { blocks[blockLeader].insert(tag); }
	void AddGuaranteedToBlock(const std::string& tag);
	void AddFriendlyToBlock(const std::shared_ptr<HoI4::Country>& country);
	void AddToFront(const std::string& tag);
	std::optional<std::string> GetBlock(const std::string& tag);

  private:
	std::vector<std::string> FindShortestPathToTarget(const std::string& source, const std::string& target);

	std::string playerTag;
	std::string archenemyTag;
	std::map<std::string, std::shared_ptr<HoI4::Country>> countries;
	HoI4::MapUtils mapUtils;

	std::map<std::string, std::set<std::string>> blocks;
	std::map<std::string, std::set<std::string>> front;
};

} // namespace HoI4



#endif // WORLD_WAR_H_