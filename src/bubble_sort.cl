__kernel void sortList(
    __global float** agents, // quadruplet (x, y, z, indexDansListeInit)
    const uint agentSize;
    const uint index)
{   
    _local bool notSorted = True;
    uint wid = get_group_id(0);
    uint lid = get_local_id(0);
    uint gs = 16;
    uint gid = get_global_id(0);//wid * gs + lid;

    //Trier pour chaque sous groupe
    while(notSorted){
        if (lid % 2 == 0){
            if (gid + 1 < ((wid + 1) * gs ) && (gid +1 < agentSize)  && agents[gid][index] < agents[gid + 1][index]){
                for (int i = 0; i < 4; i++){
                    float temp = agents[gid][i];
                    agents[gid][i] = agents[gid + 1][i];
                    agents[gid + 1][i] = temp;
                }
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        notSorted = False;
        if (lid % 2 == 1){
            if (gid + 1 < ((wid + 1) * gs ) && (gid +1 < agentSize) && agents[gid][index] < agents[gid + 1][index]){
                for (int i = 0; i < 4; i++){
                    float temp = agents[gid][i];
                    agents[gid][i] = agents[gid + 1][i];
                    agents[gid + 1][i] = temp;
                }
                notSorted = True;
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    barrier(CLK_LOCAL_MEM_FENCE);
}

__kernel void mergeList(
    __global float** agents,
    const uint agentSize,
    const uint groupeSize,
    const uint index)
{
    uint wid = get_group_id(0);
    uint lid = get_local_id(0);
    uint gid = get_global_id(0);
    
    uint firstIndex = 0;
    uint secondIndex = groupeSize/2;

    while( firstIndex < groupeSize/2 && secondIndex < groupeSize){
        if(agents[wid*groupeSize + firstIndex][index] > agents[wid*groupeSize + secondIndex][index]){
            for (int i = 0; i < 4; i++){
                float temp = agents[wid*groupeSize + firstIndex][i];
                agents[wid*groupeSize + firstIndex][i] = agents[wid*groupeSize + secondIndex][i];
                agents[wid*groupeSize + secondIndex][i] = temp;
            }
            firstIndex++;
        }
        else{
            secondIndex++;
        }

    }





}
