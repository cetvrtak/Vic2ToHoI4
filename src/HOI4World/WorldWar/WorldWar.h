#ifndef WORLD_WAR_H_
#define WORLD_WAR_H_



#include "src/HOI4World/HoI4Country.h"
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
		 const std::map<std::string, std::shared_ptr<HoI4::Country>>& countries_);

	void CreateBlocks();

	void AddToBlock(const std::string& tag, const std::string& blockLeader) { blocks[blockLeader].insert(tag); }
	void AddGuaranteedToBlock(const std::string& tag);
	void AddFriendlyToBlock(const std::shared_ptr<HoI4::Country>& country);

  private:
	std::string playerTag;
	std::string archenemyTag;
	std::map<std::string, std::shared_ptr<HoI4::Country>> countries;

	std::map<std::string, std::set<std::string>> blocks;
};

} // namespace HoI4



#endif // WORLD_WAR_H_