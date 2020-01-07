/*
*/
#ifndef  WORKSPACE
#define  WORKSPACE

#include "parser.hxx"
#include "types.hxx"


class Workspace
{
protected:
  Container agents;
  voxelsContainer sortedAgents;
  unsigned int na;

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
  void sortAgentsByX();
  void sortAgentsByY();
  void sortAgentsByZ();
  void updateAgentsDeque();
  std::deque<Agent>getNeighborhood(std::deque<unsigned int> indexs, unsigned int sideCount);

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
