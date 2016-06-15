#include "casm/app/ProjectSettings.hh"

#include "casm/app/AppIO.hh"
#include "casm/crystallography/Structure.hh"
#include "casm/clex/NeighborList.hh"

#include "casm/clex/ConfigIOSelected.hh"
#include "casm/clex/ConfigSelection.hh"

namespace CASM {

  /// \brief Default weight matrix for approximately spherical neighborhood in Cartesian coordinates
  ///
  /// Equivalent to:
  /// \code
  /// PrimNeighborList::make_weight_matrix(prim.lattice().lat_column_mat(), 10, tol());
  /// \endcode
  Eigen::Matrix3l _default_nlist_weight_matrix(const Structure &prim, double tol) {
    return PrimNeighborList::make_weight_matrix(prim.lattice().lat_column_mat(), 10, tol);
  }

  /// \brief Default includes sublattices with >= 2 components
  std::set<int> _default_nlist_sublat_indices(const Structure &prim) {
    // for now, include sublattices with >= 2 components
    std::set<int> sublat_indices;
    for(int b = 0; b < prim.basis.size(); ++b) {
      if(prim.basis[b].site_occupant().size() >= 2) {
        sublat_indices.insert(b);
      }
    }
    return sublat_indices;
  }


  /// \brief Construct CASM project settings for a new project
  ///
  /// \param root Path to new CASM project directory
  /// \param name Name of new CASM project. Use a short title suitable for prepending to file names.
  ///
  ProjectSettings::ProjectSettings(fs::path root, std::string name) :
    m_dir(root),
    m_name(name) {

    m_compile_options = RuntimeLibrary::default_compile_options();
    m_so_options = RuntimeLibrary::default_so_options();

    if(fs::exists(m_dir.casm_dir())) {
      throw std::runtime_error(
        std::string("Error in 'ProjectSettings(fs::path root, std::string name)'.\n") +
        "  A CASM project already exists at this location: '" + root.string() + "'");
    }

    // check for a prim.json
    if(!fs::is_regular_file(m_dir.prim())) {
      throw std::runtime_error(
        std::string("Error in 'ProjectSettings(fs::path root, std::string name)'.\n") +
        "  No prim.json file found at: " + m_dir.prim().string());
    }

    // generate default nlist settings
    Structure prim(read_prim(m_dir.prim()));
    m_nlist_weight_matrix = _default_nlist_weight_matrix(prim, TOL);
    m_nlist_sublat_indices = _default_nlist_sublat_indices(prim);

    // load ConfigIO
    m_config_io_dict = make_dictionary<Configuration>();

    // default 'selected' uses MASTER
    set_selected(ConfigIO::Selected());
  }

