#include <cmath>
#include <iostream>
#include <fstream>
#include <ctime>
#include <math.h>



#include "cl.hpp"
#include "util.hpp"
#include "device_picker.hpp"
#include "err_code.h"

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
  time = 0.;
  this->init();
  
  this->sortAgents();
}

Workspace::Workspace(size_t nAgents,
             Real wc, Real wa, Real ws,
             Real rc, Real ra, Real rs) :
             na(nAgents), dt(.05), time(0),
             wCohesion(wc), wAlignment(wa), wSeparation(ws),
             rCohesion(rc), rAlignment(ra), rSeparation(rs),
             max_speed(20.), max_force(80.)
{   this->init();
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
  // Choosing GPU device
  cl_uint deviceIndex = 0;
  std::vector<cl::Device> devices; 
  unsigned numDevices = getDeviceList(devices);
  device = devices[deviceIndex];
  std::string name;
  getDeviceName(device, name);
  std::cout << "\nUsing OpenCL device: " << name << "\n";
  chosen_device.push_back(device);
}

void sortList(std::deque<Agent> &unsortedAgents, unsigned int coord, unsigned int startIndex, unsigned int endIndex){
  Agent temp = unsortedAgents.at(startIndex);
  for(size_t j = endIndex-1; j > startIndex; j--){
    for(size_t k = startIndex; k<j ; k++){
      if(unsortedAgents.at(k).position[coord] > unsortedAgents.at(k+1).position[coord]){
        temp = unsortedAgents.at(k);
        unsortedAgents.at(k) = unsortedAgents.at(k+1);
        unsortedAgents.at(k+1) = temp;
      }
    }
  }
}

void Workspace::mergeLists(unsigned int startIndex1, unsigned int size1, unsigned int startIndex2, unsigned int size2, unsigned int coord){
  uint index1 = startIndex1;
  uint index2 = startIndex2;
  while(index1 < startIndex1 + size1 && index2 < startIndex2 + size2){
    if(agents.at(index1).position[coord] > agents.at(index2).position[coord]){
      Agent temp = agents.at(index1);
      agents.at(index1) = agents.at(index2);
      agents.at(index2) = temp;
      index1++;
    }
    else{
      index2++;
    }
  }
}

//Rajouter un vecteur après qui contiendra tous les index
gpuContainer Workspace::convertAgents(int index){
  // std::vector<float> temp;
  gpuContainer gpuAgents;
  for(uint i = 0; i<agents.size(); i++){
    gpuAgents.push_back(agents.at(i).position[index]);
    // temp.push_back(agents.at(i).position[1]);
    // temp.push_back(agents.at(i).position[2]);
    // temp.push_back((float)i);
    // gpuAgents.push_back(temp);
    // temp.clear();
  }
  return gpuAgents;
}

std::deque<Agent> createDeque(unsigned int startIndex, unsigned int endIndex){
  std::deque<Agent> output;
  /*for(size_t i = 0; i < agentsPlan.size(); i++){
    for(size_t j = 0; j< agentsPlan.at(i).size(); j++){
      output.push_back(agentsPlan.at(i).at(j));
    }
  }
  */
  return output;
}

void printDeque(std::deque<Agent> agents, unsigned int index){
  for(size_t i = 0; i < agents.size(); i++){
    std::cout << agents.at(i).position[index] << std::endl;
  }
}

void Workspace::sortAgentsByX(){
  // std::cout << pow(na, 1.0/3.0) << std::endl;
  // Sorting agents by X value
  sortList(agents, 0, 0, agents.size());

}

void Workspace::sortAgentsByY(){

  // creating the voxel container witch defines the ZY planes
  unsigned int numberOfIterations = pow(na, 1.0/3.0);  //2
  unsigned int numberOfPlanAgents = pow(numberOfIterations, 2); //4

  for(unsigned int i = 0; i < numberOfIterations; i++){
    sortList(agents, 1, i * numberOfPlanAgents, (i+1) * numberOfPlanAgents);
  }

}

