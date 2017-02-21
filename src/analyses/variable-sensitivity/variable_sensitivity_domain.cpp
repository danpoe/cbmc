/*******************************************************************\

Module: Abstract Interpretation

Author: Martin Brain

Date: April 2016

\*******************************************************************/


#include <util/message.h>

#include "variable_sensitivity_domain.h"


/*******************************************************************\

Function: variable_sensitivity_domaint::transform

  Inputs: The instruction before (from) and after (to) the abstract domain,
          the abstract interpreter (ai) and the namespace (ns).

 Outputs: None

 Purpose: Compute the abstract transformer for a single instruction

\*******************************************************************/

void variable_sensitivity_domaint::transform(
    locationt from,
    locationt to,
    ai_baset &ai,
    const namespacet &ns)
{
  const goto_programt::instructiont &instruction=*from;
  switch(instruction.type)
  {
  case DECL:
    // Creates a new variable, which should be top
    // but we don't store top so ... no action required
    break;

  case DEAD:
    {
    // Assign to top is the same as removing
    abstract_object_pointert top_object=
      abstract_state.abstract_object_factory(
        to_code_dead(instruction.code).symbol().type(), true);
    abstract_state.assign(to_code_dead(instruction.code).symbol(), top_object);
    }
    break;

  case ASSIGN:
    {
      const code_assignt &inst = to_code_assign(instruction.code);

      // TODO : check return values
      abstract_object_pointert r = abstract_state.eval(inst.rhs());
      abstract_state.assign(inst.lhs(), r);
    }
    break;

  case GOTO:
    {
      // TODO(tkiley): add support for flow sensitivity
#if 0
      if (flow_sensitivity == FLOW_SENSITIVE)
      {
        locationt next=from;
        next++;
        if(next==to)
          assume(not_exprt(instruction.guard));
        else
          assume(instruction.guard);
      }
#endif
    }
    break;

  case ASSUME:
    abstract_state.assume(instruction.guard);
    break;

  case FUNCTION_CALL:
    // FIXME : Ignore as not yet interprocedural
    break;

  case END_FUNCTION:
    // FIXME : Ignore as not yet interprocedural
    break;

    /***************************************************************/

  case ASSERT:
    // Conditions on the program, do not alter the data or information
    // flow and thus can be ignored.
    break;

  case SKIP:
  case LOCATION:
    // Can ignore
    break;

  case RETURN:
    throw "return instructions are depreciated";

  case START_THREAD:
  case END_THREAD:
  case ATOMIC_BEGIN:
  case ATOMIC_END:
    throw "threading not supported";

  case THROW:
  case CATCH:
    throw "exceptions not handled";

  case OTHER:
//    throw "other";
      break;

  default:
    throw "unrecognised instruction type";
  }
}

/*******************************************************************\

Function: variable_sensitivity_domaint::output

  Inputs: The output stream (out), the abstract interpreter (ai) and
          the namespace.

 Outputs: None

 Purpose: Basic text output of the abstract domain

\*******************************************************************/
void variable_sensitivity_domaint::output(
  std::ostream &out,
  const ai_baset &ai,
  const namespacet &ns) const
{
  abstract_state.output(out, ai, ns);
}

/*******************************************************************\

Function: variable_sensitivity_domaint::make_bottom

  Inputs: None

 Outputs: None

 Purpose: Sets the domain to bottom (no relations).

\*******************************************************************/
void variable_sensitivity_domaint::make_bottom()
{
  abstract_state.make_bottom();

  return;
}

/*******************************************************************\

Function: variable_sensitivity_domaint::make_top

  Inputs: None

 Outputs: None

 Purpose: Sets the domain to top (all relations).

\*******************************************************************/
void variable_sensitivity_domaint::make_top()
{
  abstract_state.make_top();
}


/*******************************************************************\

Function: variable_sensitivity_domaint::make_entry

  Inputs: None

 Outputs: None

 Purpose: Set up a sane entry state.

\*******************************************************************/
void variable_sensitivity_domaint::make_entry()
{
  abstract_state.make_bottom();
  is_bottom=false;
}

/*******************************************************************\

Function: variable_sensitivity_domaint::merge

  Inputs: The other domain (b) and it's preceding location (from) and
          current location (to).

 Outputs: True if something has changed.

 Purpose: Computes the join between "this" and "b".

\*******************************************************************/
bool variable_sensitivity_domaint::merge(
  const variable_sensitivity_domaint &b,
  locationt from,
  locationt to)
{
  // Use the abstract_environment merge
  bool any_changes=abstract_state.merge(b.abstract_state);
  if(abstract_state.get_is_bottom() && !is_bottom)
  {
    is_bottom=true;
    return true;
  }
  else
  {
    return any_changes;
  }
}




