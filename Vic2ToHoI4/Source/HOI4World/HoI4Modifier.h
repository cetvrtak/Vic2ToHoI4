#ifndef HOI4_MODIFIER_H
#define HOI4_MODIFIER_H



#include <string>
#include <vector>
#include "newParser.h"



class HoI4Modifier: commonItems::parser
{
	public:
		HoI4Modifier() = default;
		explicit HoI4Modifier(std::istream& theStream);
		HoI4Modifier(const HoI4Modifier&) = default;

		friend std::ostream& operator << (std::ostream& output, const HoI4Modifier& modifier);

		void updateRemoveTrigger(std::string newTrigger) { removeTrigger = newTrigger; }
		void addEffect(std::string modifier, std::string value) { effects.insert(make_pair(modifier, value)); }

	private:
		HoI4Modifier& operator=(const HoI4Modifier&) = delete;

		std::string icon;
		std::string enable;
		std::string removeTrigger;
		std::map<std::string, std::string> effects;
};



#endif // HOI4_MODIFIER_H