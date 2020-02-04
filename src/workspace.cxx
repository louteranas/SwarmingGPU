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


Workspace::Workspace(ArgumentParser &parser, bool enable, int groupeSize)
{
  enableGPU = enable;
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
  
  this->sortAgents(groupeSize);
}

Workspace::Workspace(size_t nAgents,
             Real wc, Real wa, Real ws,
             Real rc, Real ra, Real rs, bool enableGPU = false) :
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

  totalSortTimeGPU = 0;
  totalSortTimeCPU = 0;
  totalmergeTime = 0;
  totalNeighboorTimeGPU = 0;
  totalNeighboorTimeCPU = 0;
  totalMoveTime = 0;
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

void Workspace::sortList(std::deque<Agent> &unsortedAgents, unsigned int coord, unsigned int startIndex, unsigned int endIndex){

  start_time = static_cast<double>(timer.getTimeMilliseconds()) ;
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

  run_time = (static_cast<double>(timer.getTimeMilliseconds())) - start_time;
  totalSortTimeCPU += run_time;
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
  gpuContainer gpuAgents;
  for(uint i = 0; i<agents.size(); i++){
    gpuAgents.push_back(agents.at(i).position[index]);
  }
  return gpuAgents;
}


void printDeque(std::deque<Agent> agents, unsigned int index){
  for(size_t i = 0; i < agents.size(); i++){
    std::cout << agents.at(i).position[index] << std::endl;
  }
}

void Workspace::sortAgentsByX(){
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


void Workspace::sortAgents(int groupeSize){
  if(enableGPU){
    sortAgentsGPU(groupeSize);
    start_time = static_cast<double>(timer.getTimeMilliseconds());
    getNeighborhoodGPU();
    run_time = (static_cast<double>(timer.getTimeMilliseconds())) - start_time;
    totalNeighboorTimeGPU += run_time;  
    // std::cout << totalNeighboorTimeGPU << std::endl;
  }
  else{
    

    // we first sort by X to create YZ planes
    this->sortAgentsByX();
    // we then sort by Y to create lines
    this->sortAgentsByY();
    // we finally sort by Z to create voxels
    this->sortAgentsByZ();
    for (uint i = 0; i < agents.size(); i++){
      getNeighborhood(i);
    }
  }
  
  
}

void Workspace::mergeCPU(std::vector<float> &list1, std::vector<float> list2, std::vector<int> &index1, std::vector<int> index2){
  std::vector<float> output;
  std::vector<int> ouputIndex;
  int iterator1 = 0;
  int iterator2 = 0;
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
      output.push_back(list2[i]);
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



std::vector<int> Workspace::sortGPU(int groupeSize, std::vector<float> &agentsProjection, int startIndex, int endIndex){
  
  //Init param
  //Initialisation des listes
  start_time = static_cast<double>(timer.getTimeMilliseconds());
  //Initialisation de la liste d'index  
  std::vector<float> listIndex;
  for (int j = startIndex; j < endIndex; j++){
    listIndex.push_back(j);
  }

  //Create the globalGroup as a multiple of the workgroup
  const int globalSize = ((endIndex-startIndex) / groupeSize) * groupeSize;
  std::vector<float> h_X_rest(agentsProjection.begin() + startIndex + globalSize, agentsProjection.begin() + endIndex);
  
  std::vector<float> h_X(agentsProjection.begin() + startIndex, agentsProjection.begin() + startIndex + globalSize);

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

  run_time = (static_cast<double>(timer.getTimeMilliseconds())) - start_time;
  totalSortTimeGPU += run_time;
  //==================================================================
  //______________Kernel for merging the things__________

  start_time = static_cast<double>(timer.getTimeMilliseconds());
  cl::Program programMerge(context, util::loadProgram("merging.cl"), true);
  
  
  groupeSize *= 2;
  int tempSize = (globalSize / groupeSize) * groupeSize;
  int counter = 0;
  std::vector<float> tempAgents(h_X.begin(), h_X.end());
  std::vector<float> tempAgents_reste(h_X_rest.begin(), h_X_rest.end());


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

    tempAgents.assign(mergeX.begin(), mergeX.end()); //20
    tempIndex.assign(mergeIndex.begin(), mergeIndex.end()); 



    mergeCPU(tempAgents_reste, mergeX_reste, tempIndex_reste, mergeIndex_rest);


    groupeSize *= 2;
    tempSize = (globalSize / groupeSize) * groupeSize;
    
  }


  mergeCPU(tempAgents, tempAgents_reste, tempIndex, tempIndex_reste);
  run_time = (static_cast<double>(timer.getTimeMilliseconds())) - start_time;
  totalNeighboorTimeGPU += run_time;

  return tempIndex;


}



void Workspace::getNeighborhoodGPU(){
  if (listTotaleVoisinsIndex.size() == 0){
    cl::Buffer d_voisins;
    // Load in kernel source, creating a program object for the context
    cl::Context context(chosen_device);
    cl::CommandQueue queue(context, device);
    //Kernel for sorting with a param

    cl::Program program(context, util::loadProgram("neighborhood.cl"), true);
    int d_voisins_size = (pow(sideCount, 3) - 1) * na;
    d_voisins = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * d_voisins_size);
    std::vector<int> voisins(d_voisins_size);

    cl::Kernel kernel_neighbor = cl::Kernel(program, "getVoisins");
    kernel_neighbor.setArg(0, d_voisins);
    kernel_neighbor.setArg(1, (int) sideCount / 2);
    kernel_neighbor.setArg(2, (int) pow(agents.size(), 1.0/3.0));

    cl::NDRange global(agents.size());
    queue.enqueueNDRangeKernel(kernel_neighbor, cl::NullRange, global);
    queue.finish();
    cl::copy(queue, d_voisins, voisins.begin(), voisins.end());

    for (auto it : voisins){
      listTotaleVoisinsIndex.push_back(it);
    }
  }

  if (agents[0].neighbors.size() == 0){
    for (int i = 0; i < agents.size(); i++){
      for (int j= 0; j < pow(sideCount, 3) - 1; j++){
        agents[i].neighbors.push_back(listTotaleVoisinsIndex.at(i * (pow(sideCount, 3) - 1) + j));
      }
    }
  }
  else{
    for (int i = 0; i < agents.size(); i++){
      for (int j= 0; j < pow(sideCount, 3) - 1; j++){
        agents[i].neighbors.at(j) = listTotaleVoisinsIndex.at(i * (pow(sideCount, 3) - 1) + j);
      }
    }
  }
}





