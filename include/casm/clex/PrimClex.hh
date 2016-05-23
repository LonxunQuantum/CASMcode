#ifndef PRIMCLEX_HH
#define PRIMCLEX_HH

#include "casm/external/boost.hh"

#include "casm/misc/cloneable_ptr.hh"
#include "casm/casm_io/Log.hh"

#include "casm/crystallography/Structure.hh"
#include "casm/clex/DoFManager.hh"
#include "casm/clex/CompositionConverter.hh"
#include "casm/clex/Supercell.hh"
#include "casm/clex/Clexulator.hh"
#include "casm/clex/ChemicalReference.hh"
#include "casm/misc/cloneable_ptr.hh"
#include "casm/clex/NeighborList.hh"

#include "casm/app/DirectoryStructure.hh"
#include "casm/app/ProjectSettings.hh"

/// Cluster expansion class
namespace CASM {

  class ECIContainer;

  template<typename T, typename U> class ConfigIterator;

  /// \defgroup Clex
  ///
  /// \brief A Configuration represents the values of all degrees of freedom in a Supercell
  ///


  /// \brief PrimClex stores the primitive Structure and lots of related data
  ///
  /// \ingroup Clex
  ///
  class PrimClex {

    fs::path root;

    DirectoryStructure m_dir;
    ProjectSettings m_settings;

    std::string m_name;

    Structure prim;
    bool m_vacancy_allowed;
    Index m_vacancy_index;

    mutable DoFManager m_dof_manager;


    // CASM project current settings: used to determine where to write things
    std::vector<std::string> curr_property;
    std::string curr_clex;
    std::string curr_calctype;
    std::string curr_ref;
    std::string curr_bset;
    std::string curr_eci;

    // Runtime library compilation settings: compilation options
    std::string compile_options;
    std::string so_options;

    SiteOrbitree global_orbitree;

    /// Contains all the supercells that were involved in the enumeration.
    boost::container::stable_vector< Supercell > supercell_list;


    /// CompositionConverter specifies parameteric composition axes and converts between
    ///   parametric composition and mol composition
    bool m_has_composition_axes = false;
    CompositionConverter m_comp_converter;

    /// ChemicalReference specifies a reference for formation energies, chemical
    /// potentials, etc.
    notstd::cloneable_ptr<ChemicalReference> m_chem_ref;

    /// Stores the neighboring UnitCell and which sublattices to include in neighbor lists
    /// - mutable for lazy construction
    mutable notstd::cloneable_ptr<PrimNeighborList> m_nlist;


  public:

    typedef ConfigIterator<Configuration, PrimClex> config_iterator;
    typedef ConfigIterator<const Configuration, const PrimClex> config_const_iterator;

    // **** Constructors ****

    /// Initial construction of a PrimClex, from a primitive Structure
    PrimClex(const Structure &_prim, Log &log = default_log());

    /// Construct PrimClex from existing CASM project directory
    ///  - read PrimClex and directory structure to generate all its Supercells and Configurations, etc.
    PrimClex(const fs::path &_root, Log &log = default_log());


    // **** Accessors ****

    /// Return project name
    std::string name() const;


    // ** Directory path accessors **

    const DirectoryStructure &dir() const {
      return m_dir;
    }

    ProjectSettings &settings() {
      return m_settings;
    }

    const ProjectSettings &settings() const {
      return m_settings;
    }

    double crystallography_tol() const {
      return settings().crystallography_tol();
    }

    double lin_alg_tol() const {
      return settings().lin_alg_tol();
    }

    /// Return casm project directory path
    fs::path get_path() const;

    /// Return supercell directory path
    fs::path get_path(const Index &scel_index) const;

    /// Return configuration directory path
    fs::path get_path(const Index &scel_index, const Index &config_index) const;

    /// Return config_list.json file path
    fs::path get_config_list_path() const;

    // ** Current settings accessors **

    /// Return current property settings
    const std::vector<std::string> &get_curr_property() const;

    /// Return current clex settings
    std::string get_curr_clex() const;

    /// Return current calctype setting
    std::string get_curr_calctype() const;

    /// Return current reference setting
    std::string get_curr_ref() const;

    /// Return basis set settings
    std::string get_curr_bset() const;

    /// Return current global clexulator name
    std::string get_curr_clexulator() const;

    /// Return current eci settings
    std::string get_curr_eci() const;

    /// Return compiler options
    std::string get_compile_options() const;

    /// Return shared library options
    std::string get_so_options() const;

    // ** Composition accessors **

    /// check if CompositionConverter object initialized
    bool has_composition_axes() const;

    /// const Access CompositionConverter object
    const CompositionConverter &composition_axes() const;

    // ** Chemical reference **

    /// check if ChemicalReference object initialized
    bool has_chemical_reference() const;

    /// const Access ChemicalReference object
    const ChemicalReference &chemical_reference() const;


    // ** Prim and Orbitree accessors **

    /// const Access to primitive Structure
    const Structure &get_prim() const;

    /// const Access to global orbitree
    const SiteOrbitree &get_global_orbitree() const;

    ///Access to the primitive neighbor list
    PrimNeighborList &nlist() const;

    /// returns true if vacancy are an allowed species
    bool vacancy_allowed() const;

    /// returns the index of vacancies in composition vectors
    Index vacancy_index() const;

    // ** Supercell and Configuration accessors **

