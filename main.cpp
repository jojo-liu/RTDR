//
//  main.cpp
//  Schedule
//
//  Created by qiangqiang liu on 30/04/2017.
//  Copyright © 2017 qiangqiang liu. All rights reserved.
//

#include <iostream>
#include <vector>
#include "Task.h"
#include <unordered_map>
#include <fstream>
#include <unistd.h>

using namespace std;

unsigned int microseconds = 3000;

//check if i is the boundary of the data item in all_data_item
bool isMiddle(int m, vector<DataItem>& all_data_item) {
    for(int i = 0; i < all_data_item.size(); i++) {
        if(m == all_data_item[i].getReleaseTime() || m == all_data_item[i].getDeadline())
            return false;
    }
    return true;
}

//check if arbitrary data items share the same deadline, if so, revise the deadline of them according to Deadline Revision Rule D-3
//return value: 2 means current date item cannot set distinct deadline
//				1 means current data item shares the same deadline with other data items and can be set distinct
//				0 means current data item does not share the same deadline with other data items

//check if all data items have been retrieved
bool allRetrieved(vector<Task>& task_set) {
    for(int i = 0; i < task_set.size(); i++) {
        for(int j = 0; j < task_set[i].data_item.size(); j++) {
            if(!task_set[i].data_item[j].getIsFinished()) {
                return false;
            }
        }
    }
    return true;
}

//check if all tasks have been retrieved
bool allRetrievedTasks(vector<Task>& task_set) {
    for(int i = 0; i < task_set.size(); i++) {
        if(!task_set[i].getIsFinished()) {
            return false;
        }
    }
    return true;
}

//check if all data items have been retrieved in one task
bool allRetrievedInTask(Task& task) {
    for(int i = 0; i < task.data_item.size(); i++) {
        if(!task.data_item[i].getIsFinished()) {
            return false;
        }
    }
    return true;
}

//check if all data items in one task meet the data validity
bool allValid(Task& task) {
    for(int i = 0; i < task.data_item.size(); i++) {
        if(task.data_item[i].getRetrievalTime() + task.data_item[i].getValidInterval() < task.getFinishTime()) {
            return false;
        }
    }
    return true;
}

//find the task with the smallest dealdine
int findTaskWithSmallestDeadline(vector<Task>& task_set, int time) {
    int smallest_task_SED = INT_MAX;
    int current_task = -1;
    for(int i = 0; i < task_set.size(); i++) {
        if(task_set[i].getSED() < smallest_task_SED && !task_set[i].getIsFinished() && task_set[i].getReleaseTime() <= time) {
            current_task = i;
            smallest_task_SED = task_set[i].getSED();
        }
    }
    return current_task;
}

//check if all data items have distinct deadlines
bool allDistinct(vector<vector<DataItem>>& data_item_update) {
    vector<DataItem> res;
    for(int i = 0; i < data_item_update.size(); i++) {
        for(int j = 0; j < data_item_update[i].size(); j++) {
            res.push_back(data_item_update[i][j]);
        }
    }
    sort(res.begin(), res.end(), [](DataItem& a, DataItem& b) {
        return a.getDeadline() < b.getDeadline();
    });
    for(int k = 0; k < res.size() - 1; k++) {
        if(res[k].getDeadline() == res[k + 1].getDeadline()) {
            return false;
        }
    }
    return true;
}

int checkDistinct(DataItem& item, long m, int n, vector<vector<DataItem>>& data_item_update, int& smallest_deadline) {
    for(int i = 0; i < data_item_update.size(); i++) {
        for(int j = 0; j < data_item_update[i].size(); j++) {
            if(item.getDeadline() == data_item_update[i][j].getDeadline() && (i != m || j != n) && !data_item_update[i][j].getIsFinished()) {
                if(item.getReleaseTime() <= data_item_update[i][j].getReleaseTime()) {
                    int temp = item.getDeadline();
                    if(temp - 1 <= data_item_update[m][n].getReleaseTime()) {
                        return 2;
                    }
                    else {//sutract the deadline of data_item_update[m][n]
                        if(item.getReleaseTime() == data_item_update[i][j].getReleaseTime()) {
                            if(item.getValidInterval() >= data_item_update[i][j].getReleaseTime()) {
                                data_item_update[m][n].setDeadline(temp - 1);
                                if(data_item_update[m][n].getDeadline() < smallest_deadline) {
                                    smallest_deadline--;
                                }
                            }
                            else {
                                data_item_update[i][j].setDeadline(temp - 1);
                                if(data_item_update[m][n].getDeadline() < smallest_deadline) {
                                    smallest_deadline--;
                                }
                            }
                        }
                        else {
                            data_item_update[m][n].setDeadline(temp - 1);
                            if(data_item_update[m][n].getDeadline() < smallest_deadline) {
                                smallest_deadline--;
                            }
                        }
                        return 1;	//current item shares the same deadline with other data items
                    }
                }
                else {//sutract the deadline of data_item_update[i][j]
                    int temp = data_item_update[i][j].getDeadline();
                    if(temp - 1 <= data_item_update[i][j].getReleaseTime()) {
                        return 2;
                    }
                    else {
                        data_item_update[i][j].setDeadline(temp - 1);
                        if(data_item_update[i][j].getDeadline() < smallest_deadline) {
                            smallest_deadline--;
                        }
                        return 1;	//current item shares the same deadline with other data items
                    }
                }
            }
        }
    }
    return 0;	//current item is distinct to other data items
}


//get the data item with larggest deadline to be used in the MAKE DISTINCT operation
DataItem getDataitemWithLarggestDeadline(vector<vector<DataItem>>& data_item_update) {
    vector<DataItem> res;
    for(int i = 0; i < data_item_update.size(); i++) {
        for(int j = 0; j < data_item_update[i].size(); i++) {
            res.push_back(data_item_update[i][j]);
        }
    }
    sort(res.begin(), res.end(), [](DataItem& a, DataItem& b) {
        return a.getDeadline() < b.getDeadline();
    });
    return res.back();
}


