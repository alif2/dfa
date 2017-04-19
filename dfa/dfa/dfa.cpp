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
				printf("Could not complete DFA\n");
				return false;
			}
		}
	}

	return head->accept;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Not enough arguments\n");
		exit(1);
	}

	string fname = argv[1];
	string input = argv[2];

	string init;
	parse::state *head = NULL;
	vector<string> state, lang, acpt, delta;
	
	parse::read_file(fname, state, lang, acpt, delta, init);
	vector<parse::state*> dfa = parse::build_dfa(state, lang, acpt, delta, init, head);

	parse::minimize_dfa(dfa, head);

	run_dfa(head, input) ? printf("Yes\n") : printf("No\n");
}