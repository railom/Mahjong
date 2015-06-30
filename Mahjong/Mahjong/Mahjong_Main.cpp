// The main game code.
// Author: Alex Lobl
// Date: 6/25/2015
// Version: 0.2.1 Alpha

#include "Mahjong_Dice.cpp"
#include "Mahjong_Player.cpp"
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <iterator>

using namespace std;
using namespace std::chrono;

void see_Discards();
void update_Points(Player p, int t);
bool all_Flowers(Tile* f);
int calculate_Points(Meld* m, Tile* h, Tile l, Player p, bool sd = false, bool lt = false, bool rt = false);

#pragma region
enum Turn { EAST, SOUTH, WEST, NORTH };
enum Game { JAPANESE, HONGKONG, CHINESE, NONE, MENU };

int roll;
int game_Running;
int turn = EAST;
int turn_Counter = EAST;
int round_Counter = 0;
int move_Round = 0;
int first_Player = EAST;
int counter = 0;			// Counts the amount of tiles left in the wall.
int chow_Choice;
int tenho_Counter = 0;
int chiho_Counter = 0;
int ren_Counter = 0;

die dice_A = die();

Game M_type;

char e_Selector;
char option;
char flowers[] = { 'p', 'c', 'b', 'o', 'z', 'u', 'f', 'i' };
char honors[] = { 's', 'n', 'e', 'w', 'd', 't', 'g' };
char normals[] = { 'm', 'p', 's' };

string language = "Japanese";

bool game_is_running = true;
bool hand_Won = false;
bool pass = false;
bool last_Tile = false;

Tile wall[144];
Tile wall2[144];
Tile discards[144];
Tile* setup_Wall;
Tile drawn_Tile = NULL;
Tile last_Discard_Tile = NULL;

Player east = Player("East");
Player west = Player("West", true);
Player north = Player("North", true);
Player south = Player("South", true);
Player last_Discarder;
Player players[4] = {east, west, north, south};

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
#pragma endregion Variables

// Draws the next tile in the wall.
// Takes the setup wall and if the tile should come from the end or the beginning.
// Returns the next tile in the wall.
Tile draw_Tile(Tile* d_wall, int x = 0){

	// Draw a the next tile in the wall. If there is no tile at the current position, 
	// move to the next position until there is one.
	Tile temp;
	if (x == 0){						// If we aren't replacing a flower or season or kong start at the beginning of the wall.
		for (int i = 0; i < 144; i++){	// Out of all tiles in the wall, find one to draw.
			if (d_wall[i].value > 0){		// As long as there exists a valid tile to draw.
				temp = d_wall[i];			// Draw it.
				d_wall[i] = NULL;			// Then take the tile out from the wall so it won't be drawn again.
				return temp;			// Return the drawn tile.
			}
		}
	}
	else {								// Otherwise, start at the end of the wall to replace.
		for (int i = 143; i > 0; i--){	// Out of all the tiles in wall starting at the end, find one to draw.
			if (d_wall[i].value > 0){
				temp = d_wall[i];
				d_wall[i] = NULL;
				return temp;
			}
		}
	}
	// Will only return if there are no more valid tiles to draw in the entire wall.
	// Typically signals the end of the round or game.
	return NULL;
}

// Looks into the array and sees what the next tile in the array is. Changes as tiles
// are drawn from the front.
// Takes the setup wall.
// Returns the next tile in the wall.
Tile see_Next_Tile(Tile* w){
	for (int i = 0; i < 144; i++){
		if (w[i].value > 0){
			return w[i];
		}
	}
	return NULL;
}

// Looks into the array and sees what the last tile in the array is. Changes as tiles
// are drawn from the end.
// Takes the setup wall.
// Returns the last tile in the wall.
Tile see_Last_Tile(Tile* w){
	for (int i = 143; i > 0; i--){
		if (w[i].value > 0){
			return w[i];
		}
	}
	return NULL;
}

// Creates all the tiles necessary to play mahjong and sorts them randomly into an array.
// Returns a shuffled array of tiles.
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

// The final part of wall setup for the game. The wall is split into 4 sections and then
// one player's section is split based on the roll before ordering in the array backwards in
// drawing order.
// Takes the shuffled wall and the roll.
// Returns an array of tiles ordered in drawing order.
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

	delete[] P1_wall;
	delete[] P2_wall;
	delete[] P3_wall;
	delete[] P4_wall;
	//delete temp;
	// Return a wall divided in a pseudo-Hong Kong or Modern Chinese way. This is the wall
	// designed to be drawn from in typical mahjong fashion.
	return Split_wall;
}

