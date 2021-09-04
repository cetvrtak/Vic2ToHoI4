#include "CivilWar.h"
#include "HOI4World/HoI4Country.h"



HoI4::CivilWar::CivilWar(const Vic2::Rebellion& rebellion,
	 const std::shared_ptr<HoI4::Country>& country,
	 const Vic2::StateDefinitions& stateDefinitions,
	 const std::map<int, std::shared_ptr<Vic2::Province>>& vic2Provinces,
	 const Mappers::ProvinceMapper& provinceMapper,
	 const std::map<int, int>& provinceToStateIDMap,
	 const std::map<int, HoI4::State>& states):
	 originalTag(country->getTag())
{
	setIdeology(rebellion.getType());
	setOccupations(rebellion, country, stateDefinitions, vic2Provinces, provinceMapper, provinceToStateIDMap, states);
}


void HoI4::CivilWar::setIdeology(const std::string& rebelType)
{
	const std::map<std::string, std::string>& theMap = {{"reactionary_rebels", "absolutist"},
		 {"fascist_rebels", "fascism"},
		 {"communist_rebels", "communism"},
		 {"anarcho_liberal_rebels", "radical"},
		 {"carlist_rebels", "absolutist"},
		 {"boxer_rebels", "absolutist"},
		 {"liberal_rebels", "democratic"},
		 {"unciv_reactionary_rebels", "absolutist"},
		 // HPM
		 {"christino_rebels", "democratic"},
		 {"socialist_rebels", "communism"},
		 {"turkish_nationalist_rebels", "democratic"},
		 {"native_rebels", "democratic"}};

	if (theMap.contains(rebelType))
	{
		ideology = theMap.at(rebelType);
	}
}


constexpr float requiredOccupationPercentage = 1;

void HoI4::CivilWar::setOccupations(const Vic2::Rebellion& rebellion,
	 const std::shared_ptr<HoI4::Country>& country,
	 const Vic2::StateDefinitions& stateDefinitions,
	 const std::map<int, std::shared_ptr<Vic2::Province>>& vic2Provinces,
	 const Mappers::ProvinceMapper& provinceMapper,
	 const std::map<int, int>& provinceToStateIDMap,
	 const std::map<int, HoI4::State>& states)
{
	const auto& occupationRatios = getOccupationRatios(rebellion, stateDefinitions, vic2Provinces);
	std::set<int> partiallyOccupiedStates;

	for (const auto& [province, ratio]: occupationRatios)
	{
		const auto& hoi4Provinces = provinceMapper.getVic2ToHoI4ProvinceMapping(province);
		if (hoi4Provinces.empty())
		{
			continue;
		}
		for (const auto& hoi4Province: hoi4Provinces)
		{
			occupiedProvinces.insert(hoi4Province);
		}
		
		const auto& hoi4StateIdItr = provinceToStateIDMap.find(*hoi4Provinces.begin());
		if (hoi4StateIdItr == provinceToStateIDMap.end())
		{
			continue;
		}
		const auto& hoi4StateId = hoi4StateIdItr->second;
		const auto& hoi4StateItr = states.find(hoi4StateId);
		if (hoi4StateItr == states.end())
		{
			continue;
		}
		const auto& hoi4State = hoi4StateItr->second;
		if (const auto& owner = hoi4State.getOwner(); owner != country->getTag())
		{
			continue;
		}

		partiallyOccupiedStates.insert(hoi4StateId);

		if (ratio >= requiredOccupationPercentage)
		{
			occupiedStates.insert(std::to_string(hoi4StateId));
		}
	}

	if (const auto& capitalState = country->getCapitalState(); occupiedStates.empty() && capitalState)
	{
		for (const auto& hoi4StateId: partiallyOccupiedStates)
		{
			if (*capitalState != hoi4StateId)
			{
				occupiedStates.insert(std::to_string(hoi4StateId));
				break;
			}
		}
	}
}


std::vector<std::pair<int, float>> HoI4::CivilWar::getOccupationRatios(const Vic2::Rebellion& rebellion,
	 const Vic2::StateDefinitions& stateDefinitions,
	 const std::map<int, std::shared_ptr<Vic2::Province>>& vic2Provinces)
{
	std::vector<std::pair<int, float>> ratios;
	const auto& provinces = rebellion.getProvinces();
	for (const auto& province: provinces)
	{
		std::set<int> ownedProvinces;
		for (const auto& provinceId: stateDefinitions.getAllProvinces(province))
		{
			if (const auto& provinceItr = vic2Provinces.find(provinceId);
				 provinceItr != vic2Provinces.end() && provinceItr->second->getOwner() == rebellion.getCountry())
			{
				ownedProvinces.insert(provinceId);
			}
		}
		int numOccupied = 0;
		for (const auto& province: ownedProvinces)
		{
			if (std::find(provinces.begin(), provinces.end(), province) != provinces.end())
			{
				numOccupied++;
			}
		}
		const auto& ratio = static_cast<float>(numOccupied) / ownedProvinces.size();
		ratios.push_back(std::make_pair(province, ratio));
	}
	std::sort(ratios.begin(), ratios.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
		return a.second > b.second;
	});

	return ratios;
}