#include "casm/clex/ConfigIO.hh"

#include <functional>
#include "casm/clex/Configuration.hh"
#include "casm/clex/PrimClex.hh"
#include "casm/clex/Norm.hh"
#include "casm/clex/ConfigIOHull.hh"
#include "casm/clex/ConfigIONovelty.hh"
#include "casm/clex/ConfigIOStrucScore.hh"
#include "casm/clex/ConfigIOStrain.hh"
#include "casm/clex/ConfigIOSelected.hh"

namespace CASM {

  namespace ConfigIO_impl {

    /// \brief Expects arguments of the form 'name' or 'name(Au)', 'name(Pt)', etc.
    bool MolDependent::parse_args(const std::string &args) {
      if(args.size() > 0)
        m_mol_names.push_back(args);
      return true;
    }

    /// \brief Adds index rules corresponding to the parsed args
    void MolDependent::init(const Configuration &_tmplt) const {
      auto struc_molecule = _tmplt.get_primclex().get_prim().get_struc_molecule();

      if(m_mol_names.size() == 0) {
        for(Index i = 0; i < struc_molecule.size(); i++) {
          _add_rule(std::vector<Index>({i}));
          m_mol_names.push_back(struc_molecule[i].name);
        }
      }
      else {
        for(Index n = 0; n < m_mol_names.size(); n++) {
          Index i = 0;
          for(i = 0; i < struc_molecule.size(); i++) {
            if(struc_molecule[i].name == m_mol_names[n]) {
              _add_rule(std::vector<Index>({i}));
              break;
            }
          }
          if(i == struc_molecule.size())
            throw std::runtime_error(std::string("Format tag: '") + name() + "(" +
                                     m_mol_names[n] + ")' does not correspond to a viable composition.\n");
        }
      }
    }

    /// \brief Long header returns: 'name(Au)   name(Pt)   ...'
    std::string MolDependent::long_header(const Configuration &_tmplt) const {
      std::string t_header;
      for(Index c = 0; c < m_mol_names.size(); c++) {
        t_header += name() + "(" + m_mol_names[c] + ")";
        if(c != m_mol_names.size() - 1) {
          t_header += "   ";
        }
      }
      return t_header;
    }
  }

  namespace ConfigIO {

    // --- Comp implementations -----------

    const std::string Comp::Name = "comp";

    const std::string Comp::Desc =
      "Parametric composition parameters, individual label as argument. "
      "Without argument, all values are printed. Ex: comp(a), comp(b), etc.";

    /// \brief Returns the parametric composition
    Eigen::VectorXd Comp::evaluate(const Configuration &config) const {
      return comp(config);
    }

    /// \brief Returns true if the PrimClex has composition axes
    bool Comp::validate(const Configuration &config) const {
      return config.get_primclex().has_composition_axes();
    }

    /// \brief Expects arguments of the form 'comp(a)', 'comp(b)', etc.
    bool Comp::parse_args(const std::string &args) {
      if(args.size() == 1) {
        _add_rule(std::vector<Index>({(Index)(args[0] - 'a')}));
      }
      else if(args.size() > 1) {
        throw std::runtime_error(std::string("Format tag: 'comp(") + args + ") is invalid.\n");
        return false;
      }
      return true;
    }

    /// \brief Long header returns: 'comp(a)   comp(b)   ...'
    std::string Comp::long_header(const Configuration &_tmplt) const {
      std::string t_header;
      for(Index c = 0; c < _index_rules().size(); c++) {
        t_header += name() + "(";
        t_header.push_back((char)('a' + _index_rules()[c][0]));
        t_header.push_back(')');
        if(c != _index_rules().size() - 1) {
          t_header += "   ";
        }
      }
      return t_header;
    }


    // --- CompN implementations -----------

    const std::string CompN::Name = "comp_n";

    const std::string CompN::Desc =
      "Number of each species per unit cell, including vacancies. "
      "No argument prints all available values. Ex: comp_n, comp_n(Au), comp_n(Pt), etc.";

    /// \brief Returns the number of each species per unit cell
    Eigen::VectorXd CompN::evaluate(const Configuration &config) const {
      return comp_n(config);
    }


    // --- SiteFrac implementations -----------

