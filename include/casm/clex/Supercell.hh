#ifndef CASM_Supercell
#define CASM_Supercell

#include "casm/misc/cloneable_ptr.hh"
#include "casm/misc/Comparisons.hh"
#include "casm/crystallography/Lattice.hh"
#include "casm/symmetry/SymGroup.hh"
#include "casm/symmetry/SymGroupRepID.hh"
#include "casm/symmetry/SupercellSymInfo.hh"
#include "casm/clex/SupercellTraits.hh"
#include "casm/clex/HasPrimClex.hh"
#include "casm/clex/HasCanonicalForm.hh"
#include "casm/database/Named.hh"

namespace CASM {

  namespace xtal {
    class Site;
    class Coordinate;
    class UnitCellCoord;
    class IntegralCoordinateWithin_f;
  }
  using xtal::Site;
  using xtal::Coordinate;
  using xtal::UnitCellCoord;

  template<typename T, typename U> class ConfigIterator;
  class PermuteIterator;
  class PrimClex;
  class Clexulator;
  class ConfigDoF;
  class Configuration;
  class PrimNeighborList;
  class SuperNeighborList;
  class Structure;

  namespace DB {
    template<typename T> class DatabaseIterator;
  }

  struct ConfigMapCompare {
    bool operator()(const Configuration *A, const Configuration *B) const;
  };

  /** \defgroup Supercell
   *  \ingroup Clex
   *  \brief Represents a supercell of the primitive parent crystal structure
   *  @{
   */

  /// \brief Represents a supercell of the primitive parent crystal structure
  ///
  class Supercell : public
    DB::Named <
    Comparisons <
    SupercellCanonicalForm <
    CRTPBase<Supercell >>> > {

  public:

    using permute_const_iterator = SupercellSymInfo::permute_const_iterator;

    // **** Constructors ****

    Supercell(const Supercell &RHS);

    Supercell(std::shared_ptr<Structure const> const &_shared_prim, const Lattice &superlattice);
    Supercell(std::shared_ptr<Structure const> const &_shared_prim, const Eigen::Ref<const Eigen::Matrix3i> &superlattice_matrix);

    Supercell(const PrimClex *_prim, const Lattice &superlattice);
    Supercell(const PrimClex *_prim, const Eigen::Ref<const Eigen::Matrix3i> &superlattice_matrix);

    ~Supercell();

    // **** Coordinates ****

    /// \brief Return the sublattice index for a linear index
    Index sublat(Index linear_index) const;

    /// \brief Given a Coordinate and tolerance, return linear index into Configuration
    Index linear_index(const Coordinate &coord, double tol = TOL) const;

    /// \brief Return the linear index corresponding to integral coordinates
    Index linear_index(const UnitCellCoord &bijk) const;

    /// \brief Return the linear index corresponding to integral coordinates
    Coordinate coord(Index linear_index) const;

    /// \brief Return the integral coordinates corresponding to a linear index
    UnitCellCoord uccoord(Index linear_index) const;


    /// \brief returns maximum allowed occupation bitstring -- used for initializing enumeration counters
    std::vector<int> max_allowed_occupation() const;

    /// \brief returns Supercell-compatible configdof with zeroed DoF values and user-specified tolerance
    ConfigDoF zero_configdof(double tol) const;

    // **** Accessors ****

    const Structure &prim() const;

    std::shared_ptr<Structure const> const &shared_prim() const;

    double crystallography_tol() const;

    const PrimClex &primclex() const;

    ///Return number of primitive cells that fit inside of *this
    Index volume() const;

    Index basis_size() const;

    Index num_sites() const;

    SymGroupRep const &permutation_symrep() const;

    Eigen::Matrix3l transf_mat() const;

    /// \brief The super lattice
    const Lattice &lattice() const;

    /// Set the PrimNeighborList directly
    ///
    /// Note:
    /// - If this Supercell was constructed with a PrimClex const *, PrimNeighborList is already set
    void set_prim_nlist(std::shared_ptr<PrimNeighborList> const &shared_prim_nlist);

    /// \brief Returns the SuperNeighborList
    ///
    /// Requires that the prim_nlist has been set by one of:
    /// - constructing Supercell with a PrimClex const *
    /// - setting the PrimNeighborList directly with `set_prim_nlist`
    ///
    /// At each access, the underlying PrimNeighborList will be checked and if it has been expanded
    /// then the SuperNeighborList will be extended also. References obtained from this function
    /// will be out of date if the underlying PrimNeighborList has been expanded, so it is prudent
    /// to only access the SuperNeighborList for immediate use.
    const SuperNeighborList &nlist() const;

    // Factor group of this supercell
    const SymGroup &factor_group() const;

    // SymInfo object of this supercell
    const SupercellSymInfo &sym_info() const;

    bool operator<(const Supercell &B) const;

    /// \brief Insert the canonical form of this into the database
    std::pair<DB::DatabaseIterator<Supercell>, bool> insert() const;

  private:

    friend Comparisons<SupercellCanonicalForm<CRTPBase<Supercell>>>;
    friend DB::Named<Comparisons<SupercellCanonicalForm<CRTPBase<Supercell>>>>;

    bool eq_impl(const Supercell &B) const;

    // **** Generating functions ****

    std::string generate_name_impl() const;

    // May be nullptr, in which case, some features will throw
    const PrimClex *m_primclex;

    std::shared_ptr<Structure const> m_shared_prim;

    SupercellSymInfo m_sym_info;

    /// shared PrimNeighborList
    std::shared_ptr<PrimNeighborList> m_prim_nlist;

    /// SuperNeighborList, mutable for lazy construction
    mutable notstd::cloneable_ptr<SuperNeighborList> m_nlist;

    /// Store size of PrimNeighborList at time of construction of SuperNeighborList
    /// to enable checking if SuperNeighborList should be re-constructed
    mutable Index m_nlist_size_at_construction;

  };

  void write_pos(Supercell const &_scel);

  std::string pos_string(Supercell const &_scel);

  /// \brief Get canonical supercell from name. If not yet in database, construct and insert.
  const Supercell &make_supercell(const PrimClex &primclex, std::string name);

  /// \brief Construct non-canonical supercell from name. Uses equivalent niggli lattice.
  std::shared_ptr<Supercell> make_shared_supercell(const PrimClex &primclex, std::string name);

  Supercell &apply(const SymOp &op, Supercell &scel);

  Supercell copy_apply(const SymOp &op, const Supercell &scel);

  Eigen::Matrix3i transf_mat(const Lattice &prim_lat, const Lattice &super_lat);

  std::string generate_name(const Eigen::Matrix3i &transf_mat);

  std::string scelname(const Structure &prim, const Lattice &superlat);

  std::string canonical_scelname(const Structure &prim, const Lattice &superlat);

  namespace xtal {
    IntegralCoordinateWithin_f make_bring_within_f(const Supercell &scel);
  }

  /** @} */
}
#endif
