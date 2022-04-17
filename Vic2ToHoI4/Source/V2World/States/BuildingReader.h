#ifndef VIC2_BUILDING_READER_H
#define VIC2_BUILDING_READER_H



#include "Parser.h"



namespace Vic2
{

class BuildingReader: commonItems::parser
{
  public:
	BuildingReader();

	int getLevel(std::istream& theStream);
	int getEmployees() const { return employees; }

  private:
	int level = 0;
	int employees = 0;
};

} // namespace Vic2



#endif // VIC2_BUILDING_READER_H