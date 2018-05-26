//
//  events_test.cpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/24/18.
//  Copyright © 2018 home. All rights reserved.
//


#include <queue>
#include <string>

#include "../elevator_simulation/events.hpp"
#include "../elevator_simulation/common.hpp"

// The test is to test command generator properly picking up valid commands
// extracting valid parameters and pushing to command queue in asychronized fashion
// between control software and simulator

int main()
{
    using namespace std;

    auto command_streamer= make_shared<queue<Command>>();
    CommandGenerator cmd_gen(command_streamer, "./events_test.txt");
    cmd_gen.start();
    while(1){
        auto lock = unique_lock<mutex>(event_m);
        while (command_streamer->empty()||!cmd_gen.is_exited())
            event_cv.wait(lock);
        if (cmd_gen.is_exited()) {
            lock.unlock();
            break;
        }
#if DEBUG
        cout<<"command streamer received: "<<command_streamer->front().type << ","<< command_streamer->front().param <<endl;
#endif
        command_streamer->pop();
        lock.unlock();
        cout.flush();
    }
    
    while(command_streamer->size()) {
#if DEBUG
        cout<<"command streamer received: "<<command_streamer->front().type << ","<< command_streamer->front().param <<endl;
#endif
        command_streamer->pop();
    }
    cmd_gen.stop();
    return 0;
}
