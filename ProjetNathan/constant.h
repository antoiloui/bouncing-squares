#ifndef CONSTANT_H_DEFINED
#define CONSTANT_H_DEFINED

#include <limits.h>

#define MAX_STATE 14

#define PIXEL_WIDTH 2
#define PIXEL_HEIGHT 2

// SIZE_X and SIZE_Y must be both a multiple of 16
#define SIZE_X 256
#define SIZE_Y SIZE_X
#define ZONE_SIDE_SIZE (SIZE_X/4)

#define NB_PROCESS 16
#define NB_PROCESS_LINE 4

// time to wait in displayer process before starting displaying next iteration
#define DISP_SLEEP_TIME 10000

// limits the max number of iteration
#define MAX_ITERATION LONG_MAX

// limits the advance of the worker processes on the displayer process
// in order to avoid running out of memory
#define MAX_ADVANCEMENT 15000

#endif
