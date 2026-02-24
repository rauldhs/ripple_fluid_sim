#include <fenv.h>
#include <unistd.h>

#include "app/app.hpp"

int main() {
    // feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);

    AppSpecification app_spec = {.name = "Ripple"};

    App app(app_spec);
    app.run();
}
