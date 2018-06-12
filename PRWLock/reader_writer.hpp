//
//  reader_writer.hpp
//  PRWLock
//
//  Created by Shi Yu on 2018/6/9.
//  Copyright © 2018年 Shi Yu. All rights reserved.
//

#ifndef reader_writer_hpp
#define reader_writer_hpp

#include <stdio.h>
#include <vector>
#include "prwlock.h"
#include <signal.h>

#define MAX_NUM_READERS 1000

#define PASSIVE 0
#define FREE 1
#define LOCK 2
#define PASS 3

using std::vector;
using std::atomic_int;

class Reader {
protected:
    const int* share_var;
public:
    Reader(const int* share_var);
};

class PRWLockReader : public Reader {
private:
    PRWLock& prwlock;
    int reader_id;
    
    void read_lock();
    void read_unlock();
    void schedule_out();
    void read_share_var(); 
    
    static void report(int signo, siginfo_t *info, void *rstat);
    
    pthread_t thread_id;
    
    struct thread_info {
        thread_info(int rid, PRWLock *pl) { reader_id = rid; prwlock = pl; }
        int reader_id;
        PRWLock *prwlock;
    };
public:
    PRWLockReader(const int* share_var, int reader_id, PRWLock &prwlock);
    
    static void* read(void *reader);
};

class Writer {
protected:
    int* share_var;
    
public:
    Writer(int* share_var);
};

class PRWLockWriter : public Writer {
private:
    PRWLock &prwlock;
    void write_lock();
    void write_unlock();
    int writer_id;
    pthread_t thread_id;
public:
    
    struct report_info {
        const atomic_int *version;
        PRWLock::RStatus *rstat;
        report_info(const atomic_int *v, PRWLock::RStatus *rs) { version = v; rstat = rs; }
    };
    
    vector<report_info*> report_infos;
    
    PRWLockWriter(int* share_var, int writer_id, PRWLock &prwlock);
    
    static void* write(void *writer);
};

#endif /* reader_writer_hpp */
