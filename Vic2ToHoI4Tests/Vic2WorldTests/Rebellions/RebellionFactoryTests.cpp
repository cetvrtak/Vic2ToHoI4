#include "V2World/Rebellions/Rebellion.h"
#include "V2World/Rebellions/RebellionFactory.h"
#include "gtest/gtest.h"



TEST(Vic2World_Rebellion_Tests, CountryDefaultsToEmpty)
{
	std::stringstream input;
	const auto newRebellion = Vic2::Rebellion::Factory().importRebellion(input);

	ASSERT_TRUE(newRebellion->getCountry().empty());
}


TEST(Vic2World_Rebellion_Tests, CountryCanBeSet)
{
	std::stringstream input;
	input << "country=\"BAV\"";
	const auto newRebellion = Vic2::Rebellion::Factory().importRebellion(input);

	ASSERT_EQ("BAV", newRebellion->getCountry());
}


TEST(Vic2World_Rebellion_Tests, TypeDefaultsToEmpty)
{
	std::stringstream input;
	const auto newRebellion = Vic2::Rebellion::Factory().importRebellion(input);

	ASSERT_TRUE(newRebellion->getType().empty());
}


TEST(Vic2World_Rebellion_Tests, TypeCanBeSet)
{
	std::stringstream input;
	input << "type=\"pan_nationalist_rebels\"";
	const auto newRebellion = Vic2::Rebellion::Factory().importRebellion(input);

	ASSERT_EQ("pan_nationalist_rebels", newRebellion->getType());
}


TEST(Vic2World_Rebellion_Tests, ProvincesDefaultToEmpty)
{
	std::stringstream input;
	const auto newRebellion = Vic2::Rebellion::Factory().importRebellion(input);

	ASSERT_TRUE(newRebellion->getProvinces().empty());
}


TEST(Vic2World_Rebellion_Tests, ProvincesCanBeSet)
{
	std::stringstream input;
	input << "provinces = { 2640 2641 }";
	const auto newRebellion = Vic2::Rebellion::Factory().importRebellion(input);

	ASSERT_EQ(std::vector<int>({2640, 2641}), newRebellion->getProvinces());
}


TEST(Vic2World_Rebellion_Tests, GovernmentDefaultsToEmpty)
{
	std::stringstream input;
	const auto newRebellion = Vic2::Rebellion::Factory().importRebellion(input);

	ASSERT_TRUE(newRebellion->getGovernment().empty());
}


TEST(Vic2World_Rebellion_Tests, GovernmentCanBeSet)
{
	std::stringstream input;
	input << "government=\"presidential_dictatorship\"";
	const auto newRebellion = Vic2::Rebellion::Factory().importRebellion(input);

	ASSERT_EQ("presidential_dictatorship", newRebellion->getGovernment());
}


TEST(Vic2World_Rebellion_Tests, ArmyIdsDefaultToEmpty)
{
	std::stringstream input;
	const auto newRebellion = Vic2::Rebellion::Factory().importRebellion(input);

	ASSERT_TRUE(newRebellion->getArmyIds().empty());
}


TEST(Vic2World_Rebellion_Tests, ArmyIdsCanBeSet)
{
	std::stringstream input;
	input << "\tarmy=\n";
	input << "\t{\n";
	input << "\t\tid=17066\n";
	input << "\t\ttype=38\n";
	input << "\t}\n";
	input << "\tarmy=\n";
	input << "\t{\n";
	input << "\t\tid=17082\n";
	input << "\t\ttype=38\n";
	input << "\t}\n";
	const auto newRebellion = Vic2::Rebellion::Factory().importRebellion(input);

	ASSERT_EQ(std::set<std::string>({"17066", "17082"}), newRebellion->getArmyIds());
}