
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "../MemPool.hpp"
#include <vector>
#include <string>

TEST_CASE("MemPool integrity")
{

   MemPool<int> pool;
   pool.CreatePool(10);

   int *t1 = pool.Allocate();
   *t1 = 1;
   int *t2 = pool.Allocate();
   *t2 = 2;
   int *t3 = pool.Allocate();
   *t3 = 3;
   int *t4 = pool.Allocate();
   *t4 = 4;
   int *t5 = pool.Allocate();
   *t5 = 5;

   std::vector< std::pair<int*, int> > vec;
   vec.push_back(std::make_pair(t1, 1));
   vec.push_back(std::make_pair(t2, 2));
   vec.push_back(std::make_pair(t3, 3));
   vec.push_back(std::make_pair(t4, 4));
   vec.push_back(std::make_pair(t5, 5));

   auto check = [=](auto vec)
   {
      for (auto i : vec)
      {
         CHECK(*i.first == i.second);
      }
   };

   check(vec);

   pool.Deallocate(t3);
   vec.erase(vec.begin() + 2);
   check(vec);
   int *t6 = pool.Allocate();
   *t6 = 6;
   vec.push_back(std::make_pair(t6, 6));
   check(vec);
   pool.Deallocate(t1);
   vec.erase(vec.begin());
   check(vec);

   int *t = pool.Allocate();
   t = pool.Allocate();
   t = pool.Allocate();
   t = pool.Allocate();

   check(vec);

   int *t7 = pool.Allocate();
   *t7 = 7;
   vec.push_back(std::make_pair(t7, 7));

   pool.Deallocate(t4);
   vec.erase(vec.begin() + 1);

   check(vec);
}

TEST_CASE("MemPool many")
{
   MemPool<int> pool;
   pool.CreatePool(10000);
   int *l = nullptr;
   for (int i = 0; i < 10000; i++)
   {
      auto *p = pool.Allocate();
      l = p;
   }
   pool.Deallocate(l);
   auto *pp = pool.Allocate();
   CHECK(pp != nullptr);
}


TEST_CASE("MemPool bookkeeping")
{

   MemPool<int> pool;
   pool.CreatePool(5);

   //test internal bookkeeping
   int *t1 = pool.Allocate();
   CHECK(t1 != nullptr);
   uint32_t* p = reinterpret_cast<uint32_t*>(t1);
   //next available block at pos 1
   CHECK(*p == 1);
   int *t2 = pool.Allocate();
   p = reinterpret_cast<uint32_t*>(t2);
   CHECK(*p == 2);
   int *t3 = pool.Allocate();
   p = reinterpret_cast<uint32_t*>(t3);
   CHECK(*p == 3);
   int *t4 = pool.Allocate();
   p = reinterpret_cast<uint32_t*>(t4);
   CHECK(*p == 4);
   int *t5 = pool.Allocate();
   p = reinterpret_cast<uint32_t*>(t5);
   CHECK(*p == 5);

   //pool full
   int *t6 = pool.Allocate();
   CHECK(t6 == nullptr);

   pool.Deallocate(t5);

   p = reinterpret_cast<uint32_t*>(t5);
   CHECK(*p == 5);

   pool.Deallocate(t4);
   p = reinterpret_cast<uint32_t*>(t4);
   CHECK(*p == 4);
   pool.Deallocate(t3);
   pool.Deallocate(t2);
   pool.Deallocate(t1);
   p = reinterpret_cast<uint32_t*>(t1);
   CHECK(*p == 1);

}
template <size_t N>
static void scopy(char(&output)[N], const char* src)
{
   strncpy_s(output, src, N );
}


TEST_CASE("MemPool struct test")
{

   struct Node
   {
      char one[3];
      short t;
      int y;
      char buff[6];
   };
   MemPool<Node> pool;
   pool.CreatePool(3);

   auto *t1 = pool.Allocate();
   memset(t1, 0, sizeof(*t1)); 
   scopy(t1->buff, "test1");

   auto *t2 = pool.Allocate();
   memset(t2, 0, sizeof(*t2));
   scopy(t2->buff, "test2");
 
   auto *t3 = pool.Allocate();
   memset(t3, 0, sizeof(*t3));
   scopy(t3->buff, "test3");

   auto check = [=](auto n, std::string c)
   {
      std::string s = n -> buff;
      CHECK(s == c);
   };

   check(t1, "test1");
   check(t2, "test2");
   check(t3, "test3");

   pool.Deallocate(t3);

   auto *t4 = pool.Allocate();
   memset(t4, 0, sizeof(*t4));
   scopy(t4->buff, "test4");

   check(t1, "test1");
   check(t2, "test2");
   check(t4, "test4");

   scopy(t1->buff, "tet1x");
   scopy(t4->buff, "tes1y");

   pool.Deallocate(t2);

   check(t1, "tet1x");
   check(t4, "tes1y");

   pool.Deallocate(t1);

   auto *t5 = pool.Allocate();
   CHECK(t5 != nullptr);
   memset(t5, 0, sizeof(*t5));
   scopy(t5->buff, "xx");

   check(t4, "tes1y");
   check(t5, "xx");

}
