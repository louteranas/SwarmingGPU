__kernel void mergeList(
    __global float* agents,
    __global int* indexList,
    int groupeSize)
{   
    __local float tempAgents[2500];
    __local int tempIndex[2500];

    uint gid = get_global_id(0);

    
    uint firstIndex = 0;
    uint secondIndex = groupeSize/2;
    int counter = 0;
    while( firstIndex < groupeSize/2 && secondIndex < groupeSize){
        if(agents[gid*groupeSize + firstIndex] > agents[gid*groupeSize + secondIndex]){
            tempAgents[counter] = agents[gid*groupeSize + secondIndex];
            tempIndex[counter++] = indexList[gid*groupeSize + secondIndex];
            secondIndex++;
        }
        else{
            tempAgents[counter] = agents[gid*groupeSize + firstIndex];
            tempIndex[counter++] = indexList[gid*groupeSize + firstIndex];           
            firstIndex++;
        }
    }

    if (firstIndex < groupeSize / 2){
        for (int i = firstIndex ; i < groupeSize / 2; i++){
            tempAgents[counter] = agents[gid * groupeSize + i];
            tempIndex[counter++] = indexList[gid*groupeSize + i];
        }
    }
    else{
        for (int i = secondIndex ; i < groupeSize ; i++){
            tempAgents[counter] = agents[gid * groupeSize + i];
            tempIndex[counter++] = indexList[gid*groupeSize + i];
        }
    }
    for (int i = 0; i < groupeSize; i++){
        agents[gid * groupeSize + i] = tempAgents[i];
        indexList[gid * groupeSize + i] = tempIndex[i];
    }
}

