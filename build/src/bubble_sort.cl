__kernel void sortList(
    __global float* agents, // quadruplet (x, y, z, indexDansListeInit)
    __global int* indexList,
    const int agentSize,
    const int groupeSize)
{ 

    bool notSorted = true;
    uint wid = get_group_id(0);
    uint lid = get_local_id(0);
    uint gs = groupeSize;
    uint gid = wid * gs + lid; //get_global_id(0);//

    // agents[gid] = wid;
    //  Trier pour chaque sous groupe (gid + 1 < ((wid + 1) * gs ) &&  (gid + 1 < ((wid + 1) * gs ) && 
    while(notSorted){
        notSorted = false;
        if (lid % 2 == 0){
            if ((gid +1 < agentSize)  && agents[gid] < agents[gid + 1]){
                float temp = agents[gid];
                agents[gid] = agents[gid + 1];
                agents[gid + 1] = temp;
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        if (lid % 2 == 1){
            if ((gid +1 < agentSize) && agents[gid] < agents[gid + 1]){
                float temp = agents[gid];
                agents[gid] = agents[gid + 1];
                agents[gid + 1] = temp;
                notSorted = true;
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    
}
