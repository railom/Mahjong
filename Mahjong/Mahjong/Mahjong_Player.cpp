// A mahjong player.
// Author: Alex Lobl
// Date: 6/24/2015
// Version: 0.2.0 Alpha

#include "Mahjong_Tile.cpp"
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

struct Player{
	string wind;	 // A player has a seat wind which changes each round in the order East -> North -> West -> South -> East
	Tile* hand;		 // A player has a hand of tiles that makes up the brunt of the game.
	Tile* discards;  // In Japanese mahjong, each player discards into her own discard pile due to sacred discard.
	Tile* claimed;   // An array of tiles that have been claimed and opened as melds or seasons (if seasons are used).
	Meld* melds = new Meld[4]; // An array of melds that have been openly melded into triples or a pair.
	Meld* possible_Pairs = new Meld[13]; // An array of melds that shows the possible pairs that can be used to win.
	Meld* possible_Chows = new Meld[9]; // An array of melds that shows possible chows that can be used to win.
	Meld* possible_Pongs = new Meld[4]; // An array of melds that shows possible pongs that can be used to win.
	Meld* chow_Choices = new Meld[9];
	Meld pair;		 // The winning pair.
	int player_Value;// Purely program related. Matches a player to her corresponding season and flower.
	int hand_Points = 0; // The amount of points a player's hand is currently worth.
	int points;		 // A player has a number of points. This value can be positie or negative in some versions.
	int tile_Number = 0;
	int meld_Number = 1;
	bool riichi;	 // In Japanese mahjong, declaring a ready hand. Determines furiten status.
	bool furiten;	 // A player is furiten if she discards her wait with a ready hand and cannot win unless it is self-draw (riichi) or until her next turn.
	bool has_Won_Hand = false; // Tracks which player has won the current hand.
	bool is_AI = false;
	bool was_North = false;
	bool thirteen_Orphans = false;
	bool tenho = false;
	bool chiho = false;

	Player(){

	}

	Player(string w, bool ai = false, int x = 0){
		wind = w;
		if (wind == "East"){
			player_Value = 1;
		}
		else if (wind == "South"){
			player_Value = 2;
		}
		else if (wind == "West"){
			player_Value = 3;
		}
		else if (wind == "North"){
			player_Value = 4;
		}
		points = x;
		hand = new Tile[13];
		claimed = new Tile[8];
		is_AI = ai;
	}

	// A player can sort her hand.
	// This sort works based on suit first, putting the tiles in alphabetical order
	// then sorting each suit by tile value. O(n^2)
	Tile* sort_Hand(Tile* h){
		Tile temp;
		for (int i = 0; i <= 13; i++){
			for (int j = i + 1; j <= 12; j++){
				if (h[i].suit > h[j].suit){
					temp = h[j];
					h[j] = h[i];
					h[i] = temp;
				}
				else if (h[i].suit == h[j].suit){
					if (h[i].value > h[j].value){
						temp = h[j];
						h[j] = h[i];
						h[i] = temp;
					}
				}
			}
		}
		return h;
	}

	Meld make_Pair(Tile* h, Tile d){
		for (int i = 0; i < 13; i++){
			if (h[i] == d) return Meld(h[i], d);
		}
		
	}

	// A player knows whether or not she can make a meld with a previously discarded tile
	// based on her hand.
	bool can_Chow(Tile* h, Tile x){
		int j = 0;
		for (int i = 0; i < 9; i++){
			if (chow_Choices[i].name != "NONE"){
				chow_Choices[i] = Meld();
			}
		}
		for (int i = 0; i < 13; i++){
			if ((h[i].value == h[i+1].value - 1 && h[i+1].value == x.value - 1 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit) || (h[i].value == h[i+1].value + 1 && h[i+1].value == x.value + 1 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit) ||
				(h[i].value == h[i+1].value - 2 && h[i+1].value == x.value + 1 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit) || (h[i].value == h[i+1].value + 1 && h[i+1].value == x.value - 2 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit) ||
				(h[i].value == h[i+1].value - 1 && h[i+1].value == x.value + 2 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit) || (h[i].value == h[i+1].value + 2 && h[i+1].value == x.value - 1 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit)){
				chow_Choices[j] = Meld(h[i], h[i + 1], x);
				j++;
			}
		}
		if (chow_Choices[0].name != "NONE"){
			return true;
		}
		return false;
	}

