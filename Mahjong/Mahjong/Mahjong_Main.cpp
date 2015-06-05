// The main game code.
// Author: Alex Lobl
// Date: 6/5/2015
// Version: 0.0.1 Alpha

#include "Mahjong_Dice.cpp"
#include "Mahjong_Player.cpp"
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <iterator>

using namespace std;
using namespace std::chrono;

#pragma region
enum Turn { EAST, SOUTH, WEST, NORTH };
enum Game { JAPANESE, HONGKONG, CHINESE, NONE };

int roll;
int game_Running;
int turn = EAST;
int round_Counter = 0;
int move_Round = 0;

die A_die = die();

Game M_type;

char e_Selector;
char herp;
char option;
char flowers[] = { 'p', 'c', 'b', 'o', 'z', 'u', 'f', 'i' };
char honors[] = { 's', 'n', 'e', 'w', 'd', 't', 'g' };
char normals[] = { 'm', 'p', 's' };

bool game_is_running = true;

Tile wall[144];
Tile wall2[144];
Tile discards[144];
Tile* setup_Wall;
Tile drawn_Tile = NULL;
Tile last_Discard_Tile = NULL;

Player east = Player("East");
Player west = Player("West");
Player north = Player("North");
Player south = Player("South");

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
#pragma endregion Variables

Tile draw_Tile(Tile* wall, int x = 0){

	// Draw a the next tile in the wall. If there is no tile at the current position, 
	// move to the next position until there is one.
	Tile temp;
	if (x == 0){
		for (int i = 0; i < 144; i++){	// Out of all tiles in the wall, find one to draw.
			if (wall[i].value > 0){		// As long as there exists a valid tile to draw.
				temp = wall[i];			// Draw it.
				wall[i] = NULL;			// Then take the tile out from the wall so it won't be drawn again.
				return temp;			// Return the drawn tile.
			}
		}
	}
	else {
		for (int i = 143; i > 0; i--){
			if (wall[i].value > 0){
				temp = wall[i];
				wall[i] = NULL;
				return temp;
			}
		}
	}
	// Will only return if there are no more valid tiles to draw in the entire wall.
	// Typically signals the end of the round or game.
	return NULL;
}

