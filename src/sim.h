#ifndef SIM_H
#define SIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define DBG 1

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

// Put additional data structures here as per your requirement
struct proc_stats {
	unsigned long int 	fetched_inst_cnt;
	unsigned long int 	retired_inst_cnt;
	unsigned long int 	cycles;
	float 				ipc;
};

struct instxn {
	int op, dst, src1, src2;
	uint64_t pc;
};

#endif
