/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_UTIL_ALLOCATE_OBJECTS_H
#define CPROVER_UTIL_ALLOCATE_OBJECTS_H

#include <util/base_type.h>
#include <util/expr.h>
#include <util/namespace.h>
#include <util/source_location.h>
#include <util/std_code.h>
#include <util/symbol_table.h>
#include <util/type.h>

/// Selects the kind of allocation used by the object factories
enum class allocation_typet
{
  /// Allocate global objects
  GLOBAL,
  /// Allocate local stacked objects
  LOCAL,
  /// Allocate dynamic objects (using MALLOC)
  DYNAMIC
};

class allocate_objectst
{
public:
  allocate_objectst(
    const irep_idt &symbol_mode,
    const source_locationt &source_location,
    const irep_idt &function,
    symbol_table_baset &symbol_table)
    : symbol_mode(symbol_mode),
      source_location(source_location),
      function(function),
      symbol_table(symbol_table),
      ns(symbol_table)
  {
  }

  exprt allocate_object(
    code_blockt &assignments,
    const exprt &target_expr,
    const typet &allocate_type,
    const allocation_typet alloc_type,
    std::vector<const symbolt *> &symbols_created);

  exprt allocate_non_dynamic_object(
    code_blockt &assignments,
    const exprt &target_expr,
    const typet &allocate_type,
    const bool static_lifetime,
    std::vector<const symbolt *> &symbols_created);

  exprt allocate_dynamic_object(
    code_blockt &output_code,
    const exprt &target_expr,
    const typet &allocate_type,
    std::vector<const symbolt *> &symbols_created);

  exprt allocate_dynamic_object_with_decl(
    code_blockt &output_code,
    const exprt &target_expr);

private:
  const irep_idt &symbol_mode;
  const source_locationt &source_location;
  const irep_idt &function;

  symbol_table_baset &symbol_table;
  const namespacet ns;
};

#endif // CPROVER_UTIL_ALLOCATE_OBJECTS_H