// Player draws a tile, replaces it if it is a flower/season, and then discards a tile.
// Takes the player, the drawn tile, the wall, whether it is an open meld, and it is part of
// a kong.
// Returns the player's sorted hand, including any tiles that were removed and added.
Tile* discard_Tile(Player p, Tile d, Tile* w, bool meld = false, bool kong = false){

	// A selected tile from a player's hand is discarded into the discards.
	Player this_Player = p;
	last_Discarder = this_Player;
	Tile temp;
	bool discarded = false;
	bool konged = false;
	last_Tile = (see_Next_Tile(setup_Wall).value <= 0) ? true : false;
	int x;
	int total_Points = 0;

#pragma region
	if (!meld && !konged){
		std::cout << "Drawn tile: " << d.value << " " << d.suit << endl;
		while (d.suit == "Winter" || d.suit == "Summer" || d.suit == "Autumn" || d.suit == "Spring" ||
			d.suit == "Plum" || d.suit == "Orchid" || d.suit == "Bamboo" || d.suit == "Chrysanthemum"){
			for (int i = 0; i < 8; i++){
				if (this_Player.claimed[i].value > 0){
					this_Player.claimed[i] = d;
				}
			}
			d = draw_Tile(w, 1);
			std::cout << "Drawn tile: " << d.value << " " << d.suit << endl;
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
	}
#pragma endregion Flower Replacement

#pragma region
	if (this_Player.can_Win(this_Player.hand, d)){
		if (tenho_Counter == 0 && this_Player.wind == "East"){
			this_Player.tenho = true;
		}
		while (!pass && !hand_Won){
			std::cout << "Tsumo on " << d.value << " " << d.suit << "?" << endl;
			std::cout << "R: Tsumo.\nH: View hand.\nM: View claimed tiles.\nV: View the discards.\nS: Pass.\n";
			cin >> option;
			
			if (option == 'R' || option == 'r'){
				/*if (this_Player.can_Make_Pair(this_Player.hand, d)){
					this_Player.pair = this_Player.make_Pair(this_Player.hand, d);
					for (int i = 0; i < 13; i++){
						if (this_Player.hand[i].suit == this_Player.pair.suit && this_Player.hand[i].value == this_Player.pair.melded[0].value){
							this_Player.hand[i] = NULL;
							d = NULL;
							this_Player.hand = this_Player.sort_Hand(this_Player.hand);
							break;
						}
					}
					this_Player.make_Chows(this_Player.hand);
					this_Player.make_Pongs(this_Player.hand);
				}*/
				for (int i = 0; i < 9; i++){
					if (this_Player.possible_Chows[i].name != "NONE"){
						for (int j = 0; j < 4; j++){
							if (this_Player.melds[j].name == "NONE"){
								this_Player.melds[j] = this_Player.possible_Chows[i];
								/*this_Player.hand[this_Player.tile_Pos(this_Player.melds[j].melded[0])] = NULL;
								this_Player.hand[this_Player.tile_Pos(this_Player.melds[j].melded[1])] = NULL;
								this_Player.hand[this_Player.tile_Pos(this_Player.melds[j].melded[2])] = NULL;*/
							}
							//this_Player.hand = this_Player.sort_Hand(this_Player.hand);
							//this_Player.see_Hand();
							for (int k = 0; k < 4; k++){
								// To do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
								if (this_Player.melds[j].suit == this_Player.possible_Chows[i + 1].suit && this_Player.melds[j].melded[k].value == this_Player.possible_Chows[i + 1].melded[k].value){
									this_Player.possible_Chows[i + 1] = Meld();
								}
							}
							//this_Player.make_Chows(this_Player.hand);
							//this_Player.make_Pongs(this_Player.hand);
							break;
						}
					}
					if (i < 4){
						if (this_Player.possible_Pongs[i].name != "NONE" || this_Player.possible_Pongs[i].suit != "Grass"){
							for (int j = 0; j < 4; j++){
								if (this_Player.melds[j].name == "NONE"){
									this_Player.melds[j] = this_Player.possible_Pongs[i];
									/*for (int k = 0; k < 13; k++){
										for (int z = 0; z < 4; z++){
											if (this_Player.hand[k].suit == this_Player.melds[j].suit && this_Player.hand[k].value == this_Player.melds[j].melded[z].value){
												this_Player.hand[k] = NULL;
											}
										}
									}
									this_Player.hand = this_Player.sort_Hand(this_Player.hand);
									this_Player.see_Hand();*/
									break;
								}
							}
						}
						//this_Player.make_Chows(this_Player.hand);
						//this_Player.make_Pongs(this_Player.hand);
					}
				}
				for (int i = 0; i < 4; i++){
					if (this_Player.melds[i].name != "NONE"){
						std::cout << "Meld " << i + 1 << ": " << this_Player.melds[i].name << endl;
						for (int j = 0; j < 3; j++){
							std::cout << this_Player.melds[i].melded[j].value << " " << this_Player.melds[i].melded[j].suit << endl;
						}
					}
				}
				total_Points = calculate_Points(this_Player.melds, this_Player.hand, d, this_Player, true, last_Tile, kong);
				if (total_Points >= 3){
					for (int i = 0; i < 4; i++){
						if (this_Player.wind != players[i].wind){
							update_Points(players[i], -total_Points);
						}
						else {
							update_Points(players[i], (total_Points * 3));
						}
					}
					this_Player.has_Won_Hand = true;
					hand_Won = true;
				}
				else {
					std::cout << "False win. Not enough points." << endl;
				}
			}
			else if (option == 'H' || option == 'h'){
				this_Player.see_Hand();
			}
			else if (option == 'M' || option == 'm'){
				this_Player.see_Claimed();
			}
			else if (option == 'V' || option == 'v'){
				see_Discards();
			}
			else if (option == 'S' || option == 's'){
				pass = true;
			}
		}
	}
#pragma endregion Self Draw Win

#pragma region
	if (this_Player.can_Kong(this_Player.hand, d) && !this_Player.is_AI){
		while (!pass){
			std::cout << "Kong on " << d.value << " " << d.suit << "?" << endl;
			std::cout << "K: Kong.\nH: View hand.\nM: View claimed tiles.\nV: View the discards.\nS: Pass.\n";
			cin >> option;

			if (option == 'K' || option == 'k'){
				for (int i = 0; i < 4; i++){
					if (this_Player.melds[i].name == "NONE"){
						for (int j = 0; j < 13; j++){
							if (this_Player.hand[j].value == this_Player.hand[j + 1].value && this_Player.hand[j + 1].value == this_Player.hand[j + 2].value && this_Player.hand[j + 2].value == d.value && this_Player.hand[j].suit == this_Player.hand[j + 1].suit && this_Player.hand[j + 1].suit == this_Player.hand[j + 2].suit && this_Player.hand[j + 2].suit == d.suit){
								this_Player.melds[i] = Meld(this_Player.hand[j], this_Player.hand[j + 1], this_Player.hand[j + 2], d);
								this_Player.melds[i].hidden = true;
								this_Player.hand[j] = NULL;
								this_Player.hand[j + 1] = NULL;
								this_Player.hand[j + 2] = NULL;
								this_Player.hand = discard_Tile(this_Player, draw_Tile(setup_Wall, 1), setup_Wall, false, true);
								pass = true;
								konged = true;
								break;
							}
						}
					}
				}
			}
			else if (option == 'H' || option == 'h'){
				this_Player.see_Hand();
			}
			else if (option == 'M' || option == 'm'){
				this_Player.see_Claimed();
			}
			else if (option == 'V' || option == 'v'){
				see_Discards();
			}
			else if (option == 'S' || option == 's'){
				pass = true;
			}
		}
	}
#pragma endregion Fully concealed kong

	while (!discarded && !konged && !hand_Won){
		std::cout << "\n";
		std::cout << "Which tile would you like to discard? 1-13 from hand or the 14th drawn tile?" << endl;
		std::cout << "1-14: Discard that tile.\n15: View hand.\n16: View drawn tile.\n";
		cin >> x;

		if (x < 14){
			if (this_Player.hand[x - 1].value > 0){
				temp = this_Player.hand[x - 1];
				if (!meld) this_Player.hand[x - 1] = d;
				else this_Player.hand[x - 1] = NULL;
				discarded = true;
			}
			else{
				std::cout << "Choose an appropriate tile to discard." << endl;
			}
		}
		else if (x == 14){
			if (!meld){
				temp = d;
				discarded = true;
			}
			else{
				std::cout << "You can't discard your claimed tile." << endl;
			}
		}
		else if (x == 15){
			this_Player.see_Hand();
		}
		else if (x == 16){
			std::cout << "Drawn tile: " << d.value << " " << d.suit << endl;
		}
		else if (x > 16 || x < 1){
			std::cout << "That is not a option." << endl;
		}
	}

	discarded = false;
	int k = 0;
	while (!discarded && !konged && !hand_Won){
		if (discards[k].value <= 0){
			if (!meld) discards[k] = temp;
			else discards[k - 1] = temp;
			last_Discard_Tile = temp;
			discarded = true;
		}
		else {
			k++;
		}
	}
	this_Player.make_Pairs(this_Player.hand);
	this_Player.hand = this_Player.sort_Hand(this_Player.hand);
	this_Player.make_Chows(this_Player.hand);
	this_Player.make_Pongs(this_Player.hand);
	return this_Player.hand;
}

// Calculate the amount of points a hand is worth after a win is declared.
// Takes a player's open melds, a player's hand, the winning tile, the player,
// whether it was a self draw, it was the last tile, and it was a kong replacement.
// Returns an int for the amount of points.
int calculate_Points(Meld* m, Tile* h, Tile l, Player p, bool sd, bool lt, bool rt){
	int all_Chows = 0;		// How many chows were in hand?
	int all_Pongs = 0;		// How many pongs were in hand?
	int conc_Pongs = 0;		// How many concealed pongs in hand?
	int all_Honors = 0;		// How many honors were in hand?
	int yakuhai = 0;		// How many honor pongs scored points?
	int three_Dragons = 0;	// How many dragon pongs were in hand?
	int four_Winds = 0;		// How many wind pongs were in hand?
	int flowers = 0;		// How many flowers and seasons were claimed?
	bool kong_Replace = rt;	// Did the win happen on a replacement tile from a kong?
	bool tsumo = sd;		// Was the winning tile self-drawn?
	bool fully_Concealed = true;	// Was the hand completely concealed? Including last tile.
	bool flush = false;		// Was the hand only one suit?
	bool h_Flush = false;	// Was the hand only one suit and honors?

	// Create the last meld if did not win on pair.
	if (p.can_Pong(h, l)){
		for (int i = 0; i < 13; i++){
			if (h[i].suit == l.suit && h[i].value == l.value && l.value == h[i + 1].value){
				m[3] = Meld(h[i], h[i + 1], l);
				std::cout << "Meld 4: " << m[3].name << endl;
				for (int j = 0; j < 3; j++){
					std::cout << m[3].melded[j].value << " " << m[3].melded[j].suit << endl;
				}
			}
		}
	}
	else if (p.can_Chow(h, l)){
		for (int j = 0; j < 13; j++){
			if (h[j].value > 0){
				if ((h[j].value == h[j + 1].value - 1 && h[j + 1].value == l.value - 1 && h[j].suit == h[j + 1].suit && h[j + 1].suit == l.suit) || (h[j].value == h[j + 1].value + 1 && h[j + 1].value == l.value + 1 && h[j].suit == h[j + 1].suit && h[j + 1].suit == l.suit) ||
					(h[j].value == h[j + 1].value - 2 && h[j + 1].value == l.value + 1 && h[j].suit == h[j + 1].suit && h[j + 1].suit == l.suit) || (h[j].value == h[j + 1].value + 1 && h[j + 1].value == l.value - 2 && h[j].suit == h[j + 1].suit && h[j + 1].suit == l.suit) ||
					(h[j].value == h[j + 1].value - 1 && h[j + 1].value == l.value + 2 && h[j].suit == h[j + 1].suit && h[j + 1].suit == l.suit) || (h[j].value == h[j + 1].value + 2 && h[j + 1].value == l.value - 1 && h[j].suit == h[j + 1].suit && h[j + 1].suit == l.suit)){
					m[3] = Meld(h[j], h[j + 1], l);
					std::cout << "Meld 4: " << m[3].name << endl;
					for (int j = 0; j < 3; j++){
						std::cout << m[3].melded[j].value << " " << m[3].melded[j].suit << endl;
					}
				}
			}
		}
	}

	// For each flower or season that belongs to a player's seat, that player
	// gets 1 point. (Max of 2). If a player has no flowers or seasons, that
	// player gets 1 point.
	if (game_Running == HONGKONG || game_Running == CHINESE){
		for (int i = 0; i < 8; i++){
			if (p.claimed[i].value > 0){
				if (p.claimed[i].value == p.player_Value){
					p.hand_Points++;
				}
			}
			else{
				flowers++;
			}
		}
	}

	// If the winning hand is only one suit and no honors, it is a full flush.
	if (m[0].suit != "NONE"){
		if (m[0].suit == m[1].suit && m[1].suit == m[2].suit && m[2].suit == m[3].suit && m[3].suit == p.pair.suit){
			flush = true;
		}
		// If it is only one suit and honors, it is a half flush.
		else if ((m[0].suit == m[1].suit && m[1].suit == m[2].suit && m[2].suit == m[3].suit && m[3].suit != p.pair.suit &&
			(p.pair.suit == "Green Dragon" || p.pair.suit == "White Dragon" || p.pair.suit == "Red Dragon" || p.pair.suit == "North" ||
			p.pair.suit == "South" || p.pair.suit == "West" || p.pair.suit == "East")) || (m[0].suit == m[1].suit && m[1].suit == m[2].suit && m[2].suit != m[3].suit && m[2].suit == p.pair.suit &&
			(m[3].suit == "Green Dragon" || m[3].suit == "White Dragon" || m[3].suit == "Red Dragon" || m[3].suit == "North" ||
			m[3].suit == "South" || m[3].suit == "West" || m[3].suit == "East")) || (m[0].suit == m[1].suit && m[1].suit != m[2].suit && m[1].suit == m[3].suit && m[3].suit == p.pair.suit &&
			(m[2].suit == "Green Dragon" || m[2].suit == "White Dragon" || m[2].suit == "Red Dragon" || m[2].suit == "North" ||
			m[2].suit == "South" || m[2].suit == "West" || m[2].suit == "East")) || (m[0].suit == m[2].suit && m[1].suit != m[2].suit && m[2].suit == m[3].suit && m[3].suit == p.pair.suit &&
			(m[1].suit == "Green Dragon" || m[1].suit == "White Dragon" || m[1].suit == "Red Dragon" || m[1].suit == "North" ||
			m[1].suit == "South" || m[1].suit == "West" || m[1].suit == "East")) || (m[0].suit != m[1].suit && m[1].suit == m[2].suit && m[2].suit == m[3].suit && m[3].suit == p.pair.suit &&
			(m[0].suit == "Green Dragon" || m[0].suit == "White Dragon" || m[0].suit == "Red Dragon" || m[0].suit == "North" ||
			m[0].suit == "South" || m[0].suit == "West" || m[0].suit == "East"))){
			h_Flush = true;
		}
	}

	// For the winning melds (open and closed)...
	for (int i = 0; i < 4; i++){
		// If any of the melds were not closed, the hand is not concealed.
		if (!m[i].hidden){
			fully_Concealed = false;
		}
		// Points are earned for making a hand of only chows and a pair.
		if (m[i].name == "Chow"){
			all_Chows++;
		}
		if (m[i].name == "Pong" || m[i].name == "Kong"){
			// Points are earned for making a hand of only pongs and a pair.
			all_Pongs++;
			// More points are earned if all 4 pongs are concealed.
			if (m[i].hidden == true){
				conc_Pongs++;
			}
			// 1 point for each pong of a dragon.
			// If the hand contains 2 pongs of dragons and a pair of the other, the player
			// gets Small Three Dragons. If the hand contains 3 pongs of all the dragons, the
			// player gets Big Three Dragons.
			// The hand is worth more points if it is only made of honors tiles (winds and dragons).
			if (m[i].suit == "Green Dragon" || m[i].suit == "White Dragon" || m[i].suit == "Red Dragon"){
				yakuhai += 1;
				three_Dragons += 1;
				all_Honors += 1;
			}
			// 1 point for each pong of the player's wind (determined by seat order and
			// current hand number).
			// If the hand contains 3 pongs of winds and a pair of the other, the player
			// gets Small Four Winds. If the hand contains 4 pongs of all the winds, the
			// player gets Big Four Winds.
			if ((m[i].suit == "East" && m[i].melded[0].value == p.player_Value) ||
				(m[i].suit == "South" && m[i].melded[0].value == p.player_Value) ||
				(m[i].suit == "West" && m[i].melded[0].value == p.player_Value) ||
				(m[i].suit == "North" && m[i].melded[0].value == p.player_Value)){
				yakuhai += 1;
				four_Winds += 1;
				all_Honors += 1;
			}
			// No points are awarded for any pong of winds that doesn't match the round wind
			// or the player's wind, but they do count for S/B Four Winds and All Honors.
			else if (m[i].suit == "East" || m[i].suit == "South" || m[i].suit == "West" || m[i].suit == "North") {
				four_Winds += 1;
				all_Honors += 1;
			}
			// 1 point for each pong of the round wind. Round wind changes every 4 hands
			// when deal reaches the initial east player again.
			if ((m[i].suit == "East" && m[i].melded[0].value == round_Counter) ||
				(m[i].suit == "South" && m[i].melded[0].value == round_Counter) ||
				(m[i].suit == "West" && m[i].melded[0].value == round_Counter) ||
				(m[i].suit == "North" && m[i].melded[0].value == round_Counter)){
				yakuhai += 1;
			}
			

		}
	}

	if (lt && tsumo){
		if (language == "Japanese") std::cout << "Houtei." << endl;
		else if (language == "Chinese") std::cout << "Hoi Day." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 2;
		else if (game_Running == CHINESE) p.hand_Points += 8;
	}
	else if (lt){
		if (language == "Japanese") std::cout << "Haitei." << endl;
		else if (language == "Chinese") std::cout << "Hoi Day." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 2;
		else if (game_Running == CHINESE) p.hand_Points += 8;
	}
	if (game_Running == HONGKONG){
		if (flowers == 8){ // If the player has no flowers or seasons, 1 point.
			std::cout << "No Flowers or Seasons." << endl;
			p.hand_Points += 1;
		}
		if (all_Flowers(p.claimed)){ // If the player has all the flowers and seasons, 6 points.
			std::cout << "All Flowers and Seasons." << endl;
			p.hand_Points += 6;
		}
	}
	if (all_Chows == 4){ // If the player's hand is only chows, 1 point.
		if (language == "Japanese") std::cout << "All chows." << endl;
		else if (language == "Chinese") std::cout << "Ping Woo." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 1;
		else if (game_Running == CHINESE) p.hand_Points += 2;
	}
	else if (all_Pongs == 4){ // If the player's hand is only pongs, 3 points.
		if (language == "Japanese") std::cout << "All pongs." << endl;
		else if (language == "Chinese") std::cout << "Dui Dui Woo." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 3;
		else if (game_Running == CHINESE) p.hand_Points += 6;
	}
	if (game_Running == CHINESE){
		if (conc_Pongs == 2){
			std::cout << "Two Concealed Pongs." << endl;
			p.hand_Points += 2;
		}
		else if (conc_Pongs == 3){
			std::cout << "Three Concealed Pongs." << endl;
			p.hand_Points += 16;
		}
	}
	if (conc_Pongs == 4){ // If the player's hand is only concealed pongs, 8 points.
		if (language == "Japanese") std::cout << "Suu Ankou." << endl;
		else if (language == "Chinese") std::cout << "Kan Kan Woo." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 8;
		else if (game_Running == CHINESE) p.hand_Points += 64;
	}
	if (tsumo){ // If the player wins off of a self draw, 1 point.
		if (language == "Japanese") std::cout << "Tsumo." << endl;
		else if (language == "Chinese") std::cout << "Chi Mo." << endl;
		p.hand_Points += 1;
	}
	if (fully_Concealed && tsumo){ // If the player wins on a self draw and the hand is concealed, 1 extra point.
		if (language == "Japanese") std::cout << "Menzen Tsumo." << endl;
		else if (language == "Chinese") std::cout << "Moon Ching." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 1;
		else if (game_Running == CHINESE) p.hand_Points += 4;
	}
	if (kong_Replace){ // If the player wins on a replacement tile after a kong, 2 points.
		if (language == "Japanese") std::cout << "Rinchan Kaihou." << endl;
		else if (language == "Chinese") std::cout << "Gong Sheung Far." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 2;
		else if (game_Running == CHINESE) p.hand_Points += 8;
	}
	// If the winning tile is the last tile drawn, and is a 1 Pin, the
	// player wins with Moon from the Bottom of the Sea, 3 points.
	if (game_Running == HONGKONG){
		if (lt && (l.suit == "Pin" && l.value == 1)){
			if (language == "Japanese") std::cout << "Moon From the Bottom of the Sea." << endl;
			else if (language == "Chinese") std::cout << "Hoi Day Lao Yeut." << endl;
			p.hand_Points += 3;
		}
		// If the winning tile is the 5 Pin in a 4-5-6 Pin chow, the player
		// wins with Plum Blossom on the Roof, 3 points.
		if (kong_Replace && m[3].name == "Chow" && (m[3].melded[0].suit == "Pin" && (m[3].melded[0].value == 4 || m[3].melded[0].value == 6) && (m[3].melded[1].value == 4 || m[3].melded[1].value == 6) && m[3].melded[2].value == 5) &&
			(l.suit == "Pin" && l.value == 5)){
			if (language == "Japanese") std::cout << "Plum Blossom on the Roof." << endl;
			else if (language == "Chinese") std::cout << "Gong Shueng Mui Far." << endl;
			p.hand_Points += 3;
		}
	}
	if (flush){ // If the player wins with a full flush, 6 points.
		if (language == "Japanese") std::cout << "Chin Itsu." << endl;
		else if (language == "Chinese") std::cout << "Ching Yak Sik." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 6;
		else if (game_Running == CHINESE) p.hand_Points += 24;
	}
	else if (h_Flush){ // If the player wins with a half flush, 3 points.
		if (language == "Japanese") std::cout << "Hon Itsu." << endl;
		else if (language == "Chinese") std::cout << "Won Yat Sik." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 3;
		else if (game_Running == CHINESE) p.hand_Points += 6;
	}
	// If the player wins with Small Three Dragons, 3 points.
	if (three_Dragons == 2 && (p.pair.suit == "Green Dragon" || p.pair.suit == "White Dragon" || p.pair.suit == "Red Dragon")){
		if (language == "Japanese") std::cout << "Shou Sangen." << endl;
		else if (language == "Chinese") std::cout << "Siu Sam Yuen." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 3;
		else if (game_Running == CHINESE) p.hand_Points += 64;
	}
	else if (three_Dragons == 3) { // If the player wins with Big Three Dragons, 6 points.
		if (language == "Japanese") std::cout << "Dai Sangen." << endl;
		else if (language == "Chinese") std::cout << "Dai Sam Yeun." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 6;
		else if (game_Running == CHINESE) p.hand_Points += 88;
	}
	// If the player wins with Small Four Winds, 6 points.
	if (four_Winds == 3 && (p.pair.suit == "East" || p.pair.suit == "South" || p.pair.suit == "West" || p.pair.suit == "North")){
		if (language == "Japanese") std::cout << "Shu Suushi." << endl;
		else if (language == "Chinese") std::cout << "Siu Sei Hei." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 6;
		else if (game_Running == CHINESE) p.hand_Points += 64;
	}
	else if (four_Winds == 4){ // If the player wins with Big Four Winds, 8 points.
		if (language == "Japanese") std::cout << "Dai Suushi." << endl;
		else if (language == "Chinese") std::cout << "Dai Sei Hei." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 8;
		else if (game_Running == CHINESE) p.hand_Points += 88;
	}
	// If the player wins with All Honors, 8 points.
	if (all_Honors == 4 && (p.pair.suit == "Green Dragon" || p.pair.suit == "White Dragon" || p.pair.suit == "Red Dragon" ||
		p.pair.suit == "East" || p.pair.suit == "South" || p.pair.suit == "West" || p.pair.suit == "North")){
		if (language == "Japanese") std::cout << "Tsuu Iisou." << endl;
		else if (language == "Chinese") std::cout << "Chuen Tse." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 8;
		else if (game_Running == CHINESE) p.hand_Points += 64;
	}
	if (game_Running == CHINESE){
		if (all_Honors == 0 && (p.pair.suit != "Green Dragon" || p.pair.suit != "White Dragon" || p.pair.suit != "Red Dragon" ||
			p.pair.suit != "East" || p.pair.suit != "South" || p.pair.suit != "West" || p.pair.suit != "North")){
			std::cout << "No Honors." << endl;
			p.hand_Points += 1;
		}
	}
	// If the player wins with special hand, 13 Orphans, 8 points.
	// 13 Orphans is 1 and 9 of each suit and 1 of every honor plus a pair of any
	// of those tiles.
	if (p.thirteen_Orphans){
		if (language == "Japanese") std::cout << "Kokushi Muso." << endl;
		else if (language == "Chinese") std::cout << "Sup Sam Yiu." << endl;
		if (game_Running == HONGKONG) p.hand_Points += 8;
		else if (game_Running == CHINESE) p.hand_Points += 88;
	}
	// If the east player wins on the first draw, tenho, 8 points.
	if (game_Running != CHINESE){
		if (p.tenho){
			if (language == "Japanese") std::cout << "Tenho." << endl;
			else if (language == "Chinese") std::cout << "Tin Woo." << endl;
			p.hand_Points += 8;
		}
		// If a non-east player wins on the first discard, chiho, 8 points.
		else if (p.chiho){
			if (language == "Japanese") std::cout << "Chiho." << endl;
			else if (language == "Chinese") std::cout << "Dei Woo." << endl;
			p.hand_Points += 8;
		}
	}
	if (game_Running == CHINESE){
		p.hand_Points += yakuhai * 2;
	}
	else {
		p.hand_Points += yakuhai;
	}
	if (yakuhai > 0){
		std::cout << "Number of yakuhai: " << yakuhai << endl << endl;
	}
	cout << p.hand_Points << endl;

	return p.hand_Points;
}

// Looks at the player's claimed flowers and seasons.
// Takes a player's claimed tiles.
// Returns true if the player has 8 flowers and seasons. False otherwise.
bool all_Flowers(Tile* f){
	if (f[7].value > 0){
		return true;
	}
	return false;
}

// Changes a player's points based on round and calculated points.
// Takes a player and the total amount of calculated points.
void update_Points(Player p, int t){
	if (p.wind == "East"){
		if (turn_Counter == EAST) east.points += t;
		else if (turn_Counter == SOUTH) south.points += t;
		else if (turn_Counter == WEST) west.points += t;
		else if (turn_Counter == NORTH) north.points += t;
	}
	else if (p.wind == "South"){
		if (turn_Counter == EAST) south.points += t;
		else if (turn_Counter == SOUTH) west.points += t;
		else if (turn_Counter == WEST) north.points += t;
		else if (turn_Counter == NORTH) east.points += t;
	}
	else if (p.wind == "West"){
		if (turn_Counter == EAST) west.points += t;
		else if (turn_Counter == SOUTH) north.points += t;
		else if (turn_Counter == WEST) east.points += t;
		else if (turn_Counter == NORTH) south.points += t;
	}
	else if (p.wind == "North"){
		if (turn_Counter == EAST) north.points += t;
		else if (turn_Counter == SOUTH) east.points += t;
		else if (turn_Counter == WEST) south.points += t;
		else if (turn_Counter == NORTH) west.points += t;
	}
}

// Creates all the player hands at the same time.
// Takes the setup wall.
// Returns nothing but all players' hands are set.
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

// Shows all the tiles that have been discarded.
void see_Discards(){
	for (int i = 0; i < 144; i++){
		if (discards[i].value > 0){
			std::cout << "Tile " << i + 1 << " in discards: " << discards[i].value
				<< " " << discards[i].suit << endl;
		}
	}
}

// Easy version AI. Discards a tile at random after drawing a tile.
// Takes the player, the drawn tile, the setup wall, a random number for discarding and
// whether or not it is a meld.
void easy_AI(Player p, Tile d, Tile* w, int x, bool meld = false){
	// A selected tile from a player's hand is discarded into the discards.
	Player this_Player = p;
	last_Discarder = this_Player;
	Tile temp;
	bool discarded = false;
	last_Tile = (see_Next_Tile(setup_Wall).value <= 0) ? true : false;

	if (!meld){
		while (d.suit == "Winter" || d.suit == "Summer" || d.suit == "Autumn" || d.suit == "Spring" ||
			d.suit == "Plum" || d.suit == "Orchid" || d.suit == "Bamboo" || d.suit == "Chrysanthemum"){
			for (int i = 0; i < 8; i++){
				if (this_Player.claimed[i].value > 0){
					this_Player.claimed[i] = d;
				}
			}
			d = draw_Tile(w, 1);
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
	}

	while (!discarded){
		if (x < 14){
			if (this_Player.hand[x - 1].value > 0){
				temp = this_Player.hand[x - 1];
				if (!meld) this_Player.hand[x - 1] = d;
				else this_Player.hand[x - 1] = NULL;
				discarded = true;
			}
			else {
				x = rand() % 14 + 1;
			}
		}
		else if (x == 14){
			if (!meld){
				temp = d;
				discarded = true;
			}
			else{
				x = rand() % 14 + 1;
			}
		}
	}

	discarded = false;
	int k = 0;
	while (!discarded){
		if (discards[k].value <= 0){
			if (!meld) discards[k] = temp;
			else discards[k - 1] = temp;
			last_Discard_Tile = temp;
			discarded = true;
		}
		else {
			k++;
		}
	}

	//this_Player.make_Pairs(this_Player.hand);
	this_Player.hand = this_Player.sort_Hand(this_Player.hand);
	this_Player.make_Chows(this_Player.hand);
	this_Player.make_Pongs(this_Player.hand);
	if (tenho_Counter == 0 && this_Player.wind == "East"){
		tenho_Counter = 1;
	}
	else if (chiho_Counter == 0 && this_Player.wind == "South"){
		chiho_Counter = 1;
	}
	if (!this_Player.was_North){
		turn += 1;
	}
	else{
		turn = EAST;
	}
}

void update(){
	srand(time(NULL));

	std::cout << "What kind of mahjong do you want to play?" << endl;
	std::cout << "J -- Japanese Riichi Mahjong\nH -- Hong Kong Style\nM -- Modern Chinese Style\nQ -- Quit" << endl;
	cin >> e_Selector;
	
#pragma region
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
#pragma endregion Enum Select

#pragma region 
	if (game_Running == JAPANESE) {
		while (game_Running == JAPANESE){
			std::cout << "This mode is not implemented yet.\n";
			break;
		}
	}
#pragma endregion Japanese Mahjong

#pragma region 
	if (game_Running == HONGKONG){
		while (game_Running == HONGKONG){ // While the player is playing Hong Kong Style mahjong.
			north.was_North = true; // Identifies the north player as the original north player.
			hand_Won = false; // Sets the hand as not yet won.
			tenho_Counter = 0; // Identifies the first draw.
			round_Counter = EAST + move_Round; // Sets the current round.
			std::cout << "Round: " << round_Counter + 1 << endl; // Displays current round.
			std::cout << "Turn: " << turn_Counter + 1 << endl; // Displays current turn of the round.
			
			// Each time the hand ends, the seat winds of the players change.
#pragma region
			if (turn_Counter == EAST){
				east.wind = "East";
				east.player_Value = 1;
				south.wind = "South";
				south.player_Value = 2;
				west.wind = "West";
				west.player_Value = 3;
				north.wind = "North";
				north.player_Value = 4;
				turn = EAST;
			}
			else if (turn_Counter == SOUTH){
				east.wind = "North";
				east.player_Value = 4;
				south.wind = "East";
				south.player_Value--;
				west.wind = "South";
				west.player_Value--;
				north.wind = "West";
				north.player_Value--;
				turn = SOUTH;
			}	
			else if (turn_Counter == WEST){
				east.wind = "West";
				east.player_Value--;
				south.wind = "North";
				south.player_Value = 4;
				west.wind = "East";
				west.player_Value--;
				north.wind = "South";
				north.player_Value--;
				turn = WEST;
			}	
			else if (turn_Counter == NORTH){
				east.wind = "South";
				east.player_Value--;
				south.wind = "West";
				south.player_Value--;
				west.wind = "North";
				west.player_Value = 4;
				north.wind = "East";
				north.player_Value--;
				turn = NORTH;
			}
#pragma endregion Shift Winds

			// Initializes each players' melds and other little bits that
			// have to be set to a default value at the beginning of each hand.
#pragma region
			east.tenho = false;
			east.chiho = false;
			south.tenho = false;
			south.chiho = false;
			west.tenho = false;
			west.chiho = false;
			north.tenho = false;
			north.chiho = false;
			for (int i = 0; i < 144; i++){
				if (discards[i].value > 0){
					discards[i] = NULL;
				}
			}
			for (int k = 0; k < 4; k++){
				if (east.melds[k].melded != NULL){
					if (east.melds[k].melded[0].value > 0){
						east.melds[k].name = "NONE";
						east.melds[k].suit = "NONE";
						east.melds[k].melded[0] = NULL;
						east.melds[k].melded[1] = NULL;
						east.melds[k].melded[2] = NULL;
						if (east.melds[k].melded[3].value > 0){
							east.melds[k].melded[3] = NULL;
						}
					}
				}
				if (south.melds[k].melded != NULL){
					if (south.melds[k].melded[0].value > 0){
						south.melds[k].name = "NONE";
						south.melds[k].suit = "NONE";
						south.melds[k].melded[0] = NULL;
						south.melds[k].melded[1] = NULL;
						south.melds[k].melded[2] = NULL;
						if (south.melds[k].melded[3].value > 0){
							south.melds[k].melded[3] = NULL;
						}
					}
				}
				if (west.melds[k].melded != NULL){
					if (west.melds[k].melded[0].value > 0){
						west.melds[k].name = "NONE";
						west.melds[k].suit = "NONE";
						west.melds[k].melded[0] = NULL;
						west.melds[k].melded[1] = NULL;
						west.melds[k].melded[2] = NULL;
						if (west.melds[k].melded[3].value > 0){
							west.melds[k].melded[3] = NULL;
						}
					}
				}
				if (north.melds[k].melded != NULL){
					if (north.melds[k].melded[0].value > 0){
						north.melds[k].name = "NONE";
						north.melds[k].suit = "NONE";
						north.melds[k].melded[0] = NULL;
						north.melds[k].melded[1] = NULL;
						north.melds[k].melded[2] = NULL;
						if (north.melds[k].melded[3].value > 0){
							north.melds[k].melded[3] = NULL;
						}
					}
				}
			}
			for (int j = 0; j < 8; j++){
				if (east.claimed[j].value > 0){
					east.claimed[j] = NULL;
				}
				if (south.claimed[j].value > 0){
					east.claimed[j] = NULL;
				}
				if (west.claimed[j].value > 0){
					west.claimed[j] = NULL;
				}
				if (north.claimed[j].value > 0){
					north.claimed[j] = NULL;
				}
			}

			east.has_Won_Hand = false;
			south.has_Won_Hand = false;
			west.has_Won_Hand = false;
			north.has_Won_Hand = false;
#pragma endregion Reset Game

			roll = dice_A.HK_roll();
			std::cout << "Rolled " << roll << "\n\n" << endl;

			// Create the wall that is used to draw from.
			setup_Wall = wall_Split(wall_Setup(), roll);
			
			// Set up the initial hand for the current hand for all the players.
#pragma region
			hand_Create(setup_Wall); // Create all the hands at the same time.

			// Then sort the hands and look to see if there are any flowers or seasons.
			// Flowers and seasons are removed from the hand, and replaced from the end
			// of the wall.
			// If the position of the flower or season doesn't match a player's seat,
			// that player's hand is worth 0 points. Each flower or season of that
			// player's seat is worth 1 point (up to 2 points).
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
						if (south.claimed[j].value != south.player_Value){
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
						if (west.claimed[j].value != west.player_Value){
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
						if (north.claimed[j].value != north.player_Value){
							north.hand_Points -= 1;
						}
					}
				}
			}
			north.hand = north.sort_Hand(north.hand);
#pragma endregion Initial Hands

			// TEST CODE!!! REMOVE AFTER DONE!!!
			// END TEST CODE!!

			while (round_Counter < 6){

				// All the parts of each player's turn.
#pragma region
				pass = false;
				if (!east.is_AI){
					if (turn == EAST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
						while (turn == EAST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							east.make_Pairs(east.hand);
							east.make_Chows(east.hand);
							east.make_Pongs(east.hand);
							std::cout << east.wind << " player, choose:" << endl;
							std::cout << "D: Draw a tile.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;

							if (east.can_Chow(east.hand, last_Discard_Tile)){
								std::cout << "C: Call chow from last discard.\n";
							}
							if (east.can_Pong(east.hand, last_Discard_Tile)){
								std::cout << "P: Call pong from last discard.\n";
							}
							if (east.can_Kong(east.hand, last_Discard_Tile)){
								std::cout << "K: Call kong from last discard.\n";
							}
							if (last_Discard_Tile.value > 0){
								if (east.can_Win(east.hand, last_Discard_Tile)){
									std::cout << "R: Call mahjong from last discard.\n";
								}
							}
							cin >> option;

							if (option == 'D' || option == 'd'){
								east.hand = discard_Tile(east, draw_Tile(setup_Wall), setup_Wall);
								east.see_Hand();
								counter = 0;
								for (int i = 0; i < 144; i++){
									if (setup_Wall[i].value > 0){
										counter++;
									}
								}
								if (east.wind == "North" && tenho_Counter == 0){
									tenho_Counter = 1;
								}
								else if (east.wind == "South" && chiho_Counter == 0){
									chiho_Counter = 1;
								}
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
#pragma region
							else if (option == 'C' || option == 'c'){
								std::cout << "Which chow do you want to make?" << endl;
								east.show_Chows_Choices();
								cin >> chow_Choice;
								for (int i = 0; i < 4; i++){
									if (east.melds[i].name == "NONE"){
										east.melds[i] = east.chow_Choices[chow_Choice-1];
										east.melds[i].hidden = false;
										east.chow_Choices[chow_Choice - 1] = Meld();
										for (int j = 0; j < 13; j++){
											if (east.hand[j].value > 0){
												if (east.hand[j].suit == east.melds[i].suit && (east.hand[j].value == east.melds[i].melded[0].value || east.hand[j].value == east.melds[i].melded[1].value || east.hand[j].value == east.melds[i].melded[2].value) &&
													(east.hand[j + 1].suit == east.melds[i].suit && (east.hand[j + 1].value == east.melds[i].melded[0].value || east.hand[j + 1].value == east.melds[i].melded[1].value || east.hand[j + 1].value == east.melds[i].melded[2].value))){
													east.hand[j] = NULL;
													east.hand[j + 1] = NULL;
												}
											}
										}
										east.hand = discard_Tile(east, last_Discard_Tile, setup_Wall, true);
										break;
									}
								}
								east.see_Hand();
								turn += 1;
							}
#pragma endregion Chow-ing

#pragma region
							else if (option == 'P' || option == 'p'){
								for (int i = 0; i < 4; i++){
									if (east.melds[i].name == "NONE"){
										for (int j = 0; j < 13; j++){
											if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == last_Discard_Tile.suit){
												east.melds[i] = Meld(east.hand[j], east.hand[j + 1], last_Discard_Tile);
												east.melds[i].hidden = false;
												east.hand[j] = NULL;
												east.hand[j + 1] = NULL;
												east.hand = discard_Tile(east, last_Discard_Tile, setup_Wall, true);
												break;
											}
										}
									}
								}
								east.see_Hand();
								turn += 1;
							}
#pragma endregion Pong-ing

#pragma region
							else if (option == 'K' || option == 'k'){
								for (int i = 0; i < 4; i++){
									if (east.melds[i].name == "NONE"){
										for (int j = 0; j < 13; j++){
											if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == east.hand[j + 2].value && east.hand[j + 2].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == east.hand[j + 2].suit && east.hand[j + 2].suit == last_Discard_Tile.suit){
												east.melds[i] = Meld(east.hand[j], east.hand[j + 1], east.hand[j + 2], last_Discard_Tile);
												east.melds[i].hidden = false;
												east.hand[j] = NULL;
												east.hand[j + 1] = NULL;
												east.hand[j + 2] = NULL;
												east.hand = discard_Tile(east, draw_Tile(setup_Wall,1), setup_Wall);
												break;
											}
										}
									}
								}
								east.see_Hand();
								turn += 1;
							}
#pragma endregion Kong-ing

#pragma region
							else if (option == 'R' || option == 'r'){
								if (chiho_Counter == 0){
									if (east.wind != "East") east.chiho = true;
								}
								for (int i = 0; i < 9; i++){
									if (east.possible_Chows[i].name != "NONE"){
										for (int j = 0; j < 4; j++){
											if (east.melds[j].name == "NONE"){
												east.melds[j] = east.possible_Chows[i];
												for (int k = 0; k < 4; k++){
													// To Do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
													if (east.melds[j].suit == east.possible_Chows[i + 1].suit && east.melds[j].melded[k].value == east.possible_Chows[i + 1].melded[k].value){
														east.possible_Chows[i + 1] = Meld();
													}
												}
												break;
											}
										}
									}
									if (i < 4){
										if (east.possible_Pongs[i].name != "NONE"){
											for (int j = 0; j < 4; j++){
												if (east.melds[j].name == "NONE"){
													east.melds[j] = east.possible_Pongs[i];
													break;
												}
											}
										}
									}
								}
								for (int i = 0; i < 4; i++){
									if (east.melds[i].name != "NONE"){
										std::cout << "Meld " << i + 1 << ": " << east.melds[i].name << endl;
										for (int j = 0; j < 3; j++){
											std::cout << east.melds[i].melded[j].value << " " << east.melds[i].melded[j].suit << endl;
										}
									}
								}
								east.has_Won_Hand = true;
								std::cout << "Points won: " << calculate_Points(east.melds, east.hand, last_Discard_Tile, east) << "\n\n";
								east.points += calculate_Points(east.melds, east.hand, last_Discard_Tile, east, false);
								last_Discarder.points -= east.points;
								hand_Won = true;
							}
#pragma endregion Winning

						}
					}
				}
#pragma region
				else {
					if (turn == EAST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
						std::cout << east.wind << " takes a turn..." << endl;
						easy_AI(east, draw_Tile(setup_Wall), setup_Wall, (rand() % 14 + 1));
						counter = 0;
						for (int i = 0; i < 144; i++){
							if (setup_Wall[i].value > 0){
								counter++;
							}
						}
					}
				}
#pragma endregion AI
#pragma endregion East Turn

#pragma region
				pass = false;
				if (!south.is_AI){
					if (turn == SOUTH && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
						while (turn == SOUTH && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							south.make_Pairs(south.hand);
							south.make_Chows(south.hand);
							south.make_Pongs(south.hand);
							std::cout << south.wind << " player, choose:" << endl;
							std::cout << "D: Draw a tile.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;

							if (south.can_Chow(south.hand, last_Discard_Tile)){
								std::cout << "C: Call chow from last discard.\n";
							}
							if (south.can_Pong(south.hand, last_Discard_Tile)){
								std::cout << "P: Call pong from last discard.\n";
							}
							if (south.can_Kong(south.hand, last_Discard_Tile)){
								std::cout << "K: Call kong from last discard.\n";
							}
							if (last_Discard_Tile.value > 0){
								if (south.can_Win(south.hand, last_Discard_Tile)){
									std::cout << "R: Call mahjong from last discard.\n";
								}
							}
							cin >> option;

							if (option == 'D' || option == 'd'){
								south.hand = discard_Tile(south, draw_Tile(setup_Wall), setup_Wall);
								south.see_Hand();
								counter = 0;
								for (int i = 0; i < 144; i++){
									if (setup_Wall[i].value > 0){
										counter++;
									}
								}
								if (south.wind == "North" && tenho_Counter == 0){
									tenho_Counter = 1;
								}
								else if (south.wind == "South" && chiho_Counter == 0){
									chiho_Counter = 1;
								}
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
#pragma region
							else if (option == 'C' || option == 'c'){
								std::cout << "Which chow do you want to make?" << endl;
								south.show_Chows_Choices();
								cin >> chow_Choice;
								for (int i = 0; i < 4; i++){
									if (south.melds[i].name == "NONE"){
										south.melds[i] = south.chow_Choices[chow_Choice - 1];
										south.melds[i].hidden = false;
										south.chow_Choices[chow_Choice - 1] = Meld();
										for (int j = 0; j < 13; j++){
											if (south.hand[j].value > 0){
												if (south.hand[j].suit == south.melds[i].suit && (south.hand[j].value == south.melds[i].melded[0].value || south.hand[j].value == south.melds[i].melded[1].value || south.hand[j].value == south.melds[i].melded[2].value) &&
													(south.hand[j + 1].suit == south.melds[i].suit && (south.hand[j + 1].value == south.melds[i].melded[0].value || south.hand[j + 1].value == south.melds[i].melded[1].value || south.hand[j + 1].value == south.melds[i].melded[2].value))){
													south.hand[j] = NULL;
													south.hand[j + 1] = NULL;
												}
											}
										}
										south.hand = discard_Tile(south, last_Discard_Tile, setup_Wall, true);
										break;
									}
								}
								south.see_Hand();
								turn += 1;
							}
#pragma endregion Chow-ing

#pragma region
							else if (option == 'P' || option == 'p'){
								for (int i = 0; i < 4; i++){
									if (south.melds[i].name == "NONE"){
										for (int j = 0; j < 13; j++){
											if (south.hand[j].value == south.hand[j + 1].value && south.hand[j + 1].value == last_Discard_Tile.value && south.hand[j].suit == south.hand[j + 1].suit && south.hand[j + 1].suit == last_Discard_Tile.suit){
												south.melds[i] = Meld(south.hand[j], south.hand[j + 1], last_Discard_Tile);
												south.melds[i].hidden = false;
												south.hand[j] = NULL;
												south.hand[j + 1] = NULL;
												south.hand = discard_Tile(south, last_Discard_Tile, setup_Wall, true);
												break;
											}
										}
									}
								}
								south.see_Hand();
								turn += 1;
							}
#pragma endregion Pong-ing

#pragma region
							else if (option == 'K' || option == 'k'){
								for (int i = 0; i < 4; i++){
									if (south.melds[i].name == "NONE"){
										for (int j = 0; j < 13; j++){
											if (south.hand[j].value == south.hand[j + 1].value && south.hand[j + 1].value == south.hand[j + 2].value && south.hand[j + 2].value == last_Discard_Tile.value && south.hand[j].suit == south.hand[j + 1].suit && south.hand[j + 1].suit == south.hand[j + 2].suit && south.hand[j + 2].suit == last_Discard_Tile.suit){
												south.melds[i] = Meld(south.hand[j], south.hand[j + 1], south.hand[j + 2], last_Discard_Tile);
												south.melds[i].hidden = false;
												south.hand[j] = NULL;
												south.hand[j + 1] = NULL;
												south.hand[j + 2] = NULL;
												south.hand = discard_Tile(south, draw_Tile(setup_Wall, 1), setup_Wall);
												break;
											}
										}
									}
								}
								south.see_Hand();
								turn += 1;
							}
#pragma endregion Kong-ing

#pragma region
							else if (option == 'R' || option == 'r'){
								if (chiho_Counter == 0){
									if (south.wind != "South") south.chiho = true;
								}
								for (int i = 0; i < 9; i++){
									if (south.possible_Chows[i].name != "NONE"){
										for (int j = 0; j < 4; j++){
											if (south.melds[j].name == "NONE"){
												south.melds[j] = south.possible_Chows[i];
												for (int k = 0; k < 4; k++){
													// To Do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
													if (south.melds[j].suit == south.possible_Chows[i + 1].suit && south.melds[j].melded[k].value == south.possible_Chows[i + 1].melded[k].value){
														south.possible_Chows[i + 1] = Meld();
													}
												}
												break;
											}
										}
									}
									if (i < 4){
										if (south.possible_Pongs[i].name != "NONE"){
											for (int j = 0; j < 4; j++){
												if (south.melds[j].name == "NONE"){
													south.melds[j] = south.possible_Pongs[i];
													break;
												}
											}
										}
									}
								}
								for (int i = 0; i < 4; i++){
									if (south.melds[i].name != "NONE"){
										std::cout << "Meld " << i + 1 << ": " << south.melds[i].name << endl;
										for (int j = 0; j < 3; j++){
											std::cout << south.melds[i].melded[j].value << " " << south.melds[i].melded[j].suit << endl;
										}
									}
								}
								south.has_Won_Hand = true;
								std::cout << "Points won: " << calculate_Points(south.melds, south.hand, last_Discard_Tile, south) << "\n\n";
								south.points += calculate_Points(south.melds, south.hand, last_Discard_Tile, south, false);
								last_Discarder.points -= south.points;
								hand_Won = true;
							}
#pragma endregion Winning

						}
					}
				}
				else {
					if (turn == SOUTH & !hand_Won && see_Next_Tile(setup_Wall).value > 0){
						std::cout << south.wind << " takes a turn..." << endl;
						easy_AI(south, draw_Tile(setup_Wall), setup_Wall, (rand() % 14 + 1));
						std::cout << south.wind << " discards " << last_Discard_Tile.value << " " << last_Discard_Tile.suit << endl;

#pragma region 
						if (east.can_Pong(east.hand, last_Discard_Tile) || east.can_Kong(east.hand, last_Discard_Tile)){
							while (!pass){
								std::cout << "P: Call pong from last discard.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards.\nS: Pass.\n";
								if (east.can_Kong(east.hand, last_Discard_Tile)){
									std::cout << "K: Call kong from last discard.\n";
								}
								cin >> option;
								if (option == 'P' || option == 'p'){
									for (int i = 0; i < 4; i++){
										if (east.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == last_Discard_Tile.suit){
													east.melds[i] = Meld(east.hand[j], east.hand[j + 1], last_Discard_Tile);
													east.melds[i].hidden = false;
													east.hand[j] = NULL;
													east.hand[j + 1] = NULL;
													east.hand = discard_Tile(east, last_Discard_Tile, setup_Wall, true);
													break;
												}
											}
										}
									}
									east.see_Hand();
									pass = true;
									turn = SOUTH;
								}
								else if (option == 'K' || option == 'k'){
									for (int i = 0; i < 4; i++){
										if (east.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == east.hand[j + 2].value && east.hand[j + 2].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == east.hand[j + 2].suit && east.hand[j + 2].suit == last_Discard_Tile.suit){
													east.melds[i] = Meld(east.hand[j], east.hand[j + 1], east.hand[j + 2], last_Discard_Tile);
													east.melds[i].hidden = false;
													east.hand[j] = NULL;
													east.hand[j + 1] = NULL;
													east.hand[j + 2] = NULL;
													east.hand = discard_Tile(east, draw_Tile(setup_Wall, 1), setup_Wall);
													break;
												}
											}
										}
									}
									east.see_Hand();
									pass = true;
									turn = SOUTH;
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
								else if (option == 'S' || option == 's'){
									pass = true;
									turn = WEST;
								}
							}
						}
#pragma endregion Pong Interrupt

						counter = 0;
						for (int i = 0; i < 144; i++){
							if (setup_Wall[i].value > 0){
								counter++;
							}
						}
					}
				}
#pragma endregion South Turn

#pragma region
				pass = false;
				if (!west.is_AI){
					if (turn == WEST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
						while (turn == WEST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							west.make_Pairs(west.hand);
							west.make_Chows(west.hand);
							west.make_Pongs(west.hand);
							std::cout << west.wind << " player, choose:" << endl;
							std::cout << "D: Draw a tile.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;

							if (west.can_Chow(west.hand, last_Discard_Tile)){
								std::cout << "C: Call chow from last discard.\n";
							}
							if (west.can_Pong(west.hand, last_Discard_Tile)){
								std::cout << "P: Call pong from last discard.\n";
							}
							if (west.can_Kong(west.hand, last_Discard_Tile)){
								std::cout << "K: Call kong from last discard.\n";
							}
							if (last_Discard_Tile.value > 0){
								if (west.can_Win(west.hand, last_Discard_Tile)){
									std::cout << "R: Call mahjong from last discard.\n";
								}
							}
							cin >> option;

							if (option == 'D' || option == 'd'){
								west.hand = discard_Tile(west, draw_Tile(setup_Wall), setup_Wall);
								west.see_Hand();
								counter = 0;
								for (int i = 0; i < 144; i++){
									if (setup_Wall[i].value > 0){
										counter++;
									}
								}
								if (west.wind == "North" && tenho_Counter == 0){
									tenho_Counter = 1;
								}
								else if (west.wind == "South" && chiho_Counter == 0){
									chiho_Counter = 1;
								}
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
#pragma region
							else if (option == 'C' || option == 'c'){
								std::cout << "Which chow do you want to make?" << endl;
								west.show_Chows_Choices();
								cin >> chow_Choice;
								for (int i = 0; i < 4; i++){
									if (west.melds[i].name == "NONE"){
										west.melds[i] = west.chow_Choices[chow_Choice - 1];
										west.melds[i].hidden = false;
										west.chow_Choices[chow_Choice - 1] = Meld();
										for (int j = 0; j < 13; j++){
											if (west.hand[j].value > 0){
												if (west.hand[j].suit == west.melds[i].suit && (west.hand[j].value == west.melds[i].melded[0].value || west.hand[j].value == west.melds[i].melded[1].value || west.hand[j].value == west.melds[i].melded[2].value) &&
													(west.hand[j + 1].suit == west.melds[i].suit && (west.hand[j + 1].value == west.melds[i].melded[0].value || west.hand[j + 1].value == west.melds[i].melded[1].value || west.hand[j + 1].value == west.melds[i].melded[2].value))){
													west.hand[j] = NULL;
													west.hand[j + 1] = NULL;
												}
											}
										}
										west.hand = discard_Tile(west, last_Discard_Tile, setup_Wall, true);
										break;
									}
								}
								west.see_Hand();
								turn += 1;
							}
#pragma endregion Chow-ing

#pragma region
							else if (option == 'P' || option == 'p'){
								for (int i = 0; i < 4; i++){
									if (west.melds[i].name == "NONE"){
										for (int j = 0; j < 13; j++){
											if (west.hand[j].value == west.hand[j + 1].value && west.hand[j + 1].value == last_Discard_Tile.value && west.hand[j].suit == west.hand[j + 1].suit && west.hand[j + 1].suit == last_Discard_Tile.suit){
												west.melds[i] = Meld(west.hand[j], west.hand[j + 1], last_Discard_Tile);
												west.melds[i].hidden = false;
												west.hand[j] = NULL;
												west.hand[j + 1] = NULL;
												west.hand = discard_Tile(west, last_Discard_Tile, setup_Wall, true);
												break;
											}
										}
									}
								}
								west.see_Hand();
								turn += 1;
							}
#pragma endregion Pong-ing

#pragma region
							else if (option == 'K' || option == 'k'){
								for (int i = 0; i < 4; i++){
									if (west.melds[i].name == "NONE"){
										for (int j = 0; j < 13; j++){
											if (west.hand[j].value == west.hand[j + 1].value && west.hand[j + 1].value == west.hand[j + 2].value && west.hand[j + 2].value == last_Discard_Tile.value && west.hand[j].suit == west.hand[j + 1].suit && west.hand[j + 1].suit == west.hand[j + 2].suit && west.hand[j + 2].suit == last_Discard_Tile.suit){
												west.melds[i] = Meld(west.hand[j], west.hand[j + 1], west.hand[j + 2], last_Discard_Tile);
												west.melds[i].hidden = false;
												west.hand[j] = NULL;
												west.hand[j + 1] = NULL;
												west.hand[j + 2] = NULL;
												west.hand = discard_Tile(west, draw_Tile(setup_Wall, 1), setup_Wall);
												break;
											}
										}
									}
								}
								west.see_Hand();
								turn += 1;
							}
#pragma endregion Kong-ing

#pragma region
							else if (option == 'R' || option == 'r'){
								if (chiho_Counter == 0){
									if (west.wind != "West") west.chiho = true;
								}
								for (int i = 0; i < 9; i++){
									if (west.possible_Chows[i].name != "NONE"){
										for (int j = 0; j < 4; j++){
											if (west.melds[j].name == "NONE"){
												west.melds[j] = west.possible_Chows[i];
												for (int k = 0; k < 4; k++){
													// To Do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
													if (west.melds[j].suit == west.possible_Chows[i + 1].suit && west.melds[j].melded[k].value == west.possible_Chows[i + 1].melded[k].value){
														west.possible_Chows[i + 1] = Meld();
													}
												}
												break;
											}
										}
									}
									if (i < 4){
										if (west.possible_Pongs[i].name != "NONE"){
											for (int j = 0; j < 4; j++){
												if (west.melds[j].name == "NONE"){
													west.melds[j] = west.possible_Pongs[i];
													break;
												}
											}
										}
									}
								}
								for (int i = 0; i < 4; i++){
									if (west.melds[i].name != "NONE"){
										std::cout << "Meld " << i + 1 << ": " << west.melds[i].name << endl;
										for (int j = 0; j < 3; j++){
											std::cout << west.melds[i].melded[j].value << " " << west.melds[i].melded[j].suit << endl;
										}
									}
								}
								west.has_Won_Hand = true;
								std::cout << "Points won: " << calculate_Points(west.melds, west.hand, last_Discard_Tile, west) << "\n\n";
								west.points += calculate_Points(west.melds, west.hand, last_Discard_Tile, west, false);
								last_Discarder.points -= west.points;
								hand_Won = true;
							}
#pragma endregion Winning

						}
					}
				}
				else{
					if (turn == WEST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
						std::cout << west.wind << " takes a turn..." << endl;
						easy_AI(west, draw_Tile(setup_Wall), setup_Wall, (rand() % 14 + 1));
						std::cout << west.wind << " discards " << last_Discard_Tile.value << " " << last_Discard_Tile.suit << endl;

#pragma region 
						if (east.can_Pong(east.hand, last_Discard_Tile) || east.can_Kong(east.hand, last_Discard_Tile)){
							while (!pass){
								std::cout << "P: Call pong from last discard.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards.\nS: Pass.\n";
								if (east.can_Kong(east.hand, last_Discard_Tile)){
									std::cout << "K: Call kong from last discard.\n";
								}
								cin >> option;
								if (option == 'P' || option == 'p'){
									for (int i = 0; i < 4; i++){
										if (east.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == last_Discard_Tile.suit){
													east.melds[i] = Meld(east.hand[j], east.hand[j + 1], last_Discard_Tile);
													east.melds[i].hidden = false;
													east.hand[j] = NULL;
													east.hand[j + 1] = NULL;
													east.hand = discard_Tile(east, last_Discard_Tile, setup_Wall, true);
													break;
												}
											}
										}
									}
									east.see_Hand();
									pass = true;
									turn = SOUTH;
								}
								else if (option == 'K' || option == 'k'){
									for (int i = 0; i < 4; i++){
										if (east.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == east.hand[j + 2].value && east.hand[j + 2].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == east.hand[j + 2].suit && east.hand[j + 2].suit == last_Discard_Tile.suit){
													east.melds[i] = Meld(east.hand[j], east.hand[j + 1], east.hand[j + 2], last_Discard_Tile);
													east.melds[i].hidden = false;
													east.hand[j] = NULL;
													east.hand[j + 1] = NULL;
													east.hand[j + 2] = NULL;
													east.hand = discard_Tile(east, draw_Tile(setup_Wall, 1), setup_Wall);
													break;
												}
											}
										}
									}
									east.see_Hand();
									pass = true;
									turn = SOUTH;
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
								else if (option == 'S' || option == 's'){
									pass = true;
									turn = NORTH;
								}
							}
						}
#pragma endregion Pong Interrupt

						counter = 0;
						for (int i = 0; i < 144; i++){
							if (setup_Wall[i].value > 0){
								counter++;
							}
						}
					}
				}