    /// const Access entire supercell_list
    const boost::container::stable_vector<Supercell> &get_supercell_list() const;

    /// const Access supercell by index
    const Supercell &get_supercell(Index i) const;

    /// Access supercell by index
    Supercell &get_supercell(Index i);

    /// const Access supercell by name
    const Supercell &get_supercell(std::string scellname) const;

    /// Access supercell by name
    Supercell &get_supercell(std::string scellname);

    /// access configuration by name (of the form "scellname/[NUMBER]", e.g., ("SCEL1_1_1_1_0_0_0/0")
    const Configuration &configuration(const std::string &configname) const;
    Configuration &configuration(const std::string &configname);

    /// Configuration iterator: begin
    config_iterator config_begin();

    /// Configuration iterator: end
    config_iterator config_end();

    /// Configuration iterator: begin
    config_const_iterator config_begin() const;

    /// Configuration iterator: end
    config_const_iterator config_end() const;

    /// const Configuration iterator: begin
    config_const_iterator config_cbegin() const;

    /// const Configuration iterator: end
    config_const_iterator config_cend() const;

    /// Configuration iterator: begin
    config_iterator selected_config_begin();

    /// Configuration iterator: end
    config_iterator selected_config_end();

    /// const Configuration iterator: begin
    config_const_iterator selected_config_cbegin() const;

    /// const Configuration iterator: end
    config_const_iterator selected_config_cend() const;


    Eigen::MatrixXd shift_vectors() const;

    // **** Mutators ****

    /// Sets the composition axes, updates all configuration references,
    ///   and writes the updated configuration info
    void set_composition_axes(const CompositionConverter &_converter);

    // **** IO ****

    ///Call Configuration::write on every configuration to update files
    ///  - call update to also read all files
    void write_config_list();


    // **** Operators ****

    // **** Functions for preparing CLEXulators ****

    //Generate the global orbitree
    //John G 011013
    /// Use the given CSPECS

    //Read the global Orbitree from a clust.json file
    void read_global_orbitree(const fs::path &fclust);

    //Generate supercells of a certain volume and store them in the array of supercells
    void generate_supercells(int volStart, int volEnd, bool verbose);

    //Enumerate configurations for all the supercells that are stored in 'supercell_list'
    void print_enum_info(std::ostream &stream);
    void print_supercells() const;
    void print_supercells(std::ostream &stream) const;
    void read_supercells(std::istream &stream);
    void print_clex_configurations();


    //ParamComposition i/o and calculators in PrimClex

    void read_config_list();

    ///Fill up props of every configuration for a partucluar supercell. This will be deprecated when props disappears
    void read_scel_props(int scel_index, const std::string &JSON_output);
    ///Call read_config_props on every Supercell
    void read_all_scel_props(const std::string &JSON_output);

    ///Count over the number of configurations that are selected in all supercells
    int amount_selected() const;

    bool contains_supercell(std::string scellname, Index &index) const;

    Index add_supercell(const Lattice &superlat);

    Index add_canonical_supercell(const Lattice &superlat);

    Eigen::Matrix3i calc_transf_mat(const Lattice &superlat) const;

    /// Set internal values of each DoFEnvironment
    void set_global_dof_state(const Configuration &curr_config)const {
      m_dof_manager.set_global_dof_state(curr_config);
    };

    void set_local_dof_state(const Configuration &curr_config, Index l)const {
      m_dof_manager.set_local_dof_state(curr_config, l);
    };

    /// Delete 'properties.ref_state.X.json' files,
    /// Then call 'clear_reference_properties'
    //void clear_reference_states();

    /// Sets the root reference state to be the calculated properties of the chosen config
    /// Calls 'clear_reference_properties'
    //void set_reference_state(int refid, const Configuration &config);

    /// Check that it is valid to use 'config' as reference state 'refid', returns bool and if false, sets 'reason_invalid'
    ///   Currently checks:
    ///     1) that the necessary properties have been calculated,
    ///     2) that the same Configuration is not being used twice
    ///   Needs to check that reference states span composition space
    //bool valid_reference_state(int refid, const Configuration &config, std::string &reason_invalid) const;

    /// find calculated configurations closest to
    /// [0, 0, 0, ...], [1, 0, 0, ...], [0, 1, 0, ...], [0, 0, 1, ...], ...
    /// and set them as the root reference states, also calls regenerate_references
    /// Clears reference states and properties whether or not it succeeds
    //void set_reference_state_auto();

    /// Clear 'reference' and 'delta' properties from all Configurations
    /// Re-write all Configurations, updating:
    ///   param_composition.json
    ///   properties.calc.json
    ///   properties.ref.json
    ///   properties.delta.json
    //void generate_references();

    bool has_global_clexulator() const;
    Clexulator global_clexulator() const;

    bool has_global_eci(std::string clex_name) const;
    const ECIContainer &global_eci(std::string clex_name) const;
  private:

    /// Initialization routines
    void _init(Log &log);

    mutable ECIContainer m_global_eci;
    mutable Clexulator m_global_clexulator;
  };


  /// \brief Make orbitree. For now specifically global.
  SiteOrbitree make_orbitree(Structure &prim, const jsonParser &json);

  /// \brief Print clexulator
  void print_clexulator(const Structure &prim,
                        SiteOrbitree &tree,
                        const PrimNeighborList &nlist,
                        std::string class_name,
                        std::ostream &stream,
                        double xtal_tol);

}
#endif
