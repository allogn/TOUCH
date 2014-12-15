/* 
 * File:   test.cu
 * Author: alvis
 *
 * Created on December 15, 2014, 4:46 PM
 */

#include <test.h>

__device__ void hello()
{
    int a = 1 + 10;
    return;
}

__global__ 
void hellow() 
{
    hello();
}

__host__
void testcuda()
{
    hellow<<<10,10>>>();
}

/*
 * 
 */