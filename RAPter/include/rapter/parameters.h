#ifndef RAPTER_PARAMETERS_H
#define RAPTER_PARAMETERS_H

#include <iostream>
#include <vector>

#include "Eigen/Dense"

#include "rapter/simpleTypes.h"
#include "rapter/primitives/angles.h" // AnglesT

namespace rapter {

    //! \brief Collection of parameters that are used in almost every solution step.
    template <typename _Scalar>
    struct CommonParams
    {
            typedef _Scalar Scalar;
            //! \brief Scale parameter of input.
            _Scalar scale = 0.05;

            //! \brief Desired angles stored in radians but generated from degrees.
            AnglesT angles; // = { 0, M_PI_2, M_PI };

            //! \brief  Point-count threshold, that decides if a patch is big or small. (small: \#points < patch_pop_limit, large: \#points >= patch_pop_limit).
            //!         \n Used in \ref CandidateGenerator::generate(), \ref ProblemSetup::formulate(), \ref problemSetup::largePatchesNeedDirectionConstraint().
            //!         \n In \ref CandidateGenerator::generate() decides, when a patch can distribute its directions.
            //!         \n In \ref ProblemSetup::formulate() decides, which constraint is to be used in \ref problemSetup::largePatchesNeedDirectionConstraint().
            int patch_population_limit = -1;

            //! \brief Two lines are parallel, if their angle is smaller than this.
            //!        Used in \ref Merging::mergeSameDirGids() and visualization.
            _Scalar parallel_limit = _Scalar(1e-6);

    }; //...CommonParams

    //! \brief Collection of parameters for \ref CandidateGenerator::generate.
    template <typename _Scalar>
    struct CandidateGeneratorParams : public CommonParams<_Scalar>
    {
            /*!
             * \copydoc CandidateGeneratorParams::patch_dist_mode.
             */
            enum PatchPatchDistMode {
                                      REPR_SQR      //!< \brief \ref RepresentativeSqrPatchPatchDistanceFunctorT, \ref SpatialPatchPatchSingleDistanceFunctorT
                                    };

            /*!
             * \brief Determines, what to do with small patches during generation fase.
             */
            enum SmallPatchesMode {
                IGNORE              //!< \brief Don't send nor receive directions. Other selected primitives will have to explain this patch.
                , RECEIVE_SIMILAR   /*!  \brief Don't send but receive similar directions. It's not optimal to bias the selection to the noisy local fit. Better to use IGNORE.
                                                \note This is the default behaviour for large patches. */
                , RECEIVE_ALL       //!< \brief Don't send but receive all directions. This option makes it untractable, needs sub-sampling.
            };

            //! \brief Angle threshold for similar lines.
            //!        Used in \ref Segmentation::patchify() and \ref Merging::mergeSameDirGids().
            _Scalar   angle_limit                 = 0.08f;

            //! \brief Regiongrow uses this option to determine which functor to use for a patch-patch distance.
            //!        Used in \ref Segmentation::patchify().
            PatchPatchDistMode patch_dist_mode    = REPR_SQR;

            //! \brief Patchify takes "patch_dist_limit * scale" as maximum spatial distance.
            //!        Used in \ref Segmentation::patchify() and \ref Merging::mergeSameDirGids().
            //! \warning Are you sure this needs to be != 1.f? The patch_spatial_weight takes care of this for us.
            _Scalar   patch_dist_limit_mult       = 1.f;

            //! \brief Number of points used for primitive fitting.
            //!        Used in \ref Segmentation::orientPoints(), \ref Segmentation::regionGrow() relayed from \ref Segmentation::patchify().
            int       nn_K                        = 15;

            //! \brief Weight of spatial term in \f$ patch\_spatial\_weight^2 \cdot \frac{spat\_dist^2}{spat\_thresh} + \frac{ang\_diff^2}{ang\_thresh} < 1 \f$ regionGrowing distance functor.
            //!        Used in \ref Segmentation::patchify() and \ref Merging::mergeSameDirGids().
            //! \sa \ref rapter::RepresentativeSqrPatchPatchDistanceFunctorT.
            _Scalar   patch_spatial_weight        = _Scalar(0.5);

            //! \brief Determines, how much the angle_limit is divided by, to get the patch-similarity threshold.
            //!        Used in \ref CandidateGenerator::generate().
            _Scalar   angle_limit_div             = 10.f;

            /*! \brief Determines, what to do with small patches during generation fase.
             *         Used in \ref CandidateGenerator::generate().
             */
            SmallPatchesMode small_mode = IGNORE;

            /*! \brief Decides, what primitive counts as small. Usually a multiple of scale (4x..0.1x)
             */
            _Scalar          small_thresh_mult = 10.f;

            /*! \brief Prevent variable explosion by trusting active candidates and just solving the promoted ones.
             */
            bool safe_mode = false;

            /*! \brief Prevent variable explosion by limiting the number of output variables to this number. Default: 0, means unlimited.
             */
            int var_limit = 0;


            //_____________________________________________
            //____________________Parsers__________________
            //_____________________________________________

