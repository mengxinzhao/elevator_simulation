//
//  simulator.hpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/26/18.
//  Copyright © 2018 home. All rights reserved.
//

#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <iostream>
#include <memory>
#include <thread>

#include "common.hpp"
#include "elevator.hpp"
#include "ticker.hpp"
#include "events.hpp"

// Simulator class is the major component interfacing with control software/
//
// Simulator has a Ticker that synchronizes elevator events to its ticks. Simulation speed
// is controlled by ticker's interval setting. To the outside, a tick is a baisc time unit.
// Elevator's speed is in relative term to the tick. Ticker is a singleton object in the
// simulation for the reason that if there is more object to simulate, the tick remains global.
//
// To receive control software input it uses async_io CommandGenerator to
// extract all valid commands and push them to a command streamer that feeds event dispatcher.
// Event dispatcher dispatches commands to a mock elevator and receives its feedback. This
// data path has no respect to the simulator ticks so to be close to reality as much as possible.
//
// Elevator is a simple state machine based. Its action sychronizes to
// the simulator tick. When a command is dispatched in the middle of tick interval,
// elevator's response is delayed at most by 1 tick.
//

#pragma mark helper functions
template <typename T,typename... Args>
shared_ptr<T> create_shared_ptr(Args &&...args){
    shared_ptr<T> instance = nullptr;
    try {
         instance = make_shared<T>(args...);
    }catch  (const std::runtime_error& e) {
        cout<<e.what() << endl;
        throw Simulation_Error(SIMULATION_ERROR::BAD_MEMORY, "allocation error!");
    }
    return instance;
}

#pragma mark class
class Simulator {
public:
    //simulator state machine
    enum class SIMULATION_STATE {
        UNINITIALIZED = -1,        // nothing works for now
        INITIALIZED,               // ticker and command streamer are in place for action
        STOPPED,                   // elevator stopped moving
        STARTED,                   // elevator started moving
    };
    
    Simulator () {
        // create components
        ticker = Ticker::make_ticker();
        command_streamer = create_shared_ptr<queue<pair<Command, uint64_t>>>();
        command_generator = create_shared_ptr<CommandGenerator>(command_streamer, ticker);
        elevator = create_shared_ptr<Elevator>(ticker);
    }
    virtual ~Simulator(){}

    void run() {
        start();
        while (true) {
            auto lock = unique_lock<mutex>(event_m);
            while (command_streamer->empty() && !command_generator->is_exited())
                event_cv.wait(lock);
            
            // stop reading from input
            if (command_generator->is_exited()) {
                cout<<"Quit issued @ time "<< ticker->get_tick()<< endl;
                lock.unlock();
                break;
            }
            
            Command cmd = command_streamer->front().first;
            command_streamer->pop();
            lock.unlock();
            dispatch_command(cmd);

        }
        stop();
    }
    
    SIMULATION_STATE get_state() const { return state; }
    
protected:
    // start and stop member functions are not exposed
    bool start() {
        if (state.load(memory_order_acquire) >= SIMULATION_STATE::INITIALIZED)
            return true;
        ticker->start();
        command_generator->start();
        state = SIMULATION_STATE::INITIALIZED;
        return true;
    }

    void stop() {
        if (state.load(memory_order_acquire)>= SIMULATION_STATE::INITIALIZED) {
            // if users issue quit without stopping the elevator we have to stop it first
            elevator->stop();
            // stop command listening and dispatch
            command_generator->stop();
            // stop the clock
            ticker->stop();
            // commit state change
            state.store( SIMULATION_STATE::UNINITIALIZED, memory_order_release);
        }
    }
    
    // dispatch the command and set the simulation state
    SIMULATION_ERROR dispatch_command(Command &cmd) {
        
        // check its basic initialization status
        if (state.load(memory_order_acquire) < SIMULATION_STATE::INITIALIZED){
            cout<<"ERROR: Simulator not initialized "<<endl;
            return SIMULATION_ERROR:: INVALID_STATE;
        }
        
        SIMULATION_ERROR error = SIMULATION_ERROR::SUCCESS;
        switch (cmd.type) {
            case COMMAND::SET_RATE:
                // rate can be set any time after simulator initialized
                error = ticker->set_rate(cmd.param);
                break;
                
            case COMMAND::SET_FLOORS:
                error = elevator->set_floors(cmd.param);
                break;
                
            case COMMAND::SET_START_FLOOR:
                error = elevator->set_start_floor(cmd.param);
                break;
                
            case COMMAND::SET_SPEED:
                error = elevator->set_speed(cmd.param);
                break;
                
            case COMMAND::GOTO:
                error= elevator->goto_floor(cmd.param);
                break;
                
            case COMMAND::START:
                error = elevator->start() ;
                // commit state change only after elevator says so
                if (error == SIMULATION_ERROR::SUCCESS)
                    state.store( SIMULATION_STATE::STARTED, memory_order_release);
                break;
                
            case COMMAND::STOP:
                error= elevator->stop();
                if (error == SIMULATION_ERROR::SUCCESS)
                    state.store( SIMULATION_STATE::STOPPED, memory_order_release);
                break;
                
            default:
                error = SIMULATION_ERROR::INVALID_PARAMETER;
                break;
        }
        return error;
    }
    
    shared_ptr<Ticker> ticker = nullptr;
    shared_ptr<queue<pair <Command, uint64_t> > > command_streamer= nullptr;
    shared_ptr<CommandGenerator>  command_generator= nullptr;
    shared_ptr<Elevator> elevator = nullptr;
    atomic<SIMULATION_STATE> state{SIMULATION_STATE::UNINITIALIZED};
    
};

#endif /* SIMULATOR_HPP */
