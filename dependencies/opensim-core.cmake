# This file is included by the CMakeLists.txt in this directory.

# We list (and update) the opensim-core submodule commit here so that AppVeyor
# will invalidate its cached opensim-core installation if we change the commit.
# This commented commit hash is not actually used in the superbuild.
# opensim-core commit:
# 60dd70dc04d122569587be2aecfeaebf57e465ac

AddDependency(NAME       opensim-core
              URL        ${CMAKE_SOURCE_DIR}/../opensim-core
              CMAKE_ARGS
                    -DBUILD_API_EXAMPLES:BOOL=OFF
                    -DBUILD_TESTING:BOOL=OFF
                    -DBUILD_JAVA_WRAPPING:BOOL=${OPENSIM_JAVA_WRAPPING}
                    -DBUILD_PYTHON_WRAPPING:BOOL=${OPENSIM_PYTHON_WRAPPING}
                    -DOPENSIM_PYTHON_VERSION:STRING=${OPENSIM_PYTHON_VERSION}
                    -DOPENSIM_DEPENDENCIES_DIR:PATH=${CMAKE_INSTALL_PREFIX}
                    -DWITH_BTK:BOOL=ON
                    -DOPENSIM_INSTALL_UNIX_FHS:BOOL=OFF)


if(SUPERBUILD_opensim-core)

    # OpenSim's dependencies.
    AddDependency(NAME       BTK
                  GIT_URL    https://github.com/opensim-org/BTKCore.git
                  GIT_TAG    6d787d0be223851a8f454f2ee8c7d9e47b84cbbe
                  CMAKE_ARGS -DBUILD_SHARED_LIBS:BOOL=ON)

    AddDependency(NAME simbody
                  GIT_URL    https://github.com/simbody/simbody.git
                  GIT_TAG    Simbody-3.7
                  CMAKE_ARGS -DBUILD_EXAMPLES:BOOL=OFF 
                             -DBUILD_TESTING:BOOL=OFF)

    AddDependency(NAME       docopt
                  GIT_URL    https://github.com/docopt/docopt.cpp.git
                  GIT_TAG    3dd23e3280f213bacefdf5fcb04857bf52e90917
                  CMAKE_ARGS -DCMAKE_DEBUG_POSTFIX:STRING=_d)

    add_dependencies(opensim-core BTK simbody docopt)
endif()



