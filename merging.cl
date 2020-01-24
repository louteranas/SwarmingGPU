__kernel void mergeList(
    __global float* agents,
    __global int* indexList,
    const uint agentSize,
    const uint groupeSize)
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
