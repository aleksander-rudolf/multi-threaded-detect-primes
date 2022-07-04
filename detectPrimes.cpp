#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

//C++ barrier class (from lecture notes).
class simple_barrier {
  std::mutex m_;
  std::condition_variable cv_;
  int n_remaining_, count_;
  bool coin_;

  public:
  simple_barrier(int count = 1) { init(count); }
  void init(int count)
  {
    count_ = count;
    n_remaining_ = count_;
    coin_ = false;
  }
  bool wait()
  {
    if (count_ == 1) return true;
    std::unique_lock<std::mutex> lk(m_);
    if (n_remaining_ == 1) {
      coin_ = ! coin_;
      n_remaining_ = count_;
      cv_.notify_all();
      return true;
    }
    auto old_coin = coin_;
    n_remaining_--;
    cv_.wait(lk, [&]() { return old_coin != coin_; });
    return false;
  }
};

//Returns true if n is prime, otherwise returns false
static bool is_prime(int64_t n)
{
  //Handle trivial cases
  if (n < 2) return false;
  if (n <= 3) return true; // 2 and 3 are primes
  if (n % 2 == 0) return false; // handle multiples of 2
  if (n % 3 == 0) return false; // handle multiples of 3
  //Try to divide n by every number 5 .. sqrt(n)
  int64_t i = 5;
  int64_t max = sqrt(n);
  while (i <= max) {
    if (n % i == 0) return false;
    if (n % (i + 2) == 0) return false;
    i += 6;
  }
  //Didn't find any divisors, so it must be a prime
  return true;
}

//This function takes a list of numbers in nums[] and returns only numbers that
//are primes.
//
//The parameter n_threads indicates how many threads should be created to speed
//up the computation.

std::vector<int64_t> result;
std::atomic<int64_t> num = 0;
std::atomic<int> indx = 0;
std::atomic<bool> global_finished = false;
std::atomic<bool> global_prime = true;
std::atomic<bool> first_element_checked = false;
simple_barrier barrier = simple_barrier();

void threadFunction(std::vector<int64_t> nums, int tid, int n_threads){

  while(true){
    //Serial section
    if(0 != barrier.wait()){
      if(global_prime && first_element_checked){
        result.push_back(num);
      }
      else{
        global_prime = true;
      }
      first_element_checked = true;
      while(true){
        if(indx >= (int64_t)nums.size()){
          global_finished = true;
          break;
        }
        else{
          //Check if small numbers <= 1024 are prime in a single-thread.
          num = nums.at(indx++);
          if(num <= 1024){
            if(is_prime(num)){
              result.push_back(num);
              continue;
            }
          }
          else{
            if(num % 2 == 0 || num % 3 == 0){
              continue;
            }
            else{
              break;
            }
          }
        }
      }
    }
    //Parrallel section
    barrier.wait();
    //Check if large number > 1024 are prime use multiple-threads.
    int64_t i = 5 + 6*tid;
    int64_t max = sqrt((double)num);
    if(global_finished) break;
    while(i <= max){
      if(!global_prime) break;
      if(num % i == 0) global_prime = false;
      if(num % (i + 2) == 0) global_prime = false;
      i += 6*n_threads;
    }
  }
  return;
}

std::vector<int64_t> detect_primes(const std::vector<int64_t> & nums, int n_threads){

  //Initialize thread_pool
  std::thread thread_pool[n_threads];
  barrier.init(n_threads);

  //Create threads
  for (int i = 0; i < n_threads; i++) {
    thread_pool[i] = std::thread(threadFunction, nums, i, n_threads);
  }

  //Join threads
  for (int i = 0; i < n_threads; i++) {
    if (thread_pool[i].joinable()) thread_pool[i].join();
  }  

  return result;
}