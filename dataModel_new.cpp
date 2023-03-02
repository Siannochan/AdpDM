#include <iostream>
#include <algorithm>
#include <cstring>
#include <unordered_set>
#include <chrono>
#include "dataModel_new.h"
#include "fstlibModified.h"
using namespace std;

chrono::microseconds duration;

void chooseTarTagkByQueryLog()
{
    DataModel& dataModel = dataModels[dataModels.size() - 1];
    unordered_map<string, unsigned> tagkScorePre;
    unordered_map<string, unordered_map<string, unsigned>> tagvFreq;
    unsigned tarTagkCnt = 0;
    for(auto &tmpSk : tmpSks)
    {
        for(auto &tagkv : tmpSk)
        {
            auto res = tagkScorePre.find(tagkv.first);
            if(res == tagkScorePre.end())
            {
                auto res1 = mkInfo.find(dataModel.metric + tagkv.first);
                if(res1 != mkInfo.end())
                {
                    ++tarTagkCnt;
                    tagkScorePre.insert(make_pair(tagkv.first, res1->second));
                }
                else
                    tagkScorePre.insert(make_pair(tagkv.first, 0x7fffffff));
            }

            auto res1 = tagvFreq.find(tagkv.first);
            if(res1 == tagvFreq.end())
            {
                unordered_map<string, unsigned> tmpMp;
                tmpMp.insert(make_pair(tagkv.second, 1u));
                tagvFreq.insert(make_pair(tagkv.first, tmpMp));
            }
            else
            {
                auto res2 = res1->second.find(tagkv.second);
                if(res2 == res1->second.end())
                    res1->second.insert(make_pair(tagkv.second, 1u));
                else
                    ++res2->second;
            }
        }
    }
    vector<pair<string, unsigned>> tagkScore(tagkScorePre.begin(), tagkScorePre.end());
    sort(tagkScore.begin(), tagkScore.end(), [=](const pair<string, double>& x, const pair<string, double>& y){
        return x.second < y.second;
    });

    if(CARD_SORT)
    {
        vector<pair<string, unsigned>> tagkCard;
        for(auto &i : tagvFreq) tagkCard.emplace_back(make_pair(i.first, i.second.size()));
        sort(tagkCard.begin(), tagkCard.end(), [=](const pair<string, double>& x, const pair<string, double>& y){
            return x.second > y.second;
        });

        unordered_map<string, unsigned> disc;
        for(int i = 0; i < tagkCard.size(); ++i)
            disc.insert(make_pair(tagkCard[i].first, i));

        for(auto &tmpSk : tmpSks)
        {
            vector<pair<unsigned, string>> vec;
            vec.reserve(tmpSk.size());
            for(auto &tagkv : tmpSk)
                vec.emplace_back(make_pair(disc.find(tagkv.first)->second, tagkv.second));
            sort(vec.begin(), vec.end(), [=](const pair<unsigned, string>& x, const pair<unsigned, string>& y){
                return x.first < y.first;
            });
            for(int i = 0; i < tmpSk.size(); ++i)
                tmpSk[i] = make_pair(tagkCard[vec[i].first].first, vec[i].second);
        }
    }
    else
    {
        unordered_map<string, unsigned> disc;
        for(int i = 0; i < tagkScore.size(); ++i)
            disc.insert(make_pair(tagkScore[i].first, i));

        for(auto &tmpSk : tmpSks)
        {
            vector<pair<unsigned, string>> vec;
            vec.reserve(tmpSk.size());
            for(auto &tagkv : tmpSk)
                vec.emplace_back(make_pair(disc.find(tagkv.first)->second, tagkv.second));
            sort(vec.begin(), vec.end(), [=](const pair<unsigned, string>& x, const pair<unsigned, string>& y){
                return x.first < y.first;
            });
            for(int i = 0; i < tmpSk.size(); ++i)
                tmpSk[i] = make_pair(tagkScore[vec[i].first].first, vec[i].second);
        }
    }

    //ofstream denialOfQueryFile;
    //denialOfQuery.open("denialOfQuery", ios::app);
    for(int i = 0; i < tagkScore.size(); ++i)
    {
        dataModel.tagkSet.insert(tagkScore[i].first);
        if(i < tarTagkCnt)
        {
            dataModel.tarTagk.emplace_back(tagkScore[i].first);
            auto res = tagvFreq.find(tagkScore[i].first);
            vector<pair<string, unsigned>> tmpVec(res->second.begin(), res->second.end());
            sort(tmpVec.begin(), tmpVec.end(), [=](const pair<string, unsigned>& x, const pair<string, unsigned>& y){
                return x.second > y.second;
            });
            unsigned cntAll = 0, cntNow = 0;
            for(int j = 0; j < tmpVec.size(); ++j)
                if(tmpVec[j].second > 10) // P100: if(tmpVec[j].second > 0) or if(tmpVec[j].second)
                {
                    cntAll += tmpVec[j].second;
                    tagvFreqFile << j << '/' << tmpVec.size() << ' ' << dataModel.metric << "  "
                             << tagkScore[i].first << ": " << tmpVec[j].first << ' ' << tmpVec[j].second << endl;
                }
            set<string> tmpSet;
            for(auto &j : tmpVec)
            {
                // tagvFreqFile << dataModel.metric << ' ' << tagkScore[i].first << ' ' << j.first << endl;
                if(firstTurn && cntNow * 10 > cntAll * 9) // P100: if(firstTurn && cntNow * 10 > cntAll * 10) or delete this.
                {
                    auto tagvRes = attrTagvFreq.find(j.first);
                    if(tagvRes == attrTagvFreq.end())
                        attrTagvFreq.insert(j);
                    else tagvRes->second += j.second;
                }
                else if(!firstTurn && cntNow * 10 <= cntAll * 9) tmpSet.insert(j.first); // P100: (!firstTurn && cntNow * 10 <= cntAll * 10) or delete this.
                //else if(!firstTurn && cntNow * 10 > cntAll * 9) denialOfQuery << dataModel.metric << ' ' << tagkScore[i].first << ' ' << j.first << endl;
                cntNow += j.second;
            }
            if(!firstTurn) dataModel.tarTagv.insert(make_pair(tagkScore[i].first, tmpSet));
        }
        else
        {
            if(firstTurn)
            {
                auto mpRes = tagvFreq.find(tagkScore[i].first);
                for(auto &tagv : mpRes->second)
                {
                    auto tagvRes = attrTagvFreq.find(tagv.first);
                    if(tagvRes == attrTagvFreq.end())
                        attrTagvFreq.insert(tagv);
                    else
                        tagvRes->second += tagv.second;
                }
            }
            dataModel.attrTagk.emplace_back(tagkScore[i].first);
        }
    }
    //denialOfQuery.close();
}
ofstream outfile1("target_mk");
void chooseTarTagkByDF()
{
    DataModel& dataModel = dataModels[dataModels.size() - 1];
    unordered_map<string, unsigned> tagkScorePre;
    unordered_map<string, unordered_map<string, unsigned>> tagvFreq;
    for(auto &tmpSk : tmpSks)
    {
        for(auto &tagkv : tmpSk)
        {
            auto res = tagkScorePre.find(tagkv.first);
            if(dfRank.find(tagkv.first) == dfRank.end())
                cout << tagkv.first << endl;
            if(res == tagkScorePre.end())
                tagkScorePre.insert(make_pair(tagkv.first, dfRank.find(tagkv.first)->second));

            auto res1 = tagvFreq.find(tagkv.first);
            if(res1 == tagvFreq.end())
            {
                unordered_map<string, unsigned> tmpMp;
                tmpMp.insert(make_pair(tagkv.second, 1u));
                tagvFreq.insert(make_pair(tagkv.first, tmpMp));
            }
            else
            {
                auto res2 = res1->second.find(tagkv.second);
                if(res2 == res1->second.end())
                    res1->second.insert(make_pair(tagkv.second, 1u));
                else
                    ++res2->second;
            }
        }
    }
    vector<pair<string, unsigned>> tagkScore(tagkScorePre.begin(), tagkScorePre.end());
    sort(tagkScore.begin(), tagkScore.end(), [=](const pair<string, double>& x, const pair<string, double>& y){
        return x.second < y.second;
    });

    unordered_map<string, unsigned> discCard;
    for(int i = 0; i < tagkScore.size(); ++i)
        discCard.insert(make_pair(tagkScore[i].first, i));

//    for(auto &tmpSk : tmpSks)
//    {
//        vector<pair<unsigned, string>> vec;
//        vec.reserve(tmpSk.size());
//        for(auto &tagkv : tmpSk)
//            vec.emplace_back(make_pair(discCard.find(tagkv.first)->second, tagkv.second));
//        sort(vec.begin(), vec.end(), [=](const pair<unsigned, string>& x, const pair<unsigned, string>& y){
//            return x.first < y.first;
//        });
//        for(int i = 0; i < tmpSk.size(); ++i)
//            tmpSk[i] = make_pair(tagkScore[vec[i].first].first, vec[i].second);
//    }

    for(auto & i : tagkScore)
    {
        dataModel.tagkSet.insert(i.first);
        if(i.second <= tarTagkPos)
        {
            outfile1 << dataModel.metric << ' ' << i.first << endl;
            dataModel.tarTagk.emplace_back(i.first);
            auto res = tagvFreq.find(i.first);
            vector<pair<string, unsigned>> tmpVec(res->second.begin(), res->second.end());
            sort(tmpVec.begin(), tmpVec.end(), [=](const pair<string, unsigned>& x, const pair<string, unsigned>& y){
                return x.second > y.second;
            });
            unsigned cntAll = 0, cntNow = 0;
            for(auto & j : tmpVec)
                if(j.second > 10)
                    cntAll += j.second;
            //     tagvFreqFile << j << '/' << tmpVec.size() << ' ' << dataModel.metric << "  "
            //     << tagkScore[i].first << ": " << tmpVec[j].first << ' ' << tmpVec[j].second << endl;
            set<string> tmpSet;
            for(auto &j : tmpVec)
            {
                tagvFreqFile << dataModel.metric << ' ' << i.first << ' ' << j.first << endl;
                tmpSet.insert(j.first);
                cntNow += j.second;
                if(cntNow * 10 > cntAll * 9) break;
            }
            dataModel.tarTagv.insert(make_pair(i.first, tmpSet));
        }
        else
            dataModel.attrTagk.emplace_back(i.first);
    }
}