bool schedule(vector<Task>& task_set) {
    //initial data items in each task
    // vector<DataItem> data(5);

    // vector<vector<int>> test = {{0,6,100}, {3,7,100}, {0,5,6}, {3,5,100}, {6,9,100}};
    // for(int i = 0; i < test.size(); i++) {
    //     data[i].setReleaseTime(test[i][0]);
    //     data[i].setDeadline(test[i][1]);
    //     data[i].setValidInterval(test[i][2]);
    // }

    // task_set[0].setDataItem(data);


    //set a single vector to store all of the data items to further usage in idle remove procedure
    vector<DataItem> all_data_item;

    //set data_item_update to judge if the deadlines of all data items which have not been retrieved are different
    vector<vector<DataItem>> data_item_update;

    //set data structure to store the release time and deadline pair, then make the subset tight
    // // unordered_map<DataItem, pair<int, int>> dataitem_to_task;
    // unordered_map<DataItem, int> dataitem_to_task_i;
    // unordered_map<DataItem, int> dataitem_to_task_j;

    //    //set the scope of sub_interval
    //    int min_sub_interval = INT_MAX;
    //    int max_sub_interval = INT_MIN;

    //    //insert data items into corresponding task, and copy them into all_data_item to be used in the next operation: set sub_interval
    //    for(int i = 0; i < task_set.size(); i++) {
    //        for(int j = 0; j < task_set[i].data_item.size(); j++) {
    //            // dataitem_to_task[task_set[i].data_item[j]] = make_pair(i, j);	//record the corresponding task of each data item
    //            // dataitem_to_task_i[task_set[i].data_item[j]] = i;	//record the corresponding task of each data item
    //            // dataitem_to_task_j[task_set[i].data_item[j]] = j;	//record the corresponding task of each data item
    //            all_data_item.push_back(task_set[i].data_item[j]); //push all data items into all_data_item
    //            min_sub_interval = min(min_sub_interval, task_set[i].data_item[j].getReleaseTime());
    //            max_sub_interval = max(max_sub_interval, task_set[i].data_item[j].getDeadline());
    //        }
    //    }
    //
    //    //copy all data items into data_item_update from task_set
    //    for(auto task : task_set) {
    //        data_item_update.push_back(task.data_item);
    //    }
    //
    //
    //    //    sort(all_data_item.begin(), all_data_item.end(), [](DataItem& a, DataItem& b) {
    //    //        return a.getReleaseTime() < b.getReleaseTime();
    //    //    });
    //
    //    //set sub_interval to store each sub_interval which includes the number of slots it occupies and corresponding data items it owns
    //    vector<pair<vector<DataItem>, int>> sub_interval;	//pair.second is the start of the each sub_interval
    //
    //    //generate each sub interval
    //    for(int i = min_sub_interval; i <= max_sub_interval; i++) {
    //        if(isMiddle(i, all_data_item)) {
    //            continue;
    //        }
    //
    //        vector<DataItem> items;
    //        for(int j = 0; j < data_item_update.size(); j++) {
    //            for(int k = 0; k < data_item_update[j].size(); k++) {
    //                if(i >= data_item_update[j][k].getReleaseTime() && i < data_item_update[j][k].getDeadline()) {
    //                    items.push_back(all_data_item[j]);
    //                }
    //            }
    //        }
    //
    //        sub_interval.push_back(make_pair(items, i));
    //    }
    //
    //
    //    //since the last sub_interval may be empty, so we remove the last sub_interval
    //    if(sub_interval.back().first.size() == 0) {
    //        sub_interval.pop_back();
    //    }
    //
    //    //remove the natural idle slots with the methods given in Section VI-A, to make subset tight
    //    for(int i = 0; i < sub_interval.size(); i++) {
    //        int slots = (i + 1 < sub_interval.size() ? sub_interval[i + 1].second - sub_interval[i].second : max_sub_interval - sub_interval[i].second);
    //        int idle_slots = slots - sub_interval[i].first.size();
    //        if(idle_slots <= 0) continue;
    //        else {
    ////            cout<<"idle_slots <= 0"<<endl;
    //            for(int j = 0; j < data_item_update.size(); j++) {
    //                for(int k = 0; k < data_item_update[j].size(); k++) {
    //                    //if current data item's release time == sub_interval[i].second, then we only decrease its deadline
    //                    if(data_item_update[j][k].getReleaseTime() == sub_interval[i].second) {
    //                        int new_deadline = task_set[j].data_item[k].getDeadline() - idle_slots;
    //                        task_set[j].data_item[k].setDeadline(new_deadline);
    //                    }
    //                    //if current data item's release time > sub_interval[i].second, then we decrease its release time and deadline both
    //                    else if(data_item_update[j][k].getReleaseTime() > sub_interval[i].second) {
    //                        int new_deadline = task_set[j].data_item[k].getDeadline() - idle_slots;
    //                        int new_release_time = task_set[j].data_item[k].getReleaseTime() - idle_slots;
    //                        task_set[j].data_item[k].setDeadline(new_deadline);
    //                        task_set[j].data_item[k].setReleaseTime(new_release_time);
    //                    }
    //                    //handle the special case
    //                    else if(data_item_update[j][k].getReleaseTime() < sub_interval[i].second && data_item_update[j][k].getDeadline() >= sub_interval[i].second + 2) {
    //                        int new_deadline = task_set[j].data_item[k].getDeadline() - idle_slots;
    //                        task_set[j].data_item[k].setDeadline(new_deadline);
    //                    }
    //                }
    //            }
    //        }
    //    }


    //        for(int i = 0; i < 6; i++) {
    //            cout<<task_set[0].data_item[i].getReleaseTime()<<"   "<<task_set[0].data_item[i].getDeadline()<<endl;
    //        }


    /*
     after remove natural idle slots, task_set store all most updated data items.
     */

    //reivse the deadline of task and date items
    for(int i = 0; i < task_set.size(); i++) {
        int max_dataitem_deadline = INT_MIN;
        for(int k = 0; k < task_set[i].data_item.size(); k++) {
            max_dataitem_deadline = max(max_dataitem_deadline, task_set[i].data_item[k].getDeadline());
        }
        if(task_set[i].getDeadline() > max_dataitem_deadline) {
            task_set[i].setDeadline(max_dataitem_deadline); //set task deadline as the maximum of all data items in task
        }
        for(int j = 0; j < task_set[i].data_item.size(); j++) {
            if(task_set[i].data_item[j].getDeadline() > task_set[i].getDeadline()) {
                task_set[i].data_item[j].setDeadline(task_set[i].getDeadline());
            }
        }
    }

    //revise the deadline of two arbitraty items which share the same deadline

    //at time t, we need to revise deadline of all data items to be different

    // data_item_update.clear();
    // for(auto task : task_set) {
    // 	data_item_update.push_back(task.getDataItem());
    // }


    //sort each data_item_update[i], and find out the data item with larggest deadline

    // for(int i = 0; i < data_item_update.size(); i++) {
    // 	sort(data_item_update[i].begin(), data_item_update[i].end(), [](DataItem& a, DataItem& b) {
    // 		return a.getDeadline() < b.getDeadline();
    // 	});
    // }

    //copy all data items into data_item_update from task_set
    data_item_update.clear();
    for(auto task : task_set) {
        data_item_update.push_back(task.data_item);
    }

    //get the larggest deadline of all data items
    all_data_item.clear();
    for(int i = 0; i < data_item_update.size(); i++) {
        for(int j = 0; j < data_item_update[i].size(); j++) {
            all_data_item.push_back(data_item_update[i][j]);
        }
    }
    sort(all_data_item.begin(), all_data_item.end(), [](DataItem& a, DataItem& b) {
        return a.getDeadline() < b.getDeadline();
    });

    int larggest_deadline = all_data_item.back().getDeadline();
    int smallest_deadline = all_data_item.front().getDeadline();

    //    cout<<smallest_deadline<<" "<<larggest_deadline<<endl;

    bool distinct = true;
    while(distinct && larggest_deadline >= smallest_deadline) {//this >= judgement is not good, we can send smallest_deadline as a parameter of checkDistinct()
        bool find_larggest_deadline = false;
        for(long i = 0; i < data_item_update.size(); i++) {
            for(int j = 0; j < data_item_update[i].size(); j++) {
                if(data_item_update[i][j].getDeadline() == larggest_deadline) {
                    //                    cout<<larggest_deadline<<endl;
                    //                    cout<<data_item_update[i][j].getReleaseTime()<<endl;
                    //transfer the data item with larggest deadline to the function of checkDistinct()
                    int result = checkDistinct(data_item_update[i][j], i, j, data_item_update, smallest_deadline);
                    //                    if(larggest_deadline == 4) {
                    //                        for(int k = 0; k < data_item_update.size(); k++) {
                    //                            for(int l = 0; l < data_item_update[k].size(); l++) {
                    //                                cout<<data_item_update[k][l].getReleaseTime()<<"-->"<<data_item_update[k][l].getDeadline()<<endl;
                    //                            }
                    //                        }
                    //                    }
                    find_larggest_deadline = true;
                    if(result == 2) {
                        //                        cout<<"result == 2"<<endl;
                        distinct = false;
                        i = data_item_update.size();
                        break;
                    }
                    if(result == 1) {
                        //                        cout<<"result == 1 larggest_deadline ="<<larggest_deadline<<endl;
                        i = data_item_update.size();
                        break;
                    }
                    if(result == 0) {
                        //                        cout<<data_item_update[i][j].getReleaseTime()<<"-->"<<data_item_update[i][j].getDeadline()<<endl;
                        //                        cout<<"result == 0"<<endl;
                        larggest_deadline--;
                        i = data_item_update.size();
                        break;
                    }
                }
            }
        }
        //there is no data item which owns the deadline equals larggest_deadline
        if(!find_larggest_deadline) {
            larggest_deadline--;
        }
    }
    //    cout<<smallest_deadline<<endl;

    //synchronize the task_set and data_item_update
    for(int i = 0; i < data_item_update.size(); i++) {
        task_set[i].setDataItem(data_item_update[i]);
    }

    //    for(int i = 0; i < 6; i++) {
    //        cout<<task_set[0].data_item[i].getReleaseTime()<<"   "<<task_set[0].data_item[i].getDeadline()<<endl;
    ////        cout<<data_item_update[0][i].getReleaseTime()<<"   "<<data_item_update[0][i].getDeadline()<<endl;
    //    }

    //data items cannot make distinct, so they are not schedulable
    if(distinct == false) {
        //        cout<<"this task set is not schedulable"<<endl;
        //        vector<pair<vector<DataItem>, int>>(sub_interval).swap(sub_interval);
        //        vector<DataItem>(all_data_item).swap(all_data_item);
        //        vector<vector<DataItem>>(data_item_update).swap(data_item_update);
        //        vector<Task>(task_set).swap(task_set);
        cout<<"at first, not distinct"<<endl;
        return false;
    }

    //now we retrieve all data items with EDF

    //store all data items into all_data_item, and retrieve them in EDF(earliest deadline first)
    all_data_item.clear();
    // dataitem_to_task.clear();
    for(int i = 0; i < data_item_update.size(); i++) {
        for(int j = 0; j < data_item_update[i].size(); j++) {
            all_data_item.push_back(data_item_update[i][j]);
            // dataitem_to_task[data_item_update[i][j]] = make_pair(i, j);
        }
    }
    sort(all_data_item.begin(), all_data_item.end(), [](DataItem& a, DataItem& b) {
        return a.getDeadline() < b.getDeadline();
    });

    //    //sort data_item_update according to the release time of each data item in each task
    //    for(int i = 0; i < data_item_update.size(); i++) {
    //        sort(data_item_update[i].begin(), data_item_update[i].end(), [](DataItem& a, DataItem& b) {
    //            return a.getReleaseTime() < b.getReleaseTime();
    //        });
    //    }

    /*
     set Q(t) to store the data items which all have been released
     */

    /*
     set start time of algorithm
     */

    int time = all_data_item[0].getReleaseTime();
    //    //get the first upcoming retrieved data item at the beginning of algorithm
    //    int pos_i = -1, pos_j = -1, min_deadline = INT_MAX;
    //    for(int i = 0; i < data_item_update.size(); i++) {
    //        for(int j = 0; j < data_item_update[i].size(); j++) {
    //            if(data_item_update[i][j].getReleaseTime() <= time && !data_item_update[i][j].getIsFinished()) {
    //                if(min_deadline > data_item_update[i][j].getDeadline()) {
    //                    pos_i = i, pos_j = j;
    //                    min_deadline = data_item_update[i][j].getDeadline();
    //                }
    //            }
    //        }
    //    }
    //    cout<<min_deadline<<endl;

    //    //retrieve current data item
    //    time++;
    //    if(pos_i != -1 && pos_j != -1) {
    //        data_item_update[pos_i][pos_j].setIsFinished(true);
    //        data_item_update[pos_i][pos_j].setRetrievalTime(time - 1);
    //    }


    /*
     we can set for loop here as the real beginning of algorithm
     */
    //set the copy of data_item_update to record the original release time of each date item
    vector<vector<DataItem>> data_item_update_original;

    //set the copy of data_item_update to record the original deadline of each date item
    vector<vector<DataItem>> data_item_update_copy;

    //    //set the recover vector to store all data items whose release time set as current time
    //    vector<pair<int, int>> recover;

    //set the Pij vector of data item to confirm CR condition
    vector<DataItem> P_ij;

    //    vector<bool> postpone_task(10, false);
    vector<int> task_deadline_original(10, -1);
    //    //set the recover vector for Pij
    //    vector<vector<DataItem>> data_item_update_copy_d1;

    int pos_i = -1, pos_j = -1;
    int previous_pos_i = -1, previous_pos_j = -1;
    int max_task_deadline = INT_MIN;
    for(int i = 0; i < task_set.size(); i++) {
        max_task_deadline = max(max_task_deadline, task_set[i].getDeadline());
    }

    int count1 = 1, count2 = 1, repeat_data1 = 0, repeat_data2 = 0;
    int pre_pre_pos_i = -1, pre_pre_pos_j = -1, pre_pre_pre_pos_i = -1, pre_pre_pre_pos_j = -1;
    while(!allRetrieved(task_set) && time < max_task_deadline) {
        data_item_update_original.clear();
        data_item_update_original = data_item_update;

        //synchronize the data_item_update to task_set
        data_item_update.clear();
        for(int i = 0; i < task_set.size(); i++) {
            //            data_item_update.push_back(task_set[i].getDataItem());
            data_item_update.push_back(task_set[i].data_item);
        }

        //get the upcoming retrieved data item at current time
        pos_i = -1, pos_j = -1;
        //        recover.clear();
        int min_deadline = INT_MAX;
        for(int i = 0; i < data_item_update.size(); i++) {
            for(int j = 0; j < data_item_update[i].size(); j++) {
                if(data_item_update[i][j].getReleaseTime() <= time && !data_item_update[i][j].getIsFinished()) {
                    // recover.push_back(make_pair(i, j));
                    // //for all data items that have been released in data set q, set their release time as current time
                    // data_item_update[i][j].setReleaseTime(time);

                    //                    cout<<i<<j<<endl;
                    //get the upcoming retrieved data item with earliest deadline at current time
                    if(min_deadline > data_item_update[i][j].getDeadline()) {
                        pos_i = i, pos_j = j;
                        min_deadline = data_item_update[i][j].getDeadline();
                    }
                }
            }
        }
        //                cout<<time<<endl;
        //                cout<<pos_i<<pos_j<<endl;


        //                cout<<previous_pos_i<<"&&"<<previous_pos_j<<endl;
        //                cout<<pos_i<<"&&"<<pos_j<<endl;
        //                cout<<pre_pre_pos_i<<"&&"<<pre_pre_pos_j<<endl;
        if(pos_i != -1 && pos_j != -1 && pos_i == previous_pos_i && pos_j == previous_pos_j) {
            repeat_data1++;
        }
        if((pos_i != -1 && pos_j != -1 && pos_i == pre_pre_pos_i && pos_j == pre_pre_pos_j) || (pos_i != -1 && pos_j != -1 && pos_i == pre_pre_pre_pos_i && pos_j == pre_pre_pre_pos_j)) {
            repeat_data2++;
        }
        if(repeat_data1 > 10 || repeat_data2 > 10) {
            cout<<"this is a dead loop"<<endl;
            return false;
        }
        if(count1 % 2 == 0) {
            pre_pre_pos_i = pos_i, pre_pre_pos_j = pos_j;
        }
        if(count2 % 2 == 1) {
            pre_pre_pre_pos_i = pos_i, pre_pre_pre_pos_j = pos_j;
        }
        count1++;
        count2++;

        //        cout<<"repeat_data1: "<<repeat_data1<<endl;
        //        cout<<"repeat_data2: "<<repeat_data2<<endl;

        //set the position of retrieved data item at time - 1
        previous_pos_i = pos_i, previous_pos_j = pos_j;
        //        cout<<pos_i<<"-----"<<pos_j<<endl;
        //        cout<<time<<endl;
        if(pos_i == -1 && pos_j == -1) {
            time++;
            continue;
        }

//        cout<<time<<endl;
//        cout<<pos_i<<"--"<<pos_j<<endl;
//        for(int i = 0; i < data_item_update.size(); i++) {
//            for(int j = 0; j < data_item_update[i].size(); j++) {
//                //                cout<<task_set[0].data_item[i].getReleaseTime()<<"   "<<task_set[0].data_item[i].getDeadline()<<endl;
//                cout<<i<<"  "<<j<<":"<<data_item_update[i][j].getReleaseTime()<<"   "<<data_item_update[i][j].getDeadline()<<endl;
//                //                /cout<<data_item_update_copy[0][i].getReleaseTime()<<"   "<<data_item_update_copy[0][i].getDeadline()<<endl;
//            }
//        }
//        cout<<"--------"<<endl;

        //retrieve current data item
        if(time + data_item_update[pos_i][pos_j].getValidInterval() >= task_set[pos_i].getDeadline()) {
            //when a data item is retrieved and its task deadline does not become earlier, continue
            time++;
            data_item_update[pos_i][pos_j].setIsFinished(true);
            data_item_update[pos_i][pos_j].setRetrievalTime(time - 1);
            //            cout<<">="<<endl;
            //synchronize the task_set and data_item_update
            for(int i = 0; i < data_item_update.size(); i++) {
                task_set[i].setDataItem(data_item_update[i]);
            }

            //            if(time > data_item_update[pos_i][pos_j].getDeadline() || time > task_set[pos_i].getDeadline()) {
            ////            if(time > data_item_update[pos_i][pos_j].getDeadline()) {
            //                cout<<">= not meet deadline"<<endl;
            //                cout<<time<<endl;
            //                cout<<pos_i<<"  "<<pos_j<<endl;
            //                return false;
            //            }

            //            cout<<time<<endl;
            //            cout<<"current data item can be retrieved successfully"<<endl;
            continue;
        }
        else {
            //            cout<<"we need EDF to process current time"<<endl;
            //when a data item is retrieved and its task deadline becomes earlier, record time t
            //retrieve current data item
            time++;
            if(pos_i != -1 && pos_j != -1) {
                data_item_update[pos_i][pos_j].setIsFinished(true);
                data_item_update[pos_i][pos_j].setRetrievalTime(time - 1);
                if(time > data_item_update[pos_i][pos_j].getDeadline() || time > task_set[pos_i].getDeadline()) {
                    cout<<"< not meet deadline"<<endl;
                    return false;
                }
            }

            //for all data items that have been released in data set q, set their release time as current time
            for(int i = 0; i < data_item_update.size(); i++) {
                for(int j = 0; j < data_item_update[i].size(); j++) {
                    if(data_item_update[i][j].getReleaseTime() <= time && !data_item_update[i][j].getIsFinished()) {
                        if(time >= data_item_update[i][j].getDeadline()) {
                            cout<<"cannot set release time to t"<<endl;
                            return false;
                        }
                        //                        recover.push_back(make_pair(i, j));
                        data_item_update[i][j].setReleaseTime(time);
                    }
                }
            }

            //            //update the task deadline since it should become earlier
            //            if(!postpone_task[pos_i]) {
            //                task_set[pos_i].setDeadline(time - 1 + data_item_update[pos_i][pos_j].getValidInterval());
            //                cout<<"postpone the task deadline"<<endl;
            //            }
            //            else {
            //                postpone_task[pos_i] = false;
            //            }

            //            cout<<recover.size()<<endl;
            //synchronize the task_set and data_item_update
            for(int i = 0; i < data_item_update.size(); i++) {
                task_set[i].setDataItem(data_item_update[i]);
            }

            data_item_update_copy.clear();
            //set the copy of data_item_update to record the original deadline of each date item
            for(auto task : task_set) {
                //                data_item_update_copy.push_back(task.getDataItem());
                data_item_update_copy.push_back(task.data_item);
            }

            bool deadline_less_release = false;
            task_deadline_original.clear();
            //set tasks' deadline given current time
            for(int i = 0; i < task_set.size(); i++) {
                int new_task_deadline = task_set[i].getDeadline();
                task_deadline_original[i] = task_set[i].getDeadline();
                for(int j = 0; j < task_set[i].data_item.size(); j++) {
                    if(task_set[i].data_item[j].getIsFinished() && task_set[i].data_item[j].getRetrievalTime() +  task_set[i].data_item[j].getValidInterval() < new_task_deadline) {
                        //                        if(!postpone_task[i]) {
                        //                            //get the expiration time of retrieved data item in current task
                        //                            new_task_deadline = task_set[i].data_item[j].getRetrievalTime() + task_set[i].data_item[j].getValidInterval();
                        //                        }
                        new_task_deadline = task_set[i].data_item[j].getRetrievalTime() + task_set[i].data_item[j].getValidInterval();
                    }
                }
                task_set[i].setDeadline(new_task_deadline);
                //                if(postpone_task[i]) {
                //                    postpone_task[i] = false;
                //                }

                //update the deadline of each data item in certain task
                if(i == pos_i) {
                    int x_i = 0;
                    for(int j = 0; j < task_set[i].data_item.size(); j++) {
                        //judge if CR is satisfied, actually is, so we do the backtracing operation
                        if(task_set[i].data_item[j].getReleaseTime() >= task_set[i].getDeadline()) {
                            x_i = max(x_i, task_set[i].data_item[j].getReleaseTime() + 1 - task_set[i].getDeadline());
                        }
                    }
                    if(x_i != 0) {
                        deadline_less_release = true;
                        //recover the data items to the status at time - 1
                        data_item_update = data_item_update_original;
                        if(time == data_item_update[pos_i][pos_j].getDeadline()) {
                            cout<<"special case: cannot move release time forward"<<endl;
                            return false;
                        }
                        else {
                            if(time - 1 + x_i < data_item_update[pos_i][pos_j].getDeadline()) {
                                data_item_update[pos_i][pos_j].setReleaseTime((time - 1 + x_i));
                            }
                            else {
                                data_item_update[pos_i][pos_j].setReleaseTime(data_item_update[pos_i][pos_j].getDeadline() - 1);
                            }
                        }
//                        cout<<"release time is changed: "<<data_item_update[previous_pos_i][previous_pos_j].getReleaseTime()<<endl;
                        data_item_update[pos_i][pos_j].setIsFinished(false);
                        if(task_deadline_original[pos_i] != -1) {
                            task_set[pos_i].setDeadline(task_deadline_original[pos_i]);
                        }
                        //synchronize the task_set and data_item_update
                        for(int i = 0; i < data_item_update.size(); i++) {
                            task_set[i].setDataItem(data_item_update[i]);
                        }

                        //retrieve data item by EDF at current time - 1
//                        cout<<"time-- occurs"<<endl;
                        time--;
                        break;
                    }
                }
            }
            if(deadline_less_release == true) {
                //                cout<<"go into another CR judgement"<<endl;
                continue;
            }

            //                //update the deadline of each data item in certain task
            //                for(int j = 0; j < task_set[i].data_item.size(); j++) {
            //                    if(task_set[i].data_item[j].getDeadline() > task_set[i].getDeadline()) {
            //                        if(task_set[i].data_item[j].getReleaseTime() >= task_set[i].getDeadline()) {
            //                            cout<<"deadline is smaller than release time"<<endl;
            //                            return false;
            //                            //judge if CR is satisfied, actually is, so we do the backtracing operation
            //
            //                        }
            //                        task_set[i].data_item[j].setDeadline(task_set[i].getDeadline());
            //                    }
            //                }

            //            cout<<task_set[0].getDeadline()<<endl;

            //reivse the deadline of task and date items based on D1
            for(int i = 0; i < task_set.size(); i++) {
                int max_dataitem_deadline = INT_MIN;
                for(int k = 0; k < task_set[i].data_item.size(); k++) {
                    max_dataitem_deadline = max(max_dataitem_deadline, task_set[i].data_item[k].getDeadline());
                }
                if(task_set[i].getDeadline() > max_dataitem_deadline) {
                    task_set[i].setDeadline(max_dataitem_deadline);  //set task deadline as the maximum of all data items in task
                }
                for(int j = 0; j < task_set[i].data_item.size(); j++) {
                    if(task_set[i].data_item[j].getDeadline() > task_set[i].getDeadline()) {
                        task_set[i].data_item[j].setDeadline(task_set[i].getDeadline());
                    }
                }
            }
            //revise the data item's deadline based on D2
            //at time t, we need to revise deadline of all data items to be different

            data_item_update.clear();

            for(auto task : task_set) {
                //                data_item_update.push_back(task.getDataItem());
                data_item_update.push_back(task.data_item);
            }

            bool distinct = true;

            //get the larggest deadline of all data items which are not retrieved
            int larggest_deadline = INT_MIN;
            int smallest_deadline = INT_MAX;
            for(int i = 0; i < data_item_update.size(); i++) {
                for(int j = 0; j < data_item_update[i].size(); j++) {
                    if(!data_item_update[i][j].getIsFinished()) {
                        larggest_deadline = max(data_item_update[i][j].getDeadline(), larggest_deadline);
                        smallest_deadline = min(data_item_update[i][j].getDeadline(), smallest_deadline);
                    }
                }
            }
            //set the position of data item which cannot make distinct deadline with others
            long nondistinct_pos_i = -1, nondistinct_pos_j = -1;

            //set the number of conflict data items at time slot s
            int conflict_dataitem = 0, start_release_time = -1;

            while(distinct && larggest_deadline >= smallest_deadline) {
                bool find_larggest_deadline = false;
                for(long i = 0; i < data_item_update.size(); i++) {
                    for(int j = 0; j < data_item_update[i].size(); j++) {
                        if(data_item_update[i][j].getDeadline() == larggest_deadline && !data_item_update[i][j].getIsFinished()) {
                            find_larggest_deadline = true;
                            //transfer the data item with larggest deadline to the function of checkDistinct()
                            int result = checkDistinct(data_item_update[i][j], i, j, data_item_update, smallest_deadline);

                            if(result == 2) {
                                //                                cout<<"result == 2"<<endl;
                                //                                cout<<i<<"  "<<j<<endl;
                                distinct = false;
                                nondistinct_pos_i = i;
                                nondistinct_pos_j = j;
                                for(int k = 0; k < data_item_update.size(); k++) {
                                    for(int l = 0; l < data_item_update[k].size(); l++) {
                                        if(data_item_update[k][l].getDeadline() == data_item_update[i][j].getDeadline() &&
                                           data_item_update[k][l].getReleaseTime() == data_item_update[i][j].getReleaseTime()) {
                                            conflict_dataitem++;
                                        }
                                    }
                                }
                                start_release_time = data_item_update[i][j].getReleaseTime();
                                i = data_item_update.size();
                                break;
                            }
                            if(result == 1) {
                                //                                cout<<"result == 1 larggest_deadline ="<<larggest_deadline<<endl;
                                //                                cout<<smallest_deadline<<endl;
                                i = data_item_update.size();
                                break;
                            }
                            if(result == 0) {
                                //                                cout<<"result == 0"<<endl;
                                larggest_deadline--;
                                i = data_item_update.size();
                                break;
                            }
                        }
                    }
                }
                if(!find_larggest_deadline) {
                    larggest_deadline--;
                }
            }
//            cout<<distinct<<endl;
//            cout<<time<<endl;
//            cout<<pos_i<<"--"<<pos_j<<endl;
//            for(int i = 0; i < data_item_update.size(); i++) {
//                for(int j = 0; j < data_item_update[i].size(); j++) {
//                    //                    cout<<task_set[0].data_item[i].getReleaseTime()<<"   "<<task_set[0].data_item[i].getDeadline()<<endl;
//                    cout<<i<<"  "<<j<<":"<<data_item_update[i][j].getReleaseTime()<<"   "<<data_item_update[i][j].getDeadline()<<endl;
//                    //                    cout<<data_item_update_copy[0][i].getReleaseTime()<<"   "<<data_item_update_copy[0][i].getDeadline()<<endl;
//                }
//            }
//            cout<<"--------"<<endl;

            P_ij.clear();

            //data items cannot make distinct, so they are not schedulable
            if(distinct == false) {
                int max_deadline_P_ij = INT_MIN;
                bool CR = false;
                for(int i = 0; i < data_item_update_copy.size(); i++) {
                    for(int j = 0; j < data_item_update_copy[i].size(); j++) {
                        if(data_item_update_copy[i][j].getDeadline() > start_release_time) {
                            if(data_item_update_copy[i][j].getDeadline() - start_release_time >= P_ij.size() + 1 ||
                               max_deadline_P_ij - start_release_time >= P_ij.size() + 1) {
                                P_ij.push_back(data_item_update_copy[i][j]);
                                //判断i是否是在当前更改变小的task，限制判断当前task
                                if(data_item_update_copy[i][j].getDeadline() > task_set[i].getDeadline() && i == previous_pos_i) {
                                    CR = true;
                                }
                                max_deadline_P_ij = max(max_deadline_P_ij, data_item_update_copy[i][j].getDeadline());
                            }
                        }
                    }
                }
                //                cout<<max_deadline_P_ij<<endl;
                //                cout<<P_ij.size()<<endl;
                //                for(int i = 0; i < P_ij.size(); i++) {
                //                    cout<<P_ij[i].getReleaseTime()<<"   "<<P_ij[i].getDeadline()<<endl;
                //                }
                //since data items cannot make distinct, we need to examine CR now

//                cout<<"conflict data items: "<<conflict_dataitem<<endl;
//                cout<<"time: "<<time<<endl;

                //CR is satisfied
                if(CR) {
                    //recover the data items to the status at time - 1
                    data_item_update = data_item_update_original;
                    if(conflict_dataitem != 0) {
                        //revise the release time of retrieved data item at time - 1 based on R1
                        if(time == data_item_update[previous_pos_i][previous_pos_j].getDeadline()) {
                            cout<<"special case: cannot move release time forward"<<endl;
                            //                            postpone_task[previous_pos_i] = true;;
                            //                            data_item_update[previous_pos_i][previous_pos_j].setReleaseTime(time - 1);
                            return false;
                        }
                        else {
                            if(time - 1 + conflict_dataitem - 1 < data_item_update[previous_pos_i][previous_pos_j].getDeadline()) {
                                data_item_update[previous_pos_i][previous_pos_j].setReleaseTime(time - 1 + conflict_dataitem - 1);
                            }
                            else {
                                data_item_update[previous_pos_i][previous_pos_j].setReleaseTime(data_item_update[previous_pos_i][previous_pos_j].getDeadline() - 1);
                            }
                        }
//                        cout<<"release time is changed: "<<data_item_update[previous_pos_i][previous_pos_j].getReleaseTime()<<endl;
                        data_item_update[previous_pos_i][previous_pos_j].setIsFinished(false);
                        if(task_deadline_original[previous_pos_i] != -1) {
                            task_set[previous_pos_i].setDeadline(task_deadline_original[previous_pos_i]);
                        }
                        //不能修改task的deadline，应该直接报错
                        //                        task_set[previous_pos_i].setDeadline(task_set[previous_pos_i].getDeadline() + conflict_dataitem - 1);
                    }

                    //synchronize the task_set and data_item_update
                    for(int i = 0; i < data_item_update.size(); i++) {
                        task_set[i].setDataItem(data_item_update[i]);
                    }

                    //retrieve data item by EDF at current time - 1
                    time--;
                    continue;
                }
                else {
                    //                    cout<<"this task set is not schedulable"<<endl;
                    cout<<"not meet CR condition"<<endl;
                    return false;
                }

            }
            else {
                //synchronize the task_set and data_item_update
                for(int i = 0; i < data_item_update.size(); i++) {
                    task_set[i].setDataItem(data_item_update[i]);
                }
                continue;
            }

        }

    }
    if(!allRetrieved(task_set)) {
        cout<<"not all data items are retrieved"<<endl;
        return false;
    }
    //    cout<<"this task set is schedulable"<<endl;

    return true;
}


