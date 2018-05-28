//
//  simulator.cpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/24/18.
//  Copyright © 2018 home. All rights reserved.
//

#include <memory>
#include "simulator.hpp"

Simulator::Simulator () {
    // create components
    ticker = Ticker::make_ticker();
    command_streamer = create_shared_ptr<queue<pair<Command, uint64_t>>>();
    command_generator = create_shared_ptr<CommandGenerator>(command_streamer, ticker);
    elevator = create_shared_ptr<Elevator>(ticker);
}

void Simulator::run() {
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

// start and stop member functions are not exposed
bool Simulator::start() {
    if (state.load(memory_order_acquire) >= SIMULATION_STATE::INITIALIZED)
        return true;
    ticker->start();
    command_generator->start();
    state = SIMULATION_STATE::INITIALIZED;
    return true;
}

void Simulator::stop() {
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
SIMULATION_ERROR Simulator::dispatch_command(Command &cmd) {
    
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

