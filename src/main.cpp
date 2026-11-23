#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "../mvel/MvelEngine.hpp"
#include "TestScene.hpp"

using namespace mvel;

int main() {
    MvelEngine &app = MvelEngine::get();

    app.addScene("testscene", new factory::TestScene());
    app.drawScene("testscene");

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