/*
 implementation of the optimal algorithm in RTSS 2016 smart varient
 */

bool scheduleSmart(vector<Task>& task_set) {

    //set a single vector to store all of the data items to further usage in idle remove procedure
    vector<DataItem> all_data_item;

    //set data_item_update to judge if the deadlines of all data items which have not been retrieved are different
    vector<vector<DataItem>> data_item_update;

    /*
     set start time of algorithm
     */
    int time = INT_MAX; //initial the start time of algorithm
    int max_task_deadline = INT_MIN;
    for(int i = 0; i < task_set.size(); i++) {
        time = min(time, task_set[i].getReleaseTime());
        max_task_deadline = max(max_task_deadline, task_set[i].getDeadline());
    }
    /*
     we can set for loop here as the real beginning of algorithm
     */
    while(!allRetrievedTasks(task_set) && time < max_task_deadline) {

        //set tasks' deadline given current time
        for(int i = 0; i < task_set.size(); i++) {
            int new_task_deadline = task_set[i].getSED();
            if(!task_set[i].getIsFinished()) {
                for(int j = 0; j < task_set[i].data_item.size(); j++) {
                    if(task_set[i].data_item[j].getIsFinished() && task_set[i].data_item[j].getRetrievalTime() +  task_set[i].data_item[j].getValidInterval() < new_task_deadline) {
                        //get the expiration time of retrieved data item in current task
                        new_task_deadline = task_set[i].data_item[j].getRetrievalTime() +  task_set[i].data_item[j].getValidInterval();
                    }
                }
                task_set[i].setSED(new_task_deadline);
            }
        }

        /*
         first find the task to be executed
         */
//                cout<<"time: "<<time<<endl;
        //find the current task to be executed
        int current_task = findTaskWithSmallestDeadline(task_set, time);

//                cout<<"current_task: "<<current_task<<endl;

        //sort date items in current task due to LVF
        sort(task_set[current_task].data_item.begin(), task_set[current_task].data_item.end(), [](DataItem&a, DataItem&b) {
            return a.getValidInterval() > b.getValidInterval();
        });

        /*
         in the selected task, find the data item to be retrieved with LVF rule
         */

        //find the first non-finished data item and retrieve it, since we have select the task with the smallest deadline, we do not need
        //to worry about the new arriving data item at time t.
        bool out_from_break = false;
        for(int i = 0; i < task_set[current_task].data_item.size(); i++) {
            if(!task_set[current_task].data_item[i].getIsFinished() && time >= task_set[current_task].data_item[i].getReleaseTime()) {

//                                cout<<"retrieve "<<i<<"th data item "<<task_set[current_task].data_item[i].getReleaseTime()<<"  "<<task_set[current_task].data_item[i].getDeadline()<<"   "<<task_set[current_task].getDeadline()<<endl;
                //data avaliability
                if(time + 1 > task_set[current_task].data_item[i].getDeadline()) {
                    cout<<"not meet data avaliability"<<endl;
                    return false;
                }

                if(time + 1 > task_set[current_task].getSED()) {
                    cout<<"not meet data freshness"<<endl;
                    return false;
                }

                task_set[current_task].data_item[i].setRetrievalTime(time);
                task_set[current_task].data_item[i].setIsFinished(true);
                time++;
                if(allRetrievedInTask(task_set[current_task])) {
                    task_set[current_task].setIsFinished(true);
                    task_set[current_task].setFinishTime(time);
                    //                    if(task_set[current_task].getDeadline() < task_set[current_task].getFinishTime() || !allValid(task_set[current_task])) {
                    if(task_set[current_task].getDeadline() < task_set[current_task].getFinishTime()) {
                        cout<<"deadline is smaller than finish time"<<endl;
                        return false;
                    }
                }
                out_from_break = true;
                break;
            }
        }
        if(!out_from_break) {
            //            cout<<"no data item was retrieved"<<endl;
            time++;
        }
    }

    if(!allRetrievedTasks(task_set)) {
        return false;
    }
    return true;
}

