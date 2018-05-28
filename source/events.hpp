//
//  events.hpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/24/18.
//  Copyright © 2018 home. All rights reserved.
//

#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <regex>
#include <fstream>
#include <atomic>

#include "common.hpp"
#include "ticker.hpp"

using namespace std;

// mutex and conditional wait signal to synchronize between command generator and simulator
extern mutex event_m;
extern condition_variable event_cv;

// CommandGenerator class takes input from stdin in async fashion and
// pushes commands into the command queue that is interfacing
// with the simulator
#pragma mark class
class  CommandGenerator {
public:
    CommandGenerator(shared_ptr<queue<pair<Command,uint64_t>>>_streamer_ptr,
                     shared_ptr<Ticker> _tiker):
                    command_streamer(_streamer_ptr), ticker(_tiker) {};
    CommandGenerator(shared_ptr<queue<pair<Command,uint64_t>>>_streamer_ptr,
                     shared_ptr<Ticker> _ticker,
                     string _seq):
                    command_streamer(_streamer_ptr),ticker(_ticker), seq(_seq) {}
    virtual ~CommandGenerator() {}
    
    // start/stop getting user input from the stdin
    void start();
    void stop();
    
    bool is_exited( ) const { return exited.load(memory_order_acquire); }
protected:
    shared_ptr<queue<pair<Command,uint64_t>>>command_streamer;
    shared_ptr<Ticker> ticker;
    string seq; // testing sequence
    thread  io_thread;
    mutable atomic<bool> exited{false};
    
    // parse a line of string and extract meanful commands
    Command parse_input(string &s);
    
    // parse a string and extract number from it and form a command
    Command parse_value(string s, COMMAND type);
};
#endif /* EVENTS_HPP */

