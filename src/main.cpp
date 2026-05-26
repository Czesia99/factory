#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "../sigel/SigelEngine.hpp"
#include "TestScene.hpp"

using namespace sigel;

int main() {
    SigelEngine &app = SigelEngine::get();
    
    app.addScene("testscene", new factory::TestScene());

    app.queueScene("testscene");
    
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}