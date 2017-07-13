#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

/// What is being tested:
#include "casm/database/Selection.hh"
#include "casm/database/DatabaseDefs.hh"


/// What is being used to test it:

#include "Common.hh"
#include "FCCTernaryProj.hh"
#include "casm/crystallography/Structure.hh"
#include "casm/clex/ConfigEnumAllOccupations_impl.hh"
#include "casm/clex/ScelEnum.hh"
#include "casm/app/QueryHandler.hh"
#include "casm/container/Enumerator_impl.hh"
#include "casm/app/enum.hh"
#include "casm/kinetics/DiffusionTransformationEnum.hh"
#include "casm/kinetics/DiffTransConfigEnumPerturbations.hh"


using namespace CASM;

BOOST_AUTO_TEST_SUITE(Selection_Test)

BOOST_AUTO_TEST_CASE(Test1) {

  test::FCCTernaryProj proj;
  proj.check_init();

  PrimClex primclex(proj.dir, null_log());
  const Structure &prim(primclex.prim());
  primclex.settings().set_crystallography_tol(1e-5);

  fs::path difftrans_path = "tests/unit/kinetics/diff_trans.json";
  jsonParser diff_trans_json {difftrans_path};

  fs::path diffperturb_path = "tests/unit/kinetics/diff_perturb.json";
  jsonParser diff_perturb_json {diffperturb_path};
  Completer::EnumOption enum_opt;
  enum_opt.desc();

  ScelEnumByProps enum_scel(primclex, ScelEnumProps(1, 5));
  ConfigEnumAllOccupations::run(primclex, enum_scel.begin(), enum_scel.end());
  Kinetics::DiffusionTransformationEnum::run(primclex, diff_trans_json, enum_opt);
  Kinetics::DiffTransConfigEnumPerturbations::run(primclex, diff_perturb_json, enum_opt);
  BOOST_CHECK_EQUAL(primclex.db<Configuration>().size(), 126);

  DB::Selection<Configuration> selection(primclex);
  BOOST_CHECK_EQUAL(selection.size(), 126);
  BOOST_CHECK_EQUAL(selection.selected_size(), 0);

  auto &dict = primclex.settings().query_handler<Configuration>().dict();
  selection.set(dict, "lt(scel_size,3)");
  BOOST_CHECK_EQUAL(selection.selected_size(), 9);

  auto &dict2 = primclex.settings().query_handler<Supercell>().dict();
  BOOST_CHECK_EQUAL(primclex.db<Supercell>().size(), 13);
  DB::Selection<Supercell> selection2(primclex);
  BOOST_CHECK_EQUAL(selection2.size(), 13);
  BOOST_CHECK_EQUAL(selection2.selected_size(), 0);

  auto &dict3 = primclex.settings().query_handler<Kinetics::PrimPeriodicDiffTransOrbit>().dict();
  BOOST_CHECK_EQUAL(primclex.db<Kinetics::PrimPeriodicDiffTransOrbit>().size(), 28);
  DB::Selection<Kinetics::PrimPeriodicDiffTransOrbit> selection3(primclex);
  BOOST_CHECK_EQUAL(selection3.size(), 28);
  BOOST_CHECK_EQUAL(selection3.selected_size(), 0);

  auto &dict4 = primclex.settings().query_handler<Kinetics::DiffTransConfiguration>().dict();
  BOOST_CHECK_EQUAL(primclex.db<Kinetics::DiffTransConfiguration>().size(), 2);
  DB::Selection<Kinetics::DiffTransConfiguration> selection4(primclex);
  BOOST_CHECK_EQUAL(selection4.size(), 2);
  BOOST_CHECK_EQUAL(selection4.selected_size(), 0);

}

BOOST_AUTO_TEST_SUITE_END()
