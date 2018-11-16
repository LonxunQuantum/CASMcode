#ifndef CASM_ProjectSettings
#define CASM_ProjectSettings

#include <string>
#include <vector>
#include <map>
#include <set>

#include "casm/CASM_global_Eigen.hh"
#include "casm/app/ClexDescription.hh"
#include "casm/app/DirectoryStructure.hh"
#include "casm/casm_io/Log.hh"
#include "casm/misc/cloneable_ptr.hh"
#include "casm/app/HamiltonianModules.hh"

namespace CASM {

  class Configuration;
  class jsonParser;
  class EnumeratorHandler;
  template<typename T> class QueryHandler;
  class HamiltonianModules;

  /** \defgroup Project
   *
   *  \brief Relates to CASM project settings, directory structure, etc.
   *
   *  A CASM project encompasses all the settings, calculations, cluster
   *  expansions, Monte Carlo results, etc. related to a single parent
   *  crystal structure (known as the 'prim').
   *
   *  All the results and data related to a CASM project are stored in a
   *  directory with structure defined by the DirectoryStructure class, and
   *  accessible through the top-level data structure PrimClex.
   *
   *  @{
  */

  /// \brief Read/modify settings of an already existing CASM project
  ///
  /// - Use ProjectBuilder to create a new CASM project
  /// - Only allows modifying settings if the appropriate directories exist
  ///
  class ProjectSettings : public Logging {

  public:

    /// \brief Default constructor
    ProjectSettings();

    /// \brief Construct CASM project settings for a new project
    ///
    /// \param root Path to new CASM project directory
    /// \param name Name of new CASM project. Use a short title suitable for prepending to file names.
    ///
    explicit ProjectSettings(fs::path root, std::string name, const Logging &logging = Logging());

    /// \brief Construct CASM project settings from existing project
    ///
    /// \param root Path to existing CASM project directory. Project settings will be read.
    ///
    explicit ProjectSettings(fs::path root, const Logging &logging = Logging());

    ~ProjectSettings();


    /// \brief Get project name
    std::string name() const;

    const DirectoryStructure &dir() const {
      return m_dir;
    }

    /// \brief const Access current properties required for a ConfigType to be considered calculated
    template<typename DataObject>
    const std::vector<std::string> &properties() const;


    const std::map<std::string, ClexDescription> &cluster_expansions() const;

    bool has_clex(std::string name) const;

    const ClexDescription &clex(std::string name) const;

    const ClexDescription &default_clex() const;

    bool new_clex(const ClexDescription &desc);

    bool erase_clex(const ClexDescription &desc);

    bool set_default_clex(const std::string &clex_name);

    bool set_default_clex(const ClexDescription &desc);


    /// \brief Get neighbor list weight matrix
    Eigen::Matrix3l nlist_weight_matrix() const;

    /// \brief Get set of sublattice indices to include in neighbor lists
    const std::set<int> &nlist_sublat_indices() const;

    /// \brief Get c++ compiler
    std::pair<std::string, std::string> cxx() const;

    /// \brief Get c++ compiler options
    std::pair<std::string, std::string> cxxflags() const;

    /// \brief Get shared object options
    std::pair<std::string, std::string> soflags() const;

    /// \brief Get casm includedir
    std::pair<fs::path, std::string> casm_includedir() const;

    /// \brief Get casm libdir
    std::pair<fs::path, std::string> casm_libdir() const;

    /// \brief Get boost includedir
    std::pair<fs::path, std::string> boost_includedir() const;

    /// \brief Get boost libdir
    std::pair<fs::path, std::string> boost_libdir() const;

    /// \brief Get current compilation options string
    std::string compile_options() const;

    /// \brief Get current shared library options string
    std::string so_options() const;

    /// \brief Get current command used by 'casm view'
    std::string view_command() const;

    /// \brief Get current video viewing command used by 'casm view'
    std::string view_command_video() const;

    /// \brief Get current project crystallography tolerance
    double crystallography_tol() const;

    /// \brief Get current project linear algebra tolerance
    double lin_alg_tol() const;


    // ** Enumerators **

    EnumeratorHandler &enumerator_handler();

    const EnumeratorHandler &enumerator_handler() const;


    // ** Database **

    void set_db_name(std::string _db_name);

    std::string db_name() const;


    // ** Queries **

    template<typename DataObject>
    QueryHandler<DataObject> &query_handler();

    template<typename DataObject>
    const QueryHandler<DataObject> &query_handler() const;


    // ** Hamiltonian Modules **

    HamiltonianModules &hamiltonian_modules();

    HamiltonianModules const &hamiltonian_modules()const;

    // ** Clexulator names **

    std::string global_clexulator_name() const;


    // ** Add directories for additional project data **

    /// \brief Create new project data directory
    bool new_casm_dir() const;

    /// \brief Create new symmetry directory
    bool new_symmetry_dir() const;

    /// \brief Create new reports directory
    bool new_reports_dir() const;

    /// \brief Add a basis set directory
    bool new_bset_dir(std::string bset) const;

    /// \brief Add a cluster expansion directory
    bool new_clex_dir(std::string clex) const;


    /// \brief Add calculation settings directory path
    bool new_calc_settings_dir(std::string calctype) const;

    /// \brief Add calculation settings directory path, for supercell specific settings
    bool new_supercell_calc_settings_dir(std::string scelname, std::string calctype) const;

