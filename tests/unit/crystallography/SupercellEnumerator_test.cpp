#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include<boost/filesystem.hpp>

/// What is being tested:
#include "casm/crystallography/Lattice.hh"
#include "casm/crystallography/Structure.hh"
#include "casm/container/LinearAlgebra.hh"

/// What is being used to test it:
#include "casm/crystallography/SupercellEnumerator.hh"
#include "casm/symmetry/SymGroup.hh"
#include "casm/external/Eigen/Dense"

using namespace CASM;
boost::filesystem::path testdir("tests/unit/crystallography");

void autofail() {
  BOOST_CHECK_EQUAL(1, 0);
  return;
}

void hermite_init() {
  int dims = 5;
  int det = 30;

  HermiteCounter hermit_test(det, dims);

  Eigen::VectorXi init_diagonal(Eigen::VectorXi::Ones(dims));
  init_diagonal(0) = det;

  BOOST_CHECK_EQUAL(init_diagonal, hermit_test.diagonal());
  BOOST_CHECK_EQUAL(0, hermit_test.position());

  auto tricounter = HermiteCounter_impl::_upper_tri_counter(hermit_test.diagonal());
  Eigen::VectorXi startcount(Eigen::VectorXi::Zero(HermiteCounter_impl::upper_size(dims)));
  BOOST_CHECK_EQUAL(tricounter.current(), startcount);


  Eigen::VectorXi endcount(Eigen::VectorXi::Zero(HermiteCounter_impl::upper_size(dims)));
  endcount(0) = (det - 1);
  endcount(1) = (det - 1);
  endcount(2) = (det - 1);
  endcount(3) = (det - 1);

  auto finalcounterstate = tricounter;

  for(; tricounter.valid(); ++tricounter) {
    finalcounterstate = tricounter;
  }

  BOOST_CHECK_EQUAL(finalcounterstate.current(), endcount);


  return;
}

void spill_test() {
  Eigen::VectorXi diagonal0(Eigen::VectorXi::Ones(5));
  Eigen::VectorXi diagonal1(Eigen::VectorXi::Ones(5));
  Eigen::VectorXi diagonal2(Eigen::VectorXi::Ones(5));
  Eigen::VectorXi diagonal3(Eigen::VectorXi::Ones(5));


  int p = 0;
  diagonal0(p) = 2;
  int p0 = HermiteCounter_impl::_spill_factor(diagonal0, p, 2);
  BOOST_CHECK_EQUAL(p0, p + 1);
  BOOST_CHECK_EQUAL(diagonal0(p), 1);
  BOOST_CHECK_EQUAL(diagonal0(p + 1), 2);

  p = 3;
  diagonal1(p) = 6;
  int p1 = HermiteCounter_impl::_spill_factor(diagonal1, p, 2);
  BOOST_CHECK_EQUAL(p1, p + 1);
  BOOST_CHECK_EQUAL(diagonal1(p), 3);
  BOOST_CHECK_EQUAL(diagonal1(p + 1), 2);

  p = 3;
  diagonal2(p) = 6;
  int p2 = HermiteCounter_impl::_spill_factor(diagonal2, p, 4);
  BOOST_CHECK_EQUAL(p2, p + 1);
  BOOST_CHECK_EQUAL(diagonal2(p), 1);
  BOOST_CHECK_EQUAL(diagonal2(p + 1), 6);

  p = 2;
  diagonal3(p) = 8;
  int p3 = HermiteCounter_impl::_spill_factor(diagonal3, p, 4);
  BOOST_CHECK_EQUAL(p3, p + 1);
  BOOST_CHECK_EQUAL(diagonal3(p), 2);
  BOOST_CHECK_EQUAL(diagonal3(p + 1), 4);

  return;
}

