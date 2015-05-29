// The main game code.
// Author: Alex Lobl
// Date: 5/29/2015
// Version: A.1

#include "Mahjong_Dice.cpp"
#include "Mahjong_Tile.cpp"
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <iterator>

using namespace std;

char flowers[] = { 'p', 'c', 'b', 'o', 'z', 'u', 'f', 'i' };
char honors[] = { 's', 'n', 'e', 'w', 'd', 't', 'g' };
char normals[] = { 'm', 'p', 's' };

Tile wall[144];
Tile wall2[144];
Tile Drawn_tile = NULL;

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

Tile* Wall_setup(){
	srand(time(NULL));
	int k = 0;
	int h = 0;
	
	// Create 144 tiles including 4 flowers and 4 seasons and stick them into an array
	// in some specific order.
	for (char x : flowers){ // Create 8 flowers and seasons.
		wall[k] = Tile(x);
		k++;
	}
	for (char x : honors){
		for (int i = 0; i < 4; i++){ // Create 4 of every honor tile.
			wall[h+k] = Tile(x);
			h++;
		}
	}
	for (char x : normals){
		for (int i = 1; i <= 9; i++){
			for (int j = 0; j < 4; j++){ // Create 4 of each 1-9 in every suit
				wall[h+k] = Tile(x, i);
				h++;
			}
		}
	}

	// Take a random tile from all 144 tiles in the array and stick it into the tile
	// wall, as long as a tile exists in that position.
	for (int i = 0; i < 144; i++){
		while (wall2[i].suit == "Grass" || wall2[i].value <= 0){ // Continue to stick a tile in position wall2[i] until it is valid.
			int Rand_tile = rand() % 144;
			if (wall[Rand_tile].value > 0){ // If a valid random tile exists, put it into the wall. 
				wall2[i] = wall[Rand_tile];
				wall[Rand_tile] = Tile('q'); // Create a 'null' tile in place of the taken tile in the original array.
			}
			else{
				wall2[i] = Tile('q'); // If no tile exists in the original array, create a null tile in its place.
			}
		}
	}

	// Return a pseudo-shuffled wall of tiles.
	return wall2;
}

Tile* Wall_split(Tile* x, int roll = 0){

	// Create pseudo-walls for each player. Used to represent the wall sitting in front
	// of each player. In Hong Kong and other styles that use the flowers and seasons,
	// every players has 36 tiles in front of them, stacked 18 long and 2 high.
	// Also create a wall that is split based on the roll.
	Tile* P1_wall = new Tile[36];
	Tile* P2_wall = new Tile[36];
	Tile* P3_wall = new Tile[36];
	Tile* P4_wall = new Tile[36];
	Tile* Split_wall = new Tile[144];
	Tile* temp;
	
	int k = 0;
	int j = 0;
	int h = 1;

	// Split the wall into 4 sections, 36 tiles long for each of the 4 players.
	for (int i = 0; i < 36; i++){
		P1_wall[i] = x[k];
		k++;
	}
	for (int i = 0; i < 36; i++){
		P2_wall[i] = x[k];
		k++;
	}
	for (int i = 0; i < 36; i++){
		P3_wall[i] = x[k];
		k++;
	}
	for (int i = 0; i < 36; i++){
		P4_wall[i] = x[k];
		k++;
	}

	k = 0;
	
	// Based on the roll, split a player's wall further and then organize the new
	// wall backwards from the split. Ending with the tiles placed into a temp wall.
	// e.x. The roll is 8, thus player 4 separates 16 tiles from her wall and drawing
	// happens from the 17th tile and draws continue clockwise from the end. The draws
	// end with the first tile from temp, or temp[0] which becomes Split_wall[143].
#pragma region
	if (roll == 2 || roll == 6 || roll == 10){
		int start = roll * 2; // The amount of tiles separated based on the roll.
		temp = new Tile[start]; // Only the size of the separated tiles.
		for (int i = 0; i < start; i++){
			temp[j] = P2_wall[36 - (start - j)]; // Take the tiles from the separated from a player's wall from the end of their wall.
			j++;
		}
		for (int i = 1; i <= 36-start; i++){
			Split_wall[k] = P2_wall[36 - (start + i)]; // Put the rest of the player's tiles from the split into the start of the wall.
			k++;
		}
		// Continue clockwise to add the rest of the players' tiles into the wall.
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P1_wall[35 - i];
			k++;
		}
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P4_wall[35 - i];
			k++;
		}
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P3_wall[35 - i];
			k++;
		}
		// Add the separated tiles into the wall at the end.
		for (int i = 1; i <= start; i++){
			Split_wall[k] = temp[start - i];
			k++;
		}
	} 
#pragma endregion South_Player

