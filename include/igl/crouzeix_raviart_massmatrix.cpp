// This file is part of libigl, a simple c++ geometry processing library.
// 
// Copyright (C) 2015 Alec Jacobson <alecjacobson@gmail.com>
// 
// This Source Code Form is subject to the terms of the Mozilla Public License 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.
#include "crouzeix_raviart_massmatrix.h"
#include "unique_simplices.h"
#include "oriented_facets.h"

#include "is_edge_manifold.h"
#include "doublearea.h"

#include <cassert>
#include <vector>

template <typename MT, typename DerivedV, typename DerivedF, typename DerivedE, typename DerivedEMAP>
void igl::crouzeix_raviart_massmatrix(
    const Eigen::MatrixBase<DerivedV> & V, 
    const Eigen::MatrixBase<DerivedF> & F, 
    Eigen::SparseMatrix<MT> & M,
    Eigen::PlainObjectBase<DerivedE> & E,
    Eigen::PlainObjectBase<DerivedEMAP> & EMAP)
{
  // All occurances of directed "facets"
  Eigen::MatrixXi allE;
  oriented_facets(F,allE);
  Eigen::VectorXi _1;
  unique_simplices(allE,E,_1,EMAP);
  return crouzeix_raviart_massmatrix(V,F,E,EMAP,M);
}

template <typename MT, typename DerivedV, typename DerivedF, typename DerivedE, typename DerivedEMAP>
void igl::crouzeix_raviart_massmatrix(
    const Eigen::MatrixBase<DerivedV> & V, 
    const Eigen::MatrixBase<DerivedF> & F, 
    const Eigen::MatrixBase<DerivedE> & E,
    const Eigen::MatrixBase<DerivedEMAP> & EMAP,
    Eigen::SparseMatrix<MT> & M)
{
  using namespace Eigen;
  using namespace std;
  // Mesh should be edge-manifold (TODO: replace `is_edge_manifold` with
  // `is_facet_manifold`)
  assert(F.cols() != 3 || is_edge_manifold(F));
  // number of elements (triangles)
  const int m = F.rows();
  // Get triangle areas/volumes
  VectorXd TA;
  // Element simplex size
  const int ss = F.cols();
  switch(ss)
  {
    default:
      assert(false && "Unsupported simplex size");
    case 3:
      doublearea(V,F,TA);
      TA *= 0.5;
      break;
    case 4:
      volume(V,F,TA);
      break;
  }
  vector<Triplet<MT> > MIJV(ss*m);
  assert(EMAP.size() == m*ss);
  for(int f = 0;f<m;f++)
  {
    for(int c = 0;c<ss;c++)
    {
      MIJV[f+m*c] = Triplet<MT>(EMAP(f+m*c),EMAP(f+m*c),TA(f)/(double)(ss));
    }
  }
  M.resize(E.rows(),E.rows());
  M.setFromTriplets(MIJV.begin(),MIJV.end());
}

#ifdef IGL_STATIC_LIBRARY
// Explicit template instantiation
template void igl::crouzeix_raviart_massmatrix<double, Eigen::Matrix<double, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, 1, 0, -1, 1> >(Eigen::MatrixBase<Eigen::Matrix<double, -1, -1, 0, -1, -1> > const&, Eigen::MatrixBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> > const&, Eigen::MatrixBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> > const&, Eigen::MatrixBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> > const&, Eigen::SparseMatrix<double, 0, int>&);
template void igl::crouzeix_raviart_massmatrix<float, Eigen::Matrix<float, -1, 3, 1, -1, 3>, Eigen::Matrix<unsigned int, -1, -1, 1, -1, -1>, Eigen::Matrix<int, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, 1, 0, -1, 1> >(Eigen::MatrixBase<Eigen::Matrix<float, -1, 3, 1, -1, 3> > const&, Eigen::MatrixBase<Eigen::Matrix<unsigned int, -1, -1, 1, -1, -1> > const&, Eigen::MatrixBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> > const&, Eigen::MatrixBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> > const&, Eigen::SparseMatrix<float, 0, int>&);
#endif