void next_position_test() {
  //Example increment from one possible diagonal to the next
  Eigen::VectorXi diagonal(Eigen::VectorXi::Ones(5));
  Eigen::VectorXi next_diagonal(Eigen::VectorXi::Ones(5));
  diagonal(0) = 6;
  next_diagonal(0) = 3;
  next_diagonal(1) = 2;
  int p = 0;

  p = HermiteCounter_impl::next_spill_position(diagonal, p);

  BOOST_CHECK_EQUAL(diagonal, next_diagonal);
  BOOST_CHECK_EQUAL(p, 1);


  diagonal = Eigen::VectorXi::Ones(5);
  next_diagonal = Eigen::VectorXi::Ones(5);
  //[1 2 1 1 3]
  diagonal(1) = 2;
  diagonal(4) = 3;
  //[1 1 6 1 1]
  next_diagonal(2) = 6;

  p = 4;
  p = HermiteCounter_impl::next_spill_position(diagonal, p);

  BOOST_CHECK_EQUAL(diagonal, next_diagonal);
  BOOST_CHECK_EQUAL(p, 2);

  //*************/
  //Make sure every enumerated diagonal has the right determinant

  int det = 2 * 3 * 5 * 7;
  int dims = 5;

  Eigen::VectorXi diag = Eigen::VectorXi::Ones(dims);
  diag(0) = det;

  p = 0;
  while(p != diag.size()) {
    int testdet = 1;
    for(int i = 0; i < diag.size(); i++) {
      testdet = testdet * diag(i);
    }
    BOOST_CHECK_EQUAL(det, testdet);
    p = CASM::HermiteCounter_impl::next_spill_position(diag, p);
  }

  return;
}

void triangle_count_test() {
  HermiteCounter::Index totals = HermiteCounter_impl::upper_size(7);
  BOOST_CHECK_EQUAL(totals, -7 + 7 + 6 + 5 + 4 + 3 + 2 + 1);

  int dims = 5;
  int det = 30;

  Eigen::VectorXi mid_diagonal(Eigen::VectorXi::Ones(dims));
  mid_diagonal(0) = 5;
  mid_diagonal(1) = 3;
  mid_diagonal(4) = 2;

  auto countertest = HermiteCounter_impl::_upper_tri_counter(mid_diagonal);
  auto finalcount = countertest;

  for(; countertest.valid(); countertest++) {
    finalcount = countertest;
  }

  Eigen::VectorXi end_count_value(Eigen::VectorXi::Zero(dims));
  end_count_value(0) = 4;
  end_count_value(1) = 4;
  end_count_value(2) = 4;
  end_count_value(3) = 4;
  end_count_value(4) = 2;
  end_count_value(5) = 2;
  end_count_value(6) = 2;

  return;
}

void matrix_construction_test() {
  Eigen::VectorXi diag;
  diag.resize(4);
  diag << 2, 4, 6, 8;
  Eigen::VectorXi upper;
  upper.resize(3 + 2 + 1);
  upper << 11, 12, 13,
        21, 22,
        33;

  Eigen::MatrixXi diagmat;
  diagmat.resize(4, 4);
  diagmat << 2, 11, 12, 13,
          0, 4, 21, 22,
          0, 0, 6, 33,
          0, 0, 0, 8;

  BOOST_CHECK_EQUAL(diagmat, HermiteCounter_impl::_zip_matrix(diag, upper));

  return;
}

void increment_test() {
  HermiteCounter hermit_test(6, 4);

  Eigen::MatrixXi hermmat;
  hermmat.resize(4, 4);

  //Test starting status
  hermmat << 6, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1;

  BOOST_CHECK_EQUAL(hermmat, hermit_test());

  //Test next status
  ++hermit_test;
  hermmat << 6, 1, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1;

  BOOST_CHECK_EQUAL(hermmat, hermit_test());

  //Jump to just before you need a new diagonal
  hermmat << 6, 5, 5, 5,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1;

  while(hermit_test() != hermmat) {
    ++hermit_test;
  }

  //Check diagonal jump
  ++hermit_test;
  hermmat << 3, 0, 0, 0,
          0, 2, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1;

  BOOST_CHECK_EQUAL(hermmat, hermit_test());

  //Check invalidation and last status
  auto lastherm = hermmat;
  while(hermit_test.determinant() != 7) {
    lastherm = hermit_test();
    ++hermit_test;
  }

  hermmat << 1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 6;

  BOOST_CHECK_EQUAL(hermmat, lastherm);

  //Check determinant jump
  hermit_test = HermiteCounter(3, 4);

  //Jump to just before you need a new determinant

  hermmat << 1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 3;

  while(hermit_test() != hermmat) {
    ++hermit_test;
  }

  //Check determinant jump
  ++hermit_test;

  hermmat << 4, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1;

  BOOST_CHECK_EQUAL(hermmat, hermit_test());
  return;
}

