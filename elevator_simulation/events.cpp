//
//  events.cpp
//  events
//
//  Created by Vesper on 5/24/18.
//  Copyright © 2018 home. All rights reserved.
//


#include "events.hpp"

// start getting user input from the stdin
void CommandGenerator::start() {
    io_thread =  std::thread([&]{
        // for test only. direct ifstream to cin
        std::streambuf *cinbuf = nullptr;
        std::ifstream *ifs = nullptr;
        if (file_name.length()>0) {
            cinbuf = std::cin.rdbuf();
            ifs = new std::ifstream(file_name,std::ifstream::in);
            std::cin.rdbuf(ifs->rdbuf());
        }
        std::string token;
        std::string line;
        std::stringstream iss;
        while(!exited && std::getline(std::cin, line)) {
            iss << line;
            while(std::getline(iss, token, ',')) {
                std::transform(token.begin(), token.end(), token.begin(), ::tolower);
#if DEBUG
                std::cout<<"receiving: "<<token<<std::endl;
#endif
                if (token.find("quit")!= std::string::npos){
                    auto lock = std::unique_lock<std::mutex>(m);
                    exited = true;
                    lock.unlock();
                    cv.notify_all();
                    break;
                }
                else {
                    auto cmd = parse_input(token);
                    if (cmd.type!= COMMAND::INVALID_COMMAND){
                        auto lock = std::unique_lock<std::mutex>(m);
                        command_streamer->push(cmd);
                        lock.unlock();
                        cv.notify_all();
                    }
                    else {
                        std::cout<<token <<" invalid command" <<std::endl;
                    }
                }


            }
        }
        auto lock = std::unique_lock<std::mutex>(m);
        exited = true;
        lock.unlock();
        cv.notify_all();
        //restore cin
        if (file_name.length()>0) {
            std::cin.rdbuf(cinbuf);
            delete ifs;
        }
    });
}
void CommandGenerator::stop() {
        io_thread.join();
    };

// parse a line of string and extract meanful commands
Command CommandGenerator:: parse_input(std::string &s) {
    // a legal  command is defined as command header  + a parameter
    // such as Set rate 0.5/Set floor 10
    // once command header is parsed, no letter should be inteleaved with
    // actual parameter
    // for example Set Rate s-0.3 is a invalid command while Set Rate -0.3 is
    
    if (s.find(set_rate_token) != std::string::npos )
        return parse_value(s.substr(s.find(set_rate_token) + strlen(set_rate_token)), COMMAND::SET_RATE);
    else if (s.find(set_speed_token)!= std::string::npos)
        return parse_value(s.substr(s.find(set_speed_token) + strlen(set_speed_token)), COMMAND::SET_SPEED);
    else if (s.find(set_floors_token)!= std::string::npos)
        return parse_value(s.substr(s.find(set_floors_token) + strlen(set_floors_token)), COMMAND::SET_FLOORS);
    else if (s.find(set_start_floor_token)!= std::string::npos)
        return parse_value(s.substr(s.find(set_start_floor_token) + strlen(set_start_floor_token)), COMMAND::SET_START_FLOOR);
    else if (s.find(goto_token)!= std::string::npos)
        return parse_value(s.substr(s.find(goto_token) + strlen(goto_token)), COMMAND::GOTO);
    else if (s.find(start_token)!= std::string::npos)
        return Command(COMMAND::START);
    else if (s.find(stop_token)!= std::string::npos)
        return Command(COMMAND::STOP);
    else
        return Command(COMMAND::INVALID_COMMAND,-1.0);
}
    
Command CommandGenerator::parse_value(std::string s, COMMAND type) {
    
    std::regex re("\\s+[+-]?([0-9]*[.])?[0-9]+");
    std::smatch match;
    
    if (s.length() >0 ) {
        std::sregex_iterator next(s.begin(), s.end(), re);
        std::sregex_iterator end= std::sregex_iterator();;
        if (next != end) {
            std::smatch match = *next;
#if 0
            std::cout << match.str()<< "\n";
#endif
            return Command(type, std::stof (match.str()));
        }else {
            std::cout<<"Invalid Command. No valid parameter found" << std::endl;
            return Command(COMMAND::INVALID_COMMAND,-1.0);
        }
    }else {
        std::cout<<"No parameter given. Use defaut value"<<std::endl;
        return Command(type);
    }
}

