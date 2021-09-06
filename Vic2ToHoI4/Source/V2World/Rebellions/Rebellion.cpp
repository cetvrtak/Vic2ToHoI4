#include "Rebellion.h"


void Vic2::Rebellion::setGovernment(const std::map<std::string, Country>& countries)
{
	if (const auto& countryItr = countries.find(country); countryItr != countries.end())
	{
		government = rebelType.getGovernment(countryItr->second.getGovernment());
	}
}