    /// \brief Add calculation settings directory path, for configuration specific settings
    bool new_configuration_calc_settings_dir(std::string configname, std::string calctype) const;


    /// \brief Add a ref directory
    bool new_ref_dir(std::string calctype, std::string ref) const;


    /// \brief Add an eci directory
    bool new_eci_dir(std::string clex, std::string calctype, std::string ref, std::string bset, std::string eci) const;


    // ** Change current settings **

    /// \brief Access current properties required for a ConfigType to be considered calculated
    template<typename DataObject>
    std::vector<std::string> &properties();


    /// \brief Set neighbor list weight matrix (will delete existing Clexulator
    /// source and compiled code)
    bool set_nlist_weight_matrix(Eigen::Matrix3l M);

    /// \brief Set range of sublattice indices to include in neighbor lists (will
    /// delete existing Clexulator source and compiled code)
    template<typename SublatIterator>
    bool set_nlist_sublat_indices(SublatIterator begin, SublatIterator end);


    /// \brief Set c++ compiler (empty string to use default)
    bool set_cxx(std::string opt);

    /// \brief Set c++ compiler options (empty string to use default)
    bool set_cxxflags(std::string opt);

    /// \brief Set shared object options (empty string to use default)
    bool set_soflags(std::string opt);


    /// \brief Set casm prefix (empty string to use default)
    bool set_casm_prefix(fs::path dir);

    /// \brief Set casm includedir (empty string to use default)
    bool set_casm_includedir(fs::path dir);

    /// \brief Set casm libdir (empty string to use default)
    bool set_casm_libdir(fs::path dir);


    /// \brief Set boost prefix (empty string to use default)
    bool set_boost_prefix(fs::path dir);

    /// \brief Set boost includedir (empty string to use default)
    bool set_boost_includedir(fs::path dir);

    /// \brief Set boost libdir (empty string to use default)
    bool set_boost_libdir(fs::path dir);


    /// \brief Set command used by 'casm view'
    bool set_view_command(std::string opt);

    /// \brief Set video viewing command used by 'casm view'
    bool set_view_command_video(std::string opt);

    /// \brief Set crystallography tolerance
    bool set_crystallography_tol(double _tol);

    /// \brief Set linear algebra tolerance
    bool set_lin_alg_tol(double _tol);


    /// \brief Save settings to project settings file
    void commit() const;

    /// \brief Output as JSON
    jsonParser &to_json(jsonParser &json) const;

    /// \brief Print summary of compiler settings, as for 'casm settings -l'
    void print_compiler_settings_summary(Log &log) const;

    /// \brief Print summary of ProjectSettings, as for 'casm settings -l'
    void print_summary(Log &log) const;

    /// \brief (deprecated) Set compile options to 'opt' (empty string to use default)
    bool set_compile_options(std::string opt);

    /// \brief (deprecated) Set shared library options to 'opt' (empty string to use default)
    bool set_so_options(std::string opt);



  private:

    /// \brief Changing the neighbor list properties requires updating Clexulator source code
    void _reset_clexulators();

    /// \brief initialize default compiler options
    void _load_default_options();


    DirectoryStructure m_dir;

    std::string m_name;

    notstd::cloneable_ptr<EnumeratorHandler> m_enumerator_handler;

    // Datatype name : QueryHandler<DataType> map (type erased)
    std::map<std::string, notstd::cloneable_ptr<notstd::Cloneable> > m_query_handler;

    mutable notstd::cloneable_ptr<HamiltonianModules> m_hamiltonian_modules;

    // CASM project current settings

    // name : ClexDescription map
    std::map<std::string, ClexDescription> m_clex;

    // name of default cluster expansion
    std::string m_default_clex;

    // neighbor list settings
    Eigen::Matrix3l m_nlist_weight_matrix;
    std::set<int> m_nlist_sublat_indices;

    // Properties required to be read from calculations
    // ConfigType::name -> {prop1, prop2, ...}
    std::map<std::string, std::vector<std::string> > m_properties;

    // Runtime library compilation settings: compilation options
    std::pair<std::string, std::string> m_cxx;
    std::pair<std::string, std::string> m_cxxflags;
    std::pair<std::string, std::string> m_soflags;
    std::pair<fs::path, std::string> m_casm_includedir;
    std::pair<fs::path, std::string> m_casm_libdir;
    std::pair<fs::path, std::string> m_boost_includedir;
    std::pair<fs::path, std::string> m_boost_libdir;

    // deprecated reading exactly from settings file
    std::string m_depr_compile_options;
    // deprecated reading exactly from settings file
    std::string m_depr_so_options;

    // Command executed by 'casm view'
    std::string m_view_command;

    // Command executed by 'casm view'
    std::string m_view_command_video;

    // Crystallography tolerance
    double m_crystallography_tol;

    // Linear algebra tolerance
    double m_lin_alg_tol;

    // Database
    std::string m_db_name;
  };

  jsonParser &to_json(const ProjectSettings &set, jsonParser &json);


  /// \brief Set range of sublattice indices to include in neighbor lists (will
  /// delete existing Clexulator source and compiled code)
  template<typename SublatIterator>
  bool ProjectSettings::set_nlist_sublat_indices(SublatIterator begin, SublatIterator end) {
    _reset_clexulators();
    m_nlist_sublat_indices = std::set<int>(begin, end);
    return true;
  }

  /** @} */
}

#endif
