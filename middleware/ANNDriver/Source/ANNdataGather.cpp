/***************************************************************************
 
    file                 : SimpleDriver.cpp
    copyright            : (C) 2007 Daniele Loiacono
 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ANNdataGather.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

/* Gear Changing Constants*/
const int ANNdataGather::gearUp[6] =
    {
        5000,6000,6000,6500,7000,0
    };
const int ANNdataGather::gearDown[6] =
    {
        0,2500,3000,3000,3500,3500
    };

/* Stuck constants*/
const int ANNdataGather::stuckTime = 25;
const float ANNdataGather::stuckAngle = .523598775; //PI/6

/* Accel and Brake Constants*/
const float ANNdataGather::maxSpeedDist = 70;
const float ANNdataGather::maxSpeed = 150;
const float ANNdataGather::sin5 = 0.08716;
const float ANNdataGather::cos5 = 0.99619;

/* Steering constants*/
const float ANNdataGather::steerLock = 0.366519;
const float ANNdataGather::steerSensitivityOffset = 80.0;
const float ANNdataGather::wheelSensitivityCoeff = 1;

/* ABS Filter Constants */
const float ANNdataGather::wheelRadius[4] = { 0.3306, 0.3306, 0.3276, 0.3276 };
const float ANNdataGather::absSlip = 2.0;
const float ANNdataGather::absRange = 3.0;
const float ANNdataGather::absMinSpeed = 3.0;

/* Clutch constants */
const float ANNdataGather::clutchMax = 0.5;
const float ANNdataGather::clutchDelta = 0.05;
const float ANNdataGather::clutchRange = 0.82;
const float ANNdataGather::clutchDeltaTime = 0.02;
const float ANNdataGather::clutchDeltaRaced=10;
const float ANNdataGather::clutchDec=0.01;
const float ANNdataGather::clutchMaxModifier=1.3;
const float ANNdataGather::clutchMaxTime=1.5;


int
ANNdataGather::getGear(CarState &cs)
{

    int gear = cs.getGear();
    int rpm  = cs.getRpm();

    // if gear is 0 (N) or -1 (R) just return 1 
    if (gear<1)
        return 1;
    // check if the RPM value of car is greater than the one suggested 
    // to shift up the gear from the current one     
    if (gear <6 && rpm >= gearUp[gear-1])
        return gear + 1;
    else
    	// check if the RPM value of car is lower than the one suggested 
    	// to shift down the gear from the current one
        if (gear > 1 && rpm <= gearDown[gear-1])
            return gear - 1;
        else // otherwhise keep current gear
            return gear;
}

float
ANNdataGather::getSteer(CarState &cs)
{
	// steering angle is compute by correcting the actual car angle w.r.t. to track 
	// axis [cs.getAngle()] and to adjust car position w.r.t to middle of track [cs.getTrackPos()*0.5]
    float targetAngle=(cs.getAngle()-cs.getTrackPos()*0.5);
    // at high speed reduce the steering command to avoid loosing the control
    if (cs.getSpeedX() > steerSensitivityOffset)
        return targetAngle/(steerLock*(cs.getSpeedX()-steerSensitivityOffset)*wheelSensitivityCoeff);
    else
        return (targetAngle)/steerLock;

}
float
ANNdataGather::getAccel(CarState &cs)
{
    // checks if car is out of track
    if (cs.getTrackPos() < 1 && cs.getTrackPos() > -1)
    {
        // reading of sensor at +5 degree w.r.t. car axis
        float rxSensor=cs.getTrack(10);
        // reading of sensor parallel to car axis
        float cSensor=cs.getTrack(9);
        // reading of sensor at -5 degree w.r.t. car axis
        float sxSensor=cs.getTrack(8);

        float targetSpeed;

        // track is straight and enough far from a turn so goes to max speed
        if (cSensor>maxSpeedDist || (cSensor>=rxSensor && cSensor >= sxSensor))
            targetSpeed = maxSpeed;
        else
        {
            // approaching a turn on right
            if(rxSensor>sxSensor)
            {
                // computing approximately the "angle" of turn
                float h = cSensor*sin5;
                float b = rxSensor - cSensor*cos5;
                float sinAngle = b*b/(h*h+b*b);
                // estimate the target speed depending on turn and on how close it is
                targetSpeed = maxSpeed*(cSensor*sinAngle/maxSpeedDist);
            }
            // approaching a turn on left
            else
            {
                // computing approximately the "angle" of turn
                float h = cSensor*sin5;
                float b = sxSensor - cSensor*cos5;
                float sinAngle = b*b/(h*h+b*b);
                // estimate the target speed depending on turn and on how close it is
                targetSpeed = maxSpeed*(cSensor*sinAngle/maxSpeedDist);
            }

        }

        // accel/brake command is expontially scaled w.r.t. the difference between target speed and current one
        return 2/(1+exp(cs.getSpeedX() - targetSpeed)) - 1;
    }
    else
        return 0.3; // when out of track returns a moderate acceleration command

}