void Workspace::sortAgentsByZ(){
  // creating the voxel container witch defines the ZY planes
  unsigned int  numberOfLineAgents = pow(na, 1.0/3.0);
  unsigned int numberOfIterations = pow(numberOfLineAgents, 2);

  for(unsigned int i = 0; i < numberOfIterations; i++){
    sortList(agents, 2, i * numberOfLineAgents, (i+1) * numberOfLineAgents);
  }

  bool wrongCase = true;
  for(unsigned int i = 0; i < numberOfIterations; i++){
    for(unsigned int k = 0; k < numberOfLineAgents - 1; k++){
      wrongCase = wrongCase && (agents.at((i * numberOfLineAgents) + k).position[2] < agents.at((i * numberOfLineAgents) + (k+1)).position[2]);
    }
  }


}


void Workspace::sortAgents(){
    /*
  // we first sort by X to create YZ planes
  this->sortAgentsByX();
  // we then sort by Y to create lines
  this->sortAgentsByY();
  // we finally sort by Z to create voxels
  this->sortAgentsByZ();
  for (uint i = 0; i < agents.size(); i++){
    getNeighborhood(i);
  }
  */
}

void Workspace::mergeCPU(std::vector<float> &list1, std::vector<float> list2, std::vector<int> &index1, std::vector<int> index2){
  std::vector<float> output;
  std::vector<int> ouputIndex;
  int iterator1 = 0;
  int iterator2 = 0;
  std::cout << "MERGING LES PUTAINS DE LISTE : " << list1.size() << " | " << list2.size() << std::endl;
  if(list2.size() == 0)
    return;
  if(list1.size() == 0){
    list1.assign(list2.begin(), list2.end());
    index1.assign(index2.begin(), index2.end());
    return;
  }
  while(iterator1<list1.size() && iterator2 < list2.size()){
    if(list1[iterator1] < list2[iterator2]){
      output.push_back(list1[iterator1]);
      ouputIndex.push_back(index1[iterator1]);
      iterator1++;
    }
    else{
      output.push_back(list2[iterator2]);
      ouputIndex.push_back(index2[iterator2]);
      iterator2++;
    }
  }
  if(iterator1 < list1.size()){
    for(int i = iterator1; i < list1.size(); i++){
      output.push_back(list1[i]);
      ouputIndex.push_back(index1[i]);
    }
  }
  if(iterator2 < list2.size()){
    for(int i = iterator2; i < list2.size(); i++){
      ouputIndex.push_back(index2[i]);
    }
  }
  list1.assign(output.begin(), output.end());
  index1.assign(ouputIndex.begin(), ouputIndex.end());
}


void Workspace::bubble_sort_GPU(std::vector<float> &h_agents, std::vector<int> &indexAgents ){
  int startIndex = 0;
  float tempAgents = h_agents.at(startIndex);
  float tempIndex = indexAgents.at(startIndex);
  int endIndex = h_agents.size();

  for(size_t j = endIndex-1; j > startIndex; j--){
    for(size_t k = startIndex; k<j ; k++){
      if(h_agents[k] > h_agents[k + 1]){
        tempAgents = h_agents[k];
        h_agents[k] = h_agents[k+1];
        h_agents[k+1] = tempAgents;
        tempIndex = indexAgents[k];
        indexAgents[k] = indexAgents[k+1];
        indexAgents[k+1] = tempIndex;        
      }
    }
  }

}