void reset_test() {
  HermiteCounter hermit_test(1, 3);

  Eigen::MatrixXi hermmat;
  hermmat.resize(3, 3);


  Eigen::MatrixXi startmat = hermit_test();

  //Skip to one of the bigger determinants
  hermmat << 2, 1, 1,
          0, 2, 1,
          0, 0, 1;

  while(hermit_test() != hermmat) {
    ++hermit_test;
  }

  hermmat << 4, 0, 0,
          0, 1, 0,
          0, 0, 1;

  hermit_test.reset_current();

  BOOST_CHECK_EQUAL(hermmat, hermit_test());

  hermit_test.jump_to_determinant(1);

  BOOST_CHECK_EQUAL(startmat, hermit_test());

  return;
}

void expand_dims_test() {
  Eigen::MatrixXi expandmat(Eigen::MatrixXi::Ones(5, 5));
  expandmat = expandmat * 3;

  Eigen::VectorXi expanddims(8);
  expanddims << 1, 1, 1, 0, 1, 0, 0, 1;

  Eigen::MatrixXi expandedmat(8, 8);
  expandmat << 3, 3, 3, 0, 3, 0, 0, 3,
            3, 3, 3, 0, 3, 0, 0, 3,
            3, 3, 3, 0, 3, 0, 0, 3,
            0, 0, 0, 1, 0, 0, 0, 0,
            3, 3, 3, 0, 3, 0, 0, 3,
            0, 0, 0, 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0, 0, 1, 0,
            3, 3, 3, 0, 3, 0, 0, 1;

  BOOST_CHECK_EQUAL(expandmat, HermiteCounter_impl::_expand_dims_old(expandmat, expanddims));

  HermiteCounter minicount(1, 4);
  for(int i = 0; i < 12; i++) {
    ++minicount;
  }

  Eigen::MatrixXi endcount(4, 4);
  endcount << 1, 0, 0, 0,
           0, 2, 1, 1,
           0, 0, 1, 0,
           0, 0, 0, 1;

  BOOST_CHECK_EQUAL(endcount, minicount());

  Eigen::MatrixXi transmat(Eigen::MatrixXi::Identity(6, 6));

  Eigen::MatrixXi expanded = HermiteCounter_impl::_expand_dims(minicount(), transmat);
  Eigen::MatrixXi blockmat(6, 6);
  blockmat << 1, 0, 0, 0, 0, 0,
           0, 2, 1, 1, 0, 0,
           0, 0, 1, 0, 0, 0,
           0, 0, 0, 1, 0, 0,
           0, 0, 0, 0, 1, 0,
           0, 0, 0, 0, 0, 1;

  BOOST_CHECK_EQUAL(blockmat, expanded);

  Eigen::Matrix2Xi miniherm;
  miniherm << 2, 1,
           0, 3;

  Eigen::Matrix3i minitrans;
  minitrans << 1, 0, 0,
            0, 0, 1,
            0, 1, 0;

  Eigen::Matrix3i miniexpand;
  miniexpand << 2, 1, 0,
             0, 0, 1,
             0, 3, 0;

  BOOST_CHECK_EQUAL(HermiteCounter_impl::_expand_dims(miniherm, minitrans), miniexpand);


  return;
}

