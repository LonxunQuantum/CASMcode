#include "casm/clex/Supercell.hh"

#include <math.h>
#include <map>
#include <vector>
#include <stdlib.h>

#include "casm/clex/PrimClex.hh"
#include "casm/clex/ConfigIterator.hh"

namespace CASM {

  const std::string QueryTraits<Supercell>::name = "Supercell";

  //*******************************************************************************

  //Copy constructor is needed for proper initialization of m_prim_grid
  Supercell::Supercell(const Supercell &RHS) :
    m_primclex(RHS.m_primclex),
    m_real_super_lattice(RHS.m_real_super_lattice),
    m_prim_grid((*m_primclex).prim().lattice(), m_real_super_lattice, (*m_primclex).prim().basis.size()),
    m_name(RHS.m_name),
    m_nlist(RHS.m_nlist),
    m_canonical(nullptr),
    m_config_list(RHS.m_config_list),
    m_transf_mat(RHS.m_transf_mat),
    m_id(RHS.m_id) {
  }

  //*******************************************************************************

  Supercell::Supercell(const PrimClex *_prim, const Eigen::Ref<const Eigen::Matrix3i> &transf_mat_init) :
    m_primclex(_prim),
    m_real_super_lattice((*m_primclex).prim().lattice().lat_column_mat() * transf_mat_init.cast<double>()),
    m_prim_grid((*m_primclex).prim().lattice(), m_real_super_lattice, (*m_primclex).prim().basis.size()),
    m_canonical(nullptr),
    m_transf_mat(transf_mat_init) {
    //    fill_reciprocal_supercell();
  }

  //*******************************************************************************

  Supercell::Supercell(const PrimClex *_prim, const Lattice &superlattice) :
    m_primclex(_prim),
    m_real_super_lattice(superlattice),
    m_canonical(nullptr),
    m_prim_grid((*m_primclex).prim().lattice(), m_real_super_lattice, (*m_primclex).prim().basis.size()) {

    auto res = is_supercell(superlattice, prim().lattice(), primclex().settings().crystallography_tol());
    if(!res.first) {
      _prim->err_log() << "Error in Supercell(PrimClex *_prim, const Lattice &superlattice)" << std::endl
                       << "  Bad supercell, the transformation matrix is not integer." << std::endl;
      _prim->err_log() << "superlattice: \n" << superlattice.lat_column_mat() << std::endl;
      _prim->err_log() << "prim lattice: \n" << prim().lattice().lat_column_mat() << std::endl;
      _prim->err_log() << "lin_alg_tol: " << primclex().settings().lin_alg_tol() << std::endl;
      _prim->err_log() << "transformation matrix: \n" << prim().lattice().lat_column_mat().inverse() * superlattice.lat_column_mat() << std::endl;
      throw std::invalid_argument("Error constructing Supercell: the transformation matrix is not integer");
    }
    m_transf_mat = res.second;
  }

  /*****************************************************************/

  std::vector<int> Supercell::max_allowed_occupation() const {
    std::vector<int> max_allowed;

    // Figures out the maximum number of occupants in each basis site, to initialize counter with
    for(Index i = 0; i < prim().basis.size(); i++) {
      std::vector<int> tmp(volume(), prim().basis[i].site_occupant().size() - 1);
      max_allowed.insert(max_allowed.end(), tmp.begin(), tmp.end());
    }
    //std::cout << "max_allowed_occupation is:  " << max_allowed << "\n\n";
    return max_allowed;
  }

  /*****************************************************************/

  const Structure &Supercell::prim() const {
    return m_primclex->prim();
  }

  /// \brief Returns the SuperNeighborList
  const SuperNeighborList &Supercell::nlist() const {

    // if any additions to the prim nlist, must update the super nlist
    if(primclex().nlist().size() != m_nlist_size_at_construction) {
      m_nlist.unique().reset();
    }

    // lazy construction of neighbor list
    if(!m_nlist) {
      m_nlist_size_at_construction = primclex().nlist().size();
      m_nlist = notstd::make_cloneable<SuperNeighborList>(
                  m_prim_grid,
                  primclex().nlist()
                );
    }
    return *m_nlist;
  };

  /*****************************************************************/

  // begin and end iterators for iterating over configurations
  Supercell::config_iterator Supercell::config_begin() {
    return config_iterator(m_primclex, m_id, 0);
  }

  Supercell::config_iterator Supercell::config_end() {
    return ++config_iterator(m_primclex, m_id, config_list().size() - 1);
  }

  // begin and end const_iterators for iterating over configurations
  Supercell::config_const_iterator Supercell::config_cbegin() const {
    return config_const_iterator(m_primclex, m_id, 0);
  }

