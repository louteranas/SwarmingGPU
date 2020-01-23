//------------------------------------------------------------------------------
//
// kernel:  vadd
//
// Purpose: Compute pi
//
// input: a and b float vectors of length count
//
// output: c float vector of length count holding the sum a + b
//

__kernel void createList(
   __global float* a,
   __global float* r,
   const double step)
{
 __local float b[16];

   uint wid = get_group_id(0);
   uint lid = get_local_id(0);
   uint gs = 16;
   uint gid = get_global_id(0);//wid * gs + lid;
  r[gid] = a[gid];
  // float x = (gid - 0.5) * step;
  // a[gid] = 4.0 / (1. + x * x);
  // b[lid] = a[gid];
  // barrier(CLK_LOCAL_MEM_FENCE);

  // for(uint s = gs/2; s > 0; s >>= 1) {
  //     if(lid < s) {
  //       b[lid] += b[lid+s];
  //     }
  //     barrier(CLK_LOCAL_MEM_FENCE);
  // }
  // if(lid == 0) r[wid] = b[lid];
}
