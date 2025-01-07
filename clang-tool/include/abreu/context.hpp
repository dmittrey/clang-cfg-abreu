#pragma once

#include "ast.hpp"

namespace abreu {

struct Context {
private:
    std::vector<ast::Class*> Classes;

private:
    double MethodHidingFactor() const {
        double hidden = 0;
        double all = 0;
        for (auto* Class : Classes) {
            hidden += Class->NewHiddenMethodsCnt();

            all += Class->NewVisibleMethodsCnt();
            all += Class->NewHiddenMethodsCnt();
        }
        return hidden / all;
    }

    double AttributeHidingFactor() const {
        double hidden = 0;
        double all = 0;
        for (auto* Class : Classes) {
            hidden += Class->NewHiddenFieldsCnt();

            all += Class->NewVisibleFieldsCnt();
            all += Class->NewHiddenFieldsCnt();
        }
        return hidden / all;
    }

    double MethodInheritanceFactor() const {
        double NotOverriden = 0;
        double All = 0;
        for (auto* Class : Classes) {
            NotOverriden += Class->InheritedNotOverrideMethodsCnt();

            All += Class->InheritedNotOverrideMethodsCnt();
            All += Class->InheritedOverrideMethodsCnt();
            All += Class->NewMethodsCnt();
        }
        return NotOverriden / All;
    }

    double AttributeInheritanceFactor() const {
        double NotOverriden = 0;
        double All = 0;
        for (auto* Class : Classes) {
            NotOverriden += Class->InheritedNotOverrideFieldsCnt();

            All += Class->InheritedNotOverrideFieldsCnt();
            All += Class->InheritedOverrideFieldsCnt();
            All += Class->NewFieldsCnt();
        }
        return NotOverriden / All;
    }

    double PolymorphismFactor() const {
        double Overriden = 0;
        double All = 0;
        for (auto* Class : Classes) {
            Overriden += Class->InheritedOverrideMethodsCnt();
            // std::cout << Class->InheritedOverrideMethodsCnt() << " " << Class->NewMethodsCnt() << " " << Class->DerivedCnt() << std::endl;

            All += Class->NewMethodsCnt() * Class->DerivedCnt();
        }
        return Overriden / All;
    }

public:
    void Push(ast::Class* NewClass) {
        Classes.push_back(NewClass);
    }

public:
    void Stats() const {
        std::cout << "Method Hiding Factor: " << MethodHidingFactor() << std::endl;
        std::cout << "Attribute Hiding Factor: " << AttributeHidingFactor() << std::endl;
        std::cout << "Method Inheritance Factor: " << MethodInheritanceFactor() << std::endl;
        std::cout << "Attribute Inheritance Factor: " << AttributeInheritanceFactor() << std::endl;
        std::cout << "Polymorphism Factor: " << PolymorphismFactor() << std::endl;
    }
};

}