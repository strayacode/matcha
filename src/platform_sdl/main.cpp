#include "host_interface.h"

int main(int argc, char *argv[]) {
    log_debug("otterstation");

    if (argc != 2) {
        log_fatal("incorrect number of arguments");
    }

    std::unique_ptr<HostInterface> host_interface = std::make_unique<HostInterface>();

    if (host_interface->Initialise()) {
        host_interface->Run(argv[1]);
    }

    host_interface->Shutdown();

    return 0;
}