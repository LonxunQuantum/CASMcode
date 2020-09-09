#include "gtest/gtest.h"
#include "autotools.hh"
#include "Common.hh"

#include "casm/app/ProjectBuilder.hh"
#include "casm/app/ProjectSettings.hh"
#include "casm/clex/PrimClex.hh"
#include "casm/clex/ScelEnum.hh"
#include "casm/crystallography/Structure.hh"
#include "casm/crystallography/Superlattice.hh"
#include "casm/database/DatabaseTypes_impl.hh"

#include "crystallography/TestStructures.hh" // for test::ZrO_prim

// Enumerators
// -----------
//
// Enumerators in CASM are classes that provide iterators which when incremented iteratively
// constuct new objects, typically Supercell or Configuration. When used via the casm command line
// program subcommand `casm enum`, the constructed objects are added to a database for future use.
// When used in C++ code, the constructed objects can be stored in the database or the used in other
// ways.
//
// This example demonstrates enumerating Supercell. There are three related Supercell enumerators:
// - `ScelEnumByProps`: Enumerate Supercell by enumerating superlattices as specifying parameters
//   (CASM::xtal::ScelEnumProps) such as the beginning volume, ending volume, what the unit
//   lattice is (in terms of the prim lattice), and which lattice vectors to enumerate over. This
//   is similar to the example 002_crystallography_superlattice_test.cpp.
// - `ScelEnumByName`: Iterates over Supercell that already exist in the Supercell database by
//   specifying a list of Supercell by name. This is mostly useful as an input to other methods
//   specifying which Supercells to use as input.
// - `ScelEnum`: This enumerator is primarily intended for command line program use and allows use
//   of either `ScelEnumByProps` or `ScelEnumByName` depending on which parameters are passed.
//


// This test fixture class constructs a CASM project for enumeration examples
class ExampleEnumerationZrOScelEnum : public testing::Test {
protected:

  std::string title;
  std::shared_ptr<CASM::Structure const> shared_prim;
  CASM::ProjectSettings project_settings;
  CASM::PrimClex primclex;

  // CASM::xtal::ScelEnumProps contains parameters which control which super lattice enumeration
  int begin_volume;
  int end_volume;
  std::string dirs;
  Eigen::Matrix3i generating_matrix;
  CASM::xtal::ScelEnumProps enumeration_params;

  ExampleEnumerationZrOScelEnum():
    title("ExampleEnumerationZrOScelEnum"),
    shared_prim(std::make_shared<CASM::Structure const>(test::ZrO_prim())),
    project_settings(make_default_project_settings(*shared_prim, title)),
    primclex(project_settings, shared_prim),
    begin_volume(1),
    end_volume(5),
    dirs("abc"),
    generating_matrix(Eigen::Matrix3i::Identity()),
    enumeration_params(begin_volume, end_volume, dirs, generating_matrix) {}

};

TEST_F(ExampleEnumerationZrOScelEnum, Example1) {

  // The ScelEnumByProps variant that is constructed with a shared prim Structure makes Supercell
  // that do not have a PrimClex pointer. They are not inserted into the Supercell database
  // automatically.
  CASM::ScelEnumByProps enumerator {shared_prim, enumeration_params};

  std::vector<CASM::Supercell> supercells {enumerator.begin(), enumerator.end()};

  EXPECT_EQ(supercells.size(), 20);
  EXPECT_EQ(primclex.db<Supercell>().size(), 0);

  for(Supercell const &supercell : supercells) {
    // Supercell generated by ScelEnumByProps are in canonical form
    EXPECT_TRUE(supercell.is_canonical());

    // Only insert canonical supercells into the Supercell database
    primclex.db<Supercell>().insert(supercell);
  }

  EXPECT_EQ(primclex.db<Supercell>().size(), 20);
}

TEST_F(ExampleEnumerationZrOScelEnum, Example2) {

  // The ScelEnumByProps variant that accepts a PrimClex in the constructor inserts Supercells into
  // the Supercell database as it constructs them. The additional option "existing_only" allows
  // restricting the output Supercells to ones that are already in the Supercell database.
  bool existing_only = false;
  CASM::ScelEnumByProps enumerator {primclex, enumeration_params, existing_only};

  std::vector<CASM::Supercell> supercells {enumerator.begin(), enumerator.end()};

  EXPECT_EQ(supercells.size(), 20);
  EXPECT_EQ(primclex.db<Supercell>().size(), 20);
}