#pragma endregion West Turn

#pragma region
				pass = false;
				if (!north.is_AI){
					if (turn == NORTH && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
						while (turn == NORTH && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							north.make_Pairs(north.hand);
							north.make_Chows(north.hand);
							north.make_Pongs(north.hand);
							std::cout << north.wind << " player, choose:" << endl;
							std::cout << "D: Draw a tile.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;

							if (north.can_Chow(north.hand, last_Discard_Tile)){
								std::cout << "C: Call chow from last discard.\n";
							}
							if (north.can_Pong(north.hand, last_Discard_Tile)){
								std::cout << "P: Call pong from last discard.\n";
							}
							if (north.can_Kong(north.hand, last_Discard_Tile)){
								std::cout << "K: Call kong from last discard.\n";
							}
							if (last_Discard_Tile.value > 0){
								if (north.can_Win(north.hand, last_Discard_Tile)){
									std::cout << "R: Call mahjong from last discard.\n";
								}
							}
							cin >> option;

							if (option == 'D' || option == 'd'){
								north.hand = discard_Tile(north, draw_Tile(setup_Wall), setup_Wall);
								north.see_Hand();
								counter = 0;
								for (int i = 0; i < 144; i++){
									if (setup_Wall[i].value > 0){
										counter++;
									}
								}
								if (north.wind == "North" && tenho_Counter == 0){
									tenho_Counter = 1;
								}
								else if (north.wind == "South" && chiho_Counter == 0){
									chiho_Counter = 1;
								}
								turn += 1;
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
#pragma region
							else if (option == 'C' || option == 'c'){
								std::cout << "Which chow do you want to make?" << endl;
								north.show_Chows_Choices();
								cin >> chow_Choice;
								for (int i = 0; i < 4; i++){
									if (north.melds[i].name == "NONE"){
										north.melds[i] = north.chow_Choices[chow_Choice - 1];
										north.melds[i].hidden = false;
										north.chow_Choices[chow_Choice - 1] = Meld();
										for (int j = 0; j < 13; j++){
											if (north.hand[j].value > 0){
												if (north.hand[j].suit == north.melds[i].suit && (north.hand[j].value == north.melds[i].melded[0].value || north.hand[j].value == north.melds[i].melded[1].value || north.hand[j].value == north.melds[i].melded[2].value) &&
													(north.hand[j + 1].suit == north.melds[i].suit && (north.hand[j + 1].value == north.melds[i].melded[0].value || north.hand[j + 1].value == north.melds[i].melded[1].value || north.hand[j + 1].value == north.melds[i].melded[2].value))){
													north.hand[j] = NULL;
													north.hand[j + 1] = NULL;
												}
											}
										}
										north.hand = discard_Tile(north, last_Discard_Tile, setup_Wall, true);
										break;
									}
								}
								north.see_Hand();
								turn += 1;
							}
#pragma endregion Chow-ing

#pragma region
							else if (option == 'P' || option == 'p'){
								for (int i = 0; i < 4; i++){
									if (north.melds[i].name == "NONE"){
										for (int j = 0; j < 13; j++){
											if (north.hand[j].value == north.hand[j + 1].value && north.hand[j + 1].value == last_Discard_Tile.value && north.hand[j].suit == north.hand[j + 1].suit && north.hand[j + 1].suit == last_Discard_Tile.suit){
												north.melds[i] = Meld(north.hand[j], north.hand[j + 1], last_Discard_Tile);
												north.melds[i].hidden = false;
												north.hand[j] = NULL;
												north.hand[j + 1] = NULL;
												north.hand = discard_Tile(north, last_Discard_Tile, setup_Wall, true);
												break;
											}
										}
									}
								}
								north.see_Hand();
								turn += 1;
							}
#pragma endregion Pong-ing

#pragma region
							else if (option == 'K' || option == 'k'){
								for (int i = 0; i < 4; i++){
									if (north.melds[i].name == "NONE"){
										for (int j = 0; j < 13; j++){
											if (north.hand[j].value == north.hand[j + 1].value && north.hand[j + 1].value == north.hand[j + 2].value && north.hand[j + 2].value == last_Discard_Tile.value && north.hand[j].suit == north.hand[j + 1].suit && north.hand[j + 1].suit == north.hand[j + 2].suit && north.hand[j + 2].suit == last_Discard_Tile.suit){
												north.melds[i] = Meld(north.hand[j], north.hand[j + 1], north.hand[j + 2], last_Discard_Tile);
												north.melds[i].hidden = false;
												north.hand[j] = NULL;
												north.hand[j + 1] = NULL;
												north.hand[j + 2] = NULL;
												north.hand = discard_Tile(north, draw_Tile(setup_Wall, 1), setup_Wall);
												break;
											}
										}
									}
								}
								north.see_Hand();
								turn += 1;
							}
#pragma endregion Kong-ing

#pragma region
							else if (option == 'R' || option == 'r'){
								if (chiho_Counter == 0){
									if (north.wind != "North") north.chiho = true;
								}
								for (int i = 0; i < 9; i++){
									if (north.possible_Chows[i].name != "NONE"){
										for (int j = 0; j < 4; j++){
											if (north.melds[j].name == "NONE"){
												north.melds[j] = north.possible_Chows[i];
												for (int k = 0; k < 4; k++){
													// To Do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
													if (north.melds[j].suit == north.possible_Chows[i + 1].suit && north.melds[j].melded[k].value == north.possible_Chows[i + 1].melded[k].value){
														north.possible_Chows[i + 1] = Meld();
													}
												}
												break;
											}
										}
									}
									if (i < 4){
										if (north.possible_Pongs[i].name != "NONE"){
											for (int j = 0; j < 4; j++){
												if (north.melds[j].name == "NONE"){
													north.melds[j] = north.possible_Pongs[i];
													break;
												}
											}
										}
									}
								}
								for (int i = 0; i < 4; i++){
									if (north.melds[i].name != "NONE"){
										std::cout << "Meld " << i + 1 << ": " << north.melds[i].name << endl;
										for (int j = 0; j < 3; j++){
											std::cout << north.melds[i].melded[j].value << " " << north.melds[i].melded[j].suit << endl;
										}
									}
								}
								north.has_Won_Hand = true;
								std::cout << "Points won: " << calculate_Points(north.melds, north.hand, last_Discard_Tile, north) << "\n\n";
								north.points += calculate_Points(north.melds, north.hand, last_Discard_Tile, north, false);
								last_Discarder.points -= north.points;
								hand_Won = true;
							}
#pragma endregion Winning

						}
					}
				}
				else{
					if (turn == NORTH && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
						std::cout << north.wind << " takes a turn..." << endl;
						easy_AI(north, draw_Tile(setup_Wall), setup_Wall, (rand() % 14 + 1));
						std::cout << north.wind << " discards " << last_Discard_Tile.value << " " << last_Discard_Tile.suit << endl;

						counter = 0;
						for (int i = 0; i < 144; i++){
							if (setup_Wall[i].value > 0){
								counter++;
							}
						}
					}
				}
#pragma endregion North Turn

				std::cout << "\n";

#pragma region
				if (last_Tile && !hand_Won){
					if (last_Discarder.wind != east.wind && east.can_Win(east.hand, last_Discard_Tile)){
						for (int i = 0; i < 9; i++){
							if (east.possible_Chows[i].name != "NONE"){
								for (int j = 0; j < 4; j++){
									if (east.melds[j].name == "NONE"){
										east.melds[j] = east.possible_Chows[i];
										for (int k = 0; k < 4; k++){
											// To Do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
											if (east.melds[j].suit == east.possible_Chows[i + 1].suit && east.melds[j].melded[k].value == east.possible_Chows[i + 1].melded[k].value){
												east.possible_Chows[i + 1] = Meld();
											}
										}
										break;
									}
								}
							}
							if (i < 4){
								if (east.possible_Pongs[i].name != "NONE"){
									for (int j = 0; j < 4; j++){
										if (east.melds[j].name == "NONE"){
											east.melds[j] = east.possible_Pongs[i];
											break;
										}
									}
								}
							}
						}
						for (int i = 0; i < 4; i++){
							if (east.melds[i].name != "NONE"){
								std::cout << "Meld " << i + 1 << ": " << east.melds[i].name << endl;
								for (int j = 0; j < 3; j++){
									std::cout << east.melds[i].melded[j].value << " " << east.melds[i].melded[j].suit << endl;
								}
							}
						}
						east.has_Won_Hand = true;
						std::cout << "Points won: " << calculate_Points(east.melds, east.hand, last_Discard_Tile, east) << "\n\n";
						east.points += calculate_Points(east.melds, east.hand, last_Discard_Tile, east, false, last_Tile);
						last_Discarder.points -= east.points;
						hand_Won = true;
					}
					else if (last_Discarder.wind != south.wind && south.can_Win(south.hand, last_Discard_Tile)){

					}
					else if (last_Discarder.wind != west.wind && west.can_Win(west.hand, last_Discard_Tile)){

					}
					else if (last_Discarder.wind != north.wind && north.can_Win(north.hand, last_Discard_Tile)){

					}
				}
#pragma endregion Last Discarded Tile

				if (counter == 0 || hand_Won){
					if (round_Counter == NORTH && turn_Counter == NORTH){
						// Game end.
						round_Counter = 6;
						game_Running = NONE;
					}
					else{
						if (turn_Counter == NORTH){
							move_Round += 1;
							turn_Counter = EAST;
						}
						else{
							if (!hand_Won){
								ren_Counter++;
							}
							else if ((east.has_Won_Hand && east.wind == "East") || (south.has_Won_Hand && south.wind == "East")
								|| (west.has_Won_Hand && west.wind == "East") || (north.has_Won_Hand && north.wind == "East")){
								ren_Counter++;
							}
							else{
								turn_Counter = turn_Counter + 1;
							}
						}
						round_Counter = 6;

					}
				}
			}
		}
	}
