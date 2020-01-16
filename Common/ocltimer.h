double executeKernel(const cl::CommandQueue &queue, const cl::Kernel &kernel, const cl::NDRange &globalRange, const cl::NDRange &localRange)
{
  const int TOTAL_ITS = 100;
  double total_time = 0.;
  std::cout << "Executing...";
  for (int i = 0; i < TOTAL_ITS; i++)
  {
    cl::Event eKernel;
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalRange, localRange, NULL, &eKernel);
    queue.finish();
    auto infStart = eKernel.getProfilingInfo<CL_PROFILING_COMMAND_START>();
    auto infFinish = eKernel.getProfilingInfo<CL_PROFILING_COMMAND_END>();
    total_time += (infFinish - infStart) / 1000000.;
  }
  std::cout << "Done!" << std::endl;
  return total_time / TOTAL_ITS;
}