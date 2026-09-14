#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <ctime>


#define ON 1
#define OFF 0


using Indices = std::pair<int, int>;

struct Interval 
{
	int start, end;
};


std::vector<Interval> createRandomIntervals(int count, int minValue = 0, int maxValue = 100) 
{
    std::vector<Interval> result;

#if OFF
    //SRAND
    std::random_device rd;
    std::mt19937 gen(rd());
#endif

#if ON
    //RAND
    std::mt19937 gen(std::time(nullptr));
#endif

    std::uniform_int_distribution<> dist(minValue, maxValue);

    for (int i = 0; i < count; i++) {
        int start = dist(gen);
        int end = dist(gen);

        if (start > end)
            std::swap(start, end);

        result.push_back({ start, end });
    }

    return result;
}


void printIntervals(const std::vector<Interval>& intervals)
{
    std::cout << "Intervals:\n";

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


int main(void)
{
    int count = 3, min = 1, max = 100;

    std::cout << "Fast\n";
    std::vector<Interval> intervals1 = createRandomIntervals(count, min, max);
    printIntervals(intervals1);

    intervals1 = fastMergeIntresectionsInterval(intervals1);
    printIntervals(intervals1);

    std::cout << '\n';

    std::cout << "Slow\n";
    std::vector<Interval> intervals2 = createRandomIntervals(count, min, max);
    printIntervals(intervals2);

    slowMergeIntersectionsInterval(intervals2);
    printIntervals(intervals2);
}