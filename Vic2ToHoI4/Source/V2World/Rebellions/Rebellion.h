#ifndef REBELLION_H
#define REBELLION_H



#include "V2World/Countries/Country.h"
#include "V2World/Rebellions/RebelType.h"
#include <string>
#include <vector>



namespace Vic2
{

class Rebellion
{
  public:
	class Factory;

	void assignRebelType(const RebelType& rebelType_) { rebelType = rebelType_; }
	void setGovernment(const std::map<std::string, Country>& countries);

	[[nodiscard]] const auto& getCountry() const { return country; }
	[[nodiscard]] const auto& getType() const { return type; }
	[[nodiscard]] const auto& getProvinces() const { return provinces; }
	[[nodiscard]] const auto& getRebelType() const { return rebelType; }
	[[nodiscard]] const auto& getGovernment() const { return government; }

  private:
	std::string country;
	std::string type;
	std::vector<int> provinces;
	RebelType rebelType;
	std::string government;
};

} // namespace Vic2



#endif // REBELLION_H