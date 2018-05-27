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
#pragma mark class

class Ticker
{
public:
    virtual ~Ticker() { cout<<"Ticker stopped"<<endl; }
    
    static shared_ptr<Ticker> make_ticker()
    {
        if (!instance)
            instance = shared_ptr<Ticker>(new Ticker());
        return instance;
    }
    
    static shared_ptr<Ticker> make_ticker(float _rate)
    {
        if (!instance)
            instance = shared_ptr<Ticker>(new Ticker(_rate));
        return instance;
    }
    
    bool start() {
        ticker_thread = thread([&] {
            run();
        });
        return true;
    }
    void stop() {
        stop_tick.set_value();
        if (ticker_thread.joinable())
            ticker_thread.join();
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
        //shared_future<void >done = stop_tick.get_future();
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
    
    promise<void> stop_tick;
    shared_future<void> done;
    mutable float rate = 1.0;
    mutable chrono::duration<long, milli>  interval = 1000ms; // base beat
    atomic <uint64_t> ticks{0};
    thread ticker_thread;
    
private:
    static shared_ptr<Ticker> instance ;
    
    Ticker(float _rate) {
        set_rate(_rate);
        done = stop_tick.get_future();
    }
    Ticker() {
        done = stop_tick.get_future();
    }
};

shared_ptr<Ticker>  Ticker::instance = nullptr;

#endif /* TICKER_HPP */
