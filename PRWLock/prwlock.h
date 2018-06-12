//
//  prwlock.h
//  PRWLock
//
//  Created by Shi Yu on 2018/6/10.
//  Copyright © 2018年 Shi Yu. All rights reserved.
//

#ifndef prwlock_h
#define prwlock_h

#include <pthread.h>
#include <vector>

#define FREE 0
#define PASSIVE 1
#define LOCK 2

using std::vector;

class PRWLock {
private:
public:
    
    struct RStatus {
        int version;
        int status;
        RStatus() { version = 0; status = PASSIVE; }
    };
    
    pthread_mutex_t write_lock_mutex;
    vector<pthread_t*> reader_threads;
    vector<RStatus> rstatus;
    int wstatus;
    std::atomic_int active;
    std::atomic_int version;
    
    PRWLock(int num_readers) {
        active = 0;
        wstatus = FREE;
        rstatus.resize(num_readers);
        reader_threads.resize(num_readers);
        pthread_mutex_init(&write_lock_mutex, nullptr);
    }
};

#endif /* prwlock_h */
