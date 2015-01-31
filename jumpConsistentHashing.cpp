#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

#include "boost/program_options.hpp"
using namespace boost::program_options;

// From Google paper "A Fast, Minimal Memory, Consistent Hash Algorithm"
int32_t JumpConsistentHash(uint64_t key, int32_t num_buckets)
{
    int64_t b = 1, j = 0;
    while (j < num_buckets)
    {
        b = j;
        key = key * 2862933555777941757ULL + 1;
        j = (b + 1) * (double(1LL << 31) / double((key >> 33) + 1));
    }

    return b;
}

vector<string> readIds(const string& iFileName)
{
    vector<string> idVector;
    ifstream fileStream;
    fileStream.open(iFileName);

    if (fileStream)
    {
        string id;
        while (getline(fileStream, id))
        {
            idVector.push_back(id);
        }
    }
    else
    {
        cout << "Error while reading file " << iFileName << endl;
    }

    return idVector;
}

bool parseCommandLine(int argc, char **argv, std::string& ioFileName, int& ioDelta)
{
    bool areValidArguments = true;
    options_description optionDescription("Options");
    optionDescription.add_options()
        ("help,h", "help message")
        ("file", value<string>()->required(), "the file containing the ids to hash")
        ("delta,d", value<int>()->default_value(1), "the delta in bucket number");

    positional_options_description positionalOptions;
    positionalOptions.add("file", 1);
    variables_map variablesMap;
    try
    {
        store(command_line_parser(argc, argv).options(optionDescription)
            .positional(positionalOptions).run(), variablesMap);
        notify(variablesMap);
        ioFileName = variablesMap["file"].as<string>();
        ioDelta = variablesMap["delta"].as<int>();
    }
    catch (boost::program_options::error& exception)
    {
        if (!variablesMap.count("help"))
        {
            cout << exception.what() << endl;
        }
        cout << optionDescription << endl;
        areValidArguments = false;
    }

    return areValidArguments;
}

template <typename T>
void printMap(const T& iMap)
{
    for (const auto& pair : iMap)
    {
        cout << pair.first << ": " << pair.second << endl;
    }

    cout << endl;
}

template <typename T>
void printIdDistributions(const T& iOldDistribution, const T& iNewDistribution)
{
    cout << "Old distribution" << endl;
    printMap(iOldDistribution);
    cout << "New distribution" << endl;
    printMap(iNewDistribution);
}

void printMoveRatio(int iOccurrences, int iMovesNeeded, int iDelta)
{
    (iDelta >= 0) ? cout << "Adding " << iDelta : cout << "removing " << iDelta;
    cout << " buckets generates " << static_cast<double>(iMovesNeeded) / iOccurrences * 100 << " % of moves " << endl << endl;
}

template <typename Function>
void bucketing(const vector<string>& iIdVector, int iOriginalNumberOfBuckets, int iDelta, Function bucketizer)
{
    const int newNumberOfBuckets = iOriginalNumberOfBuckets + iDelta;
    int occurrences = iIdVector.size();
    int movesNeeded = 0;
    // Old mapping between the bucketId and the number of ids per bucket
    map<int32_t, uint64_t> oldDistribution;
    // Mapping between the id and the corresponding bucket
    map<string, int32_t> oldMapping;
    // New mapping between the bucketId and the number of ids per bucket
    map<int32_t, uint64_t> newDistribution;
    
    for (const auto& id : iIdVector)
    {
        int32_t oldBucket = bucketizer(id, iOriginalNumberOfBuckets);
        oldDistribution[oldBucket] += 1;
        oldMapping[id] = oldBucket;

        int32_t newBucket = bucketizer(id, newNumberOfBuckets);
        newDistribution[newBucket] += 1;

        if (newBucket != oldMapping[id])
        {
            ++movesNeeded;
        }
    }

    printIdDistributions(oldDistribution, newDistribution);
    printMoveRatio(occurrences, movesNeeded, iDelta);
}

int32_t consistentHashing(const std::string& iId, int iNumberOfBuckets)
{
    assert(iNumberOfBuckets > 0);
    std::hash<std::string> hashFunction;
    return JumpConsistentHash(hashFunction(iId), iNumberOfBuckets);
}

int32_t moduloArithmetic(const std::string& iId, int iNumberOfBuckets)
{
    assert(iNumberOfBuckets > 0);
    std::hash<std::string> hashFunction;
    return hashFunction(iId) % iNumberOfBuckets;
}

int main(int argc, char **argv)
{
    int delta;
    string fileName;

    if (!parseCommandLine(argc, argv, fileName, delta))
    {
        return 1;
    }

    auto idVector = readIds(fileName);
    if (!idVector.empty())
    {
        for (int buckets = 2; buckets < 10; ++buckets)
        {
            cout << "Case under analysis: from " << buckets << " to " << buckets + delta << " buckets" << endl;
            cout << "*** Consistent hashing ***" << endl;
            bucketing(idVector, buckets, delta, consistentHashing);

            cout << "*** Modulo Arithmetic ***" << endl;
            bucketing(idVector, buckets, delta, moduloArithmetic);
            cout << "End of case" << endl << endl;
        }
    }
}