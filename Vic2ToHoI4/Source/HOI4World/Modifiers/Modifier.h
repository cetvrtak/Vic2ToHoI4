#ifndef HOI4_MODIFIER_H
#define HOI4_MODIFIER_H



#include <string>
#include <vector>
#include "newParser.h"

namespace HoI4
{

class Modifier: commonItems::parser
{
	public:
		explicit Modifier(std::istream& theStream);
		Modifier(const Modifier&) = default;

		friend std::ostream& operator << (std::ostream& output, const Modifier& modifier);

		std::string getIcon() const { return icon; }
		std::string getEnable() const { return enable; }
		std::string getRemoveTrigger() const { return removeTrigger; }
		std::map<std::string, std::string> getEffects() const { return effects; }

		void updateEnable(std::string newEnable) { enable = std::move(newEnable); }
		void updateRemoveTrigger(std::string newTrigger) { removeTrigger = std::move(newTrigger); }
		void addEffect(std::string modifier, std::string value) { effects.insert(std::move(make_pair(modifier, value))); }

	private:
		Modifier& operator=(const Modifier&) = delete;

		std::string icon;
		std::string enable;
		std::string removeTrigger;
		std::map<std::string, std::string> effects;
};

}

#endif // HOI4_MODIFIER_H