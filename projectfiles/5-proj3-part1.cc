/*
 *  CISC-3415 Robotics
 *  Project 3 - Part 1
 *  Date: 20th October 2019
 *  Credit To: Simon Parsons
 *
 ** Group Members *************************************************************
 *    
 *  Benjamin Yi
 *  Emmanuel Desdunes
 *  Montasir Omi
 *  Shahzad Ahmad
 *
 ** Description ***************************************************************
 * 
 *  This program uses the BlobfinderProxy of PlayerStage, giving the roomba
 *  access to a camera as well as a camera library that has a built in detection
 *  system for R-G-B colors. Here, we focus on only red objects, and the roomba's
 *  objective is to search for and locate a red beacon, and head towards it, then
 *  stop at about two-feet from the beacon.
 *  Since the robot must slow down based on distance from the beacon, we use
 *  a special ratio using the area of the beacon, a value which ultimately 
 *  determines the distance of the beacon from the robot. 
 *  The ratio is (1/sqrt(area))*C, where C is a value of our choice.
 */


#include <iostream>
#include <cstdlib>
#include <libplayerc++/playerc++.h>
#include <stdint.h>

int main(int argc, char *argv[]) {  
	using namespace PlayerCc;  

	// Set up proxies
	PlayerClient    robot("localhost");  
	BumperProxy     bp(&robot,0);  
	Position2dProxy pp(&robot,0);
	PlayerClient    blobfinder("localhost");
	BlobfinderProxy bf(&blobfinder,0);

	// Variables

	player_blobfinder_data_t  blobList;   // Structure holding the blobs found
	player_blobfinder_blob_t* theBlobs;   // A point to a list of blobs
	player_blobfinder_blob_t  myBlob;     // A single blob

	// Allow the program to take charge of the motors (take care now)
	pp.SetMotorEnable(true);


	// Control loop
	while(true) {    
		double turnrate, speed;

		// Read from the proxies
		robot.Read();
		blobfinder.Read();

		// If there are any blobs at all, do something
		if (bf.GetCount() > 0){
			double maxarea = 0;						  // Variable to determine largest blob area
			int maxi = -1;									// Variable for the index of the largest blob
			                                // within the array of bf.GetBlob()

      // Looks for the index of the largest red blob
			for(int i = 0; i < bf.GetCount(); i++){
				if ((short)bf.GetBlob(i).color != 0) continue;		// If the blob is not red, ignore it
				if (maxarea < bf.GetBlob(i).area) {								// If the ith blob is the biggest
					maxarea = bf.GetBlob(i).area;										// change the maxi index to the ith blob
					maxi = i;
				}
			}
			
			// If maxi is still -1, then no blob has been detected.
			if (maxi == -1) {
				std::cout << "Not found!" << std::endl;
				continue;
			}
			
			// Console output for BLOB information
			std::cout << std::endl;
			std::cout << "Id: "    << bf.GetBlob(maxi).id    << std::endl;
			std::cout << "Color: " << (short)bf.GetBlob(maxi).color << std::endl;
			std::cout << "Area: "  << bf.GetBlob(maxi).area  << std::endl;
			std::cout << "X: "     << bf.GetBlob(maxi).x     << std::endl;
			std::cout << "Y: "     << bf.GetBlob(maxi).y     << std::endl;
			std::cout << std::endl;
			
			// Ignore small, insignficant blobs, to prevent chasing random targets.
			if (maxarea < 500) continue;
			
			// Main control for roomba navigation

			// If the center of the blob is too far towards the left, then turn right
			if (bf.GetBlob(maxi).x < 140) {
				turnrate = 0.1;
			
			// If the center of the blob is too far towards the right, then turn left
			} else if (bf.GetBlob(maxi).x > 180) {
				turnrate = -0.1;
			
			// If the center of the blob is is "good" position,
			// stop turning and start moving.
			} else {
					turnrate = 0.0;
					// If the area of the largest blob is greater than X (5100),
					// we are at about two-feet from the beacon, so STOP.
					if (bf.GetBlob(maxi).area > 5100) {
						speed = 0;

					// If we are still some distance from the beacon, determine speed
					// using a special ratio.
					// Our Special Ratio:
					// -- We square root the maxarea for the purpose of speed linearity
					// -- We then invert this value because as the maxarea gets larger
					//    the speed should decrease (robot should SLOW DOWN!)
					// -- This value is multiplied by some constant, which is purely
					//    based on guessing and checking the speed to our liking
					} else {
						speed = (1/sqrt(maxarea))*4;
					}
			}

		// If no blobs are detected, then rotate and search until a blob has been found
		} else {
			speed = 0;
			turnrate = 0.4;
		}
		pp.SetSpeed(speed, turnrate);

	}// end of while(true) loop
}


