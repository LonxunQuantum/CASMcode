#ifndef CASM_core_io_traits_global_enum
#define CASM_core_io_traits_global_enum

#include "casm/global/enum.hh"
#include "casm/casm_io/traits/enum.hh"

namespace CASM {

  ENUM_TRAITS(COORD_TYPE)
  ENUM_TRAITS(PERIODICITY_TYPE)
  ENUM_TRAITS(EQUIVALENCE_TYPE)
  ENUM_TRAITS(CELL_TYPE)
  ENUM_TRAITS(OnError)

}

#endif
