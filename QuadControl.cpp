#include "Common.h"
#include "QuadControl.h"

#include "Utility/SimpleConfig.h"

#include "Utility/StringUtils.h"
#include "Trajectory.h"
#include "BaseController.h"
#include "Math/Mat3x3F.h"

#ifdef __PX4_NUTTX
#include <systemlib/param/param.h>
#endif

void QuadControl::Init()
{
  BaseController::Init();

  // variables needed for integral control
  integratedAltitudeError = 0;
    
#ifndef __PX4_NUTTX
  // Load params from simulator parameter system
  ParamsHandle config = SimpleConfig::GetInstance();
   
  // Load parameters (default to 0)
  kpPosXY = config->Get(_config+".kpPosXY", 0);
  kpPosZ = config->Get(_config + ".kpPosZ", 0);
  KiPosZ = config->Get(_config + ".KiPosZ", 0);
     
  kpVelXY = config->Get(_config + ".kpVelXY", 0);
  kpVelZ = config->Get(_config + ".kpVelZ", 0);

  kpBank = config->Get(_config + ".kpBank", 0);
  kpYaw = config->Get(_config + ".kpYaw", 0);

  kpPQR = config->Get(_config + ".kpPQR", V3F());

  maxDescentRate = config->Get(_config + ".maxDescentRate", 100);
  maxAscentRate = config->Get(_config + ".maxAscentRate", 100);
  maxSpeedXY = config->Get(_config + ".maxSpeedXY", 100);
  maxAccelXY = config->Get(_config + ".maxHorizAccel", 100);

  maxTiltAngle = config->Get(_config + ".maxTiltAngle", 100);

  minMotorThrust = config->Get(_config + ".minMotorThrust", 0);
  maxMotorThrust = config->Get(_config + ".maxMotorThrust", 100);
#else
  // load params from PX4 parameter system
  //TODO
  param_get(param_find("MC_PITCH_P"), &Kp_bank);
  param_get(param_find("MC_YAW_P"), &Kp_yaw);
#endif
}

VehicleCommand QuadControl::GenerateMotorCommands(float collThrustCmd, V3F momentCmd)
{
  // Convert a desired 3-axis moment and collective thrust command to 
  //   individual motor thrust commands
  // INPUTS: 
  //   collThrustCmd: desired collective thrust [N]
  //   momentCmd: desired rotation moment about each axis [N m]
  // OUTPUT:
  //   set class member variable cmd (class variable for graphing) where
  //   cmd.desiredThrustsN[0..3]: motor commands, in [N]

  // HINTS: 
  // - you can access parts of momentCmd via e.g. momentCmd.x
  // You'll need the arm length parameter L, and the drag/thrust ratio kappa

  ////////////////////////////// BEGIN STUDENT CODE ///////////////////////////
    //Bar = Desired Acceleration
  float StraightLength = L / sqrt(2);
  float C_Bar = collThrustCmd;
  float P_Bar =  momentCmd[0] / (StraightLength);
  float Q_Bar =  momentCmd[1] / (StraightLength);
  float R_Bar = -momentCmd[2] / (kappa);

  //using the inverse of tha propeller matrix...
  float omega_4 = (C_Bar + P_Bar + Q_Bar + R_Bar) * .25 ;
  float omega_3 = (C_Bar - P_Bar + Q_Bar - R_Bar) * .25;
  float omega_2 = (C_Bar - P_Bar - Q_Bar + R_Bar) * .25;
  float omega_1 = (C_Bar + P_Bar - Q_Bar - R_Bar) * .25;
    

  cmd.desiredThrustsN[0] = omega_4; //Front Left
  cmd.desiredThrustsN[1] = omega_3; //Front Right
  cmd.desiredThrustsN[2] = omega_1; //Rear Left
  cmd.desiredThrustsN[3] = omega_2; //Rear Right

  /////////////////////////////// END STUDENT CODE ////////////////////////////

  return cmd;
}



V3F QuadControl::BodyRateControl(V3F pqrCmd, V3F pqr)
{
  // Calculate a desired 3-axis moment given a desired and current body rate
  // INPUTS: 
  //   pqrCmd: desired body rates [rad/s]
  //   pqr: current or estimated body rates [rad/s]
  // OUTPUT:
  //   return a V3F containing the desired moments for each of the 3 axes

  // HINTS: 
  //  - you can use V3Fs just like scalars: V3F a(1,1,1), b(2,3,4), c; c=a-b;
  //  - you'll need parameters for moments of inertia Ixx, Iyy, Izz
  //  - you'll also need the gain parameter kpPQR (it's a V3F)

  V3F momentCmd;

  ////////////////////////////// BEGIN STUDENT CODE ///////////////////////////
  V3F ErrorPQR;
  V3F u_barPQR;

  // 0 = p
  ErrorPQR[0] = pqrCmd[0] - pqr[0];
  u_barPQR[0] = ErrorPQR[0] * kpPQR[0];

  // 1 = q
  ErrorPQR[1] = pqrCmd[1] - pqr[1];
  u_barPQR[1] = ErrorPQR[1] * kpPQR[1];

  // 2 = r
  ErrorPQR[2] = pqrCmd[2] - pqr[2];
  u_barPQR[2] = ErrorPQR[2] * kpPQR[2]; 


    
  momentCmd = { Ixx * u_barPQR[0], Iyy * u_barPQR[1], Izz * u_barPQR[2] };

  /////////////////////////////// END STUDENT CODE ////////////////////////////

  return momentCmd;
}

