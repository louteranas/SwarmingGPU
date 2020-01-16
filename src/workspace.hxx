/*
*/
#ifndef  WORKSPACE
#define  WORKSPACE




#include "vector.hxx"
#include "agent.hxx"
#include "parser.hxx"
#include "types.hxx"


class Workspace
{
protected:
  Container agents;
  voxelsContainer sortedAgents;
  unsigned int na;
  //A initialiser
  unsigned int sideCount = 5;

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


  Real domainsize;
  void init();
  void sortAgents();
  void sortAgentsGpu();
  void sortAgentsByX();
  void sortAgentsByY();
  void sortAgentsByZ();
  gpuContainer convertAgents();
  void mergeLists(unsigned int startIndex1, unsigned int size1, unsigned int startIndex2, unsigned int size2, unsigned int coord);
  void getNeighborhood(uint index);

public:
  Workspace(ArgumentParser &parser);

  Workspace(size_t nAgents,
  Real wc, Real wa, Real ws,
  Real rc, Real ra, Real rs);
  void move();
  void simulate(int nsteps);
  void save(int stepid);
};

#endif
