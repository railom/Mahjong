// Dice roller for a variety of mahjong types
// Author: Alex Lobl
// Date: 5/27/2015
// Version: A.1

#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <time.h>
#include <math.h>
#include <iostream>

using namespace std;

struct die{
	int side_count;		// A die has some number of sides.
	int face_num;		// Each side has a number associated with it.

	die(int x = 6){			// Creates a die with x number of faces. Standard dice have 6 faces.
		side_count = x;
	}

	int roll(){				// A roll of a die results in any number between 1 and the number of faces.
		return rand() % side_count + 1;
	}

	int HK_roll(){
		return roll() + roll();
	}

	int MC_roll(){
		return roll() + roll() + roll() + roll();
	}

};