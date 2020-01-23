#ifndef TYPES
#define TYPES

#include <vector>
#include <deque>

// Forward declaration
class Agent;

typedef float Real;
typedef std::deque<Agent> Container;
typedef std::deque<std::deque<std::deque<Agent>>> voxelsContainer;
typedef std::vector<float> gpuContainer;

#endif
