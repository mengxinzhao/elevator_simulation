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
// is controlled by ticker's interval setting. To the outside, a tick is a basic time unit.
// Elevator's speed is in relative term to the tick. Ticker is a singleton object in the
// simulation for the reason that if there is more object to simulate, the tick remains global.
//
// To receive control software input it uses async_io CommandGenerator to
// extract all valid commands and push them to a command streamer that feeds event dispatcher.
// Event dispatcher dispatches commands to a mock elevator and receives its feedback. This
// data path has no respect to the simulator ticks so to be close to reality as much as possible.
//
// Elevator is a simple state machine based. Its action synchronizes to
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
    
    Simulator ();
    virtual ~Simulator(){}

    void run();
    
    SIMULATION_STATE get_state() const { return state; }
    
protected:
    // start and stop member functions are not exposed
    bool start();
    void stop();
    
    // dispatch the command and set the simulation state
    SIMULATION_ERROR dispatch_command(Command &cmd);
    
    shared_ptr<Ticker> ticker = nullptr;
    shared_ptr<queue<pair <Command, uint64_t> > > command_streamer= nullptr;
    shared_ptr<CommandGenerator>  command_generator= nullptr;
    shared_ptr<Elevator> elevator = nullptr;
    atomic<SIMULATION_STATE> state{SIMULATION_STATE::UNINITIALIZED};
    
};

#endif /* SIMULATOR_HPP */
