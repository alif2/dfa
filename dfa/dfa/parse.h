#pragma once
class parse
{
public:
	static std::vector<std::string> parse::split(std::string str, char token);
	static std::vector<std::string> parse::split_delta(std::string str);
	static void parse::read_file(std::string fname, std::vector<std::string> &state, std::vector<std::string> &lang, std::vector<std::string> &acpt, std::vector<std::string> &delta, std::string &init);
};