//
// Created by Siannodel on 2022/10/7.
//

#ifndef EXTRACT_EXTRACT_H
#define EXTRACT_EXTRACT_H

#include <iostream>
#include <vector>
#include <set>
#include <unordered_map>
#include <fstream>
#include <unordered_set>
using namespace std;

#define ONLY_TARGET_FEATURES true

const int LEN = 1e5 + 5;

vector<vector<pair<string, string>>> tmpSks;

unsigned tarTagkPos;

unordered_map<string, unsigned> dfValue, metricFreq, metricAccessCnt, highFreqMkInQuery;

unordered_set<string> highFreqMetric, lowFreqMetric;


#endif //EXTRACT_EXTRACT_H
