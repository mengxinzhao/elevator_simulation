//
//  test_common.hpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/26/18.
//  Copyright © 2018 home. All rights reserved.
//

#ifndef TEST_COMMON_HPP
#define TEST_COMMON_HPP

#include <vector>
#include <string>
#include <fstream>
#include <getopt.h>
using namespace  std;
static inline vector<string> read_all_lines(string file_name) {
    vector<string> read_out ={};
    string line;
    ifstream file(file_name);
    if (file.is_open()) {
        while (getline(file, line))
            read_out.push_back(line);
    }
    return read_out;
}

static struct option long_options[]= {
    {"file",   required_argument,  0,  'f'},
    {0,         0,                  0,   0}
};

string parse_arguments (int argc, char **argv) {
    
    int c=0,index = 0;
    string file_name;
    if (!argc) {
        cout<<"No test file specified. Use -f file_name" <<endl;
        return {};
    }
    while ((c =  getopt_long_only(argc, argv, "f:", long_options, &index))!= -1){
        switch (c) {
            case 'f':
                file_name.assign(std::string(optarg));
                break;
            default:
                cout<<"Invalid argument"<<endl;
                break;
        }
        
    }
    
    return file_name;
}

#endif /* TEST_COMMON_HPP */
