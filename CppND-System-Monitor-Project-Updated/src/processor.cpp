#include <unistd.h>

#include "processor.h"
#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() { 
    values = LinuxParser::CpuUtilization();
    int user = std::stoi(values[0]);
    int nice = std::stoi(values[1]);
    int system = std::stoi(values[2]);
    int idle = std::stoi(values[3]);
    int iowait = std::stoi(values[4]);
    int irq = std::stoi(values[5]);
    int softirq = std::stoi(values[6]);
    int steal = std::stoi(values[7]);
    int guest = std::stoi(values[8]);
    int guest_nice = std::stoi(values[9]);
    user = user - guest;
    nice = nice - guest_nice;
    int idlealltime = idle + iowait;
    int totaltime = user + nice + system + irq + softirq + idlealltime + steal
        + guest + guest_nice;

    sleep(1);

    values = LinuxParser::CpuUtilization();
    user = std::stoi(values[0]);
    nice = std::stoi(values[1]);
    system = std::stoi(values[2]);
    idle = std::stoi(values[3]);
    iowait = std::stoi(values[4]);
    irq = std::stoi(values[5]);
    softirq = std::stoi(values[6]);
    steal = std::stoi(values[7]);
    guest = std::stoi(values[8]);
    guest_nice = std::stoi(values[9]);
    user = user - guest;
    nice = nice - guest_nice;
    int newIdlealltime = idle + iowait;
    int newTotaltime = user + nice + system + irq + softirq + newIdlealltime + 
        steal + guest + guest_nice;

    float delta_total = newTotaltime - totaltime;
    float delta_idle = newIdlealltime - idlealltime;
    float cpu_percentage = (delta_total - delta_idle) / delta_total;

    return cpu_percentage;
}