#pragma once

#include <unordered_set>

#include "attribute.hpp"

namespace abreu {

namespace ast {

using namespace clang;

std::unordered_map<clang::CXXRecordDecl *, int> DerivedCount = {};
std::unordered_map<clang::RecordDecl *, int> ReferenceCount = {};

bool AreMethodSignaturesEqual(const CXXMethodDecl *Method1, const CXXMethodDecl *Method2) {
    // Method names
    if (Method1->getNameAsString() != Method2->getNameAsString()) {
        return false;
    }

    // Params count
    if (Method1->getNumParams() != Method2->getNumParams()) {
        return false;
    }

    // Param types
    for (unsigned i = 0; i < Method1->getNumParams(); ++i) {
        QualType ParamType1 = Method1->getParamDecl(i)->getType().getCanonicalType();
        QualType ParamType2 = Method2->getParamDecl(i)->getType().getCanonicalType();
        if (ParamType1 != ParamType2) {
            return false;
        }
    }

    // CV-qual
    if (Method1->getType().getLocalCVRQualifiers() != Method2->getType().getLocalCVRQualifiers()) {
        return false;
    }

    return true;
}

void FillAttributesAndMethods(clang::CXXRecordDecl* Record, std::vector<clang::CXXMethodDecl*>& Methods, std::vector<Attribute>& Attributes) {
    std::vector<std::string> Result;
    std::unordered_map<std::string, clang::CXXMethodDecl*> Getters;

    for (clang::CXXMethodDecl* Meth : Record->methods()) {
        std::string MethName = Meth->getNameAsString();

        if (MethName.find("get") == 0 && Meth->getNumParams() == 0) {
            std::cout << "Found get: " << Meth->getNameAsString() << std::endl;
            std::string PropertyName = MethName.substr(3);
            Getters[PropertyName] = Meth;
        }
    }

    // std::cout << "Num getters: " << Getters.size() << std::endl;

    for (clang::CXXMethodDecl* Meth : Record->methods()) {
        std::string MethName = Meth->getNameAsString();

        if (MethName.find("get") == 0 && Meth->getNumParams() == 0)
            continue;
        if (MethName.find("set") == 0 && Meth->getNumParams() == 1) {
            std::string PropertyName = MethName.substr(3);

            if (Getters.count(PropertyName)) {
                // Если access spec одинаковый и есть get/set
                if (Getters[PropertyName]->getAccess() == Meth->getAccess())
                    Attributes.push_back({PropertyName, Meth->getAccess()});

                // Если access spec не сошлись, то беру по самому строгому
                else
                    Attributes.push_back({PropertyName, std::max(Getters[PropertyName]->getAccess(), Meth->getAccess())});
                Getters.erase(PropertyName);
            } 
            else {
                Methods.push_back(Meth);
            }
        } else {
            if (!Meth->isImplicit())
                Methods.push_back(Meth);
        }
    }

    // std::cout << "Num getters: " << Getters.size() << std::endl;
    // std::cout << "Num Attributes: " << Attributes.size() << std::endl;
    // std::cout << "Num Methods: " << Methods.size() << std::endl;

    // for (const auto pair : Methods)
        // std::cout << "Mathod: " << pair->getNameAsString() << std::endl;

    // Не нашедшие пару, туда же
    for (const auto& pair : Getters)
        Methods.push_back(pair.second);
}

struct Class {
private:
    clang::CXXRecordDecl* Record_ = nullptr;

    std::unordered_set<clang::CXXMethodDecl*> InheritedMethods = {};
    std::unordered_set<Attribute> InheritedAttributes = {};

    std::vector<clang::CXXMethodDecl*> Methods = {};
    std::vector<Attribute> Attributes = {};

    std::unordered_set<clang::CXXMethodDecl*> OverrideMethods = {};
    std::unordered_set<Attribute> OverrideAttributes = {};

    std::unordered_set<clang::CXXMethodDecl*> NewMethods = {};
    std::unordered_set<Attribute> NewAttributes = {};

    std::unordered_set<clang::CXXMethodDecl*> NewVisibleMethods = {};
    std::unordered_set<clang::CXXMethodDecl*> NewHiddenMethods = {};

    std::unordered_set<Attribute> NewVisibleAttributes = {};
    std::unordered_set<Attribute> NewHiddenAttributes = {};

public:
    int NewVisibleMethodsCnt() const { return NewVisibleMethods.size(); }
    int NewHiddenMethodsCnt() const { return NewHiddenMethods.size(); }

    int NewVisibleAttributesCnt() const { return NewVisibleAttributes.size(); }
    int NewHiddenAttributesCnt() const { return NewHiddenAttributes.size(); }

    int InheritedNotOverrideMethodsCnt() const { return InheritedMethods.size() - OverrideMethods.size(); }
    int InheritedOverrideMethodsCnt() const { return OverrideMethods.size(); }
    int NewMethodsCnt() const { return NewMethods.size(); }

    int InheritedNotOverrideAttributesCnt() const { return InheritedAttributes.size() - OverrideAttributes.size(); }
    int InheritedOverrideAttributesCnt() const { return OverrideAttributes.size(); }
    int NewAttributesCnt() const { return NewAttributes.size(); }

