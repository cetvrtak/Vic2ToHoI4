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
class State;

class CivilWar
{
  public:
	CivilWar(const Vic2::Rebellion& rebellion,
		 const std::string& originalCountryTag,
		 const Vic2::StateDefinitions& stateDefinitions,
		 const std::map<int, std::shared_ptr<Vic2::Province>>& vic2Provinces,
		 const Mappers::ProvinceMapper& provinceMapper,
		 const std::map<int, int>& provinceToStateIDMap,
		 const std::map<int, HoI4::State>& states);

	void setOccupations(const Vic2::Rebellion& rebellion,
		 const std::string& originalCountryTag,
		 const Vic2::StateDefinitions& stateDefinitions,
		 const std::map<int, std::shared_ptr<Vic2::Province>>& vic2Provinces,
		 const Mappers::ProvinceMapper& provinceMapper,
		 const std::map<int, int>& provinceToStateIDMap,
		 const std::map<int, HoI4::State>& states);
	std::vector<std::pair<int, float>> getOccupationRatios(const Vic2::Rebellion& rebellion,
		 const Vic2::StateDefinitions& stateDefinitions,
		 const std::map<int, std::shared_ptr<Vic2::Province>>& vic2Provinces);

	[[nodiscard]] const auto& getOriginalTag() const { return originalTag; }
	[[nodiscard]] const auto& getVic2RebelType() const { return vic2RebelType; }
	[[nodiscard]] const auto& getVic2Capital() const { return vic2Capital; }
	[[nodiscard]] const auto& getVic2Government() const { return vic2Government; }
	[[nodiscard]] const auto& getVic2Ideology() const { return vic2Ideology; }
	[[nodiscard]] const auto& getOccupiedStates() const { return occupiedStates; }
	[[nodiscard]] const auto& getOccupiedProvinces() const { return occupiedProvinces; }
	[[nodiscard]] const auto& getVic2Armies() const { return vic2Armies; }

  private:
	std::string originalTag;
	std::string vic2RebelType;
	int vic2Capital;
	std::string vic2Government;
	std::string vic2Ideology;
	std::set<std::string> occupiedStates;
	std::set<int> occupiedProvinces;
	std::vector<Vic2::Army> vic2Armies;
};

} // namespace HoI4



#endif