//
//  common.hpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/24/18.
//  Copyright © 2018 home. All rights reserved.
//

#ifndef COMMON_H
#define COMMON_H

#include <exception>
#include <iostream>
#include <string>
#include <cmath>

// command things

enum SIMULATION_ERROR {
    InvalidParameter,
    InvalidState,
};

struct Simulation_Error: std::runtime_error {
    SIMULATION_ERROR  error;
    explicit Simulation_Error(SIMULATION_ERROR err, const std::string &s): std::runtime_error(s), error(err){}
};

// TODO: template const chars
static const char  *set_rate_token = "set rate";
static const char  *set_floors_token = "set floor";
static const char  *set_speed_token = "set speed";
static const char  *set_start_floor_token = "set start floor";
static const char  *goto_token = "goto";
static const char  *start_token = "start";
static const char  *stop_token = "stop";


enum COMMAND {INVALID_COMMAND = -1, SET_RATE, SET_FLOORS, SET_SPEED, SET_START_FLOOR, START, GOTO, STOP};

struct Command {
    COMMAND type;
    float param;
    Command(COMMAND _type, float _param): type(_type),param(_param) {
        switch (type) {
            case SET_RATE:
            case SET_SPEED:
                if (param <= 0)
                    type = INVALID_COMMAND;
                break;
            case SET_FLOORS:
                // floors need to be both positive and integer
                if (param <= 0)
                    type = INVALID_COMMAND;
                param = std::floor(param);
                break;
            case GOTO:
            case SET_START_FLOOR:
                // don't allow negative numbers otherwise the simulator need to know the lowest/highest
                // floor and totol floors to be able to properly run
                if (param < 0)
                    type = INVALID_COMMAND;
                param = std::floor(param);
                break;
            default:
                // STOP/START we don't care
                break;
        }
    }
    Command(COMMAND _type):type(_type) {
        switch (type) {
            case SET_RATE:
            case SET_START_FLOOR:
                param = 1.0;
                break;
            case SET_SPEED:
                param = 0.5;
                break;
            case SET_FLOORS:
                param = 4;
                break;
            case GOTO:
                // goto command with explicit parameter is an invalid one
                type = INVALID_COMMAND;
                break;
            // start/stop don't really need a parameter
            case START:
            case STOP:
            default:
                param = -1;
                break;
        }
    }
};

#endif /* COMMON_H */
