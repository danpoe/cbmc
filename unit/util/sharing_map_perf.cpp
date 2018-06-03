#include <random>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <set>
#include <cassert>
#include <cstring>

#include <util/threeval.h>
#include <util/sharing_map.h>

#define NUM_WIDTH 16

enum class Map { MAP, UNORDERED_MAP, SHARING_MAP };
enum class Test { MEMORY, INSERT_RUNTIME, FIND_RUNTIME };

// Config
size_t num_ops = 10000000;
size_t num_ran = 100000000;
Map m = Map::MAP;
Test t = Test::MEMORY;
bool allow_repeats = true;

typedef sharing_mapt<unsigned, unsigned> smt;
typedef std::map<unsigned, unsigned> mapt;
typedef std::unordered_map<unsigned, unsigned> unorderedt;

void runtime_insert()
{
  smt sm;
  mapt map;
  unorderedt unordered;

  std::set<unsigned> seen;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<unsigned> distribution(0, num_ran - 1);
  auto random_num = std::bind(distribution, generator);

  using clock = std::chrono::system_clock;
  using std::chrono::time_point;
  using std::chrono::duration_cast;
  using std::chrono::nanoseconds;

  std::size_t dur = 0;

  for(size_t i = 0; i < num_ops; i++)
  {
    time_point<clock> before;
    time_point<clock> after;

    unsigned n = 0;

    if(allow_repeats)
    {
      n = random_num();
    }
    else
    {
      n = random_num();
      auto r = seen.insert(n);

      while(!r.second)
      {
        n = random_num();
        r = seen.insert(n);
      }
    }

    if(m == Map::MAP)
    {
      assert(allow_repeats || map.find(n) == map.end());
      before = clock::now();
      map.insert(std::make_pair(n, 0));
      after = clock::now();
    }
    else if(m == Map::UNORDERED_MAP)
    {
      assert(allow_repeats || unordered.find(n) == unordered.end());
      before = clock::now();
      unordered.insert(std::make_pair(n, 0));
      after = clock::now();
    }
    else if(m == Map::SHARING_MAP)
    {
      if(allow_repeats)
      {
        before = clock::now();
        sm.insert(n, 0);
        after = clock::now();
      }
      else
      {
        assert(!sm.has_key(n));
        before = clock::now();
        sm.insert(n, 0, tvt(false));
        after = clock::now();
      }
    }

    auto diff = after - before;
    auto duration = duration_cast<nanoseconds>(diff);
    auto add = duration.count();

    assert(add > 0);

    dur += add;
  }

  std::cout << "Insert time (ns): " << std::setw(NUM_WIDTH) << dur << std::endl;
}

void runtime_find()
{
  smt sm;
  mapt map;
  unorderedt unordered;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<unsigned> distribution(0, num_ran - 1);
  auto random_num = std::bind(distribution, generator);

  using clock = std::chrono::system_clock;
  using std::chrono::time_point;
  using std::chrono::duration_cast;
  using std::chrono::nanoseconds;

  // Fill maps
  for(size_t i = 0; i < num_ops; i++)
  {
    unsigned n = random_num();

    if(m == Map::MAP)
    {
      map.insert(std::make_pair(n, 0));
    }
    else if(m == Map::UNORDERED_MAP)
    {
      unordered.insert(std::make_pair(n, 0));
    }
    else if(m == Map::SHARING_MAP)
    {
      sm.insert(n, 0);
    }
  }

  std::size_t dur = 0;

  // Find elements
  for(size_t i = 0; i < num_ops; i++)
  {
    time_point<clock> before;
    time_point<clock> after;

    unsigned n = random_num();

    if(m == Map::MAP)
    {
      before = clock::now();
      map.find(n);
      after = clock::now();
    }
    else if(m == Map::UNORDERED_MAP)
    {
      before = clock::now();
      unordered.find(n);
      after = clock::now();
    }
    else if(m == Map::SHARING_MAP)
    {
      const smt &const_sm = sm;
      before = clock::now();
      const_sm.find(n);
      after = clock::now();
    }

    auto diff = after - before;
    auto duration = duration_cast<nanoseconds>(diff);
    auto add = duration.count();

    assert(add > 0);

    dur += add;
  }

  std::cout << "Find time (ns): " << std::setw(NUM_WIDTH) << dur << std::endl;
}

void memory()
{
  smt sm;
  mapt map;
  unorderedt unordered;

  std::default_random_engine generator;
  std::uniform_int_distribution<unsigned> distribution(0, num_ran - 1);
  auto random_num = std::bind(distribution, generator);

  for(size_t i = 0; i < num_ops; i++)
  {
    unsigned n = random_num();

    if(m == Map::MAP)
    {
      map.insert(std::make_pair(n, 0));
    }
    else if(m == Map::UNORDERED_MAP)
    {
      unordered.insert(std::make_pair(n, 0));
    }
    else if(m == Map::SHARING_MAP)
    {
      sm.insert(n, 0);
    }
  }
}

int main(int argc, char *argv[])
{
  // Parse arguments

  assert(num_ops < num_ran);
  assert(argc >= 4);
  assert(argc <= 5);

  const char *str_ops = argv[1];
  num_ops = atoi(str_ops);
  num_ran = num_ops * 10;

  const char *c_map = argv[2];
  assert(strlen(c_map) == 1);

  const char *c_test = argv[3];
  assert(strlen(c_test) == 1);

  const char *c_repeats = argc == 5 ? argv[4] : "-";
  assert(strlen(c_repeats) == 1);

  if(*c_test == 'm')
    t = Test::MEMORY;
  else if(*c_test == 'i')
    t = Test::INSERT_RUNTIME;
  else if(*c_test == 'f')
    t = Test::FIND_RUNTIME;
  else
    assert(false);

  if(*c_map == 'u')
    m = Map::UNORDERED_MAP;
  else if(*c_map == 'm')
    m = Map::MAP;
  else if(*c_map == 's')
    m = Map::SHARING_MAP;
  else
    assert(false);

  if(*c_repeats == 'r')
    allow_repeats = true;
  else
  {
    assert(*c_repeats == '-');
    allow_repeats = false;
  }

  // Run test

  if(t == Test::MEMORY)
  {
    memory();
  }
  else if(t == Test::INSERT_RUNTIME)
  {
    runtime_insert();
  }
  else if(t == Test::FIND_RUNTIME)
  {
    runtime_find();
  }

  return 0;
}

