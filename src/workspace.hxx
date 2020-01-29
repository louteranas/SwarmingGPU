/*
*/
#ifndef  WORKSPACE
#define  WORKSPACE



#include "cl.hpp"

#include "vector.hxx"
#include "agent.hxx"
#include "parser.hxx"
#include "types.hxx"


class Workspace
{
protected:
  Container agents;
  std::vector<cl::Device> chosen_device;
  cl::Device device;
  voxelsContainer sortedAgents;
  unsigned int na;
  //A initialiser
  unsigned int sideCount = 3;

  Real dt;
  int time;

  Real wCohesion, wAlignment, wSeparation;
  Real rCohesion, rAlignment, rSeparation;
  Real maxU;

  Real max_speed;
  Real max_force;

  Real tUpload, tDownload, tCohesion, tAlignment, tSeparation;

  // Size of the domain
  Real lx, ly, lz;

  // Lower bound of the domain
  Real xmin, ymin, zmin;

  // Padding around the domain
  Real padding;

  bool enableGPU;

  std::deque<int> listTotaleVoisinsIndex;

  Real domainsize;
  void init();
  void sortAgents();
  std::vector<int> sortGPU(int groupeSize, std::vector<float> &agentsX, int startIndex, int endIndex);
  void sortAgentsByComponentGPU(int coordIndex, int startIndex, int endIndex);
  void sortAgentsGPU();
  void sortAgentsByX();
  void sortAgentsByY();
  void sortAgentsByZ();
  gpuContainer convertAgents(int index);
  void mergeLists(unsigned int startIndex1, unsigned int size1, unsigned int startIndex2, unsigned int size2, unsigned int coord);
  void getNeighborhood(uint index);
  void bubble_sort_GPU(std::vector<float> &h_agents, std::vector<int> &indexAgents);
  void getNeighborhoodGPU();
  
public:
  Workspace(ArgumentParser &parser, bool enable);

  double start_time;
  double run_time;
  double totalSortTimeGPU;
  double totalSortTimeCPU;
  double totalmergeTime;
  double totalNeighboorTimeGPU;
  double totalNeighboorTimeCPU;
  util::Timer timer;
  Workspace(size_t nAgents,
  Real wc, Real wa, Real ws,
  Real rc, Real ra, Real rs, bool enable);
  void move();
  void mergeCPU(std::vector<float> &list1, std::vector<float> list2, std::vector<int> &index1, std::vector<int> index2);
  void simulate(int nsteps);
  void save(int stepid);
};

#endif
