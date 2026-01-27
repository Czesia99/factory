#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "../sigel/SigelEngine.hpp"

int main() {
    sigel::SigelEngine app;
    
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}