#include <fenv.h>
#include <unistd.h>

#include "app/app.hpp"

int main() {
    // feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);

    AppSpecification app_spec = {.name = "fluid_sim"};

    App app(app_spec);
    app.run();
}
