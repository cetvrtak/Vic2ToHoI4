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

	[[nodiscard]] const auto& getEmployees() const { return employees; }
	[[nodiscard]] const auto& getIncome() const { return income; }
	[[nodiscard]] const auto& getType() const { return type; }

  private:
	void registerKeys();
 	int employees = 0;
 	double income = 0.0;
 	std::string type;
};

} // namespace Vic2



#endif // RGO_H_
