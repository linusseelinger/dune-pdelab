// -*- tab-width: 4; indent-tabs-mode: nil -*-

#ifdef HAVE_CONFIG_H
#include "config.h"     
#endif

#include <string>
#include <sstream>

#include <dune/common/fvector.hh>
#include <dune/common/mpihelper.hh>
#include <dune/common/smartpointer.hh>
#include <dune/common/static_assert.hh>

#include <dune/grid/io/file/vtk/subsamplingvtkwriter.hh>
#ifdef HAVE_ALBERTA
#include <dune/grid/albertagrid.hh>
#include <dune/grid/albertagrid/gridfactory.hh>
#endif
#ifdef HAVE_ALUGRID
#include <dune/grid/alugrid.hh>
#endif
#ifdef HAVE_UG
#include <dune/grid/uggrid.hh>
#include <dune/grid/uggrid/uggridfactory.hh>
#endif

#include "../common/vtkexport.hh"
#include "../gridfunctionspace/gridfunctionspace.hh"
#include "../gridfunctionspace/gridfunctionspaceutilities.hh"
#include "../finiteelementmap/edges02dfem.hh"

template<typename GV>
void edgeS02DGridFunctionSpace (const GV& gv, const std::string &suffix = "", unsigned int index = 0)
{
  typedef typename GV::Grid::ctype D; // domain type
  typedef double R;                   // range type

  std::ostringstream filename;
  filename << "edges02dgridfunctionspace";
  if(suffix != "") filename << "-" << suffix;

  Dune::PDELab::EdgeS02DLocalFiniteElementMap<GV,R> fem(gv);   // maps entity to finite element

  typedef Dune::PDELab::GridFunctionSpace<
    GV,
    Dune::PDELab::EdgeS02DLocalFiniteElementMap<GV,R>
    > GFS;
  GFS gfs(gv,fem);                    // make grid function space

  typedef typename GFS::template VectorContainer<R>::Type X;
  X x(gfs,0.0);                       // make coefficient vector
  x[index % gfs.globalSize()] = 1.0;  // set a component

  typedef Dune::PDELab::DiscreteGridFunctionGlobal<GFS,X> DGF;
  DGF dgf(gfs,x);                     // make a grid function

  Dune::SubsamplingVTKWriter<GV> vtkwriter(gv,3);  // plot result
  vtkwriter.addVertexData(new Dune::PDELab::VTKGridFunctionAdapter<DGF>(dgf,"edges02d"));
  vtkwriter.write(filename.str(),Dune::VTKOptions::ascii);
}

// UnitTriangle

template<typename Grid>
class UnitTriangleMaker {
  dune_static_assert(Grid::dimension == 2, "Dimension of grid must be 2");
  dune_static_assert(Grid::dimensionworld == 2, "Dimension of world must be 2");
public:
  static Dune::SmartPointer<Grid> create() {
    Dune::GridFactory<Grid> gf;
    Dune::FieldVector<typename Grid::ctype, 2> pos;

    pos[0] = 0; pos[1] = 0; gf.insertVertex(pos);
    pos[0] = 1; pos[1] = 0; gf.insertVertex(pos);
    pos[0] = 0; pos[1] = 1; gf.insertVertex(pos);

    Dune::GeometryType type;
    type.makeTriangle();
    std::vector<unsigned int> vid(3);

    vid[0] = 0; vid[1] = 1; vid[2] = 2; gf.insertElement(type, vid);

    return gf.createGrid();
  }
};

#ifdef HAVE_ALUGRID
template<>
class UnitTriangleMaker<Dune::ALUSimplexGrid<2,2> > {
  typedef Dune::ALUSimplexGrid<2,2> Grid;
public:
  static Dune::SmartPointer<Grid> create() {
    return new Grid("grids/2dtriangle.alu");
  }
};
#endif // HAVE_ALUGRID

// TriangulatedUnitSquare

template<typename Grid>
class TriangulatedUnitSquareMaker {
  dune_static_assert(Grid::dimension == 2, "Dimension of grid must be 2");
  dune_static_assert(Grid::dimensionworld == 2, "Dimension of world must be 2");
public:
  static Dune::SmartPointer<Grid> create() {
    Dune::GridFactory<Grid> gf;
    Dune::FieldVector<typename Grid::ctype, 2> pos;

    pos[0] = 0; pos[1] = 0; gf.insertVertex(pos);
    pos[0] = 1; pos[1] = 0; gf.insertVertex(pos);
    pos[0] = 0; pos[1] = 1; gf.insertVertex(pos);
    pos[0] = 1; pos[1] = 1; gf.insertVertex(pos);

    Dune::GeometryType type;
    type.makeTriangle();
    std::vector<unsigned int> vid(3);

    vid[0] = 0; vid[1] = 1; vid[2] = 2; gf.insertElement(type, vid);
    vid[0] = 1; vid[1] = 2; vid[2] = 3; gf.insertElement(type, vid);

    return gf.createGrid();
  }
};

#ifdef HAVE_ALUGRID
template<>
class TriangulatedUnitSquareMaker<Dune::ALUSimplexGrid<2,2> > {
  typedef Dune::ALUSimplexGrid<2,2> Grid;
public:
  static Dune::SmartPointer<Grid> create() {
    return new Grid("grids/2dsimplex.alu");
  }
};
#endif // HAVE_ALUGRID

template<typename Grid>
void test(Dune::SmartPointer<Grid> grid, unsigned int index, int &result, std::string name = "", unsigned int refine = 0)
{
  grid->globalRefine(refine);

  if(name == "") name = grid->name();

  edgeS02DGridFunctionSpace(grid->leafView(), name);
  result = 0;
}

int main(int argc, char** argv)
{
  try{
    //Maybe initialize Mpi
    Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);

    // default exitcode 77 (=skipped); returned in case none of the supported
    // Grids were found
    int result = 77;

#ifdef HAVE_ALBERTA
    test(UnitTriangleMaker          <Dune::AlbertaGrid<2, 2>    >::create(),
         0, result, "alberta-triangle", 2);
    test(TriangulatedUnitSquareMaker<Dune::AlbertaGrid<2, 2>    >::create(),
         0, result, "alberta-square",   2);
#endif

#ifdef HAVE_ALUGRID
    test(UnitTriangleMaker          <Dune::ALUSimplexGrid<2, 2> >::create(),
         0, result, "alu-triangle",     2);
    test(TriangulatedUnitSquareMaker<Dune::ALUSimplexGrid<2, 2> >::create(),
         0, result, "alu-square",       2);
#endif // HAVE_ALUGRID

#ifdef HAVE_UG
    test(UnitTriangleMaker          <Dune::UGGrid<2>            >::create(),
         0, result, "ug-triangle",      2);
    test(TriangulatedUnitSquareMaker<Dune::UGGrid<2>            >::create(),
         0, result, "ug-square",        2);
#endif // HAVE_ALBERTA

    return result;
  }
  catch (Dune::Exception &e){
    std::cerr << "Dune reported error: " << e << std::endl;
    return 1;
  }
  catch (...){
    std::cerr << "Unknown exception thrown!" << std::endl;
    return 1;
  }
}
