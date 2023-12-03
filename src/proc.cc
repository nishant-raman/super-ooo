#include "reg.h"
#include "proc.h"

using namespace std;

// Processor Constructor
Processor::Processor (proc_params _params, FILE* _i_mem)
	: params(_params), i_mem(_i_mem), rob(_params.rob_size), iq(_params.iq_size) {

	// Initialize processor state
	clk = 0;
	pipeline_empty = false;
	eof = false;
	// Initialize processor statistics
	stats.fetched_inst_cnt = 0;
	stats.retired_inst_cnt = 0;
	stats.cycles = clk;
	stats.ipc = 0;

	ready2retire = 0;
}

// Run the processor
void Processor::runProc () {
	//while (nextCycle()) {
	do {
		if (DBG) {
			cout << "Starting... fic=" << stats.fetched_inst_cnt << " ric=" << stats.retired_inst_cnt << " cycle=" << clk << endl;
		}

		retire();
		writeback();
		execute();
		issue();
		dispatch();
		regRead();
		rename();
		decode();
		fetch();
		
		if (DBG) {
			cout << "Ending... fic=" << stats.fetched_inst_cnt << " ric=" << stats.retired_inst_cnt << " cycle=" << clk << endl;
		}
	//}
	} while (nextCycle());
}

// Increment cycle
bool Processor::nextCycle () {
	stats.ipc = (float)stats.retired_inst_cnt/(float)stats.cycles;
	if (eof && pipeline_empty)
		return false;
	else {

		// clk count starts from 0 (for timing)
		clk++;
		// cycle count starts from 1 (to get accurate count)
		stats.cycles++;
		return true;
	}
};

// Fetch stage logic
void Processor::fetch () {
	
	bool stall = !de.empty();
	instxn i;
	rob_entry decode_bundle_entry;
	
	if (!stall) {
		for (int idx=0; (idx<(int)params.width) && (!eof); idx++) {
    		if (fscanf(i_mem, "%lx %d %d %d %d", &i.pc, &i.op, &i.dst, &i.src1, &i.src2) != EOF) {
				
				pipeline_empty = false;
				stats.fetched_inst_cnt++;
				if (DBG) {
					cout << "Fetched Instruction " << idx << " :";
					cout << " pc " << i.pc;
					cout << " op " << i.op;
					cout << " dst " << i.dst;
					cout << " src1 " << i.src1;
					cout << " src2 " << i.src2;
					cout << endl;
				}

				// write to DE pipeline register
				decode_bundle_entry.i = i;
				decode_bundle_entry.t_fe_start = clk;
				decode_bundle_entry.t_de_start = clk+1;
				de.push_back(decode_bundle_entry);

			} else
				eof = true;

		}
	}
}

void Processor::decode () {
	bool stall = !rn.empty();
	if (!stall) {
		while (!de.empty()) {
			rob_entry rename_bundle_entry = de.front();
			rename_bundle_entry.t_rn_start = clk+1;
			de.pop_front();
			rn.push_back(rename_bundle_entry);
		}
	}
}

void Processor::rename () {

	bool stall = !rr.empty();

	// NOTE: Retire pending ready instructions
	while (ready2retire--) {
		rob.deleteHead();
	}
	ready2retire = 0;	// after while loop r2r is -1

	
	bool rob_is_free = rob.isFree((unsigned int) rn.size());
	if (rob_is_free && !stall) {
		
		while (!rn.empty()) {
			// allocate instruction in ROB
			rob_entry entry = rn.front();
			entry.rdy = false;
			entry.t_rr_start = clk+1;
			rob.insertEntry(entry);

			// rename source registers
			int src1;
			src1 = entry.i.src1;
			entry.rs1_rdy = false; // Added so that wakeup from EX is handled without error
			if (src1  == -1)
				entry.rs1_rn = false;
			else {
				bool v = rmt.getValid(src1);
				entry.rs1_rn = v;
				entry.i.src1 = v ? (int) rmt.getROBTag(src1) : src1;
			}
			
			int src2;
			src2 = entry.i.src2;
			entry.rs2_rdy = false; // Added so that wakeup from EX is handled without error
			if (src2  == -1)
				entry.rs2_rn = false;
			else {
				bool v = rmt.getValid(src2);
				entry.rs2_rn = v;
				entry.i.src2 = v ? (int) rmt.getROBTag(src2) : src2;
			}

			// rename dst reg
			// NOTE: no renaming needed since tag field already exists
			entry.tag = rob.getROBTag();
			
			// update RMT
			int dst;
			dst = entry.i.dst;
			if (dst != -1) {
				rmt.setValid(dst, true);
				rmt.setROBTag(dst, entry.tag);
			}

			rn.pop_front();
			// feed into RR register
			rr.push_back(entry);
		}
	}
}

