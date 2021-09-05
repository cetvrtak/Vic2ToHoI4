#ifndef REBEL_TYPE_H
#define REBEL_TYPE_H value



#include "Parser.h"



namespace Vic2
{

class RebelType: commonItems::parser
{

  public:
	class Factory;

	[[nodiscard]] const auto& getGovernmentMap() const { return governmentMap; }
	[[nodiscard]] const auto& getIdeology() const { return ideology; }
	[[nodiscard]] const auto& getGovernment(const std::string& govt) const { return governmentMap.at(govt); }

  private:
	std::map<std::string, std::string> governmentMap;
	std::string ideology;
};

} // namespace Vic2



#endif // REBEL_TYPE_H