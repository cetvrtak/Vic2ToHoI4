#ifndef REBEL_TYPES_H
#define REBEL_TYPES_H value



#include "Parser.h"
#include "RebelType.h"



namespace Vic2
{

class RebelTypes: commonItems::parser
{

  public:
	class Factory;

	[[nodiscard]] const auto& getRebelTypesMap() const { return rebelTypesMap; }
	[[nodiscard]] const auto& getRebelType(const std::string& type) const { return rebelTypesMap.at(type); }

  private:
	std::map<std::string, RebelType> rebelTypesMap;
};

} // namespace Vic2



#endif // REBEL_TYPES_H