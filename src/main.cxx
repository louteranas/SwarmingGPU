
// Agent (particle model)
#include "agent.hxx"

#include "util.hpp"
#include "types.hxx"
#include "parser.hxx"

#include "workspace.hxx"


int run(int argc, char ** argv, bool enableGPU, int agentSize, int steps){
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
  Workspace workspace(parser, enableGPU);
  // Launch simulation
  int nSteps = parser("steps").asInt();
  workspace.simulate(nSteps);
  return 0;
}

// Main class for running the parallel flocking sim
int main(int argc, char **argv) {
  double start_time;
  double run_time;
  util::Timer timer;
  std::cout << " RUNING ON CPU" << std::endl;
  for(int i = 27; i < 1200 ; i += 40){
    timer.reset();
    start_time = static_cast<double>(timer.getTimeMilliseconds()) / 1000.0;
    run(argc, argv, false, i, 1000);
    run_time = (static_cast<double>(timer.getTimeMilliseconds()) / 1000.0) - start_time;
    std::cout << "run time for " << i <<  " agents is : "<< run_time << std::endl;
  }
  
  std::cout << " RUNING ON GPU" << std::endl;
  for(int i = 27; i < 1200 ; i += 40){
    timer.reset();
    start_time = static_cast<double>(timer.getTimeMilliseconds()) / 1000.0;
    run(argc, argv, true, i, 1000);
    run_time = (static_cast<double>(timer.getTimeMilliseconds()) / 1000.0) - start_time;
    std::cout << "run time for " << i <<  " agents is : "<< run_time << std::endl;
  }
  
  return 0;
}

