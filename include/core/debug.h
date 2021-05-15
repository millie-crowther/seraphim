#ifndef SERAPHIM_CORE_DEBUG
#define SERAPHIM_CORE_DEBUG

#include <stdio.h>
#include <stdlib.h>

#define SERAPHIM_DEBUG true

#define PANIC(message) do { printf(message); exit(1); } while (0)

#endif