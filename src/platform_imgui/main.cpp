#include <memory>
#include "host_interface.h"

int main() {
    std::unique_ptr<HostInterface> host_interface = std::make_unique<HostInterface>();

    if (host_interface->Initialise()) {
        host_interface->Run();
    }

    host_interface->Shutdown();

    return 0;
}