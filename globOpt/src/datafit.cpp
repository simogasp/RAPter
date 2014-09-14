#include "globfit2/globOpt_types.h" // _2d, _3d namespaces
#include "globfit2/util/parse.h" // GF2::console

#include "optimization/qp/solver.h" // todo: move datafit to datafit.h

int datafit( int argc, char** argv )
{
    if ( GF2::console::find_switch(argc,argv,"--datafit3D") )
    {
        return GF2::Solver::datafit< GF2::_3d::PrimitiveContainerT
                                   , GF2::_3d::InnerPrimitiveContainerT
                                   , GF2::_3d::PrimitiveT
                                   >( argc, argv );
    }
    else
    {
        return GF2::Solver::datafit< GF2::_2d::PrimitiveContainerT
                                   , GF2::_2d::InnerPrimitiveContainerT
                                   , GF2::_2d::PrimitiveT
                                   >( argc, argv );
    } //...if find_switch
}

