

__kernel void mergeList(
    __global float* agents,
    __global int* indexList,
    int agentSize,

    int groupeSize)
{   
    __local float tempAgents[2500];
    __local int tempIndex[2500];
    uint wid = get_group_id(0);
    uint lid = get_local_id(0);
    uint gid = get_global_id(0);

    
    uint firstIndex = 0;
    uint secondIndex = groupeSize/2;
    int counter = 0;
    while( firstIndex < groupeSize/2 && secondIndex < groupeSize){
        if(agents[wid*groupeSize + firstIndex] > agents[wid*groupeSize + secondIndex]){
            tempAgents[counter] = agents[wid*groupeSize + secondIndex];
            tempIndex[counter++] = indexList[wid*groupeSize + secondIndex];
            secondIndex++;
        }
        else{
            tempAgents[counter] = agents[wid*groupeSize + firstIndex];
            tempIndex[counter++] = indexList[wid*groupeSize + firstIndex];           
            firstIndex++;
        }
    }

    if (firstIndex < groupeSize / 2){
        for (int i = firstIndex ; i < groupeSize / 2; i++){
            tempAgents[counter] = agents[wid * groupeSize + i];
            tempIndex[counter++] = indexList[wid*groupeSize + i];
        }
    }
    else{
        for (int i = secondIndex ; i < groupeSize ; i++){
            tempAgents[counter] = agents[wid * groupeSize + i];
            tempIndex[counter++] = indexList[wid*groupeSize + i];
        }
    }
    for (int i = 0; i < groupeSize; i++){
        agents[wid * groupeSize + i] = tempAgents[i];
        indexList[wid * groupeSize + i] = tempIndex[i];
    }
}

