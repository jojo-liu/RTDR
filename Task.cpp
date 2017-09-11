//
// Created by qiangqiang liu on 02/06/2017.
//

//
//  Task.cpp
//  Schedule
//
//  Created by qiangqiang liu on 30/04/2017.
//  Copyright © 2017 qiangqiang liu. All rights reserved.
//

#include "Task.h"
#include <iostream>
#include <vector>

using namespace std;
Task::Task(){}
Task::Task(vector<DataItem>& data_item, int release_time, int deadline, int sed) {
    this->data_item = data_item;
    this->release_time = release_time;
    this->deadline = deadline;
    this->sed = sed;
}
Task::~Task(){}
void Task::setReleaseTime(int release_time) {
    this->release_time = release_time;
}

int Task::getReleaseTime() {
    return release_time;
}

void Task::setDeadline(int deadline) {
    this->deadline = deadline;
}

int Task::getDeadline() {
    return deadline;
}

void Task::setFinishTime(int finish_time) {
    this->finish_time = finish_time;
}

int Task::getFinishTime() {
    return finish_time;
}

void Task::setDataItem(vector<DataItem>& data_item) {
    this->data_item = data_item;
}
vector<DataItem>& Task::getDataItem() {
    return data_item;
}

void Task::setIsFinished(bool is_finished) {
    this->is_finished = is_finished;
}
bool Task::getIsFinished() {
    return is_finished;
}

void Task::setSED(int sed) {
    this->sed = sed;
}

int Task::getSED() {
    return sed;
}

// //confirm if current task is finished or not
// bool Task::isFinished(vector<DataItem>& data_item) {
// 	return data_item.size() == 0;	//when current task is finished, the size of data_item should be 0
// }

// //resest the deadline of talk according to all data items
// void Task::updateDeadline() {
// 	//we sort the data items in task ascendingly according to the deadline of each data item
// 	//we can delete data item conveniently after we finish certain data item
// 	sort(data_item.begin(), data_item.end() [](DataItem& a, DataItem& b) {
// 		return a.getAvailableTime() + a.getValidInterval() < b.getAvailableTime() + b.getValidInterval();});

// 	//delete data items which is finished in current task
// 	while(data_item.back().getIsFinished() ==  true) {
// 		data_item.pop_back();
// 	}

// 	//update the deadline of task, the smaller one between deadline and dmax(the max deadline of all dataitems)
// 	this->deadline = min(deadline, data_item.back().getAvailableTime() + data_item.back().getValidInterval());
// }
