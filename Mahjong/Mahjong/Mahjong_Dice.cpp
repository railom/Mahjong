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

	die(int x){			// Creates a die with x number of faces.
		side_count = x;
	}

	int roll(){
		return rand() % side_count + 1;
	}

	int HK_roll(){
		return roll() + roll();
	}

};

/*void main(){
	srand(time(NULL));
	die die1 = die(6);

	int result = die1.roll();
	cout << result << endl;
	result = die1.HK_roll();
	cout << result << endl;
	system("pause");
}*/