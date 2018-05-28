//
//  ticker.hpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/25/18.
//  Copyright © 2018 home. All rights reserved.
//

#ifndef TICKER_HPP
#define TICKER_HPP

#include <iostream>
#include <future>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "common.hpp"
using namespace std;

extern mutex ticker_m;
extern condition_variable ticker_cv;

// Ticker works as a heart beat of the simulation
// Elevator and simulator actions sychronize to a global ticker
#pragma mark class

class Ticker
{
public:
    virtual ~Ticker() {}
    
    static shared_ptr<Ticker> make_ticker();
    static shared_ptr<Ticker> make_ticker(float _rate);
    
    bool start();
    void stop();
    
    float get_rate () const { return rate ;};
        
    long get_interval() const {return interval.count();}
    
    uint64_t get_tick () const {return ticks;}
    
    SIMULATION_ERROR set_rate (float _rate);
    
protected:
    future<void> run();

    void tickWrapper() ;
    
    promise<void> stop_tick;
    shared_future<void> done;
    mutable float rate = 1.0;
    mutable chrono::duration<long, milli>  interval = 1000ms; // base beat
    atomic <uint64_t> ticks{0};
    thread ticker_thread;
    
private:
    static shared_ptr<Ticker> instance ;
    
    Ticker(float _rate) ;
    Ticker() ;
};


#endif /* TICKER_HPP */
