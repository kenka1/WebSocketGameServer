#include <thread>

#include <gtest/gtest.h>

#include "utils/ts_queue.hpp"

TEST(TSQueueTest, Empty)
{
  ep::TSQueue<int> data;
  EXPECT_EQ(data.Empty(), true);
}

TEST(TSQueueTest, ZeroSize)
{
  ep::TSQueue<int> data;
  EXPECT_EQ(data.Size(), 0);
}

TEST(TSQueueTest, Push)
{
  ep::TSQueue<int> data;
  EXPECT_EQ(data.Empty(), true);

  std::thread t([&data]{
    data.Push(1);
    data.Push(1);
    data.Push(1);
  });
  t.join();
  
  EXPECT_EQ(data.Size(), 3);
  EXPECT_EQ(data.Empty(), false);
}

TEST(TSQueueTest, TryPop)
{
  ep::TSQueue<int> data;

  auto value = data.TryPop();
  EXPECT_EQ(value, std::nullopt);

  data.Push(1);
  auto value2 = data.TryPop();
  EXPECT_EQ(value2, 1);
}

TEST(TSQueueTest, WaitAndPop)
{
  ep::TSQueue<int> data;
  data.Push(1);

  auto value = data.WaitAndPop();
  EXPECT_EQ(value, 1);
}