void it_matrix_test(boost::filesystem::path expected_mats) {
  jsonParser readmats(expected_mats);

  Array<Eigen::Matrix3Xi> past_enumerated_mat;
  int minvol, maxvol;

  from_json(minvol, readmats["min_vol"]);
  from_json(maxvol, readmats["max_vol"]);
  from_json(past_enumerated_mat, readmats["mats"]);

  boost::filesystem::path posfile;
  Array<Eigen::Matrix3Xi> enumerated_mat;

  from_json(posfile, readmats["source"]);
  Structure test_struc(testdir / posfile);
  Lattice test_lat = test_struc.lattice();
  const auto effective_pg = test_struc.factor_group();

  SupercellEnumerator<Lattice> test_enumerator(test_lat, effective_pg, minvol, maxvol);

  for(auto it = test_enumerator.begin(); it != test_enumerator.end(); ++it) {
    enumerated_mat.push_back(it.matrix());
  }

  BOOST_CHECK_EQUAL(past_enumerated_mat.size(), enumerated_mat.size());

  for(Index i = 0; i < past_enumerated_mat.size(); i++) {
    BOOST_CHECK(enumerated_mat.contains(past_enumerated_mat[i]));
  }

  return;
}

void it_lat_test(boost::filesystem::path expected_lats) {
  jsonParser readlats(expected_lats);
  Array<Lattice> past_enumerated_lats;
  int minvol, maxvol;

  from_json(minvol, readlats["min_vol"]);
  from_json(maxvol, readlats["max_vol"]);
  from_json(past_enumerated_lats, readlats["lats"]);

  boost::filesystem::path posfile;
  Array<Lattice> enumerated_lats;

  from_json(posfile, readlats["source"]);
  Structure test_struc(testdir / posfile);
  Lattice test_lat = test_struc.lattice();
  const auto effective_pg = test_struc.factor_group();

  test_lat.generate_supercells(enumerated_lats, effective_pg, minvol, maxvol, 3, Eigen::Matrix3i::Identity());


  BOOST_CHECK_EQUAL(past_enumerated_lats.size(), enumerated_lats.size());

  for(Index i = 0; i < past_enumerated_lats.size(); i++) {
    BOOST_CHECK(enumerated_lats.contains(past_enumerated_lats[i]));
  }

  return;
}

void unroll_test() {
  Eigen::MatrixXi mat5(5, 5);
  mat5 << 1, 12, 11, 10, 9,
       0, 2, 13, 15, 8,
       0, 0, 3, 14, 7,
       0, 0, 0, 4, 6,
       0, 0, 0, 0, 5;

  Eigen::VectorXi vec5(5 + 4 + 3 + 2 + 1);
  vec5 << 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15;

  BOOST_CHECK_EQUAL(vec5, HermiteCounter_impl::_canonical_unroll(mat5));

  return;

  Eigen::Matrix3i mat3;
  mat3 << 1, 6, 5,
       0, 2, 4,
       0, 0, 3;

  Eigen::Vector3i vec3(3 + 2 + 1);
  vec3 << 1, 2, 3, 4, 5, 6;

  BOOST_CHECK_EQUAL(vec3, HermiteCounter_impl::_canonical_unroll(mat3));

  return;
}

void compare_test() {
  Eigen::Matrix3i low, high;

  low << 1, 9, 9,
      0, 9, 9,
      0, 9, 9;

  high << 2, 0, 0,
       0, 1, 0,
       0, 0, 1;

  BOOST_CHECK_EQUAL(HermiteCounter_impl::_canonical_compare(low, high), 1);

  low << 1, 9, 9,
      0, 9, 9,
      0, 9, 9;

  high << 1, 10, 9,
       0, 9, 9,
       0, 9, 9;

  BOOST_CHECK_EQUAL(HermiteCounter_impl::_canonical_compare(low, high), 1);

  return;
}

void trans_enum_test() {
  Lattice testlat(Lattice::fcc());
  SymGroup pg;
  testlat.generate_point_group(pg);
  int dims = 3;
  Eigen::Matrix3i transmat;

  transmat << -1, 1, 1,
           1, -1, 1,
           1, 1, -1;

  Lattice bigunit = make_supercell(testlat, transmat);

  SupercellEnumerator<Lattice> enumerator(testlat, pg, 1, 5 + 1, dims, transmat);

  std::vector<Lattice> enumerated_lat(enumerator.begin(), enumerator.end());

  for(Index i = 0; i > enumerated_lat.size(); i++) {
    BOOST_CHECK(enumerated_lat[i].is_supercell_of(bigunit));
  }

  return;
}

