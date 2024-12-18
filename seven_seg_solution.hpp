#ifndef MATEXP_SOLUTION_INCLUDED
#define MATEXP_SOLUTION_INCLUDED

#include <cstdlib>
#include <stdio.h>
#include <cstdint>
#include <unistd.h>
#include <thread>
#include <vector>

#include "function_map.hpp"
#include "archlab.h"

/*
    N - Size of src array integers
    M - The number of digits in the largest integer in src
    T - Number of threads
    C - Chunk size of the workload
*/


/*
    Converts an integer into an array of seven segment representations
    
    param1: segArray - string with seven segment representation
    param2: segNumber - integer to convert into seven segment representation
*/
inline void seven_segConversion_solution(char *segArray, int32_t segNumber) {
    // Declare variables
    uint32_t position, segIndex;
    int32_t mask;
    char segMap[10] = {0x7E, 0x30, 0x6D, 0x79, 0x33,
                       0x5B, 0x5F, 0x70, 0x7F, 0x73};
    
    // Pad the array with zeros => O(M)
    std::fill(segArray, segArray + 11, 0x0);

    // Convert negative numbers to positive numbers => O(1)
    mask = segNumber >> 31;
    segArray[0] = mask & 0x1;
    segNumber = (segNumber ^ mask) - mask;

    // Fill the array backwards with the seven segment representation => O(M)
    for (position = 10; segNumber > 0; segNumber /= 10, --position) {
        segIndex = segNumber % 10;
        segArray[position] = segMap[segIndex];
    }
}

/*
    Worker function for multi-threading seven segment conversion.

    Eaach thread is given a workload for a portion of the array to convert.
    The workload is determined by the start and end index of the array.
    
    param1: start - start index of the array
    param2: end - end index of the array
    param3: dst - array of strings with seven segment representations
    param4: src - array of integers to convert
*/
void worker(int start, int end, char **dst, int32_t *src) {
    // Convert the workload to seven segment => O(C)
    for (int i = start; i < end; ++i) {
        seven_segConversion_solution(dst[i], src[i]);
    }
}

/*  
    Convert an array of integers into an array of seven segment representation.

    Multithreading is used to convert the array of integers into seven segment
    representations. Each thread is assigned a chunk of the array to convert.
    The number of threads is determined by the number of cores on the machine.
    
    param1: dst - array of strings with seven segment representations
    param2: src - array of integers to convert
    param3: size - size of the src array
*/
void seven_seg_solution(char **dst, int32_t *src, uint32_t size) {
    // Declare variables
    uint32_t totalThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(totalThreads);
    uint32_t chunks = size / totalThreads;

    // Create threads, assign workload, and convert to seven segment => O(N)
    for (int index = 0; index < totalThreads; ++index) {
        int start = index * chunks;
        int end = (index == totalThreads - 1) ? size : start + chunks;
        threads[index] = std::thread(worker, start, end, dst, src);
    }

    // Join threads => O(T)
    for (auto &thread : threads) {
        thread.join();
    }
}

#endif
