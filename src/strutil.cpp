#include <string>
#include <sstream>
#include <nori/strutil.h>

NORI_NAMESPACE_BEGIN

std::string repeat(int n, std::string val) {
    std::ostringstream oss;
    for(int i = 0; i < n; i++) {
        oss << val;
    }
    
    return oss.str();
}

NORI_NAMESPACE_END