void chooseTarTagk()
{
    DataModel& dataModel = dataModels[dataModels.size() - 1];
    unordered_map<string, set<string>> tagkCardPre;
    unordered_map<string, unsigned> tagkFreq, tagvFreq;
    for(auto &tmpSk : tmpSks)
    {
        for(auto &tagkv : tmpSk)
        {
            auto res1 = tagkCardPre.find(tagkv.first);
            if(res1 == tagkCardPre.end())
            {
                set<string> tmpSet;
                tmpSet.insert(tagkv.second);
                tagkCardPre.insert(make_pair(tagkv.first, tmpSet));
            }
            else
                res1->second.insert(tagkv.second);

            auto res2 = tagkFreq.find(tagkv.first);
            if(res2 == tagkFreq.end())
                tagkFreq.insert(make_pair(tagkv.first, 1u));
            else
                ++res2->second;

            if(tagkv.first == "poz_kpg_blvls_grdcu")
            {
                auto res3 = tagvFreq.find(tagkv.second);
                if(res3 == tagvFreq.end())
                    tagvFreq.insert(make_pair(tagkv.second, 1u));
                else
                    ++res3->second;
            }
        }
    }

    vector<pair<string, double>> tagkScore;
    tagkScore.reserve(tagkCardPre.size());
    for(auto &i : tagkCardPre)
        tagkScore.emplace_back(i.first, (double)i.second.size() / (double)tagkFreq.find(i.first)->second);

    //for(auto &i : tagkScore)
    //  cout << i.first << ' ' << tagkCardPre.find(i.first)->second.size() << ' ' << tagkFreq.find(i.first)->second << ' ' << i.second << endl;

    vector<pair<string, unsigned>> tagVec(tagvFreq.begin(), tagvFreq.end());
    sort(tagVec.begin(), tagVec.end(), [=](const pair<string, unsigned>& x, const pair<string, unsigned>& y){
        return x.second < y.second;
    });
    for(auto &i : tagVec)
        cout << i.first << ' ' << i.second << endl;

    sort(tagkScore.begin(), tagkScore.end(), [=](const pair<string, double>& x, const pair<string, double>& y){
        return x.second < y.second;
    });

    unordered_map<string, unsigned> discCard;
    for(int i = 0; i < tagkScore.size(); ++i)
        discCard.insert(make_pair(tagkScore[i].first, i));

    for(auto &tmpSk : tmpSks)
    {
        vector<pair<unsigned, string>> vec;
        vec.reserve(tmpSk.size());
        for(auto &tagkv : tmpSk)
            vec.emplace_back(make_pair(discCard.find(tagkv.first)->second, tagkv.second));
        sort(vec.begin(), vec.end(), [=](const pair<unsigned, string>& x, const pair<unsigned, string>& y){
            return x.first < y.first;
        });
        for(int i = 0; i < tmpSk.size(); ++i)
            tmpSk[i] = make_pair(tagkScore[vec[i].first].first, vec[i].second);
    }

    for(int i = 0; i < tagkScore.size(); ++i)
    {
        dataModel.tagkSet.insert(tagkScore[i].first);
        if(i <= tagkScore.size() / 5)
            dataModel.tarTagk.emplace_back(tagkScore[i].first);
        else
            dataModel.attrTagk.emplace_back(tagkScore[i].first);
    }
}