#pragma endregion Hong Kong Mahjong

#pragma region
		if (game_Running == CHINESE){
			while (game_Running == CHINESE){ // While the player is playing Hong Kong Style mahjong.
				north.was_North = true; // Identifies the north player as the original north player.
				hand_Won = false; // Sets the hand as not yet won.
				tenho_Counter = 0; // Identifies the first draw.
				round_Counter = EAST + move_Round; // Sets the current round.
				std::cout << "Round: " << round_Counter + 1 << endl; // Displays current round.
				std::cout << "Turn: " << turn_Counter + 1 << endl; // Displays current turn of the round.

				// Each time the hand ends, the seat winds of the players change.
#pragma region
				if (turn_Counter == EAST){
					east.wind = "East";
					east.player_Value = 1;
					south.wind = "South";
					south.player_Value = 2;
					west.wind = "West";
					west.player_Value = 3;
					north.wind = "North";
					north.player_Value = 4;
					turn = EAST;
				}
				else if (turn_Counter == SOUTH){
					east.wind = "North";
					east.player_Value = 4;
					south.wind = "East";
					south.player_Value--;
					west.wind = "South";
					west.player_Value--;
					north.wind = "West";
					north.player_Value--;
					turn = SOUTH;
				}
				else if (turn_Counter == WEST){
					east.wind = "West";
					east.player_Value--;
					south.wind = "North";
					south.player_Value = 4;
					west.wind = "East";
					west.player_Value--;
					north.wind = "South";
					north.player_Value--;
					turn = WEST;
				}
				else if (turn_Counter == NORTH){
					east.wind = "South";
					east.player_Value--;
					south.wind = "West";
					south.player_Value--;
					west.wind = "North";
					west.player_Value = 4;
					north.wind = "East";
					north.player_Value--;
					turn = NORTH;
				}
#pragma endregion Shift Winds

				// Initializes each players' melds and other little bits that
				// have to be set to a default value at the beginning of each hand.
#pragma region
				east.tenho = false;
				east.chiho = false;
				south.tenho = false;
				south.chiho = false;
				west.tenho = false;
				west.chiho = false;
				north.tenho = false;
				north.chiho = false;
				for (int i = 0; i < 144; i++){
					if (discards[i].value > 0){
						discards[i] = NULL;
					}
				}
				for (int k = 0; k < 4; k++){
					if (east.melds[k].melded != NULL){
						if (east.melds[k].melded[0].value > 0){
							east.melds[k].name = "NONE";
							east.melds[k].suit = "NONE";
							east.melds[k].melded[0] = NULL;
							east.melds[k].melded[1] = NULL;
							east.melds[k].melded[2] = NULL;
							if (east.melds[k].melded[3].value > 0){
								east.melds[k].melded[3] = NULL;
							}
						}
					}
					if (south.melds[k].melded != NULL){
						if (south.melds[k].melded[0].value > 0){
							south.melds[k].name = "NONE";
							south.melds[k].suit = "NONE";
							south.melds[k].melded[0] = NULL;
							south.melds[k].melded[1] = NULL;
							south.melds[k].melded[2] = NULL;
							if (south.melds[k].melded[3].value > 0){
								south.melds[k].melded[3] = NULL;
							}
						}
					}
					if (west.melds[k].melded != NULL){
						if (west.melds[k].melded[0].value > 0){
							west.melds[k].name = "NONE";
							west.melds[k].suit = "NONE";
							west.melds[k].melded[0] = NULL;
							west.melds[k].melded[1] = NULL;
							west.melds[k].melded[2] = NULL;
							if (west.melds[k].melded[3].value > 0){
								west.melds[k].melded[3] = NULL;
							}
						}
					}
					if (north.melds[k].melded != NULL){
						if (north.melds[k].melded[0].value > 0){
							north.melds[k].name = "NONE";
							north.melds[k].suit = "NONE";
							north.melds[k].melded[0] = NULL;
							north.melds[k].melded[1] = NULL;
							north.melds[k].melded[2] = NULL;
							if (north.melds[k].melded[3].value > 0){
								north.melds[k].melded[3] = NULL;
							}
						}
					}
				}
				for (int j = 0; j < 8; j++){
					if (east.claimed[j].value > 0){
						east.claimed[j] = NULL;
					}
					if (south.claimed[j].value > 0){
						east.claimed[j] = NULL;
					}
					if (west.claimed[j].value > 0){
						west.claimed[j] = NULL;
					}
					if (north.claimed[j].value > 0){
						north.claimed[j] = NULL;
					}
				}

				east.has_Won_Hand = false;
				south.has_Won_Hand = false;
				west.has_Won_Hand = false;
				north.has_Won_Hand = false;
#pragma endregion Reset Game

				roll = dice_A.HK_roll();
				std::cout << "Rolled " << roll << "\n\n" << endl;

				// Create the wall that is used to draw from.
				setup_Wall = wall_Split(wall_Setup(), roll);

				// Set up the initial hand for the current hand for all the players.
#pragma region
				hand_Create(setup_Wall); // Create all the hands at the same time.

				// Then sort the hands and look to see if there are any flowers or seasons.
				// Flowers and seasons are removed from the hand, and replaced from the end
				// of the wall.
				// If the position of the flower or season doesn't match a player's seat,
				// that player's hand is worth 0 points. Each flower or season of that
				// player's seat is worth 1 point (up to 2 points).
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
							if (south.claimed[j].value != south.player_Value){
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
							if (west.claimed[j].value != west.player_Value){
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
							if (north.claimed[j].value != north.player_Value){
								north.hand_Points -= 1;
							}
						}
					}
				}
				north.hand = north.sort_Hand(north.hand);
#pragma endregion Initial Hands

				// TEST CODE!!! REMOVE AFTER DONE!!!
				// END TEST CODE!!

				while (round_Counter < 6){

					// All the parts of each player's turn.
#pragma region
					pass = false;
					if (!east.is_AI){
						if (turn == EAST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							while (turn == EAST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
								east.make_Pairs(east.hand);
								east.make_Chows(east.hand);
								east.make_Pongs(east.hand);
								std::cout << east.wind << " player, choose:" << endl;
								std::cout << "D: Draw a tile.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;

								if (east.can_Chow(east.hand, last_Discard_Tile)){
									std::cout << "C: Call chow from last discard.\n";
								}
								if (east.can_Pong(east.hand, last_Discard_Tile)){
									std::cout << "P: Call pong from last discard.\n";
								}
								if (east.can_Kong(east.hand, last_Discard_Tile)){
									std::cout << "K: Call kong from last discard.\n";
								}
								if (last_Discard_Tile.value > 0){
									if (east.can_Win(east.hand, last_Discard_Tile)){
										std::cout << "R: Call mahjong from last discard.\n";
									}
								}
								cin >> option;

								if (option == 'D' || option == 'd'){
									east.hand = discard_Tile(east, draw_Tile(setup_Wall), setup_Wall);
									east.see_Hand();
									counter = 0;
									for (int i = 0; i < 144; i++){
										if (setup_Wall[i].value > 0){
											counter++;
										}
									}
									if (east.wind == "North" && tenho_Counter == 0){
										tenho_Counter = 1;
									}
									else if (east.wind == "South" && chiho_Counter == 0){
										chiho_Counter = 1;
									}
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
#pragma region
								else if (option == 'C' || option == 'c'){
									std::cout << "Which chow do you want to make?" << endl;
									east.show_Chows_Choices();
									cin >> chow_Choice;
									for (int i = 0; i < 4; i++){
										if (east.melds[i].name == "NONE"){
											east.melds[i] = east.chow_Choices[chow_Choice - 1];
											east.melds[i].hidden = false;
											east.chow_Choices[chow_Choice - 1] = Meld();
											for (int j = 0; j < 13; j++){
												if (east.hand[j].value > 0){
													if (east.hand[j].suit == east.melds[i].suit && (east.hand[j].value == east.melds[i].melded[0].value || east.hand[j].value == east.melds[i].melded[1].value || east.hand[j].value == east.melds[i].melded[2].value) &&
														(east.hand[j + 1].suit == east.melds[i].suit && (east.hand[j + 1].value == east.melds[i].melded[0].value || east.hand[j + 1].value == east.melds[i].melded[1].value || east.hand[j + 1].value == east.melds[i].melded[2].value))){
														east.hand[j] = NULL;
														east.hand[j + 1] = NULL;
													}
												}
											}
											east.hand = discard_Tile(east, last_Discard_Tile, setup_Wall, true);
											break;
										}
									}
									east.see_Hand();
									turn += 1;
								}
#pragma endregion Chow-ing

#pragma region
								else if (option == 'P' || option == 'p'){
									for (int i = 0; i < 4; i++){
										if (east.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == last_Discard_Tile.suit){
													east.melds[i] = Meld(east.hand[j], east.hand[j + 1], last_Discard_Tile);
													east.melds[i].hidden = false;
													east.hand[j] = NULL;
													east.hand[j + 1] = NULL;
													east.hand = discard_Tile(east, last_Discard_Tile, setup_Wall, true);
													break;
												}
											}
										}
									}
									east.see_Hand();
									turn += 1;
								}
#pragma endregion Pong-ing

#pragma region
								else if (option == 'K' || option == 'k'){
									for (int i = 0; i < 4; i++){
										if (east.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == east.hand[j + 2].value && east.hand[j + 2].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == east.hand[j + 2].suit && east.hand[j + 2].suit == last_Discard_Tile.suit){
													east.melds[i] = Meld(east.hand[j], east.hand[j + 1], east.hand[j + 2], last_Discard_Tile);
													east.melds[i].hidden = false;
													east.hand[j] = NULL;
													east.hand[j + 1] = NULL;
													east.hand[j + 2] = NULL;
													east.hand = discard_Tile(east, draw_Tile(setup_Wall, 1), setup_Wall);
													break;
												}
											}
										}
									}
									east.see_Hand();
									turn += 1;
								}
#pragma endregion Kong-ing

#pragma region
								else if (option == 'R' || option == 'r'){
									if (chiho_Counter == 0){
										if (east.wind != "East") east.chiho = true;
									}
									for (int i = 0; i < 9; i++){
										if (east.possible_Chows[i].name != "NONE"){
											for (int j = 0; j < 4; j++){
												if (east.melds[j].name == "NONE"){
													east.melds[j] = east.possible_Chows[i];
													for (int k = 0; k < 4; k++){
														// To Do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
														if (east.melds[j].suit == east.possible_Chows[i + 1].suit && east.melds[j].melded[k].value == east.possible_Chows[i + 1].melded[k].value){
															east.possible_Chows[i + 1] = Meld();
														}
													}
													break;
												}
											}
										}
										if (i < 4){
											if (east.possible_Pongs[i].name != "NONE"){
												for (int j = 0; j < 4; j++){
													if (east.melds[j].name == "NONE"){
														east.melds[j] = east.possible_Pongs[i];
														break;
													}
												}
											}
										}
									}
									for (int i = 0; i < 4; i++){
										if (east.melds[i].name != "NONE"){
											std::cout << "Meld " << i + 1 << ": " << east.melds[i].name << endl;
											for (int j = 0; j < 3; j++){
												std::cout << east.melds[i].melded[j].value << " " << east.melds[i].melded[j].suit << endl;
											}
										}
									}
									east.has_Won_Hand = true;
									std::cout << "Points won: " << calculate_Points(east.melds, east.hand, last_Discard_Tile, east) << "\n\n";
									east.points += calculate_Points(east.melds, east.hand, last_Discard_Tile, east, false);
									last_Discarder.points -= east.points;
									hand_Won = true;
								}
#pragma endregion Winning

							}
						}
					}