/*
void restricted_test()
{
    Lattice testlat=Lattice::fcc();
    SymGroup pg;
    testlat.generate_point_group(pg);
    int dims=1;

    SupercellEnumerator<Lattice> enumerator(testlat,pg,1,5+1,dims);
    std::vector<Lattice> enumerated_lat(enumerator.begin(), enumerator.end());

    BOOST_CHECK_EQUAL(enumerated_lat.size(),5);

    int l=1;
    for(auto it=enumerated_lat.begin(); it!=enumerated_lat.end(); ++it)
    {
        Eigen::Matrix3i comp_transmat;
        comp_transmat<<(l),0,0,
            0,1,0,
            0,0,1;

        Lattice comparelat=make_supercell(testlat,comp_transmat);

        Lattice nigglicompare=niggli(comparelat,pg,TOL);
        Lattice nigglitest=niggli(*it,pg,TOL);

        BOOST_CHECK(nigglicompare==nigglitest);
        l++;
    }

    return;
}
*/

void restricted_test() {
  std::vector<Lattice> all_test_lats;
  all_test_lats.push_back(Lattice::fcc());
  all_test_lats.push_back(Lattice::bcc());
  all_test_lats.push_back(Lattice::cubic());
  all_test_lats.push_back(Lattice::hexagonal());

  for(Index t = 0; t < all_test_lats.size(); t++) {
    Lattice testlat = all_test_lats[t];
    SymGroup pg;
    testlat.generate_point_group(pg);
    int dims = 1;

    SupercellEnumerator<Lattice> enumerator(testlat, pg, 1, 15 + 1, dims);

    int l = 1;
    for(auto it = enumerator.begin(); it != enumerator.end(); ++it) {
      Eigen::Matrix3i comp_transmat;
      comp_transmat << (l), 0, 0,
                    0, 1, 0,
                    0, 0, 1;

      BOOST_CHECK(it.matrix() == canonical_hnf(comp_transmat, pg, testlat));
      l++;
    }
  }

  return;
}


BOOST_AUTO_TEST_SUITE(SupercellEnumeratorTest)

BOOST_AUTO_TEST_CASE(HermiteConstruction) {
  hermite_init();
}

BOOST_AUTO_TEST_CASE(HermiteImpl) {
  spill_test();
  next_position_test();
  triangle_count_test();
  matrix_construction_test();
  reset_test();
  unroll_test();
  compare_test();
}

BOOST_AUTO_TEST_CASE(HermiteCounting) {
  increment_test();
}

//Tests in here were created by first getting results from
//before HermiteCounter existed and then making sure the results
//didn't change after it was introduced
BOOST_AUTO_TEST_CASE(EnumeratorConsistency) {

  it_matrix_test(boost::filesystem::path(testdir / "POS1_1_6_mats.json"));
  it_matrix_test(boost::filesystem::path(testdir / "PRIM1_2_9_mats.json"));
  it_matrix_test(boost::filesystem::path(testdir / "PRIM2_4_7_mats.json"));
  it_matrix_test(boost::filesystem::path(testdir / "PRIM4_1_8_mats.json"));

  it_lat_test(boost::filesystem::path(testdir / "POS1_2_6_lats.json"));
  it_lat_test(boost::filesystem::path(testdir / "PRIM1_2_9_lats.json"));
  it_lat_test(boost::filesystem::path(testdir / "PRIM2_3_7_lats.json"));
  it_lat_test(boost::filesystem::path(testdir / "PRIM4_1_8_lats.json"));
  it_lat_test(boost::filesystem::path(testdir / "PRIM5_1_8_lats.json"));
}

BOOST_AUTO_TEST_CASE(RestrictedEnumeration) {
  trans_enum_test();
  restricted_test();
  //restricted_trans_enum_test();
}

BOOST_AUTO_TEST_SUITE_END()
