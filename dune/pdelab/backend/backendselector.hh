#ifndef DUNE_PDELAB_BACKENDSELECTOR_HH
#define DUNE_PDELAB_BACKENDSELECTOR_HH
namespace Dune {
  namespace PDELab {

    template<typename Backend, typename GridFunctionSpace, typename FieldType>
    struct BackendVectorSelectorHelper
    {};

    template<typename GridFunctionSpace, typename FieldType>
    struct BackendVectorSelector
    {
      typedef typename GridFunctionSpace::Traits::Backend Backend;
      typedef typename BackendVectorSelectorHelper<Backend, GridFunctionSpace, FieldType>::Type Type;
    };

    template<typename Backend, typename VU, typename VV, typename E>
    struct BackendMatrixSelector
    {
      typedef typename Backend::template MatrixHelper<VV,VU,E>::type Type;
    };

  }
}

#endif
