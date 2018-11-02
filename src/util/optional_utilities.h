/*******************************************************************\

 Module: functions that are useful with optionalt

 Author: Diffblue Ltd.

\*******************************************************************/

#include "optional.h"

#include <map>
#include <iterator>

/// Lookup a key in a map, if found return the associated value, nullopt otherwise
template<typename keyt, typename valuet>
optionalt<valuet> optional_lookup(const std::map<keyt, valuet>& map, const keyt& key)
{
  auto const it = map.find(key);
  if(it != map.end()) {
    return it->second;
  }
  return nullopt;
}