void genDataModel(const string& metric)
{
    // Now we choose all metrics, so it's no need to filter.
//    if(highFreqMetric.find(metric) == highFreqMetric.end())
//    {
//        ofstream nonHighFreqMetricFile("nonHighFreqMetricSks", ios::app);
//        nonHighFreqMetricFile << metric << endl;
//        string tagkList = "";
//        for(auto &tmpSk : tmpSks)
//        {
//            for(int i = 0; i < tmpSk.size(); ++i)
//            {
//                if(i) nonHighFreqMetricFile << '\u001D';
//                nonHighFreqMetricFile << tmpSk[i].first << '\u001F' << tmpSk[i].second;
//            }
//            nonHighFreqMetricFile << endl;
//        }
//        nonHighFreqMetricFile.close();
//        groups.clear();
//        tmpSks.clear();
//        return;
//    }
//    if(metric != "eda_wefi_bfbdvt_vfryfwq_htqfw")
//    {
//        groups.clear();
//        tmpSks.clear();
//        return;
//    }
    DataModel tmpDataModel;
    tmpDataModel.metric = metric;
    dataModels.emplace_back(tmpDataModel);
    chooseTarTagkByQueryLog();
    if(firstTurn) { groups.clear(); tmpSks.clear(); return; }

    DataModel& dataModel = dataModels[dataModels.size() - 1];
    unordered_map<string, unsigned> tagsList2GroupId;

    for(auto &tmpSk : tmpSks)
    {
        auto start = chrono::system_clock::now();
        vector<pair<string, string>> tarTagkvList;
        vector<string> attrTagvList;
        for(auto &tagkv : tmpSk)
        {
            auto res = dataModel.tarTagv.find(tagkv.first);
            if(res != dataModel.tarTagv.end())
            {
                if(res->second.find(tagkv.second) == res->second.end())
                {
                    tarTagkvList.emplace_back(make_pair(tagkv.first, "\u001E"));
                    attrTagvList.emplace_back(tagkv.second);
                }
                else
                    tarTagkvList.emplace_back(tagkv);
            }
            else attrTagvList.emplace_back(tagkv.second);
        }
        string tagsList = metric;
        for(auto &i : tarTagkvList)
        {
            tagsList += i.first;
            tagsList += i.second;
        }
        auto end = chrono::system_clock::now();
        duration += chrono::duration_cast<chrono::microseconds>(end - start);

        // find tagkList id.
        string tagkList = "";
        for(auto &tagkv : tmpSk)
            tagkList += (tagkv.first + "\u001F");

        unsigned tagkListId;
        if(tagkLists.find(tagkList) == tagkLists.end())
        {
            tagkLists.insert(make_pair(tagkList, tagkListId = tagkLists.size()));
            id2tagkList.emplace_back(tagkList);
        }
        else
            tagkListId = tagkLists.find(tagkList)->second;

        auto res = tagsList2GroupId.find(tagsList);
        unsigned groupId;
        if(res == tagsList2GroupId.end())
        {
            Group group;
            group.tarTags = tarTagkvList;
            group.seriesKey.emplace_back(make_pair(tagkListId, attrTagvList));
            groups.emplace_back(group);
            groupId = groups.size() - 1;
            tagsList2GroupId.insert(make_pair(tagsList, groupId));

            for(auto &i : tarTagkvList)
            {
                start = chrono::system_clock::now();
                auto res1 = tagk2TagvSetId.find(i.first);
                if(res1 == tagk2TagvSetId.end())
                {
                    vector<unsigned> posting;
                    postings.emplace_back(posting);
                    postings[postings.size() - 1].emplace_back(groupId + allGroupCount);
                    unordered_map<string, unsigned> tagvSet;
                    tagvSet.insert(make_pair(i.second, postings.size() - 1));
                    tagv2PostingId.emplace_back(tagvSet);
                    tagk2TagvSetId.insert(make_pair(i.first, tagv2PostingId.size() - 1));
                }
                else
                {
                    auto res2 = tagv2PostingId[res1->second].find(i.second);
                    if(res2 == tagv2PostingId[res1->second].end())
                    {
                        vector<unsigned> posting;
                        postings.emplace_back(posting);
                        postings[postings.size() - 1].emplace_back(groupId + allGroupCount);
                        tagv2PostingId[res1->second].insert(make_pair(i.second, postings.size() - 1));
                    }
                    else
                        postings[res2->second].emplace_back(groupId + allGroupCount);
                }
                end = chrono::system_clock::now();
                duration += chrono::duration_cast<chrono::microseconds>(end - start);
            }
        }
        else
            groups[groupId = res->second].seriesKey.emplace_back(make_pair(tagkListId, attrTagvList));


    }

//     group part.
    //ofstream outfile("allGroups", ios::app);
    for(auto &i : groups)
    {
        ofstream outfile("Groups/group" + to_string(groupCount++));

        // outfile << metric << '\u001E';
        if(MULTI_TAGV)
        {
            for(int j = 0; j < i.tarTags.size(); ++j)
            {
                if(j) outfile << '\u001D';
                outfile << i.tarTags[j].first << '\u001F' << i.tarTags[j].second;
            }
            outfile << endl;
        }

        for(auto &j : i.seriesKey)
        {
            if(TAGK_LIST) outfile << j.first << ' ';
            else outfile << id2tagkList[j.first] << ' ';

            if(!MULTI_TAGV)
                for(int k = 0; k < i.tarTags.size(); ++k)
                {
                    if(k) outfile << '\u001F';
                    outfile << i.tarTags[k].second;
                }

            for(int k = 0; k < j.second.size(); ++k)
            {
                if(k) outfile << '\u001F';
                auto dictRes = tagvDict.find(j.second[k]);
                if(!SINGLE_TAGV || dictRes == tagvDict.end())
                    outfile << j.second[k];
                else
                    outfile << '\u001E' << dictRes->second;
            }
            outfile << endl;
        }
        outfile.close();
    }

    dataModel.groupRangeL = allGroupCount;
    allGroupCount += groups.size();
    dataModel.groupRangeR = allGroupCount - 1;
    groups.clear();
    tmpSks.clear();
}

