add_library(core
    core.h core.cpp
    system.h system.cpp
    scheduler.h scheduler.cpp

    ee/context.h ee/context.cpp
    ee/cop0.h ee/cop0.cpp
    ee/cop1.h ee/cop1.cpp
    ee/disassembler.h ee/disassembler.cpp
    ee/interpreter.h ee/interpreter.cpp
    ee/decoder.h ee/executor.h
    ee/instruction.h
    ee/intc.h ee/intc.cpp
    ee/timers.h ee/timers.cpp
    ee/dmac.h ee/dmac.cpp

    iop/context.h iop/context.cpp
    iop/interpreter.h iop/interpreter.cpp
    iop/disassembler.h iop/disassembler.cpp
    iop/cop0.h iop/cop0.cpp
    iop/dmac.h iop/dmac.cpp
    iop/intc.h iop/intc.cpp
    iop/timers.h iop/timers.cpp
    iop/cdvd.h iop/cdvd.cpp
    iop/sio2.h iop/sio2.cpp
    iop/executor.h
    iop/instruction.h

    gif.h gif.cpp

    gs/context.h gs/context.cpp

    vu/vu.h vu/vu.cpp

    vif/vif.h vif/vif.cpp

    ipu/ipu.h ipu/ipu.cpp

    sif/sif.h sif/sif.cpp

    elf_loader.h elf_loader.cpp

    spu/spu.h spu/spu.cpp
)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PRIVATE common)