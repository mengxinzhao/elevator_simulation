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

using namespace std;

static mutex ticker_m;
static condition_variable ticker_cv;

// Ticker works as a heart beat of the simulation
// Elevator and simulator actions sychronize to a global ticker
class Ticker
{
public:
    virtual ~Ticker() { cout<<"Ticker stopped"<<endl; }
    static shared_ptr<Ticker> make_ticker(shared_future<void> f, float _rate)
    {
        if (!instance)
            instance = shared_ptr<Ticker>(new Ticker( f,_rate));
        return instance;
    }
    
    static shared_ptr<Ticker> make_ticker(shared_future<void> f)
    {
        if (!instance)
           instance = shared_ptr<Ticker>(new Ticker(f));
        return instance;
    }
    
    // run the ticker
    future<void> run() {
        return async(launch::async, &Ticker::tickWrapper, this);
    }
    
    float get_rate () const { return rate ;};
        
    long get_interval() const {return interval.count();}
    
    uint64_t get_tick () const {return ticks;}
    
    bool set_rate (float _rate) {
        if (_rate > 0) {
            rate = _rate;
            interval = chrono::duration<long, ratio<1,1000> >(static_cast<int>(1000 / rate));
            cout<<"tick interval set up to "<< interval.count()<<" milliseconds"<<endl;
            return true;
        }else{
            cout<<"Rate can't be negative or 0" << endl;
            cout<<"tick interval remains "<< interval.count()<<" milliseconds"<<endl;
            return false;
        }
    }
    
protected:
    void tickWrapper() {
        future_status status;
        do {
            status = done.wait_for(interval); // waits for interval
            if (status == future_status::timeout) {
                // notify all listeners
                lock_guard<mutex> lock(ticker_m);
                ticks++;
#if DEBUG_TICKER
                cout<<"ticks: " << ticks<< endl;
#endif
                ticker_cv.notify_all();
            }
        } while (status != future_status::ready);
    }
    
    shared_future<void> done;
    mutable float rate = 1.0;
    mutable chrono::duration<long, milli>  interval = 1000ms; // base beat
    atomic <uint64_t> ticks{0} ;
    
private:
    static shared_ptr<Ticker> instance ;
    
    Ticker(shared_future<void> f, float _rate) : done{f} {
        set_rate(_rate);
    }
    
    Ticker(shared_future<void> f) : done{f} { }
};

shared_ptr<Ticker>  Ticker::instance = nullptr;

#endif /* TICKER_HPP */
