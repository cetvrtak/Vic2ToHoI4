#ifndef REBELLION_H
#define REBELLION_H



#include "V2World/Countries/Country.h"
#include "V2World/Military/Army.h"
#include "V2World/Rebellions/RebelType.h"
#include <string>
#include <vector>



namespace Vic2
{

class Rebellion
{
  public:
	class Factory;
	class Army;

	void assignRebelType(const RebelType& rebelType_) { rebelType = rebelType_; }
	void assignRebelArmy(const Vic2::Army& army) { armies.push_back(army); }

	[[nodiscard]] const auto& getCountry() const { return country; }
	[[nodiscard]] const auto& getType() const { return type; }
	[[nodiscard]] const auto& getProvinces() const { return provinces; }
	[[nodiscard]] const auto& getRebelType() const { return rebelType; }
	[[nodiscard]] const auto& getGovernment() const { return government; }
	[[nodiscard]] const auto& getArmyIds() const { return armyIds; }
	[[nodiscard]] const auto& getArmies() const { return armies; }
	[[nodiscard]] const auto& getIndependence() const { return independence; }

  private:
	std::string country;
	std::string type;
	std::vector<int> provinces;
	RebelType rebelType;
	std::string government;
	std::set<std::string> armyIds;
	std::vector<Vic2::Army> armies;
	std::string independence;
};

} // namespace Vic2



#endif // REBELLION_H