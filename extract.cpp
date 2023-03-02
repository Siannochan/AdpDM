#include <algorithm>
#include "extract.h"

using namespace std;

ofstream features_file, lowFreqMetricFeaturesFile;
void pre(const string& filename)
{
    ifstream infile(filename);
    ofstream outfile("metricTagk");
    unordered_map<string, short> mp;
    char sk[LEN], metric[LEN], tagk[LEN], tagv[LEN];
    string lastMetric = "Siannodel";
    unsigned long long cntAll = 0, cntMetricSize = 0;
    vector<pair<string, pair<unsigned, unsigned>>> metricFeatureVec; // <metric, <metricFreq, metricCard>>
    while(infile.getline(sk, LEN - 1))
    {
        ++cntAll, ++cntMetricSize;
        unsigned i;
        
        // read the metric part.
        for(i = 1; sk[i] != '\u001E'; ++i)
            metric[i - 1] = sk[i];
        metric[i - 1] = '\0';
        if(metric != lastMetric)
        {
            if(lastMetric != "Siannodel")
            {
                for(auto &mk : mp)
                    outfile << lastMetric << ' ' << mk.first << endl;
                metricFeatureVec.emplace_back(make_pair(metric, make_pair(cntMetricSize, mp.size())));
                mp.clear();
            }
            lastMetric = metric;
            cntMetricSize = 1;
        }

        // read the tagkv part.
        bool now_tagk = true;
        unsigned j = 0;
        for(++i;; ++i)
        {
            if(sk[i] == '\u001D' || sk[i + 1] == '\0')
            {
                tagv[j] = '\0';
                string tagk_str = tagk;
                if(mp.find(tagk_str) == mp.end())
                    mp.insert(make_pair(tagk_str, 0));

                if(sk[i + 1] == '\0') break;
                now_tagk = true;
                j = 0;
                continue;
            }
            if(sk[i] == '\u001F' && sk[i - 1] != '\u001F')
            {now_tagk = false; tagk[j] = '\0'; j = 0; continue;}
            if(now_tagk)
                tagk[j++] = sk[i];
            else
                tagv[j++] = sk[i];
        }
    }
    for(auto &mk : mp)
        outfile << metric << ' ' << mk.first << endl;
    mp.clear();
    metricFeatureVec.emplace_back(make_pair(metric, make_pair(cntMetricSize, mp.size())));

    infile.close();
    outfile.close();
    cout << "all serieskeys count: " << cntAll << endl;

    sort(metricFeatureVec.begin(), metricFeatureVec.end(), [=]
    (const pair<string, pair<unsigned, unsigned>>& x, const pair<string, pair<unsigned, unsigned>>& y){
        return x.second.first > y.second.first;
    });

    for(auto &i : metricFeatureVec)
        metricFreq.insert(make_pair(i.first, i.second.first));

    unsigned cntNow = 0;


    ofstream mfFile("MetricFeatures");

    for(int i = 0; /*cntNow * 100 <= cntAll * 95*/ i < metricFeatureVec.size(); ++i)
    {
        if(cntNow * 100 <= cntAll * 95)
            highFreqMetric.insert(metricFeatureVec[i].first);
        else
            lowFreqMetric.insert(metricFeatureVec[i].first);
        cntNow += metricFeatureVec[i].second.first;
    }
    for(int i = 0; i < metricFeatureVec.size(); ++i)
    {
        auto res = metricAccessCnt.find(metricFeatureVec[i].first);
        mfFile << metricFeatureVec[i].first << ' ' << metricFeatureVec[i].second.first
        << ' ' << metricFeatureVec[i].second.second << ' ';
        mfFile << (res == metricAccessCnt.end() ? 0u : res->second) << endl;
    }
    mfFile.close();
}

