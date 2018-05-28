//
//  events.cpp
//  events
//
//  Created by Vesper on 5/24/18.
//  Copyright © 2018 home. All rights reserved.
//


#include "events.hpp"

mutex event_m;
condition_variable event_cv;

// start getting user input from the stdin
void CommandGenerator:: start() {
    io_thread =  thread([&] {
        // for testing only. direct string to cin
        std::stringstream test_cin;
        streambuf *cinbuf = nullptr;
        if (seq.length()>0) {
            test_cin << seq;
            cinbuf = cin.rdbuf();
            cin.rdbuf(test_cin.rdbuf());
        }
        string token;
        string line;
        while(getline(cin, line)) {
            stringstream iss;
            iss << line;
            while(getline(iss, token, ',')) {
                transform(token.begin(), token.end(), token.begin(), ::tolower);
#if DEBUG_COMMAND
                cout<<"receiving: "<<token<<endl;
#endif
                auto cmd = parse_input(token);
                if (cmd.type!= COMMAND::INVALID_COMMAND && cmd.type != COMMAND::QUIT) {
                    lock_guard<mutex> lock(event_m);
                    // get the command and its arrival timestamp
                    command_streamer->emplace(cmd, ticker->get_tick());
                    //cout<<"command streamer size: "<<command_streamer->size()<<endl;
                    event_cv.notify_all();
                } else if (cmd.type == COMMAND::QUIT) {
                    lock_guard<mutex> lock(event_m);
                    exited.store(true,memory_order_release);
                    event_cv.notify_all();
                    break;
                } else
                    cout<<"ERROR: "<< token <<" invalid command" <<endl;
            }
            if (exited.load(memory_order_acquire))
                break;
        }

        lock_guard<mutex> lock(event_m);
        if (!exited.load())
            exited.store(true,memory_order_release);
        event_cv.notify_all();
        
        //restore cin
        if (seq.length()>0) {
            cin.rdbuf(cinbuf);
        }
    });
}

void CommandGenerator:: stop() {
    lock_guard<mutex> lock(event_m);
    if (!exited.load())
        exited.store(true,memory_order_release);
    event_cv.notify_all();
    
    if (io_thread.joinable())
        io_thread.join();
}

// parse a line of string and extract meanful commands
Command CommandGenerator:: parse_input(string &s) {
    // a legal  command is defined as command header  + a parameter
    // such as Set rate 0.5/Set floor 10
    // once command header is parsed, no letter should be inteleaved with
    // actual parameter
    // for example Set Rate s-0.3 is a invalid command while Set Rate -0.3 is
    
    if (s.find(set_rate_token) != string::npos )
        return parse_value(s.substr(s.find(set_rate_token) + strlen(set_rate_token)), COMMAND::SET_RATE);
    else if (s.find(set_speed_token)!= string::npos)
        return parse_value(s.substr(s.find(set_speed_token) + strlen(set_speed_token)), COMMAND::SET_SPEED);
    else if (s.find(set_floors_token)!= string::npos)
        return parse_value(s.substr(s.find(set_floors_token) + strlen(set_floors_token)), COMMAND::SET_FLOORS);
    else if (s.find(set_start_floor_token)!= string::npos)
        return parse_value(s.substr(s.find(set_start_floor_token) + strlen(set_start_floor_token)), COMMAND::SET_START_FLOOR);
    else if (s.find(goto_token)!= string::npos)
        return parse_value(s.substr(s.find(goto_token) + strlen(goto_token)), COMMAND::GOTO);
    else if (s.find(start_token)!= string::npos)
        return Command(COMMAND::START);
    else if (s.find(stop_token)!= string::npos)
        return Command(COMMAND::STOP);
    else if (s.find(quit_token)!= string::npos)
        return Command(COMMAND::QUIT);
    else
        return Command(COMMAND::INVALID_COMMAND,-1.0);
}

// parse a string and extract number from it and form a command
Command CommandGenerator:: parse_value(string s, COMMAND type) {
    
    regex re("\\s+[+-]?([0-9]*[.])?[0-9]+");
    smatch match;
    
    if (s.length() >0 ) {
        sregex_iterator next(s.begin(), s.end(), re);
        sregex_iterator end= sregex_iterator();;
        if (next != end) {
            smatch match = *next;
#if 0
            cout << match.str()<<endl;
#endif
            return Command(type, stof (match.str()));
        }else {
            return Command(COMMAND::INVALID_COMMAND,-1.0);
        }
    }else {
        return Command(type);
    }
}
