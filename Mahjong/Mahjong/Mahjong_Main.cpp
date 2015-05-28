// The main game code.
// Author: Alex Lobl
// Date: 5/27/2015
// Version: A.1

#include "Mahjong_Dice.cpp"
#include "Mahjong_Tile.cpp"
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <iterator>
//#include <chrono>

using namespace std;

Tile wall[144] = { Tile('p'), Tile('c'), Tile('b'), Tile('o'), Tile('z'), Tile('u'), Tile('f'), Tile('i'),
Tile('s'), Tile('s'), Tile('s'), Tile('s'), Tile('n'), Tile('n'), Tile('n'), Tile('n'), Tile('e'), Tile('e'), Tile('e'),
Tile('e'), Tile('w'), Tile('w'), Tile('w'), Tile('w'), Tile('d'), Tile('d'), Tile('d'), Tile('d'), Tile('t'), Tile('t'),
Tile('t'), Tile('t'), Tile('g'), Tile('g'), Tile('g'), Tile('g'), Tile('p', 1), Tile('p', 1), Tile('p', 1), Tile('p', 1),
Tile('p', 2), Tile('p', 2), Tile('p', 2), Tile('p', 2), Tile('p', 3), Tile('p', 3), Tile('p', 3), Tile('p', 3), Tile('p', 4),
Tile('p', 4), Tile('p', 4), Tile('p', 4), Tile('p', 5), Tile('p', 5), Tile('p', 5), Tile('p', 5), Tile('p', 6), Tile('p', 6),
Tile('p', 6), Tile('p', 6), Tile('p', 7), Tile('p', 7), Tile('p', 7), Tile('p', 7), Tile('p', 8), Tile('p', 8), Tile('p', 8),
Tile('p', 8), Tile('p', 9), Tile('p', 9), Tile('p', 9), Tile('p', 9), Tile('m', 9), Tile('m', 9), Tile('m', 9), Tile('m', 9),
Tile('m', 8), Tile('m', 8), Tile('m', 8), Tile('m', 8), Tile('m', 7), Tile('m', 7), Tile('m', 7), Tile('m', 7), Tile('m', 6),
Tile('m', 6), Tile('m', 6), Tile('m', 6), Tile('m', 5), Tile('m', 5), Tile('m', 5), Tile('m', 5), Tile('m', 4), Tile('m', 4),
Tile('m', 4), Tile('m', 4), Tile('m', 3), Tile('m', 3), Tile('m', 3), Tile('m', 3), Tile('m', 2), Tile('m', 2), Tile('m', 2),
Tile('m', 2), Tile('m', 1), Tile('m', 1), Tile('m', 1), Tile('m', 1), Tile('s', 1), Tile('s', 1), Tile('s', 1), Tile('s', 1), 
Tile('s', 2), Tile('s', 2), Tile('s', 2), Tile('s', 2), Tile('s', 3), Tile('s', 3), Tile('s', 3), Tile('s', 3), Tile('s', 4),
Tile('s', 4), Tile('s', 4), Tile('s', 4), Tile('s', 5), Tile('s', 5), Tile('s', 5), Tile('s', 5), Tile('s', 6), Tile('s', 6),
Tile('s', 6), Tile('s', 6), Tile('s', 7), Tile('s', 7), Tile('s', 7), Tile('s', 7), Tile('s', 8), Tile('s', 8), Tile('s', 8),
Tile('s', 8), Tile('s', 9), Tile('s', 9), Tile('s', 9), Tile('s', 9) };

Tile wall2[144];
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

Tile* Wall_setup(){
	srand(time(NULL));

	for (int i = 0; i < 144; i++){
		while (wall2[i].suit == "Grass" || wall2[i].value <= 0){
			int Rand_tile = rand() % 144;
			if (wall[Rand_tile].value > 0){
				wall2[i] = wall[Rand_tile];
				wall[Rand_tile] = Tile('q');
			}
			else{
				wall2[i] = Tile('q');
			}
		}
	}

	return wall2;
}

void main(){
	srand(time(NULL));
	//unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	die a_die = die();
	Tile* Setup_wall;
	Tile a_tile = Tile('t');
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
			Setup_wall = Wall_setup();
			
			cout << "Rolled " << a_die.HK_roll() << endl;
			
			cout << "Tile 1: " << Setup_wall[0].value << " " << Setup_wall[0].suit << endl;
			cout << "Tile 2: " << Setup_wall[1].value << " " << Setup_wall[1].suit << endl;
			cout << "Tile 3: " << Setup_wall[2].value << " " << Setup_wall[2].suit << endl;
			cout << "Tile 4: " << Setup_wall[3].value << " " << Setup_wall[3].suit << endl;
			cout << "Tile 5: " << Setup_wall[4].value << " " << Setup_wall[4].suit << endl;
			cout << "Tile 6: " << Setup_wall[5].value << " " << Setup_wall[5].suit << endl;
			cout << "Tile 7: " << Setup_wall[6].value << " " << Setup_wall[6].suit << endl;
			cout << "Tile 8: " << Setup_wall[7].value << " " << Setup_wall[7].suit << endl;
			cout << "Tile 9: " << Setup_wall[8].value << " " << Setup_wall[8].suit << endl;
			cout << "Tile 10: " << Setup_wall[9].value << " " << Setup_wall[9].suit << endl;
			cout << "Tile 11: " << Setup_wall[10].value << " " << Setup_wall[10].suit << endl;
			cout << "Tile 12: " << Setup_wall[11].value << " " << Setup_wall[11].suit << endl;
			cout << "Tile 13: " << Setup_wall[12].value << " " << Setup_wall[12].suit << endl;

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