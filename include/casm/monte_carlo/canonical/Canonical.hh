#ifndef CASM_Canonical_HH
#define CASM_Canonical_HH

#include "casm/clex/Clex.hh"
#include "casm/enumerator/OrderParameter.hh"
#include "casm/monte_carlo/Conversions.hh"
#include "casm/monte_carlo/MonteCarlo.hh"
#include "casm/monte_carlo/MonteDefinitions.hh"
#include "casm/monte_carlo/OccCandidate.hh"
#include "casm/monte_carlo/OccLocation.hh"
#include "casm/monte_carlo/canonical/CanonicalConditions.hh"
#include "casm/monte_carlo/canonical/CanonicalEvent.hh"
#include "casm/monte_carlo/canonical/CanonicalSettings.hh"

namespace CASM {
namespace Monte {

///
/// Derives from base MonteCarlo class, to be used for simulations at constant
/// temperature and chemical potential.
///
/// As with all the other derived Monte Carlo classes, member functions must
/// follow a specific naming convention to be used with templated routines
/// currently defined in MonteDriver.hh:
///      -conditions
///      -set_conditions
///      -propose
///      -check
///      -accept
///      -reject
///      -write_results
///
class Canonical : public MonteCarlo {
 public:
  static const ENSEMBLE ensemble;
  typedef CanonicalEvent EventType;
  typedef CanonicalConditions CondType;
  typedef CanonicalSettings SettingsType;

  /// \brief Constructs a Canonical object and prepares it for running based on
  /// Settings
  Canonical(const PrimClex &primclex, const SettingsType &settings, Log &_log);

  /// \brief Return number of steps per pass. Equals number of sites with
  /// variable occupation.
  size_type steps_per_pass() const;

  /// \brief Return current conditions
  const CondType &conditions() const;

  /// \brief Set conditions and clear previously collected data
  void set_conditions(const CondType &new_conditions);

  /// \brief Set configdof and clear previously collected data
  void set_configdof(const ConfigDoF &configdof, const std::string &msg = "");

  /// \brief Set configdof and conditions and clear previously collected data
  std::pair<ConfigDoF, std::string> set_state(
      const CanonicalConditions &new_conditions,
      const CanonicalSettings &settings);

  /// \brief Set configdof and conditions and clear previously collected data
  void set_state(const CondType &new_conditions, const ConfigDoF &configdof,
                 const std::string &msg = "");

  /// \brief Propose a new event, calculate delta properties, and return
  /// reference to it
  const EventType &propose();

  /// \brief Based on a random number, decide if the change in energy from the
  /// proposed event is low enough to be accepted.
  bool check(const EventType &event);

  /// \brief Accept proposed event. Change configuration accordingly and update
  /// energies etc.
  void accept(const EventType &event);

  /// \brief Nothing needs to be done to reject a CanonicalEvent
  void reject(const EventType &event);

  /// \brief Write results to files
  void write_results(size_type cond_index) const;

  /// \brief Formation energy, normalized per primitive cell
  const double &formation_energy() const { return *m_formation_energy; }

  /// \brief Potential energy, normalized per primitive cell
  const double &potential_energy() const { return *m_potential_energy; }

  /// \brief Correlations, normalized per primitive cell
  const Eigen::VectorXd &corr() const { return *m_corr; }

  /// \brief Number of atoms of each type, normalized per primitive cell
  const Eigen::VectorXd &comp_n() const { return *m_comp_n; }

  /// \brief Order parameters (intensive)
  const Eigen::VectorXd &eta() const { return *m_eta; }

  /// \brief Get potential energy
  double potential_energy(const Configuration &config) const;

  Clexulator const &clexulator() const {
    return m_formation_energy_clex.clexulator;
  }

  /// \brief Get the order parameter calculator (must be copied to be used)
  std::shared_ptr<OrderParameter const> order_parameter() const {
    return m_order_parameter;
  }

  /// \brief Get the random alloy correlation calculator
  std::shared_ptr<RandomAlloyCorrCalculator> random_alloy_corr_f() const {
    return m_random_alloy_corr_f;
  }