// returns a desired roll and pitch rate 
V3F QuadControl::RollPitchControl(V3F accelCmd, Quaternion<float> attitude, float collThrustCmd)
{
  // Calculate a desired pitch and roll angle rates based on a desired global
  //   lateral acceleration, the current attitude of the quad, and desired
  //   collective thrust command
  // INPUTS: 
  //   accelCmd: desired acceleration in global XY coordinates [m/s2]
  //   attitude: current or estimated attitude of the vehicle
  //   collThrustCmd: desired collective thrust of the quad [N]
  // OUTPUT:
  //   return a V3F containing the desired pitch and roll rates. The Z
  //     element of the V3F should be left at its default value (0)

  // HINTS: 
  //  - we already provide rotation matrix R: to get element R[1,2] (python) use R(1,2) (C++)
  //  - you'll need the roll/pitch gain kpBank
  //  - collThrustCmd is a force in Newtons! You'll likely want to convert it to acceleration first

  V3F pqrCmd;
  Mat3x3F R = attitude.RotationMatrix_IwrtB();

  ////////////////////////////// BEGIN STUDENT CODE ///////////////////////////

    float c = collThrustCmd / mass;

    float bodyRefX = R(0, 2);
    float bodyRefXError = CONSTRAIN(accelCmd[0] / c, -maxTiltAngle, maxTiltAngle) - bodyRefX;
    float bodyXComamandedDot = bodyRefXError * kpBank;

    float bodyRefY = R(1, 2);
    float bodyRefYError = CONSTRAIN(accelCmd[1] / c, -maxTiltAngle, maxTiltAngle) - bodyRefY;
    float bodyYComamandedDot = bodyRefYError * kpBank;
  
    if (c < 0) {
        bodyXComamandedDot = kpBank * -bodyRefX;
        bodyYComamandedDot = kpBank * -bodyRefY;
    }

  float Commanded_P = 0;
  float Commanded_Q = 0;

   Commanded_P = (R(1, 0) * bodyXComamandedDot + -R(0, 0) * bodyYComamandedDot) / R(2, 2);
   Commanded_Q = (R(1, 1) * bodyXComamandedDot + -R(0, 1) * bodyYComamandedDot) / R(2, 2);
  
  pqrCmd[0] = Commanded_P;
  pqrCmd[1] = Commanded_Q;
  

  /////////////////////////////// END STUDENT CODE ////////////////////////////

  return pqrCmd;
}

float QuadControl::AltitudeControl(float posZCmd, float velZCmd, float posZ, float velZ, Quaternion<float> attitude, float accelZCmd, float dt)
{
  // Calculate desired quad thrust based on altitude setpoint, actual altitude,
  //   vertical velocity setpoint, actual vertical velocity, and a vertical 
  //   acceleration feed-forward command
  // INPUTS: 
  //   posZCmd, velZCmd: desired vertical position and velocity in NED [m]
  //   posZ, velZ: current vertical position and velocity in NED [m]
  //   accelZCmd: feed-forward vertical acceleration in NED [m/s2]
  //   dt: the time step of the measurements [seconds]
  // OUTPUT:
  //   return a collective thrust command in [N]

  // HINTS: 
  //  - we already provide rotation matrix R: to get element R[1,2] (python) use R(1,2) (C++)
  //  - you'll need the gain parameters kpPosZ and kpVelZ
  //  - maxAscentRate and maxDescentRate are maximum vertical speeds. Note they're both >=0!
  //  - make sure to return a force, not an acceleration
  //  - remember that for an upright quad in NED, thrust should be HIGHER if the desired Z acceleration is LOWER

  Mat3x3F R = attitude.RotationMatrix_IwrtB();
  float thrust = 0;

  ////////////////////////////// BEGIN STUDENT CODE ///////////////////////////
  float Z_Error = posZCmd - posZ;
  float Z_Error_dot = velZCmd - velZ;
  float Body_Z = R(2, 2);
  integratedAltitudeError = integratedAltitudeError + Z_Error * dt;

  float P_controller = kpPosZ * Z_Error;
  float PD_controller = kpVelZ * Z_Error_dot;

  float PI_Controller = KiPosZ * integratedAltitudeError;

  float u1_bar = P_controller + PD_controller + accelZCmd + PI_Controller;

  float Z_acc = (u1_bar - 9.81f) / Body_Z;

  thrust = CONSTRAIN(Z_acc, -maxAscentRate / dt, maxAscentRate / dt) * mass;
  /////////////////////////////// END STUDENT CODE ////////////////////////////
  
  return -thrust;
}

