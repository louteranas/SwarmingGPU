
// Agent (particle model)
#include "agent.hxx"

#include "types.hxx"
#include "parser.hxx"

#include "workspace.hxx"

// Main class for running the parallel flocking sim
int main(int argc, char **argv) {
  // Create parser
  ArgumentParser parser;

  // Add options to parser
  parser.addOption("agents", 1000);
  parser.addOption("steps", 20000);
  parser.addOption("wc", 7.0);//7.0 1.0);
  parser.addOption("wa", 12.0);// 12.0 1.0);
  parser.addOption("ws", 55.0);// 55.0 1.5);

  parser.addOption("rc", 90);
  parser.addOption("ra", 90);
  parser.addOption("rs", 25);

  // Parse command line arguments
  parser.setOptions(argc, argv);

  // Create workspace
  Workspace workspace(parser);

  // Launch simulation
  int nSteps = parser("steps").asInt();
  workspace.simulate(nSteps);

  return 0;
}
