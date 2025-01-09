#pragma once

#include <unordered_set>

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

struct Class {
private:
    clang::CXXRecordDecl* Record_ = nullptr;

    std::unordered_set<clang::CXXMethodDecl*> InheritedMethods = {};
    std::unordered_set<clang::FieldDecl*> InheritedFields = {};

    std::unordered_set<clang::CXXMethodDecl*> Methods = {};
    std::unordered_set<clang::FieldDecl*> Fields = {};

    std::unordered_set<clang::CXXMethodDecl*> OverrideMethods = {};
    std::unordered_set<clang::FieldDecl*> OverrideFields = {};

    std::unordered_set<clang::CXXMethodDecl*> NewMethods = {};
    std::unordered_set<clang::FieldDecl*> NewFields = {};

    std::unordered_set<clang::CXXMethodDecl*> NewVisibleMethods = {};
    std::unordered_set<clang::CXXMethodDecl*> NewHiddenMethods = {};

    std::unordered_set<clang::FieldDecl*> NewVisibleFields = {};
    std::unordered_set<clang::FieldDecl*> NewHiddenFields = {};

public:
    int NewVisibleMethodsCnt() const { return NewVisibleMethods.size(); }
    int NewHiddenMethodsCnt() const { return NewHiddenMethods.size(); }

    int NewVisibleFieldsCnt() const { return NewVisibleFields.size(); }
    int NewHiddenFieldsCnt() const { return NewHiddenFields.size(); }

    int InheritedNotOverrideMethodsCnt() const { return InheritedMethods.size() - OverrideMethods.size(); }
    int InheritedOverrideMethodsCnt() const { return OverrideMethods.size(); }
    int NewMethodsCnt() const { return NewMethods.size(); }

    int InheritedNotOverrideFieldsCnt() const { return InheritedFields.size() - OverrideFields.size(); }
    int InheritedOverrideFieldsCnt() const { return OverrideFields.size(); }
    int NewFieldsCnt() const { return NewFields.size(); }

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

                for (clang::CXXMethodDecl* Meth : BaseDecl->methods()) {
                    if (Meth->isImplicit())
                        continue;
                    if (Meth->getAccess() == clang::AccessSpecifier::AS_public 
                        || Meth->getAccess() == clang::AccessSpecifier::AS_protected)
                        InheritedMethods.insert(Meth);
                }

                for (clang::FieldDecl* Field : BaseDecl->fields()) {
                    if (Field->getAccess() == clang::AccessSpecifier::AS_public
                        || Field->getAccess() == clang::AccessSpecifier::AS_protected)
                        InheritedFields.insert(Field);
                }

                traverseBaseClasses(BaseDecl);
            }
        }
    }

public:
    Class(clang::CXXRecordDecl* Record, clang::ASTContext *Context) : Record_(Record) {
        std::cout << "Name: " << Record->getNameAsString() << std::endl;

        traverseBaseClasses(Record);

        for (clang::FieldDecl* Field : Record->fields()) {
            std::cout << Field->getNameAsString();
            clang::QualType type = Field->getType();

            // Проверяем, является ли это поле указателем на структуру
            if (type->isPointerType()) {
                const clang::QualType pointeeType = type->getPointeeType(); // Достаем тип, на который указывает поле

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

            std::cout << std::endl; // Завершаем строку вывода
        }


        std::cout << "Inherited Methods Count: " << InheritedMethods.size() << std::endl;
        std::cout << "Inherited Fields Count: " << InheritedFields.size() << std::endl;

        for (clang::CXXMethodDecl* Meth : Record->methods()) {
            if (Meth->isImplicit())
                continue;
            Methods.insert(Meth);
        }

        for (clang::FieldDecl* Field : Record->fields()) {
            Fields.insert(Field);
        }

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

        for (clang::FieldDecl* Field : Fields) {
            bool found = false;
            for (clang::FieldDecl* InheritedField : InheritedFields) {
                if (Field->getName() == InheritedField->getName()) {
                    OverrideFields.insert(Field);
                    found = true;
                    break;
                }
            }
            if (!found)
                NewFields.insert(Field);
        }

        std::cout << "Override Methods Count: " << OverrideMethods.size() << std::endl;
        std::cout << "Override Fields Count: " << OverrideFields.size() << std::endl;

        for (clang::CXXMethodDecl* Meth : NewMethods) {
            if (Meth->getAccess() == clang::AccessSpecifier::AS_public)
                NewVisibleMethods.insert(Meth);
            if (Meth->getAccess() == clang::AccessSpecifier::AS_protected || Meth->getAccess() == clang::AccessSpecifier::AS_private)
                NewHiddenMethods.insert(Meth);
        }

        std::cout << "New Visible Methods Count: " << NewVisibleMethods.size() << std::endl;
        std::cout << "New Hidden Methods Count: " << NewHiddenMethods.size() << std::endl;

        for (clang::FieldDecl* Field : NewFields) {
            if (Field->getAccess() == clang::AccessSpecifier::AS_public)
                NewVisibleFields.insert(Field);
            if (Field->getAccess() == clang::AccessSpecifier::AS_protected || Field->getAccess() == clang::AccessSpecifier::AS_private)
                NewHiddenFields.insert(Field);
        }

        std::cout << "New Visible Fields Count: " << NewVisibleFields.size() << std::endl;
        std::cout << "New Hidden Fields Count: " << NewHiddenFields.size() << std::endl;

        std::cout << std::endl;
    }
};

} // ast
} // abreu