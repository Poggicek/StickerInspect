#include "timer.h"
#include <vector>

std::vector<NextTickFn> g_vecNextTick;

void NextTick(NextTickFn fn)
{
	g_vecNextTick.push_back(fn);
}