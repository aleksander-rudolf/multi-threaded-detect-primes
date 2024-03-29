#include "detectPrimes.h"
#include <chrono>
#include <iomanip>
#include <iostream>

namespace forisek { bool is_prime(uint64_t x); };

struct Timer {
  // return elapsed time (in seconds) since last reset/or construction
  // reset_p = true will reset the time
  double elapsed(bool resetFlag = false)
  {
    double result = 1e-6
        * std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::steady_clock::now() - start)
              .count();
    if (resetFlag) reset();
    return result;
  }
  // reset the time to 0
  void reset() { start = std::chrono::steady_clock::now(); }
  Timer() { reset(); }

  private:
  std::chrono::time_point<std::chrono::steady_clock> start;
};

int main(int argc, char ** argv)
{
  /// parse command line arguments
  int nThreads = 1;
  if (argc != 1 && argc != 2) {
    std::cout << "Usage: " << argv[0] << " [nThreads]\n"
              << "    the default for nThreads is 1 thread.\n";
    exit(-1);
  }
  if (argc == 2) nThreads = atoi(argv[1]);

  /// handle invalid arguments
  if (nThreads < 1 || nThreads > 256) {
    std::cout << "Bad arguments. 1 <= nThreads <= 256!\n";
    exit(-1);
  }
  std::cout << "Using " << nThreads << " thread" << (nThreads == 1 ? "" : "s")
            << ".\n";
  std::vector<int64_t> nums;
  while (1) {
    int64_t num;
    if (!(std::cin >> num)) break;
    nums.push_back(num);
  }
  /// time detect_primes()
  Timer t;
  std::vector<int64_t> primes = detect_primes(nums, nThreads);
  double elapsed = t.elapsed();

  /// report results
  std::cout << "Identified " << primes.size() << " primes:\n";
  constexpr size_t max_line_width = 77;
  std::string line;
  for (auto num : primes) {
    auto numstr = std::to_string(num);
    if( line.empty()) { line = numstr; continue; }
    std::string line2 = line + " " + numstr;
    if( line2.size() > max_line_width) {
      std::cout << "  " << line << "\n";
      line = numstr;
    } else {
      line = line2;
    }
  }
  if( line.size()) std::cout << "  " << line << "\n";

  std::cout << "\nFinished in " << std::fixed << std::setprecision(4) << elapsed
            << "s\n";
  return 0;
}
