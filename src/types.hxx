#ifndef TYPES
#define TYPES

#include <deque>

// Forward declaration
class Agent;

typedef double Real;
typedef std::deque<Agent> Container;
typedef std::deque<std::deque<std::deque<Agent>>> voxelsContainer;

#endif
