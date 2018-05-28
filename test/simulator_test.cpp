//
//  simulatior_test.cpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/27/18.
//  Copyright © 2018 home. All rights reserved.
//

#include <random>
#include <cassert>

#include "../elevator_simulation/simulator.hpp"
#include "./test_common.hpp"

// test simulator properly handling all events and dispatches and status update
int main() {
    using namespace std;
    
    auto test_data = read_all_lines("./elevator_test.txt");
    default_random_engine rand_gen((random_device())());
    uniform_int_distribution<uint64_t>distribution(0,20);
    
    auto *simulator = new Simulator();
    assert(simulator->get_state()== Simulator::SIMULATION_STATE::UNINITIALIZED);
    delete simulator;
    cout<<"========================================"<<endl;
    auto *sim = new Simulator();
    std::stringstream test_cin;
    streambuf *cinbuf = nullptr;
    if (test_data.back().length()>0) {
        test_cin << test_data.back();
        cinbuf = cin.rdbuf();
        cin.rdbuf(test_cin.rdbuf());
    }
    thread run = thread([&] {sim->run();} );
    run.join();
    if (test_data.back().length()>0) {
        cin.rdbuf(cinbuf);
    }
    delete sim;
    
//    while (!test_data.empty()) {
//        cout<<"========================================"<<endl;
//        auto *sim = new Simulator();
//        std::stringstream test_cin;
//        streambuf *cinbuf = nullptr;
//        if (test_data.back().length()>0) {
//            test_cin << test_data.back();
//            cinbuf = cin.rdbuf();
//            cin.rdbuf(test_cin.rdbuf());
//        }
//        thread run = thread([&] {sim->run();} );
//        run.join();
//
//        //restore cin
//        if (test_data.back().length()>0) {
//            cin.rdbuf(cinbuf);
//        }
//        delete sim;
//        test_data.pop_back();
//    }

    return 0;
}
