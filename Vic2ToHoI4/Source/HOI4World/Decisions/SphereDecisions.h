#ifndef SPHERE_DECISIONS_H
#define SPHERE_DECISIONS_H

namespace HoI4
{

class SphereDecisions:
{
	public:
		void outputTargetTrigger(
			std::vector<std::shared_ptr<HoI4::Country>> greatPowers,
			std::vector<std::string> potentialSpherelings
		);

	private:
		std::vector<decisionsCategory> decisions;
};

}

#endif // SPHERE_DECISIONS_H