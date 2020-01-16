#include <iostream>
#include <fstream>
#include <sstream>

#include "oclhelper.h"
#include "oclerror.h"

using namespace cl;

namespace oclHelper
{

/*
Initialize runtime with default platform,
default device and build context
*/
oclRuntime::oclRuntime(uint idxP, uint idxD)
{
  std::vector<cl::Platform> platforms;

  try
  {
    // Get list of available OpenCL platforms
    cl::Platform::get(&platforms);
  }
  catch (cl::Error err)
  {
    std::cerr << "OpenCL Error: " << err.what() << " returned " << err_code(err.err()) << std::endl;
    throw;
  }

  // Select platform
  if (idxP > 0 && idxP < platforms.size())
  {
    m_platform = platforms[idxP];
  }
  else
  {
    m_platform = platforms[0];
  }

  std::vector<cl::Device> devices;
  try
  {
    // Get list of all available OpenCL devices on the platform
    m_platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
  }
  catch (cl::Error err)
  {
    std::cerr << "OpenCL Error: " << err.what() << " returned " << err_code(err.err()) << std::endl;
    throw;
  }

  // Select device
  if (idxD > 0 && idxD < devices.size())
  {
    m_device = devices[idxP];
  }
  else
  {
    m_device = devices[devices.size() - 1];
  }

  // Set compiler options
  m_compile_parameters = std::string("-Werror ");

  std::string sInfo;
  cl_device_type dev_type;
  m_device.getInfo(CL_DEVICE_TYPE, &dev_type);
  m_wavefront_size = 1;

  if (dev_type == CL_DEVICE_TYPE_GPU)
  {
    m_device.getInfo(CL_DEVICE_NAME, &sInfo);
    if (sInfo.find("AMD") != std::string::npos)
      m_wavefront_size = 64;
    if (sInfo.find("NVIDIA") != std::string::npos)
      m_wavefront_size = 32;
  }

  m_compile_parameters += " -DWAVEFRONT_SIZE=" + std::to_string(m_wavefront_size);

  // Create context
  try
  {
    m_context = cl::Context(m_device);
  }
  catch (cl::Error err)
  {
    std::cerr << "OpenCL Error: " << err.what() << " returned " << err_code(err.err()) << std::endl;
    throw;
  }

  // Create command Queue
  try
  {
    m_queue = cl::CommandQueue(m_context, CL_QUEUE_PROFILING_ENABLE);
  }
  catch (cl::Error err)
  {
    std::cerr << "OpenCL Error: " << err.what() << " returned " << err_code(err.err()) << std::endl;
    throw;
  }

} // namespace oclHelper

oclRuntime::~oclRuntime()
{
}

void oclRuntime::setPlatform(int idx)
{
  std::vector<cl::Platform> platforms;
  try
  {
    // Get list of available OpenCL platforms
    cl::Platform::get(&platforms);
  }
  catch (cl::Error err)
  {
    std::cerr << "OpenCL Error: " << err.what() << " returned " << err_code(err.err()) << std::endl;
    throw;
  }

  unsigned idxP = 0;
  unsigned idxD = 0;
  std::vector<cl::Device> devices;

  if (idx < 0 || idx > platforms.size())
  {
    //  Display available platforms for invalid index
    for (auto pl : platforms)
    {
      if (platforms.size() > 1)
      {
        std::cout << idxP++ << ". ";
      }
      // Display information about platform
      displayPlatformInfo(pl);

      // Select all devices on the given platform
      pl.getDevices(CL_DEVICE_TYPE_ALL, &devices);
      idxD = 0;

      // For each device display only basic information
      for (auto dev : devices)
        std::cout << '\t' << idxD++ << ". " << dev.getInfo<CL_DEVICE_NAME>() << '/' << dev.getInfo<CL_DEVICE_VENDOR>() << std::endl;
    }

    // Platform selection
    if (platforms.size() > 1)
    {
      std::cout << "Select platform index: ";
      std::cin >> idxP;
      while (idxP < 0 || idxP > platforms.size())
      {
        std::cout << "Invalid platform index. Select again:";
        std::cin >> idxP;
      }
    }
    else
    {
      idxP = 0;
    }
    m_platform = platforms[idxP];
  }
}

void oclRuntime::setDevice(int idx = -1)
{
  std::vector<cl::Device> devices;
  try
  {
    // Get list of available OpenCL platforms
    m_platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    ;
  }
  catch (cl::Error err)
  {
    std::cerr << "OpenCL Error: " << err.what() << " returned " << err_code(err.err()) << std::endl;
    throw;
  }

  unsigned idxD = 0;
  if (idx < 0 || idx > devices.size())
  {
    //  Display available platforms for invalid index
    for (auto dev : devices)
    {
      if (devices.size() > 1)
      {
        std::cout << idxD++ << ". ";
      }
      // Display information about platform
      displayDeviceInfo(dev);
    }

    // Platform selection
    if (devices.size() > 1)
    {
      std::cout << "Select device index: ";
      std::cin >> idxD;
      while (idxD < 0 || idxD > devices.size())
      {
        std::cout << "Invalid platform index. Select again:";
        std::cin >> idxD;
      }
    }
    else
    {
      idxD = devices.size() - 1;
    }
    m_device = devices[idxD];
  }
}

void oclRuntime::displayPlatformInfo(cl::Platform &pl,
                                     cl_platform_info info)
{

  std::string s;
  try
  {
    pl.getInfo(info, &s);
    std::cout << s << std::endl;
  }
  catch (cl::Error err)
  {
    std::cerr << "OpenCL Error: " << err.what() << " returned " << err_code(err.err()) << std::endl;
    throw;
  }
}

void oclRuntime::displayPlatformInfo(cl::Platform &pl)
{
  std::cout << std::string(10, '=') << "\t Platform information " << std::endl;
  std::cout << "Platform Vendor \t: ";
  displayPlatformInfo(pl, CL_PLATFORM_VENDOR);
  std::cout << "Platform Version \t: ";
  displayPlatformInfo(pl, CL_PLATFORM_VERSION);
  std::cout << "Platform Profile \t: ";
  displayPlatformInfo(pl, CL_PLATFORM_PROFILE);
  std::cout << "Platform Name \t\t: ";
  displayPlatformInfo(pl, CL_PLATFORM_NAME);
  std::cout << "Platform Extensions \t: ";
  displayPlatformInfo(pl, CL_PLATFORM_EXTENSIONS);
}

void oclRuntime::displayPlatformInfo()
{
  this->displayPlatformInfo(m_platform);
}

void oclRuntime::displayDeviceInfo(cl::Device &dev, cl_device_info dev_info)
{
  std::string s;
  try
  {
    dev.getInfo(dev_info, &s);
    std::cout << s << std::endl;
  }
  catch (cl::Error err)
  {
    std::cerr << "OpenCL Error: " << err.what() << " returned " << err_code(err.err()) << std::endl;
    throw;
  }
}

void oclRuntime::displayDeviceInfo(cl::Device &dev)
{
  std::cout << "\n"
            << std::string(10, '=') << "\t Device information " << std::endl;

  std::cout << "Device Name \t\t: ";
  displayDeviceInfo(dev, CL_DEVICE_NAME);
  std::cout << "Device Version  \t: ";
  displayDeviceInfo(dev, CL_DEVICE_OPENCL_C_VERSION);
  std::cout << "Device Driver Version \t: ";
  displayDeviceInfo(dev, CL_DRIVER_VERSION);
  std::cout << "Device Build Kernels \t: ";
  displayDeviceInfo(dev, CL_DEVICE_BUILT_IN_KERNELS);
}

void oclRuntime::displayDeviceInfo()
{
  this->displayDeviceInfo(m_device);
}

void oclRuntime::buildProgramFromFile(cl::Program &program, std::string filename)
{
  std::ifstream infile(filename);
  std::stringstream buffer;
  buffer << infile.rdbuf();
  std::string src_string = buffer.str();

  std::cout << "Building kernel with options \"" << m_compile_parameters << "\"" << std::endl;

  cl::Program::Sources src(1, std::make_pair(src_string.c_str(), src_string.size()));
  std::string buildLog;
  program = cl::Program(m_context, src);
  try
  {
    program.build(m_compile_parameters.c_str());
  }
  catch (cl::Error err)
  {

    program.getBuildInfo(m_device, CL_PROGRAM_BUILD_LOG, &buildLog);
    std::cerr << "OpenCL Error: " << err.what() << " returned " << err_code(err.err()) << std::endl;

    std::cerr << "Build log:" << std::endl
              << std::string(10, '=') << std::endl
              << buildLog << std::endl
              << std::string(10, '=') << std::endl;
    throw;
  }

  program.getBuildInfo(m_device, CL_PROGRAM_BUILD_LOG, &buildLog);
  if (buildLog != "")
    std::cout << "Build log:" << std::endl
              << std::string(10, '=') << std::endl
              << buildLog << std::endl
              << std::string(10, '=') << std::endl;
}

void oclRuntime::addCompileOptions(std::string option)
{
  m_compile_parameters += " " + option;
}

void oclRuntime::resetCompileOptions()
{
  m_compile_parameters = std::string("");
}

} // namespace oclHelper