#include <iostream>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <queue>
#include <chrono>
#include "recall.h"
#include "fstlibModified.h"

using namespace std;
vector<fst::map<uint32_t>> fstMp;

unsigned nHFTCnt, nHFMCnt, qTCnt, attrTCnt;
bool outputFlag;

chrono::microseconds forwardIndexDuration;
chrono::microseconds invertedIndexDuration;
chrono::microseconds IODuration;
chrono::microseconds filterDuration;

vector<string> stringSplit(const string& str, char delim)
{
    stringstream ss(str);
    string item;
    vector<string> elems;
    while (getline(ss, item, delim))
    {
        if (!item.empty())
            elems.emplace_back(item);
    }
    return elems;
}

void nonHighFreqMetricRecallProcess()
{
    // TODO
    cout << "「未完成」だって何度でも言うんだ！" << endl;
    ++nHFMCnt;
}

void recallProcess()
{
    auto res = dataModelRange.find(queryMetric);
    if(res == dataModelRange.end())
    {nonHighFreqMetricRecallProcess(); return;}
    auto l = res->second.first, r = res->second.second;
    vector<pair<string, string>> attrTagkv, nonHighFreqTagkv;
    vector<unsigned> groupsId;
    vector<pair<unsigned, int>> originalPostings;
    // find posting.
    for(unsigned i = 0; i < queryTagkv.size(); ++i)
    {
        auto start = chrono::system_clock::now(); //forward index duration start.
        auto res1 = tagk2FSTid.find(queryTagkv[i].first);
        auto end = chrono::system_clock::now(); // forward index duration end.
        forwardIndexDuration += chrono::duration_cast<chrono::microseconds>(end - start);

        if(res1 != tagk2FSTid.end())
        {
            auto start = chrono::system_clock::now(); //forward index duration start.
            unsigned postingOffset;
            auto& mp = fstMp[res1->second];
            if(!mp.contains(queryTagkv[i].second))
            {
                if(!mp.contains("\u001E"))
                {
                    //cout << i.second << endl;
                    continue;
                }
                //cout << i.second << endl;
                postingOffset = mp["\u001E"];
                nonHighFreqTagkv.emplace_back(queryTagkv[i]);
            }
            else
                postingOffset = mp[queryTagkv[i].second];
            auto end = chrono::system_clock::now(); // forward index duration end.
            forwardIndexDuration += chrono::duration_cast<chrono::microseconds>(end - start);

            start = chrono::system_clock::now(); // invert index duration start.
            ifstream postingFile("../DataModel/postings",ios::in|ios::binary);
            postingFile.seekg(postingOffset);
            unsigned siz, num;
            postingFile.read((char *)&siz, sizeof(unsigned));
            for(auto &k : originalPostings) if(k.second == 1) k.second = 0;
            unsigned pos = 0;
            while(siz--)
            {
                postingFile.read((char *)&num, sizeof(unsigned));
                if(i == 0 && l <= num && num <= r)
                    originalPostings.emplace_back(make_pair(num, 1));
                else if(i)
                {
                    if(pos >= originalPostings.size()) continue;
                    while(pos < originalPostings.size() - 1 && originalPostings[i].first < num)
                    {originalPostings[pos++].second = -1;} // お前はもう死んでいる。
                    if(originalPostings[pos].first == num && originalPostings[pos].second == 0)
                        originalPostings[pos++].second = 1;
                    if(siz == 0)
                        while(pos < originalPostings.size()) originalPostings[pos++].second = -1;
                }
            }
            end = chrono::system_clock::now(); // invert index duration end.
            invertedIndexDuration += chrono::duration_cast<chrono::microseconds>(end - start);
        }
        else attrTagkv.emplace_back(queryTagkv[i]);
    }
    if(attrTagkv.size() == queryTagkv.size()) // 不幸だー!!
        for(unsigned i = l; i <= r; ++i) groupsId.emplace_back(i);
    else for(auto &k : originalPostings) if(k.second == 1) groupsId.emplace_back(k.first);

    //attrTCnt += attrTagkv.size(), nHFTCnt += nonHighFreqTagkv.size(), qTCnt += queryTagkv.size();


    ofstream outfile("queryResult/queryResult" + to_string(queryCount));
    if(attrTagkv.empty() && nonHighFreqTagkv.empty())
    {
        for(auto &groupId : groupsId)
        {
            ifstream infile("../DataModel/Groups/group" + to_string(groupId));
            unsigned tagkListId;
            string sk;
            vector<pair<string, string>> tarTagkvs;
            vector<string> tmpTarTagkvs;

            auto start = chrono::system_clock::now(); // IO duration start.
            infile >> sk;
            auto end = chrono::system_clock::now(); // IO duration end.
            IODuration += chrono::duration_cast<chrono::microseconds>(end - start);

            tmpTarTagkvs = stringSplit(sk, '\u001D');
            for(auto &i : tmpTarTagkvs)
            {
                vector<string> resVec = stringSplit(i, '\u001F');
                tarTagkvs.emplace_back(make_pair(resVec[0], resVec[1]));
            }
            while(infile >> tagkListId)
            {
                sk = queryMetric + '\u001E';
                vector<string> tagvs;
                string tagvString;

                start = chrono::system_clock::now(); // IO duration start.
                infile >> tagvString;
                end = chrono::system_clock::now(); // IO duration end.
                IODuration += chrono::duration_cast<chrono::microseconds>(end - start);

                tagvs = stringSplit(tagvString, '\u001F');
                vector<string>& tagkList = tagkLists[tagkListId];
                unsigned tarTagsPos = 0, tagvPos = 0;
                for(int i = 0; i < tagkList.size(); ++i)
                {
                    if(i) sk += '\u001D';
                    sk += tagkList[i] + '\u001F';
                    if(tagkList[i] == tarTagkvs[tarTagsPos].first && tarTagkvs[tarTagsPos].second != "\u001E")
                        sk += tarTagkvs[tarTagsPos++].second;
                    else
                        sk += tagvs[tagvPos++];
                }
                outfile << sk << endl;
                outputFlag = true;
            }
            infile.close();
        }
    }
    else
    {
        for(auto &groupId : groupsId)
        {
            ifstream infile("../DataModel/Groups/group" + to_string(groupId));
            unsigned tagkListId;
            string sk;
            vector<pair<string, string>> tarTagkvs;
            vector<string> tmpTarTagkvs;

            auto start = chrono::system_clock::now(); // IO duration start.
            infile >> sk;
            auto end = chrono::system_clock::now(); // IO duration end.
            IODuration += chrono::duration_cast<chrono::microseconds>(end - start);

            tmpTarTagkvs = stringSplit(sk, '\u001D');
            for(auto &i : tmpTarTagkvs)
            {
                vector<string> resVec = stringSplit(i, '\u001F');
                tarTagkvs.emplace_back(make_pair(resVec[0], resVec[1]));
            }
            while(infile >> tagkListId)
            {
                unsigned attrTagsHitCount = 0, nHFTagvHitCount = 0;
                sk = queryMetric + '\u001E';
                vector<string> tagvs;
                string tagvString;

                start = chrono::system_clock::now(); // IO duration start.
                infile >> tagvString;
                end = chrono::system_clock::now(); // IO duration end.
                IODuration += chrono::duration_cast<chrono::microseconds>(end - start);

                tagvs = stringSplit(tagvString, '\u001F');
                vector<string>& tagkList = tagkLists[tagkListId];
                unsigned tarTagsPos = 0, tagvPos = 0;

                start = chrono::system_clock::now(); // filter duration start.
                for(int i = 0; i < tagkList.size(); ++i)
                {
                    if(i) sk += '\u001D';
                    sk += tagkList[i] + '\u001F';
                    string tagv;
                    if(tarTagsPos < tarTagkvs.size() &&
                    tagkList[i] == tarTagkvs[tarTagsPos].first && tarTagkvs[tarTagsPos].second != "\u001E")
                        sk += (tagv = tarTagkvs[tarTagsPos++].second);
                    else
                    {
                        sk += (tagv = tagvs[tagvPos++]);
                        if(tarTagsPos < tarTagkvs.size() &&
                           tagkList[i] == tarTagkvs[tarTagsPos].first && tarTagkvs[tarTagsPos].second == "\u001E")
                            ++tarTagsPos;
                    }

                    for(auto &tagkv : attrTagkv)
                        if(tagkv.first == tagkList[i] && tagkv.second == tagv)
                        {++attrTagsHitCount; break;}
                    for(auto &tagkv : nonHighFreqTagkv)
                        if(tagkv.first == tagkList[i] && tagkv.second == tagv)
                        {++nHFTagvHitCount; break;}
                }
                end = chrono::system_clock::now(); // filter duration end.
                filterDuration += chrono::duration_cast<chrono::microseconds>(end - start);

                if(attrTagsHitCount == attrTagkv.size() && nHFTagvHitCount == nonHighFreqTagkv.size())
                { outfile  << sk << endl; outputFlag = true; }
            }
            infile.close();
        }
    }
    outfile.close();
}

