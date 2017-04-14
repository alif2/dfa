#include <stdlib.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include "parse.h"

using namespace std;

vector<string> parse::split(string str, char token) {
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

vector<string> parse::split_delta(string str) {
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

void parse::read_file(string fname, vector<string> &state, vector<string> &lang, vector<string> &acpt, vector<string> &delta, string &init) {
	ifstream file;
	file.open(fname);

	string line;
	if (file.is_open()) {
		while (getline(file, line)) {
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