  /// \brief Construct CASM project settings from existing project
  ///
  /// \param root Path to existing CASM project directory. Project settings will be read.
  ///
  ProjectSettings::ProjectSettings(fs::path root) :
    m_dir(root) {

    if(fs::exists(m_dir.casm_dir())) {

      m_compile_options = RuntimeLibrary::default_compile_options();
      m_so_options = RuntimeLibrary::default_so_options();

      try {

        // read .casmroot current settings
        fs::ifstream file(m_dir.project_settings());
        jsonParser settings(file);

        from_json(m_properties, settings["curr_properties"]);
        from_json(m_bset, settings["curr_bset"]);
        from_json(m_calctype, settings["curr_calctype"]);
        from_json(m_ref, settings["curr_ref"]);
        from_json(m_clex, settings["curr_clex"]);
        from_json(m_eci, settings["curr_eci"]);

        if(settings.contains("compile_options")) {
          settings["compile_options"].get(m_compile_options);
        }
        if(settings.contains("so_options")) {
          settings["so_options"].get(m_so_options);
        }

        settings.get_if(m_view_command, "view_command");
        from_json(m_name, settings["name"]);

        settings.get_else(m_crystallography_tol, "tol", TOL);
        settings.get_if(m_crystallography_tol, "crystallography_tol");

        settings.get_else(m_lin_alg_tol, "lin_alg_tol", 1e-10);

        // read nlist settings, or generate defaults
        Structure prim;
        bool and_commit = false;
        if(!settings.contains("nlist_weight_matrix") || !settings.contains("nlist_sublat_indices")) {
          _reset_clexulators();
          prim = Structure(read_prim(m_dir.prim()));
          and_commit = true;
        }

        if(settings.contains("nlist_weight_matrix")) {
          from_json(m_nlist_weight_matrix, settings["nlist_weight_matrix"]);
        }
        else {
          m_nlist_weight_matrix = _default_nlist_weight_matrix(prim, crystallography_tol());
        }

        if(settings.contains("nlist_sublat_indices")) {
          from_json(m_nlist_sublat_indices, settings["nlist_sublat_indices"]);
        }
        else {
          m_nlist_sublat_indices = _default_nlist_sublat_indices(prim);
        }

        // load ConfigIO
        m_config_io_dict = make_dictionary<Configuration>();

        // default 'selected' uses MASTER
        set_selected(ConfigIO::Selected());

        // migrate existing query_alias from deprecated 'query_alias.json'
        jsonParser &alias_json = settings["query_alias"];
        if(fs::exists(m_dir.query_alias())) {
          jsonParser depr(m_dir.query_alias());
          for(auto it = depr.begin(); it != depr.end(); ++it) {
            if(!alias_json.contains(it.name())) {
              alias_json[it.name()] = it->get<std::string>();
              and_commit = true;
            }
          }
        }

        // add aliases to dictionary
        for(auto it = alias_json.begin(); it != alias_json.end(); ++it) {
          add_alias(it.name(), it->get<std::string>(), std::cerr);
        }

        if(and_commit) {
          commit();
        }

      }
      catch(std::exception &e) {
        std::cerr << "Error in ProjectSettings::ProjectSettings(const fs::path root).\n" <<
                  "  Error reading " << m_dir.project_settings() << std::endl;
        throw e;
      }
    }
    else if(!fs::exists(m_dir.casm_dir())) {
      throw std::runtime_error(
        std::string("Error in 'ProjectSettings(fs::path root, std::string name)'.\n") +
        "  A CASM project does not exist at this location: '" + root.string() + "'");
    }

  }


  /// \brief Get project name
  std::string ProjectSettings::name() const {
    return m_name;
  }

  /// \brief Get current properties
  const std::vector<std::string> &ProjectSettings::properties() const {
    return m_properties;
  }

  /// \brief Get current basis set name
  std::string ProjectSettings::bset() const {
    return m_bset;
  }

  /// \brief Get current calctype name
  std::string ProjectSettings::calctype() const {
    return m_calctype;
  }

  /// \brief Get current ref name
  std::string ProjectSettings::ref() const {
    return m_ref;
  }

  /// \brief Get current cluster expansion name
  std::string ProjectSettings::clex() const {
    return m_clex;
  }

  /// \brief Get current eci name
  std::string ProjectSettings::eci() const {
    return m_eci;
  }

  /// \brief Get neighbor list weight matrix
  Eigen::Matrix3l ProjectSettings::nlist_weight_matrix() const {
    return m_nlist_weight_matrix;
  }

  /// \brief Get set of sublattice indices to include in neighbor lists
  const std::set<int> &ProjectSettings::nlist_sublat_indices() const {
    return m_nlist_sublat_indices;
  }

  /// \brief Get current compilation options string
  std::string ProjectSettings::compile_options() const {
    return m_compile_options;
  }

  /// \brief Get current shared library options string
  std::string ProjectSettings::so_options() const {
    return m_so_options;
  }

  /// \brief Get current command used by 'casm view'
  std::string ProjectSettings::view_command() const {
    return m_view_command;
  }

  /// \brief Get current project crystallography tolerance
  double ProjectSettings::crystallography_tol() const {
    return m_crystallography_tol;
  }

  /// \brief Get current project linear algebra tolerance
  double ProjectSettings::lin_alg_tol() const {
    return m_lin_alg_tol;
  }

  // ** Configuration properties **

  const DataFormatterDictionary<Configuration> &ProjectSettings::config_io() const {
    return m_config_io_dict;
  }

  /// \brief Set the selection to be used for the 'selected' column
  void ProjectSettings::set_selected(const ConfigIO::Selected &selection) {
    if(m_config_io_dict.find("selected") != m_config_io_dict.end()) {
      m_config_io_dict.erase("selected");
    }
    m_config_io_dict.insert(
      datum_formatter_alias(
        "selected",
        selection,
        "Returns true if configuration is specified in the input selection"
      )
    );
  }

  /// \brief Set the selection to be used for the 'selected' column
  void ProjectSettings::set_selected(const ConstConfigSelection &selection) {
    set_selected(ConfigIO::selected_in(selection));
  }

