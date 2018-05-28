//
//  elevator.hpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/25/18.
//  Copyright © 2018 home. All rights reserved.
//

#ifndef ELEVATOR_HPP
#define ELEVATOR_HPP

#include "common.hpp"
#include "ticker.hpp"

#include <utility>
#include <stack>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <iomanip>

using namespace std;

// Elevator is a mock object that takes simulation input command and perform a correspoding action
#pragma mark class

class Elevator {
public:
    Elevator( shared_ptr<Ticker> _ticker): ticker(_ticker) {}
    virtual ~Elevator();
    
    SIMULATION_ERROR set_speed (float _speed );
    float get_speed () const { return speed; }
    
    SIMULATION_ERROR set_floors(float  _floors);
    float get_floors() const {return number_floors ;}
    
    SIMULATION_ERROR set_start_floor (float _start_floor );
    float get_start_floor() const {return start_floor; }
    
    SIMULATION_ERROR start();
    SIMULATION_ERROR stop();
    
    SIMULATION_ERROR goto_floor (float _floor_num);
    float get_current_loc() const { return location; }
    
    bool is_running () const {return running.load();}
    bool is_idle() const { return running.load() && pending_loc<0; } // started but no new task assigned
    
protected:
    mutable float speed = 0.5 ; // 0.5 floor per second
    mutable float number_floors = 4;
    mutable float start_floor = 1;
    
    //a simplied state machine of elevator
    atomic<bool>  running{false};
    float location = 1;
    float pending_loc = -1;
    float next_loc = 1;

    thread run_thread;
    shared_ptr<Ticker> ticker;
};

#endif /* ELEVATOR_HPP */