void df(const string& filename)
{
    ifstream infile(filename);
    unordered_map<string, unsigned> tagkFreq;
    string metric, tagk;
    double tmp;
    while(infile >> metric)
    {
        infile >> tagk >> tmp >> tmp >> tmp;
        auto res = tagkFreq.find(tagk);
        if(res == tagkFreq.end())
            tagkFreq.insert(make_pair(tagk, 1u));
        else
            ++res->second;
    }
    infile.close();

    vector<pair<string, unsigned>> vec(tagkFreq.begin(), tagkFreq.end());
    sort(vec.begin(), vec.end(), [=](const pair<string, unsigned>& x, const pair<string, unsigned>& y){
        return x.second > y.second;
    });

    tarTagkPos = vec.size() / 3;
    for(int i = 0; i < vec.size(); ++i)
        dfRank.insert(make_pair(vec[i].first, i));
    ofstream df_feature("df_feature");
    for(auto &i : vec)
        df_feature << i.first << " " << i.second << endl;
}

void readFile(const string& filename)
{
    auto start = chrono::system_clock::now();
    ifstream infile(filename);
    char sk[LEN], metric[LEN], tagk[LEN], tagv[LEN];
    string lastMetric = "Siannodel";
    while(infile.getline(sk, LEN - 1))
    {
        vector<pair<string, string>> tmpSk;
        unsigned i;

        // read the metric part.
        for(i = 1; sk[i] != '\u001E'; ++i)
            metric[i - 1] = sk[i];
        metric[i - 1] = '\0';
        if(metric != lastMetric)
        {
            auto end = chrono::system_clock::now();
            duration += chrono::duration_cast<chrono::microseconds>(end - start);
            if(lastMetric != "Siannodel")
                genDataModel(lastMetric);
            lastMetric = metric;
            start = chrono::system_clock::now();
        }
        // read the tagkv part.
        bool now_tagk = true;
        unsigned j = 0;
        for(++i;; ++i)
        {
            if(sk[i] == '\u001D' || sk[i + 1] == '\0')
            {
                tagv[j] = '\0';
                string tagk_str = tagk, tagv_str = tagv;
                //cout << tagk_str << ' ' << tagv_str << endl;
                tmpSk.emplace_back(make_pair(tagk_str, tagv_str));
                if(sk[i + 1] == '\0') break;
                now_tagk = true;
                j = 0;
                continue;
            }
            if(sk[i] == '\u001F')
            {now_tagk = false; tagk[j] = '\0'; j = 0; continue;}
            if(now_tagk)
                tagk[j++] = sk[i];
            else
                tagv[j++] = sk[i];
        }
        tmpSks.emplace_back(tmpSk);
    }

    genDataModel(metric);
    infile.close();
    cout << "Finish reading file." << endl;
}

