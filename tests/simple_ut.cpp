#include <include/unrolled_list.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <list>

/*
    Ниже представлен ряд тестов, где используются (вместе, раздельно и по-очереди):
        - empty
        - clear
        - erase (single element)
        - erase (multiple elements)
        - insert (multiple elements)
    Методы применяются на unrolled_list и на std::list.
    Ожидается, что в итоге порядок элементов в контейнерах будут идентичен
*/

TEST( UnrrolledLinkedList, clear) {
    unrolled_list<char> unrolled_list;

    unrolled_list.push_back('a');
    unrolled_list.push_back('b');
    unrolled_list.push_back('c');
    unrolled_list.push_back('d');
    unrolled_list.push_back('e');

    unrolled_list.clear();

    ASSERT_TRUE(unrolled_list.empty()) << "Список не пуст после вызова clear";
}

TEST(UnrrolledLinkedList, eraseSingleElement) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    auto std_list_elem = std_list.begin();
    auto unrolled_list_elem = unrolled_list.begin();
    std::advance(std_list_elem, 777);
    std::advance(unrolled_list_elem, 777);

    std_list.erase(std_list_elem);
    unrolled_list.erase(unrolled_list_elem);

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list))
    << "Списки не равны после удаления одного элемента";
}

TEST (UnrrolledLinkedList, eraseMultipleElements ) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    auto std_list_start = std_list.begin();
    for (int i = 0; i < 400; ++i) {
        ++std_list_start;
    }
    auto std_list_end = std_list_start;
    for (int i = 0; i < 100; ++i) {
        ++std_list_end;
    }
    auto unrolled_list_start  = unrolled_list.begin();
    for (int i = 0; i < 400; ++i) {
        ++unrolled_list_start;
    }
    auto unrolled_list_end = unrolled_list_start;
    for (int i = 0; i < 100; ++i) {
        ++unrolled_list_end;
    }

    std_list.erase(std_list_start, std_list_end);
    unrolled_list.erase(unrolled_list_start, unrolled_list_end);

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list))
    << "Списки не равны после удаления нескольких элементов";
}

TEST(UnrrolledLinkedList, insertMultipleElements) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 10; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    auto std_list_elem = std_list.begin();
    auto unrolled_list_elem = unrolled_list.begin();
    std::advance(std_list_elem,5);
    std::advance(unrolled_list_elem,5);

    std_list.insert(std_list_elem, 5, 55);
    unrolled_list.insert(unrolled_list_elem, 5, 55);

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list))
    << "Списки не равны после вставки нескольких элементов";
}

/*
    Ниже представлен ряд тестов, где используются (вместе, раздельно и по-очереди):
        - size
        - push_back
        - pop_back
        - pop_front
        - push_front
        - insert (single element)
    Методы применяются на unrolled_list и на std::list.
    Ожидается, что в итоге порядок элементов в контейнерах будут идентичен
*/

TEST(UnrolledLinkedList, pushBack) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, pushFront) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_front(i);
        unrolled_list.push_front(i);
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, pushMixed) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 2 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, insertAndPushMixed) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 3 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else if (i % 3 == 1) {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        } else {
            auto std_it = std_list.begin();
            auto unrolled_it = unrolled_list.begin();
            std::advance(std_it, std_list.size() / 2);
            std::advance(unrolled_it, std_list.size() / 2);
            std_list.insert(std_it, i);
            unrolled_list.insert(unrolled_it, i);
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, popFrontBack) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    for (int i = 0; i < 500; ++i) {
        if (i % 2 == 0) {
            std_list.pop_back();
            unrolled_list.pop_back();
        } else {
            std_list.pop_front();
            unrolled_list.pop_front();
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    for (int i = 0; i < 500; ++i) {
        std_list.pop_back();
        unrolled_list.pop_back();
    }

    ASSERT_TRUE(unrolled_list.empty());
}
