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

// The test is to test command generator properly picking up valid commands
// extracting valid parameters and pushing to command queue in asychronized fashion
// between control software and simulator
const char* parse_type(COMMAND type) {
    switch (type) {
        case INVALID_COMMAND:
            return "invalid command";
        case SET_RATE:
            return set_rate_token;
        case SET_SPEED:
            return  set_speed_token;
        case SET_FLOORS:
            return  set_floors_token;
        case SET_START_FLOOR:
            return set_start_floor_token;
        case START:
            return start_token;
        case STOP:
            return stop_token;
        case GOTO:
            return goto_token;
        default:
            return " ";
    }
}

int main()
{
    using namespace std;

    std::promise<void> done;
    std::shared_future<void> done_future(done.get_future());
    
    auto t1 = Ticker:: make_ticker(done_future);
    t1->set_rate(10);
    
    thread ticker_thread = thread([&]{
        t1->run();
    });
    
    // let the test clock start
    std::this_thread::sleep_for(1s);
    
    auto command_streamer= shared_ptr<queue<pair<Command, uint64_t>> > (new  queue<pair<Command, uint64_t>> );
    CommandGenerator cmd_gen(command_streamer, t1, "./events_test.txt");
    cmd_gen.start();
    
    while(1){
        auto lock = unique_lock<mutex>(event_m);
        while (command_streamer->empty() && !cmd_gen.is_exited())
            event_cv.wait(lock);
        
        // stop reading from input
        if (cmd_gen.is_exited()) {
            lock.unlock();
            break;
        }
#if DEBUG
        Command cmd = command_streamer->front().first;
        uint64_t time_stamp = command_streamer->front().second;
        cout<<"command streamer received: "<<parse_type(cmd.type) << ","<< setprecision(2)<< cmd.param <<" @ " << time_stamp<< endl;
#endif
        command_streamer->pop();
        lock.unlock();
        cout.flush();
    }
    
    while(command_streamer->size()) {
#if DEBUG
        Command cmd = command_streamer->front().first;
        uint64_t time_stamp = command_streamer->front().second;
        cout<<"command streamer received: "<<parse_type(cmd.type) << ","<< setprecision(2)<<cmd.param <<" @ " << time_stamp<< endl;
#endif
        command_streamer->pop();
    }
    cmd_gen.stop();
    
    // stop ticker
    done.set_value();
    ticker_thread.join();

    return 0;
}