#pragma region
					else {
						if (turn == EAST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							std::cout << east.wind << " takes a turn..." << endl;
							easy_AI(east, draw_Tile(setup_Wall), setup_Wall, (rand() % 14 + 1));
							counter = 0;
							for (int i = 0; i < 144; i++){
								if (setup_Wall[i].value > 0){
									counter++;
								}
							}
						}
					}
#pragma endregion AI
#pragma endregion East Turn

#pragma region
					pass = false;
					if (!south.is_AI){
						if (turn == SOUTH && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							while (turn == SOUTH && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
								south.make_Pairs(south.hand);
								south.make_Chows(south.hand);
								south.make_Pongs(south.hand);
								std::cout << south.wind << " player, choose:" << endl;
								std::cout << "D: Draw a tile.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;

								if (south.can_Chow(south.hand, last_Discard_Tile)){
									std::cout << "C: Call chow from last discard.\n";
								}
								if (south.can_Pong(south.hand, last_Discard_Tile)){
									std::cout << "P: Call pong from last discard.\n";
								}
								if (south.can_Kong(south.hand, last_Discard_Tile)){
									std::cout << "K: Call kong from last discard.\n";
								}
								if (last_Discard_Tile.value > 0){
									if (south.can_Win(south.hand, last_Discard_Tile)){
										std::cout << "R: Call mahjong from last discard.\n";
									}
								}
								cin >> option;

								if (option == 'D' || option == 'd'){
									south.hand = discard_Tile(south, draw_Tile(setup_Wall), setup_Wall);
									south.see_Hand();
									counter = 0;
									for (int i = 0; i < 144; i++){
										if (setup_Wall[i].value > 0){
											counter++;
										}
									}
									if (south.wind == "North" && tenho_Counter == 0){
										tenho_Counter = 1;
									}
									else if (south.wind == "South" && chiho_Counter == 0){
										chiho_Counter = 1;
									}
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
#pragma region
								else if (option == 'C' || option == 'c'){
									std::cout << "Which chow do you want to make?" << endl;
									south.show_Chows_Choices();
									cin >> chow_Choice;
									for (int i = 0; i < 4; i++){
										if (south.melds[i].name == "NONE"){
											south.melds[i] = south.chow_Choices[chow_Choice - 1];
											south.melds[i].hidden = false;
											south.chow_Choices[chow_Choice - 1] = Meld();
											for (int j = 0; j < 13; j++){
												if (south.hand[j].value > 0){
													if (south.hand[j].suit == south.melds[i].suit && (south.hand[j].value == south.melds[i].melded[0].value || south.hand[j].value == south.melds[i].melded[1].value || south.hand[j].value == south.melds[i].melded[2].value) &&
														(south.hand[j + 1].suit == south.melds[i].suit && (south.hand[j + 1].value == south.melds[i].melded[0].value || south.hand[j + 1].value == south.melds[i].melded[1].value || south.hand[j + 1].value == south.melds[i].melded[2].value))){
														south.hand[j] = NULL;
														south.hand[j + 1] = NULL;
													}
												}
											}
											south.hand = discard_Tile(south, last_Discard_Tile, setup_Wall, true);
											break;
										}
									}
									south.see_Hand();
									turn += 1;
								}
#pragma endregion Chow-ing

#pragma region
								else if (option == 'P' || option == 'p'){
									for (int i = 0; i < 4; i++){
										if (south.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (south.hand[j].value == south.hand[j + 1].value && south.hand[j + 1].value == last_Discard_Tile.value && south.hand[j].suit == south.hand[j + 1].suit && south.hand[j + 1].suit == last_Discard_Tile.suit){
													south.melds[i] = Meld(south.hand[j], south.hand[j + 1], last_Discard_Tile);
													south.melds[i].hidden = false;
													south.hand[j] = NULL;
													south.hand[j + 1] = NULL;
													south.hand = discard_Tile(south, last_Discard_Tile, setup_Wall, true);
													break;
												}
											}
										}
									}
									south.see_Hand();
									turn += 1;
								}
#pragma endregion Pong-ing

#pragma region
								else if (option == 'K' || option == 'k'){
									for (int i = 0; i < 4; i++){
										if (south.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (south.hand[j].value == south.hand[j + 1].value && south.hand[j + 1].value == south.hand[j + 2].value && south.hand[j + 2].value == last_Discard_Tile.value && south.hand[j].suit == south.hand[j + 1].suit && south.hand[j + 1].suit == south.hand[j + 2].suit && south.hand[j + 2].suit == last_Discard_Tile.suit){
													south.melds[i] = Meld(south.hand[j], south.hand[j + 1], south.hand[j + 2], last_Discard_Tile);
													south.melds[i].hidden = false;
													south.hand[j] = NULL;
													south.hand[j + 1] = NULL;
													south.hand[j + 2] = NULL;
													south.hand = discard_Tile(south, draw_Tile(setup_Wall, 1), setup_Wall);
													break;
												}
											}
										}
									}
									south.see_Hand();
									turn += 1;
								}
#pragma endregion Kong-ing

#pragma region
								else if (option == 'R' || option == 'r'){
									if (chiho_Counter == 0){
										if (south.wind != "South") south.chiho = true;
									}
									for (int i = 0; i < 9; i++){
										if (south.possible_Chows[i].name != "NONE"){
											for (int j = 0; j < 4; j++){
												if (south.melds[j].name == "NONE"){
													south.melds[j] = south.possible_Chows[i];
													for (int k = 0; k < 4; k++){
														// To Do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
														if (south.melds[j].suit == south.possible_Chows[i + 1].suit && south.melds[j].melded[k].value == south.possible_Chows[i + 1].melded[k].value){
															south.possible_Chows[i + 1] = Meld();
														}
													}
													break;
												}
											}
										}
										if (i < 4){
											if (south.possible_Pongs[i].name != "NONE"){
												for (int j = 0; j < 4; j++){
													if (south.melds[j].name == "NONE"){
														south.melds[j] = south.possible_Pongs[i];
														break;
													}
												}
											}
										}
									}
									for (int i = 0; i < 4; i++){
										if (south.melds[i].name != "NONE"){
											std::cout << "Meld " << i + 1 << ": " << south.melds[i].name << endl;
											for (int j = 0; j < 3; j++){
												std::cout << south.melds[i].melded[j].value << " " << south.melds[i].melded[j].suit << endl;
											}
										}
									}
									south.has_Won_Hand = true;
									std::cout << "Points won: " << calculate_Points(south.melds, south.hand, last_Discard_Tile, south) << "\n\n";
									south.points += calculate_Points(south.melds, south.hand, last_Discard_Tile, south, false);
									last_Discarder.points -= south.points;
									hand_Won = true;
								}
#pragma endregion Winning

							}
						}
					}
					else {
						if (turn == SOUTH & !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							std::cout << south.wind << " takes a turn..." << endl;
							easy_AI(south, draw_Tile(setup_Wall), setup_Wall, (rand() % 14 + 1));
							std::cout << south.wind << " discards " << last_Discard_Tile.value << " " << last_Discard_Tile.suit << endl;

#pragma region 
							if (east.can_Pong(east.hand, last_Discard_Tile) || east.can_Kong(east.hand, last_Discard_Tile)){
								while (!pass){
									std::cout << "P: Call pong from last discard.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards.\nS: Pass.\n";
									if (east.can_Kong(east.hand, last_Discard_Tile)){
										std::cout << "K: Call kong from last discard.\n";
									}
									cin >> option;
									if (option == 'P' || option == 'p'){
										for (int i = 0; i < 4; i++){
											if (east.melds[i].name == "NONE"){
												for (int j = 0; j < 13; j++){
													if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == last_Discard_Tile.suit){
														east.melds[i] = Meld(east.hand[j], east.hand[j + 1], last_Discard_Tile);
														east.melds[i].hidden = false;
														east.hand[j] = NULL;
														east.hand[j + 1] = NULL;
														east.hand = discard_Tile(east, last_Discard_Tile, setup_Wall, true);
														break;
													}
												}
											}
										}
										east.see_Hand();
										pass = true;
										turn = SOUTH;
									}
									else if (option == 'K' || option == 'k'){
										for (int i = 0; i < 4; i++){
											if (east.melds[i].name == "NONE"){
												for (int j = 0; j < 13; j++){
													if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == east.hand[j + 2].value && east.hand[j + 2].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == east.hand[j + 2].suit && east.hand[j + 2].suit == last_Discard_Tile.suit){
														east.melds[i] = Meld(east.hand[j], east.hand[j + 1], east.hand[j + 2], last_Discard_Tile);
														east.melds[i].hidden = false;
														east.hand[j] = NULL;
														east.hand[j + 1] = NULL;
														east.hand[j + 2] = NULL;
														east.hand = discard_Tile(east, draw_Tile(setup_Wall, 1), setup_Wall);
														break;
													}
												}
											}
										}
										east.see_Hand();
										pass = true;
										turn = SOUTH;
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
									else if (option == 'S' || option == 's'){
										pass = true;
										turn = WEST;
									}
								}
							}
#pragma endregion Pong Interrupt

							counter = 0;
							for (int i = 0; i < 144; i++){
								if (setup_Wall[i].value > 0){
									counter++;
								}
							}
						}
					}
