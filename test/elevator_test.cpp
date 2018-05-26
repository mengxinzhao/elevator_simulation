//
//  elevator_test.cpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/25/18.
//  Copyright © 2018 home. All rights reserved.
//

#include <future>
#include <thread>
#include <random>

#include "../elevator_simulation/common.hpp"
#include "../elevator_simulation/ticker.hpp"
#include "../elevator_simulation/elevator.hpp"
#include "../elevator_simulation/events.hpp"

// the test is to perform elevator related commands:
// Set Speed, Set Floors, Set Start Floor, Goto
// Set Rate/Start/Stop is simulator level command
// But Here I use Stop to stop the testing

void dispatch_elevator_command(Command cmd, Elevator &elv) {
    switch (cmd.type) {
        case COMMAND::SET_SPEED:
            elv.set_speed(cmd.param);
            break;
        case COMMAND::SET_FLOORS:
            elv.set_floors(cmd.param);
            break;
        case COMMAND::SET_START_FLOOR:
            elv.set_start_floor(cmd.param);
            break;
        case COMMAND::GOTO:
            elv.goto_floor(cmd.param);
            break;
        case COMMAND::STOP:
            elv.stop();
            break;
        default:
            break;
    }
    return;
}

int main()
{
    using namespace std;
    
    std::promise<void> done;
    std::shared_future<void> done_future(done.get_future());
    
    auto ticker = Ticker:: make_ticker(done_future);
    ticker->set_rate(10);
    thread ticker_thread = thread([&]{
        ticker->run();
    });
    
    this_thread::sleep_for(1s);
    
    default_random_engine rand_gen((random_device())());
    // simulate the each arrival time of goto commands  to the elevator as an independant event
    // uniform random distribution between [current_time, future_time) 100 ticks away.ideally should be max
    uniform_int_distribution<uint64_t>distribution(0,10);
    
    Elevator test_elevator(ticker);
    
    auto command_streamer= shared_ptr<queue<pair<Command, uint64_t>> > (new  queue<pair<Command, uint64_t>> );
    CommandGenerator cmd_gen(command_streamer, ticker, "./elevator_test.txt");
    cmd_gen.start();
    // this block simulate the simulator event dispatch
    while(1){
        auto lock = unique_lock<mutex>(event_m);
        while (command_streamer->empty() && !cmd_gen.is_exited())
            event_cv.wait(lock);
        
        // manually change the arrival time here
        // otherwise it is from input
        auto draw = 5;//distribution(rand_gen);
        command_streamer->front().second += draw;
        
        // wait till the event becomes current
        unique_lock<mutex> lk(ticker_m);
        while (ticker->get_tick() < command_streamer->front().second)
            ticker_cv.wait(lk);
        lk.unlock();
#if 0
        Command cmd = command_streamer->front().first;
        uint64_t time_stamp = command_streamer->front().second;
        cout<<"command streamer received: "<<cmd.type << ","<< cmd.param <<" @ " << time_stamp<< endl;
#endif
        dispatch_elevator_command(command_streamer->front().first, test_elevator);
        command_streamer->pop();
        
        // stop reading from input
        if (cmd_gen.is_exited()) {
            lock.unlock();
            cout<<"QUIT issued "<< endl;
            cout<<"Command remains:" << command_streamer->size()<<std::endl;
            break;
        }
        
        lock.unlock();
        cout.flush();
    }
    // flush command streamer

    command_streamer.reset();
    cmd_gen.stop();

    done.set_value();
    ticker_thread.join();
    ticker.reset();
    
    return 0;
}
