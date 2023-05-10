#pragma once

#include <chrono>

struct Timer {
  std::chrono::high_resolution_clock::time_point st;
  Timer(){ start(); }
  void start(){ st = std::chrono::high_resolution_clock::now(); }
  std::chrono::milliseconds::rep result(){
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end-st).count();
  }
};
