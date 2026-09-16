#include <iostream>
#include <vector>
#include <algorithm>


// Эти объявления не создают новых сущностей — они лишь говорят компилятору
// "такие struct/функции существуют где-то в проекте с такой сигнатурой".
// Реальные определения (тела функций, поля структуры) берутся из main.cpp
// на этапе линковки. Порядок полей и сигнатуры должны совпадать один в один.
using Indices = std::pair<int, int>;

struct Interval
{
    int start, end;
};

std::vector<Interval> createRandomIntervals(int count, int minValue = 0, int maxValue = 100, int maxLength = 5);
bool intersect(const Interval& a, const Interval& b);
void merge(Interval& a, Interval& b);
std::vector<Interval> fastMergeIntresectionsInterval(std::vector<Interval>& intervals);
void slowMergeIntersectionsInterval(std::vector<Interval>& intervals);

//----------------------------------------------------------------------
//----------------------------- Тесты -----------------------------------
//----------------------------------------------------------------------

static int g_testsRun = 0;
static int g_testsFailed = 0;

#define CHECK(condition) \
    do { \
        ++g_testsRun; \
        if (!(condition)) { \
            ++g_testsFailed; \
            std::cout << "  [FAIL] " << __func__ << ": " << #condition \
                      << " (строка " << __LINE__ << ")\n"; \
        } \
    } while (false)

bool isValidMergedResult(const std::vector<Interval>& result)
{
    for (size_t i = 0; i + 1 < result.size(); ++i) {
        if (result[i].start > result[i + 1].start) return false;
        if (intersect(result[i], result[i + 1])) return false;
    }
    return true;
}

void test_EmptyInput()
{
    std::vector<Interval> empty;
    auto result = fastMergeIntresectionsInterval(empty);
    CHECK(result.empty());
}

void test_SingleInterval()
{
    std::vector<Interval> input = { {1, 5} };
    auto result = fastMergeIntresectionsInterval(input);
    CHECK(result.size() == 1);
    CHECK(result[0].start == 1);
    CHECK(result[0].end == 5);
}

void test_NonIntersectingIntervalsStaySeparate()
{
    std::vector<Interval> input = { {1, 2}, {5, 6}, {10, 11} };
    auto result = fastMergeIntresectionsInterval(input);
    CHECK(result.size() == 3);
    CHECK(isValidMergedResult(result));
}

void test_NestedIntervalIsAbsorbed()
{
    std::vector<Interval> input = { {1, 10}, {3, 4} };
    auto result = fastMergeIntresectionsInterval(input);
    CHECK(result.size() == 1);
    CHECK(result[0].start == 1);
    CHECK(result[0].end == 10);
}

void test_TouchingEndsAreMerged()
{
    std::vector<Interval> input = { {1, 2}, {2, 3} };
    auto result = fastMergeIntresectionsInterval(input);
    CHECK(result.size() == 1);
    CHECK(result[0].start == 1);
    CHECK(result[0].end == 3);
}

void test_AllIntervalsMergeIntoOne()
{
    std::vector<Interval> input = { {1, 3}, {2, 5}, {4, 8}, {7, 10} };
    auto result = fastMergeIntresectionsInterval(input);
    CHECK(result.size() == 1);
    CHECK(result[0].start == 1);
    CHECK(result[0].end == 10);
}

void test_FastAndSlowAgreeOnRandomData()
{
    for (int trial = 0; trial < 20; ++trial) {
        std::vector<Interval> input = createRandomIntervals(30, 0, 100, 10);

        std::vector<Interval> forFast = input;
        auto fastResult = fastMergeIntresectionsInterval(forFast);

        std::vector<Interval> forSlow = input;
        slowMergeIntersectionsInterval(forSlow);
        std::sort(forSlow.begin(), forSlow.end(),
            [](const Interval& a, const Interval& b) { return a.start < b.start; });

        CHECK(fastResult.size() == forSlow.size());
        if (fastResult.size() == forSlow.size()) {
            for (size_t i = 0; i < fastResult.size(); ++i) {
                CHECK(fastResult[i].start == forSlow[i].start);
                CHECK(fastResult[i].end == forSlow[i].end);
            }
        }
        CHECK(isValidMergedResult(fastResult));
    }
}

void runAllTests()
{
    std::cout << "\n--- Тесты ---\n";

    test_EmptyInput();
    test_SingleInterval();
    test_NonIntersectingIntervalsStaySeparate();
    test_NestedIntervalIsAbsorbed();
    test_TouchingEndsAreMerged();
    test_AllIntervalsMergeIntoOne();
    test_FastAndSlowAgreeOnRandomData();

    std::cout << g_testsRun - g_testsFailed << " / " << g_testsRun << " проверок пройдено\n";
    if (g_testsFailed == 0)
        std::cout << "Все тесты пройдены.\n";
    else
        std::cout << g_testsFailed << " проверок упало.\n";
}