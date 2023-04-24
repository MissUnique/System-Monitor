#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <vector>
#include <string>

class Processor {
 public:
  float Utilization();  

 private:
  std::vector<std::string> values;
};

#endif