/*

 */

#ifndef OCLHELPER_H
#define OCLHELPER_H

// Must be used to enable cl::Error
#define __CL_ENABLE_EXCEPTIONS

#include <vector>
#include <iostream>

#include <memory>
#include <string>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#include "cl.hpp"
#else
#include <CL/cl.hpp>
#endif

// #define MAX_INFO_STRING 256

namespace oclHelper
{

class oclRuntime
{
private:
  // Required object for running OpenCL Kernel
  //
  cl::Platform m_platform;
  cl::Device m_device;
  cl::Context m_context;
  cl::CommandQueue m_queue;

  int createContext();
  int createCommandQueue();
  int resetDevice();

  int getPlatformList();

  std::string m_compile_parameters;

  uint m_wavefront_size;

public:
  // Constructor
  oclRuntime();
  oclRuntime(uint idxP = 0, uint idxD = 0);

  //

  // Destructor
  ~oclRuntime();

  /// Getters
  cl::Platform &getPlatformID() { return m_platform; };
  cl::Device &getDevice() { return m_device; }
  cl::Context &getContext() { return m_context; }
  cl::CommandQueue &getCmdQueue() { return m_queue; }

  // Get a command queue by index, create it if doesn't exist
  cl::CommandQueue &getCmdQueue(unsigned index,
                                cl_command_queue_properties properties = 0);

  // Get number of compute units of current device
  cl_uint getNumComputeUnit() const;

  // Reset platform
  void setPlatform(int idx = -1);

  // Reset device
  void setDevice(int idx);

  // Compilation options
  void addCompileOptions(std::string);
  void resetCompileOptions();

  /// Print information of the platform
  void displayPlatformInfo();
  void displayPlatformInfo(cl::Platform &pl);
  void displayPlatformInfo(cl::Platform &pl, cl_platform_info pl_info);

  void displayDeviceInfo();
  void displayDeviceInfo(cl::Device &dev);
  void displayDeviceInfo(cl::Device &dev, cl_device_info device_info);

  int displayAllInfo();

  void buildProgramFromFile(cl::Program &, std::string);
  void buildProgramFromSource(cl::Program &, std::string);
};

} // namespace oclHelper

#endif
