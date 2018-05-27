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


#endif /* TEST_COMMON_HPP */
