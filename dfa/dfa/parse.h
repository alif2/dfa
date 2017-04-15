#pragma once

class parse
{
public:
	// Must forward declare transition state for circular reference
	struct trns;

	struct state {
		std::string name;
		bool accept;
		std::vector<struct trns> trns;
	};

	struct trns {
		char trch;
		struct state *trste;
	};

	static std::vector<std::string> split(std::string str, char token);
	static std::vector<std::string> split_delta(std::string str);
	static void read_file(std::string fname, std::vector<std::string> &state, std::vector<std::string> &lang, std::vector<std::string> &acpt, std::vector<std::string> &delta, std::string &init);
	static std::vector<struct state*> build_dfa(std::vector<std::string> state, std::vector<std::string> lang, std::vector<std::string> acpt, std::vector<std::string> delta, std::string init, struct state *&head);
};