void Workspace::sortAgentsGpu(int groupeSize, std::vector<float> &agentsProjection, int startIndex, int endIndex){
  
  //Init param

  //Initialisation des listes
  
  //Initialisation de la liste d'index  
  std::vector<float> listIndex;
  for (int j = startIndex; j < endIndex; j++){
    listIndex.push_back(j);
  }


  for (int j = 0; j < agentsProjection.size(); j++){
      std::cout <<  listIndex[j]<<" : " << agentsProjection[j] <<" | "  ;
  }
   std::cout << std::endl;
  

  //Create the globalGroup as a multiple of the workgroup
  const int globalSize = ((endIndex-startIndex) / groupeSize) * groupeSize;
  std::vector<float> h_X_rest(agentsProjection.begin() + globalSize, agentsProjection.end());
  std::vector<float> h_X(agentsProjection.begin(), agentsProjection.begin() + globalSize);
  
  std::vector<int> h_index_rest(listIndex.begin() + globalSize, listIndex.end());
  std::vector<int> h_index(listIndex.begin(), listIndex.begin() + globalSize);
  if (h_X_rest.size() > 1){
    bubble_sort_GPU(h_X_rest, h_index_rest);
  }


  cl::Buffer d_X;
  cl::Buffer d_index;

  // Load in kernel source, creating a program object for the context
  cl::Context context(chosen_device);
  cl::CommandQueue queue(context, device);
  //Kernel for sorting with a param

  cl::Program program(context, util::loadProgram("bubble_sort.cl"), true);

  d_X = cl::Buffer(context, h_X.begin(), h_X.end(), CL_MEM_READ_WRITE, true);
  d_index = cl::Buffer(context, h_index.begin(), h_index.end(), CL_MEM_READ_WRITE, true);
  
  cl::Kernel kernel_sort = cl::Kernel(program, "sortList");
  kernel_sort.setArg(0, d_X);
  kernel_sort.setArg(1, d_index);
  kernel_sort.setArg(2, globalSize);
  kernel_sort.setArg(3, groupeSize);
  
  cl::NDRange global(globalSize);
  cl::NDRange local(groupeSize);

  queue.enqueueNDRangeKernel(kernel_sort, cl::NullRange, global, local);
  queue.finish();
  cl::copy(queue, d_X, h_X.begin(), h_X.end());
  cl::copy(queue, d_index, h_index.begin(), h_index.end());

  //==================================================================
  //______________Kernel for merging the things__________
  cl::Program programMerge(context, util::loadProgram("merging.cl"), true);
  
  
  groupeSize *= 2;
  int tempSize = (globalSize / groupeSize) * groupeSize;
  int counter = 0;
  std::vector<float> tempAgents(h_X.begin(), h_X.end());
  std::vector<float> tempAgents_reste(h_X_rest.begin(), h_X_rest.end());
  std::cout << tempAgents_reste.size()<< std::endl;

  std::vector<int> tempIndex(h_index.begin(), h_index.end());
  std::vector<int> tempIndex_reste(h_index_rest.begin(), h_index_rest.end());

  while(tempSize/groupeSize > 0){
    //Split the vector into 2 of the good size
    std::vector<float> mergeX(tempAgents.begin(), tempAgents.begin() + tempSize);
    std::vector<float> mergeX_reste(tempAgents.begin() + tempSize, tempAgents.end());

    std::vector<int> mergeIndex(tempIndex.begin(), tempIndex.begin() + tempSize);
    std::vector<int> mergeIndex_rest(tempIndex.begin() + tempSize, tempIndex.end());

    d_X = cl::Buffer(context, mergeX.begin(), mergeX.end(), CL_MEM_READ_WRITE, true);
    d_index = cl::Buffer(context, mergeIndex.begin(), mergeIndex.end(), CL_MEM_READ_WRITE, true);

    cl::Kernel kernel_merge = cl::Kernel(programMerge, "mergeList");
    kernel_merge.setArg(0, d_X);
    kernel_merge.setArg(1, d_index);
    kernel_merge.setArg(2, tempSize);
    kernel_merge.setArg(3, groupeSize);

    global = cl::NDRange(tempSize);
    local = cl::NDRange(groupeSize);

    queue.enqueueNDRangeKernel(kernel_merge, cl::NullRange, global, local);
    queue.finish();
    cl::copy(queue, d_X, mergeX.begin(), mergeX.end());
    cl::copy(queue, d_index, mergeIndex.begin(), mergeIndex.end());

    tempAgents.assign(mergeX.begin(), mergeX.end());

    mergeCPU(tempAgents_reste, mergeX_reste, tempIndex_reste, mergeIndex_rest);

    tempIndex.assign(mergeIndex.begin(), mergeIndex.end());

    groupeSize *= 2;
    tempSize = (globalSize / groupeSize) * groupeSize;

  }


  mergeCPU(tempAgents, tempAgents_reste, tempIndex, tempIndex_reste);

  for (int j = 0; j < tempAgents.size(); j++){
      std::cout <<  tempIndex[j] << " : " << tempAgents[j]<< " | ";
  }
   std::cout << std::endl;
  for (int j = 0; j < tempIndex.size(); j++){
      std::cout <<  tempIndex[j]<<" ";
  }
   std::cout << std::endl;


}







