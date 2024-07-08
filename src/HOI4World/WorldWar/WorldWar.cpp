#include "WorldWar.h"
#include "src/HOI4World/HoI4Country.h"
using namespace std;

HoI4::WorldWar::WorldWar(const std::string& playerTag_,
	 const std::string& archenemyTag_,
	 const std::map<std::string, std::shared_ptr<HoI4::Country>>& countries_):
	 playerTag(std::move(playerTag_)), archenemyTag(std::move(archenemyTag_)), countries(std::move(countries_))
{
}
