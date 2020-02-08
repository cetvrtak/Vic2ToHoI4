#ifndef HOI4_DYNAMIC_MODIFIERS_H
#define HOI4_DYNAMIC_MODIFIERS_H

#include "newParser.h"
#include "../Ideologies/Ideologies.h"

namespace HoI4
{

class Modifier;

class DynamicModifiers: commonItems::parser
{

public:
    DynamicModifiers();
    [[nodiscard]] const auto& getDynamicModifiers() const { return dynamicModifiers;}

    void updateDynamicModifiers(const Ideologies& ideologies);

private:
    std::map<std::string, Modifier> dynamicModifiers;
};

}

#endif //HOI4_DYNAMIC_MODIFIERS_H