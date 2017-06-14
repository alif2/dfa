#include <algorithm>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "parse.h"

using namespace std;

/*
    Parse.cpp contains functions for parsing a .dfa file and creating a linked list of transversable states
    Also includes functions for minimizing a DFA
*/

/* Split string by token */
vector<string> parse::split(string str, char token) {
	// Ignore inline comments
	int comment = str.find('#');
	if (comment >= 0) {
		str = str.substr(0, comment);
	}

	// Remove whitespace
	str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());

	vector<string> vec;
	unsigned int i = str.find(':') + 1;

	// Find token and split string
	while (i < str.length()) {
		int loc = str.find(token, i);
		if (loc < 0) {
			loc = str.length();
		}

		vec.push_back(str.substr(i, loc - i));
		i = loc + 1;
	}

	return vec;
}

/* Split transition list (delta) string */
vector<string> parse::split_delta(string str) {
	// Ignore inline comments
	int comment = str.find('#');
	if (comment >= 0) {
		str = str.substr(0, comment);
	}

	// Remove whitespace
	str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());

	// Split delta string on delta tokens (, - ;)
	vector<string> vec;
	unsigned int i = str.find(':') + 1;
	while (i < str.length()) {
		int loc = str.find(',', i);
		vec.push_back(str.substr(i, loc - i));
		i = loc + 1;

		loc = str.find('-', i);
		vec.push_back(str.substr(i, loc - i));
		i = loc + 2;

		loc = str.find(';', i);
		if (loc < 0) {
			loc = str.length() - 1;
		}

		vec.push_back(str.substr(i, loc - i));
		i = loc + 1;

		loc++;
	}

	return vec;
}

/* Read .dfa file input into corresponding vectors */
void parse::read_file(string fname, vector<string> &state, vector<string> &lang, vector<string> &acpt, vector<string> &delta, string &init) {
	ifstream file;
	file.open(fname);

	string line;
	if (file.is_open()) {
		while (getline(file, line)) {
			// Ignore commented lines
			if (line.at(0) == '#') continue;

			else if (line.substr(0, 6).compare("states") == 0) {
				state = split(line, ';');
			}

			else if (line.substr(0, 14).compare("input_alphabet") == 0) {
				lang = split(line, ';');
			}

			else if (line.substr(0, 11).compare("start_state") == 0) {
				init = split(line, ';').at(0);
			}

			else if (line.substr(0, 13).compare("accept_states") == 0) {
				acpt = split(line, ';');
			}

			else if (line.substr(0, 5).compare("delta") == 0) {
				delta = split_delta(line);
			}
		}

		file.close();
	}

	else {
		printf("Error: Could not open DFA file\n");
		exit(1);
	}
}

/* Create a DFA from tokenized string vectors
   *head points to the start state
   The returned vector contains a pointer to all the states.
   This vector can be safely deleted without removing the internal references between states and should be in order to avoid memory leaks
*/
vector<struct parse::state*> parse::build_dfa(vector<string> state, vector<string> lang, vector<string> acpt, vector<string> delta, string init, struct parse::state *&head) {
	vector<struct parse::state*> dfa;
	head = NULL;

	// Create set of all states. All must be created before building transitions
	for (unsigned int i = 0; i < state.size(); i++) {
		struct parse::state *st = new struct parse::state();
		st->name = state.at(i);
		st->accept = false;

		// Set the start state
		if (head == NULL) {
			if (st->name.compare(init) == 0) {
				head = st;
			}
		}

		for (unsigned int j = 0; j < acpt.size(); j++) {
			if (acpt.at(j).compare(st->name) == 0) {
				st->accept = true;
				break;
			}
		}

		dfa.push_back(st);
	}

	// Create state transitions
	for (unsigned int i = 0; i < delta.size(); i += 3) {
		string srtst = delta.at(i);
		char transc = delta.at(i + 1).at(0);
		string endst = delta.at(i + 2);

		// Locate first node
		for (unsigned int j = 0; j < dfa.size(); j++) {
			if (dfa.at(j)->name.compare(srtst) == 0) {
				struct parse::trns trst;
				trst.trch = transc;

				// Shortcut for self-loops
				if (srtst.compare(endst) == 0) {
					trst.trste = dfa.at(j);
				}
				
				// Locate node to transition to
				else {
					for (unsigned int k = 0; k < dfa.size(); k++) {
						if (dfa.at(k)->name.compare(endst) == 0) {
							trst.trste = dfa.at(k);
							break;
						}
					}
				}

				dfa.at(j)->trns.push_back(trst);
				break;
			}
		}
	}

	return dfa;
}

