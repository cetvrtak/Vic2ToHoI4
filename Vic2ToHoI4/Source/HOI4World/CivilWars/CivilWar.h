#ifndef CIVIL_WAR_H
#define CIVIL_WAR_H value



#include "Mappers/Provinces/ProvinceMapper.h"
#include "V2World/Provinces/Province.h"
#include "V2World/Rebellions/Rebellion.h"
#include "V2World/States/StateDefinitions.h"
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>



namespace HoI4
{

class Army;
class Country;

class State;

class CivilWar
{
  public:
	CivilWar(const Vic2::Rebellion& rebellion,
		 const std::shared_ptr<HoI4::Country>& country,
		 const Vic2::StateDefinitions& stateDefinitions,
		 const std::map<int, std::shared_ptr<Vic2::Province>>& vic2Provinces,
		 const Mappers::ProvinceMapper& provinceMapper,
		 const std::map<int, int>& provinceToStateIDMap,
		 const std::map<int, HoI4::State>& states);

	void setIdeology(const std::string& rebelType);
	void setOccupations(const Vic2::Rebellion& rebellion,
		 const std::shared_ptr<HoI4::Country>& country,
		 const Vic2::StateDefinitions& stateDefinitions,
		 const std::map<int, std::shared_ptr<Vic2::Province>>& vic2Provinces,
		 const Mappers::ProvinceMapper& provinceMapper,
		 const std::map<int, int>& provinceToStateIDMap,
		 const std::map<int, HoI4::State>& states);
	std::vector<std::pair<int, float>> getOccupationRatios(const Vic2::Rebellion& rebellion,
		 const Vic2::StateDefinitions& stateDefinitions,
		 const std::map<int, std::shared_ptr<Vic2::Province>>& vic2Provinces);

	[[nodiscard]] const auto& getOriginalTag() const { return originalTag; }
	[[nodiscard]] const auto& getIdeology() const { return ideology; }
	[[nodiscard]] const auto& getOccupiedStates() const { return occupiedStates; }
	[[nodiscard]] const auto& getOccupiedProvinces() const { return occupiedProvinces; }
	[[nodiscard]] const auto& getRebelArmies() const { return rebelArmies; }

  private:
	std::string originalTag;
	std::string ideology;
	std::set<std::string> occupiedStates;
	std::set<int> occupiedProvinces;
	std::set<Army> rebelArmies;
};

} // namespace HoI4



#endif