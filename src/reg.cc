#include "sim.h"
#include "reg.h"

using namespace std;

//-------------------------------------------------------------------------
//	PipelineStructure functions

// Inserts a new entry into the ROB if there is space
bool ROB::insertEntry (rob_entry tail) {
	if (pipe_struct.size() != size) {
		tail.tag = newTag();
		pipe_struct.push_back(tail);
		tags[tail.tag] = --(pipe_struct.end());
		return true;
	} else
		return false;
};

// Inserts a new entry into the ROB if there is space
bool IQ::insertEntry (rob_entry tail) {
	if (pipe_struct.size() != size) {
		pipe_struct.push_back(tail);
		tags[tail.tag] = --(pipe_struct.end());
		return true;
	} else
		return false;
};

// deletes the Head of ROB
void PipelineStructure::deleteHead () {
	unsigned int remove_tag = pipe_struct.begin()->tag;
	tags.erase(remove_tag);
	pipe_struct.pop_front();
}

// Removes Head entry of ROB, and gives entry
void PipelineStructure::removeHead (rob_entry &head) {
	unsigned int remove_tag = pipe_struct.begin()->tag;
	tags.erase(remove_tag);
	head = pipe_struct.front();
	pipe_struct.pop_front();
}

// get Head of ROB
void PipelineStructure::getHead (rob_entry &head) {
	head = pipe_struct.front();
}

// gets the an entry of ROB
void PipelineStructure::getEntry (unsigned int key, rob_entry &entry) {
	if (!pipe_struct.empty() && (tags.find(key)!=tags.end()))
		entry = *(tags[key]);
}

void ROB::getEntry (unsigned int key, rob_entry &entry) {
	validTag(key);
	PipelineStructure::getEntry(key, entry);
}

// updates ROB entry
void PipelineStructure::updateEntry (unsigned int key, rob_entry entry) {
	if (!pipe_struct.empty() && (tags.find(key)!=tags.end()))
		*(tags[key]) = entry;
}

bool PipelineStructure::isHeadReady () {
	if (pipe_struct.empty())
		return false;
	else {
		if (pipe_struct.front().rdy)
			return true;
		else
			return false;
	}
}

// Finds if entry in rob with tag "key" is ready or not
bool PipelineStructure::isReady (unsigned int key) {
	if (!pipe_struct.empty() && (tags.find(key)!=tags.end())) {
		if (tags[key]->rdy)
			return true;
		else
			return false;
	} else
		return false;
}

// Finds if entry in rob with tag "key" is ready or not
bool ROB::isReady (unsigned int key) {
	validTag(key);
	return PipelineStructure::isReady(key);
}

// check if ROB has instruction bundle width of free entries
bool PipelineStructure::isFree (unsigned int width) {
	unsigned int available_entries;
	available_entries = size - pipe_struct.size();
	if (width > available_entries)
		return false;
	else
		return true;
}

unsigned int ROB::newTag () {
	if (pipe_struct.empty())
		return start_tag;
	else if (pipe_struct.back().tag == (size-1))
		return start_tag;
	else
		return pipe_struct.back().tag + 1;
}

void ROB::validTag 	(unsigned int &key) {
	// FIXME is this sufficient
	// might get tags outside range - so bring it back inside range
	if (key >= size)
		key -= size;
}

unsigned int PipelineStructure::getROBTag () {
	return pipe_struct.back().tag;
}

bool IQ::getNextReady (rob_entry &iq_entry) {
	bool exists = false;
	list<rob_entry>::iterator itr;
	for(itr=pipe_struct.begin(); itr!=pipe_struct.end(); itr++) {
		if (itr->rs1_rdy && itr->rs2_rdy) {
			iq_entry = *itr;
			exists = true;
			break;
		}
	}
	return exists;
}

// deletes the Head of ROB
void IQ::deleteEntry (rob_entry remove_entry) {
	unsigned int remove_tag = remove_entry.tag;
	list<rob_entry>::iterator remove_entry_itr = tags[remove_tag];
	tags.erase(remove_tag);
	pipe_struct.erase(remove_entry_itr);
}

// Removes Head entry of ROB, and gives entry
void IQ::removeEntry (rob_entry remove_entry, rob_entry &loc) {
	unsigned int remove_tag = remove_entry.tag;
	list<rob_entry>::iterator remove_entry_itr = tags[remove_tag];
	tags.erase(remove_tag);
	loc = *remove_entry_itr;
	pipe_struct.erase(remove_entry_itr);
}

void IQ::wakeup (unsigned int tag) {
	list<rob_entry>::iterator itr;
	for (itr=pipe_struct.begin(); itr!=pipe_struct.end(); itr++) {
		if (itr->rs1_rn && (itr->i.src1 == (int)tag))
			itr->rs1_rdy = true;
		if (itr->rs2_rn && (itr->i.src2 == (int)tag))
			itr->rs2_rdy = true;
	}
}


//-------------------------------------------------------------------------
//	RMT functions

bool RMT::getValid (int index) {
	return rmt[index].valid;
}

unsigned int RMT::getROBTag (int index) {
	return rmt[index].tag;
}

void RMT::setValid (int index, bool valid) {
	rmt[index].valid = valid;
}

void RMT::setROBTag (int index, unsigned int rob_tag) {
	rmt[index].tag = rob_tag;
}