/* Transform a DFA into a minimal DFA */
void parse::minimize_dfa(vector<struct parse::state*> &dfa, parse::state *&head) {
	vector<pair<struct parse::state*, struct parse::state*>> pairs;
	vector<pair<struct parse::state*, struct parse::state*>> remaining;

	// Create set of pairs of all states
	for (unsigned int i = 0; i < dfa.size(); i++) {
		for (unsigned int j = i + 1; j < dfa.size(); j++) {
			pairs.push_back(pair<struct parse::state*, struct parse::state*>(dfa.at(i), dfa.at(j)));

			// Ignore pairs of accept-not accept states, since we can't merge them
			if ((dfa.at(i)->accept && dfa.at(j)->accept || (!dfa.at(i)->accept && !dfa.at(j)->accept))) {
				remaining.push_back(pair<struct parse::state*, struct parse::state*>(dfa.at(i), dfa.at(j)));
			}
		}
	}

	minimize(pairs, remaining);
	merge(dfa, remaining, head);
}

/* Determine what DFA states must be minimized in order to create a minimal DFA */
void parse::minimize(vector<pair<struct parse::state*, struct parse::state*>> &pairs, vector<pair<struct parse::state*, struct parse::state*>> &remaining) {
	bool modified = false;

	// For each unmarked pair
	for (unsigned int i = 0; i < remaining.size(); i++) {
		bool deleted = false;
		struct parse::state* first = remaining.at(i).first;
		struct parse::state* second = remaining.at(i).second;

		// For each character in the language for the first element
		for (unsigned int j = 0; j < first->trns.size(); j++) {
			char ftrch = first->trns.at(j).trch;

			// That matches the character in the language of the second element
			for (unsigned int k = 0; k < second->trns.size(); k++) {
				char strch = second->trns.at(k).trch;

				// If the pair pointed to by the transition function is marked (not in vector), mark (remove) the originating pair
				if (ftrch == strch) {
					struct parse::state* ftrst = first->trns.at(j).trste;
					struct parse::state* ltrst = second->trns.at(k).trste;
					
					bool found = false;

					// Determine if the pair (order-agnostic) is in the set of possible pairs
					for (unsigned int m = 0; m < pairs.size(); m++) {
						if ((pairs.at(m).first == ftrst && pairs.at(m).second == ltrst) || (pairs.at(m).first == ltrst && pairs.at(m).second == ftrst)) {
							found = true;
							break;
						}
					}

					// Determine if the pair is unmarked (in the vector). If so, remove (mark) it
					if (found) {
						for (unsigned int m = 0; m < remaining.size(); m++) {
							if ((remaining.at(m).first == ftrst && remaining.at(m).second == ltrst) || (remaining.at(m).first == ltrst && remaining.at(m).second == ftrst)) {
								found = false;
							}
						}

						if (found) {
							remaining.erase(remaining.begin() + i);
							deleted = true;
							modified = true;
							break;
						}
					}
				}
			}

			if (deleted) {
				i--;
				break;
			}
		}
	}

	if (modified) {
		parse::minimize(pairs, remaining);
	}
}

/* Merge DFA states. Reset head if head node merged */
void parse::merge(vector<struct parse::state*> &dfa, vector<pair<struct parse::state*, struct parse::state*>> &remaining, parse::state *&head) {
	for (unsigned int i = 0; i < remaining.size(); i++) {
		struct parse::state *first = remaining.at(i).first;
		struct parse::state *second = remaining.at(i).second;

		struct parse::state *merged = new parse::state;
		merged->accept = first->accept;
		merged->name = first->name + "/" + second->name;
		
		// Remap transitions
		vector<trns> trans;
		for (unsigned int j = 0; j < first->trns.size(); j++) {
			struct trns ftrns = first->trns.at(j);

			trns ntrans;
			ntrans.trch = ftrns.trch;

			if (ftrns.trste == first || ftrns.trste == second) {
				ntrans.trste = merged;
			}

			else {
				ntrans.trste = ftrns.trste;
			}

			trans.push_back(ntrans);
		}

		merged->trns = trans;

		if (head == first || head == second) {
			head = merged;
		}

		dfa.push_back(merged);

		// Remap any pointers pointing to pre-merge nodes
		for (unsigned int j = 0; j < dfa.size(); j++) {
			for (unsigned int k = 0; k < dfa.at(j)->trns.size(); k++) {
				trns trns = dfa.at(j)->trns.at(k);
				
				if (trns.trste == first || trns.trste == second) {
					dfa.at(j)->trns.at(k).trste = merged;
				}
			}
		}

		// Cleanup
		for (unsigned int j = 0; j < dfa.size(); j++) {
			if (dfa.at(j) == first || dfa.at(j) == second) {
				dfa.erase(dfa.begin() + j);
				j--;
			}
		}

		delete first;
		delete second;
	}
}