void Workspace::getNeighborhood(uint index){
  //agents.at(index).neighbors.clear();
  int radius = sideCount/2;
  // std::cout << sideCount << " is bigger than " << pow(na, 1.0/3.0) << " ? " << std::endl;
  // std::cout << ( pow(sideCount,3) > na) << " ? " << std::endl;
  // std::cout << ( sideCount > pow(na, 1.0/3.0)) << " ? " << std::endl;
  // we compare sideCount^3 with na instade of sideCount with square cube of na becuare it causes float issues
  if( pow(sideCount,3) > na){
    perror("Neighborhood radius is too big, try a smaller one");
    return;
  }
  int cote = pow(na, 1./3.);
  int currentZ = index % cote;
  int tempIndex = index / cote;
  int currentY = tempIndex % cote;
  tempIndex = tempIndex / cote;
  int currentX = tempIndex % cote; 
  int i = 0;
  if (agents.at(index).neighbors.size() > 0){
    for (int z = - radius; z <(int) radius + 1; z++){
      for (int y = - radius; y < (int) radius + 1; y++){
        for (int x = - radius; x < (int)radius + 1; x++){
          if (x != 0 || y != 0 || z != 0){
          agents.at(index).neighbors.at(i) = (( currentX + x) * cote * cote
           + (currentY + y)  * cote + ( currentZ + z)) % na;
          }
        }
      }
    }
  }
  else{
    for (int z = - radius; z <(int) radius + 1; z++){
      for (int y = - radius; y < (int) radius + 1; y++){
        for (int x = - radius; x < (int)radius + 1; x++){
          if (x != 0 || y != 0 || z != 0){
            agents.at(index).neighbors.push_back((( currentX + x) * cote * cote
              + (currentY + y)  * cote + ( currentZ + z) )% na);
          }
        }
      }
    }
  }


}

void Workspace::move()
{

    // Compute forces applied on specific agent
    for(size_t k = 0; k< na; k++){
      agents[k].compute_force_sorted(agents);


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

void Workspace::simulate(int nsteps) {
  // store initial positions
    save(0);
    std::vector<float> agentsProjection = convertAgents(0);
    sortAgentsGpu(4, agentsProjection, 0, agentsProjection.size());
    // sortAgentsGpu((uint) agents.size(), 6);
/*
    // perform nsteps time steps of the simulation
    int step = 0;
    std::cout << " starting the mouvement " << std::endl;
    this->sortAgents();
    std::cout << " First Sort Done ! " << std::endl;
    
    while (step++ < nsteps){
      this->move();
       if (step % 100 == 0){
         this->sortAgents();
         std::cout << step << std::endl;
       }
      if (step % 20 == 0){
        //this->sortAgents();
        save(step);
      }
      //if (step %100 == 0){
      //  std::cout << step << std::endl;
     // }


    }
    */
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

}