void df(const string& filename)
{
    ifstream infile(filename);
    unordered_map<string, unsigned> tagkFreq;
    string metric, tagk;
    while(infile >> metric)
    {
        infile >> tagk;
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

      tarTagkPos = vec[vec.size() / 3].second;
    for(auto & i : vec)
        dfValue.insert(i);
    ofstream df_feature("df_feature");
    for(auto &i : vec)
        df_feature << i.first << " " << i.second << endl;
}

void queryLogRead(const string& filename, double ratio)
{
    ifstream yFile(filename);
    string metric, tagk;
    unsigned num, cntAll = 0, cntNow = 0;
    while(yFile >> metric)
    {
        yFile >> tagk >> num;
        cntAll += num;
    }
    yFile.close();
    yFile.open(filename);
    while(yFile >> metric)
    {
        yFile >> tagk >> num;
        auto res = metricAccessCnt.find(metric);
        if(res == metricAccessCnt.end())
            metricAccessCnt.insert(make_pair(metric, 1u));
        else
            ++res->second;
        if(cntNow * 100 < cntAll * ratio)
            highFreqMkInQuery.insert(make_pair(metric + tagk, num));
        cntNow += num;
    }
}

void extractFeatures(const string& metric)
{
    unordered_map<string, unsigned> tagkScorePre, tagkFreq;
    unordered_map<string, unordered_set<string>> tagkCardPre;
    for(auto &tmpSk : tmpSks)
    {
        for(auto &tagkv : tmpSk)
        {
            auto res1 = tagkScorePre.find(tagkv.first);
            if(dfValue.find(tagkv.first) == dfValue.end())
                cout << tagkv.first << endl;
            if(res1 == tagkScorePre.end())
                tagkScorePre.insert(make_pair(tagkv.first, dfValue.find(tagkv.first)->second));

            auto res2 = tagkFreq.find(tagkv.first);
            if(res2 == tagkFreq.end())
                tagkFreq.insert(make_pair(tagkv.first, 1u));
            else
                ++res2->second;

            auto res3 = tagkCardPre.find(tagkv.first);
            if(res3 == tagkCardPre.end())
            {
                unordered_set<string> tmpSet;
                tmpSet.insert(tagkv.second);
                tagkCardPre.insert(make_pair(tagkv.first, tmpSet));
            }
            else
                res3->second.insert(tagkv.second);
        }
    }
    vector<pair<string, unsigned>> tagkScore(tagkScorePre.begin(), tagkScorePre.end());
    sort(tagkScore.begin(), tagkScore.end(), [=](const pair<string, unsigned>& x, const pair<string, unsigned>& y){
        return x.second > y.second;
    });

    vector<pair<string, unsigned>> tagkFreqVec(tagkFreq.begin(), tagkFreq.end());
    sort(tagkFreqVec.begin(), tagkFreqVec.end(), [=](const pair<string, unsigned>& x, const pair<string, unsigned>& y){
        return x.second > y.second;
    });

    unordered_map<string, unsigned> tagkFreqRank;
    for(unsigned i = 0; i < tagkFreqVec.size(); ++i)
        tagkFreqRank.insert(make_pair(tagkFreqVec[i].first, i + 1));

    vector<pair<string, unsigned>> tagkCardVec;
    tagkCardVec.reserve(tagkCardPre.size());
    for(auto &i : tagkCardPre)
        tagkCardVec.emplace_back(i.first, i.second.size());
    sort(tagkCardVec.begin(), tagkCardVec.end(), [=](const pair<string, unsigned>& x, const pair<string, unsigned>& y){
        return x.second < y.second;
    });

    unordered_map<string, unsigned> tagkCard, tagkCardRank;
    for(unsigned i = 0; i < tagkCardVec.size(); ++i)
    {
        tagkCard.insert(tagkCardVec[i]);
        tagkCardRank.insert((make_pair(tagkCardVec[i].first, i)));
    }

    unsigned tagkCnt = tagkCardVec.size(), dmSize = metricFreq.find(metric)->second;

    for(auto &i : tagkScore)
    {
        if(!ONLY_TARGET_FEATURES || i.second >= tarTagkPos && highFreqMetric.find(metric) != highFreqMetric.end())
        features_file << metric << ' ' << i.first << ' ' << tagkCard.find(i.first)->second << ' ' <<
        tagkCardRank.find(i.first)->second << ' ' << (double)tagkCardRank.find(i.first)->second / tagkCnt <<
        ' ' << tagkFreq.find(i.first)->second << ' ' << tagkFreqRank.find(i.first)->second << ' ' <<
        (double)tagkFreqRank.find(i.first)->second / tagkCnt << ' ' << dmSize << ' ' << dfValue.find(i.first)->second <<
        ' ' << (highFreqMkInQuery.find(metric + i.first) != highFreqMkInQuery.end() ?
                highFreqMkInQuery.find(metric + i.first)->second : 0u) << endl;
        else if(i.second >= tarTagkPos)
        lowFreqMetricFeaturesFile << metric << ' ' << i.first << ' ' << tagkCard.find(i.first)->second << ' ' <<
        tagkCardRank.find(i.first)->second << ' ' << (double)tagkCardRank.find(i.first)->second / tagkCnt <<
        ' ' << tagkFreq.find(i.first)->second << ' ' << tagkFreqRank.find(i.first)->second << ' ' <<
        (double)tagkFreqRank.find(i.first)->second / tagkCnt << ' ' << dmSize << ' ' << dfValue.find(i.first)->second <<
        ' ' << (highFreqMkInQuery.find(metric + i.first) != highFreqMkInQuery.end() ?
                highFreqMkInQuery.find(metric + i.first)->second : 0u) << endl;
    }
    tmpSks.clear();
}

void readFile(const string& filename)
{
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
            if(lastMetric != "Siannodel")
            {
                //if(!ONLY_TARGET_FEATURES || highFreqMetric.find(metric) != highFreqMetric.end())
                extractFeatures(metric);
            }
            lastMetric = metric;
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
                tmpSk.emplace_back(make_pair(tagk_str, tagv_str));
                if(sk[i + 1] == '\0') break;
                now_tagk = true;
                j = 0;
                continue;
            }
            if(sk[i] == '\u001F' && sk[i - 1] != '\u001F')
            {now_tagk = false; tagk[j] = '\0'; j = 0; continue;}
            if(now_tagk)
                tagk[j++] = sk[i];
            else
                tagv[j++] = sk[i];
        }
        tmpSks.emplace_back(tmpSk);
    }
    //if(!ONLY_TARGET_FEATURES || highFreqMetric.find(metric) != highFreqMetric.end())
    extractFeatures(metric);
    infile.close();

    cout << "Finish reading file." << endl;
}

