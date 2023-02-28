#pragma once

#include <string>
#include "common/types.h"
#include "common/log.h"
#include "core/iop/instruction.h"

namespace iop {

std::string DisassembleInstruction(Instruction inst, u32 pc);
std::string GetRegisterName(int reg);

} // namespace iop