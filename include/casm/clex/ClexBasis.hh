#ifndef CLEXBASIS_HH
#define CLEXBASIS_HH

#include <string>
#include "casm/basis_set/BasisSet.hh"


namespace CASM {
  class Structure;
  class UnitCell;


  class PrimNeighborList;

  class BasisBuilder;

  // This is a hack to forward-declare IntegralCluster.  Forward declarations
  // of typedefs should probably get their own *.hh files, without any dependencies
  template<typename CoordType>
  class CoordCluster;
  class UnitCellCoord;
  typedef CoordCluster<UnitCellCoord> IntegralCluster;

  class ClexBasis {
  public:
    typedef std::vector<BasisSet> BSetOrbit;
    typedef std::string DoFKey;
    typedef std::vector<BSetOrbit>::const_iterator BSetOrbitIterator;

    /// \brief Initialize from Structure, in order to get Site DoF and global DoF info
    ClexBasis(Structure const &_prim) {}

    /// \brief Total number of BasisSet orbits
    Index n_orbits() const;

    /// \brief Total number of basis functions
    Index n_funcions() const;

    /// \brief Const access of clust basis of orbit @param orbit_ind and equivalent cluster @param equiv_ind
    BasisSet const &clust_basis(Index orbit_ind,
                                Index equiv_ind) const;

    /// \brief Const access of BSetOrbit of orbit @param orbit_ind
    BSetOrbit const &bset_orbit(Index orbit_ind) const;

    /// \brief Const iterator to first BasisSet orbit
    BSetOrbitIterator begin() const {
      return m_bset_tree.cbegin();
    }

    /// \brief Const iterator to first BasisSet orbit
    BSetOrbitIterator cbegin() const {
      return m_bset_tree.cbegin();
    }

    /// \brief Const past-the-end iterator for BasisSet orbits
    BSetOrbitIterator end() const {
      return m_bset_tree.cbegin();
    }

    /// \brief Const past-the-end iterator for BasisSet orbits
    BSetOrbitIterator cend() const {
      return m_bset_tree.cbegin();
    }

    jsonParser const &bspecs() const {
      return m_bspecs;
    }

    /// \brief Const access to dictionary of all site BasisSets
    std::map<DoFKey, std::vector<BasisSet> > const &site_bases()const {
      return m_site_bases;
    }

    /// \brief Const access to dictionary of all global BasisSets
    std::map<DoFKey, BasisSet> const &global_bases()const {
      return m_global_bases;
    }

    /// \brief generate clust_basis for all equivalent clusters in @param _orbitree
    template<typename OrbitIterType>
    void generate(OrbitIterType _begin,
                  OrbitIterType _end,
                  jsonParser const &_bspecs,
                  Index max_poly_order = -1);

  private:
    template<typename OrbitType>
    BasisSet _construct_prototype_basis(OrbitType const &_orbit,
                                        std::vector<DoFKey> const &local_keys,
                                        std::vector<DoFKey> const &global_keys,
                                        Index max_poly_order) const;

    /// \brief Performs heavy lifting for populating site bases in m_site_bases
    void _populate_site_bases(Structure const &_prim);

    notstd::cloneable_ptr<BasisBuilder> m_basis_builder;

    /// \brief Collection of all cluster BasisSets, one per cluster orbit
    std::vector<BSetOrbit> m_bset_tree;

    /// \brief Dictionary of all site BasisSets, initialized on construction
    std::map<DoFKey, std::vector<BasisSet> > m_site_bases;

    /// \brief Dictionary of all global BasisSets, initialized
    std::map<DoFKey, BasisSet> m_global_bases;

    jsonParser m_bspecs;
  };


  class BasisBuilder {
  public:
    virtual ~BasisBuilder() {}

    virtual void prepare(Structure const &_prim) {

    }

    virtual std::vector<ClexBasis::DoFKey> filter_dof_types(std::vector<ClexBasis::DoFKey> const &_dof_types) {
      return _dof_types;
    }

    virtual void pre_generate() {

    }

    virtual BasisSet build(IntegralCluster const &_prototype,
                           std::vector<BasisSet const *> const &_arg_bases,
                           Index max_poly_order,
                           Index min_poly_order) = 0;

    std::unique_ptr<BasisBuilder> clone()const {
      return std::unique_ptr<BasisBuilder>(_clone());
    }

  private:
    virtual BasisBuilder *_clone()const = 0;

  };

  /// Print cluster with basis_index and nlist_index (from 0 to size()-1), followed by cluster basis functions
  /// Functions are labeled \Phi_{i}, starting from i = @param begin_ind
  /// Returns the number of functions that were printed
  Index print_clust_basis(std::ostream &stream,
                          BasisSet _clust_basis,
                          IntegralCluster const &_prototype,
                          Index func_ind = 0,
                          int space = 18,
                          char delim = '\n');

  /// returns std::vector of std::string, each of which is
  template<typename OrbitType>
  std::vector<std::string> orbit_function_cpp_strings(ClexBasis::BSetOrbit _bset_orbit, // used as temporary
                                                      OrbitType const &_clust_orbit,
                                                      PrimNeighborList &_nlist,
                                                      std::vector<FunctionVisitor *> const &labelers);

  /// nlist_index is the index into the nlist for the site the flower centers on
  template<typename OrbitType>
  std::vector<std::string> flower_function_cpp_strings(ClexBasis::BSetOrbit _bset_orbit, // used as temporary
                                                       OrbitType const &_clust_orbit,
                                                       PrimNeighborList &_nlist,
                                                       std::vector<FunctionVisitor *> const &labelers,
                                                       Index sublat_index);

  /// b_index is the basis site index, f_index is the index of the configurational site basis function in Site::occupant_basis
  /// nlist_index is the index into the nlist for the site the flower centers on
  template<typename OrbitType>
  std::map< UnitCell, std::vector< std::string > > delta_occfunc_flower_function_cpp_strings(ClexBasis::BSetOrbit _bset_orbit, // used as temporary
      OrbitType const &_clust_orbit,
      PrimNeighborList &_nlist,
      BasisSet site_basis,
      const std::vector<FunctionVisitor *> &labelers,
      Index nlist_index,
      Index b_index,
      Index f_index);


  namespace ClexBasis_impl {
    std::vector<ClexBasis::DoFKey> extract_dof_types(Structure const &_prim);

    BasisSet construct_clust_dof_basis(IntegralCluster const &_clust,
                                       std::vector<BasisSet const *> const &site_dof_sets);
  }
}
#include "casm/clex/ClexBasis_impl.hh"
#endif