  /// \brief Add user-defined query alias
  void ProjectSettings::add_alias(const std::string &alias_name,
                                  const std::string &alias_command,
                                  std::ostream &serr) {

    auto new_formatter =  datum_formatter_alias<Configuration>(alias_name, alias_command, m_config_io_dict);
    auto key = m_config_io_dict.key(new_formatter);

    // if not in dictionary (includes operator dictionary), add
    if(m_config_io_dict.find(key) == m_config_io_dict.end()) {
      m_config_io_dict.insert(new_formatter);
    }
    // if a user-created alias, over-write with message
    else if(m_aliases.find(alias_name) != m_aliases.end()) {
      serr << "WARNING: I already know '" << alias_name << "' as:\n"
           << "             " << m_aliases[alias_name] << "\n"
           << "         I will forget it and learn '" << alias_name << "' as:\n"
           << "             " << alias_command << std::endl;
      m_config_io_dict.insert(new_formatter);
    }
    // else do not add, throw error
    else {
      std::stringstream ss;
      ss << "Error: Attempted to over-write standard CASM query name with user alias.\n";
      throw std::runtime_error(ss.str());
    }

    // save alias
    m_aliases[alias_name] = alias_command;

  }

  /// \brief Return map containing aliases
  ///
  /// - key: alias name
  /// - value: alias command
  const std::map<std::string, std::string> &ProjectSettings::aliases() const {
    return m_aliases;
  }


  // ** Clexulator names **

  std::string ProjectSettings::global_clexulator() const {
    return name() + "_Clexulator";
  }


  // ** Add directories for additional project data **

  /// \brief Create new project data directory
  bool ProjectSettings::new_casm_dir() const {
    return fs::create_directory(m_dir.casm_dir());
  }

  /// \brief Create new symmetry directory
  bool ProjectSettings::new_symmetry_dir() const {
    return fs::create_directory(m_dir.symmetry_dir());
  }

  /// \brief Add a basis set directory
  bool ProjectSettings::new_bset_dir(std::string bset) const {
    return fs::create_directories(m_dir.bset_dir(bset));
  }

  /// \brief Add a cluster expansion directory
  bool ProjectSettings::new_clex_dir(std::string clex) const {
    return fs::create_directories(m_dir.clex_dir(clex));
  }


  /// \brief Add calculation settings directory path
  bool ProjectSettings::new_calc_settings_dir(std::string calctype) const {
    return fs::create_directories(m_dir.calc_settings_dir(calctype));
  }

  /// \brief Add calculation settings directory path, for supercell specific settings
  bool ProjectSettings::new_supercell_calc_settings_dir(std::string scelname, std::string calctype) const {
    return fs::create_directories(m_dir.supercell_calc_settings_dir(scelname, calctype));
  }

  /// \brief Add calculation settings directory path, for configuration specific settings
  bool ProjectSettings::new_configuration_calc_settings_dir(std::string configname, std::string calctype) const {
    return fs::create_directories(m_dir.configuration_calc_settings_dir(configname, calctype));
  }


  /// \brief Add a ref directory
  bool ProjectSettings::new_ref_dir(std::string calctype, std::string ref) const {
    return fs::create_directories(m_dir.ref_dir(calctype, ref));
  }

  /// \brief Add an eci directory
  bool ProjectSettings::new_eci_dir(std::string clex, std::string calctype, std::string ref, std::string bset, std::string eci) const {
    return fs::create_directories(m_dir.eci_dir(clex, calctype, ref, bset, eci));
  }


  // ** Change current settings **

  /// \brief Access current properties
  std::vector<std::string> &ProjectSettings::properties() {
    return m_properties;
  }

  /// \brief Set current basis set to 'bset', if 'bset' exists
  bool ProjectSettings::set_bset(std::string bset) {
    auto all = m_dir.all_bset();
    if(std::find(all.begin(), all.end(), bset) != all.end()) {
      m_bset = bset;
      return true;
    }
    return false;
  }

  /// \brief Set current calctype to 'calctype', if 'calctype' exists
  bool ProjectSettings::set_calctype(std::string calctype) {
    auto all = m_dir.all_calctype();
    if(std::find(all.begin(), all.end(), calctype) != all.end()) {
      m_calctype = calctype;
      return true;
    }
    return false;
  }

