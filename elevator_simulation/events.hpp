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
static mutex event_m;
static condition_variable event_cv;

// CommandGenerator class takes input from stdin in async fashion and
// pushes commands into the command queue that is interfacing
// with the simulator
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
    
    // start getting user input from the stdin
    void start() {
        io_thread =  thread([&] {
            // for testing only. direct string to cin
            std::stringstream test_cin;
            streambuf *cinbuf = nullptr;
            if (seq.length()>0) {
                test_cin << seq;
                cinbuf = cin.rdbuf();
                cin.rdbuf(test_cin.rdbuf());
            }
            string token;
            string line;
            stringstream iss;
            while(getline(cin, line)) {
                iss << line;
                while(getline(iss, token, ',')) {
                    transform(token.begin(), token.end(), token.begin(), ::tolower);
#if DEBUG_COMMAND
                    cout<<"receiving: "<<token<<endl;
#endif
                    auto cmd = parse_input(token);
                    if (cmd.type!= COMMAND::INVALID_COMMAND) {
                        lock_guard<mutex> lock(event_m);
                        // get the command and its arrival timestamp
                        command_streamer->emplace(cmd, ticker->get_tick());
                        //cout<<"command streamer size: "<<command_streamer->size()<<endl;
                        event_cv.notify_all();
                    } else
                        cout<<"ERROR: "<< token <<" invalid command" <<endl;
                }
            }
            lock_guard<mutex> lock(event_m);
            if (!exited.load())
                exited.store(true,memory_order_release);
            event_cv.notify_all();
            
            //restore cin
            if (seq.length()>0) {
                cin.rdbuf(cinbuf);
            }
        });
    }
    void stop() {
        io_thread.join();
    }
    
    bool is_exited( ) const { return exited.load(memory_order_acquire); }
protected:
    shared_ptr<queue<pair<Command,uint64_t>>>command_streamer;
    shared_ptr<Ticker> ticker;
    string seq; // testing sequence
    thread  io_thread;
    mutable atomic<bool> exited{false};
    
    // parse a line of string and extract meanful commands
    Command parse_input(string &s) {
        // a legal  command is defined as command header  + a parameter
        // such as Set rate 0.5/Set floor 10
        // once command header is parsed, no letter should be inteleaved with
        // actual parameter
        // for example Set Rate s-0.3 is a invalid command while Set Rate -0.3 is

        if (s.find(set_rate_token) != string::npos )
            return parse_value(s.substr(s.find(set_rate_token) + strlen(set_rate_token)), COMMAND::SET_RATE);
        else if (s.find(set_speed_token)!= string::npos)
            return parse_value(s.substr(s.find(set_speed_token) + strlen(set_speed_token)), COMMAND::SET_SPEED);
        else if (s.find(set_floors_token)!= string::npos)
            return parse_value(s.substr(s.find(set_floors_token) + strlen(set_floors_token)), COMMAND::SET_FLOORS);
        else if (s.find(set_start_floor_token)!= string::npos)
            return parse_value(s.substr(s.find(set_start_floor_token) + strlen(set_start_floor_token)), COMMAND::SET_START_FLOOR);
        else if (s.find(goto_token)!= string::npos)
            return parse_value(s.substr(s.find(goto_token) + strlen(goto_token)), COMMAND::GOTO);
        else if (s.find(start_token)!= string::npos)
            return Command(COMMAND::START);
        else if (s.find(stop_token)!= string::npos)
            return Command(COMMAND::STOP);
        else
            return Command(COMMAND::INVALID_COMMAND,-1.0);
    }
    
    // parse a string and extract number from it and form a command
    Command parse_value(string s, COMMAND type) {
        
        regex re("\\s+[+-]?([0-9]*[.])?[0-9]+");
        smatch match;
        
        if (s.length() >0 ) {
            sregex_iterator next(s.begin(), s.end(), re);
            sregex_iterator end= sregex_iterator();;
            if (next != end) {
                smatch match = *next;
#if 0
                cout << match.str()<<endl;
#endif
                return Command(type, stof (match.str()));
            }else {
                //cout<<"ERROR: Invalid Command. No valid parameter found" << endl;
                return Command(COMMAND::INVALID_COMMAND,-1.0);
            }
        }else {
            //cout<<"ERROR: No parameter given. Use defaut value"<<endl;
            return Command(type);
        }
    }
};
#endif /* EVENTS_HPP */
