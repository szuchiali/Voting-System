# include <iostream>
# include <cstdlib>
# include <cstring>
# include <string>
# include <vector>
// ...

std::vector<std::string> parseCmd(const std::string& raw_line, const std::string& delim) ;
std::vector<std::string> parseHttpCmd(const std::string& raw_http, const std::string& delim);
