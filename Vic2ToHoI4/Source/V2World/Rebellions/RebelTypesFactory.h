#ifndef REBEL_TYPES_FACTORY_H
#define REBEL_TYPES_FACTORY_H



#include "Configuration.h"
#include "Parser.h"
#include "RebelTypes.h"
#include <memory>



namespace Vic2
{

class RebelTypes::Factory: commonItems::parser
{
  public:
	Factory();
	std::unique_ptr<RebelTypes> importRebelTypes(const Configuration& theConfiguration);

  private:
	std::unique_ptr<RebelTypes> rebelTypes;
};

} // namespace Vic2



#endif // REBEL_TYPES_FACTORY_H