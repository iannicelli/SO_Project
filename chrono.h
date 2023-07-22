#pragma once
#include <iostream>
#include <sys/time.h>
using namespace std;

struct Chrono {
  double elapsed=0;
  Chrono(const std::string& name_):
    name(name_){
  }
  int num_events=0;
  std::string name;
  inline void print(ostream& os) {
    cerr << "Chrono| name: " << name
         << " Total: " << elapsed
         << " #events: " << num_events
         << " Avg: " <<  elapsed/num_events << endl;
  }
  struct Event {
    Chrono& _chrono;
    Event(Chrono& chrono_):
      _chrono(chrono_){
      gettimeofday(&t_start, 0);
    }
    inline ~Event() {
      struct timeval t_end;
      struct timeval duration;
      gettimeofday(&t_end, 0);
      timersub(&t_end, & t_start, &duration); 
      _chrono.elapsed += duration.tv_sec*1000.f+duration.tv_usec*1e-3;
      ++ _chrono.num_events;
    }
    struct timeval t_start;
  };
};
