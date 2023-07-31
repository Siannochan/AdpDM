#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <chrono>
using namespace std;

const int LEN = 1e4;
const unsigned int BLOCK_SIZE = 1000;


void postingGen(const string& filename)
{
    ifstream infile(filename);
    char sk[LEN], metric[LEN], tagk[LEN], tagv[LEN];
    unordered_map<string, vector<unsigned>> posting;
    vector<pair<string, pair<unsigned, unsigned>>> metricRange;
    unsigned skCnt = 0;
    string lastMetric = "Siannodel";
    ofstream blockFile("serieskey/block0");
    while(infile.getline(sk, LEN - 1))
    {
        if(skCnt && skCnt / BLOCK_SIZE != (skCnt - 1) / BLOCK_SIZE)
        {
            blockFile.close();
            blockFile.open("serieskey/block" + to_string(skCnt / BLOCK_SIZE));
            cout << skCnt / BLOCK_SIZE << endl;
        }
        blockFile << sk << endl;
        ++skCnt;
        unsigned i;
        // read the metric part.
        for(i = 1; sk[i] != '\u001E'; ++i)
            metric[i - 1] = sk[i];
        metric[i - 1] = '\0';
        if(metric != lastMetric)
        {
            if(lastMetric != "Siannodel")
                metricRange.emplace_back(make_pair(lastMetric, make_pair(
                    (metricRange.size() == 0 ? 1 : metricRange[metricRange.size() - 1].second.second + 1),
                    skCnt - 1)));
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
                auto res = posting.find(tagk_str + "\u001F" + tagv_str);
                if(res == posting.end())
                {
                    vector<unsigned> vec;
                    vec.emplace_back(skCnt);
                    posting.insert(make_pair(tagk_str + "\u001F" + tagv_str, vec));
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
    blockFile.close();

    // posting part.
    vector<pair<string, unsigned long long>> tagkv2offset;
    ofstream outfile("originPosting", ios::out | ios::binary);
    for(auto &i : posting)
    {
        tagkv2offset.emplace_back(make_pair(i.first, outfile.tellp()));
        unsigned siz = i.second.size();
        outfile.write(reinterpret_cast<char const*>(&siz), sizeof(unsigned));
        for(auto &j : i.second)
            outfile.write(reinterpret_cast<char const*>(&j), sizeof(unsigned));
    }
    outfile.close();

    // tagkv map part.
    outfile.open("tagkvMp");
    for(auto &i : tagkv2offset)
        outfile << i.first << endl << i.second << endl;
    outfile.close();

    // metric range part.
    outfile.open("metricInfo");
    for(auto &i : metricRange)
        outfile << i.first << ' ' << i.second.first << ' ' << i.second.second << endl;
    outfile.close();


//    // statistics for Table4.
//    unsigned long long postingLengthSum = 0;
//    vector<pair<string, vector<unsigned>>> postingVec(posting.begin(), posting.end());
//    for(auto &i: postingVec)
//        postingLengthSum += i.second.size();
//    cout << "avg len: " << postingLengthSum / postingVec.size() << endl;
//    cout << "term cnt: " << postingVec.size() << endl;
}

//char sk[(int)2e7][LEN];
//
//void throughputTest(const string& filename)
//{
//    ifstream infile(filename);
//    unsigned iter = 0;
//    while(infile.getline(sk[iter], LEN - 1)) ++iter;
//    infile.close();
//
//    auto start = chrono::system_clock::now();
//
//    char metric[LEN], tagk[LEN], tagv[LEN];
//    unordered_map<string, vector<unsigned>> posting;
//    vector<pair<string, pair<unsigned, unsigned>>> metricRange;
//    unsigned skCnt = 0;
//    string lastMetric = "Siannodel";
//    for(int skNow = 0; skNow < iter; ++skNow)
//    {
//        ++skCnt;
//        unsigned i;
//        // read the metric part.
//        for(i = 1; sk[skNow][i] != '\u001E'; ++i)
//            metric[i - 1] = sk[skNow][i];
//        metric[i - 1] = '\0';
//        if(metric != lastMetric)
//        {
//            if(lastMetric != "Siannodel")
//                metricRange.emplace_back(make_pair(lastMetric, make_pair(
//                        (metricRange.size() == 0 ? 1 : metricRange[metricRange.size() - 1].second.second + 1),
//                        skCnt - 1)));
//            lastMetric = metric;
//        }
//        // read the tagkv part.
//        bool now_tagk = true;
//        unsigned j = 0;
//        for(++i;; ++i)
//        {
//            if(sk[skNow][i] == '\u001D' || sk[skNow][i + 1] == '\0')
//            {
//                tagv[j] = '\0';
//                string tagk_str = tagk, tagv_str = tagv;
//                auto res = posting.find(tagk_str + "\u001F" + tagv_str);
//                if(res == posting.end())
//                {
//                    vector<unsigned> vec;
//                    vec.emplace_back(skCnt);
//                    posting.insert(make_pair(tagk_str + "\u001F" + tagv_str, vec));
//                }
//                else
//                    res->second.emplace_back(skCnt);
//
//                if(sk[skNow][i + 1] == '\0') break;
//                now_tagk = true;
//                j = 0;
//                continue;
//            }
//            if(sk[skNow][i] == '\u001F')
//            {now_tagk = false; tagk[j] = '\0'; j = 0; continue;}
//            if(now_tagk)
//                tagk[j++] = sk[skNow][i];
//            else
//                tagv[j++] = sk[skNow][i];
//        }
//    }
//    auto end = chrono::system_clock::now();
//    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
//    cout << "Index construct time cost: " << ((double)(duration.count()) *
//                              chrono::microseconds::period::num / chrono::microseconds::period::den) << endl;
//
//    start = chrono::system_clock::now();
//
//    string skStr;
//    for(int i = 0; i < iter; ++i) skStr = sk[i];
//
//    end = chrono::system_clock::now();
//    duration = chrono::duration_cast<chrono::microseconds>(end - start);
//    cout << "Pure write time cost: " << ((double)(duration.count()) *
//                                              chrono::microseconds::period::num / chrono::microseconds::period::den) << endl;
//
//    cout << "Sk count: " << iter << endl;
//}

int main()
{
    auto start = chrono::system_clock::now();
    postingGen("../newData/series-key-asi-1340");
    auto end = chrono::system_clock::now();
    cout << ((double)((chrono::duration_cast<chrono::microseconds>(end - start)).count()) *
                              chrono::microseconds::period::num) << endl;
    //throughputTest("../newData/series-key-10047");
    return 0;
}