CarControl
ANNdataGather::wDrive(CarState cs)
{
	// check if car is currently stuck
	if ( fabs(cs.getAngle()) > stuckAngle )
    {
		// update stuck counter
        stuck++;
    }
    else
    {
    	// if not stuck reset stuck counter
        stuck = 0;
    }

	// after car is stuck for a while apply recovering policy
    if (stuck > stuckTime)
    {
    	/* set gear and sterring command assuming car is 
    	 * pointing in a direction out of track */
    	
    	// to bring car parallel to track axis
        float steer = - cs.getAngle() / steerLock; 
        int gear=-1; // gear R
        
        // if car is pointing in the correct direction revert gear and steer  
        if (cs.getAngle()*cs.getTrackPos()>0)
        {
            gear = 1;
            steer = -steer;
        }

        // Calculate clutching
        clutching(cs,clutch);

        // build a CarControl variable and return it
        CarControl cc (1.0,0.0,gear,steer,clutch);
        return cc;
    }

    else // car is not stuck
    {
    	// compute accel/brake command
        float accel_and_brake = getAccel(cs);
        // compute gear 
        int gear = getGear(cs);
        // compute steering
        float steer = getSteer(cs);
        

        // normalize steering
        if (steer < -1)
            steer = -1;
        if (steer > 1)
            steer = 1;
        
        // set accel and brake from the joint accel/brake command 
        float accel,brake;
        if (accel_and_brake>0)
        {
            accel = accel_and_brake;
            brake = 0;
        }
        else
        {
            accel = 0;
            // apply ABS to brake
            brake = filterABS(cs,-accel_and_brake);
        }

        // Calculate clutching
        clutching(cs,clutch);

        // build a CarControl variable and return it
        CarControl cc(accel,brake,gear,steer,clutch);

		// Record input - output
		ofstream myfile("ANNtraining.txt", std::ios_base::app);
		

		// Needs to consider standardising input values for better networks
		// Inputs from CarState
		// Check TORCS competition manual for description of all possible inputs
		// Performance is much better if you normalise as much as possible inputs (between -1 and 1, or 0 to 1)
		myfile << cs.getAngle() << " " << cs.getTrackPos() << "\n";

		// Network outputs should be between -1 and 1!
		// Outputs from CarControl
		// Check TORCS competition manual for description of all possible outputs
		myfile << accel << " " << brake << " " << steer << "\n";

		myfile.close();

		samples++; // counting the number of samples gathered in current training

		return cc;
    }
}

float
ANNdataGather::filterABS(CarState &cs,float brake)
{
	// convert speed to m/s
	float speed = cs.getSpeedX() / 3.6;
	// when spedd lower than min speed for abs do nothing
    if (speed < absMinSpeed)
        return brake;
    
    // compute the speed of wheels in m/s
    float slip = 0.0f;
    for (int i = 0; i < 4; i++)
    {
        slip += cs.getWheelSpinVel(i) * wheelRadius[i];
    }
    // slip is the difference between actual speed of car and average speed of wheels
    slip = speed - slip/4.0f;
    // when slip too high applu ABS
    if (slip > absSlip)
    {
        brake = brake - (slip - absSlip)/absRange;
    }
    
    // check brake is not negative, otherwise set it to zero
    if (brake<0)
    	return 0;
    else
    	return brake;
}

void
ANNdataGather::onShutdown()
{
    cout << "Bye bye!" << endl;
	// Network topology // [number of training cases, number of inputs, number of outputs]
	ifstream myfile("ANNtraining.txt");
	ofstream myfinalfile("ANNtrainingFinal.txt");

	myfinalfile << samples << " 2 3\n";
	myfinalfile << myfile.rdbuf();

	myfile.close();
	myfinalfile.close();
	std::remove("ANNtraining.txt");
	std::rename("ANNtrainingFinal.txt", "ANNtraining.txt");
}

void
ANNdataGather::onRestart()
{
    cout << "Restarting the race!" << endl;
}

void
ANNdataGather::clutching(CarState &cs, float &clutch)
{
  double maxClutch = clutchMax;

  // Check if the current situation is the race start
  if (cs.getCurLapTime()<clutchDeltaTime  && stage==RACE && cs.getDistRaced()<clutchDeltaRaced)
    clutch = maxClutch;

  // Adjust the current value of the clutch
  if(clutch > 0)
  {
    double delta = clutchDelta;
    if (cs.getGear() < 2)
	{
      // Apply a stronger clutch output when the gear is one and the race is just started
	  delta /= 2;
      maxClutch *= clutchMaxModifier;
      if (cs.getCurLapTime() < clutchMaxTime)
        clutch = maxClutch;
	}

    // check clutch is not bigger than maximum values
	clutch = min(maxClutch,double(clutch));

	// if clutch is not at max value decrease it quite quickly
	if (clutch!=maxClutch)
	{
	  clutch -= delta;
	  clutch = max(0.0,double(clutch));
	}
	// if clutch is at max value decrease it very slowly
	else
		clutch -= clutchDec;
  }
}

void
ANNdataGather::init(float *angles)
{

	// set angles as {-90,-75,-60,-45,-30,20,15,10,5,0,5,10,15,20,30,45,60,75,90}

	for (int i=0; i<5; i++)
	{
		angles[i]=-90+i*15;
		angles[18-i]=90-i*15;
	}

	for (int i=5; i<9; i++)
	{
			angles[i]=-20+(i-5)*5;
			angles[18-i]=20-(i-5)*5;
	}
	angles[9]=0;
}
