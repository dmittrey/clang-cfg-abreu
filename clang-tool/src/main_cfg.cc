#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Support/CommandLine.h>

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "action.hpp"

#include <string>

using namespace std;
using namespace llvm;
using namespace clang;
using namespace clang::tooling;

int main(int argc, char **argv) {
    if (argc > 1) {
        // Имя файла передаётся вторым аргументом командной строки
        std::ifstream inputFile(argv[1]);
        
        if (!inputFile.is_open()) {
            std::cerr << "Ошибка: не удалось открыть файл " << argv[1] << std::endl;
            return 1;
        }

        // Считываем содержимое файла в строку
        std::string code((std::istreambuf_iterator<char>(inputFile)),
                          std::istreambuf_iterator<char>());

        inputFile.close();

        // Передаём считанный код в clang tool
        clang::tooling::runToolOnCode(std::make_unique<ControlFlowAction>(), code);
    } else {
        std::cerr << "Ошибка: укажите путь до файла как аргумент командной строки." << std::endl;
        return 1;
    }

    return 0;
}