Tile* wall_Setup(){
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

Tile* wall_Split(Tile* x, int roll = 0){

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

Tile* discard_Tile(Player p, Tile d, Tile* w){

	// A selected tile from a player's hand is discarded into the discards.
	Player this_Player = p;
	Tile temp;
	bool discarded = false;
	int x;

	cout << "Drawn tile: " << d.value << " " << d.suit << endl;
	while (d.suit == "Winter" || d.suit == "Summer" || d.suit == "Autumn" || d.suit == "Spring" ||
		d.suit == "Plum" || d.suit == "Orchid" || d.suit == "Bamboo" || d.suit == "Chrysanthemum"){
		for (int i = 0; i < 8; i++){
			if (this_Player.claimed[i].value > 0){
				this_Player.claimed[i] = d;
			}
		}
		d = draw_Tile(w, 1);
		cout << "Drawn tile: " << d.value << " " << d.suit << endl;
	}
	for (int j = 0; j < 8; j++){
		if (this_Player.hand_Points != 0){
			if (this_Player.claimed[j].value > 0){
				if (this_Player.claimed[j].value != this_Player.player_Value){
					this_Player.hand_Points -= 1;
				}
			}
		}
	}

	while (!discarded){
		cout << "\n";
		cout << "Which tile would you like to discard? 1-13 from hand or the 14th drawn tile?" << endl;
		cout << "1-14: Discard that tile.\n15: View hand.\n16: View drawn tile.\n";
		cin >> x;

		if (x < 14){
			temp = this_Player.hand[x - 1];
			this_Player.hand[x - 1] = d;
			discarded = true;
		}
		else if (x == 14){
			temp = d;
			discarded = true;
		}
		else if (x == 15){
			this_Player.see_Hand();
		}
		else if (x == 16){
			cout << "Drawn tile: " << d.value << " " << d.suit << endl;
		}
		else if (x > 16 || x < 1){
			cout << "That is not a option." << endl;
		}
	}

	discarded = false;
	int k = 0;
	while (!discarded){
		if (discards[k].value <= 0){
			discards[k] = temp;
			last_Discard_Tile = temp;
			discarded = true;
		}
		else {
			k++;
		}

	}
	return this_Player.sort_Hand(this_Player.hand);
}

void hand_Create(Tile* t_wall){
	
	// At the beginning of each round, players draw 13 tiles in a pattern of 4 tiles at a time
	// until each player has 12 tiles and then in order, each player draws 1 more to create a hand
	// of 13 tiles. The player in east position will also draw the 14th tile as part of her "turn".
	// This function handles the first 13 tiles for each player.

	int e = 0;		  // Position in east player's hand you're looking at from 0 - 12 (13 tiles)
	int w = 0;		  // Position in west player's hand you're looking at from 0 - 12 (13 tiles)
	int s = 0;		  // Position in south player's hand you're looking at from 0 - 12 (13 tiles)
	int n = 0;		  // Position in north player's hand you're looking at from 0 - 12 (13 tiles)
	int play_Num = 0; // Which player are you looking at? 0 = East, 1 = South, 2 = West, 3 = North

	for (int i = 0; i < 12; i++){ // Overall the wall is drawn from 12 times. Each player draws 4 tiles from it 3 times.
		for (int j = 0; j < 4; j++){ // Each player draws 4 tiles at a time up to 12 tiles.
			switch (play_Num){
			case 0:
				east.hand[e] = draw_Tile(t_wall);
				e++;
				break;
			case 1:
				south.hand[s] = draw_Tile(t_wall);
				s++;
				break;
			case 2:
				west.hand[w] = draw_Tile(t_wall);
				w++;
				break;
			case 3:
				north.hand[n] = draw_Tile(t_wall);
				n++;
				break;
			}
		}
		if (play_Num == 3){ // If you're looking at the north player, return to the east player.
			play_Num = 0;
		}
		else{ // Otherwise move to the next player in counter-clockwise fashion.
			play_Num++;
		}
	}

	// Draw the 13th tile into each player's hand in order.
	east.hand[12] = draw_Tile(t_wall);
	south.hand[12] = draw_Tile(t_wall);
	west.hand[12] = draw_Tile(t_wall);
	north.hand[12] = draw_Tile(t_wall);
}

void see_Discards(){
	for (int i = 0; i < 144; i++){
		if (discards[i].value > 0){
			cout << "Tile " << i + 1 << " in discards: " << discards[i].value
				<< " " << discards[i].suit << endl;
		}
	}
}


void update(){
	srand(time(NULL));

	cout << "What kind of mahjong do you want to play?" << endl;
	cout << "J -- Japanese Riichi Mahjong\nH -- Hong Kong Style\nM -- Modern Chinese Style\nQ -- Quit" << endl;
	cin >> e_Selector;
	
	if (e_Selector == 'J' || e_Selector == 'j'){
		M_type = JAPANESE;
		game_Running = JAPANESE;
	}
	else if (e_Selector == 'H' || e_Selector == 'h'){
		M_type = HONGKONG;
		game_Running = HONGKONG;
	}
	else if (e_Selector == 'C' || e_Selector == 'c'){
		M_type = CHINESE;
		game_Running = CHINESE;
	}
	else if (e_Selector == 'Q' || e_Selector == 'q'){
		M_type = NONE;
		game_Running = NONE;
	}

#pragma region 
	if (game_Running == JAPANESE) {
		while (game_Running == JAPANESE){
			cout << "This mode is not implemented yet.\n";
			cout << "Play again? Y/N" << endl;
			cin >> herp;
			if (herp == 'N' || herp == 'n'){
				break;
			}
		}
	}
#pragma endregion Japanese Mahjong

#pragma region 
	if (game_Running == HONGKONG){
		while (game_Running == HONGKONG){
			round_Counter = EAST + move_Round;
			cout << "Round: " << round_Counter << endl;
			move_Round = 0;
			roll = A_die.HK_roll();
			cout << "Rolled " << roll << "\n\n" << endl;

			setup_Wall = wall_Split(wall_Setup(), roll);
			east.hand_Points = 1;
			south.hand_Points = 1;
			north.hand_Points = 1;
			west.hand_Points = 1;

#pragma region
			hand_Create(setup_Wall);

			east.hand = east.sort_Hand(east.hand);
			for (int i = 0; i < 13; i++){
				while (east.hand[i].suit == "Winter" || east.hand[i].suit == "Summer" || east.hand[i].suit == "Autumn" || east.hand[i].suit == "Spring" ||
					east.hand[i].suit == "Plum" || east.hand[i].suit == "Orchid" || east.hand[i].suit == "Bamboo" || east.hand[i].suit == "Chrysanthemum"){
					for (int j = 0; j < 8; j++){
						if (east.claimed[j].value <= 0){
							east.claimed[j] = east.hand[i];
							break;
						}
					}
					east.hand[i] = draw_Tile(setup_Wall, 1);
				}
			}
			for (int j = 0; j < 8; j++){
				if (east.hand_Points != 0){
					if (east.claimed[j].value > 0){
						if (east.claimed[j].value != 1){
							east.hand_Points -= 1;
						}
					}
				}
			}
			east.hand = east.sort_Hand(east.hand);
			south.hand = south.sort_Hand(south.hand);
			for (int i = 0; i < 13; i++){
				while (south.hand[i].suit == "Winter" || south.hand[i].suit == "Summer" || south.hand[i].suit == "Autumn" || south.hand[i].suit == "Spring" ||
					south.hand[i].suit == "Plum" || south.hand[i].suit == "Orchid" || south.hand[i].suit == "Bamboo" || south.hand[i].suit == "Chrysanthemum"){
					for (int j = 0; j < 8; j++){
						if (south.claimed[j].value > 0){
							south.claimed[j] = south.hand[i];
						}
					}
					south.hand[i] = draw_Tile(setup_Wall, 1);
				}
			}
			for (int j = 0; j < 8; j++){
				if (south.hand_Points != 0){
					if (south.claimed[j].value > 0){
						if (south.claimed[j].value != 1){
							south.hand_Points -= 1;
						}
					}
				}
			}
			south.hand = south.sort_Hand(south.hand);
			west.hand = west.sort_Hand(west.hand);
			for (int i = 0; i < 13; i++){
				while (west.hand[i].suit == "Winter" || west.hand[i].suit == "Summer" || west.hand[i].suit == "Autumn" || west.hand[i].suit == "Spring" ||
					west.hand[i].suit == "Plum" || west.hand[i].suit == "Orchid" || west.hand[i].suit == "Bamboo" || west.hand[i].suit == "Chrysanthemum"){
					for (int j = 0; j < 8; j++){
						if (west.claimed[j].value > 0){
							west.claimed[j] = west.hand[i];
						}
					}
					west.hand[i] = draw_Tile(setup_Wall, 1);
				}
			}
			for (int j = 0; j < 8; j++){
				if (west.hand_Points != 0){
					if (west.claimed[j].value > 0){
						if (west.claimed[j].value != 1){
							west.hand_Points -= 1;
						}
					}
				}
			}
			west.hand = west.sort_Hand(west.hand);
			north.hand = north.sort_Hand(north.hand);
			for (int i = 0; i < 13; i++){
				while (north.hand[i].suit == "Winter" || north.hand[i].suit == "Summer" || north.hand[i].suit == "Autumn" || north.hand[i].suit == "Spring" ||
					north.hand[i].suit == "Plum" || north.hand[i].suit == "Orchid" || north.hand[i].suit == "Bamboo" || north.hand[i].suit == "Chrysanthemum"){
					for (int j = 0; j < 8; j++){
						if (north.claimed[j].value > 0){
							north.claimed[j] = north.hand[i];
						}
					}
					north.hand[i] = draw_Tile(setup_Wall, 1);
				}
			}
			for (int j = 0; j < 8; j++){
				if (north.hand_Points != 0){
					if (north.claimed[j].value > 0){
						if (north.claimed[j].value != 1){
							north.hand_Points -= 1;
						}
					}
				}
			}
			north.hand = north.sort_Hand(north.hand);
#pragma endregion Initial Hands
			
			while (round_Counter < round_Counter + 1){

#pragma region
				if (turn == EAST){
					while (turn == EAST){
						cout << "East player, choose:" << endl;
						cout << "D: Draw a tile.\nC: Claim the last discard.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;
						cin >> option;

						if (option == 'D' || option == 'd'){
							east.hand = discard_Tile(east, draw_Tile(setup_Wall), setup_Wall);
							east.see_Hand();
							turn += 1;
						}
						else if (option == 'C' || option == 'c'){
							cout << "You take the last discard" << endl;
							turn += 1;
						}
						else if (option == 'H' || option == 'h'){
							east.see_Hand();
						}
						else if (option == 'M' || option == 'm'){
							east.see_Claimed();
						}
						else if (option == 'V' || option == 'v'){
							see_Discards();
						}
					}
				}
#pragma endregion East Turn

#pragma region
				if (turn == SOUTH){
					while (turn == SOUTH){
						cout << "South player, choose:" << endl;
						cout << "D: Draw a tile.\nC: Claim the last discard.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;
						cin >> option;

						if (option == 'D' || option == 'd'){
							south.hand = discard_Tile(south, draw_Tile(setup_Wall), setup_Wall);
							south.see_Hand();
							turn += 1;
						}
						else if (option == 'C' || option == 'c'){
							cout << "You take the last discard" << endl;
							turn += 1;
						}
						else if (option == 'H' || option == 'h'){
							south.see_Hand();
						}
						else if (option == 'M' || option == 'm'){
							south.see_Claimed();
						}
						else if (option == 'V' || option == 'v'){
							see_Discards();
						}
					}
				}
#pragma endregion South Turn

#pragma region
				if (turn == WEST){
					while (turn == WEST){
						cout << "West player, choose:" << endl;
						cout << "D: Draw a tile.\nC: Claim the last discard.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;
						cin >> option;

						if (option == 'D' || option == 'd'){
							west.hand = discard_Tile(west, draw_Tile(setup_Wall), setup_Wall);
							west.see_Hand();
							turn += 1;
						}
						else if (option == 'C' || option == 'c'){
							cout << "You take the last discard" << endl;
							turn += 1;
						}
						else if (option == 'H' || option == 'h'){
							west.see_Hand();
						}
						else if (option == 'M' || option == 'm'){
							west.see_Claimed();
						}
						else if (option == 'V' || option == 'v'){
							see_Discards();
						}
					}
				}
#pragma endregion West Turn

#pragma region
				if (turn == NORTH){
					while (turn == NORTH){
						cout << "North player, choose:" << endl;
						cout << "D: Draw a tile.\nC: Claim the last discard.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;
						cin >> option;

						if (option == 'D' || option == 'd'){
							north.hand = discard_Tile(north, draw_Tile(setup_Wall), setup_Wall);
							north.see_Hand();
							turn = EAST;
						}
						else if (option == 'C' || option == 'c'){
							cout << "You take the last discard" << endl;
							turn = EAST;
						}
						else if (option == 'H' || option == 'h'){
							north.see_Hand();
						}
						else if (option == 'M' || option == 'm'){
							north.see_Claimed();
						}
						else if (option == 'V' || option == 'v'){
							see_Discards();
						}
					}
				}
#pragma endregion North Turn

				cout << "\n\n";

				cin >> herp;
				if (draw_Tile(setup_Wall).value == NULL || herp == 'e'){
					move_Round = 1;
					round_Counter += move_Round;
				}
			}
		}
	}
#pragma endregion Hong Kong Mahjong

#pragma region
		if (game_Running == CHINESE){
			while (game_Running == CHINESE){
				cout << "This mode is not implemented yet.\n";
				break;
			}
		}
#pragma endregion Modern Chinese Mahjong

#pragma region
		if (game_Running == NONE){
			cout << "Thank you for playing.\n";
			game_is_running = false;
		}
#pragma endregion Quit
	
}

int get_Tick(){
	return system_clock::now().time_since_epoch().count();
}

void main(){
	const int FRAMES_PER_SECOND = 60;
	const int SKIP_TICKS = 1000 / FRAMES_PER_SECOND;

	int next_Tick = get_Tick();

	while (game_is_running){
		update();

		next_Tick += SKIP_TICKS;
	}
}

