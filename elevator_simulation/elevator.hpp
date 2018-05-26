//
//  elevator.hpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/25/18.
//  Copyright © 2018 home. All rights reserved.
//

#ifndef ELEVATOR_H
#define ELEVATOR_H

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
class Elevator {
public:
    Elevator( shared_ptr<Ticker> _ticker): ticker(_ticker) {}
    virtual ~Elevator(){}
    
    bool set_speed (float _speed ) {
        // <= 0 test only for unit test of elevator
        // in real simulation invalid command will never get here
        if (_speed <= 0) {
            cout<< "ERROR: Speed can't be negative or 0. Remain the current setting: "<< setprecision(2)<< speed << endl;
            return false;
        }else if (running) {
            cout<< "ERROR: Speed can't be changed after elevator starts. Remain the current setting: "<< setprecision(2)<< speed << endl;
            return false;
        }else {
            speed = _speed;
            return true;
        }
    }
    
    float get_speed () const { return speed; }
    
    bool set_floors(float  _floors) {
        if (_floors <= 0) {
            cout<< "ERROR: Floor number can't be negative or 0 . Remain the current setting: "
                << setprecision(2)<< number_floors << endl;
            return false;
        }else if (running) {
            cout<< "ERROR: Floor can't be changed after elevator starts. Remain the current setting: "
                << setprecision(2) << number_floors << endl;
            return false;
        }else {
            number_floors = _floors;
            cout<<" FLOORS: "<< (int)number_floors<<endl;
            return true;
        }
    }
    
    float get_floors() const {return number_floors ;}
    
    bool set_start_floor (float _start_floor ) {
        if (_start_floor < 0) {
            cout<< "ERROR: Starting floor number can't be negative . Remain the current setting: "
                << setprecision(2)<< start_floor << endl;
            return false;
        }else if (running) {
            cout<< "ERROR: Starting floor can't be changed after elevator starts. Remain the current setting: "
                << setprecision(2) << start_floor << endl;
            return false;
        }else {
            start_floor = location = _start_floor;
            return true;
        }
    }
    
    bool goto_floor (float _floor_num) {
        if (_floor_num < 0 ||_floor_num >= number_floors ) {
            cout<< "ERROR: Floor number can't be negative or bigger than total floors . Remain the current setting: "<< endl;
            return false;
        }else if (location == _floor_num){
            cout <<"AT floor: "<< _floor_num << endl;
            return true;
        }else {
            pending_loc = _floor_num;
            if (!running) {
                // let the elevator run
                running = true;
                run_thread = thread([&]{
                    while (true) {
                        unique_lock <mutex>lock(ticker_m);
                        while (running)
                            ticker_cv.wait(lock);
                        if (!running)
                            break;
                        // check if there is pending task otherwise do nothing
                        if ( pending_loc >= 0 ) {
                            if (location == pending_loc) {
                                cout<< "AT floor "<< pending_loc<<endl;
                                pending_loc = -1;
                            }
                            else {
                                // caculate where to move the elevator
                                location = next_loc;
                                if (location > pending_loc) {
                                    //move down in next tick
                                    next_loc = location - speed;
                                }else
                                    next_loc = location + speed;
                            }
                        }
                    }
                });
            }
            return true;
        }
    }
    
    float get_current_loc() const { return location; }
    
    bool stop() {
        if (!running) {
            cout<<"ERROR: Already stopped! AT " << setprecision(2)<< location <<endl;
            return false;
        }
        // When stop command is signalled it performs an immediate stop regardless of pending task
        // as specified in the problem statement
        lock_guard <mutex>lock(ticker_m);
        running = false;
        ticker_cv.notify_all();
        run_thread.join();
        return true;
    }
    
protected:
    mutable float speed = 0.5 ; // 0.5 floor per second
    mutable float number_floors = 4;
    mutable float start_floor = 1;
    
    //a simplied state machine of elevator
    bool  running = false;
    float location = 1;
    float pending_loc = -1;
    float next_loc = 1;
    float speed_per_tick = 1;

    thread run_thread;
    shared_ptr<Ticker> ticker;
    
};



#endif /* ELEVATOR_H */