  Supercell::config_const_iterator Supercell::config_cend() const {
    return ++config_const_iterator(m_primclex, m_id, config_list().size() - 1);
  }

  /*****************************************************************/

  const SymGroup &Supercell::factor_group() const {
    if(!m_factor_group.size())
      generate_factor_group();
    return m_factor_group;
  }

  /*****************************************************************/

  // permutation_symrep() populates permutation symrep if needed
  const Permutation &Supercell::factor_group_permute(Index i) const {
    return *(permutation_symrep().get_permutation(factor_group()[i]));
  }
  /*****************************************************************/

  // PrimGrid populates translation permutations if needed
  const Permutation &Supercell::translation_permute(Index i) const {
    return m_prim_grid.translation_permutation(i);
  }

  /*****************************************************************/

  // PrimGrid populates translation permutations if needed
  const std::vector<Permutation> &Supercell::translation_permute() const {
    return m_prim_grid.translation_permutations();
  }

  /*****************************************************************/

  /// \brief Begin iterator over translation permutations
  Supercell::permute_const_iterator Supercell::translate_begin() const {
    return permute_begin();
  }

  /*****************************************************************/

  /// \brief End iterator over translation permutations
  Supercell::permute_const_iterator Supercell::translate_end() const {
    return permute_begin().begin_next_fg_op();
  }

  /*****************************************************************/
  /* //Example usage case:
   *  Supercell my_supercell;
   *  Configuration my_config(my_supercell, configuration_info);
   *  ConfigDoF my_dof=my_config.configdof();
   *  my_dof.is_canonical(my_supercell.permute_begin(),my_supercell.permute_end());
   */
  Supercell::permute_const_iterator Supercell::permute_begin() const {
    return permute_it(0, 0); // starting indices
  }

  /*****************************************************************/

  Supercell::permute_const_iterator Supercell::permute_end() const {
    return permute_it(factor_group().size(), 0); // one past final indices
  }

  /*****************************************************************/

  Supercell::permute_const_iterator Supercell::permute_it(Index fg_index, Index trans_index) const {
    return permute_const_iterator(SymGroupRep::RemoteHandle(factor_group(), permutation_symrep_ID()),
                                  m_prim_grid,
                                  fg_index, trans_index); // one past final indices
  }

  //*******************************************************************************

  /// \brief Get the PrimClex crystallography_tol
  double Supercell::crystallography_tol() const {
    return primclex().crystallography_tol();
  }

  //***********************************************************

  void Supercell::generate_factor_group()const {
    m_real_super_lattice.find_invariant_subgroup(prim().factor_group(), m_factor_group);
    m_factor_group.set_lattice(m_real_super_lattice);
    return;
  }

  //***********************************************************

  void Supercell::generate_permutations()const {
    if(!m_perm_symrep_ID.empty()) {
      std::cerr << "WARNING: In Supercell::generate_permutations(), but permutations data already exists.\n"
                << "         It will be overwritten.\n";
    }
    m_perm_symrep_ID = m_prim_grid.make_permutation_representation(factor_group(), prim().basis_permutation_symrep_ID());
    //m_trans_permute = m_prim_grid.make_translation_permutations(basis_size()); <--moved to PrimGrid

    /*
      std::cerr << "For SCEL " << " -- " << name() << " Translation Permutations are:\n";
      for(int i = 0; i < m_trans_permute.size(); i++)
      std::cerr << i << ":   " << m_trans_permute[i].perm_array() << "\n";

      std::cerr << "For SCEL " << " -- " << name() << " factor_group Permutations are:\n";
      for(int i = 0; i < m_factor_group.size(); i++){
    std::cerr << "Operation " << i << ":\n";
    m_factor_group[i].print(std::cerr,FRAC);
    std::cerr << '\n';
    std::cerr << i << ":   " << m_factor_group[i].get_permutation_rep(m_perm_symrep_ID)->perm_array() << '\n';

    }
    std:: cerr << "End permutations for SCEL " << name() << '\n';
    */

    return;
  }

  //***********************************************************

  /// \brief Return supercell name
  ///
  /// - If lattice is the canonical equivalent, then return 'SCELV_A_B_C_D_E_F'
  /// - Else, return 'SCELV_A_B_C_D_E_F.$FG_INDEX', where $FG_INDEX is the index of the first
  ///   symmetry operation in the primitive structure's factor group such that the lattice
  ///   is equivalent to `apply(fg_op, canonical equivalent)`
  std::string Supercell::generate_name() const {
    if(is_canonical()) {
      return CASM::generate_name(m_transf_mat);
    }
    else {
      /*
      ... to do ...
      Supercell& canon = canonical_form();
      ScelEnumEquivalents e(canon);

      for(auto it = e.begin(); it != e.end(); ++it) {
        if(this->is_equivalent(*it)) {
          break;
        }
      }

      m_name = canon.name() + "." + std::to_string(e.sym_op().index());
      */

      Supercell &canon = canonical_form();
      return canon.name() + ".non_canonical_equivalent";
    }
  }

