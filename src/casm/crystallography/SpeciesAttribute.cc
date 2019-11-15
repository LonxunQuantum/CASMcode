#include "casm/misc/CASM_Eigen_math.hh"
#include "casm/crystallography/SpeciesAttribute.hh"
#include "casm/crystallography/SymType.hh"

namespace CASM {
  namespace xtal {
    SpeciesAttribute &SpeciesAttribute::apply_sym(SymOp const &_op) {
      m_value = traits().symop_to_matrix(get_matrix(_op), get_translation(_op), get_time_reversal(_op)) * m_value;
      return *this;
    }

    //*******************************************************************

    bool SpeciesAttribute::identical(SpeciesAttribute const &other, double _tol) const {
      return name() == other.name() && almost_equal(value(), other.value(), _tol);
    }
  }

}
