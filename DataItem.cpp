//
// Created by qiangqiang liu on 02/06/2017.
//

//
//  DataItem.cpp
//  Schedule
//
//  Created by qiangqiang liu on 30/04/2017.
//  Copyright © 2017 qiangqiang liu. All rights reserved.
//

#include "DataItem.h"
#include <iostream>

using namespace std;

//initial DataItem object
DataItem::DataItem(){}
DataItem::DataItem(int release_time, int deadline, int validity_interval, bool is_finished):
release_time(release_time), deadline(deadline), validity_interval(validity_interval), is_finished(false) {}
DataItem::~DataItem(){}

// void setAvailableTime(int available_time) {
// 	this->available_time = available_time;
// }

// int getAvailableTime() {
// 	return available_time;
// }

// void setAvailablePeriod(int available_period) {
// 	this->available_period = available_period;
// }

// int getAvailablePeriod() {
// 	return available_period;
// }

void DataItem::setReleaseTime(int release_time) {
    this->release_time = release_time;
}

int DataItem::getReleaseTime() {
    return release_time;
}

void DataItem::setRetrievalTime(int retrieval_time) {
    this->retrieval_time = retrieval_time;
}

int DataItem::getRetrievalTime() {
    return retrieval_time;
}

void DataItem::setValidInterval(int validity_interval) {
    this->validity_interval = validity_interval;
}

int DataItem::getValidInterval() {
    return validity_interval;
}

// void DataItem::setAvailableFactor(double available_factor) {
// 	this->available_factor = available_factor;
// }

// double DataItem::getAvailableFactor() {
// 	return available_factor;
// }

void DataItem::setIsFinished(bool is_finished) {
    this->is_finished = is_finished;
}
bool DataItem::getIsFinished() {
    return is_finished;
}

void DataItem::setDeadline(int deadline) {
    this->deadline = deadline;
}
int DataItem::getDeadline() {
    return deadline;
}

// //confirm if current data item is finished
// void DataItem::isFinished() {
// 	if(isValid() && isAvailable())
// 		is_finished = true;
// 	else
// 		is_finished = false;
// }

// //confirm if current data item subjects to data freshness
// bool DataItem::isValid() {
// 	return available_time + validity_interval >= release_time + retrieval_time;
// }

// //confirm if current data item subject to data availability
// bool DataItem::isAvailable() {
// 	int wait = (release_time - available_time) % available_period;
// 	return (wait >= 0) && (wait <= available_factor * available_period);
// }

bool DataItem::isEqual(DataItem& left, DataItem& right) {
    if(left.getDeadline() == right.getDeadline() && left.getValidInterval() == right.getValidInterval() && left.getReleaseTime() == right.getReleaseTime()) {
        return true;
    }
    return false;
}

