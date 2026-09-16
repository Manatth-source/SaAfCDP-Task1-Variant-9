#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <cstdint>

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define NOMINMAX
#if defined(_WIN32)
#include <windows.h>
#endif


using Indices = std::pair<int, int>;

struct Interval
{
    int start, end;
};


std::vector<Interval> createRandomIntervals(int count, int minValue = 0, int maxValue = 100, int maxLength = 5)
{
    std::random_device seed;
    std::mt19937 gen(seed());

    std::uniform_int_distribution<int> startDis(minValue, maxValue);
    std::uniform_int_distribution<int> lengthDis(0, maxLength);

    std::vector<Interval> result;
    result.reserve(count);

    for (int i = 0; i < count; ++i) {
        const int start = startDis(gen);
        const int end = std::min(start + lengthDis(gen), maxValue);
        result.push_back({ start, end });
    }

    return result;
}


void printIntervals(const std::string& title, const std::vector<Interval>& intervals)
{
    std::cout << title << '\n';

    for (Interval interval : intervals)
        std::cout << '{' << interval.start << ", " << interval.end << "} ";

    std::cout << '\n';
}


bool intersect(const Interval& a, const Interval& b)
{
    return a.start <= b.end && a.end >= b.start;
}


void merge(Interval& a, Interval& b)
{
    a.start = std::min(a.start, b.start);
    a.end = std::max(a.end, b.end);
}


Indices isCorrect(const std::vector<Interval>& intervals)
{
    for (int i = 0; i < static_cast<int>(intervals.size()) - 1; ++i) {
        for (int j = i + 1; j < intervals.size(); ++j) {
            if (intersect(intervals[i], intervals[j])) return { i, j };
        }
    }

    return { -1, -1 };
}


//---------------------------------Slow---------------------------------
void slowMergeIntersectionsInterval(std::vector<Interval>& intervals)
{
    while (true) {
        Indices ind = isCorrect(intervals);

        if (ind == std::make_pair(-1, -1))
            return;

        merge(intervals[ind.first], intervals[ind.second]);
        intervals.erase(intervals.begin() + ind.second);
    }
}


//---------------------------------Fast---------------------------------
std::vector<Interval> fastMergeIntresectionsInterval(std::vector<Interval>& intervals)
{
    std::vector<Interval> result;

    if (intervals.empty()) {
        return result;
    }

    std::sort(intervals.begin(), intervals.end(),
        [](const Interval& a, const Interval& b) {
            return a.start < b.start;
        });

    Interval current = intervals[0];

    for (size_t i = 1; i < intervals.size(); ++i) {
        if (intersect(current, intervals[i])) {
            merge(current, intervals[i]);
        }
        else {
            result.push_back(current);
            current = intervals[i];
        }
    }

    result.push_back(current);

    return result;
}

//----------------------------------------------------------------------
//----------------------------- Замер -----------------------------------
//----------------------------------------------------------------------

uint64_t readTicks()
{
    return __rdtsc();
}

// volatile-переменная: компилятор обязан реально писать в неё,
// поэтому не может выкинуть вычисление результата как "неиспользуемое"
static volatile long long g_sink = 0;

void consume(const std::vector<Interval>& v)
{
    long long acc = 0;
    for (const auto& iv : v) acc += iv.start + iv.end;
    g_sink += acc;
}

uint64_t median(std::vector<uint64_t> samples)
{
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

void runBenchmark(int count, int minValue, int maxValue, int maxLength,
    int repeats)
{
    std::vector<Interval> source = createRandomIntervals(count, minValue, maxValue, maxLength);

    std::vector<uint64_t> fastTicks, slowTicks;
    fastTicks.reserve(repeats);
    slowTicks.reserve(repeats);

    for (int r = 0; r < repeats; ++r) {
        std::vector<Interval> tmp1 = source;

        uint64_t t1 = readTicks();
        std::vector<Interval> fastResult = fastMergeIntresectionsInterval(tmp1);
        uint64_t t2 = readTicks();
        consume(fastResult);
        fastTicks.push_back(t2 - t1);

        std::vector<Interval> tmp2 = source;

        uint64_t t3 = readTicks();
        slowMergeIntersectionsInterval(tmp2);
        uint64_t t4 = readTicks();
        consume(tmp2);
        slowTicks.push_back(t4 - t3);
    }

    std::cout << "count=" << count
        << "  fast median ticks=" << median(fastTicks)
        << "  slow median ticks=" << median(slowTicks) << '\n';
}

//----------------------------------------------------------------------

void runAllTests();

int main(void)
{
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    int count = 20, min = 1, length = 10, max = (length / 2) * count;

#if 0
    std::vector<Interval> intervals1 = createRandomIntervals(count, min, max, length);
    std::vector<Interval> intervals2 = intervals1;
    printIntervals("Initial", intervals1);

    std::cout << '\n';

    intervals1 = fastMergeIntresectionsInterval(intervals1);
    printIntervals("Fast result:", intervals1);

    std::cout << '\n';

    slowMergeIntersectionsInterval(intervals2);
    std::sort(intervals2.begin(), intervals2.end(),
        [](const Interval& a, const Interval& b) {
            return a.start < b.start;
        });
    printIntervals("Slow result:", intervals2);
#endif

    runAllTests();

    std::cout << "\n--- Замеры ---\n";
    runBenchmark(5, min, (length / 2) * 5, length, 10);
    runBenchmark(10, min, (length / 2) * 10, length, 10);
    runBenchmark(25, min, (length / 2) * 25, length, 10);
    runBenchmark(50, min, (length / 2) * 50, length, 10);
    runBenchmark(100, min, (length / 2) * 100, length, 10);
    runBenchmark(200, min, (length / 2) * 200, length, 10);
    runBenchmark(400, min, (length / 2) * 400, length, 10);
    runBenchmark(800, min, (length / 2) * 800, length, 10);
    runBenchmark(1600, min, (length / 2) * 1600, length, 10);


    //std::cout << "\nsink=" << g_sink << " (нужно только для честного замера)\n";

    return 0;
}