void Processor::regRead () {

	bool stall = !di.empty();

	if (!stall) {
		while (!rr.empty()) {
			rob_entry regrd_bundle_entry = rr.front();
			
			// check src1 ready
			bool src_rn;
			int src;

			src_rn 	= regrd_bundle_entry.rs1_rn;
			src 	= regrd_bundle_entry.i.src1; 
			//rdy		= regrd_bundle_entry.rs1_rdy;

			if (src_rn) {
				if (rob.isReady(src))
					regrd_bundle_entry.rs1_rdy = true;
				// FIXME not needed? explicitly taken care of in rename and wakeups
				//else
				//	regrd_bundle_entry.rs1_rdy = rdy ? true : false;
			} else {
				regrd_bundle_entry.rs1_rdy = true;
			}

			// check src2 ready
			src_rn 	= regrd_bundle_entry.rs2_rn;
			src 	= regrd_bundle_entry.i.src2; 

			if (src_rn) {
				if (rob.isReady(src))
					regrd_bundle_entry.rs2_rdy = true;
				//else
				//	regrd_bundle_entry.rs2_rdy = false;
			} else {
				regrd_bundle_entry.rs2_rdy = true;
			}

			// remove entry from RR
			rr.pop_front();

			// Add entry to DI
			regrd_bundle_entry.t_di_start = clk+1;
			di.push_back(regrd_bundle_entry);
		}
	}
}

void Processor::dispatch () {

	bool iq_is_free = iq.isFree((unsigned int) di.size());
	if (iq_is_free) {
		while (!di.empty()) {
			rob_entry iq_bundle_entry = di.front();
			iq_bundle_entry.t_is_start = clk+1;
			di.pop_front();
			iq.insertEntry(iq_bundle_entry);
		}
	}
} 

void Processor::issue () {

	rob_entry execute_bundle_entry;
	int timer;

	for (int idx=0; 
		((idx<(int)params.width) && 	// Issue max of superscalar width instxn
		 (iq.getNextReady(execute_bundle_entry))); // Issue if there is ready IQ entry
		idx++) {

		execute_bundle_entry.t_ex_start = clk+1;
		switch (execute_bundle_entry.i.op) {
			case 0:
				timer = 1;
				break;
			case 1:
				timer = 2;
				break;
			case 2:
				timer = 5;
				break;
			default:
				timer = 0;
				break;
		}

		// Add to execute list
		execute_list.push_back({timer,execute_bundle_entry});

		// Remove from IQ
		iq.deleteEntry(execute_bundle_entry);
	}
}

void Processor::execute () {
	// add up to <width> of entries to list ( 1D list of 5*width max)
	// each cycle decrement counter of all entries
	// if counter 0 - 
	// 		wakeup dependent all others from prev stages
	//		add entries to WB
	//		pop from list

	if (DBG) {
		if (execute_list.size() > execute_depth*params.width) {
			cout << "ERROR - Violating execute stage pipeline size: ";
			cout << (int)execute_list.size();
		   	cout << "(should be: " << (int) execute_depth*params.width << endl;
		}
	}

	list<ex_entry>::iterator itr;
	for (itr=execute_list.begin(); itr!=execute_list.end(); itr++) {
		itr->timer--;
		if (itr->timer == 0) {
			// wakeup
			wakeup(itr->i_entry.tag);
			// add to WB
			rob_entry writeback_bundle_entry;
			writeback_bundle_entry = itr->i_entry;
			writeback_bundle_entry.t_wb_start = clk+1;
			wb.push_back(writeback_bundle_entry);
			// remove from execution
			itr = execute_list.erase(itr);
			itr--; // Needed since erase by default increments itr
		}
	}
}

void Processor::wakeup (unsigned int tag) {

	list<rob_entry>::iterator itr;

	// Wakeup RR entries
	for (itr=rr.begin(); itr!=rr.end(); itr++) {
		if (itr->rs1_rn && (itr->i.src1 == (int)tag))
			itr->rs1_rdy = true;
		if (itr->rs2_rn && (itr->i.src2 == (int)tag))
			itr->rs2_rdy = true;
	}

	// Wakeup DI entries
	for (itr=di.begin(); itr!=di.end(); itr++) {
		if (itr->rs1_rn && (itr->i.src1 == (int)tag))
			itr->rs1_rdy = true;
		if (itr->rs2_rn && (itr->i.src2 == (int)tag))
			itr->rs2_rdy = true;
	}
	
	// Wakeup IQ entries
	iq.wakeup(tag);
}

