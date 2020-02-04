
// Agent (particle model)
#include "agent.hxx"

#include "util.hpp"
#include "types.hxx"
#include "parser.hxx"

#include "workspace.hxx"


int run(int argc, char ** argv, bool enableGPU, int agentSize, int steps, int groupeSize){
  // Create parser
  ArgumentParser parser;

  // Add options to parser
  parser.addOption("agents", agentSize);
  parser.addOption("steps", steps);
  parser.addOption("wc", 7.0);//7.0 1.0);
  parser.addOption("wa", 12.0);// 12.0 1.0);
  parser.addOption("ws", 55.0);// 55.0 1.5);

  parser.addOption("rc", 90);
  parser.addOption("ra", 90);
  parser.addOption("rs", 25);

  // Parse command line arguments
  parser.setOptions(argc, argv);
  // Create workspace
  Workspace workspace(parser, enableGPU, groupeSize);
  // Launch simulation
  int nSteps = parser("steps").asInt();
  workspace.simulate(nSteps, groupeSize);
  if(enableGPU){
    std::cout << "total Sub Sorting time is : " << workspace.totalSortTimeGPU/1000.0 << "s" << std::endl;
    std::cout << "total Merging time is : " << workspace.totalmergeTime/1000.0 << "s" << std::endl;
    std::cout << "total Sorting time is : " << (workspace.totalSortTimeGPU + workspace.totalmergeTime)/1000.0 << "s" << std::endl;
    std::cout << "total Neighboor time is : " << workspace.totalNeighboorTimeCPU/1000.0 << "s" << std::endl;
    std::cout << "total Move time is : " << workspace.totalMoveTime/1000.0 << "s" << std::endl;
  }
  else{
    std::cout << "total Sorting time is : " << workspace.totalSortTimeCPU/1000.0 << "s" << std::endl;
    std::cout << "total Neighboors time is : " << workspace.totalNeighboorTimeCPU/1000.0 << "s" << std::endl;
    std::cout << "total Move time is : " << workspace.totalMoveTime/1000.0 << "s" << std::endl;
  }
  return 0;
}


void benchmark(int argc, char **argv){
  double start_time;
  double run_time;
  util::Timer timer;
  for(int i = 27; i < 1200 ; i += 110){
    std::cout << "RUNING ON CPU : number of agents is " << i << " , the details of the run are : \n";
    timer.reset();
    start_time = static_cast<double>(timer.getTimeMilliseconds()) / 1000.0;
    run(argc, argv, false, i, 1000, 32);
    run_time = (static_cast<double>(timer.getTimeMilliseconds()) / 1000.0) - start_time;
    std::cout << "total run time is : " << run_time << "s" << std::endl;
  }

  for(int i = 27; i < 1100 ; i += 110){
    std::cout << "RUNING ON GPU : number of agents is " << i << " , the details of the run are : \n";
    timer.reset();
    start_time = static_cast<double>(timer.getTimeMilliseconds()) / 1000.0;
    run(argc, argv, true, i, 1000, (int)i/4);
    run_time = (static_cast<double>(timer.getTimeMilliseconds()) / 1000.0) - start_time;
    std::cout << "total run time is : " << run_time << "s" << std::endl;
  }

  std::cout << "RUNING ON GPU : number of agents is " << 120 << " , testing the effect of workgroupe sizes : \n";
  for(int i = 2; i < 120 ; i += 5){
    std::cout << "workgroupe size is " << i << " , the details of the run are : \n";
    timer.reset();
    start_time = static_cast<double>(timer.getTimeMilliseconds()) / 1000.0;
    run(argc, argv, true, 120, 1000, i);
    run_time = (static_cast<double>(timer.getTimeMilliseconds()) / 1000.0) - start_time;
    std::cout << "total run time is : " << run_time << "s" << std::endl;
  }
}

// Main class for running the parallel flocking sim
int main(int argc, char **argv) {
  bool bench = false;
  bool enableGPU = false;
  if(bench)
    benchmark(argc, argv);
  else
    run(argc, argv, enableGPU, 127, 500, 32);
  
  return 0;
}

