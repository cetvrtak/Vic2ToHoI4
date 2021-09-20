#ifndef REBEL_ARMY_H
#define REBEL_ARMY_H



#include "Parser.h"
#include "Rebellion.h"


namespace Vic2
{


class Rebellion::Army: commonItems::parser
{

  public:
	Army(std::istream& theStream);

	[[nodiscard]] const auto& getId() const { return id; }

  private:
	std::string id;
};


} // namespace Vic2



#endif // REBEL_ARMY_H