#pragma endregion South Turn

#pragma region
					pass = false;
					if (!west.is_AI){
						if (turn == WEST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							while (turn == WEST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
								west.make_Pairs(west.hand);
								west.make_Chows(west.hand);
								west.make_Pongs(west.hand);
								std::cout << west.wind << " player, choose:" << endl;
								std::cout << "D: Draw a tile.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;

								if (west.can_Chow(west.hand, last_Discard_Tile)){
									std::cout << "C: Call chow from last discard.\n";
								}
								if (west.can_Pong(west.hand, last_Discard_Tile)){
									std::cout << "P: Call pong from last discard.\n";
								}
								if (west.can_Kong(west.hand, last_Discard_Tile)){
									std::cout << "K: Call kong from last discard.\n";
								}
								if (last_Discard_Tile.value > 0){
									if (west.can_Win(west.hand, last_Discard_Tile)){
										std::cout << "R: Call mahjong from last discard.\n";
									}
								}
								cin >> option;

								if (option == 'D' || option == 'd'){
									west.hand = discard_Tile(west, draw_Tile(setup_Wall), setup_Wall);
									west.see_Hand();
									counter = 0;
									for (int i = 0; i < 144; i++){
										if (setup_Wall[i].value > 0){
											counter++;
										}
									}
									if (west.wind == "North" && tenho_Counter == 0){
										tenho_Counter = 1;
									}
									else if (west.wind == "South" && chiho_Counter == 0){
										chiho_Counter = 1;
									}
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
#pragma region
								else if (option == 'C' || option == 'c'){
									std::cout << "Which chow do you want to make?" << endl;
									west.show_Chows_Choices();
									cin >> chow_Choice;
									for (int i = 0; i < 4; i++){
										if (west.melds[i].name == "NONE"){
											west.melds[i] = west.chow_Choices[chow_Choice - 1];
											west.melds[i].hidden = false;
											west.chow_Choices[chow_Choice - 1] = Meld();
											for (int j = 0; j < 13; j++){
												if (west.hand[j].value > 0){
													if (west.hand[j].suit == west.melds[i].suit && (west.hand[j].value == west.melds[i].melded[0].value || west.hand[j].value == west.melds[i].melded[1].value || west.hand[j].value == west.melds[i].melded[2].value) &&
														(west.hand[j + 1].suit == west.melds[i].suit && (west.hand[j + 1].value == west.melds[i].melded[0].value || west.hand[j + 1].value == west.melds[i].melded[1].value || west.hand[j + 1].value == west.melds[i].melded[2].value))){
														west.hand[j] = NULL;
														west.hand[j + 1] = NULL;
													}
												}
											}
											west.hand = discard_Tile(west, last_Discard_Tile, setup_Wall, true);
											break;
										}
									}
									west.see_Hand();
									turn += 1;
								}
#pragma endregion Chow-ing

#pragma region
								else if (option == 'P' || option == 'p'){
									for (int i = 0; i < 4; i++){
										if (west.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (west.hand[j].value == west.hand[j + 1].value && west.hand[j + 1].value == last_Discard_Tile.value && west.hand[j].suit == west.hand[j + 1].suit && west.hand[j + 1].suit == last_Discard_Tile.suit){
													west.melds[i] = Meld(west.hand[j], west.hand[j + 1], last_Discard_Tile);
													west.melds[i].hidden = false;
													west.hand[j] = NULL;
													west.hand[j + 1] = NULL;
													west.hand = discard_Tile(west, last_Discard_Tile, setup_Wall, true);
													break;
												}
											}
										}
									}
									west.see_Hand();
									turn += 1;
								}
#pragma endregion Pong-ing

#pragma region
								else if (option == 'K' || option == 'k'){
									for (int i = 0; i < 4; i++){
										if (west.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (west.hand[j].value == west.hand[j + 1].value && west.hand[j + 1].value == west.hand[j + 2].value && west.hand[j + 2].value == last_Discard_Tile.value && west.hand[j].suit == west.hand[j + 1].suit && west.hand[j + 1].suit == west.hand[j + 2].suit && west.hand[j + 2].suit == last_Discard_Tile.suit){
													west.melds[i] = Meld(west.hand[j], west.hand[j + 1], west.hand[j + 2], last_Discard_Tile);
													west.melds[i].hidden = false;
													west.hand[j] = NULL;
													west.hand[j + 1] = NULL;
													west.hand[j + 2] = NULL;
													west.hand = discard_Tile(west, draw_Tile(setup_Wall, 1), setup_Wall);
													break;
												}
											}
										}
									}
									west.see_Hand();
									turn += 1;
								}
#pragma endregion Kong-ing

#pragma region
								else if (option == 'R' || option == 'r'){
									if (chiho_Counter == 0){
										if (west.wind != "West") west.chiho = true;
									}
									for (int i = 0; i < 9; i++){
										if (west.possible_Chows[i].name != "NONE"){
											for (int j = 0; j < 4; j++){
												if (west.melds[j].name == "NONE"){
													west.melds[j] = west.possible_Chows[i];
													for (int k = 0; k < 4; k++){
														// To Do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
														if (west.melds[j].suit == west.possible_Chows[i + 1].suit && west.melds[j].melded[k].value == west.possible_Chows[i + 1].melded[k].value){
															west.possible_Chows[i + 1] = Meld();
														}
													}
													break;
												}
											}
										}
										if (i < 4){
											if (west.possible_Pongs[i].name != "NONE"){
												for (int j = 0; j < 4; j++){
													if (west.melds[j].name == "NONE"){
														west.melds[j] = west.possible_Pongs[i];
														break;
													}
												}
											}
										}
									}
									for (int i = 0; i < 4; i++){
										if (west.melds[i].name != "NONE"){
											std::cout << "Meld " << i + 1 << ": " << west.melds[i].name << endl;
											for (int j = 0; j < 3; j++){
												std::cout << west.melds[i].melded[j].value << " " << west.melds[i].melded[j].suit << endl;
											}
										}
									}
									west.has_Won_Hand = true;
									std::cout << "Points won: " << calculate_Points(west.melds, west.hand, last_Discard_Tile, west) << "\n\n";
									west.points += calculate_Points(west.melds, west.hand, last_Discard_Tile, west, false);
									last_Discarder.points -= west.points;
									hand_Won = true;
								}
#pragma endregion Winning

							}
						}
					}
					else{
						if (turn == WEST && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							std::cout << west.wind << " takes a turn..." << endl;
							easy_AI(west, draw_Tile(setup_Wall), setup_Wall, (rand() % 14 + 1));
							std::cout << west.wind << " discards " << last_Discard_Tile.value << " " << last_Discard_Tile.suit << endl;

#pragma region 
							if (east.can_Pong(east.hand, last_Discard_Tile) || east.can_Kong(east.hand, last_Discard_Tile)){
								while (!pass){
									std::cout << "P: Call pong from last discard.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards.\nS: Pass.\n";
									if (east.can_Kong(east.hand, last_Discard_Tile)){
										std::cout << "K: Call kong from last discard.\n";
									}
									cin >> option;
									if (option == 'P' || option == 'p'){
										for (int i = 0; i < 4; i++){
											if (east.melds[i].name == "NONE"){
												for (int j = 0; j < 13; j++){
													if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == last_Discard_Tile.suit){
														east.melds[i] = Meld(east.hand[j], east.hand[j + 1], last_Discard_Tile);
														east.melds[i].hidden = false;
														east.hand[j] = NULL;
														east.hand[j + 1] = NULL;
														east.hand = discard_Tile(east, last_Discard_Tile, setup_Wall, true);
														break;
													}
												}
											}
										}
										east.see_Hand();
										pass = true;
										turn = SOUTH;
									}
									else if (option == 'K' || option == 'k'){
										for (int i = 0; i < 4; i++){
											if (east.melds[i].name == "NONE"){
												for (int j = 0; j < 13; j++){
													if (east.hand[j].value == east.hand[j + 1].value && east.hand[j + 1].value == east.hand[j + 2].value && east.hand[j + 2].value == last_Discard_Tile.value && east.hand[j].suit == east.hand[j + 1].suit && east.hand[j + 1].suit == east.hand[j + 2].suit && east.hand[j + 2].suit == last_Discard_Tile.suit){
														east.melds[i] = Meld(east.hand[j], east.hand[j + 1], east.hand[j + 2], last_Discard_Tile);
														east.melds[i].hidden = false;
														east.hand[j] = NULL;
														east.hand[j + 1] = NULL;
														east.hand[j + 2] = NULL;
														east.hand = discard_Tile(east, draw_Tile(setup_Wall, 1), setup_Wall);
														break;
													}
												}
											}
										}
										east.see_Hand();
										pass = true;
										turn = SOUTH;
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
									else if (option == 'S' || option == 's'){
										pass = true;
										turn = NORTH;
									}
								}
							}
#pragma endregion Pong Interrupt

							counter = 0;
							for (int i = 0; i < 144; i++){
								if (setup_Wall[i].value > 0){
									counter++;
								}
							}
						}
					}
