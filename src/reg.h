#ifndef REG_H
#define REG_H

#include <list>
#include <vector>
//#include <tr1/unordered_map>
#include <unordered_map>
#include "sim.h"

const int N_ARCH_REG = 67;

// NOTE started off as a rob entry
// now it is a general structure instruction metadata to flow through processor
struct rob_entry {
	bool rdy;
	unsigned int tag;
	// instruction identifiers
	instxn i;
	// timing information
	unsigned long int t_fe_start, t_de_start, t_rn_start, t_rr_start; 
	unsigned long int t_di_start, t_is_start, t_ex_start, t_wb_start; 
	unsigned long int t_rt_start;
	// renaming identifiers
	bool rs1_rn, rs2_rn;
	bool rs1_rdy, rs2_rdy;
};

class PipelineStructure {
	unsigned int start_tag;
	unsigned int size;
	std::list<rob_entry> pipe_struct;
	std::unordered_map<unsigned int, std::list<rob_entry>::iterator> tags;

	unsigned int newTag();

	public:
		PipelineStructure(unsigned int _size) : size(_size) {start_tag = 0;}
		
		bool insertEntry(rob_entry);
		void deleteHead();
		void removeHead(rob_entry &);
		void getHead(rob_entry &);
		void getEntry(unsigned int, rob_entry &);
		void updateEntry(unsigned int, rob_entry);
		bool isHeadReady();
		bool isReady(unsigned int);
		bool isFree(unsigned int width);
		unsigned int getROBTag();
		// FIXME fxn for IQ only
		bool getNextReady(rob_entry &);
		void deleteEntry(rob_entry);
		void removeEntry(rob_entry, rob_entry &);
		void wakeup(unsigned int);
};

struct rmt_entry {
	bool 				valid;
	unsigned int 		tag;
};

class RMT {
	std::vector<rmt_entry> rmt;

	public:
		RMT() {rmt.resize(N_ARCH_REG); for(int i=0; i<N_ARCH_REG; i++) rmt[i].valid = false; }

		bool getValid(int);
		unsigned int getROBTag(int);

		void setValid(int, bool);
		void setROBTag(int, unsigned int);
};

// FIXME
// implement later as virtual derived class 
//class IQ {
//	std::list<rob_entry> iq;
//}

struct ex_entry {
	int	timer; // max of 5
	rob_entry		i_entry;
};

#endif
