// A collection of tiles used in mahjong.
// Author: Alex Lobl
// Date: 5/27/2015
// Version: A.1

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <random>
#include <algorithm>
#include <chrono>
//#include <time.h>

using namespace std;


struct Tile{
	string suit;			// A tile can be any of 4 suits or an honor.
	int value;				// A tile of a suit can have value 1-9.
	//bool Dora_indicator;	// Whether or not the tile indicates the dora tile. Used in Japanese mahjong.
	//bool red;				// Whether or not the tile is red, worth an extra point in a winning hand.

	Tile(){
	}

	Tile(char t){
		switch (t){
			case 'd':
				suit = "Red Dragon";
				value = 42;
				break;
			case 't':
				suit = "White Dragon";
				value = 42;
				break;
			case 'g':
				suit = "Green Dragon";
				value = 42;
				break;
			case 's':
				suit = "South";
				value = 2;
				break;
			case 'w':
				suit = "West";
				value = 3;
				break;
			case 'n':
				suit = "North";
				value = 4;
				break;
			case 'e':
				suit = "East";
				value = 1;
				break;
			case 'z':
				suit = "Spring";
				value = 1;
				break;
			case 'u':
				suit = "Summer";
				value = 2;
				break;
			case 'f':
				suit = "Autumn";
				value = 3;
				break;
			case 'i':
				suit = "Winter";
				value = 4;
				break;
			case 'p':
				suit = "Plum";
				value = 1;
				break;
			case 'o':
				suit = "Orchid";
				value = 2;
				break;
			case 'c':
				suit = "Chrysanthemum";
				value = 3;
				break;
			case 'b':
				suit = "Bamboo";
				value = 4;
				break;
			default:
				suit = "Grass";
				value = -1;
				break;
		}
	}

	Tile(char t, int x){
		switch (t){
			case 'p':
				suit = "Pin";
				value = x;
				//red = False;
				break;
			case 's':
				suit = "Sou";
				value = x;
				//red = False;
				break;
			case 'm':
				suit = "Man";
				value = x;
				//red = False;
				break;
			default:
				suit = "Grass";
				value = -1;
				break;
		}
	}

};