/*
 implementation of the optimal algorithm in RTSS 2016
 */

bool scheduleRTSS(vector<Task>& task_set) {

    //set a single vector to store all of the data items to further usage in idle remove procedure
    vector<DataItem> all_data_item;

    //set data_item_update to judge if the deadlines of all data items which have not been retrieved are different
    vector<vector<DataItem>> data_item_update;

    /*
     set start time of algorithm
     */
    int time = INT_MAX; //initial the start time of algorithm
    int max_task_deadline = INT_MIN;
    for(int i = 0; i < task_set.size(); i++) {
        time = min(time, task_set[i].getReleaseTime());
        max_task_deadline = max(max_task_deadline, task_set[i].getDeadline());
    }
    /*
     we can set for loop here as the real beginning of algorithm
     */

    while(!allRetrievedTasks(task_set) || time >= max_task_deadline) {

        //set tasks' deadline given current time
        for(int i = 0; i < task_set.size(); i++) {
            int new_task_deadline = task_set[i].getDeadline();
            if(!task_set[i].getIsFinished()) {
                for(int j = 0; j < task_set[i].data_item.size(); j++) {
                    if(task_set[i].data_item[j].getIsFinished() && task_set[i].data_item[j].getRetrievalTime() +  task_set[i].data_item[j].getValidInterval() < new_task_deadline) {
                        //get the expiration time of retrieved data item in current task
                        new_task_deadline = task_set[i].data_item[j].getRetrievalTime() +  task_set[i].data_item[j].getValidInterval();
                    }
                }
                task_set[i].setDeadline(new_task_deadline);
            }
        }

        /*
         first find the task to be executed
         */

        //find the current task to be executed
        int current_task = findTaskWithSmallestDeadline(task_set, time);

        //sort date items in current task due to LVF
        sort(task_set[current_task].data_item.begin(), task_set[current_task].data_item.end(), [](DataItem&a, DataItem&b) {
            return a.getValidInterval() > b.getValidInterval();
        });

        /*
         in the selected task, find the data item to be retrieved with LVF rule
         */

        //find the first non-finished data item and retrieve it, since we have select the task with the smallest deadline, we do not need
        //to worry about the new arriving data item at time t.

        int i = 0;
        //find the first non-finished data item in current task
        while(task_set[current_task].data_item[i].getIsFinished()) {
            i++;
        }
        if(time < task_set[current_task].data_item[i].getReleaseTime()) {
            return false;
        }
        else {
            //            if(time + 1 > task_set[current_task].data_item[i].getDeadline()) {
            //                return false;
            //            }
            task_set[current_task].data_item[i].setRetrievalTime(time);
            task_set[current_task].data_item[i].setIsFinished(true);
            time++;
            if(allRetrievedInTask(task_set[current_task])) {
                task_set[current_task].setIsFinished(true);
                task_set[current_task].setFinishTime(time);
                if(task_set[current_task].getDeadline() < task_set[current_task].getFinishTime() || !allValid(task_set[current_task])) {
                    return false;
                }
            }
        }
    }
    if(!allRetrievedTasks(task_set)) {
        return false;
    }
    return true;
}

