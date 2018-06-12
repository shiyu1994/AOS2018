//
//  reader_writer.cpp
//  PRWLock
//
//  Created by Shi Yu on 2018/6/9.
//  Copyright © 2018年 Shi Yu. All rights reserved.
//

#include "reader_writer.hpp"
#include <set>
#include <iostream>
#include <pthread.h>

using std::cout;
using std::endl;

Reader::Reader(const int* share_var) {
    this->share_var = share_var;
}

PRWLockReader::PRWLockReader(const int* share_var, int reader_id, PRWLock &prwlock):
Reader(share_var), prwlock(prwlock) {
    this->reader_id = reader_id;
    pthread_create(&thread_id, nullptr, &(PRWLockReader::read), (void*)this);
    prwlock.reader_threads[reader_id] = &thread_id;
}

void PRWLockReader::read_lock() {
    PRWLock::RStatus stat = prwlock.rstatus[reader_id];
    stat.status = PASSIVE;
    while(prwlock.wstatus != FREE) {
        stat.status = FREE;
        stat.version = prwlock.version;
        while(prwlock.wstatus != FREE);
        stat.status = PASSIVE;
    }
}

void PRWLockReader::read_unlock() {
    PRWLock::RStatus stat = prwlock.rstatus[reader_id];
    if(stat.status == PASSIVE) {
        stat.status = FREE;
    }
    else {
        prwlock.active -= 1;
    }
}

void PRWLockReader::schedule_out() {
    PRWLock::RStatus stat = prwlock.rstatus[reader_id];
    if(stat.status == PASSIVE) {
        prwlock.active += 1;
        stat.status = FREE;
    }
    stat.version = prwlock.version; 
}

void PRWLockReader::report(int signo, siginfo_t* info, void *ctx) {
    PRWLockWriter::report_info *rinfo = (PRWLockWriter::report_info*)info->si_value.sival_ptr;
    int version = *(rinfo->version);
    PRWLock::RStatus *rstat = rinfo->rstat;
    if(rstat->status != PASSIVE) {
        rstat->version = version;
    }
}

void PRWLockReader::read_share_var() {
    cout << "read share variable value: " << (*share_var) << endl;
}

void* PRWLockReader::read(void *rd) {
    struct sigaction act;
    act.sa_sigaction = report;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &act, nullptr);
    PRWLockReader *reader = (PRWLockReader*)rd;
    
    while(true) {
        reader->read_lock();
        reader->read_share_var();
        reader->read_unlock();
    }
}

PRWLockWriter::PRWLockWriter(int* share_var, int writer_id, PRWLock &prwlock):
Writer(share_var), prwlock(prwlock) {
    this->writer_id = writer_id;
    report_infos.resize(prwlock.reader_threads.size());
    for(int i = 0; i < report_infos.size(); ++i) {
        report_infos[i] = new report_info(&prwlock.version, &prwlock.rstatus[i]);
    }
    pthread_create(&thread_id, nullptr, &(PRWLockWriter::write), (void*)this);
}

void PRWLockWriter::write_lock() {
    pthread_mutex_lock(&prwlock.write_lock_mutex);
    if(prwlock.wstatus == PASS) {
        return;
    }
    prwlock.wstatus = LOCK;
    ++prwlock.version;
    std::set<int> cores_wait;
    for(int i = 0; i < prwlock.rstatus.size(); ++i) {
        PRWLock::RStatus stat = prwlock.rstatus[i];
        if(stat.version != prwlock.version) {
            union sigval val;
            val.sival_ptr = report_infos[i];
            pthread_sigqueue(*prwlock.reader_threads[i], SIGINT, val);
            cores_wait.insert(i);
        }
    }
    for(int i : cores_wait) {
        while(prwlock.rstatus[i].version != prwlock.version);
    }
    while(prwlock.active != 0);
}

void PRWLockWriter::write_unlock() {
    prwlock.wstatus = FREE;
    pthread_mutex_unlock(&prwlock.write_lock_mutex);
}

void* PRWLockWriter::write(void *wt) {
    PRWLockWriter *writer = (PRWLockWriter*)wt;
    while(true) {
        writer->write_lock();
        cout << "writer " << writer->writer_id << " increasing share var from "
            << (*(writer->share_var)) << " to " << ((*(writer->share_var)) + 1) << endl;
        (*(writer->share_var)) += 1;
        writer->write_unlock();
    }
}

