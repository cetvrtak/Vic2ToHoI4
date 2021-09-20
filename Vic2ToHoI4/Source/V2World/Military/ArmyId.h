#ifndef ARMY_ID_H_
#define ARMY_ID_H_



#include "Parser.h"


namespace Vic2
{

class ArmyId: commonItems::parser
{

public:
	ArmyId(std::istream& theStream);

	[[nodiscard]] const auto& getId() const { return id; }

private:
	std::string id;

};

} // namespace Vic2



#endif // ARMY_ID_H_