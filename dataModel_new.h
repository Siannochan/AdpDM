//
// Created by Siannodel on 2022/9/8.
//

#ifndef DATAMODEL_DATAMODEL_H
#define DATAMODEL_DATAMODEL_H

#include <vector>
#include <cstring>
#include <set>
#include <unordered_map>
#include <fstream>

#define CARD_SORT true
#define TAGK_LIST true
#define MULTI_TAGV true
#define SINGLE_TAGV false

const unsigned BLOCK_SIZE = 256 * 1024;
const unsigned LEN = 1e5 + 5;

using namespace std;

unordered_map<string, unsigned> tagkLists;
vector<string> id2tagkList;

vector<vector<pair<string, string>>> tmpSks;

struct Group
{
    vector<pair<string, string>> tarTags;
    vector<pair<unsigned, vector<string>>> seriesKey; // tagkList id and attr tagv.
};

vector<Group> groups;
unsigned groupCount;

struct DataModel
{
    string metric;
    set<string> tagkSet;
    vector<string> tarTagk, attrTagk;
    unordered_map<string, set<string>> tarTagv;
    unsigned groupRangeL, groupRangeR;
};

vector<DataModel> dataModels;

unordered_map<string, unsigned> tagk2TagvSetId;
vector<unordered_map<string, unsigned>> tagv2PostingId;
vector<vector<unsigned>> postings;

unordered_map<string, unsigned> dfRank;

unsigned tarTagkPos;

set<string> highFreqMetric;

ofstream tagvFreqFile("tagvFreqInfo");

unsigned allGroupCount;

unordered_map<string, unsigned> mkInfo;

// global tagv dict.
unordered_map<string, unsigned> attrTagvFreq, tagvDict;
bool firstTurn = SINGLE_TAGV;

#endif //DATAMODEL_DATAMODEL_H