    const std::string SiteFrac::Name = "site_frac";

    const std::string SiteFrac::Desc =
      "Fraction of sites occupied by a species, including vacancies. "
      "No argument prints all available values. Ex: site_frac(Au), site_frac(Pt), etc.";

    /// \brief Returns the site fraction
    Eigen::VectorXd SiteFrac::evaluate(const Configuration &config) const {
      return site_frac(config);
    }


    // --- AtomFrac implementations -----------

    const std::string AtomFrac::Name = "atom_frac";

    const std::string AtomFrac::Desc =
      "Fraction of atoms that are a particular species, excluding vacancies.  "
      "Without argument, all values are printed. Ex: atom_frac(Au), atom_frac(Pt), etc.";

    /// \brief Returns the site fraction
    Eigen::VectorXd AtomFrac::evaluate(const Configuration &config) const {
      return species_frac(config);
    }


    // --- Corr implementations -----------

    const std::string Corr::Name = "corr";

    const std::string Corr::Desc =
      "Average correlation values, normalized per primitive cell; "
      "accepts range as argument, for example corr(ind1:ind2)";

    /// \brief Returns the atom fraction
    Eigen::VectorXd Corr::evaluate(const Configuration &config) const {
      return correlations(config, m_clexulator);
    }

    /// \brief If not yet initialized, use the global clexulator from the PrimClex
    void Corr::init(const Configuration &_tmplt) const {
      if(!m_clexulator.initialized()) {
        m_clexulator = _tmplt.get_primclex().global_clexulator();
      }

      VectorXdAttribute<Configuration>::init(_tmplt);

    }

    // --- Clex implementations -----------

    const std::string Clex::Name = "clex";

    const std::string Clex::Desc =
      "Predicted property value, currently supports 'clex(formation_energy)' and "
      "'clex(formation_energy_per_species)'. Default is 'clex(formation_energy)'.";

    Clex::Clex() :
      ScalarAttribute<Configuration>(Name, Desc) {
      parse_args("");
    }

    Clex::Clex(const Clexulator &clexulator, const ECIContainer &eci, const std::string args) :
      ScalarAttribute<Configuration>(Name, Desc),
      m_clexulator(clexulator),
      m_eci(eci) {
      parse_args(args);
    }

    /// \brief Returns the atom fraction
    double Clex::evaluate(const Configuration &config) const {
      return m_eci * correlations(config, m_clexulator) / _norm(config);
    }

    /// \brief Clone using copy constructor
    std::unique_ptr<Clex> Clex::clone() const {
      return std::unique_ptr<Clex>(this->_clone());
    }

    /// \brief If not yet initialized, use the global clexulator and eci from the PrimClex
    void Clex::init(const Configuration &_tmplt) const {
      if(!m_clexulator.initialized()) {
        m_clexulator = _tmplt.get_primclex().global_clexulator();
        m_eci = _tmplt.get_primclex().global_eci("formation_energy");
      }
    }

    /// \brief Expects 'clex', 'clex(formation_energy)', or 'clex(formation_energy_per_species)'
    bool Clex::parse_args(const std::string &args) {
      m_clex_name = args;
      if(args == "") {
        m_norm = notstd::make_cloneable<Norm<Configuration> >();
        return true;
      }
      if(args == "formation_energy") {
        m_norm = notstd::make_cloneable<Norm<Configuration> >();
        return true;
      }
      if(args == "formation_energy_per_species") {
        m_norm = notstd::make_cloneable<NormPerSpecies>();
        return true;
      }
      else {
        std::stringstream ss;
        ss << "Error parsing arguments for 'clex'.\n"
           "  Received: " << args << "\n"
           "  Allowed options are: 'formation_energy' (Default), or 'formation_energy_per_species'";
        throw std::runtime_error(ss.str());
      }
    }

    /// \brief Returns the normalization
    double Clex::_norm(const Configuration &config) const {
      return (*m_norm)(config);
    }

    /// \brief Clone using copy constructor
    Clex *Clex::_clone() const {
      return new Clex(*this);
    }


    /*End ConfigIO*/
  }

  namespace ConfigIO {

    template<>
    Selected selected_in(const ConfigSelection<true> &_selection) {
      return Selected(_selection);
    }

