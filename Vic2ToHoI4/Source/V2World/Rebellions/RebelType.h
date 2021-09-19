#ifndef REBEL_TYPE_H
#define REBEL_TYPE_H value



#include "Parser.h"



namespace Vic2
{

class RebelType: commonItems::parser
{

  public:
	class Factory;

	[[nodiscard]] const auto& getIdeology() const { return ideology; }

  private:
	std::string ideology;
};

} // namespace Vic2



#endif // REBEL_TYPE_H