#pragma endregion West Turn

#pragma region
					pass = false;
					if (!north.is_AI){
						if (turn == NORTH && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							while (turn == NORTH && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
								north.make_Pairs(north.hand);
								north.make_Chows(north.hand);
								north.make_Pongs(north.hand);
								std::cout << north.wind << " player, choose:" << endl;
								std::cout << "D: Draw a tile.\nH: View your hand.\nM: View your claimed tiles.\nV: View the discards." << endl;

								if (north.can_Chow(north.hand, last_Discard_Tile)){
									std::cout << "C: Call chow from last discard.\n";
								}
								if (north.can_Pong(north.hand, last_Discard_Tile)){
									std::cout << "P: Call pong from last discard.\n";
								}
								if (north.can_Kong(north.hand, last_Discard_Tile)){
									std::cout << "K: Call kong from last discard.\n";
								}
								if (last_Discard_Tile.value > 0){
									if (north.can_Win(north.hand, last_Discard_Tile)){
										std::cout << "R: Call mahjong from last discard.\n";
									}
								}
								cin >> option;

								if (option == 'D' || option == 'd'){
									north.hand = discard_Tile(north, draw_Tile(setup_Wall), setup_Wall);
									north.see_Hand();
									counter = 0;
									for (int i = 0; i < 144; i++){
										if (setup_Wall[i].value > 0){
											counter++;
										}
									}
									if (north.wind == "North" && tenho_Counter == 0){
										tenho_Counter = 1;
									}
									else if (north.wind == "South" && chiho_Counter == 0){
										chiho_Counter = 1;
									}
									turn += 1;
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
#pragma region
								else if (option == 'C' || option == 'c'){
									std::cout << "Which chow do you want to make?" << endl;
									north.show_Chows_Choices();
									cin >> chow_Choice;
									for (int i = 0; i < 4; i++){
										if (north.melds[i].name == "NONE"){
											north.melds[i] = north.chow_Choices[chow_Choice - 1];
											north.melds[i].hidden = false;
											north.chow_Choices[chow_Choice - 1] = Meld();
											for (int j = 0; j < 13; j++){
												if (north.hand[j].value > 0){
													if (north.hand[j].suit == north.melds[i].suit && (north.hand[j].value == north.melds[i].melded[0].value || north.hand[j].value == north.melds[i].melded[1].value || north.hand[j].value == north.melds[i].melded[2].value) &&
														(north.hand[j + 1].suit == north.melds[i].suit && (north.hand[j + 1].value == north.melds[i].melded[0].value || north.hand[j + 1].value == north.melds[i].melded[1].value || north.hand[j + 1].value == north.melds[i].melded[2].value))){
														north.hand[j] = NULL;
														north.hand[j + 1] = NULL;
													}
												}
											}
											north.hand = discard_Tile(north, last_Discard_Tile, setup_Wall, true);
											break;
										}
									}
									north.see_Hand();
									turn += 1;
								}
#pragma endregion Chow-ing

#pragma region
								else if (option == 'P' || option == 'p'){
									for (int i = 0; i < 4; i++){
										if (north.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (north.hand[j].value == north.hand[j + 1].value && north.hand[j + 1].value == last_Discard_Tile.value && north.hand[j].suit == north.hand[j + 1].suit && north.hand[j + 1].suit == last_Discard_Tile.suit){
													north.melds[i] = Meld(north.hand[j], north.hand[j + 1], last_Discard_Tile);
													north.melds[i].hidden = false;
													north.hand[j] = NULL;
													north.hand[j + 1] = NULL;
													north.hand = discard_Tile(north, last_Discard_Tile, setup_Wall, true);
													break;
												}
											}
										}
									}
									north.see_Hand();
									turn += 1;
								}
#pragma endregion Pong-ing

#pragma region
								else if (option == 'K' || option == 'k'){
									for (int i = 0; i < 4; i++){
										if (north.melds[i].name == "NONE"){
											for (int j = 0; j < 13; j++){
												if (north.hand[j].value == north.hand[j + 1].value && north.hand[j + 1].value == north.hand[j + 2].value && north.hand[j + 2].value == last_Discard_Tile.value && north.hand[j].suit == north.hand[j + 1].suit && north.hand[j + 1].suit == north.hand[j + 2].suit && north.hand[j + 2].suit == last_Discard_Tile.suit){
													north.melds[i] = Meld(north.hand[j], north.hand[j + 1], north.hand[j + 2], last_Discard_Tile);
													north.melds[i].hidden = false;
													north.hand[j] = NULL;
													north.hand[j + 1] = NULL;
													north.hand[j + 2] = NULL;
													north.hand = discard_Tile(north, draw_Tile(setup_Wall, 1), setup_Wall);
													break;
												}
											}
										}
									}
									north.see_Hand();
									turn += 1;
								}
#pragma endregion Kong-ing

#pragma region
								else if (option == 'R' || option == 'r'){
									if (chiho_Counter == 0){
										if (north.wind != "North") north.chiho = true;
									}
									for (int i = 0; i < 9; i++){
										if (north.possible_Chows[i].name != "NONE"){
											for (int j = 0; j < 4; j++){
												if (north.melds[j].name == "NONE"){
													north.melds[j] = north.possible_Chows[i];
													for (int k = 0; k < 4; k++){
														// To Do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
														if (north.melds[j].suit == north.possible_Chows[i + 1].suit && north.melds[j].melded[k].value == north.possible_Chows[i + 1].melded[k].value){
															north.possible_Chows[i + 1] = Meld();
														}
													}
													break;
												}
											}
										}
										if (i < 4){
											if (north.possible_Pongs[i].name != "NONE"){
												for (int j = 0; j < 4; j++){
													if (north.melds[j].name == "NONE"){
														north.melds[j] = north.possible_Pongs[i];
														break;
													}
												}
											}
										}
									}
									for (int i = 0; i < 4; i++){
										if (north.melds[i].name != "NONE"){
											std::cout << "Meld " << i + 1 << ": " << north.melds[i].name << endl;
											for (int j = 0; j < 3; j++){
												std::cout << north.melds[i].melded[j].value << " " << north.melds[i].melded[j].suit << endl;
											}
										}
									}
									north.has_Won_Hand = true;
									std::cout << "Points won: " << calculate_Points(north.melds, north.hand, last_Discard_Tile, north) << "\n\n";
									north.points += calculate_Points(north.melds, north.hand, last_Discard_Tile, north, false);
									last_Discarder.points -= north.points;
									hand_Won = true;
								}
#pragma endregion Winning

							}
						}
					}
					else{
						if (turn == NORTH && !hand_Won && see_Next_Tile(setup_Wall).value > 0){
							std::cout << north.wind << " takes a turn..." << endl;
							easy_AI(north, draw_Tile(setup_Wall), setup_Wall, (rand() % 14 + 1));
							std::cout << north.wind << " discards " << last_Discard_Tile.value << " " << last_Discard_Tile.suit << endl;

							counter = 0;
							for (int i = 0; i < 144; i++){
								if (setup_Wall[i].value > 0){
									counter++;
								}
							}
						}
					}