void writeFile()
{
    vector<unsigned> postingOffset;
    // posting part.
    ofstream outfile("postings", ios::out | ios::binary);
   // ofstream outfile("postings");
    for(auto &i : postings)
    {
        postingOffset.emplace_back(outfile.tellp()); // It's here now!
        unsigned siz = i.size();
        outfile.write(reinterpret_cast<char const*>(&siz), sizeof(unsigned));
        for(auto &j : i)
            outfile.write(reinterpret_cast<char const*>(&j), sizeof(unsigned));
//    for(auto &j : i)
//        outfile << j << ' ';
//    outfile << endl;
    }
    outfile.close();

    for(auto &i : tagv2PostingId)
        for(auto &j : i)
            j.second = postingOffset[j.second];

    // fst part.
    unsigned fstCount = 0;
    for(auto &i : tagv2PostingId)
    {
        std::stringstream ss;
        const vector<pair<string, unsigned>> tmpVec(i.begin(), i.end());
        fst::compile<uint32_t>(tmpVec, ss, false);
        const auto& byte_code0 = ss.str();
        ofstream outfile("FST/fst" + to_string(fstCount++), ios::out | ios::binary);
        outfile.write((char*)byte_code0.data(), byte_code0.size());
        outfile.close();
    }
    cout << "FST count: " << fstCount << endl;

    // tagk art part.
    // TODO: store art.
    outfile.open("tagkArt");
    for(auto &i : tagk2TagvSetId)
        outfile << i.first + ' ' + to_string(i.second) << endl;
    outfile.close();

    // data model info.
    outfile.open("dataModelInfo");
    for(auto & dataModel : dataModels)
    {
        outfile << dataModel.metric << ' ' << dataModel.groupRangeL << ' '
        << dataModel.groupRangeR << endl;
//        outfile << dataModel.tarTagk.size() << ' ' << dataModel.metric << endl;
//        for(const auto& j : dataModel.tarTagk)
//            outfile << j << endl;
    }
    outfile.close();

    // tagkList part.
    outfile.open("tagkListDict");
    vector<pair<string, unsigned>> tagkListsVec(tagkLists.begin(), tagkLists.end());
    sort(tagkListsVec.begin(), tagkListsVec.end(), [=](const pair<string, unsigned> &x, const pair<string, unsigned> &y){
        return x.second < y.second;
    });
    for(auto &tagkList : tagkListsVec)
        outfile << tagkList.first << endl;
    outfile.close();

    // tagv dict part.
    outfile.open("globalTagvDict");
    for(auto &i : tagvDict)
        outfile << i.first << ' ' << i.second << endl;
    outfile.close();

    cout << "Group count: " << allGroupCount << endl;
}

