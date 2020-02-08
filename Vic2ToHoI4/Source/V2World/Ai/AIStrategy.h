#ifndef VIC2_AI_STRATEGY_H_
#define VIC2_AI_STRATEGY_H_



#include "newParser.h"



namespace Vic2
{

class AIStrategy: commonItems::parser
{
	public:
		explicit AIStrategy(const std::string& strategyType, std::istream& theStream);

		std::string getType() const { return type; }
		std::string getID() const { return id; }
		int getValue() const { return value; }

	private:
		AIStrategy& operator=(const AIStrategy&) = delete;

		std::string type;
		std::string id;
		int value = 0;
};

}



#endif // VIC2_AISTRATEGY_H_