#ifndef PROC_H
#define PROC_H

#include <inttypes.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include "sim.h"
#include "reg.h"

class Processor {
	private:
		unsigned long int 	clk;
		proc_params 		params;
		proc_stats stats;
		FILE* 				i_mem; 
		bool pipeline_empty, eof; // termination signals
		const int execute_depth = 5;
		const int deadlock_tolerance = 1000;
		int deadlock;
		
		// Microarchitectural Registers
		std::list<rob_entry> de,rn,rr,di,wb;	// DE,RN,RR,DI,WB Pipeline registers
		std::list<ex_entry> execute_list;
		RMT rmt;
		ROB rob;
		IQ 	iq;
		// ARF

	public:
		Processor(proc_params, FILE*);
		//~Processor();

		void runProc();
		bool nextCycle();
		void fetch();
		void decode();
		void rename();
		void regRead();
		void dispatch();
		void issue();
		void execute();
		void wakeup(unsigned int);
		void writeback();
		void retire();
		void printTiming(rob_entry);
		void printFinalResults();

};

#endif
