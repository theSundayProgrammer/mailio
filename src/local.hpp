
#ifndef LOCAL_HEADER_FOR_SIMPLESERVER
#define LOCAL_HEADER_FOR_SIMPLESERVER
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <filesystem>
bool is_prefix(std::string const& prefix,std::string const& candidate);
std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ");
std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ");
std::filesystem::path get_unique_filename(std::filesystem::path const& folder, std::filesystem::path const& fname);
typedef   std::map<std::string,std::string> name_value;
name_value parse_input(std::istream& ifs) ;
void log_error(std::string const& str);
#endif