void metricProc(const string& filename)
{
    ifstream infile(filename);
    string metric;
    unsigned num;
    while(infile >> metric)
    {
        highFreqMetric.insert(metric);
        infile >> num;
    }
    infile.close();
}

void dfPre(const string& filename)
{
    ifstream infile(filename);
    ofstream outfile("metricTagk");
    unordered_map<string, short> mp;
    char sk[LEN], metric[LEN], tagk[LEN], tagv[LEN];
    string lastMetric = "Siannodel";
    unsigned long long cntAll = 0;
    while(infile.getline(sk, LEN - 1))
    {
        ++cntAll;
        unsigned i;
        // read the metric part.
        for(i = 0; sk[i] != '\u001E'; ++i)
            metric[i] = sk[i];
        metric[i] = '\0';
        if(metric != lastMetric)
        {
            if(lastMetric != "Siannodel")
            {
                for(auto &mk : mp)
                    outfile << metric << ' ' << mk.first << endl;
                mp.clear();
            }
            lastMetric = metric;
        }

        // read the tagkv part.
        bool now_tagk = true;
        unsigned j = 0;
        for(++i;; ++i)
        {
            if(sk[i] == '\u001D' || sk[i] == '\0')
            {
                tagv[j] = '\0';
                string tagk_str = tagk;
                if(mp.find(tagk_str) == mp.end())
                    mp.insert(make_pair(tagk_str, 0));

                if(sk[i] == '\0') break;
                now_tagk = true;
                j = 0;
                continue;
            }
            if(sk[i] == '\u001F')
            {now_tagk = false; tagk[j] = '\0'; j = 0; continue;}
            if(now_tagk)
                tagk[j++] = sk[i];
            else
                tagv[j++] = sk[i];
        }
    }
    infile.close();
    outfile.close();
    cout << cntAll << endl;
}

