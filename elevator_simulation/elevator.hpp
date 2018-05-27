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
class Elevator {
public:
    Elevator( shared_ptr<Ticker> _ticker): ticker(_ticker) {}
    virtual ~Elevator(){
        if ( running.load(memory_order_acquire))
            stop();
    }
    
    bool set_speed (float _speed ) {
        // <= 0 test only for unit test of elevator
        // in real simulation invalid command will never get here
        if (_speed <= 0) {
            cout<< "ERROR: Speed can't be negative or 0. Remain the current setting: "<< setprecision(4)<< speed << endl;
            return false;
        }else if (running.load(memory_order_acquire)) {
            cout<< "ERROR: Speed can't be changed after elevator starts. Remain the current setting: "<< setprecision(4)<< speed << endl;
            return false;
        }else {
            speed = _speed;
            cout<<"SPEED: "<< setprecision(4)<< speed<<endl;
            return true;
        }
    }
    
    float get_speed () const { return speed; }
    
    bool set_floors(float  _floors) {
        if (_floors <= 0) {
            cout<< "ERROR: Total floor number " << _floors << "  can't be negative or 0 . Remain the previous setting: "
                << setprecision(4)<< number_floors << endl;
            return false;
        }else if (running.load(memory_order_acquire)) {
            cout<< "ERROR: Floor can't be changed after elevator starts. Remain the current setting: "
                << setprecision(4) << number_floors << endl;
            return false;
        }else {
            number_floors = _floors;
            cout<<"FLOORS: "<< (int)number_floors<<endl;
            return true;
        }
    }
    
    float get_floors() const {return number_floors ;}
    
    bool set_start_floor (float _start_floor ) {
        if (_start_floor < 0 || _start_floor >= number_floors) {
            cout<< "ERROR: Starting floor number "<< _start_floor <<
            " can't be negative or equal/bigger than total floor . Remain previous setting: "<< setprecision(4)<< start_floor << endl;
            return false;
        }else if (running.load(memory_order_acquire)) {
            cout<< "ERROR: Starting floor can't be changed after elevator starts. Remain the current setting: "
                << setprecision(4) << start_floor << endl;
            return false;
        }else {
            start_floor = location = _start_floor;
            cout<<"START FLOORS: "<< (int)start_floor<<endl;
            return true;
        }
    }
    
    bool goto_floor (float _floor_num) {
        if (_floor_num < 0 ||_floor_num >= number_floors ) {
            cout<< "ERROR: Goto floor number " <<_floor_num<<
            " can't be negative or bigger than total floors . Remain the previous setting pending floor: "
            << pending_loc<<endl;
            return false;
        }else if (location == _floor_num){
            cout <<"GOTO floor: "<< _floor_num << "@ time " << ticker->get_tick()<< endl;
            cout <<"AT floor: "<< _floor_num << "@ time " << ticker->get_tick()<< endl;
            pending_loc = -1;
            return true;
        }else {
            pending_loc = _floor_num;
            cout <<"GOTO floor: "<< pending_loc << "@ time " << ticker->get_tick()<< endl;
            if (!running.load(memory_order_acquire)) {
                // let the elevator run
                running.store(true,memory_order_release);
                run_thread = thread([&]{
                    while (running.load(memory_order_acquire)) {
                        if (!running.load(memory_order_acquire)) {
                            cout<<"Stopped from simulator" << endl;
                            break;
                        }
#if DEBUG_ELEVATOR
                        cout<< "current floor "<< setprecision(4) << location<< "@ time  " << ticker->get_tick()<< endl;
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
            }
            return true;
        }
    }
    
    float get_current_loc() const { return location; }
    
    bool is_running () const {return running.load();}
    
    void stop() {

        // When stop command is signalled it performs an immediate stop regardless of pending task
        running.store(false,memory_order_release);
        if (run_thread.joinable())
            run_thread.join();
        cout<<"STOPPED "<<"@ time  " << ticker->get_tick()<< endl;
        return ;
    }
    
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