  //***********************************************************

  Supercell &Supercell::canonical_form() const {
    if(!m_canonical) {
      m_canonical = &primclex().supercell(primclex().add_supercell(real_super_lattice()));
    }
    return *m_canonical;
  }

  //***********************************************************
  /**  Check if a Structure fits in this Supercell
   *  - Checks that 'structure'.lattice is supercell of 'real_super_lattice'
   *  - Does *NOT* check basis sites
   */
  //***********************************************************
  bool Supercell::is_supercell_of(const Structure &structure) const {
    Eigen::Matrix3d mat;
    return is_supercell_of(structure, mat);
  };

  //***********************************************************
  /**  Check if a Structure fits in this Supercell
   *  - Checks that 'structure'.lattice is supercell of 'real_super_lattice'
   *  - Does *NOT* check basis sites
   */
  //***********************************************************
  bool Supercell::is_supercell_of(const Structure &structure, Eigen::Matrix3d &mat) const {
    Structure tstruct = structure;
    SymGroup point_group;
    tstruct.lattice().generate_point_group(point_group);
    return m_real_super_lattice.is_supercell_of(tstruct.lattice(), point_group, mat);
  };

  //***********************************************************
  /**  Generate a Configuration from a Structure
   *  - Generally expected the user will first call
   *      Supercell::is_supercell_of(const Structure &structure, Matrix3<double> multimat)
   *  - tested OK for perfect prim coordinates, not yet tested with relaxed coordinates using 'tol'
   */
  //***********************************************************
  Configuration Supercell::configuration(const BasicStructure<Site> &structure_to_config, double tol) {
    //Because the user is a fool and the supercell may not be a supercell (This still doesn't check the basis!)
    Eigen::Matrix3d transmat;
    if(!structure_to_config.lattice().is_supercell_of(prim().lattice(), prim().factor_group(), transmat)) {
      std::cerr << "ERROR in Supercell::configuration" << std::endl;
      std::cerr << "The provided structure is not a supercell of the PRIM. Tranformation matrix was:" << std::endl;
      std::cerr << transmat << std::endl;
      exit(881);
    }

    std::cerr << "WARNING in Supercell::config(): This routine has not been tested on relaxed structures using 'tol'" << std::endl;
    //std::cout << "begin config()" << std::endl;
    //std::cout << "  mat:\n" << mat << std::endl;

    const Structure &prim = (*m_primclex).prim();

    // create a 'superstruc' that fills '*this'
    BasicStructure<Site> superstruc = structure_to_config.create_superstruc(m_real_super_lattice);

    //std::cout << "superstruc:\n";
    //superstruc.print(std::cout);
    //std::cout << " " << std::endl;

    // Set the occuation state of a Configuration from superstruc
    //   Allow Va on sites where Va are allowed
    //   Do not allow interstitials, print an error message and exit
    Configuration config(*this);

    // Initially set occupation to -1 (for unknown) on every site
    config.set_occupation(std::vector<int>(num_sites(), -1));

    Index _linear_index, b;
    int val;

    // For each site in superstruc, set occ index
    for(Index i = 0; i < superstruc.basis.size(); i++) {
      //std::cout << "i: " << i << "  basis: " << superstruc.basis[i] << std::endl;
      _linear_index = linear_index(Coordinate(superstruc.basis[i]), tol);
      b = sublat(_linear_index);

      // check that we're not over-writing something already set
      if(config.occ(_linear_index) != -1) {
        std::cerr << "Error in Supercell::config." << std::endl;
        std::cerr << "  Adding a second atom on site: linear index: " << _linear_index << " bijk: " << uccoord(_linear_index) << std::endl;
        exit(1);
      }

      // check that the Molecule in superstruc is allowed on the site in 'prim'
      if(!prim.basis[b].contains(superstruc.basis[i].occ_name(), val)) {
        std::cerr << "Error in Supercell::config." << std::endl;
        std::cerr << "  The molecule: " << superstruc.basis[i].occ_name() << " is not allowed on basis site " << b << " of the Supercell prim." << std::endl;
        exit(1);
      }
      config.set_occ(_linear_index, val);
    }

    // Check that vacant sites are allowed
    for(Index i = 0; i < config.size(); i++) {
      if(config.occ(i) == -1) {
        b = sublat(i);

        if(prim.basis[b].contains("Va", val)) {
          config.set_occ(i, val);
        }
        else {
          std::cerr << "Error in Supercell::config." << std::endl;
          std::cerr << "  Missing atom.  Vacancies are not allowed on the site: " << uccoord(i) << std::endl;
          exit(1);
        }
      }
    }

    return config;

  };

