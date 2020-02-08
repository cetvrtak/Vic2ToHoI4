#include "DynamicModifiers.h"
#include "Modifier.h"

HoI4::DynamicModifiers::DynamicModifiers()
{
    clearRegisteredKeywords();
    registerRegex("[a-zA-Z0-9_]+", [this](const std::string& modifierName, std::istream& theStream){
        Modifier modifier(theStream);
        dynamicModifiers.insert(make_pair(modifierName, modifier));
    });
    parseFile("converterDynamicModifiers.txt");
}

void HoI4::DynamicModifiers::updateDynamicModifiers(const Ideologies& ideologies)
{
    if (ideologies.getMajorIdeologies().count("fascism"))
    {
        auto& revanchismModifier = (dynamicModifiers.find("revanchism"))->second;
        std::string newString;

        newString = "= {\n";
        newString += "\t\tNOT = { has_government = fascism }\n";
        newString += "\t}\n";
        revanchismModifier.updateEnable(newString);

        revanchismModifier.addEffect("fascism_drift", "var:revanchism");
    }
    if (!ideologies.getMajorIdeologies().count("fascism"))
    {
        dynamicModifiers.erase("revanchism_fasc");
    }
}