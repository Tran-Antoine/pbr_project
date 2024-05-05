#include <parser/lsystem.h>
#include <core/common.h>
#include <pcg32.h>

using namespace nori;


int main(int argc, char **argv) {

    auto random = pcg32();
    auto config = LSystemGrammar::fromConfig(random, "scenes/tree2/treegrammar.l6");
    std::cout << "Config successfully parsed\n";

}