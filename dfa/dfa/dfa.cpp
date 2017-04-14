#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include "parse.h"

using namespace std;

struct trns {
	char trch;
	struct state *trste;
};

struct state {
	string name;
	bool accept;
	vector<struct trns> trns;
};

bool run_dfa(struct state *head, string input) {
	struct state *st = head;
	unsigned int i;
	for (i = 0; i < input.length(); i++) {
		for (unsigned int j = 0; j < st->trns.size(); j++) {
			if (st->trns.at(j).trch == input.at(i)) {
				st = st->trns.at(j).trste;
				break;
			}

			if (j == st->trns.size() - 1) {
				cout << "Error: Could not complete DFA\n";
				return false;
			}
		}
	}
	return st->accept;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		cout << "Not enough arguments\n";
		exit(1);
	}

	string fname = argv[1];
	string input = argv[2];

	string init;
	vector<string> state, lang, acpt, delta;
	parse::read_file(fname, state, lang, acpt, delta, init);

	vector<struct state> dfa;
	for (unsigned int i = 0; i < state.size(); i++) {
		struct state st;
		st.name = state.at(i);
		st.accept = false;

		for (unsigned int j = 0; j < acpt.size(); j++) {
			if (acpt.at(j).compare(st.name) == 0) {
				st.accept = true;
				break;
			}
		}
		dfa.push_back(st);
	}

	for (unsigned int i = 0; i < delta.size(); i += 3) {
		string srtst = delta.at(i);
		string trans = delta.at(i + 1);
		string endst = delta.at(i + 2);

		for (unsigned int j = 0; j < dfa.size(); j++) {
			if (dfa.at(j).name.compare(srtst) == 0) {
				struct trns trst;
				trst.trch = trans.at(0);
				
				if (srtst.compare(endst) == 0) {
					trst.trste = &dfa.at(j);
				}
				
				else {
					for (unsigned int k = 0; k < dfa.size(); k++) {
						if (dfa.at(k).name.compare(endst) == 0) {
							trst.trste = &dfa.at(k);
							break;
						}
					}
				}

				dfa.at(j).trns.push_back(trst);
			}
		}
	}

	run_dfa(&dfa.at(0), input) ? cout << "Yes\n" : cout << "No\n";
	
}