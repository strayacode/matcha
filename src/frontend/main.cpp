#include <memory>
#include "host_interface.h"

int main() {
    std::unique_ptr<HostInterface> host_interface = std::make_unique<HostInterface>();

    if (host_interface->initialise()) {
        host_interface->run();
    }

    host_interface->shutdown();

    return 0;
}