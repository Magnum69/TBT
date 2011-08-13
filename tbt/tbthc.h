
#ifndef _TBT_BASE_H
#define _TBT_BASE_H

/*! \mainpage Introduction
 *
 * TBT&mdash;<em>The Basic Toolbox for heterogeneous programming</em>&mdash;is a C++-library
 * that makes it easy to use OpenCL for heterogeneous programming. It provides an easy-to-use
 * interface to OpenCL and implements basic primitives that can also be used with user OpenCL
 * programs.
 *
 * \section Overview
 * TBT provides the following functionality:
 *   - Automatic selection of the OpenCL platform and creation of a global OpenCL context and command queues for the OpenCL devices.
 *   - Automatic caching of program binaries.
 *   - Generic host and device arrays associating structured data with a specific OpenCL device and command queue.
 *   - Easy access to device-specific information and simplified enqueueing of commands.
 *   - Basic primitives for building OpenCL programs.
 *
 * \section hello-world TBT's hello world
 * This is a simple TBT-program that sorts an array on the GPU.
 *
 * \code
 * #include <tbt/algorithm.h>
 *
 * int main()
 * {
 *     // Create a global context for a GPU device (automatically selects an appropriate platform)
 *     tbt::createContext(CL_DEVICE_TYPE_GPU);
 *
 *     // Get a device controller in the global context.
 *     tbt::DeviceController *devCon = tbt::getDeviceController();
 *     
 *     const size_t n = 4096;
 *     tbt::MappedArray<cl_uint> mappedArray(devCon, n);  // Create an array on the host, mapped to a GPU device 
 *     initRandom(mappedArray);                           // initialize array with random data (on the host)
 *
 *     mappedArray.mapHostToDevice();                     // Transfer array from host to device
 *     radixSort(mappedArray);                            // Sort the array on the GPU device
 *     mappedArray.mapDeviceToHostBlocking();             // Transfer (sorted) array back to host
 * }
 * \endcode
 */


/**
 * \defgroup context Global Configuration
 * \brief These classes and functions allow to select the global platform and context, and provide access to
 *        platform, context, and device controllers.
 *
 * \defgroup memobjects Memory Objects
 * \brief These classes implement high-level memory objects stored on the device, the host, or both.
 *
 * \defgroup iterators Iterators
 * \brief These classes represent iterators pointing into memory objects.
 *
 * \defgroup algorithm Algorithms
 * \brief These classes and functions implement the algorithms available in TBT. They work on device arrays
 *        which determine on what device an algorithm is run.
 */


#define __NO_STD_VECTOR             // Use cl::vector instead of STL version
#define __CL_ENABLE_EXCEPTIONS      // use exceptions
#pragma warning( disable : 4290 )   // avoid useless warnings with Visual C++ and OpenCL exceptions


#include <CL/cl.hpp>
#include <iostream>

#include <tbt/Error.h>


namespace tbt
{

	//! The type for array indices.
	typedef ptrdiff_t index_t;

}


#endif
