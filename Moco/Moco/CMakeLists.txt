
set(MOCO_SOURCES
        osimMoco.h
        osimMocoDLL.h
        About.h
        About.cpp
        RegisterTypes_osimMoco.h
        RegisterTypes_osimMoco.cpp
        MocoUtilities.h
        MocoUtilities.cpp
        MocoStudy.h
        MocoStudy.cpp
        MocoBounds.h
        MocoBounds.cpp
        MocoVariableInfo.h
        MocoVariableInfo.cpp
        MocoProblem.h
        MocoProblem.cpp
        MocoProblemInfo.h
        MocoProblemRep.h
        MocoProblemRep.cpp
        MocoGoal/MocoGoal.h
        MocoGoal/MocoGoal.cpp
        MocoGoal/MocoMarkerFinalGoal.h
        MocoGoal/MocoMarkerFinalGoal.cpp
        MocoGoal/MocoMarkerTrackingGoal.h
        MocoGoal/MocoMarkerTrackingGoal.cpp
        MocoWeightSet.h
        MocoGoal/MocoStateTrackingGoal.h
        MocoGoal/MocoStateTrackingGoal.cpp
        MocoGoal/MocoSumSquaredStateGoal.h
        MocoGoal/MocoSumSquaredStateGoal.cpp
        MocoGoal/MocoContactTrackingGoal.cpp
        MocoGoal/MocoContactTrackingGoal.h
        MocoGoal/MocoControlGoal.h
        MocoGoal/MocoControlGoal.cpp
        MocoGoal/MocoControlTrackingGoal.h
        MocoGoal/MocoControlTrackingGoal.cpp
        MocoGoal/MocoJointReactionGoal.h
        MocoGoal/MocoJointReactionGoal.cpp
        MocoGoal/MocoOrientationTrackingGoal.h
        MocoGoal/MocoOrientationTrackingGoal.cpp
        MocoGoal/MocoOutputGoal.h
        MocoGoal/MocoOutputGoal.cpp
        MocoGoal/MocoTranslationTrackingGoal.h
        MocoGoal/MocoTranslationTrackingGoal.cpp
        MocoGoal/MocoAngularVelocityTrackingGoal.h
        MocoGoal/MocoAngularVelocityTrackingGoal.cpp
        MocoGoal/MocoAccelerationTrackingGoal.h
        MocoGoal/MocoAccelerationTrackingGoal.cpp
        MocoGoal/MocoInitialActivationGoal.cpp
        MocoGoal/MocoInitialActivationGoal.h
        MocoGoal/MocoInitialVelocityEquilibriumDGFGoal.h
        MocoGoal/MocoInitialVelocityEquilibriumDGFGoal.cpp
        MocoGoal/MocoInitialForceEquilibriumGoal.h
        MocoGoal/MocoInitialForceEquilibriumGoal.cpp
        MocoGoal/MocoPeriodicityGoal.h
        MocoGoal/MocoPeriodicityGoal.cpp
        MocoSolver.h
        MocoSolver.cpp
        MocoDirectCollocationSolver.h
        MocoDirectCollocationSolver.cpp
        MocoTrajectory.h
        MocoTrajectory.cpp
        MocoTropterSolver.h
        MocoTropterSolver.cpp
        MocoParameter.h
        MocoParameter.cpp
        MocoConstraint.h
        MocoConstraint.cpp
        MocoControlBoundConstraint.cpp
        MocoControlBoundConstraint.h
        MocoFrameDistanceConstraint.h
        MocoFrameDistanceConstraint.cpp
        Components/DeGrooteFregly2016Muscle.h
        Components/DeGrooteFregly2016Muscle.cpp
        Components/ActivationCoordinateActuator.h
        Components/StationPlaneContactForce.h
        Components/StationPlaneContactForce.cpp
        Components/PositionMotion.h
        Components/PositionMotion.cpp
        Components/ModelFactory.h
        Components/ModelFactory.cpp
        Components/DiscreteForces.h
        Components/DiscreteForces.cpp
        Components/AccelerationMotion.h
        Components/AccelerationMotion.cpp
        Components/MultivariatePolynomialFunction.h
        MocoCasADiSolver/MocoCasADiSolver.h
        MocoCasADiSolver/MocoCasADiSolver.cpp
        MocoCasADiSolver/MocoCasOCProblem.h
        MocoCasADiSolver/MocoCasOCProblem.cpp
        MocoCasADiSolver/CasOCProblem.h
        MocoCasADiSolver/CasOCProblem.cpp
        MocoCasADiSolver/CasOCSolver.h
        MocoCasADiSolver/CasOCSolver.cpp
        MocoCasADiSolver/CasOCFunction.h
        MocoCasADiSolver/CasOCFunction.cpp
        MocoCasADiSolver/CasOCTranscription.h
        MocoCasADiSolver/CasOCTranscription.cpp
        MocoCasADiSolver/CasOCTrapezoidal.h
        MocoCasADiSolver/CasOCTrapezoidal.cpp
        MocoCasADiSolver/CasOCHermiteSimpson.h
        MocoCasADiSolver/CasOCHermiteSimpson.cpp
        MocoCasADiSolver/CasOCIterate.h
        MocoInverse.cpp
        MocoInverse.h
        MocoTrack.h
        MocoTrack.cpp
        Common/TableProcessor.h
        ModelProcessor.h
        ModelOperators.h
        MocoTool.h
        MocoTool.cpp
        MocoConstraintInfo.h
        MocoConstraintInfo.cpp
        MocoStudyFactory.h
        MocoStudyFactory.cpp)
if (MOCO_WITH_TROPTER)
    list(APPEND MOCO_SOURCES
            tropter/TropterProblem.h
            tropter/TropterProblem.cpp
            )
endif ()
add_library(osimMoco SHARED ${MOCO_SOURCES})
target_link_libraries(osimMoco PUBLIC osimTools
        PRIVATE casadi)
if (MOCO_WITH_TROPTER)
    target_link_libraries(osimMoco PRIVATE tropter)
endif ()

target_include_directories(osimMoco INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Moco>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set_target_properties(osimMoco PROPERTIES
        DEFINE_SYMBOL OSIMMOCO_EXPORTS
        PROJECT_LABEL "Library - osimMoco"
        FOLDER "Moco"
        )
if (MOCO_WITH_TROPTER)
    target_compile_definitions(osimMoco PUBLIC MOCO_WITH_TROPTER)
endif ()
target_compile_definitions(osimMoco PRIVATE
        OPENSIM_MOCO_VERSION=${MOCO_FULL_VERSION}
        )

MocoAddInstallRPATHSelf(TARGET osimMoco LOADER)
MocoAddInstallRPATHSimbody(TARGET osimMoco LOADER
        FROM "${CMAKE_INSTALL_LIBDIR}")

if (WIN32)
    add_dependencies(osimMoco Copy_OpenSim_DLLs)
endif ()

install(TARGETS osimMoco EXPORT OpenSimMocoTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# TODO do not install InverseMuscleSolverMotionData.h b/c it uses Eigen.
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Moco
        FILES_MATCHING PATTERN "*.h*")
