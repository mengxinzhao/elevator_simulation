//
//  ticker_test.cpp
//  elevator_simulation
//
//  Created by Vesper Zhao on 5/25/18.
//  Copyright © 2018 home. All rights reserved.
//


#include "../elevator_simulation/common.hpp"
#include "../elevator_simulation/ticker.hpp"

// This is to test ticker being able to properly tick according to a set valid interval
// and ticking value is global
int main (){
    using namespace std;

    // create a promise and associated shared_future
//    promise<void> done;
//
//    auto t1 = Ticker:: make_ticker(done.get_future());
//    auto t2 = Ticker:: make_ticker(done.get_future());
//
    // start the threads
    auto t1 = Ticker:: make_ticker();
    
    //auto x = t1->run();
    auto x = t1->start();
    cout<<"rate: " << t1->get_rate()<< ", "<<t1->get_interval() <<" milliseconds"<<endl;
    this_thread::sleep_for(5s);
    
    // test a combination of tick rate changes
    t1->set_rate(0.5);
    cout<<"rate: " << t1->get_rate()<< ", "<<t1->get_interval() <<" milliseconds"<<endl;
    this_thread::sleep_for(10s);
    
    t1->set_rate(-0.5);
    cout<<"rate: " << t1->get_rate()<< ", "<<t1->get_interval() <<" milliseconds"<<endl;
    this_thread::sleep_for(10s);
    
    t1->set_rate(1.2);
    cout<<"rate: " << t1->get_rate()<< ", "<<t1->get_interval() <<" milliseconds"<<endl;
    this_thread::sleep_for(10s);
    
    t1->set_rate(0);
    cout<<"rate: " << t1->get_rate()<< ", "<<t1->get_interval() <<" milliseconds"<<endl;
    this_thread::sleep_for(10s);
    
    t1->set_rate(4.5);
    cout<<"rate: " << t1->get_rate()<< ", "<<t1->get_interval() <<" milliseconds"<<endl;
    this_thread::sleep_for(10s);
    
    t1->set_rate(0.2);
    cout<<"rate: " << t1->get_rate()<< ", "<<t1->get_interval() <<" milliseconds"<<endl;
    this_thread::sleep_for(10s);
    
    t1->stop();

}
