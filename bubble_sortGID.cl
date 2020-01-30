__kernel void sortList(
    __global float* agents, // quadruplet (x, y, z, indexDansListeInit)
    __global int* indexList,
    const int agentSize,
    const int groupeSize)
{  
    __local bool b[1];
    b[0] = true;
    uint wid = get_group_id(0);
    uint lid = get_local_id(0);
    uint gs = groupeSize;
    uint gid = wid * gs + lid; //get_global_id(0);//

    while(b[0]){
        b[0] = false;
        if (lid % 2 == 0){
            if ((lid +1 < gs)  && agents[gid] > agents[gid + 1]){
                float temp = agents[gid];
                agents[gid] = agents[gid + 1];
                agents[gid + 1] = temp;
                int tempIndex = indexList[gid];
                indexList[gid] = indexList[gid + 1];
                indexList[gid + 1] = tempIndex;
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        if (lid % 2 == 1){
            if ((lid + 1 < gs) && agents[gid] > agents[gid + 1]){
                float temp = agents[gid];
                agents[gid] = agents[gid + 1];
                agents[gid + 1] = temp;
                int tempIndex = indexList[gid];
                indexList[gid] = indexList[gid + 1];
                indexList[gid + 1] = tempIndex;
                b[0] = true;
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }


    barrier(CLK_LOCAL_MEM_FENCE);
}
