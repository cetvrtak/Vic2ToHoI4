#include "V2World/Rebellions/RebelType.h"
#include "V2World/Rebellions/RebelTypeFactory.h"
#include "gtest/gtest.h"



TEST(Vic2World_RebelType_Tests, IdeologyDefaultsToEmpty)
{
	std::stringstream input;
	const auto newRebelType = Vic2::RebelType::Factory().importRebelType(input);

	ASSERT_TRUE(newRebelType->getIdeology().empty());
}


TEST(Vic2World_RebelType_Tests, IdeologyCanBeSet)
{
	std::stringstream input;
	input << "ideology = reactionary";
	const auto newRebelType = Vic2::RebelType::Factory().importRebelType(input);

	ASSERT_EQ("reactionary", newRebelType->getIdeology());
}