void mkPre(const string& filename)
{
    ifstream infile(filename);
    string metric, tagk;
    unsigned cnt = 0; double tmp;
    while(infile >> metric)
    {
        infile >> tagk>>tmp>>tmp>>tmp>>tmp>>tmp>>tmp>>tmp>>tmp>>tmp;
        mkInfo.insert(make_pair(metric + tagk, ++cnt));
        highFreqMetric.insert(metric);
    }
    infile.close();
}

void checkMetric(const string& filename)
{
    ifstream infile(filename);
    char sk[LEN];
    string last_metric = "Siannodel";
    unordered_set<string> metrics;
    while(infile.getline(sk, LEN - 1))
    {
        for(auto &i : sk)
            if(i == '\u001E')
                i = '\0';
        if(sk != last_metric && metrics.find(sk) != metrics.end())
            cout << sk << endl;
        metrics.insert(sk);
        last_metric = sk;
    }
}

void tgk_v(const string& filename)
{
    ifstream infile(filename);
    char sk[LEN], metric[LEN], tagk[LEN], tagv[LEN];
    unordered_map<string, vector<unsigned>> posting;
    unsigned skCnt = 0;
    while(infile.getline(sk, LEN - 1))
    {
        ++skCnt;
        unsigned i;
        // read the metric part.
        for(i = 1; sk[i] != '\u001E'; ++i)
            metric[i - 1] = sk[i];
        metric[i - 1] = '\0';
        // read the tagkv part.
        bool now_tagk = true;
        unsigned j = 0;
        for(++i;; ++i)
        {
            if(sk[i] == '\u001D' || sk[i + 1] == '\0')
            {
                tagv[j] = '\0';
                string tagk_str = tagk, tagv_str = tagv;
                auto res = posting.find(tagk_str + tagv_str);
                if(res == posting.end())
                {
                    vector<unsigned> vec;
                    vec.emplace_back(skCnt);
                    posting.insert(make_pair(tagk_str + tagv_str, vec));
                }
                else
                    res->second.emplace_back(skCnt);

                if(sk[i + 1] == '\0') break;
                now_tagk = true;
                j = 0;
                continue;
            }
            if(sk[i] == '\u001F')
            {now_tagk = false; tagk[j] = '\0'; j = 0; continue;}
            if(now_tagk)
                tagk[j++] = sk[i];
            else
                tagv[j++] = sk[i];
        }
    }
    infile.close();

    // posting part.
    ofstream outfile("originPosting", ios::out | ios::binary);
    for(auto &i : posting)
    {
//        unsigned siz = i.second.size();
//        outfile.write(reinterpret_cast<char const*>(&siz), sizeof(unsigned));
//        for(auto &j : i.second)
//            outfile.write(reinterpret_cast<char const*>(&j), sizeof(unsigned));
    for(auto &j : i.second)
        outfile << j << ' ';
    outfile << endl;
    }
    outfile.close();
}

