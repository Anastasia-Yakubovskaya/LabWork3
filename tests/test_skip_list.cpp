#include <gtest/gtest.h>
#include "skip_list.h"

#include <vector>
#include <algorithm>
#include <string>

class SkipListTest : public ::testing::Test {
protected:
    void SetUp() override {
        sl.insert(5);
        sl.insert(3);
        sl.insert(7);
    }

    stl::skip_list<int> sl;
};

TEST_F(SkipListTest, InitialState) {
    EXPECT_FALSE(sl.empty());
    EXPECT_EQ(sl.size(), 3);
}

TEST_F(SkipListTest, InsertOperations) {
    auto res = sl.insert(2);
    EXPECT_TRUE(res.second);
    EXPECT_EQ(*res.first, 2);
    EXPECT_EQ(sl.size(), 4);

    res = sl.insert(5);
    EXPECT_FALSE(res.second);
    EXPECT_EQ(*res.first, 5);
    EXPECT_EQ(sl.size(), 4);
}

TEST_F(SkipListTest, Iteration) {
    std::vector<int> expected = {3, 5, 7};
    std::vector<int> actual(sl.begin(), sl.end());
    EXPECT_EQ(expected, actual);
}

TEST_F(SkipListTest, FindOperations) {
    auto it = sl.find(3);
    EXPECT_NE(it, sl.end());
    EXPECT_EQ(*it, 3);

    it = sl.find(10);
    EXPECT_EQ(it, sl.end());
}

TEST_F(SkipListTest, EraseOperations) {
    auto it = sl.find(5);
    it = sl.erase(it);
    EXPECT_EQ(sl.size(), 2);
    EXPECT_EQ(*it, 7);

    EXPECT_EQ(sl.erase(3), 1);
    EXPECT_EQ(sl.size(), 1);

    EXPECT_EQ(sl.erase(10), 0);
    EXPECT_EQ(sl.size(), 1);
}

TEST_F(SkipListTest, CopyConstructor) {
    stl::skip_list<int> sl2(sl);
    EXPECT_EQ(sl.size(), 3);
    EXPECT_EQ(sl2.size(), 3);

    std::vector<int> v1(sl.begin(), sl.end());
    std::vector<int> v2(sl2.begin(), sl2.end());
    EXPECT_EQ(v1, v2);
}

TEST_F(SkipListTest, MoveConstructor) {
    stl::skip_list<int> sl2(std::move(sl));
    EXPECT_TRUE(sl.empty());
    EXPECT_EQ(sl2.size(), 3);

    std::vector<int> expected = {3, 5, 7};
    std::vector<int> actual(sl2.begin(), sl2.end());
    EXPECT_EQ(expected, actual);
}

TEST_F(SkipListTest, AssignmentOperator) {
    stl::skip_list<int> sl2;
    sl2 = sl;
    EXPECT_EQ(sl.size(), 3);
    EXPECT_EQ(sl2.size(), 3);

    std::vector<int> v1(sl.begin(), sl.end());
    std::vector<int> v2(sl2.begin(), sl2.end());
    EXPECT_EQ(v1, v2);
}

TEST_F(SkipListTest, MoveAssignment) {
    stl::skip_list<int> sl2;
    sl2 = std::move(sl);
    EXPECT_TRUE(sl.empty());
    EXPECT_EQ(sl2.size(), 3);

    std::vector<int> expected = {3, 5, 7};
    std::vector<int> actual(sl2.begin(), sl2.end());
    EXPECT_EQ(expected, actual);
}

TEST_F(SkipListTest, InitializerList) {
    stl::skip_list<int> sl2 = {5, 3, 7, 2, 8};
    EXPECT_EQ(sl2.size(), 5);

    std::vector<int> expected = {2, 3, 5, 7, 8};
    std::vector<int> actual(sl2.begin(), sl2.end());
    EXPECT_EQ(expected, actual);
}

TEST_F(SkipListTest, RangeConstructor) {
    std::vector<int> input = {5, 3, 7, 2, 8};
    stl::skip_list<int> sl2(input.begin(), input.end());
    EXPECT_EQ(sl2.size(), 5);

    std::sort(input.begin(), input.end());
    std::vector<int> actual(sl2.begin(), sl2.end());
    EXPECT_EQ(input, actual);
}

TEST(SkipListStringTest, StringOperations) {
    stl::skip_list<std::string> sl;
    sl.insert("apple");
    sl.insert("banana");
    sl.insert("cherry");

    EXPECT_EQ(sl.size(), 3);

    auto it = sl.find("banana");
    EXPECT_NE(it, sl.end());
    EXPECT_EQ(*it, "banana");

    it = sl.erase(it);
    EXPECT_EQ(sl.size(), 2);
    EXPECT_EQ(*it, "cherry");
}

TEST(SkipListAdvancedTest, EqualRange) {
    stl::skip_list<std::string> sl = {"a", "b", "b", "c"};
    
    auto range = sl.equal_range("b");
    EXPECT_NE(range.first, sl.end());
    EXPECT_EQ(*range.first, "b");
    EXPECT_EQ(++range.first, range.second);
}

TEST(SkipListAdvancedTest, LowerUpperBound) {
    stl::skip_list<int> sl = {10, 20, 30, 40, 50};
    
    auto lb = sl.lower_bound(25);
    EXPECT_NE(lb, sl.end());
    EXPECT_EQ(*lb, 30);
    
    auto ub = sl.upper_bound(30);
    EXPECT_NE(ub, sl.end());
    EXPECT_EQ(*ub, 40);
}

TEST(SkipListAdvancedTest, ClearOperation) {
    stl::skip_list<std::string> sl = {"a", "b", "c"};
    
    EXPECT_EQ(sl.size(), 3);
    sl.clear();
    EXPECT_TRUE(sl.empty());
    EXPECT_EQ(sl.size(), 0);
    EXPECT_EQ(sl.begin(), sl.end());
}

TEST(SkipListAdvancedTest, SwapOperation) {
    stl::skip_list<int> sl1 = {1, 3, 5};
    stl::skip_list<int> sl2 = {2, 4, 6};
    
    sl1.swap(sl2);
    
    std::vector<int> v1(sl1.begin(), sl1.end());
    std::vector<int> expected1 = {2, 4, 6};
    EXPECT_EQ(v1, expected1);
    
    std::vector<int> v2(sl2.begin(), sl2.end());
    std::vector<int> expected2 = {1, 3, 5};
    EXPECT_EQ(v2, expected2);
}

TEST(SkipListCustomComparatorTest, DescendingOrder) {
    auto cmp = [](int a, int b) { return a > b; };
    stl::skip_list<int, decltype(cmp)> sl(cmp);
    
    sl.insert(5);
    sl.insert(3);
    sl.insert(7);
    sl.insert(1);
    sl.insert(9);
    
    std::vector<int> expected = {9, 7, 5, 3, 1};
    std::vector<int> actual(sl.begin(), sl.end());
    EXPECT_EQ(expected, actual);
}

TEST(SkipListCustomComparatorTest, FindWithCustomComparator) {
    auto cmp = [](int a, int b) { return a > b; };
    stl::skip_list<int, decltype(cmp)> sl(cmp);
    sl.insert(5);
    sl.insert(3);
    sl.insert(7);
    
    auto it = sl.find(5);
    EXPECT_NE(it, sl.end());
    EXPECT_EQ(*it, 5);
    
    it = sl.find(2);
    EXPECT_EQ(it, sl.end());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
