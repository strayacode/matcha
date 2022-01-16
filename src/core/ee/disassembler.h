#pragma once

#include <string>
#include <array>
#include <sstream>
#include <iomanip>
#include <map>
#include "common/cpu_types.h"
#include "common/types.h"
#include "common/log.h"

std::string EEDisassembleInstruction(CPUInstruction inst, u32 pc);
std::string EEGetRegisterName(int reg);