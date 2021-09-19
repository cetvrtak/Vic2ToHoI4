#ifndef REBEL_TYPE_FACTORY_H
#define REBEL_TYPE_FACTORY_H



#include "Parser.h"
#include "RebelType.h"
#include <memory>



namespace Vic2
{

class RebelType::Factory: commonItems::parser
{
  public:
	Factory();
	std::unique_ptr<RebelType> importRebelType(std::istream& theStream);

  private:
	std::unique_ptr<RebelType> rebelType;
};

} // namespace Vic2



#endif // REBEL_TYPE_FACTORY_H