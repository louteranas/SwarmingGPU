#include <cmath>
#include <iostream>
#include <fstream>
#include <ctime>
#include <math.h>

#include "agent.hxx"
#include "vector.hxx"
#include "workspace.hxx"

Workspace::Workspace(ArgumentParser &parser)
{

  na = parser("agents").asInt();

  wCohesion = parser("wc").asDouble();
  wAlignment = parser("wa").asDouble();
  wSeparation = parser("ws").asDouble();

  rCohesion = parser("rc").asDouble();
  rAlignment = parser("ra").asDouble();
  rSeparation = parser("rs").asDouble();
  dt= 0.01;
  max_speed = 20.0;
  max_force = 80.0;
  time = 0.,

  this->init();
  this->sortAgents();}

Workspace::Workspace(size_t nAgents,
             Real wc, Real wa, Real ws,
             Real rc, Real ra, Real rs) :
             na(nAgents), dt(.05), time(0),
             wCohesion(wc), wAlignment(wa), wSeparation(ws),
             rCohesion(rc), rAlignment(ra), rSeparation(rs),
             max_speed(20.), max_force(80.)
{ this->init();
  this->sortAgents();}

void Workspace::init(){
  lx = 800.0;
  ly = 800.0;
  lz = 800.0;

  padding = 0.02 * lx;
  // Random generator seed
  srand48(std::time(0));

  // Initialize agents
  // This loop may be quite expensive due to random number generation
  for(size_t j = 0; j < na; j++){

    // Create random position
    //Vector position(lx*(0.02 + drand48()), ly*(0.02 + drand48()), lz*(0.02 + drand48()));
    Vector position(lx*(0.02 + drand48()), ly*(0.02 + drand48()), lz*(0.02 + drand48()));
    Vector velocity(160 * (drand48() - 0.5), 160*(drand48()- 0.5), 160*(drand48() - 0.5));

    // Create random velocity
    agents.push_back(Agent(position, velocity, Zeros()));
    agents.back().max_force = max_force;
    agents.back().max_speed = max_speed;
    agents.back().ra = rAlignment;
    agents.back().rc = rCohesion;
    agents.back().rs = rSeparation;

    // std::cout << position.x << std::endl;

  }
}

std::deque<Agent> sortList(std::deque<Agent> unsortedAgents, unsigned int index){
  std::deque<Agent> agents = unsortedAgents;
  Agent temp = agents.at(0);
  for(size_t j = agents.size()-1; j > 0; j--){
    for(size_t k = 0; k<j ; k++){
      if(agents.at(k).position[index] > agents.at(k+1).position[index]){
        temp = agents.at(k);
        agents.at(k) = agents.at(k+1);
        agents.at(k+1) = temp;
      }
    }
  }
  return agents;
}

std::deque<Agent> createDeque(std::deque<std::deque<Agent>> agentsPlan){
  std::deque<Agent> agents;
  for(size_t i = 0; i < agentsPlan.size(); i++){
    for(size_t j = 0; j< agentsPlan.at(i).size(); j++){
      agents.push_back(agentsPlan.at(i).at(j));
    }
  }
  return agents;
}

void printDeque(std::deque<Agent> agents, unsigned int index){
  for(size_t i = 0; i < agents.size(); i++){
    std::cout << agents.at(i).position[index] << std::endl;
  }
}

void Workspace::sortAgentsByX(){
  // std::cout << pow(na, 1.0/3.0) << std::endl;
  // Sorting agents by X value
  agents = sortList(agents, 0);

  // creating the voxel container witch defines the ZY planes
  unsigned int sideSize = pow(na, 1.0/3.0);
  int counter = 0;
  for(size_t i = 0; i < sideSize; i++){
    std::deque<std::deque<Agent>> agentsZY;
    for(size_t j = 0; j < sideSize; j++){
      Container agentsY;
      for(size_t k = 0; k < sideSize; k++){
        agentsY.push_back(agents.at(counter));
        counter++;
      }
      agentsZY.push_back(agentsY);
    }
    sortedAgents.push_back(agentsZY);
  }

  // for debug
  // for(size_t i = 0; i < sortedAgents.size(); i++){
  //   for(size_t j = 0; j < sortedAgents.at(i).size(); j++){
  //     printDeque(sortedAgents.at(i).at(j), 0);
  //   }
  // }

}

