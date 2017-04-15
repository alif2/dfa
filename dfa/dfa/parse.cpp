#include <stdlib.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include "parse.h"

using namespace std;

// Split string by token
vector<string> parse::split(string str, char token) {
	// Ignore inline comments
	int comment = str.find('#');
	if (comment >= 0) {
		str = str.substr(0, comment);
	}

	// Remove whitespace
	str.erase(remove_if(str.begin(), str.end(), isspace), str.end());

	vector<string> vec;
	unsigned int i = str.find(':') + 1;

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

// Split transition list (delta) string
vector<string> parse::split_delta(string str) {
	// Ignore inline comments
	int comment = str.find('#');
	if (comment >= 0) {
		str = str.substr(0, comment);
	}

	// Remove whitespace
	str.erase(remove_if(str.begin(), str.end(), isspace), str.end());

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

// Read .dfa file input
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
}

/* Create a DFA from tokenized string vectors
   *head contains the start state
   The returned vector contains a pointer to all the states.
   This vector can be safely deleted without removing the internal references between states and should be in order to avoid memory leaks
*/
vector<struct parse::state*> parse::build_dfa(vector<string> state, vector<string> lang, vector<string> acpt, vector<string> delta, string init, struct parse::state *&head) {
	vector<struct parse::state*> dfa;
	head = NULL;

	// Create set of all states. All must be created before building transitions
	for (unsigned int i = 0; i < state.size(); i++) {
		struct parse::state *st = new struct parse::state;
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
		string trans = delta.at(i + 1);
		string endst = delta.at(i + 2);

		for (unsigned int j = 0; j < dfa.size(); j++) {
			if (dfa.at(j)->name.compare(srtst) == 0) {
				struct parse::trns trst;
				trst.trch = trans.at(0);

				// Shortcut for self-loops to avoid triple looping
				if (srtst.compare(endst) == 0) {
					trst.trste = dfa.at(j);
				}

				else {
					for (unsigned int k = 0; k < dfa.size(); k++) {
						if (dfa.at(k)->name.compare(endst) == 0) {
							trst.trste = dfa.at(k);
							break;
						}
					}
				}

				dfa.at(j)->trns.push_back(trst);
			}
		}
	}

	return dfa;
}