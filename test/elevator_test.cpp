//
//  elevator_test.cpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/25/18.
//  Copyright © 2018 home. All rights reserved.
//

#include "../elevator_simulation/common.hpp"
#include "../elevator_simulation/ticker.hpp"
#include "../elevator_simulation/elevator.hpp"
#include "../elevator_simulation/events.hpp"

// the test is to perform elevator related commands:
// Set Speed, Set Floors, Set Start Floor, Goto
// Set Rate is simulator level command

int main()
{
    using namespace std;
    
    std::promise<void> done;
    std::shared_future<void> done_future(done.get_future());
    
    auto ticker =Ticker:: make_ticker(done_future);
    ticker->set_rate(5);
    thread ticker_thread = thread([&]{
        ticker->run();
    });
    
    // read all lines of test sequence
    auto command_streamer= make_shared<queue<Command>>();
    CommandGenerator cmd_gen(command_streamer, "../test/elevator_test.txt");
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

    done.set_value();
    ticker_thread.join();
    return 0;
}
