#pragma once

#include <string>
#include <functional>
#include <clang/Basic/Specifiers.h>

namespace abreu {
namespace ast {

struct Attribute {
    std::string Name;
    clang::AccessSpecifier Access;

    bool operator==(const Attribute& other) const {
        return Name == other.Name && Access == other.Access;
    }
};

}  // ast
}  // abreu

namespace std {
template <>
struct hash<abreu::ast::Attribute> {
    size_t operator()(const abreu::ast::Attribute& attr) const noexcept {
        size_t h1 = std::hash<std::string>{}(attr.Name);
        size_t h2 = std::hash<int>{}(static_cast<int>(attr.Access));
        return h1 ^ (h2 << 1); // Combina os dois hashes
    }
};
}