//
//  events_test.cpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/24/18.
//  Copyright © 2018 home. All rights reserved.
//


#include <queue>
#include <string>
#include <iostream>
#include <iomanip>

#include "../elevator_simulation/events.hpp"
#include "../elevator_simulation/common.hpp"
#include "../elevator_simulation/ticker.hpp"
#include "test_common.hpp"

// The test is to test command generator properly picking up valid commands
// extracting valid parameters and pushing to command queue in asychronized fashion
// between control software and simulator
const char* parse_type(COMMAND type) {
    switch (type) {
        case COMMAND::INVALID_COMMAND:
            return "invalid command";
        case COMMAND::SET_RATE:
            return set_rate_token;
        case COMMAND::SET_SPEED:
            return  set_speed_token;
        case COMMAND::SET_FLOORS:
            return  set_floors_token;
        case COMMAND::SET_START_FLOOR:
            return set_start_floor_token;
        case COMMAND::START:
            return start_token;
        case COMMAND::STOP:
            return stop_token;
        case COMMAND::GOTO:
            return goto_token;
        default:
            return " ";
    }
}

int main()
{
    using namespace std;

    auto t1 = Ticker:: make_ticker();
    t1->set_rate(10);
    t1->start();
    // let the test clock start
    std::this_thread::sleep_for(50ms);
    
    auto test_data = read_all_lines("./elevator_test.txt");
    
    while (!test_data.empty()) {
        cout<<"========================================"<<endl;
        auto command_streamer= shared_ptr<queue<pair<Command, uint64_t>> > (new  queue<pair<Command, uint64_t>> );
        CommandGenerator cmd_gen(command_streamer, t1, test_data.back());
        cmd_gen.start();
        
        while(1){
            auto lock = unique_lock<mutex>(event_m);
            while (command_streamer->empty() && !cmd_gen.is_exited())
                event_cv.wait(lock);
            
            // stop reading from input
            if (cmd_gen.is_exited()) {
                cout<<"Quit issued"<<endl;
                lock.unlock();
                break;
            }
#if DEBUG
            Command cmd = command_streamer->front().first;
            uint64_t time_stamp = command_streamer->front().second;
            cout<<"command streamer received: "<<parse_type(cmd.type) << ","<< setprecision(4)<< cmd.param << endl;
#endif
            command_streamer->pop();
            lock.unlock();
            cout.flush();
        }
        
        while(command_streamer->size()) {
#if DEBUG
            Command cmd = command_streamer->front().first;
            uint64_t time_stamp = command_streamer->front().second;
            cout<<"command streamer received: "<<parse_type(cmd.type) << ","<< setprecision(4)<<cmd.param <<endl;
#endif
            command_streamer->pop();
        }
        cmd_gen.stop();
        test_data.pop_back();
    }
    
    // stop tickers
    t1->stop();

    return 0;
}
