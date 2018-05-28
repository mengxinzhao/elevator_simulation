//
//  main.cpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/24/18.
//  Copyright © 2018 home. All rights reserved.
//

#include <memory>
#include "simulator.hpp"

int main(){
    using namespace std;
    unique_ptr<Simulator>  simulator = make_unique<Simulator>();
    cout<<"Waiting for commands...Enter \"Quit\" to exit the simulation"<<endl;
    simulator->run();
    return 0;
    
}