 private:
  /// \brief Formation energy, normalized per primitive cell
  double &_formation_energy() { return *m_formation_energy; }

  /// \brief Potential energy, normalized per primitive cell
  double &_potential_energy() { return *m_potential_energy; }

  /// \brief Correlations, normalized per primitive cell
  Eigen::VectorXd &_corr() { return *m_corr; }

  /// \brief Number of atoms of each type, normalized per primitive cell
  Eigen::VectorXd &_comp_n() { return *m_comp_n; }

  /// \brief Order parameters (intensive)
  Eigen::VectorXd &_eta() { return *m_eta; }

  Clexulator const &_clexulator() const {
    return m_formation_energy_clex.clexulator;
  }

  const ECIContainer &_eci() const { return m_formation_energy_clex.eci; }

  void _calc_delta_point_corr(Eigen::VectorXd &dcorr, size_type l,
                              int new_occ) const;

  /// \brief Calculate delta correlations for an event
  void _set_dCorr(CanonicalEvent &event) const;

  /// \brief Print correlations to _log()
  void _print_correlations(const Eigen::VectorXd &corr, std::string title,
                           std::string colheader) const;

  /// \brief Calculate delta properties for an event and update the event with
  /// those properties
  void _update_deltas(CanonicalEvent &event) const;

  /// \brief Calculate properties given current conditions
  void _update_properties();

  /// \brief Generate supercell filling ConfigDoF from default configuration
  ConfigDoF _default_motif() const;

  /// \brief Generate minimum potential energy ConfigDoF
  std::pair<ConfigDoF, std::string> _auto_motif(
      const CanonicalConditions &cond) const;

  /// \brief Generate minimum potential energy ConfigDoF for this supercell
  std::pair<ConfigDoF, std::string> _restricted_auto_motif(
      const CanonicalConditions &cond) const;

  /// \brief Generate supercell filling ConfigDoF from configuration
  ConfigDoF _configname_motif(const std::string &configname) const;

  /// \brief Construct m_candidate,  m_cand_to_index, m_occ_loc,
  /// m_canonical_swaps, m_grand_canonical_swaps
  void _make_possible_swaps(const CanonicalSettings &settings);

  /// \brief Find a OccSwap to help enforce composition
  std::vector<OccSwap>::const_iterator _find_grand_canonical_swap(
      const Configuration &config, std::vector<OccSwap>::const_iterator begin,
      std::vector<OccSwap>::const_iterator end);

  /// \brief Enforce composition by repeatedly applying grand canonical events
  ConfigDoF _enforce_conditions(const ConfigDoF &configdof);

  /// Holds Clexulator and ECI references
  Clex m_formation_energy_clex;

  /// Holds order parameter calculator
  std::shared_ptr<OrderParameter> m_order_parameter;

  /// Holds random alloy corr calculator
  std::shared_ptr<RandomAlloyCorrCalculator> m_random_alloy_corr_f;

  /// Convert sublat/asym_unit and species/occ index
  Conversions m_convert;

  /// Convert sublat/asym_unit and species/occ index
  OccCandidateList m_cand;

  /// Keeps track of what sites have which occupants
  OccLocation m_occ_loc;

  /// Conditions (T, mu). Initially determined by m_settings, but can be changed
  /// halfway through the run
  CanonicalConditions m_condition;

  /// Event to propose, check, accept/reject:
  CanonicalEvent m_event;

  // ---- Pointers to properties for faster access

  /// \brief Formation energy, normalized per primitive cell
  double *m_formation_energy;

  /// \brief Potential energy, normalized per primitive cell
  double *m_potential_energy;

  /// \brief Correlations, normalized per primitive cell
  Eigen::VectorXd *m_corr;

  /// \brief Number of atoms of each type, normalized per primitive cell
  Eigen::VectorXd *m_comp_n;

  /// \brief Order parameters (intensive)
  Eigen::VectorXd *m_eta;
};

}  // namespace Monte
}  // namespace CASM

#endif