  /// \brief Set current calculation reference to 'ref', if 'ref' exists
  bool ProjectSettings::set_ref(std::string calctype, std::string ref) {
    auto all = m_dir.all_ref(calctype);
    if(std::find(all.begin(), all.end(), ref) != all.end()) {
      m_ref = ref;
      return true;
    }
    return false;
  }

  /// \brief Set current cluster expansion to 'clex', if 'clex' exists
  bool ProjectSettings::set_clex(std::string clex) {
    auto all = m_dir.all_clex();
    if(std::find(all.begin(), all.end(), clex) != all.end()) {
      m_clex = clex;
      return true;
    }
    return false;
  }

  /// \brief Set current eci to 'eci', if 'eci' exists
  bool ProjectSettings::set_eci(std::string clex, std::string calctype, std::string ref, std::string bset, std::string eci) {
    auto all = m_dir.all_eci(clex, calctype, ref, bset);
    if(std::find(all.begin(), all.end(), eci) != all.end()) {
      m_eci = eci;
      return true;
    }
    return false;
  }

  /// \brief Set neighbor list weight matrix (will delete existing Clexulator
  /// source and compiled code)
  bool ProjectSettings::set_nlist_weight_matrix(Eigen::Matrix3l M) {

    // changing the neighbor list properties requires updating Clexulator source code
    // for now we just remove existing source/compiled files
    _reset_clexulators();

    m_nlist_weight_matrix = M;
    return true;
  }

  /// \brief Set compile options to 'opt'
  bool ProjectSettings::set_compile_options(std::string opt) {
    m_compile_options = opt;
    return true;
  }

  /// \brief Set shared library options to 'opt'
  bool ProjectSettings::set_so_options(std::string opt) {
    m_so_options = opt;
    return true;
  }

  /// \brief Set command used by 'casm view'
  bool ProjectSettings::set_view_command(std::string opt) {
    m_view_command = opt;
    return true;
  }

  /// \brief Set crystallography tolerance
  bool ProjectSettings::set_crystallography_tol(double _tol) {
    m_crystallography_tol = _tol;
    return true;
  }

  /// \brief Set linear algebra tolerance
  bool ProjectSettings::set_lin_alg_tol(double _tol) {
    m_lin_alg_tol = _tol;
    return true;
  }

  /// \brief Save settings to file
  void ProjectSettings::commit() const {

    try {
      SafeOfstream file;
      file.open(m_dir.project_settings());

      jsonParser json;
      to_json(*this, json);

      json.print(file.ofstream(), 2, 18);
      file.close();
    }
    catch(...) {
      std::cerr << "Uncaught exception in ProjectSettings::commit()" << std::endl;
      /// re-throw exceptions
      throw;
    }

  }

  /// \brief Changing the neighbor list properties requires updating Clexulator
  /// source code. This will remove existing source/compiled files
  void ProjectSettings::_reset_clexulators() {
    auto all_bset = m_dir.all_bset();
    for(auto it = all_bset.begin(); it != all_bset.end(); ++it) {
      fs::remove(m_dir.clexulator_src(name(), *it));
      fs::remove(m_dir.clexulator_o(name(), *it));
      fs::remove(m_dir.clexulator_so(name(), *it));
    }
  }

  jsonParser &to_json(const ProjectSettings &set, jsonParser &json) {

    json = jsonParser::object();

    json["name"] = set.name();
    json["curr_properties"] = set.properties();
    json["curr_clex"] = set.clex();
    json["curr_calctype"] = set.calctype();
    json["curr_ref"] = set.ref();
    json["curr_bset"] = set.bset();
    json["curr_eci"] = set.eci();
    json["nlist_weight_matrix"] = set.nlist_weight_matrix();
    json["nlist_sublat_indices"] = set.nlist_sublat_indices();
    if(set.compile_options() != RuntimeLibrary::default_compile_options()) {
      json["compile_options"] = set.compile_options();
    }
    if(set.so_options() != RuntimeLibrary::default_so_options()) {
      json["so_options"] = set.so_options();
    }
    json["view_command"] = set.view_command();
    json["crystallography_tol"] = set.crystallography_tol();
    json["crystallography_tol"].set_scientific();
    json["lin_alg_tol"] = set.lin_alg_tol();
    json["lin_alg_tol"].set_scientific();
    json["query_alias"] = set.aliases();

    return json;
  }

}

