#ifndef SPHERE_DECISIONS_H
#define SPHERE_DECISIONS_H



#include "DecisionsFile.h"
#include <map>
#include <set>
#include <string>



namespace HoI4
{

class Country;

class SphereDecisions: public DecisionsFile
{
	public:
		void updateSphereDecisions(const std::vector<std::shared_ptr<HoI4::Country>>& greatPowers);
};

}



#endif // SPHERE_DECISIONS_H