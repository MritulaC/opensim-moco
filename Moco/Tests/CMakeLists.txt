include(CMakeParseArguments)

# Create an exectuable for the file ${TEST_NAME}.cpp, which depends on
# libraries ${LIB_DEPENDENCIES}. Also create a CTest test for this executable.
function(MocoAddTest)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs LIB_DEPENDS RESOURCES)
    cmake_parse_arguments(MOCOTEST
            "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    add_executable(${MOCOTEST_NAME} ${MOCOTEST_NAME}.cpp Testing.h)
    # To organize targets in Visual Studio's Solution Explorer.
    set_target_properties(${MOCOTEST_NAME} PROPERTIES FOLDER "Moco/Tests")
    target_link_libraries(${MOCOTEST_NAME} osimMoco ${MOCOTEST_LIB_DEPENDS})
    target_include_directories(${MOCOTEST_NAME}
            PRIVATE "${CMAKE_SOURCE_DIR}/tropter/external/catch")
    add_test(NAME ${MOCOTEST_NAME} COMMAND ${MOCOTEST_NAME})
    # Tests likely run faster if we parallelize the tests rather than the
    # individual problems.
    set_property(TEST ${MOCOTEST_NAME} APPEND PROPERTY
            ENVIRONMENT "OPENSIM_MOCO_PARALLEL=0")
    file(COPY ${MOCOTEST_RESOURCES} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()

MocoAddTest(NAME testMocoInterface)

MocoAddTest(NAME testTableProcessor)

MocoAddTest(NAME testModelProcessor)

MocoAddTest(NAME testMocoGoals)

MocoAddTest(NAME testMocoParameters)

MocoAddTest(NAME testImplicit)

MocoAddTest(NAME testConstraints)

MocoAddTest(NAME testContact)

MocoAddTest(NAME testMocoActuators)

MocoAddTest(NAME testMocoInverse RESOURCES
        subject_walk_armless_18musc.osim
        subject_walk_armless_coordinates.mot
        subject_walk_armless_grfs.mot
        subject_walk_armless_external_loads.xml
        std_testMocoInverse_subject_18musc_solution.sto
        )

MocoAddTest(NAME testMocoTrack RESOURCES
        ../Archive/Tests/testGait10dof18musc_subject01.osim
        walk_gait1018_state_reference.mot
        walk_gait1018_subject01_grf.xml
        walk_gait1018_subject01_grf.mot
        std_testMocoTrackGait10dof18musc_solution.sto
        )

# MocoAddTest(NAME testMultivariatePolynomial)

MocoAddTest(NAME testMocoAnalytic)