void readQuery(const string& filename)
{
    ifstream infile(filename);
    unsigned tagsCount;
    string tagk, tagv;
//    ofstream hitQueryFile("queryHit");

    while(infile >> queryMetric)
    {
        infile >> tagsCount;
        queryTagkv.clear();
        while(tagsCount--)
        {
            infile >> tagk >> tagv;
            queryTagkv.emplace_back(make_pair(tagk, tagv));
        }
        ++queryCount;
//        outputFlag = false;
        recallProcess();
//        if(outputFlag)
//        {
//            hitQueryFile << queryMetric << ' ' << queryTagkv.size() << ' ';
//            for(auto &i : queryTagkv) hitQueryFile << i.first << ' ' << i.second << ' ';
//            hitQueryFile << endl;
//        }
    }
//    hitQueryFile.close();
}

void readInfo()
{
    for(unsigned i = 0;; ++i)
    {
        ifstream infile("../DataModel/FST/fst" + to_string(i),ios::in | ios::binary | ios::ate);
        if(!infile.is_open()) break;
        infile.seekg(0, ios::end);
        int file_len = (int)infile.tellg();
        infile.seekg(0, ios::beg);
        string str;
        char* tmp_str = new char[file_len];
        infile.read(tmp_str, file_len);
        infile.close();
        fst::map<uint32_t> mp(tmp_str, file_len);
        fstMp.emplace_back(mp);
    }

    ifstream infile("../DataModel/dataModelInfo");
    string metric;
    unsigned l, r;
    while(infile >> metric)
    {
        infile >> l >> r;
        dataModelRange.insert(make_pair(metric, make_pair(l, r)));
    }
    infile.close();

    string tagk;
    unsigned FSTid;
    infile.open("../DataModel/tagkArt");
    while(infile >> tagk)
    {
        infile >> FSTid;
        tagk2FSTid.insert(make_pair(tagk, FSTid));
    }
    infile.close();

    infile.open("../DataModel/tagkListDict");
    string tagkList;
    while(infile >> tagkList)
        tagkLists.emplace_back(stringSplit(tagkList, '\u001F'));
    infile.close();
}

int main()
{
    readInfo();
    readQuery("../Baseline/query205");
    cout << nHFTCnt << ' ' << nHFMCnt << ' ' << qTCnt << ' ' << attrTCnt << endl;

    cout << "Forward index time cost: " << ((double)(forwardIndexDuration.count()) *
                              chrono::microseconds::period::num) << endl;
    cout << "Inverted index time cost: " << ((double)(invertedIndexDuration.count()) *
                              chrono::microseconds::period::num) << endl;
    cout << "IO time cost: " << ((double)(IODuration.count()) *
                              chrono::microseconds::period::num) << endl;
    cout << "Filter time cost: " << ((double)(filterDuration.count()) *
                              chrono::microseconds::period::num) << endl;

    // P50:  85520 1354 121362 105 18w
    // P80:  75874 1354 121362 105 80w
    // P90:  70122 1354 121362 105 77.63MB
    // P100: 55758 1354 121362 105 180w 125MB
    return 0;
}
