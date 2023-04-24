#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  float memory, memTotal, memFree, totalUsedMem, cached, sReclaimable, shmem, CachedMem_Y, buffers_B, nonCachBuffMem_G;
  string line, key, value;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") memTotal = stoi(value);
        else if (key == "MemFree:") memFree = stoi(value);
        else if (key == "Cached:") cached = stoi(value);
        else if (key == "SReclaimable:") sReclaimable = stoi(value);
        else if (key == "Shmem:") shmem = stoi(value);
        else if (key == "Buffers:") buffers_B = stoi(value);
      }
    }
  }
  totalUsedMem = memTotal - memFree;
  CachedMem_Y = cached + sReclaimable - shmem;
  nonCachBuffMem_G = totalUsedMem - (buffers_B + CachedMem_Y);
  memory = (nonCachBuffMem_G + buffers_B + CachedMem_Y) / memTotal;
  return memory; 
}

// Read and return the system uptime
long LinuxParser::UpTime() { 
  long uptime;
  string line;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return uptime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  long uptime = LinuxParser::UpTime();
  long jiffies = uptime * sysconf(_SC_CLK_TCK);
  return jiffies; 
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  string line, value;
  int n = 14, utime, stime_, cutime, cstime;
  long activePidJiffies;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (n > 1){
      linestream >> value;
      n--;
    }
    linestream >> utime >> stime_ >> cutime >> cstime;
  }
  activePidJiffies = utime + stime_ + cutime + cstime;
  return activePidJiffies;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  std::vector<std::string> values = LinuxParser::CpuUtilization();
  int user = std::stoi(values[0]);
  int nice = std::stoi(values[1]);
  int system = std::stoi(values[2]);
  int irq = std::stoi(values[5]);
  int softirq = std::stoi(values[6]);
  int steal = std::stoi(values[7]);
  int guest = std::stoi(values[8]);
  int guest_nice = std::stoi(values[9]);
  long activeJiffies = user + nice + system + irq + softirq + steal + guest + guest_nice;
  return activeJiffies; 
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  std::vector<std::string> values = LinuxParser::CpuUtilization();
  int idle = std::stoi(values[3]);
  int iowait = std::stoi(values[4]);
  long idleJiffies = idle + iowait;
  return idleJiffies; 
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  std::string line, v, name;
  int n = 10;
  vector<string> cpu;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> name;
    while (n > 0){
      linestream >> v;
      cpu.emplace_back(v);
      n--;
    }
  }
  return cpu;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  int totProcess, value;
  string line, key;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          totProcess = value;
          break;
        }
      }
    }
  }
  return totProcess;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  int runProcess, value;
  string line, key;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          runProcess = value;
          break;
        }
      }
    }
  }
  return runProcess;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) std::getline(stream, line);
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  string line, key, mem;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> mem;
      if (key == "VmSize:") break;
    }
  }
  mem = to_string(std::stoi(mem) / 1000);
  return mem;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  string line, key, userID;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> userID;
      if (key == "Uid:") break;
    }
  }
  return userID;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) { 
  string userID, line, key, user_, value;
  userID = LinuxParser::Uid(pid);
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> user_ >> key >> value;
      if (value == userID) break;
    }
  }
  return user_;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) { 
  long uptime;
  string line, value;
  int n = 22;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (n > 0){
      linestream >> value;
      n--;
    }
  }
  uptime = std::stoi(value);
  uptime /= sysconf(_SC_CLK_TCK);
  return uptime; 
}

// Read and return CPU utilization of a process
float LinuxParser::CpuUtilization(int pid) {
  string line, value;
  int n = 14, utime, stime_, cutime, cstime, starttime, uptime;
  long seconds;
  float cpu_usage, totaltime_, hertz;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (n > 1){
      linestream >> value;
      n--;
    }
    linestream >> utime >> stime_ >> cutime >> cstime;
    for (n = 0; n < 5; n++) linestream >> starttime;
  }
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  hertz = sysconf(_SC_CLK_TCK);
  totaltime_ = utime + stime_ + cutime + cstime;
  seconds = uptime - (starttime / hertz);
  cpu_usage = 100 * ((totaltime_ / hertz) / seconds);
  return cpu_usage;
}
