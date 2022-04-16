#ifndef RGO_H_
#define RGO_H_


#include "Parser.h"



namespace Vic2
{

class RGO: commonItems::parser
{
  public:
	RGO() = default;
	explicit RGO(std::istream& theStream);

	[[nodiscard]] int getEmployees() const { return employees; }

  private:
	void registerKeys();
 	int employees = 0;
};

} // namespace Vic2



#endif // RGO_H_
