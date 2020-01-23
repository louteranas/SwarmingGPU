__kernel void sortList(
    __global float* agentsIndex, // quadruplet (x, y, z, indexDansListeInit)
    __global int* indexList,
    const int agentSize)
{ 

    // _local bool notSorted = True;
    // uint wid = get_group_id(0);
    // uint lid = get_local_id(0);
    // uint gs = 16;
    uint gid = get_global_id(0);//wid * gs + lid;

    agentsIndex[get_group_id(0)] = 12;

    
}
