//
//  main.cpp
//  PRWLock
//
//  Created by Shi Yu on 2018/6/9.
//  Copyright © 2018年 Shi Yu. All rights reserved.
//

#include <iostream>
#include <omp.h>
#include <pthread.h>
#include "reader_writer.hpp"
#include <vector>

using std::vector;
using std::cout;
using std::endl;

struct brlock_read_arg {
    int reader_id;
    pthread_mutex_t *r_mutex;
    brlock_read_arg(int rid, pthread_mutex_t* rmtx) {
        reader_id = rid;
        r_mutex = rmtx;
    }
};

struct brlock_write_arg {
    int writer_id;
    int num_readers;
    pthread_mutex_t* r_mutexes;
    brlock_write_arg(int wid, int nrds, pthread_mutex_t* rmtxs) {
        writer_id = wid;
        num_readers = nrds;
        r_mutexes = rmtxs;
    }
};

void *brlock_read(void* arg) {
    brlock_read_arg* read_arg = (brlock_read_arg*)arg;
    auto r_mutex = read_arg->r_mutex;
    auto reader_id = read_arg->reader_id;
    while(true) {
        pthread_mutex_lock(r_mutex);
        cout << "reader " << reader_id << " reads" << endl;
        pthread_mutex_unlock(r_mutex);
    }
}

void *brlock_write(void* arg) {
    brlock_write_arg* write_arg = (brlock_write_arg*)arg;
    int num_readers = write_arg->num_readers;
    int writer_id = write_arg->writer_id;
    auto r_mutexes = write_arg->r_mutexes;
    while (true) {
        for(int i = 0; i < num_readers; ++i) {
            pthread_mutex_lock(&r_mutexes[i]);
        }
        cout << "writer " << writer_id << " writes" << endl;
        for(int i = 0; i < num_readers; ++i) {
            pthread_mutex_unlock(&r_mutexes[i]);
        }
    }
}

void brlock() {
    vector<pthread_mutex_t> reader_mutexes;
    int num_readers = 10;
    int num_writers = 2;
    reader_mutexes.resize(num_readers);
    for(int i = 0; i < num_readers; ++i) {
        pthread_mutex_init(&reader_mutexes[i], nullptr);
    }
    
    vector<pthread_t> reader_pids(num_readers);
    vector<pthread_t> writer_pids(num_writers);
    for(int i = 0; i < num_readers; ++i) {
        brlock_read_arg* arg = new brlock_read_arg(i, &reader_mutexes[i]);
        pthread_create(&reader_pids[i], nullptr, &(brlock_read), (void*)(arg)); 
    }
    for(int i = 0; i < num_writers; ++i) {
        brlock_write_arg* arg = new brlock_write_arg(i, num_readers, reader_mutexes.data());
        pthread_create(&writer_pids[i], nullptr, &(brlock_write), (void*)arg);
    }
}

int main(int argc, const char * argv[]) {
    int num_readers = 10;
    int num_writers = 2;
    
    PRWLock lock(num_readers);
    
    vector<PRWLockReader*> readers;
    vector<PRWLockWriter*> writers;
    
    readers.resize(num_readers);
    writers.resize(num_writers);
    
    int share_var;
    for(int i = 0; i < num_readers; ++i) {
        readers[i] = new PRWLockReader(&share_var, i, lock);
    }
    for(int i = 0; i < num_writers; ++i) {
        writers[i] = new PRWLockWriter(&share_var, i, lock);
    }
    return 0;
}
