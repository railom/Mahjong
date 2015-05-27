// The main game code.
// Author: Alex Lobl
// Date: 5/27/2015
// Version: A.1

#include "Mahjong_Dice.cpp"
#include "Mahjong_Tile.cpp"
#include <stdio.h>
#include <stdlib.h>
//#include <chrono>

using namespace std;

void main(){
	srand(time(NULL));
	//unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	die a_die = die(6);
	//Tile* tiles = nullptr;
	Tile a_tile = Tile('p', 9);
	char M_type;

	cout << "What kind of mahjong do you want to play?" << endl;
	cout << "J -- Japanese Riichi Mahjong\nH -- Hong Kong Style\nM -- Modern Chinese Style\nQ -- Quit" << endl;
	cin >> M_type;
	switch (M_type)
	{
		case 'J':
			cout << "This mode is not implemented yet.\n";
			system("Pause");
			break;

		case 'H':
			cout << "Rolled " << a_die.HK_roll() << endl;
			//tiles = tile_setup();
			//shuffle(tiles[0], tiles[144], default_random_engine(seed));
			cout << "A tile: " << a_tile.suit << " " << a_tile.value << endl;
			system("Pause");
			break;
	
		case 'M':
			cout << "This mode is not implemented yet.\n";
			system("Pause");
			break;
	
		case 'Q':
			cout << "Thank you for playing.\n";
			system("Pause");
			break;
	
		default:
			cout << "You did not enter a valid option.\n";
			system("Pause");
			break;
	}

	
}