/*
 implementation of the optimal algorithm in RTSS 2016 simple varient
 */

bool scheduleSimple(vector<Task>& task_set) {

    //set a single vector to store all of the data items to further usage in idle remove procedure
    vector<DataItem> all_data_item;

    //set data_item_update to judge if the deadlines of all data items which have not been retrieved are different
    vector<vector<DataItem>> data_item_update;

    /*
     set start time of algorithm
     */
    int time = INT_MAX; //initial the start time of algorithm
    int max_task_deadline = INT_MIN;
    for(int i = 0; i < task_set.size(); i++) {
        time = min(time, task_set[i].getReleaseTime());
        max_task_deadline = max(max_task_deadline, task_set[i].getDeadline());
    }

    /*
     we can set for loop here as the real beginning of algorithm
     */

    while(!allRetrievedTasks(task_set) || time >= max_task_deadline) {

        //set tasks' deadline given current time
        for(int i = 0; i < task_set.size(); i++) {
            int new_task_deadline = task_set[i].getDeadline();
            if(!task_set[i].getIsFinished()) {
                for(int j = 0; j < task_set[i].data_item.size(); j++) {
                    if(task_set[i].data_item[j].getIsFinished() && task_set[i].data_item[j].getRetrievalTime() +  task_set[i].data_item[j].getValidInterval() < new_task_deadline) {
                        //get the expiration time of retrieved data item in current task
                        new_task_deadline = task_set[i].data_item[j].getRetrievalTime() +  task_set[i].data_item[j].getValidInterval();
                    }
                }
                task_set[i].setDeadline(new_task_deadline);
            }
        }

        /*
         first find the task to be executed
         */

        //find the current task to be executed
        int current_task = findTaskWithSmallestDeadline(task_set, time);

        //sort date items in current task due to LVF
        sort(task_set[current_task].data_item.begin(), task_set[current_task].data_item.end(), [](DataItem&a, DataItem&b) {
            return a.getValidInterval() > b.getValidInterval();
        });

        /*
         in the selected task, find the data item to be retrieved with LVF rule
         */

        //find the first non-finished data item and retrieve it, since we have select the task with the smallest deadline, we do not need
        //to worry about the new arriving data item at time t.

        int i = 0;
        //find the first non-finished data item in current task
        while(task_set[current_task].data_item[i].getIsFinished()) {
            i++;
        }
        if(time < task_set[current_task].data_item[i].getReleaseTime()) {
            time = task_set[current_task].data_item[i].getReleaseTime();
        }
        //check data availability
        //        if(time + 1 > task_set[current_task].data_item[i].getDeadline()) {
        //            return false;
        //        }
        task_set[current_task].data_item[i].setRetrievalTime(time);
        task_set[current_task].data_item[i].setIsFinished(true);
        time++;
        if(allRetrievedInTask(task_set[current_task])) {
            task_set[current_task].setIsFinished(true);
            task_set[current_task].setFinishTime(time);
            if(task_set[current_task].getDeadline() < task_set[current_task].getFinishTime() || !allValid(task_set[current_task])) {
                return false;
            }
        }
    }
    if(!allRetrievedTasks(task_set)) {
        return false;
    }
    return true;
}

