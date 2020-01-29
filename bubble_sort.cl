__kernel void sortList(
    __global float* agents, // quadruplet (x, y, z, indexDansListeInit)
    __global int* indexList,
    const int agentSize,
    const int groupeSize)
{  
    __local bool b[1];
    __local float localAgent[500];
    __local float localIndex[500];   
    b[0] = true;
    uint wid = get_group_id(0);
    uint lid = get_local_id(0);
    uint gs = groupeSize;
    uint gid = wid * gs + lid; //get_global_id(0);//

    localAgent[lid] = agents[gid];
    localIndex[lid] = indexList[gid];
   
    barrier(CLK_LOCAL_MEM_FENCE);

    while(b[0]){
        b[0] = false;
        if (lid % 2 == 0){
            if ((lid +1 < gs)  && localAgent[lid] > localAgent[lid + 1]){
                float temp = localAgent[lid];
                localAgent[lid] = localAgent[lid + 1];
                localAgent[lid + 1] = temp;
                int tempIndex = localIndex[lid];
                localIndex[lid] = localIndex[lid + 1];
                localIndex[lid + 1] = tempIndex;
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        if (lid % 2 == 1){
            if ((lid + 1 < gs) && localAgent[lid] > localAgent[lid + 1]){
                float temp = localAgent[lid];
                localAgent[lid] = localAgent[lid + 1];
                localAgent[lid + 1] = temp;
                int tempIndex = localIndex[lid];
                localIndex[lid] = localIndex[lid + 1];
                localIndex[lid + 1] = tempIndex;
                b[0] = true;
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    agents[gid] = localAgent[lid];
    indexList[gid] = localIndex[lid];    

    barrier(CLK_LOCAL_MEM_FENCE);
}