void Workspace::sortAgentsByY(){

  // creating the voxel container witch defines the ZY planes
  for(size_t i = 0; i < sortedAgents.size(); i++){
    std::deque<Agent> agentsPlan = createDeque(sortedAgents.at(i));
    agentsPlan = sortList(agentsPlan, 1);
    int counter = 0;
    for(size_t j = 0; j < sortedAgents.at(i).size(); j++){
      for(size_t k = 0; k < sortedAgents.at(i).at(j).size(); k++){
        sortedAgents.at(i).at(j).at(k) = agentsPlan.at(counter);
        counter++;
      }
    }
  }

  // for debug
  // for(size_t i = 0; i < sortedAgents.size(); i++){
  //     std::cout << "Here is the plan "<< i << std::endl;
  //   for(size_t j = 0; j < sortedAgents.at(i).size(); j++){
  //     printDeque(sortedAgents.at(i).at(j), 1);
  //   }
  // }

}

void Workspace::sortAgentsByZ(){

  // creating the voxel container witch defines the ZY planes
  for(size_t i = 0; i < sortedAgents.size(); i++){
    for(size_t j = 0; j < sortedAgents.at(i).size(); j++){
      for(size_t k = 0; k < sortedAgents.at(i).at(j).size(); k++){
        sortedAgents.at(i).at(j) = sortList(sortedAgents.at(i).at(j), 2);
      }
    }
  }

  // for debug
  // for(size_t i = 0; i < sortedAgents.size(); i++){
  //     std::cout << "Here is the plan X "<< i << std::endl;
  //   for(size_t j = 0; j < sortedAgents.at(i).size(); j++){
  //     std::cout << "Here is the line Y "<< j << std::endl;
  //     printDeque(sortedAgents.at(i).at(j), 2);
  //   }
  // }

}

void Workspace::sortAgents(){
  // we first sort by X to create YZ planes
  this->sortAgentsByX();
  // we then sort by Y to create lines
  this->sortAgentsByY();
  // we finally sort by Z to create voxels
  this->sortAgentsByZ();
  
  this->updateAgentsDeque();
  // std::deque<unsigned int> indexs;
  // indexs.push_back(0);
  // indexs.push_back(0);
  // indexs.push_back(0);
  // getNeighborhood(indexs, 5);

  for (uint i = 0; i < agents.size(); i++){
    getNeighborhood(i);
  }
}

