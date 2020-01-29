#ifndef SPHERE_DECISIONS_H
#define SPHERE_DECISIONS_H



#include "DecisionsFile.h"
#include <map>
#include <set>
#include <string>



namespace HoI4
{

class SphereDecisions: public DecisionsFile
{
	public:
		void updateSphereDecisions();
};

}



#endif // SPHERE_DECISIONS_H