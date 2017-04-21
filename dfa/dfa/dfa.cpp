#include <vector>
#include <stack>
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
				printf("Could not complete DFA\n");
				return false;
			}
		}
	}

	return head->accept;
}

/* Compare 2 DFAs to determine if they are equivalent */
bool compare_dfa(parse::state *head, parse::state *head2) {
	stack<parse::state*> s1, s2;
	vector<parse::state*> visited, visited2;

	s1.push(head);
	s2.push(head2);
	visited.push_back(head);
	visited2.push_back(head2);

	while (s1.size() > 0) {
		parse::state *node = s1.top();
		s1.pop();

		parse::state *node2 = s2.top();
		s2.pop();

		for (unsigned int i = 0; i < node->trns.size(); i++) {
			parse::state *vnode = node->trns.at(i).trste;
			bool nvisited = false;

			parse::state *vnode2;
			bool nvisited2 = false;

			for (unsigned int j = 0; j < node2->trns.size(); j++) {
				if () {

				}
			}

			for (unsigned int j = 0; j < visited.size(); j++) {
				if (visited.at(j) == vnode) {
					nvisited = true;
					break;
				}
			}

			if (!nvisited) {
				s1.push(vnode);
				visited.push_back(vnode);
			}
		}
	}
	return false;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Not enough arguments\n");
		exit(1);
	}

	string fname = argv[1];
	string input = argv[2];

	string init;
	parse::state *head = NULL, *head2 = NULL;
	vector<string> state, lang, acpt, delta;
	
	parse::read_file(fname, state, lang, acpt, delta, init);
	vector<parse::state*> dfa = parse::build_dfa(state, lang, acpt, delta, init, head);
	parse::minimize_dfa(dfa, head);

	if (input.substr(input.length() - 4, 4).compare(".dfa") == 0) {
		parse::read_file(input, state, lang, acpt, delta, init);
		vector<parse::state*> dfa2 = parse::build_dfa(state, lang, acpt, delta, init, head2);
		parse::minimize_dfa(dfa2, head2);

		compare_dfa(head, head2);
	}

	run_dfa(head, input) ? printf("Yes\n") : printf("No\n");
}