void Workspace::getNeighborhood(uint index){

  agents.at(index).neighbors.clear();
  int radius = sideCount/2;
  // std::cout << sideCount << " is bigger than " << pow(na, 1.0/3.0) << " ? " << std::endl;
  // std::cout << ( pow(sideCount,3) > na) << " ? " << std::endl;
  // std::cout << ( sideCount > pow(na, 1.0/3.0)) << " ? " << std::endl;
  // we compare sideCount^3 with na instade of sideCount with square cube of na becuare it causes float issues
  if( pow(sideCount,3) > na){
    perror("Nighborhood radius is too big, try a smaller one");
    return;
  }

  int cote = pow(na, 1./3.);
  for (int z = - radius; z < radius + 1; z++){
    for (int y = - radius; z < radius + 1; y++){
      for (int x = - radius; x < radius + 1; x++){
        agents.at(index).neighbors.push_back(agents.at(((x * cote * cote) + (y * cote) + z) % na);
      }
    }
  }
  //std::cout << "la taille des voisins est " << neighbors.size()<<std::endl;
}

void Workspace::updateAgentsDeque(){
  std::deque<Agent> NewAgents;
  for(size_t i = 0; i < sortedAgents.size(); i++){
    for(size_t j = 0; j < sortedAgents.at(i).size(); j++){
      for(size_t k = 0; k < sortedAgents.at(i).at(j).size(); k++){
        NewAgents.push_back(sortedAgents.at(i).at(j).at(k));
      }
    }
  }
  agents = NewAgents;
}

void Workspace::move()
{

    // Compute forces applied on specific agent
    for(size_t k = 0; k< na; k++){

      agents[k].compute_force(agents, k, rCohesion);


      agents[k].direction = agents[k].cohesion*wCohesion
        + agents[k].alignment*wAlignment
        + agents[k].separation*wSeparation;
    }



    // Time integraion using euler method
    for(size_t k = 0; k< na; k++){
      agents[k].velocity += dt*agents[k].direction;

      double speed = agents[k].velocity.norm()/max_speed;
      if (speed > 1. && speed>0.) {
        agents[k].velocity /= speed;
      }
      agents[k].position += dt*agents[k].velocity;

      if(agents[k].position.x <40)
        agents[k].position.x = lx - 40;
      if(agents[k].position.x >lx - 40)
        agents[k].position.x = 40;
      if(agents[k].position.y <40)
        agents[k].position.y = ly - 40;
      if(agents[k].position.y >ly - 40)
        agents[k].position.y = 40;
      if(agents[k].position.z <40)
        agents[k].position.z = lz - 40;
      if(agents[k].position.z >lz - 40)
        agents[k].position.z = 40;

    }


}

void Workspace::move_sorted(){


  for(size_t i = 0; i < sortedAgents.size(); i++){
    for(size_t j = 0; j < sortedAgents.at(i).size(); j++){
      for(size_t k = 0; k < sortedAgents.at(i).at(j).size(); k++){
        std::deque<unsigned int> index = {(unsigned int) i, (unsigned int) j, (unsigned int) k};
        //A mettre juste après le tri
        //Container neighbors = getNeighborhood(index);
        sortedAgents.at(i).at(j).at(k).compute_force_sorted();
        sortedAgents.at(i).at(j).at(k).direction = sortedAgents.at(i).at(j).at(k).cohesion*wCohesion
      + sortedAgents.at(i).at(j).at(k).alignment*wAlignment
      + sortedAgents.at(i).at(j).at(k).separation*wSeparation;

      }
    }
  }


  for(size_t i = 0; i < sortedAgents.size(); i++){
    for(size_t j = 0; j < sortedAgents.at(i).size(); j++){
      for(size_t k = 0; k < sortedAgents.at(i).at(j).size(); k++){
        sortedAgents.at(i).at(j).at(k).velocity += dt*sortedAgents.at(i).at(j).at(k).direction;

        double speed = sortedAgents.at(i).at(j).at(k).velocity.norm()/max_speed;
        if (speed > 1. && speed>0.) {
          sortedAgents.at(i).at(j).at(k).velocity /= speed;
        }
        sortedAgents.at(i).at(j).at(k).position += dt*sortedAgents.at(i).at(j).at(k).velocity;

        if(sortedAgents.at(i).at(j).at(k).position.x <40)
          sortedAgents.at(i).at(j).at(k).position.x = lx - 40;
        if(sortedAgents.at(i).at(j).at(k).position.x >lx - 40)
          sortedAgents.at(i).at(j).at(k).position.x = 40;
        if(sortedAgents.at(i).at(j).at(k).position.y <40)
          sortedAgents.at(i).at(j).at(k).position.y = ly - 40;
        if(sortedAgents.at(i).at(j).at(k).position.y >ly - 40)
          sortedAgents.at(i).at(j).at(k).position.y = 40;
        if(sortedAgents.at(i).at(j).at(k).position.z <40)
          sortedAgents.at(i).at(j).at(k).position.z = lz - 40;
        if(sortedAgents.at(i).at(j).at(k).position.z >lz - 40)
          sortedAgents.at(i).at(j).at(k).position.z = 40;

      }
    }
  }
  this->updateAgentsDeque();
}


void Workspace::simulate(int nsteps) {
  // store initial positions
    save(0);
    // perform nsteps time steps of the simulation
    int step = 0;
    this->sortAgents();
    while (step++ < nsteps){
      this->move_sorted();
      if (step % 20 ==0){
        save(step);
        this->sortAgents();
      }
      // store every 20 steps
/*
      if (step%20 == 0){
        this->sortAgents();
        save(step);
      }*/

    }
}

void Workspace::save(int stepid) {
  std::ofstream myfile;
  std::ofstream myfile2;

  myfile.open("boids.xyz", stepid==0 ? std::ios::out : std::ios::app);

    myfile << std::endl;
    myfile << na << std::endl;
    for (size_t p=0; p<na; p++)
        myfile << "B " << agents[p].position;

    myfile.close();

  // visual debgging

  // updateAgentsDeque();
  // myfile2.open("boidsModified.xyz", stepid==0 ? std::ios::out : std::ios::app);

  //   myfile2 << std::endl;
  //   myfile2 << pow(na, 1.0/3.0) << std::endl;
  //   for (size_t p=0; p<pow(na, 1.0/3.0)/*na*/; p++)
  //       myfile2 << "B " << agents[p].position;

  //   myfile2.close();
}
