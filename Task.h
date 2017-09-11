//
// Created by qiangqiang liu on 02/06/2017.
//

#ifndef SCHEDULE_TASK_H
#define SCHEDULE_TASK_H
//
//  Task.hpp
//  Schedule
//
//  Created by qiangqiang liu on 30/04/2017.
//  Copyright © 2017 qiangqiang liu. All rights reserved.
//

#include <stdio.h>
#include "DataItem.h"
#include <vector>

//definition of task

/*
 In each task, assume that all data items we provided follow "single channel" rule, which means after we retrieve one data item,
 we can continue to retrieve another, they are not overlapped
 */
class Task {
private:
    int release_time;
    int deadline;
    int finish_time;

    //for compare the paiority of different task in RTSS2016
    int sed;

    bool is_finished;		//set isFinished to confirm if current task is finished or not
public:	//set data_item to be public to be easily operate
    std::vector<DataItem> data_item;

public:
    Task();
    Task(std::vector<DataItem>& data_item, int release_time, int deadline, int sed);
    ~Task();

    void setReleaseTime(int release_time);
    int getReleaseTime();

    void setDeadline(int deadline);
    int getDeadline();

    void setFinishTime(int finish_time);
    int getFinishTime();

    void setDataItem(std::vector<DataItem>& data_item);
    std::vector<DataItem>& getDataItem();

    void setIsFinished(bool is_finished);
    bool getIsFinished();

    void setSED(int sed);
    int getSED();

    // bool isFinished(vector<DataItem>& data_item);
    // void updateDeadline();//resest the deadline of talk according to all data items
};

#endif /* Task_h */
