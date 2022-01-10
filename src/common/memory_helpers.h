#pragma once

#define in_range(lower_bound, upper_bound, addr) (addr >= (unsigned)lower_bound) && (addr < (unsigned)upper_bound)