void Processor::writeback () {
	// fix renamed values by checking rob
	// set ready 1
	// writeback to rob

	while (!wb.empty()) {

		rob_entry wb2rob_entry, rob_temp;

		wb2rob_entry = wb.front();
		rob.getEntry(wb2rob_entry.tag, rob_temp);

		wb2rob_entry.i.src1 	= rob_temp.i.src1;
		wb2rob_entry.i.src2 	= rob_temp.i.src2;
		wb2rob_entry.rs1_rn 	= rob_temp.rs1_rn;
		wb2rob_entry.rs2_rn 	= rob_temp.rs2_rn;
		wb2rob_entry.rs1_rdy 	= rob_temp.rs1_rdy;
		wb2rob_entry.rs2_rdy 	= rob_temp.rs2_rdy;

		wb2rob_entry.rdy		= true;
		wb2rob_entry.t_rt_start	= clk+1;

		rob.updateEntry(wb2rob_entry.tag, wb2rob_entry);

		wb.pop_front();
	}
}

void Processor::retire () {

	if (DBG) {
		cout << "Retiring instruction bundle" << endl;
	}

	// If ROB not empty
	for (int idx=0; idx<(int)params.width; idx++) {

		rob_entry head;

		if (rob.isHeadReady()) {
			// Retire from ROB in 2 stages:
			// 	a) increment ready to retire for each ready instruction
			// 	b) in register rename before adding rob entries, 
			// 		remove the ready to retire entries
			// This resolves a software implementation induced deadlock:
			// If in cycle n, an instruction in RN stage gets rob tag renaming
			// in cycle n+1, rob tag entry retired and removed from list
			// before consumer instruction receives speculative rob value.
			// rob.removeHead(head);
			ready2retire++;
			rob.getHead(head);

			// Update RMT if it has rename set for retiring rob tag
			int dst = head.i.dst;
			if (dst > 0) {
				if (rmt.getROBTag(dst) == head.tag)
					rmt.setValid(dst, false);
			}

			printTiming(head);
			stats.retired_inst_cnt++;
		} else {
			break;
		}
	}

	// If no more instruction in pipeline assert it
	if (stats.retired_inst_cnt == stats.fetched_inst_cnt)
		pipeline_empty = true;

	if (DBG) {
		cout << "Retired instruction bundle" << endl;
	}
}

void Processor::printTiming (rob_entry rti) {
	cout << stats.retired_inst_cnt;
	cout << " fu{" << rti.i.op << "}";
	cout << " src{" << rti.i.src1 << "," << rti.i.src2 << "}";
	cout << " dst{" << rti.i.dst << "}";
	cout << " FE{" << rti.t_fe_start << "," << rti.t_de_start - rti.t_fe_start << "}";
	cout << " DE{" << rti.t_de_start << "," << rti.t_rn_start - rti.t_de_start << "}";
	cout << " RN{" << rti.t_rn_start << "," << rti.t_rr_start - rti.t_rn_start << "}";
	cout << " RR{" << rti.t_rr_start << "," << rti.t_di_start - rti.t_rr_start << "}";
	cout << " DI{" << rti.t_di_start << "," << rti.t_is_start - rti.t_di_start << "}";
	cout << " IS{" << rti.t_is_start << "," << rti.t_ex_start - rti.t_is_start << "}";
	cout << " EX{" << rti.t_ex_start << "," << rti.t_wb_start - rti.t_ex_start << "}";
	cout << " WB{" << rti.t_wb_start << "," << rti.t_rt_start - rti.t_wb_start << "}";
	cout << " RT{" << rti.t_rt_start << "," << clk - rti.t_rt_start + 1 << "}"; // FIXME is it +1 really or just lucky?
	cout << endl;
}

void Processor::printFinalResults () {
	// print config
	cout << "# === Processor Configuration ===" << endl;
	cout << "# ROB_SIZE = " << params.rob_size << endl;
	cout << "# IQ_SIZE  = " << params.iq_size << endl;
	cout << "# WIDTH    = " << params.width << endl;
	// print final results
	cout << "# === Simulation Results ========" << endl;
	cout << "# Dynamic Instruction Count    = " << stats.retired_inst_cnt << endl;
	cout << "# Cycles                       = " << stats.cycles << endl;
	cout << "# Instructions Per Cycle (IPC) = " << stats.ipc << endl;
}