// returns a desired acceleration in global frame
V3F QuadControl::LateralPositionControl(V3F posCmd, V3F velCmd, V3F pos, V3F vel, V3F accelCmdFF)
{
  // Calculate a desired horizontal acceleration based on 
  //  desired lateral position/velocity/acceleration and current pose
  // INPUTS: 
  //   posCmd: desired position, in NED [m]
  //   velCmd: desired velocity, in NED [m/s]
  //   pos: current position, NED [m]
  //   vel: current velocity, NED [m/s]
  //   accelCmdFF: feed-forward acceleration, NED [m/s2]
  // OUTPUT:
  //   return a V3F with desired horizontal accelerations. 
  //     the Z component should be 0
  // HINTS: 
  //  - use the gain parameters kpPosXY and kpVelXY
  //  - make sure you limit the maximum horizontal velocity and acceleration
  //    to maxSpeedXY and maxAccelXY

  // make sure we don't have any incoming z-component
  accelCmdFF.z = 0;
  velCmd.z = 0;
  posCmd.z = pos.z;

  // we initialize the returned desired acceleration to the feed-forward value.
  // Make sure to _add_, not simply replace, the result of your controller
  // to this variable
  V3F accelCmd = accelCmdFF;

  ////////////////////////////// BEGIN STUDENT CODE ///////////////////////////
  float X_Error = posCmd.x - pos.x;
  float X_Error_Dot = velCmd.x - vel.x;
  float Y_Error = posCmd.y - pos.y;
  float Y_Error_Dot = velCmd.y - vel.y;

  //if (X_Error < .005 && X_Error > -.005 && X_Error_Dot < .005 && X_Error_Dot > -.005) {
  //    X_Error = 0;
  //    X_Error_Dot = 0;
  //}

  //if (Y_Error < .065 && Y_Error > -.065) {
  //    Y_Error = 0;
  //}

  //if (Y_Error_Dot < .1 && Y_Error_Dot > -.1) {
  //    Y_Error_Dot = 0;
  //}

  float X_PController = X_Error * kpPosXY;
  float X_PDController = X_Error_Dot * kpVelXY;
  float Y_PController = Y_Error * kpPosXY;
  float Y_PDController = Y_Error_Dot * kpVelXY;

  float X_Dot_Dot_Command = accelCmd[0] + X_PController + X_PDController;
  float Y_Dot_Dot_Command = accelCmd[1] + Y_PController + Y_PDController;
  
  accelCmd.x = -CONSTRAIN(X_Dot_Dot_Command, -maxAccelXY, maxAccelXY);
  accelCmd.y = -CONSTRAIN(Y_Dot_Dot_Command, -maxAccelXY, maxAccelXY);

  /////////////////////////////// END STUDENT CODE ////////////////////////////

  return accelCmd;
}

// returns desired yaw rate
float QuadControl::YawControl(float yawCmd, float yaw)
{
  // Calculate a desired yaw rate to control yaw to yawCmd
  // INPUTS: 
  //   yawCmd: commanded yaw [rad]
  //   yaw: current yaw [rad]
  // OUTPUT:
  //   return a desired yaw rate [rad/s]
  // HINTS: 
  //  - use fmodf(foo,b) to unwrap a radian angle measure float foo to range [0,b]. 
  //  - use the yaw control gain parameter kpYaw

  float yawRateCmd=0;
  ////////////////////////////// BEGIN STUDENT CODE ///////////////////////////
  yawCmd = fmod(yawCmd, (2.0f * F_PI));

   if (yawCmd > 0) {
      if (yaw >= -F_PI && yaw <= -F_PI / 2)
      {
          yaw += (2.0f * F_PI);
      }
    }
   if(yawCmd < 0){
      if (yaw <= F_PI && yaw >= F_PI / 2)
      {
          yaw -= (2.0f * F_PI);
      }
   }

  yawRateCmd = kpYaw * (yawCmd - yaw);

  //if (yawRateCmd < 0 && kpYaw < 0 && yaw < 0) {
  //    yawRateCmd = yawRateCmd * -1;
  //}

  /////////////////////////////// END STUDENT CODE ////////////////////////////

  return yawRateCmd;

}

VehicleCommand QuadControl::RunControl(float dt, float simTime)
{
  curTrajPoint = GetNextTrajectoryPoint(simTime);

  float collThrustCmd = AltitudeControl(curTrajPoint.position.z, curTrajPoint.velocity.z, estPos.z, estVel.z, estAtt, curTrajPoint.accel.z, dt);

  // reserve some thrust margin for angle control
  float thrustMargin = .1f*(maxMotorThrust - minMotorThrust);
  collThrustCmd = CONSTRAIN(collThrustCmd, (minMotorThrust+ thrustMargin)*4.f, (maxMotorThrust-thrustMargin)*4.f);
  
  V3F desAcc = LateralPositionControl(curTrajPoint.position, curTrajPoint.velocity, estPos, estVel, curTrajPoint.accel);
  
  V3F desOmega = RollPitchControl(desAcc, estAtt, collThrustCmd);
  desOmega.z = YawControl(curTrajPoint.attitude.Yaw(), estAtt.Yaw());

  V3F desMoment = BodyRateControl(desOmega, estOmega);

  return GenerateMotorCommands(collThrustCmd, desMoment);
}
