#ifndef LATENCY_PROFILE_H_
#define LATENCY_PROFILE_H_

#include <cmath>
#include <cstdio>
#include <ctime>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#if TEST_LATENCY_CALCULATIONS
#include <iostream>
#endif

#define NANO_MULTIPLIER 1000000000

class LatencyProfilingStats {
   private:
    std::timespec startTime, endTime, delta;
    std::map<unsigned long, unsigned long> latencies;
    std::string identifier;
    bool started_;

    void sub_timespec() {
        delta.tv_sec = endTime.tv_sec - startTime.tv_sec;
        delta.tv_nsec = endTime.tv_nsec - startTime.tv_nsec;
        if (delta.tv_sec >= 0 && delta.tv_nsec < 0) {
            delta.tv_nsec += NANO_MULTIPLIER;
            delta.tv_sec--;
        } else if (delta.tv_sec < 0 && delta.tv_nsec) {
            delta.tv_nsec -= NANO_MULTIPLIER;
            delta.tv_sec++;
        }
    }

    void add_to_latency() {
        unsigned long latency = delta.tv_sec * NANO_MULTIPLIER + delta.tv_nsec;
        if (latencies.find(latency) != latencies.end()) {
            latencies[latency]++;
        } else {
            latencies[latency] = 1;
        }
    }

   public:
    std::string get_the_stats() {
        if (latencies.empty()) {
            return "Latency of " + identifier + " Is empty";
        }

        unsigned long n = 0;
        unsigned long total_sum = 0;
        unsigned long mean = 0;
        unsigned long median = 0;
        std::vector<unsigned long> mode;
        unsigned long max_count = 0;
        unsigned long lower_range = 1000000;
        unsigned long upper_range = 0;

        std::map<double, unsigned long> percentiles_values = {
            {1, 0},  {10, 0},   {25, 0},    {50, 0},     {75, 0},     {90, 0},
            {99, 0}, {99.9, 0}, {99.99, 0}, {99.999, 0}, {99.9999, 0}};

        for (auto it = latencies.begin(); it != latencies.end(); it++) {
            n += it->second;                      // Total number of elements
            total_sum += it->first * it->second;  // Total summation of elements

            // Lower Range
            if (lower_range > it->first) {
                lower_range = it->first;
            }

            // Upper Range
            if (upper_range < it->first) {
                upper_range = it->first;
            }

            // Mode calculation
            if (max_count < it->second) {
                max_count = it->second;
                mode.clear();
                mode.push_back(it->first);
            } else if (max_count == it->second) {
                mode.push_back(it->first);
            }

#if TEST_LATENCY_CALCULATIONS
            std::cout << it->first << ": " << it->second << std::endl;
#endif
        }

        for (auto it = percentiles_values.begin();
             it != percentiles_values.end(); it++) {
            it->second = it->first * n / 100;
        }

        unsigned long counter = 0;
        auto it = latencies.begin();
        auto it1 = percentiles_values.begin();

        while (it1 != percentiles_values.end() && it != latencies.end()) {
            if (counter + it->second < it1->second) {
                counter += it->second;
                it++;
            } else {
                it1->second = it->first;
                it1++;
            }
        }

        mean = total_sum / n;
        median = percentiles_values[50];

        std::stringstream ss;
        ss << "Latency: " << identifier.c_str() << "\n";
        ss << "Mean: " << mean << "\n";
        ss << "Median: " << median << "\n";
        ss << "Mode: ";
        for (auto it = mode.begin(); it != mode.end(); it++) {
            ss << *it;
            if (it != mode.end())
                ss << ", ";
            else
                ss << "\n";
        }

        ss << "Range: " << lower_range << "(l), " << upper_range << "(u)\n";
        ss << "Percentiles: ";

        int percentiles_map_size = percentiles_values.size();
        int idx = 1;
        for (auto it = percentiles_values.begin();
             it != percentiles_values.end(); it++) {
            ss << "m" << it->first << "(" << it->second << ")";
            if (idx != percentiles_map_size)
                ss << ", ";
            else
                ss << "\n";
            idx++;
        }
        return ss.str();
    }

    LatencyProfilingStats(const std::string& iden) {
        this->identifier = iden;
        started_ = false;
    }

    void start() {
        started_ = true;
        clock_gettime(CLOCK_REALTIME, &startTime);
    }

    void end() {
        if (started_) {
            clock_gettime(CLOCK_REALTIME, &endTime);
            sub_timespec();
            add_to_latency();
        }
        started_ = false;
    }
};

class LatencyProfilingHelper {
   private:
    LatencyProfilingStats& latency_reference;

   public:
    explicit LatencyProfilingHelper(LatencyProfilingStats& _latency_pointer)
        : latency_reference(_latency_pointer) {
        latency_reference.start();
    }
    ~LatencyProfilingHelper() { latency_reference.end(); }
};

#endif /* LATENCY_PROFILE_H_ */
