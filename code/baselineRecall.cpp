#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <chrono>
using namespace std;

const unsigned int BLOCK_SIZE = 1000;

unordered_map<string, long long> tagkv2offset;
unordered_map<string, pair<unsigned, unsigned>> metricRange;
vector<string> queryTagkv;
string queryMetric;
unsigned queryCount;

chrono::microseconds forwardIndexDuration;
chrono::microseconds invertedIndexDuration;
chrono::microseconds IODuration;
chrono::microseconds filterDuration;

char sks[BLOCK_SIZE + 1][(int)1e5 + 5];

void recallProcess()
{
    // <posting number, flag> flag: 1 for alive, 0 for unknown, -1 for dead.
    vector<pair<unsigned, int>> originalPostings;
    ifstream postingFile("originPosting",ios::in|ios::binary);

    auto metricRes = metricRange.find(queryMetric);
    if(metricRes == metricRange.end()) return;
    unsigned l, r;
    l = metricRes->second.first;
    r = metricRes->second.second;
    for(int i = 0; i < queryTagkv.size(); ++i)
    {
        auto start = chrono::system_clock::now(); //forward index duration start.
        auto res = tagkv2offset.find(queryTagkv[i]);
        if(res == tagkv2offset.end()) return;
        auto end = chrono::system_clock::now(); // forward index duration end.
        forwardIndexDuration += chrono::duration_cast<chrono::microseconds>(end - start);

        start = chrono::system_clock::now(); // invert index duration start.
        postingFile.seekg(res->second);
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
                while(pos < originalPostings.size() - 1 && originalPostings[pos].first < num)
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
    postingFile.close();

    auto start = chrono::system_clock::now(); // invert index duration start.
    vector<unsigned> postingIntersection;
    for(auto &k : originalPostings)
        if(k.second == 1) postingIntersection.emplace_back(k.first);


    auto end = chrono::system_clock::now(); // invert index duration end.
    invertedIndexDuration += chrono::duration_cast<chrono::microseconds>(end - start);

    ofstream outfile("QueryResult/query_result_" + to_string(queryCount));
    for(int i = 0; i < postingIntersection.size(); ++i)
    {
        // posting number start from 1.
        // block id start from 0.
        ifstream infile("serieskey/block" + to_string((postingIntersection[i] - 1) / BLOCK_SIZE));

        start = chrono::system_clock::now(); // IO duration start.

        unsigned iter = 0;
        while(infile.getline(sks[iter++], (int)1e5)){}
        end = chrono::system_clock::now(); // IO duration end.
        IODuration += chrono::duration_cast<chrono::microseconds>(end - start);

        start = chrono::system_clock::now(); // filter duration start.
        iter = -1;
        unsigned cnt = postingIntersection[i] % BLOCK_SIZE;
        //char sk[(int)1e5 + 5];
        while(cnt--) ++iter; // infile.getline(sk, (int)1e5);   the last sk read is the one we need.
        outfile << sks[iter] << endl;
        while(i + 1 < postingIntersection.size() &&
              (postingIntersection[i + 1] - 1) / BLOCK_SIZE == (postingIntersection[i] - 1) / BLOCK_SIZE)
        {
            cnt = postingIntersection[i + 1] - postingIntersection[i];
            while(cnt--) ++iter; //infile.getline(sk, (int)1e5); the last sk read is the one we need.
            outfile << sks[iter] << endl;
            ++i;
        }
        end = chrono::system_clock::now(); // filter duration end.
        filterDuration += chrono::duration_cast<chrono::microseconds>(end - start);

        infile.close();
    }

    outfile.close();
}

void readInfo()
{
    ifstream infile("tagkvMp");
    char tagkv[10005];
    unsigned long long offset;
    int cnt = 0;
    while(infile.getline(tagkv, 10000))
    {
        infile >> offset;
        tagkv2offset.insert(make_pair(tagkv, offset));
        infile.getline(tagkv, 10000);
    }
    infile.close();

    infile.open("metricInfo");
    string metric;
    unsigned l, r;
    while(infile >> metric)
    {
        infile >> l >> r;
        metricRange.insert(make_pair(metric, make_pair(l, r)));
    }
    infile.close();
}

void readQuery(const string& filename)
{

    unsigned tagsCount;
    ifstream infile(filename);
    string tagk, tagv;
    while(infile >> queryMetric)
    {
        infile >> tagsCount;
        queryTagkv.clear();
        while(tagsCount--)
        {
            infile >> tagk >> tagv;
            queryTagkv.emplace_back(tagk + "\u001F" + tagv);
        }
        ++queryCount;
        recallProcess();
    }
    infile.close();
}

int main()
{
    readInfo();
    readQuery("../Recall/Queries"); // Edit this to address query file.

    cout << "Forward index time cost: " << ((double)(forwardIndexDuration.count()) *
                              chrono::microseconds::period::num) << endl;
    cout << "Inverted index time cost: " << ((double)(invertedIndexDuration.count()) *
                              chrono::microseconds::period::num) << endl;
    cout << "IO time cost: " << ((double)(IODuration.count()) *
                              chrono::microseconds::period::num) << endl;
    cout << "Filter time cost: " << ((double)(filterDuration.count()) *
                              chrono::microseconds::period::num) << endl;
    return 0;
}