	bool can_Pong(Tile* h, Tile x){
		for (int i = 0; i < 13; i++){
			if (h[i].value == h[i + 1].value && h[i + 1].value == x.value && h[i].suit == h[i + 1].suit && h[i + 1].suit == x.suit){
				return true;
			}
		}
		return false;
	}

	bool can_Kong(Tile* h, Tile x){
		for (int i = 0; i < 13; i++){
			if (h[i].value == h[i + 1].value && h[i + 1].value == h[i + 2].value && h[i + 2].value == x.value && h[i].suit == h[i + 1].suit && h[i + 1].suit == h[i + 2].suit && h[i + 2].suit == x.suit){
				return true;
			}
		}
		return false;
	}

	// A player wins when she has 4 triples and a pair (total of 14 tiles).
	// She is considered to be ready to win when she is one tile away from winning.
	bool can_Win(Tile* h, Tile x){
		shift_Chows();
		// Check open melds only. If there are 4 open melds and a pair, or if there are 3 open melds, a player can pong or chow, and has a pair.
		if ((melds[3].name != "NONE" && can_Make_Pair(h, x)) || (melds[2].name != "NONE" && possible_Pairs[0].name != "NONE" && (can_Chow(h, x) || can_Pong(h, x)))){
			return true;
		}
		// Check possible combinations of chows and pongs in hand.
		else if ((possible_Chows[3].name != "NONE" && can_Make_Pair(h, x)) || (possible_Chows[2].name != "NONE" && possible_Pairs[0].name != "NONE" && (can_Chow(h, x) || can_Pong(h, x)))
			|| (possible_Chows[2].name != "NONE" && possible_Pongs[0].name != "NONE" && can_Make_Pair(h, x)) || (possible_Chows[1].name != "NONE" && possible_Pongs[1].name != "NONE" && can_Make_Pair(h, x))
			|| (possible_Chows[1].name != "NONE" && possible_Pongs[0].name != "NONE" && possible_Pairs[0].name != "NONE" && (can_Chow(h, x) || can_Pong(h, x)))
			|| (possible_Chows[0].name != "NONE" && possible_Pongs[2].name != "NONE" && can_Make_Pair(h, x)) || (possible_Chows[0].name != "NONE" && possible_Pongs[1].name != "NONE" && possible_Pairs[0].name != "NONE" && (can_Chow(h, x) || can_Pong(h, x)))
			|| (possible_Pongs[3].name != "NONE" && can_Make_Pair(h, x)) || (possible_Pongs[2].name != "NONE" && possible_Pairs[0].name != "NONE" && (can_Chow(h, x) || can_Pong(h, x)))){
			return true;
		} 
		// Check combination of open and closed melds and pairs.
		else if ((melds[0].name != "NONE" && possible_Chows[2].name != "NONE" && can_Make_Pair(h, x)) || (melds[0].name != "NONE" && possible_Chows[1].name != "NONE" && possible_Pairs[0].name != "NONE" && (can_Chow(h, x) || can_Pong(h,x))) 
			|| (melds[0].name != "NONE" && possible_Chows[1].name != "NONE" && possible_Pongs[0].name != "NONE" && can_Make_Pair(h, x)) || (melds[0].name != "NONE" && possible_Pongs[2].name != "NONE" && can_Make_Pair(h, x)) || (melds[0].name != "NONE" && possible_Pongs[1].name != "NONE" && possible_Pairs[0].name != "NONE" && (can_Chow(h, x) || can_Pong(h, x)))
			|| (melds[0].name != "NONE" && possible_Pongs[1].name != "NONE" && possible_Chows[0].name != "NONE" && can_Make_Pair(h, x)) || (melds[1].name != "NONE" && possible_Chows[1].name != "NONE" && can_Make_Pair(h,x)) || (melds[1].name != "NONE" && possible_Chows[0].name != "NONE" && possible_Pairs[0].name != "NONE" && (can_Chow(h,x) || can_Pong(h,x)))
			|| (melds[1].name != "NONE" && possible_Chows[0].name != "NONE" && possible_Pongs[0].name != "NONE" && can_Make_Pair(h,x)) || (melds[1].name != "NONE" && possible_Pongs[0].name != "NONE" && possible_Pairs[0].name != "NONE" && (can_Chow(h,x) || can_Pong(h,x))) || (melds[1].name != "NONE" && possible_Pongs[1].name != "NONE" && can_Make_Pair(h,x))
			|| (melds[2].name != "NONE" && possible_Chows[0].name != "NONE" && can_Make_Pair(h,x)) || (melds[2].name != "NONE" && possible_Pongs[0].name != "NONE" && can_Make_Pair(h,x))){
			return true;
		}
		// Thirteen Orphans 
		else if ((has_Tile(Tile('p', 1)) || (x.suit == "Pin" && x.value == 1)) && (has_Tile(Tile('p', 9)) || (x.suit == "Pin" && x.value == 9)) && (has_Tile(Tile('m', 1)) || (x.suit == "Man" && x.value == 1)) && (has_Tile(Tile('m', 9)) || (x.suit == "Man" && x.value == 9)) && (has_Tile(Tile('s', 1)) || (x.suit == "Sou" && x.value == 1))
			&& (has_Tile(Tile('s', 9)) || (x.suit == "Sou" && x.value == 9)) && (has_Tile(Tile('g')) || x.suit == "Green Dragon") && (has_Tile(Tile('t')) || x.suit == "White Dragon") && (has_Tile(Tile('d')) || x.suit == "Red Dragon") && (has_Tile(Tile('w')) || x.suit == "West") && (has_Tile(Tile('s')) || x.suit == "South") && (has_Tile(Tile('n')) || x.suit == "North")
			&& (has_Tile(Tile('e')) || x.suit == "East") && (can_Make_Pair(h, x) || possible_Pairs[0].name != "NONE")){
			thirteen_Orphans = true;
			return true;
		}
		return false;
	}