#pragma region
	else if (roll == 3 || roll == 7 || roll == 11){
		int start = roll * 2; // The amount of tiles separated based on the roll.
		temp = new Tile[start]; // Only the size of the separated tiles.
		for (int i = 0; i < start; i++){
			temp[j] = P3_wall[36 - (start - j)]; // Take the tiles from the separated from a player's wall from the end of their wall.
			j++;
		}
		for (int i = 1; i <= 36 - start; i++){
			Split_wall[k] = P3_wall[36 - (start + i)]; // Put the rest of the player's tiles from the split into the start of the wall.
			k++;
		}
		// Continue clockwise to add the rest of the players' tiles into the wall.
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P2_wall[35 - i];
			k++;
		}
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P1_wall[35 - i];
			k++;
		}
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P4_wall[35 - i];
			k++;
		}
		// Add the separated tiles into the wall at the end.
		for (int i = 1; i <= start; i++){
			Split_wall[k] = temp[start - i];
			k++;
		}
	}
#pragma endregion West_Player

#pragma region
	else if (roll == 4 || roll == 8 || roll == 12){
		int start = roll * 2; // The amount of tiles separated based on the roll.
		temp = new Tile[start]; // Only the size of the separated tiles.
		for (int i = 0; i < start; i++){
			temp[j] = P4_wall[36 - (start - j)]; // Take the tiles from the separated from a player's wall from the end of their wall.
			j++;
		}
		for (int i = 1; i <= 36 - start; i++){
			Split_wall[k] = P4_wall[36 - (start + i)]; // Put the rest of the player's tiles from the split into the start of the wall.
			k++;
		}
		// Continue clockwise to add the rest of the players' tiles into the wall.
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P3_wall[35 - i];
			k++;
		}
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P2_wall[35 - i];
			k++;
		}
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P1_wall[35- i];
			k++;
		}
		// Add the separated tiles into the wall at the end.
		for (int i = 1; i <= start; i++){
			Split_wall[k] = temp[start - i];
			k++;
		}
	}
#pragma endregion North_Player

#pragma region
	else if (roll == 1 || roll == 5 || roll == 9){
		int start = roll * 2; // The amount of tiles separated based on the roll.
		temp = new Tile[start]; // Only the size of the separated tiles.
		for (int i = 0; i < start; i++){
			temp[j] = P1_wall[36 - (start - j)]; // Take the tiles from the separated from a player's wall from the end of their wall.
			j++;
		}
		for (int i = 1; i <= 36 - start; i++){
			Split_wall[k] = P1_wall[36 - (start + i)]; // Put the rest of the player's tiles from the split into the start of the wall.
			k++;
		}
		// Continue clockwise to add the rest of the players' tiles into the wall.
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P4_wall[35 - i];
			k++;
		}
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P3_wall[35 - i];
			k++;
		}
		for (int i = 0; i < 36; i++){
			Split_wall[k] = P2_wall[35 - i];
			k++;
		}	
		// Add the separated tiles into the wall at the end.
		for (int i = 1; i <= start; i++){
			Split_wall[k] = temp[start - i];
			k++;
		}
	}
#pragma endregion East_Player

	// Return a wall divided in a pseudo-Hong Kong or Modern Chinese way. This is the wall
	// designed to be drawn from in typical mahjong fashion.
	return Split_wall;
}

Tile Draw_tile(Tile* wall){

	// Draw a the next tile in the wall. If there is no tile at the current position, 
	// move to the next position until there is one.
	Tile temp;
	for (int i = 0; i < 144; i++){ // Out of all tiles in the wall, find one to draw.
		if (wall[i].value > 0){ // As long as there exists a valid tile to draw.
			temp = wall[i]; // Draw it.
			wall[i] = NULL; // Then take the tile out from the wall so it won't be drawn again.
			return temp; // Return the drawn tile.
		}
	}
	// Will only return if there are no more valid tiles to draw in the entire wall.
	// Typically signals the end of the round or game.
	return NULL;
}

void main(){
	srand(time(NULL));
	int roll;
	die A_die = die();
	Tile* Setup_wall;
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
			roll = A_die.HK_roll();
			cout << "Rolled " << roll << "\n\n" << endl;

			Setup_wall = Wall_split(Wall_setup(), roll);
			/*
			Drawn_tile = Draw_tile(Setup_wall);
			cout << "Drawn 1: " << Drawn_tile.value << " " << Drawn_tile.suit << endl;
			Drawn_tile = Draw_tile(Setup_wall);
			cout << "Drawn 2: " << Drawn_tile.value << " " << Drawn_tile.suit << endl;*/
			
			for (int i = 0; i < 144; i++){
				cout << "Tile " << i << ": " << Setup_wall[i].value << " " << Setup_wall[i].suit << endl;
			}

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