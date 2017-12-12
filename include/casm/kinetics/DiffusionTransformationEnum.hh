#ifndef CASM_DiffusionTransformationEnum
#define CASM_DiffusionTransformationEnum

#include "casm/container/Counter.hh"
#include "casm/container/InputEnumerator.hh"
#include "casm/kinetics/DiffusionTransformation.hh"
#include "casm/symmetry/OrbitGeneration.hh"
#include "casm/misc/cloneable_ptr.hh"

#include "casm/casm_io/InputParser_impl.hh"
#include "casm/clusterography/ClusterSpecsParser_impl.hh"
#include "casm/casm_io/json_io/SpeciesSetParser_impl.hh"

namespace CASM {

  namespace Kinetics {

    class DiffTransEnumParser : public InputParser {

    public:

      DiffTransEnumParser(
        const PrimClex &_primclex,
        jsonParser &_input,
        fs::path _path,
        bool _required);

      std::set<std::string> required_species() const;

      std::set<std::string> excluded_species() const;

      bool dry_run() const;

      COORD_TYPE coordinate_mode() const;

      ORBIT_PRINT_MODE orbit_print_mode() const;

      const PrimPeriodicClustersByMaxLength &cspecs() const;

    private:
      const PrimClex &m_primclex;
      std::shared_ptr<PrimPeriodicClustersByMaxLength> m_cspecs_parser;
      std::shared_ptr<SpeciesSetParser> m_require;
      std::shared_ptr<SpeciesSetParser> m_exclude;
    };


    /// \brief Enumerate DiffusionTransformation for a particular IntegralCluster
    ///
    /// - InputEnumerator
    /// - Outputs all valid DiffusionTransformation for a particular
    ///   IntegralCluster. Results may include duplicates, non-canonical, unsorted forms.
    /// -
    /// - To get unique orbits, see for example make_prim_periodic_diff_trans_orbits
    ///
    class DiffusionTransformationEnum : public InputEnumeratorBase<DiffusionTransformation> {

      // -- Required members -------------------

    public:

      /// \brief Construct with an IntegralCluster
      DiffusionTransformationEnum(const IntegralCluster &clust);

      std::string name() const override {
        return enumerator_name;
      }

      static const std::string enumerator_name;
      static const std::string interface_help;

      /// Implements run
      template<typename DatabaseType>
      static int run(
        const PrimClex &primclex,
        const jsonParser &_kwargs,
        const Completer::EnumOption &enum_opt,
        DatabaseType &db);

      /// Implements run
      static int run(const PrimClex &primclex, const jsonParser &_kwargs, const Completer::EnumOption &enum_opt);

    private:


      /// Implements increment
      void increment() override;



      // -- Unique -------------------

      const Structure &prim() const;

      const IntegralCluster &cluster() const;

      /// \brief The occ_counter contains the from/to occupation values for each site
      void _init_occ_counter();

      /// \brief Returns container of 'from' species locations
      std::vector<SpeciesLocation> _init_from_loc(const std::vector<Index> &occ_values);

      /// \brief Returns container of 'to' species locations
      std::vector<SpeciesLocation> _init_to_loc(const std::vector<Index> &occ_values);

      /// \brief Returns container of 'from' or 'to' species locations
      std::vector<SpeciesLocation> _init_loc(const std::vector<Index> &occ_values, Index offset);

      /// \brief Uses m_cluster, m_occ_counter, m_from_loc, and m_to_loc to set m_current
      void _set_current();
      void _update_current_occ_transform();
      void _set_current_loc();
      void _update_current_to_loc();


      Counter<std::vector<Index> > m_occ_counter;
      std::vector<SpeciesLocation> m_from_loc;
      std::vector<SpeciesLocation> m_to_loc;

      IntegralCluster m_cluster;
      notstd::cloneable_ptr<DiffusionTransformation> m_current;
    };

    template<typename OrbitOutputIterator, typename IntegralClusterOrbitInputIterator>
    OrbitOutputIterator make_prim_periodic_diff_trans_orbits(
      IntegralClusterOrbitInputIterator begin,
      IntegralClusterOrbitInputIterator end,
      double xtal_tol,
      OrbitOutputIterator result,
      const PrimClex *primclex);
  }
}

#endif