/*
 this is the main function which initial all data items and tasks, execute the scheduling alogrithm for general case
 */

int main(int argc, const char * argv[]) {
    //    //initiall tasks
    //    vector<Task> task_set(1);
    //    task_set[0].setReleaseTime(0);
    //    task_set[0].setDeadline(200);
    //
    //    //initial data items in each task
    //    vector<DataItem> data(5);
    //
    //    vector<vector<int>> test = {{0,6,100}, {3,7,100}, {0,5,6}, {3,5,100}, {6,9,100}};
    //    for(int i = 0; i < test.size(); i++) {
    //        data[i].setReleaseTime(test[i][0]);
    //        data[i].setDeadline(test[i][1]);
    //        data[i].setValidInterval(test[i][2]);
    //    }
    //
    //    task_set[0].setDataItem(data);


    int task_num = 1;

    //initiall tasks
    //    vector<vector<Task>> tasks(100, vector<Task>(10));
    vector<vector<Task>> tasks(10000, vector<Task>(task_num));

    //    //initiall tasks
    //    vector<vector<Task>> tasks(10, vector<Task>(3));
    //    vector<vector<Task>> tasks(10, vector<Task>(5));

    ifstream fileinput("/Users/liuqiangqiang/Documents/IndependentStudy/inputdata/final/10000_3d_utl_taskno/N1U10.txt", ios::in);
    //    ifstream fileinput("/Users/liuqiangqiang/Documents/IndependentStudy/inputdata/differentA/F10.txt", ios::in);
    //    ifstream fileinput("/Users/liuqiangqiang/Documents/IndependentStudy/inputdata/3task_5item.txt", ios::in);
    //    ifstream fileinput("/Users/liuqiangqiang/Documents/IndependentStudy/inputdata/5task5data.txt", ios::in);
    if(!fileinput) {
        cerr<<"cannot open file"<<endl;
        exit(0);
    }
    long long interval;
    vector<int> task_line(4);
    vector<int> dataitem_line(5);


    for(int i = 0; i < 10000; i++) {
        fileinput >> interval;
        int dataitem_count = 0;
        for(int j = 0; j < task_num; j++) {
            for(int k = 0; k < 4; k++) {
                fileinput >> task_line[k];
            }
            tasks[i][j].setReleaseTime(task_line[1]);
            tasks[i][j].setDeadline(task_line[2]);
            tasks[i][j].setSED(task_line[2]);//set the sed equals to dealine originally
            vector<DataItem> temp(task_line[3]);
            tasks[i][j].setDataItem(temp);
            dataitem_count += task_line[3];
        }
        fileinput >> interval;
        for(int j = 0; j < dataitem_count; j++) {
            for(int k = 0; k < 5; k++) {
                fileinput >> dataitem_line[k];
            }
            tasks[i][dataitem_line[0] - 1].data_item[dataitem_line[1] - 1].setReleaseTime(dataitem_line[2]);
            tasks[i][dataitem_line[0] - 1].data_item[dataitem_line[1] - 1].setDeadline(dataitem_line[3]);
            tasks[i][dataitem_line[0] - 1].data_item[dataitem_line[1] - 1].setValidInterval(dataitem_line[4]);
        }
    }

    fileinput.close();
    int feasible = 0;

    //    for(int i = 0; i < 10; i++) {
    //        if(schedule(tasks[i])) {
    //            cout<<i<<"  1"<<endl;
    //            usleep(microseconds);
    //            feasible++;
    //        }
    //        else {
    //            cout<<i<<"  0"<<endl;
    //            usleep(microseconds);
    //        }
    //    }

    for(int i = 0; i < 10000; i++) {
        if(schedule(tasks[i])) {
            cout<<i<<"  1"<<endl;
            //            usleep(microseconds);
            feasible++;
            cout<<feasible<<endl;
        }
        else {
            cout<<i<<"  0"<<endl;
            //            usleep(microseconds);
            cout<<feasible<<endl;
        }
    }
    cout<<feasible<<endl;
    return 0;
}
