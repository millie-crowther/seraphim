#include "core/scheduler.h"

#include <queue>

std::priority_queue<scheduler::task_t, std::vector<scheduler::task_t>, scheduler::task_t::comparator_t> task_queue;