    int DerivedCnt() const { 
        std::cout << Record_->getNameAsString() << std::endl;
        std::cout << "Derived count: " << DerivedCount[Record_] << std::endl;
        return DerivedCount[Record_]; 
    }
    int ReferenceCnt() const {
        std::cout << Record_->getNameAsString() << std::endl;
        std::cout << "Reference count: " << ReferenceCount[Record_] << std::endl;
        return ReferenceCount[Record_];
    }

private:
    void traverseBaseClasses(const clang::CXXRecordDecl *CXXRD) {
        if (!CXXRD || !CXXRD->hasDefinition())
            return;

        for (const auto &Base : CXXRD->bases()) {
            // Get the type of the base class
            const clang::Type *BaseType = Base.getType().getTypePtr();
            clang::CXXRecordDecl *BaseDecl = BaseType->getAsCXXRecordDecl();

            if (BaseDecl) {
                std::cout << "  - " << BaseDecl->getNameAsString() << "\n";

                DerivedCount[BaseDecl]++;

                std::vector<clang::CXXMethodDecl*> BaseMethods = {};
                std::vector<Attribute> BaseAttributes = {};
                FillAttributesAndMethods(BaseDecl, BaseMethods, BaseAttributes);

                for (clang::CXXMethodDecl* Meth : BaseMethods) {
                    if (Meth->isImplicit())
                        continue;
                    if (Meth->getAccess() == clang::AccessSpecifier::AS_public 
                        || Meth->getAccess() == clang::AccessSpecifier::AS_protected)
                        InheritedMethods.insert(Meth);
                }

                for (auto Attr : BaseAttributes)
                    if (Attr.Access == clang::AccessSpecifier::AS_public
                        || Attr.Access == clang::AccessSpecifier::AS_protected)
                            InheritedAttributes.insert(Attr);

                traverseBaseClasses(BaseDecl);
            }
        }
    }

public:
    Class(clang::CXXRecordDecl* Record, clang::ASTContext *Context) : Record_(Record) {
        std::cout << "Name: " << Record->getNameAsString() << std::endl;

        traverseBaseClasses(Record);

        std::cout << "Inherited Methods Count: " << InheritedMethods.size() << std::endl;
        std::cout << "Inherited Attributes Count: " << InheritedAttributes.size() << std::endl;

        // Found class refs
        for (clang::FieldDecl* Field : Record->fields()) {
            std::cout << "FIELD: " << Field->getNameAsString();
            clang::QualType type = Field->getType();

            // Structure pointee
            if (type->isPointerType()) {
                const clang::QualType pointeeType = type->getPointeeType();

                if (const clang::RecordType *RefRecord = pointeeType->getAs<clang::RecordType>()) {
                    std::cout << " IS POINTER TO RECORD" << std::endl;

                    auto *ReferencedDecl = RefRecord->getDecl();
                    if (ReferencedDecl->getNameAsString() != Record->getNameAsString()) {
                        ReferenceCount[Record]++;
                    }
                }
            } else if (const clang::RecordType *RefRecord = type->getAs<clang::RecordType>()) {
                std::cout << " IS RECORD" << std::endl;

                auto *ReferencedDecl = RefRecord->getDecl();
                if (ReferencedDecl->getNameAsString() != Record->getNameAsString()) {
                    ReferenceCount[Record]++;
                }
            }

            std::cout << std::endl;
        }

        // Methods & Attrs
        FillAttributesAndMethods(Record, Methods, Attributes);

        // Overriden & New
        for (clang::CXXMethodDecl* Meth : Methods) {
            bool found = false;
            for (clang::CXXMethodDecl* InheritedMeth : InheritedMethods) {
                if (AreMethodSignaturesEqual(Meth, InheritedMeth)) {
                    OverrideMethods.insert(Meth);
                    found = true;
                    break;
                }
            }
            if (!found)
                NewMethods.insert(Meth);
        }

        for (Attribute Attr : Attributes) {
            bool found = false;
            for (Attribute InheritedAttr : InheritedAttributes) {
                if (Attr.Name == InheritedAttr.Name) {
                    OverrideAttributes.insert(Attr);
                    found = true;
                    break;
                }
            }
            if (!found)
                NewAttributes.insert(Attr);
        }

        std::cout << "Override Methods Count: " << OverrideMethods.size() << std::endl;
        std::cout << "Override Attributes Count: " << OverrideAttributes.size() << std::endl;

        for (clang::CXXMethodDecl* Meth : NewMethods) {
            if (Meth->getAccess() == clang::AccessSpecifier::AS_public)
                NewVisibleMethods.insert(Meth);
            if (Meth->getAccess() == clang::AccessSpecifier::AS_protected || Meth->getAccess() == clang::AccessSpecifier::AS_private)
                NewHiddenMethods.insert(Meth);
        }

        std::cout << "New Visible Methods Count: " << NewVisibleMethods.size() << std::endl;
        std::cout << "New Hidden Methods Count: " << NewHiddenMethods.size() << std::endl;

        for (Attribute Attr : NewAttributes) {
            if (Attr.Access == clang::AccessSpecifier::AS_public)
                NewVisibleAttributes.insert(Attr);
            if (Attr.Access == clang::AccessSpecifier::AS_protected || Attr.Access == clang::AccessSpecifier::AS_private)
                NewHiddenAttributes.insert(Attr);
        }

        std::cout << "New Visible Attributes Count: " << NewVisibleAttributes.size() << std::endl;
        std::cout << "New Hidden Attributes Count: " << NewHiddenAttributes.size() << std::endl;

        std::cout << std::endl;
    }
};

} // ast
} // abreu