    template<>
    Selected selected_in(const ConfigSelection<false> &_selection) {
      return Selected(_selection);
    }

    Selected selected_in() {
      return Selected();
    }


    GenericConfigFormatter<std::string> configname() {
      return GenericConfigFormatter<std::string>("configname",
                                                 "Configuration name, in the form 'SCEL#_#_#_#_#_#_#/#'",
      [](const Configuration & config)->std::string {
        return config.name();
      });
    }

    GenericConfigFormatter<std::string> scelname() {
      return GenericConfigFormatter<std::string>("scelname",
                                                 "Supercell name, in the form 'SCEL#_#_#_#_#_#_#'",
      [](const Configuration & config)->std::string {
        return config.get_supercell().get_name();
      });
    }



    GenericConfigFormatter<std::string> calc_status() {
      return GenericConfigFormatter<std::string>("calc_status",
                                                 "Status of calculation.",
                                                 CASM::calc_status,
                                                 CASM::has_calc_status);
    }

    GenericConfigFormatter<std::string> failure_type() {
      return GenericConfigFormatter<std::string>("failure_type",
                                                 "Reason for calculation failure.",
                                                 CASM::failure_type,
                                                 CASM::has_failure_type);
    }


    GenericConfigFormatter<Index> scel_size() {
      return GenericConfigFormatter<Index>("scel_size",
                                           "Supercell volume, given as the integer number of unit cells",
      [](const Configuration & config)->Index {
        return config.get_supercell().volume();
      });
    }

    GenericConfigFormatter<Index> multiplicity() {
      return GenericConfigFormatter<Index>("multiplicity",
                                           "Symmetric multiplicity of the configuration, excluding translational equivalents.",
      [](const Configuration & config)->Index {
        return config.get_prim().factor_group().size() / config.factor_group(config.get_supercell().permute_begin(), config.get_supercell().permute_end()).size();
      });
    }


    /*
    GenericConfigFormatter<bool> selected() {
      return GenericConfigFormatter<bool>("selected",
                                          "Specifies whether configuration is selected (1/true) or not (0/false)",
      [](const Configuration & config)->bool{
        return config.selected();
      });
    }
    */

    GenericConfigFormatter<double> relaxed_energy() {
      return GenericConfigFormatter<double>(
               "relaxed_energy",
               "DFT relaxed energy, normalized per primitive cell",
               CASM::relaxed_energy,
               has_relaxed_energy);
    }

    GenericConfigFormatter<double> relaxed_energy_per_species() {
      return GenericConfigFormatter<double>(
               "relaxed_energy_per_atom",
               "DFT relaxed energy, normalized per atom",
               CASM::relaxed_energy_per_species,
               has_relaxed_energy);
    }

    GenericConfigFormatter<double> reference_energy() {
      return GenericConfigFormatter<double>(
               "reference_energy",
               "reference energy, normalized per primitive cell, as determined by current reference states",
               CASM::reference_energy,
               has_reference_energy);
    }

    GenericConfigFormatter<double> reference_energy_per_species() {
      return GenericConfigFormatter<double>(
               "reference_energy_per_atom",
               "reference energy, normalized per atom, as determined by current reference states",
               CASM::reference_energy_per_species,
               has_reference_energy);
    }

    GenericConfigFormatter<double> formation_energy() {
      return GenericConfigFormatter<double>(
               "formation_energy",
               "DFT formation energy, normalized per primitive cell and measured "
               "relative to current reference states",
               CASM::formation_energy,
               has_formation_energy);
    }

    GenericConfigFormatter<double> formation_energy_per_species() {
      return GenericConfigFormatter<double>(
               "formation_energy_per_atom",
               "DFT formation energy, normalized per atom and measured relative to "
               "current reference states",
               CASM::formation_energy_per_species,
               has_formation_energy);
    }

    /*Generic1DDatumFormatter<std::vector<double>, Configuration >relaxation_strain() {
      return Generic1DDatumFormatter<std::vector<double>, Configuration >("relaxation_strain",
                                                                          "Green-Lagrange strain of dft-relaxed configuration, relative to the ideal crystal.  Ordered as [E(0,0), E(1,1), E(2,2), E(1,2), E(0,2), E(0,1)].  Accepts index as argument on interval [0,5]",
                                                                          CASM::relaxation_strain,
                                                                          has_relaxation_strain,
      [](const std::vector<double> &cont)->Index{
        return 6;
      });
      }*/