	// Whether or not it's possible to make a pair. Used more for end of hand.
	bool can_Make_Pair(Tile* h, Tile x){
		for (int i = 0; i < 13; i++){
			if (h[i].value > 0){
				if (h[i].value == x.value && h[i].suit == x.suit){
					pair = Meld(h[i], x);
					return true;
				}
			}
		}
		return false;
	}

	// Checks the hand for a specified tile.
	bool has_Tile(Tile x){
		for (int i = 0; i < 13; i++){
			if (hand[i].suit == x.suit && hand[i].value == x.value){
				return true;
			}
		}
		return false;
	}

	int count_Tile(Tile x){
		if (has_Tile(x)){
			for (int i = 0; i < 13; i++){
				if (hand[i].suit == x.suit && hand[i].value == x.value){
					tile_Number++;
				}
			}
		}
		return tile_Number;
	}

	int count_Melds(Meld m){
		for (int i = 0; i < 9; i++){
			if (possible_Chows[i].melded != NULL && possible_Chows[i + 1].melded != NULL){
				/*if (m.suit == possible_Chows[i].suit && possible_Chows[i].suit == possible_Chows[i + 1].suit 
					&& (m.melded[0].value == possible_Chows[i].melded[0].value && possible_Chows[i].melded[0].value == possible_Chows[i + 1].melded[0].value) 
					&& (m.melded[1].value == possible_Chows[i].melded[1].value && possible_Chows[i].melded[1].value == possible_Chows[i + 1].melded[1].value)
					&& (m.melded[2].value == possible_Chows[i].melded[2].value && possible_Chows[i].melded[2].value == possible_Chows[i + 1].melded[2].value)){
					meld_Number++;
				}*/
				if (m == possible_Chows[i] && possible_Chows[i] == possible_Chows[i + 1]){
					meld_Number++;
				}
			}
		}
		return meld_Number;
	}

	int tile_Pos(Tile x){
			for (int i = 0; i < 13; i++){
				if (hand[i].suit == x.suit && hand[i].value == x.value){
					return i;
				}
			}
	}

	// Creates all possible pairs that may be used in the end of a hand
	// to win.
	void make_Pairs(Tile* h){
		int j = 0;
		for (int i = 0; i < 13; i++){
			if (h[i].value > 0){
				if (h[i].value == h[i + 1].value && h[i].suit == h[i + 1].suit){
					possible_Pairs[j] = Meld(h[i], h[i + 1]);
					j++;
				}
			}
		}
	}

