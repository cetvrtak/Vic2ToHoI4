#ifndef HOI4_DIVISION_TYPE_H
#define HOI4_DIVISION_TYPE_H



#include <map>
#include <ostream>
#include <string>



namespace HoI4
{

class DivisionType
{
  public:
	DivisionType(std::string _name,
		 std::string _type,
		 const int _location,
		 const double _experience,
		 double _startEquipment,
		 std::map<std::string, int> _regiments):
		 name(std::move(_name)), type(std::move(_type)), location(_location), experience(_experience),
		 startEquipment(_startEquipment), regiments(_regiments)
	{
	}
	DivisionType(int manpower, int artillery, int infantry, int truck, int tank):
		 manpower(std::move(manpower)), artillery(std::move(artillery)), infantry(std::move(infantry)), truck(std::move(truck)), tank(std::move(tank))
	{
	}

	friend std::ostream& operator<<(std::ostream& out, const DivisionType&);

	[[nodiscard]] const auto& getType() const { return type; }
	[[nodiscard]] const auto& getRegiments() const { return regiments; }

	int manpower;
	int infantry;
	int artillery;
	int truck;
	int tank;

  private:
	std::string name;
	std::string type;
	int location = 0;
	double experience = 0.0;
	double startEquipment = 0.7;
	std::map<std::string, int> regiments;
};

} // namespace HoI4



#endif // HOI4_DIVISION_TYPE_H