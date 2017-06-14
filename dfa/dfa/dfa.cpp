#include <cstdlib>
#include <iostream>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

#include "parse.h"

using namespace std;

/* Execute the DFA by traversing paths for matched input
   Fails if no paths are available when has remaining input
*/
bool run_dfa(struct parse::state *head, string input) {
	for (unsigned int i = 0; i < input.length(); i++) {
		for (unsigned int j = 0; j < head->trns.size(); j++) {
			if (head->trns.at(j).trch == input.at(i)) {
				head = head->trns.at(j).trste;
				break;
			}

			if (j >= head->trns.size() - 1) {
				cout << "Could not complete DFA\n";
				return false;
			}
		}
	}

	return head->accept;
}

/* Compare 2 DFAs to determine if they are equivalent */
string compare_dfa(parse::state *head, parse::state *head2) {
	stack<parse::state*> s1, s2;
	vector<parse::state*> visited, visited2;

	// Node, parent node, transition character
	vector<tuple<parse::state*, parse::state*, char>> path, path2;

	s1.push(head);
	s2.push(head2);
	visited.push_back(head);
	visited2.push_back(head2);

	path.push_back(make_tuple(head, head, NULL));
	path2.push_back(make_tuple(head2, head2, NULL));

	// Depth-first search both DFAs
	while (s1.size() > 0) {
		parse::state *node = s1.top();
		s1.pop();

		parse::state *node2 = s2.top();
		s2.pop();

		for (unsigned int i = 0; i < node->trns.size(); i++) {
			parse::state *vnode = node->trns.at(i).trste;
			char vchar = node->trns.at(i).trch;
			bool nvisited = false;

			parse::state *vnode2 = NULL;
			char vchar2;
			bool nvisited2 = false;

			// Look for matching node in second DFA
			for (unsigned int j = 0; j < node2->trns.size(); j++) {
				if (node2->trns.at(j).trch == node->trns.at(i).trch) {
					vnode2 = node2->trns.at(j).trste;
					vchar2 = node2->trns.at(j).trch;
					break;
				}
			}

			// Determine if current node in first DFA has been visited
			for (unsigned int j = 0; j < visited.size(); j++) {
				if (visited.at(j) == vnode) {
					nvisited = true;
					break;
				}
			}

			// Determine if current node in second DFA has been visited
			for (unsigned int j = 0; j < visited2.size(); j++) {
				if (visited2.at(j) == vnode2) {
					nvisited2 = true;
					break;
				}
			}

			// Triggered if DFA structures differ
			if (nvisited != nvisited2 || node->accept != node2->accept) {
				path.push_back(make_tuple(vnode, node, vchar));
				path2.push_back(make_tuple(vnode2, node2, vchar2));

				// Backtrace input on path
				string input;
				tuple<parse::state*, parse::state*, char> current = path.at(path.size() - 1);
				while (path.size() > 1) {
					input = get<2>(current) + input;

					for (unsigned int k = 0; k < path.size(); k++) {
						if (get<0>(path.at(k)) == get<1>(current)) {
							current = path.at(k);
							path.erase(path.begin() + k);
						}
					}
				}

				return input;
			}

			// Continue traversal if more nodes to visit
			if (!nvisited) {
				s1.push(vnode);
				visited.push_back(vnode);
				path.push_back(make_tuple(vnode, node, vchar));
			}

			if (!nvisited2) {
				s2.push(vnode2);
				visited2.push_back(vnode2);
				path2.push_back(make_tuple(vnode2, node2, vchar2));
			}	
		}
	}

	return "";
}

/*
    If input is a DFA and a string, determine if the DFA acceps the string.
    If input is 2 DFAs, determine if they are equivalent
*/
int main(int argc, char **argv) {
	if (argc < 3) {
		cout << "Not enough arguments\n";
		exit(1);
	}

	string fname = argv[1];
	string input = argv[2];

	string init;
	parse::state *head = NULL, *head2 = NULL;
	vector<string> state, lang, acpt, delta;
	
    // Parse the first DFA
	parse::read_file(fname, state, lang, acpt, delta, init);
	vector<parse::state*> dfa = parse::build_dfa(state, lang, acpt, delta, init, head);
	parse::minimize_dfa(dfa, head);

    // Compare 2 DFAs for equivalence
	if (input.length() > 3 && input.substr(input.length() - 4, 4).compare(".dfa") == 0) {
		// Parse the second DFA
        parse::read_file(input, state, lang, acpt, delta, init);
		vector<parse::state*> dfa2 = parse::build_dfa(state, lang, acpt, delta, init, head2);
		parse::minimize_dfa(dfa2, head2);

		string diff = compare_dfa(head, head2);
		if (diff.size() > 0) {
			cout << "No\n";
			cout << diff << "\n";
		}
		else {
			cout << "Yes\n";
		}
	}

    // Determine if DFA accepts string
	else {
		run_dfa(head, input) ? cout << "Yes\n" : cout << "No\n";
	}
}
