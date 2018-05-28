//
//  ticker.cpp
//  simulator
//
//  Created by Vesper on 5/27/18.
//  Copyright © 2018 home. All rights reserved.
//

#include  "ticker.hpp"

mutex ticker_m;
condition_variable ticker_cv;

shared_ptr<Ticker>  Ticker::instance = nullptr;

Ticker::Ticker(float _rate) {
    set_rate(_rate);
    done = stop_tick.get_future();
}
Ticker::Ticker() {
    done = stop_tick.get_future();
}

shared_ptr<Ticker> Ticker:: make_ticker()
{
    if (!instance)
        instance = shared_ptr<Ticker>(new Ticker());
    return instance;
}

shared_ptr<Ticker> Ticker::make_ticker(float _rate)
{
    if (!instance)
        instance = shared_ptr<Ticker>(new Ticker(_rate));
    return instance;
}

bool Ticker::start() {
    ticker_thread = thread([&] {
        run();
    });
    return true;
}
void Ticker::stop() {
    cout<<"Ticker stopped"<<endl;
    try {
        stop_tick.set_value();
    }catch (const std::future_error & e) {
        cout<<e.what()<<endl;
    }
    if (ticker_thread.joinable())
        ticker_thread.join();
}

SIMULATION_ERROR Ticker::set_rate (float _rate) {
    if (_rate > 0) {
        rate = _rate;
        interval = chrono::duration<long, ratio<1,1000> >(static_cast<int>(1000 / rate));
        cout<<"Tick rate: "<< rate <<"  Tick interval set up to "<< interval.count()<<" milliseconds"<<endl;
        return SIMULATION_ERROR::SUCCESS;
    }else{
        cout<<"Rate can't be negative or 0" << endl;
        cout<<"Tick rate remains "<< rate<<endl;
        return SIMULATION_ERROR::INVALID_PARAMETER;
    }
}

future<void> Ticker::run() {
    return async(launch::async, &Ticker::tickWrapper, this);
}

void Ticker::tickWrapper() {
    future_status status;
    do {
        status = done.wait_for(interval); // waits for interval
        if (status == future_status::timeout) {
            // notify all listeners
            lock_guard<mutex> lock(ticker_m);
            ticks++;
#if DEBUG_TICKER
            cout<<"ticks: " << ticks<< endl;
#endif
            ticker_cv.notify_all();
        }
    } while (status != future_status::ready);
}


