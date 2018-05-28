//
//  elevator_test.cpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/25/18.
//  Copyright © 2018 home. All rights reserved.
//
 
#include <thread>
#include <random>

#include "../elevator_simulation/common.hpp"
#include "../elevator_simulation/ticker.hpp"
#include "../elevator_simulation/elevator.hpp"
#include "../elevator_simulation/events.hpp"
#include "./test_common.hpp"

// the test is to perform elevator related commands:
// Set Speed, Set Floors, Set Start Floor, Goto, Start, Stop

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
        case COMMAND::START:
            elv.start();
            break;
        default:
            break;
    }
    return;
}

int main()
{
    using namespace std;
    
    auto ticker = Ticker:: make_ticker(100);
    ticker->start();
    
    auto test_data = read_all_lines("./elevator_test.txt");
    while (!test_data.empty()) {
        cout<<"========================================"<<endl;
        default_random_engine rand_gen((random_device())());
        // simulate the each arrival time of goto commands to the elevator as an independant event
        // uniform random distribution between [current_time, future_time) 20 ticks away.ideally should be max
        uniform_int_distribution<uint64_t>distribution(0,20);
        
        Elevator test_elevator(ticker);
        
        shared_ptr<queue<pair<Command, uint64_t>>> command_streamer = make_shared<queue<pair<Command, uint64_t>>>();
        CommandGenerator cmd_gen(command_streamer, ticker,test_data.back());
        cmd_gen.start();
        // this block simulate the simulator event dispatch
        while(1){
            auto lock = unique_lock<mutex>(event_m);
            while (command_streamer->empty() && !cmd_gen.is_exited())
                event_cv.wait(lock);
            
            // stop reading from input
            if (cmd_gen.is_exited()) {
                cout<<"Quit issued @ time "<< ticker->get_tick()<< endl;
                lock.unlock();
                break;
            }
            // manually change the arrival time here
            // otherwise it is from input
            auto draw = distribution(rand_gen);
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
            
            lock.unlock();
            
        }
        // flush command streamer
        cmd_gen.stop();
        test_data.pop_back();
    }

    ticker->stop();
    return 0;
}