void globalTagvDictGen()
{
    firstTurn = false, dataModels.clear();
    vector<pair<string, unsigned>> tagvFreqVec(attrTagvFreq.begin(), attrTagvFreq.end());
    sort(tagvFreqVec.begin(), tagvFreqVec.end(), [=](pair<string, unsigned>& x, pair<string, unsigned>& y){
        return x.second > y.second;
    });
    ofstream tagvDictPre("tagvDictPre");
    for(auto &i : tagvFreqVec) tagvDictPre << i.first << ' ' << i.second << endl;
    unsigned t = (1000u < tagvFreqVec.size() ? 1000u : tagvFreqVec.size());
    for(unsigned i = 0; i < t; ++i)
        tagvDict.insert(make_pair(tagvFreqVec[i].first, i));
}

int main()
{
    // dfPre("../data/series-key-35223");
    // df("features");
    // metricProc("0928");
//    ofstream nonHighFreqMetricFile("nonHighFreqMetricSks");//groupFile("allGroups");
//    nonHighFreqMetricFile.close();// groupFile.close();
    mkPre("asi_1339_target_mk");
    readFile("../newData/series-key-asi-1340");
    if(SINGLE_TAGV)
    {
        globalTagvDictGen();
        readFile("../newData/series-key-asi-1340");
    }
   // writeFile();
        cout << "Time cost: " << ((double)(duration.count()) *
                              chrono::microseconds::period::num) << endl;
    //tgk_v("../newData/series-key-10048");
    // checkMetric("../data/series-key-92640");
    return 0;
}
