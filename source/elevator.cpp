//
//  elevator.cpp
//  elevator_simulation
//
//  Created by Vesper on 5/27/18.
//  Copyright © 2018 home. All rights reserved.
//

#include "elevator.hpp"


extern mutex ticker_m;
extern condition_variable ticker_cv;

Elevator:: ~Elevator() {
    if ( running.load(memory_order_acquire))
        stop();
}

SIMULATION_ERROR Elevator:: set_speed (float _speed ) {
    // <= 0 test only for unit test of elevator
    // in real simulation invalid command will never get here
    if (_speed <= 0) {
        cout<< "ERROR: Speed can't be negative or 0. Remain the current setting: "<< setprecision(4)<< speed << endl;
        return SIMULATION_ERROR::INVALID_PARAMETER;
    }else if (running.load(memory_order_acquire)) {
        cout<< "ERROR: Speed can't be changed after elevator starts. Remain the current setting: "<< setprecision(4)<< speed << endl;
        return SIMULATION_ERROR::INVALID_STATE;
    }else {
        speed = _speed;
        cout<<"SPEED: "<< setprecision(4)<< speed<<endl;
        return SIMULATION_ERROR::SUCCESS;
    }
}

SIMULATION_ERROR Elevator::set_floors(float  _floors) {
    if (_floors <= 0) {
        cout<< "ERROR: Total floor number " << _floors << "  can't be negative or 0 . Remain the previous setting: "
        << setprecision(4)<< number_floors << endl;
        return SIMULATION_ERROR::INVALID_PARAMETER;
    }else if (running.load(memory_order_acquire)) {
        cout<< "ERROR: Floor can't be changed after elevator starts. Remain the current setting: "
        << setprecision(4) << number_floors << endl;
        return SIMULATION_ERROR::INVALID_STATE;
    }else {
        number_floors = _floors;
        cout<<"FLOORS: "<< (int)number_floors<<endl;
        return SIMULATION_ERROR::SUCCESS;
    }
}


SIMULATION_ERROR Elevator:: set_start_floor (float _start_floor ) {
    if (_start_floor < 0 || _start_floor >= number_floors) {
        cout<< "ERROR: Starting floor number "<< _start_floor <<
        " can't be negative or equal/bigger than total floor . Remain previous setting: "<< setprecision(4)<< start_floor << endl;
        return SIMULATION_ERROR::INVALID_PARAMETER;
    }else if (running.load(memory_order_acquire)) {
        cout<< "ERROR: Starting floor can't be changed after elevator starts. Remain the current setting: "
        << setprecision(4) << start_floor << endl;
        return SIMULATION_ERROR::INVALID_STATE;
    }else {
        start_floor = location = _start_floor;
        cout<<"START FLOORS: "<< (int)start_floor<<endl;
        return SIMULATION_ERROR::SUCCESS;
    }
}

SIMULATION_ERROR Elevator:: start() {
    if (running.load(memory_order_acquire)) {
        cout<<"Elevator already started." << endl;
        return SIMULATION_ERROR:: INVALID_PARAMETER;
    }
    // let the elevator run
    running.store(true,memory_order_release);
    run_thread = thread([&]{
        cout << "STARTED "<<" @ time "<< ticker->get_tick()<<endl;
        while (running.load(memory_order_acquire)) {
            if (!running.load(memory_order_acquire)) {
                cout<<"Stopped from simulator" << endl;
                break;
            }
#if DEBUG_ELEVATOR
            cout<< "current floor "<< setprecision(4) << location<< "@ time " << ticker->get_tick()<< endl;
#endif
            // check if there is pending task otherwise do nothing
            if ( pending_loc >= 0 ) {
                if (location == pending_loc) {
                    cout<< "AT floor "<<setprecision(4)<< pending_loc<< "@ time " << ticker->get_tick()<< endl;
                    pending_loc = -1;
                }
                else {
                    // caculate where to move the elevator
                    if (location > pending_loc) {
                        //move down in next tick
                        next_loc = location - speed;
                        if (next_loc <= pending_loc)
                            next_loc = pending_loc; // elevator moves faster than tikcer
                    }else {
                        next_loc = location + speed;
                        if (next_loc >= pending_loc)
                            next_loc = pending_loc;
                    }
                    location = next_loc;
                }
            }
            // sync to the next clock
            unique_lock<mutex> lk(ticker_m);
            ticker_cv.wait(lk);
            lk.unlock();
        }
    });
    return SIMULATION_ERROR::SUCCESS;
}

SIMULATION_ERROR Elevator:: stop() {
    if (!running.load(memory_order_acquire)) {
        cout <<"Elevator already stopped" <<endl;
        return SIMULATION_ERROR::INVALID_PARAMETER;
    }
    
    // When stop command is signalled it performs an immediate stop regardless of pending task
    running.store(false,memory_order_release);
    if (run_thread.joinable())
        run_thread.join();
    cout<<"STOPPED "<<"@ time  " << ticker->get_tick()<< endl;
    return SIMULATION_ERROR::SUCCESS;
}

SIMULATION_ERROR Elevator:: goto_floor (float _floor_num) {
    if (_floor_num < 0 ||_floor_num >= number_floors ) {
        cout<< "ERROR: Goto floor number " <<_floor_num<<
        " can't be negative or bigger than total floors . Remain the previous pending floor: "
        << pending_loc<<endl;
        return SIMULATION_ERROR::INVALID_PARAMETER;
    }else if (location == _floor_num){
        cout <<"GOTO floor: "<< _floor_num << "@ time " << ticker->get_tick()<< endl;
        cout <<"AT floor: "<< _floor_num << "@ time " << ticker->get_tick()<< endl;
        pending_loc = -1;
        return SIMULATION_ERROR::SUCCESS;
    }else if (!running.load(memory_order_acquire)) {
        cout <<"ERROR: Elevator is not started yet"<<endl;
        return SIMULATION_ERROR::INVALID_STATE;
    }else {
        pending_loc = _floor_num;
        cout <<"GOTO floor: "<< pending_loc << "@ time " << ticker->get_tick()<< endl;
        return SIMULATION_ERROR::SUCCESS;
    }
}


