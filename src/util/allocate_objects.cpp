/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include "allocate_objects.h"

#include <util/c_types.h>
#include <util/fresh_symbol.h>
#include <util/pointer_offset_size.h>

/// Installs a new symbol in the symbol table, pushing the corresponding
/// `symbolt` object to the field `symbols_created` and emits to `assignments`
/// a new assignment of the form `<target_expr> := address-of(new_object)`. The
/// `allocate_type` may differ from `target_expr.type()`, e.g. for target_expr
/// having type `int*` and `allocate_type` being an `int[10]`.
///
/// \param assignments: The code block to add code to.
/// \param target_expr: The expression which we are allocating a symbol for.
/// \param allocate_type: Type of the object allocated
/// \param alloc_type: Allocation type (global, local or dynamic)
/// \param symbols_created: Created symbols to be declared by the caller
/// \return An expression denoting the address of the newly allocated object.
exprt allocate_objectst::allocate_object(
  code_blockt &assignments,
  const exprt &target_expr,
  const typet &allocate_type,
  const allocation_typet alloc_type,
  std::vector<const symbolt *> &symbols_created)
{
  switch(alloc_type)
  {
  case allocation_typet::LOCAL:
    return allocate_non_dynamic_object(
      assignments, target_expr, allocate_type, false, symbols_created);
    break;

  case allocation_typet::GLOBAL:
    return allocate_non_dynamic_object(
      assignments, target_expr, allocate_type, true, symbols_created);
    break;

  case allocation_typet::DYNAMIC:
    return allocate_dynamic_object(
      assignments, target_expr, allocate_type, symbols_created);
    break;
  }

  UNREACHABLE;
}

/// Generates code for allocating a non-dynamic local or global object.
///
/// \param assignments: Code block to which the necessary code is added
/// \param target_expr: Expression to which the necessary memory will be
///   allocated, its type should be pointer to `allocate_type`
/// \param allocate_type: Type of the object allocated
/// \param static_lifetime: If `true` a global object will be created, if
///   `false` a local object will be created
/// \param symbols_created: Created symbols to be declared by the caller
/// \return An address_of_exprt of the newly allocated object.
exprt allocate_objectst::allocate_non_dynamic_object(
  code_blockt &assignments,
  const exprt &target_expr,
  const typet &allocate_type,
  const bool static_lifetime,
  std::vector<const symbolt *> &symbols_created)
{
  symbolt &aux_symbol = get_fresh_aux_symbol(
    allocate_type,
    id2string(function),
    "tmp_object_factory",
    source_location,
    symbol_mode,
    symbol_table);

  aux_symbol.is_static_lifetime = static_lifetime;
  symbols_created.push_back(&aux_symbol);

  exprt aoe = address_of_exprt(aux_symbol.symbol_expr());

  if(!base_type_eq(allocate_type, target_expr.type().subtype(), ns))
  {
    aoe = typecast_exprt(aoe, target_expr.type());
  }

  code_assignt code(target_expr, aoe);
  code.add_source_location() = source_location;
  assignments.add(code);

  return aoe;
}

/// Generates code for allocating a dynamic object. This is used in
/// allocate_object() and also in the library preprocessing for allocating
/// strings.
///
/// \param output_code: Code block to which the necessary code is added
/// \param target_expr: Expression to which the necessary memory will be
///   allocated, its type should be pointer to `allocate_type`
/// \param allocate_type: Type of the object allocated
/// \param symbols_created: Created symbols to be declared by the caller
/// \return The dynamic object created
exprt allocate_objectst::allocate_dynamic_object(
  code_blockt &output_code,
  const exprt &target_expr,
  const typet &allocate_type,
  std::vector<const symbolt *> &symbols_created)
{
  // build size expression
  exprt object_size = size_of_expr(allocate_type, ns);

  if(allocate_type.id() != ID_empty)
  {
    INVARIANT(!object_size.is_nil(), "Size of objects should be known");

    // malloc expression
    side_effect_exprt malloc_expr(
      ID_allocate, pointer_type(allocate_type), source_location);
    malloc_expr.copy_to_operands(object_size);
    malloc_expr.copy_to_operands(false_exprt());

    // create a symbol for the malloc expression so we can initialize
    // without having to do it potentially through a double-deref, which
    // breaks the to-SSA phase.
    symbolt &malloc_sym = get_fresh_aux_symbol(
      pointer_type(allocate_type),
      id2string(function),
      "malloc_site",
      source_location,
      symbol_mode,
      symbol_table);

    symbols_created.push_back(&malloc_sym);

    code_assignt assign(malloc_sym.symbol_expr(), malloc_expr);
    assign.add_source_location() = source_location;
    output_code.add(assign);

    exprt malloc_symbol_expr = malloc_sym.symbol_expr();

    if(!base_type_eq(allocate_type, target_expr.type().subtype(), ns))
    {
      malloc_symbol_expr =
        typecast_exprt(malloc_symbol_expr, target_expr.type());
    }

    code_assignt code(target_expr, malloc_symbol_expr);
    code.add_source_location() = source_location;
    output_code.add(code);

    return malloc_sym.symbol_expr();
  }
  else
  {
    // make null
    null_pointer_exprt null_pointer_expr(to_pointer_type(target_expr.type()));
    code_assignt code(target_expr, null_pointer_expr);
    code.add_source_location() = source_location;
    output_code.add(code);

    return exprt();
  }
}

/// Generates code for allocating a dynamic object and creates declarations for
/// the generated symbols.
///
/// \param target_expr: Expression to which the necessary memory will be
///   allocated
/// \param output_code: Code block to which the necessary code is added
/// \return The dynamic object created
exprt allocate_objectst::allocate_dynamic_object_with_decl(
  code_blockt &output_code,
  const exprt &target_expr)
{
  std::vector<const symbolt *> symbols_created;
  code_blockt tmp_block;
  const typet &allocate_type = target_expr.type().subtype();

  const exprt dynamic_object = allocate_dynamic_object(
    tmp_block, target_expr, allocate_type, symbols_created);

  // Add the following code to output_code for each symbol that's been created:
  //   <type> <identifier>;
  for(const symbolt *const symbol_ptr : symbols_created)
  {
    code_declt decl(symbol_ptr->symbol_expr());
    decl.add_source_location() = source_location;
    output_code.add(decl);
  }

  for(const auto &code : tmp_block.statements())
    output_code.add(code);

  return dynamic_object;
}
