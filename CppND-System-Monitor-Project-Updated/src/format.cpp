#include <string>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 
  int h, m, s;
  h = seconds / 3600;
  m = (seconds % 3600) / 60;
  s = (seconds % 3600) % 60;
  string time;
  time = std::to_string(h) + ':' + std::to_string(m) + ':' + std::to_string(s);
  return time; 
}