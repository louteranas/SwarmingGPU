/*
 **  PROGRAM: Approximation of pi
 **
 **  PURPOSE: This program will numerically compute the integral of
 **           4/(1+x*x)
 **
 **           from 0 to 1. The value of this integral is pi.
 **           The is the original sequential program. It uses the timer
 **           from the OpenMP runtime library
 **
 **  USAGE: ./pi
 **
 */
#include "util.hpp"
#include "cl.hpp"
#include "device_picker.hpp"
#include "err_code.h"

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
static long num_steps = 100000000;
double step;
extern double wtime();   // returns time since some fixed past point (wtime.c)

int main (int argc, char *argv[])
{
    int i;

    double x, pi, sum = 0.0;
    double start_time; // Starting time

    int groupSize = 16;

    step = 1.0/(double) num_steps;


    util::Timer timer;
    std::vector<float> h_a(num_steps);
    std::vector<float> h_r((int) num_steps / groupSize);

    cl::Buffer d_a, d_r, d_b;



    cl_uint deviceIndex = 2;
    parseArguments(argc, argv, &deviceIndex);

    // Get list of devices
    std::vector<cl::Device> devices;
    unsigned numDevices = getDeviceList(devices);

    // Check device index in range
    if (deviceIndex >= numDevices)
    {
        std::cout << "Invalid device index (try '--list')\n";
        return EXIT_FAILURE;
    }

    cl::Device device = devices[deviceIndex];

    std::string name;
    getDeviceName(device, name);
    std::cout << "\nUsing OpenCL device: " << name << "\n";

    std::vector<cl::Device> chosen_device;
    chosen_device.push_back(device);

    // ------------------------------------------------------------------
    // Run sequential matmul
    // ------------------------------------------------------------------


    timer.reset();
    for (i=1;i<= num_steps; i++){
        x = (i-0.5)*step;
        sum = sum + 4.0/(1.0+x*x);
    }
    pi = step * sum;
    double run_time = static_cast<double>(timer.getTimeMilliseconds()) / 1000.0;
    std::cout<<"pi with "<<num_steps<<" steps is "
        << pi <<" in "
        <<run_time<<" seconds"<<std::endl;


    // ------------------------------------------------------------------
    // Run parallel pi
    // ------------------------------------------------------------------


    timer.reset();
    //Première création de la liste

    // Load in kernel source, creating a program object for the context
    cl::Context context(chosen_device);
    cl::CommandQueue queue(context, device);
    cl::Program program(context, util::loadProgram("pi.cl"), true);
    // Create the compute kernel from the program
    d_a = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * num_steps);
    d_r = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * (int) (num_steps / groupSize));
    cl::Kernel kernel_pi = cl::Kernel(program, "createList");
    kernel_pi.setArg(0, d_a);
    kernel_pi.setArg(1, d_r);
    kernel_pi.setArg(2, step);

    cl::NDRange global(num_steps);
    cl::NDRange local(groupSize);

    start_time = static_cast<double>(timer.getTimeMilliseconds()) / 1000.0;
    queue.enqueueNDRangeKernel(kernel_pi, cl::NullRange, global, local);
    queue.finish();
    run_time = (static_cast<double>(timer.getTimeMilliseconds()) / 1000.0) - start_time;

    cl::copy(queue, d_a, h_a.begin(), h_a.end());
    cl::copy(queue, d_r, h_r.begin(), h_r.end());
    sum = 0;

    for (int j =0; j < h_r.size(); j++){
      sum += h_r[j];
    }
    std::cout << "pi :" << sum * step << std::endl;
    


}
