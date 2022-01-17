add_library(core
    core.h core.cpp
    system.h system.cpp
    scheduler.h scheduler.cpp

    ee/ee_core.h ee/ee_core.cpp
    ee/cop0.h ee/cop0.cpp
    ee/cop1.h ee/cop1.cpp
    ee/disassembler.h ee/disassembler.cpp
    ee/ee_interpreter.h ee/ee_interpreter.cpp
    ee/interpreter_table.h ee/interpreter_table.cpp

    ee/intc.h ee/intc.cpp
    ee/timers.h ee/timers.cpp
    ee/dmac.h ee/dmac.cpp

    iop/cpu_core.h iop/cpu_core.cpp
    iop/cpu_regs.h
    iop/interpreter/interpreter.h iop/interpreter/interpreter.cpp
    iop/disassembler.h iop/disassembler.cpp
    iop/cop0.h iop/cop0.cpp
    iop/interpreter/instructions/instruction_cop0.cpp
    iop/interpreter/instructions/instruction_secondary.cpp
    iop/interpreter/instructions/instruction_primary.cpp
    iop/dmac.h iop/dmac.cpp
    iop/interrupt_controller.h iop/interrupt_controller.cpp
    iop/timers.h iop/timers.cpp

    memory/memory.h memory/memory.cpp
    memory/memory_constants.h

    gif/gif.h gif/gif.cpp

    gs/gs.h gs/gs.cpp

    vu/vu.h vu/vu.cpp

    vif/vif.h vif/vif.cpp

    ipu/ipu.h ipu/ipu.cpp

    sif/sif.h sif/sif.cpp

    elf_loader.h elf_loader.cpp
)

include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(core PRIVATE common)