void Workspace::getNeighborhood(uint index){
  int radius = sideCount/2;
  // we compare sideCount^3 with na instade of sideCount with square cube of na becuare it causes float issues
  if( pow(sideCount,3) > na){
    perror("Neighborhood radius is too big, try a smaller one");
    return;
  }
  int cote = pow(na, 1./3.);
  int currentZ = index  % cote;
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

void Workspace::sortAgentsByComponentGPU(int coordIndex, int startIndex, int endIndex, int groupeSize){
  std::vector<float> agentsProjection = convertAgents(coordIndex);

  std::vector<int> agentsIndex  = sortGPU((int)groupeSize, agentsProjection, startIndex, endIndex);
  std::vector<Agent> tempAgents;

  for (int i = 0 ; i < endIndex - startIndex; i++){
    tempAgents.push_back(agents.at(0));
  }

  for (int i = 0 ; i < endIndex - startIndex; i++){
    tempAgents[i] = agents.at(agentsIndex[i]);
  } 

  for (int i = 0; i < endIndex - startIndex; i++){
    agents.at(startIndex + i)  = tempAgents[i];
  }

}

void Workspace::sortAgentsGPU(int groupeSize){
  sortAgentsByComponentGPU(0, 0, agents.size(), groupeSize);


   // creating the voxel container witch defines the ZY planes
  int numberOfIterationsY = pow(agents.size(), 1.0/3.0);  //2
  int numberOfPlanAgentsY = pow(numberOfIterationsY, 2); //4

  for(unsigned int i = 0; i < numberOfIterationsY; i++){
    sortAgentsByComponentGPU(1, i * numberOfPlanAgentsY, (i+1) * numberOfPlanAgentsY, std::max(numberOfIterationsY, groupeSize/numberOfIterationsY));
  }

  // creating the voxel container witch defines the ZY planes
  int  numberOfLineAgentsZ = pow(agents.size(), 1.0/3.0);
  int numberOfIterationsZ = pow(numberOfLineAgentsZ, 2);

  for(unsigned int i = 0; i < numberOfIterationsZ; i++){
    sortAgentsByComponentGPU(2, i * numberOfLineAgentsZ, (i+1) * numberOfLineAgentsZ, std::max(numberOfIterationsZ, groupeSize/numberOfIterationsZ));
  }

}

void Workspace::simulate(int nsteps, int groupeSize) {
  // store initial positions
    save(0);
  // perform nsteps time steps of the simulation
    
    int step = 0;
    std::cout << " starting the mouvement " << std::endl;
    
    while (step++ < nsteps){
      start_time = (static_cast<double>(timer.getTimeMilliseconds()));
      this->move();
      run_time = (static_cast<double>(timer.getTimeMilliseconds())) - start_time;
      totalMoveTime += run_time;
       if (step % 100 == 0){

        this->sortAgents(groupeSize);
      }
      if (step % 20 == 0){;
        std::cout << "step N°" << step << " saved. "<< std::endl;
        save(step);
      }
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

}
