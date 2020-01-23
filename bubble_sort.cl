__kernel void sortList(
    __global float* agentsIndex, // quadruplet (x, y, z, indexDansListeInit)
    __global float* indexList,
    const int agentSize)
{ 

    // _local bool notSorted = True;
     uint wid = get_group_id(0);
  //uint lid = get_local_id(0);
    // uint gs = 16;
    uint gid = get_global_id(0);//wid * gs + lid;

    agentsIndex[0] = 12;
    indexList[gid] = 12;

    //Trier pour chaque sous groupe
    // while(notSorted){
    //     if (lid % 2 == 0){
    //         if (gid + 1 < ((wid + 1) * gs ) && (gid +1 < agentSize)  && agentsIndex[gid] < agentsIndex[gid + 1]){
    //             //Swap de agent
    //             float temp = agentsIndex[gid];
    //             agentsIndex[gid] = agentsIndex[gid + 1];
    //             agentsIndex[gid + 1] = temp;
    //             //Swap de index
    //             int tempIndex = indexList[gid];
    //             indexList[gid] = indexList[gid + 1];
    //             indexList[gid + 1] = temp;               
    //         }
    //     }
    //     barrier(CLK_LOCAL_MEM_FENCE);
    //     notSorted = False;
    //     if (lid % 2 == 1){
    //         if (gid + 1 < ((wid + 1) * gs ) && (gid +1 < agentSize) && agentsIndex[gid] < agentsIndex[gid + 1]){
    //             //Swap de agent
    //             float temp = agentsIndex[gid];
    //             agentsIndex[gid] = agentsIndex gid + 1];
    //             agentsIndex[gid + 1] = temp;
    //             //Swap de index
    //             int tempIndex = indexList[gid] 
    //             indexList[gid] = indexList[gid + 1];
    //             indexList[gid + 1] = temp; 
    //             notSorted = True;
    //         }
    //     }
    //     barrier(CLK_LOCAL_MEM_FENCE);
    // }


    // barrier(CLK_LOCAL_MEM_FENCE);
}

