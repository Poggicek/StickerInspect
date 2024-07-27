#pragma once

#include <functional>

typedef std::function<void()> NextTickFn;
void NextTick(NextTickFn fn);
extern std::vector<NextTickFn> g_vecNextTick;