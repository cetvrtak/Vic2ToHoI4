#ifndef REBELLION_H
#define REBELLION_H



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

	[[nodiscard]] const auto& getCountry() const { return country; }
	[[nodiscard]] const auto& getType() const { return type; }
	[[nodiscard]] const auto& getProvinces() const { return provinces; }
	[[nodiscard]] const auto& getRebelType() const { return rebelType; }

  private:
	std::string country;
	std::string type;
	std::vector<int> provinces;
	RebelType rebelType;
};

} // namespace Vic2



#endif // REBELLION_H