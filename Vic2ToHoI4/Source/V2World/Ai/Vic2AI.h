#ifndef VIC2_VIC2AI_H_
#define VIC2_VIC2AI_H_



#include "newParser.h"



namespace Vic2
{

class AIStrategy;


class Vic2AI: commonItems::parser
{
	public:
		explicit Vic2AI(std::istream& theStream);

		std::vector<const AIStrategy*> getStrategies() const { return aiStrategies; }

	private:
		Vic2AI& operator=(const Vic2AI&) = delete;

		std::vector<const AIStrategy*> aiStrategies;
};

}



#endif // VIC2_VIC2AI_H_