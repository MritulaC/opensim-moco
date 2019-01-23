/* -------------------------------------------------------------------------- *
 * OpenSim Moco: sandboxSitToStand.cpp                                        *
 * -------------------------------------------------------------------------- *
 * Copyright (c) 2017 Stanford University and the Authors                     *
 *                                                                            *
 * Author(s): Nicholas Bianco                                                 *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0          *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include <Moco/osimMoco.h>
#include <Moco/InverseMuscleSolver/GlobalStaticOptimization.h>
#include <OpenSim/Common/osimCommon.h>
#include <OpenSim/Simulation/osimSimulation.h>
#include <OpenSim/Actuators/osimActuators.h>
#include <OpenSim/Simulation/SimbodyEngine/PointOnLineConstraint.h>

using namespace OpenSim;
using SimTK::Vec3;

/// Convenience function to apply an CoordinateActuator to the model.
void addCoordinateActuator(Model& model, std::string coordName,
    double optimalForce) {

    auto& coordSet = model.updCoordinateSet();

    auto* actu = new CoordinateActuator();
    actu->setName("tau_" + coordName);
    actu->setCoordinate(&coordSet.get(coordName));
    actu->setOptimalForce(1);
    actu->setMinControl(-optimalForce);
    actu->setMaxControl(optimalForce);
    model.addComponent(actu);
}

/// This essentially removes the effect of passive muscle fiber forces from the 
/// model.
void minimizePassiveFiberForces(Model& model) {
    const auto& muscleSet = model.getMuscles();
    Array<std::string> muscNames;
    muscleSet.getNames(muscNames);
    for (int i = 0; i < muscNames.size(); ++i) {
        const auto& name = muscNames.get(i);
        FiberForceLengthCurve fflc(
            model.getComponent<Millard2012EquilibriumMuscle>(
                "/forceset/" + name).getFiberForceLengthCurve());
        fflc.set_strain_at_one_norm_force(100000);
        fflc.set_stiffness_at_low_force(0.00000001);
        fflc.set_stiffness_at_one_norm_force(0.0001);
        fflc.set_curviness(0);
        model.updComponent<Millard2012EquilibriumMuscle>(
            "/forceset/" + name).setFiberForceLengthCurve(fflc);
    }
}

/// Create a right-leg only model with the following options:
///     1) actuatorType: "torques" or "muscles"
Model createRightLegModel(const std::string& actuatorType) {
    Model model("Rajagopal2015_right_leg_9musc_bottom_up.osim");
    model.finalizeConnections(); // Need this here to access offset frames.

    // Replace subtalar_r and mtp_r joints with weld joints.
    replaceJointWithWeldJoint(model, "subtalar_r");
    replaceJointWithWeldJoint(model, "mtp_r");

    // Replace hip_r ball joint and replace with pin joint.
    auto& hip_r = model.updJointSet().get("hip_r");
    PhysicalOffsetFrame* femur_r_offset_hip_r
        = PhysicalOffsetFrame().safeDownCast(hip_r.getParentFrame().clone());
    PhysicalOffsetFrame* pelvis_offset_hip_r
        = PhysicalOffsetFrame().safeDownCast(hip_r.getChildFrame().clone());
    model.updJointSet().remove(&hip_r);
    auto* hip_r_pin = new PinJoint("hip_r",
        model.getBodySet().get("femur_r"),
        femur_r_offset_hip_r->get_translation(),
        femur_r_offset_hip_r->get_orientation(),
        model.getBodySet().get("pelvis"),
        pelvis_offset_hip_r->get_translation(),
        pelvis_offset_hip_r->get_orientation());
    hip_r_pin->updCoordinate().setName("hip_flexion_r");
    model.addJoint(hip_r_pin);

    // Make pelvis heavier to simulate torso.
    //model.updBodySet().get("pelvis").setMass(10.0);

    if (actuatorType == "torques") {
        // Remove muscles and add coordinate actuators.
        addCoordinateActuator(model, "hip_flexion_r", 50);
        addCoordinateActuator(model, "knee_angle_r", 50);
        addCoordinateActuator(model, "ankle_angle_r", 50);
        removeMuscles(model);
    }
    else if (actuatorType == "muscles") {
        // Strengthen muscles.
        auto& muscSet = model.updMuscles();
        double factor = 10.0;
        for (int i = 0; i < muscSet.getSize(); ++i) {
            auto& musc = muscSet.get(i);
            musc.setMaxIsometricForce(factor*musc.getMaxIsometricForce());
        }

        // Remove effect of passive fiber forces.
        minimizePassiveFiberForces(model);
    }
    else {
        OPENSIM_THROW(Exception, "Invalid actuator type");
    }

    // Finalize model and print.
    model.finalizeFromProperties();
    model.finalizeConnections();
    model.print("RightLeg_" + actuatorType + ".osim");

    return model;
}

struct Options {
    std::string actuatorType = "torques";
    int num_mesh_points = 10;
    double convergence_tol = 1e-2;
    double constraint_tol = 1e-2;
    int max_iterations = 100000;
    std::string solver = "snopt";
    std::string dynamics_mode = "explicit";
    std::string hessian_approx = "limited-memory";
    TimeSeriesTable controlsGuess = {};
    MocoIterate previousSolution = {};
};

MocoSolution minimizeControlEffortRightLeg(const Options& opt) {
    MocoTool moco;
    moco.setName("sandboxRightLeg_" + opt.actuatorType + 
        "_minimize_control_effort");
    MocoProblem& mp = moco.updProblem();
    Model model = createRightLegModel(opt.actuatorType);
    mp.setModelCopy(model);

    // Set bounds.
    mp.setTimeBounds(0, 1);
    mp.setStateInfo("/jointset/hip_r/hip_flexion_r/value", {-1, 1},
        -1, 0);
    mp.setStateInfo("/jointset/walker_knee_r/knee_angle_r/value", {-3, 0}, 
        -SimTK::Pi / 1.5, 0);
    mp.setStateInfo("/jointset/ankle_r/ankle_angle_r/value", {-1, 1}, 
        -SimTK::Pi / 4.0, 0);

    if (opt.actuatorType == "muscles") {
        for (int i = 0; i < model.getMuscles().getSize(); ++i) {
            auto muscPath = model.getMuscles().get(i).getAbsolutePathString();
            mp.setControlInfo(muscPath, {0.02, 1});
        }
    }

    auto* effort = mp.addCost<MocoControlCost>();
    effort->setName("control_effort");

    MocoTropterSolver& ms = moco.initSolver();
    ms.set_num_mesh_points(opt.num_mesh_points);
    ms.set_verbosity(2);
    ms.set_dynamics_mode(opt.dynamics_mode);
    ms.set_optim_convergence_tolerance(opt.convergence_tol);
    ms.set_optim_constraint_tolerance(opt.constraint_tol);
    ms.set_optim_solver(opt.solver);
    if (opt.solver == "ipopt") {
        ms.set_optim_hessian_approximation(opt.hessian_approx);
    }
    ms.set_transcription_scheme("hermite-simpson");
    ms.set_optim_max_iterations(opt.max_iterations);
    ms.set_enforce_constraint_derivatives(true);
    ms.set_minimize_lagrange_multipliers(true);
    ms.set_lagrange_multiplier_weight(10);
    ms.set_velocity_correction_bounds({-1, 1});
    if (opt.previousSolution.empty()) {
        auto guess = ms.createGuess("bounds");
        // If the controlsGuess struct field is not empty, use it to set the
        // controls in the trajectory guess.
        if (opt.controlsGuess.getMatrix().nrow() != 0) {
            for (const auto& label : opt.controlsGuess.getColumnLabels()) {
                // Get the 
                SimTK::Vector controlGuess =
                    opt.controlsGuess.getDependentColumn(label);
                // Interpolate controls guess to correct length.
                SimTK::Vector prevTime = createVectorLinspace(
                    opt.controlsGuess.getMatrix().nrow(), 0, 1);
                auto controlGuessInterp = interpolate(prevTime, controlGuess,
                    guess.getTime());
                // Set the guess for this control.
                guess.setControl(label, controlGuessInterp);
            }
        }
        ms.setGuess(guess);
    } else {
        ms.setGuess(opt.previousSolution);
    }

    MocoSolution solution = moco.solve().unseal();
    moco.visualize(solution);

    return solution;
}

TimeSeriesTable createGuessFromGSO(const MocoSolution& torqueSolution,
    const Options& opt) {

    GlobalStaticOptimization gso;
    Model model = createRightLegModel("muscles");
    gso.setModel(model);
    gso.setKinematicsData(torqueSolution.exportToStatesTable());

    // Create the net generalized force data for the GS0 problem.
    // ----------------------------------------------------------

    // The net generalized forces must be provided as a TimeSeriesTable,
    // which requires a std::vector<double> time vector.
    const auto& timeVec = torqueSolution.getTime();
    std::vector<double> time;
    for (int i = 0; i < timeVec.size(); ++i) {
        time.push_back(timeVec[i]);
    }

    // Create the labels for the net generalized forces, which are the absolute
    // path names for the model coordinates.
    const auto& coordSet = model.getCoordinateSet();
    std::vector<std::string> coordNames;
    for (int i = 0; i < coordSet.getSize(); ++i) {
        std::string coordNameFullPath =
            coordSet.get(i).getAbsolutePathString();
        // Skip the knee_angle_r_beta coordinate, which is uniquely enforced
        // via the coordinate coupler constraint.
        if (coordNameFullPath.find("knee_angle_r_beta") == std::string::npos) {
            coordNames.push_back(coordNameFullPath);
            std::cout << coordNameFullPath << std::endl;
        }
       
    }

    std::cout << torqueSolution.getControlsTrajectory().ncol() << std::endl;

    // Create the net generalized forces table and pass it to the GSO problem.
    TimeSeriesTable netGenForces(time, torqueSolution.getControlsTrajectory(),
        coordNames);
    gso.setNetGeneralizedForcesData(netGenForces);
    // Filter the net generalized forces to ensure smooth control solutions.
    gso.set_lowpass_cutoff_frequency_for_joint_moments(10);

    // Solve.
    GlobalStaticOptimization::Solution gsoSol = gso.solve();

    // Get muscle activations.
    TimeSeriesTable controls = gsoSol.activation;

    // Append other controls.
    if (gsoSol.other_controls.hasColumnLabels()) {
        for (const auto& name : gsoSol.other_controls.getColumnLabels()) {
            controls.appendColumn(name,
                gsoSol.other_controls.getDependentColumn(name));
        }
    }
    return controls;
}

MocoSolution stateTrackingRightLeg(const Options& opt) {
    MocoTool moco;
    moco.setName("sandboxRightLeg_" + opt.actuatorType + "_state_tracking");
    MocoProblem& mp = moco.updProblem();
    Model model = createRightLegModel(opt.actuatorType);
    mp.setModelCopy(model);

    // Get previous solution.
    MocoIterate prevSol = opt.previousSolution;

    // Get states trajectory from previous solution. Need to set the problem
    // model and call initSystem() to create the table internally.
    model.initSystem();
    TimeSeriesTable prevStateTraj =
        prevSol.exportToStatesTrajectory(mp).exportToTable(model);

    const auto& prevTime = prevSol.getTime();
    mp.setTimeBounds(prevTime[0], prevTime[prevTime.size() - 1]);
    mp.setStateInfo("/jointset/hip_r/hip_flexion_r/value", {-1, 1});
    mp.setStateInfo("/jointset/ankle_r/ankle_angle_r/value", {-1, 1});

    auto* tracking = mp.addCost<MocoStateTrackingCost>();
    tracking->setName("tracking");
    tracking->setReference(prevStateTraj);
    // Don't track coordinates enforced by constraints.
    tracking->setWeight("/jointset/patellofemoral_r/knee_angle_r_beta/value", 0);
    tracking->setWeight("/jointset/patellofemoral_r/knee_angle_r_beta/speed", 0);

    auto* effort = mp.addCost<MocoControlCost>();
    effort->setName("effort");

    MocoTropterSolver& ms = moco.initSolver();
    ms.set_num_mesh_points(opt.num_mesh_points);
    ms.set_verbosity(2);
    ms.set_dynamics_mode(opt.dynamics_mode);
    ms.set_optim_convergence_tolerance(opt.convergence_tol);
    ms.set_optim_constraint_tolerance(opt.constraint_tol);
    ms.set_optim_solver(opt.solver);
    ms.set_transcription_scheme("hermite-simpson");
    ms.set_optim_max_iterations(opt.max_iterations);
    ms.set_enforce_constraint_derivatives(true);
    ms.set_velocity_correction_bounds({-1, 1});
    // Need this term to recover the correct multipliers. (explicit only)
    ms.set_minimize_lagrange_multipliers(true);
    ms.set_lagrange_multiplier_weight(0.1);

    // Create guess.
    // -------------
    auto guess = ms.createGuess("bounds");
    // Controls guess.
    // If the controlsGuess struct field is not empty, use it to set the
    // controls in the trajectory guess.
    if (opt.controlsGuess.getMatrix().nrow() != 0) {
        for (const auto& label : opt.controlsGuess.getColumnLabels()) {
            // Get the 
            SimTK::Vector controlGuess =
                opt.controlsGuess.getDependentColumn(label);
            // Interpolate controls guess to correct length.
            auto controlGuessInterp = interpolate(prevTime, controlGuess,
                guess.getTime());
            // Set the guess for this control.
            guess.setControl(label, controlGuessInterp);
        }
    }
    // States guess.
    guess.setStatesTrajectory(prevStateTraj);
    ms.setGuess(guess);

    MocoSolution solution = moco.solve().unseal();
    moco.visualize(solution);

    return solution;
}

void main() {
    // Predictive problem.
    Options opt;
    opt.num_mesh_points = 10;
    opt.solver = "snopt";
    opt.dynamics_mode = "implicit";
    // Tight tolerances needed for smooth solution.
    opt.constraint_tol = 1e-6;
    opt.convergence_tol = 1e-6;
    //opt.actuatorType = "muscles";
    //opt.previousSolution = MocoSolution(
    //    "sandboxRightLeg_torques_minimize_control_effort_solution.sto");
    MocoSolution torqueSolEffort = minimizeControlEffortRightLeg(opt);
    //MocoSolution torqueSolEffort(
    //       "sandboxRightLeg_torques_minimize_control_effort_solution.sto");

    // Tracking problem.
    //opt.dynamics_mode = "explicit";
    //opt.previousSolution = torqueSolEffort;
    //opt.constraint_tol = 1e-6;
    //opt.convergence_tol = 1e-6;
    //opt.num_mesh_points = 20;
    //MocoSolution torqueSolTracking = stateTrackingRightLeg(opt);

    //std::cout << "Predictive versus tracking comparison" << std::endl;
    //std::cout << "-------------------------------------" << std::endl;
    //std::cout << "States RMS error: ";
    //std::cout <<
    //    torqueSolTracking.compareContinuousVariablesRMS(torqueSolEffort,
    //    {}, {"none"}, {"none"}, {"none"});
    //std::cout << std::endl;
    //std::cout << "Controls RMS error: ";
    //std::cout <<
    //    torqueSolTracking.compareContinuousVariablesRMS(torqueSolEffort,
    //    {"none"}, {}, {"none"}, {"none"});
    //std::cout << std::endl;

    //opt.actuatorType = "muscles";
    //opt.constraint_tol = 1e-2;
    //opt.convergence_tol = 1e-2;
    //MocoSolution muscleSolEffort = minimizeControlEffortRightLeg(opt);
}