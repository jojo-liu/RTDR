//
// Created by qiangqiang liu on 02/06/2017.
//

#ifndef SCHEDULE_DATAITEM_H
#define SCHEDULE_DATAITEM_H
//
//  DataItem.hpp
//  Schedule
//
//  Created by qiangqiang liu on 30/04/2017.
//  Copyright © 2017 qiangqiang liu. All rights reserved.
//
#include <stdio.h>
//definition of DataItem
class DataItem {
private:
    /*
     for each data item, assume most parameters except available_factor are integer
     */
    // int available_time;		//the earlist time of data item is available
    // int available_period;	//available period of data item
    // double available_factor;	//set available factor where 0 < available_factor < 1

    int retrieval_time;		//the time when data item starts being retrieved
    int validity_interval;	//validity interval of data item
    int release_time; 		//the time when data item releases
    bool is_finished;		//set isFinished to confirm if current data item is finished or not
    int deadline;			//the deadline of data item

public:
    DataItem();
    DataItem(int release_time, int deadline, int validity_interval, bool is_finished);
    ~DataItem();

    // void setAvailableTime(int available_time);
    // int getAvailableTime();
    // void setAvailablePeriod(int available_time);
    // int getAvailablePeriod();
    void setReleaseTime(int release_time);
    int getReleaseTime();
    void setRetrievalTime(int retrieval_time);
    int getRetrievalTime();
    void setValidInterval(int validity_interval);
    int getValidInterval();
    // void setAvailableFactor(double available_factor);
    // double getAvailableFactor();
    void setIsFinished(bool is_finished);
    bool getIsFinished();

    // bool isValid();
    // bool isAvailable();
    // void isFinished();

    void setDeadline(int deadline);
    int getDeadline();

    bool isEqual(DataItem& left, DataItem& b);
};

#endif //SCHEDULE_DATAITEM_H