            inline int parsePatchDistMode( std::string const& patch_dist_mode_string )
            {
                int err = EXIT_SUCCESS;
                if ( !patch_dist_mode_string.compare("representative_sqr") )
                {
                    this->patch_dist_mode = REPR_SQR;
                }
                else
                {
                    err = EXIT_FAILURE;
                    std::cerr << "[" << __func__ << "]: " << "Could NOT parse " << patch_dist_mode_string << ", assuming " << printPatchDistMode() << std::endl;
                }

                return err;
            } // ...parsePatchDistMode()

            inline std::string printPatchDistMode() const
            {
                switch ( patch_dist_mode )
                {
                    case REPR_SQR: return std::string("representative_sqr"); break;
                    default:       return "UNKNOWN"; break;
                }
            } // ...printPatchDistMode()

    }; // ...struct CandidateGeneratorParams

    //! \brief Collection of parameters the \ref ProblemSetup::formulate needs.
    template <typename _Scalar>
    struct ProblemSetupParams : public CommonParams<_Scalar>
    {
        //! \brief Options to calculate data cost of primitive by.
        enum DATA_COST_MODE { ASSOC_BASED    = 0 //!< \brief \sa \ref problemSetup::associationBasedDataCost
                            , BAND_BASED     = 1 //!< \brief \sa \ref problemSetup::bandBasedDataCost
                            , INSTANCE_BASED = 2 //!< \brief \sa \ref problemSetup::instanceBasedDataCost
                            };
        //! \brief Options to calculate constraints for patches in optimization.
        enum CONSTR_MODE    { PATCH_WISE     = 0 //!< \brief \sa \ref problemSetup::everyPatchNeedsDirectionConstraint
                            , POINT_WISE     = 1 //!< \brief \sa \ref problemSetup::everyPointNeedsPatchConstraint
                            , HYBRID         = 2 //!< \brief \sa \ref problemSetup::largePatchesNeedDirectionConstraint
                            };

        using CommonParams<_Scalar>::scale;
        using CommonParams<_Scalar>::angles;

        //! Decides, which constraint mode to use in \ref problemSetup::largePatchesNeedDirectionConstraint.
        using CommonParams<_Scalar>::patch_population_limit;

        //! \brief Optimization weights. [ 0: unary, 1: pairwise, 2: complexity ]
        Eigen::Matrix<_Scalar,-1,1>  weights         = (Eigen::Matrix<_Scalar,-1,1>(3,1) << 10000, 10, 1).finished();
        //! \brief \copydoc CONSTR_MODE
        CONSTR_MODE                  constr_mode     = HYBRID;
        //! \brief \copydoc DATA_COST_MODE
        DATA_COST_MODE               data_cost_mode  = ASSOC_BASED;

        //! \brief Added to pairwise cost, if direction ids don't match.
        _Scalar                      dir_id_bias     = _Scalar( 0. );

        //! \brief dataterm = data_weight * freq_weight/#instances * data cost
        _Scalar                      freq_weight     = _Scalar( 0. );

        int     useAngleGen = 0;
        _Scalar truncAngle  = 0.;

        _Scalar spatial_weight_coeff = _Scalar( 10. ); // E_pw = (1. + spatial_weight_coeff) * comp(P0,P1)

        static const _Scalar w_mod_base          /* = 0.1f*/;  // E_data *= w_mod_base + (1. - w_mod_base) * f(#j/|P|), usually 0.1

        //! \brief neighbourhood is this times scale, usually 1.0 or 2.0
        _Scalar spatial_weight_dist_mult = 2.;      // (used to be 3. for 3D)
        _Scalar collapseAngleSqrt = 0.07; // sqrt( 0.1 deg ) == 0.041 (rad^-1)
    };

//    template <typename _Scalar>
//    const _Scalar ProblemSetupParams<_Scalar>::spatial_weight_coeff = 10.f; // 10 was good
    //template <typename _Scalar>
    //const _Scalar ProblemSetupParams<_Scalar>::spatial_weight_distance = 2.f; // 2 was good
    template <typename _Scalar>
    const _Scalar ProblemSetupParams<_Scalar>::w_mod_base = 0.1f; // 0.1 was good

    template <typename _Scalar>
    struct MergeParams : public CandidateGeneratorParams<_Scalar>
    {
        using CommonParams<_Scalar>::scale;
        using CommonParams<_Scalar>::angles;
        using CommonParams<_Scalar>::parallel_limit;
        using CandidateGeneratorParams<_Scalar>::patch_dist_limit_mult;
        using CandidateGeneratorParams<_Scalar>::angle_limit;
        using CandidateGeneratorParams<_Scalar>::patch_spatial_weight;

        //! \brief Adopt orphaned points greedily, 1: unambiguous only, 2: all.
        int do_adopt = 2;

        /*! \brief Multiplied by scale decides, whether two patches are adjacent (have two extrema closer than spatial_threshold_mult * scale).
         *         Used in \ref Merging::mergeSameDirGids(). */
        _Scalar spatial_threshold_mult = _Scalar( 2.5 );

        bool is3D;
    };

}

#endif // RAPTER_PARAMETERS_H