#pragma endregion North Turn

					std::cout << "\n";

#pragma region
					if (last_Tile && !hand_Won){
						if (last_Discarder.wind != east.wind && east.can_Win(east.hand, last_Discard_Tile)){
							for (int i = 0; i < 9; i++){
								if (east.possible_Chows[i].name != "NONE"){
									for (int j = 0; j < 4; j++){
										if (east.melds[j].name == "NONE"){
											east.melds[j] = east.possible_Chows[i];
											for (int k = 0; k < 4; k++){
												// To Do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
												if (east.melds[j].suit == east.possible_Chows[i + 1].suit && east.melds[j].melded[k].value == east.possible_Chows[i + 1].melded[k].value){
													east.possible_Chows[i + 1] = Meld();
												}
											}
											break;
										}
									}
								}
								if (i < 4){
									if (east.possible_Pongs[i].name != "NONE"){
										for (int j = 0; j < 4; j++){
											if (east.melds[j].name == "NONE"){
												east.melds[j] = east.possible_Pongs[i];
												break;
											}
										}
									}
								}
							}
							for (int i = 0; i < 4; i++){
								if (east.melds[i].name != "NONE"){
									std::cout << "Meld " << i + 1 << ": " << east.melds[i].name << endl;
									for (int j = 0; j < 3; j++){
										std::cout << east.melds[i].melded[j].value << " " << east.melds[i].melded[j].suit << endl;
									}
								}
							}
							east.has_Won_Hand = true;
							std::cout << "Points won: " << calculate_Points(east.melds, east.hand, last_Discard_Tile, east) << "\n\n";
							east.points += calculate_Points(east.melds, east.hand, last_Discard_Tile, east, false, last_Tile);
							last_Discarder.points -= east.points;
							hand_Won = true;
						}
						else if (last_Discarder.wind != south.wind && south.can_Win(south.hand, last_Discard_Tile)){

						}
						else if (last_Discarder.wind != west.wind && west.can_Win(west.hand, last_Discard_Tile)){

						}
						else if (last_Discarder.wind != north.wind && north.can_Win(north.hand, last_Discard_Tile)){

						}
					}
#pragma endregion Last Discarded Tile

					if (counter == 0 || hand_Won){
						if (round_Counter == NORTH && turn_Counter == NORTH){
							// Game end.
							round_Counter = 6;
							game_Running = NONE;
						}
						else{
							if (turn_Counter == NORTH){
								move_Round += 1;
								turn_Counter = EAST;
							}
							else{
								if (!hand_Won){
									ren_Counter++;
								}
								else if ((east.has_Won_Hand && east.wind == "East") || (south.has_Won_Hand && south.wind == "East")
									|| (west.has_Won_Hand && west.wind == "East") || (north.has_Won_Hand && north.wind == "East")){
									ren_Counter++;
								}
								else{
									turn_Counter = turn_Counter + 1;
								}
							}
							round_Counter = 6;

						}
					}
				}
			}
		}
#pragma endregion Modern Chinese Mahjong

#pragma region
		if (game_Running == NONE){
			std::cout << "Thank you for playing.\n";
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