void extraFeatures(const string& filename)
{
    ifstream infile(filename);
    char sk[LEN], metric[LEN], tagk[LEN], tagv[LEN];
    string lastMetric = "Siannodel";
    unordered_map<string, unsigned> tagkFreq, metricCard;
    ofstream tagkFreqFile("tagkFreq"), metricCardFile("metricCard");
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
            if(lastMetric != "Siannodel")
            {
                metricCardFile << lastMetric << ' ' << metricCard.size() << endl;
                metricCardFile.clear();
            }
            lastMetric = metric;
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
                if(metricCard.find(tagk_str) == metricCard.end())
                    metricCard.insert(make_pair(tagk_str, 0));
                if(tagkFreq.find(tagk_str) == tagkFreq.end())
                    tagkFreq.insert(make_pair(tagk_str, 1u));
                else
                    ++tagkFreq.find(tagk_str)->second;
                tmpSk.emplace_back(make_pair(tagk_str, tagv_str));
                if(sk[i + 1] == '\0') break;
                now_tagk = true;
                j = 0;
                continue;
            }
            if(sk[i] == '\u001F' && sk[i - 1] != '\u001F')
            {now_tagk = false; tagk[j] = '\0'; j = 0; continue;}
            if(now_tagk)
                tagk[j++] = sk[i];
            else
                tagv[j++] = sk[i];
        }
        tmpSks.emplace_back(tmpSk);
    }
    metricCardFile << metric << ' ' << metricCard.size() << endl;
    for(auto &kf : tagkFreq)
        tagkFreqFile << kf.first << ' ' << kf.second << endl;
    metricCardFile.close();
    tagkFreqFile.close();
    infile.close();
}

void allTagsCnt(const string& filename)
{
    ifstream infile(filename);
    char sk[LEN], metric[LEN], tagk[LEN], tagv[LEN];
    string lastMetric = "Siannodel";
    unordered_map<string, unsigned> tags;
    unsigned tagsCnt = 0;
    while(infile.getline(sk, LEN - 1))
    {
        vector<pair<string, string>> tmpSk;
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
                if(tags.find(tagk_str) == tags.end())
                    {
                      tags.insert(make_pair(tagk_str, 0));
                      ++tagsCnt;
                    }
                
                if(sk[i + 1] == '\0') break;
                now_tagk = true;
                j = 0;
                continue;
            }
            if(sk[i] == '\u001F' && sk[i - 1] != '\u001F')
            {now_tagk = false; tagk[j] = '\0'; j = 0; continue;}
            if(now_tagk)
                tagk[j++] = sk[i];
            else
                tagv[j++] = sk[i];
        }
    }
    cout << tagsCnt << endl;
    infile.close();
}

int main()
{
    features_file.open(ONLY_TARGET_FEATURES ? "target_features" : "all_features");
    lowFreqMetricFeaturesFile.open("lowFreqMetricFeatures");
    string filename = "../newData/series-key-tpp-710";
    queryLogRead("tpp_710_mk", 95);
    pre(filename);
    df("metricTagk");
    readFile(filename);
//    extraFeatures(filename);
//    allTagsCnt(filename);
    return 0;
}
