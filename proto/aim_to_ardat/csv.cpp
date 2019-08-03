#include "csv.h"

std::regex CsvHelper::s_stringPattern("^\\s*\"([^\"]*)\"\\s*[,\\n]");
std::regex CsvHelper::s_doublePattern("^\\s*(-?\\d+\\.\\d+)\\s*[,\\n]");
