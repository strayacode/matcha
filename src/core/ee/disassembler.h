#pragma once

#include <string>
#include "common/types.h"
#include "core/ee/context.h"

namespace ee {

std::string DisassembleInstruction(Instruction inst, u32 pc);
std::string GetRegisterName(int reg);

} // namespace ee