    GenericConfigFormatter<bool> is_calculated() {
      return GenericConfigFormatter<bool>("is_calculated",
                                          "True (1) if all current properties have been been calculated for the configuration",
                                          CASM::is_calculated);
    }

    GenericConfigFormatter<bool> is_primitive() {
      return GenericConfigFormatter<bool>("is_primitive",
                                          "True (1) if the configuration cannot be described within a smaller supercell",
                                          CASM::is_primitive);
    }

    GenericConfigFormatter<bool> is_canonical() {
      return GenericConfigFormatter<bool>("is_canonical",
                                          "True (1) if the configuration cannot be transfromed by symmetry to a configuration with higher lexicographic order",
                                          CASM::is_canonical);
    }

    GenericConfigFormatter<double> rms_force() {
      return GenericConfigFormatter<double>("rms_force",
                                            "Root-mean-square forces of relaxed configurations, determined from DFT (eV/Angstr.)",
                                            CASM::rms_force,
                                            has_rms_force);
    }

    GenericConfigFormatter<double> basis_deformation() {
      return GenericConfigFormatter<double>("basis_deformation",
                                            "Cost function that describes the degree to which basis sites have relaxed",
                                            CASM::basis_deformation,
                                            has_basis_deformation);
    }

    GenericConfigFormatter<double> lattice_deformation() {
      return GenericConfigFormatter<double>("lattice_deformation",
                                            "Cost function that describes the degree to which lattice has relaxed.",
                                            CASM::lattice_deformation,
                                            has_lattice_deformation);
    }

    GenericConfigFormatter<double> volume_relaxation() {
      return GenericConfigFormatter<double>("volume_relaxation",
                                            "Change in volume due to relaxation, expressed as the ratio V/V_0.",
                                            CASM::volume_relaxation,
                                            has_volume_relaxation);
    }

    /*End ConfigIO*/
  }

  template<>
  StringAttributeDictionary<Configuration> make_string_dictionary<Configuration>() {

    using namespace ConfigIO;
    StringAttributeDictionary<Configuration> dict;

    dict.insert(
      configname(),
      scelname(),
      calc_status(),
      failure_type()
    );

    return dict;
  }

  template<>
  BooleanAttributeDictionary<Configuration> make_boolean_dictionary<Configuration>() {

    using namespace ConfigIO;
    BooleanAttributeDictionary<Configuration> dict;

    dict.insert(
      is_calculated(),
      is_canonical(),
      is_primitive(),
      //selected(),
      selected_in(),
      OnClexHull(),
      OnHull()
    );

    return dict;
  }

  template<>
  IntegerAttributeDictionary<Configuration> make_integer_dictionary<Configuration>() {

    using namespace ConfigIO;
    IntegerAttributeDictionary<Configuration> dict;

    dict.insert(
      scel_size(),
      multiplicity()
    );

    return dict;
  }

  template<>
  ScalarAttributeDictionary<Configuration> make_scalar_dictionary<Configuration>() {

    using namespace ConfigIO;
    ScalarAttributeDictionary<Configuration> dict;

    dict.insert(
      Clex(),
      HullDist(),
      ClexHullDist(),
      Novelty(),
      relaxed_energy(),
      relaxed_energy_per_species(),
      reference_energy(),
      reference_energy_per_species(),
      formation_energy(),
      formation_energy_per_species(),
      rms_force(),
      basis_deformation(),
      lattice_deformation(),
      volume_relaxation()
    );

    return dict;
  }

  template<>
  VectorXdAttributeDictionary<Configuration> make_vectorxd_dictionary<Configuration>() {

    using namespace ConfigIO;
    VectorXdAttributeDictionary<Configuration> dict;

    dict.insert(
      AtomFrac(),
      Comp(),
      CompN(),
      Corr(),
      RelaxationStrain(),
      DoFStrain(),
      SiteFrac(),
      StrucScore()
    );

    return dict;
  }

}