	void make_Chows(Tile* h, Tile d = NULL){
		Tile drawn = d;
		int k = 0;
		int l = 0;
		for (int i = 0; i < 13; i++){
			k = i + 2;
			if (h[i].value > 0){
				for (int j = i + 1; j < 13 - i; j++){
					if (k < 13){
						if (h[i].suit == h[j].suit && h[j].suit == h[k].suit){
							if (h[i].value == h[j].value - 1 && h[j].value == h[k].value - 1){
								possible_Chows[l] = Meld(h[i], h[j], h[k]);
								l++;
							}
							k++;
						}
					}
				}
			}
		}
	}

	void make_Pongs(Tile* h, Tile d = NULL){
		int j = 0;
		for (int i = 0; i < 13; i++){
			if (h[i].value > 0){
				if (h[i].value == h[i + 1].value && h[i + 1].value == h[i + 2].value && h[i].suit == h[i + 1].suit && h[i + 1].suit == h[i + 2].suit){
					possible_Pongs[j] = Meld(h[i], h[i + 1], h[i + 2]);
					j++;
				}
			}
		}
	}

	void shift_Chows(){
		for (int i = 0; i < 9; i++){
			if (possible_Chows[i].name != "NONE"){
				// To Do: Test having the same chow 2 or more times (123 123 123 123 pin). Open or closed. Currently should break.
				/*if (count_Melds(possible_Chows[i]) > 1){
					i = i + count_Melds(possible_Chows[i]);
				}
				if (i < 9 && possible_Chows[i].name != "NONE"){*/
				for (int k = 0; k < 4; k++){
					if (possible_Chows[i].suit == possible_Chows[i + 1].suit && possible_Chows[i].melded[k].value == possible_Chows[i + 1].melded[k].value){
						possible_Chows[i + 1] = Meld();
					}
				}
				//}
			}
		}
		for (int i = 0; i < 9; i++){
			if (i + 1 < 9){
				if (possible_Chows[i].name == "NONE" && possible_Chows[i + 1].name != "NONE"){
					possible_Chows[i] = possible_Chows[i + 1];
					possible_Chows[i + 1] = Meld();
				}
			}
		}
	}

	// A player can look at what she has openly claimed.
	// Includes flowers, seasons, and any melds.
	void see_Claimed(){
		cout << "Tiles claimed: " << endl;
		for (int i = 0; i < 8; i++){
			if (claimed[i].value > 0){
				cout << claimed[i].value << " " << claimed[i].suit << endl;
			}
		}
		for (int i = 0; i < 4; i++){
			if (melds[i].name != "NONE"){
				cout << melds[i].name << " of:" << endl;
				for (int j = 0; j < 4; j++){
					if (melds[i].melded[j].value > 0){
						cout << melds[i].melded[j].value << " " << melds[i].melded[j].suit << endl;
					}
				}
			}
		}
		cout << endl;
	}

	// A player can see the value of her hand.
	// This may not be necessary, or possibly be integrated for use
	// with a beginner mahjong level. For people who can't read points
	// from hand.
	void see_Hand_Points(){
		cout << "Current Hand Value: " << hand_Points << endl << endl;
	}

	// A player can see what's in her hand.
	void see_Hand(){
		for (int i = 0; i < 13; i++){
			cout << "Tile " << i + 1 << ": " << hand[i].value
				<< " " << hand[i].suit << endl;
		}
		cout << "\n";
	}

	void show_Chows_Choices(){
		for (int i = 0; i < 9; i++){
			if (chow_Choices[i].name != "NONE"){
				for (int j = 0; j < 4; j++){
					if (chow_Choices[i].melded[j].value > 0){
						cout << "Chow " << i + 1 << ": " << chow_Choices[i].melded[j].value << " " << chow_Choices[i].melded[j].suit << endl;
					}
				}
				cout << "\n";
			}
		}
	}

	// Memory cleanup.
	void free_Player_Memory(){
		delete[] melds;
		delete[] possible_Pairs;
		delete[] hand;
		delete[] claimed;
	}
};