  //***********************************************************
  /**  Returns a Structure equivalent to the Supercell
   *  - basis sites are ordered to agree with Supercell::config_index_to_bijk
   *  - occupation set to prim default, not curr_state
   */
  //***********************************************************
  Structure Supercell::superstructure() const {
    // create a 'superstruc' that fills '*this'
    Structure superstruc = (*m_primclex).prim().create_superstruc(m_real_super_lattice);

    // sort basis sites so that they agree with config_index_to_bijk
    //   This sorting may not be necessary,
    //   but it depends on how we construct the config_index_to_bijk,
    //   so I'll leave it in for now just to be safe
    for(Index i = 0; i < superstruc.basis.size(); i++) {
      superstruc.basis.swap_elem(i, linear_index(superstruc.basis[i]));
    }

    //superstruc.reset();

    //set_site_internals() is better than Structure::reset(), because
    //it doesn't destroy all the info that
    //Structure::create_superstruc makes efficiently
    superstruc.set_site_internals();
    return superstruc;

  }

  //***********************************************************
  /**  Returns a Structure equivalent to the Supercell
   *  - basis sites are ordered to agree with Supercell::config_index_to_bijk
   *  - occupation set to config
   *  - prim set to (*m_primclex).prim
   */
  //***********************************************************
  Structure Supercell::superstructure(const Configuration &config) const {
    // create a 'superstruc' that fills '*this'
    Structure superstruc = superstructure();

    // set basis site occupants
    for(Index i = 0; i < superstruc.basis.size(); i++) {
      superstruc.basis[i].set_occ_value(config.occ(i));
    }

    // setting the occupation changes symmetry properties, so must reset
    superstruc.reset();

    return superstruc;

  }

  /**
   * This is a safer version that takes an Index instead of an actual Configuration.
   * It might be better to have the version that takes a Configuration private,
   * that way you can't pass it anything that's incompatible.
   */

  Structure Supercell::superstructure(Index config_index) const {
    if(config_index >= config_list().size()) {
      std::cerr << "ERROR in Supercell::superstructure" << std::endl;
      std::cerr << "Requested superstructure of configuration with index " << config_index << " but there are only " << config_list().size() << " configurations" << std::endl;
      exit(185);
    }
    return superstructure(m_config_list[config_index]);
  }

  //***********************************************************
  /**  Returns an std::vector<int> consistent with
   *     Configuration::occupation that is all vacancies.
   *     A site which can not contain a vacancy is set to -1.
   */
  //***********************************************************
  std::vector<int> Supercell::vacant() const {
    std::vector<int> occupation(num_sites(), -1);
    int b, index;
    for(Index i = 0; i < num_sites(); i++) {
      b = sublat(i);
      if(prim().basis[b].contains("Va", index)) {
        occupation[i] = index;
      }
    }
    return occupation;
  }

  bool Supercell::operator<(const Supercell &B) const {
    if(&primclex() != &B.primclex()) {
      throw std::runtime_error(
        "Error using Supercell::operator<(const Supercell& B): "
        "Only Supercell with the same PrimClex may be compared this way.");
    }
    if(volume() != B.volume()) {
      return volume() < B.volume();
    }
    return real_super_lattice() < B.real_super_lattice();
  }

  bool Supercell::_eq(const Supercell &B) const {
    if(&primclex() != &B.primclex()) {
      throw std::runtime_error(
        "Error using Supercell::operator==(const Supercell& B): "
        "Only Supercell with the same PrimClex may be compared this way.");
    }
    return transf_mat() == B.transf_mat();
  }


  Supercell &apply(const SymOp &op, Supercell &scel) {
    return scel = copy_apply(op, scel);
  }

  Supercell copy_apply(const SymOp &op, const Supercell &scel) {
    return Supercell(&scel.primclex(), copy_apply(op, scel.real_super_lattice()));
  }


  std::string generate_name(const Eigen::Matrix3i &transf_mat) {
    std::string name_str;

    Eigen::Matrix3i H = hermite_normal_form(transf_mat).first;
    name_str = "SCEL";
    std::stringstream tname;
    //Consider using a for loop with HermiteCounter_impl::_canonical_unroll here
    tname << H(0, 0)*H(1, 1)*H(2, 2)
          << "_" << H(0, 0) << "_" << H(1, 1) << "_" << H(2, 2)
          << "_" << H(1, 2) << "_" << H(0, 2) << "_" << H(0, 1);
    name_str.append(tname.str());

    return name_str;
  }

}

