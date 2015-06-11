// A mahjong player.
// Author: Alex Lobl
// Date: 6/11/2015
// Version: 0.1.1 Alpha

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
	Meld* possible_Pairs = new Meld[7]; // An array of melds that shows the possible pairs that can be used to win.
	Meld* possible_Chows = new Meld[9]; // An array of melds that shows possible chows that can be used to win.
	Meld* possible_Pongs = new Meld[4]; // An array of melds that shows possible pongs that can be used to win.
	Meld pair;		 // The winning pair.
	int player_Value;// Purely program related. Matches a player to her corresponding season and flower.
	int hand_Points; // The amount of points a player's hand is currently worth.
	int points;		 // A player has a number of points. This value can be positie or negative in some versions.
	bool riichi;	 // In Japanese mahjong, declaring a ready hand. Determines furiten status.
	bool furiten;	 // A player is furiten if she discards her wait with a ready hand and cannot win unless it is self-draw (riichi) or until her next turn.
	bool has_Won_Hand = false; // Tracks which player has won the current hand.
	bool is_AI = false;
	bool was_North = false;

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

	// A player knows whether or not she can make a meld with a previously discarded tile
	// based on her hand.
	// TODO: Return all possible matches, not just the first one.
	bool can_Chow(Tile* h, Tile x){
		for (int i = 0; i < 13; i++){
			if ((h[i].value == h[i+1].value - 1 && h[i+1].value == x.value - 1 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit) || (h[i].value == h[i+1].value + 1 && h[i+1].value == x.value + 1 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit) ||
				(h[i].value == h[i+1].value - 2 && h[i+1].value == x.value + 1 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit) || (h[i].value == h[i+1].value + 1 && h[i+1].value == x.value - 2 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit) ||
				(h[i].value == h[i+1].value - 1 && h[i+1].value == x.value + 2 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit) || (h[i].value == h[i+1].value + 2 && h[i+1].value == x.value - 1 && h[i].suit == h[i+1].suit && h[i+1].suit == x.suit)){
				return true;
			}
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
		if ((melds[3].name != "NONE" && can_Make_Pair(h, x)) || (melds[2].name != "NONE" && possible_Pairs[0].name != "NONE" && (can_Chow(h, x) || can_Pong(h,x)))){
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

	void make_Pongs(Tile* h, Tile d = NULL){
		int j = 0;
		for (int i = 0; i < 13; i++){
			if (h[i].value == h[i + 1].value && h[i + 1].value == h[i + 2].value && h[i].suit == h[i + 1].suit && h[i + 1].suit == h[i + 2].suit){
				possible_Pongs[j] = Meld(h[i], h[i + 1], h[i + 2]);
				j++;
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

	// Memory cleanup.
	void free_Player_Memory(){
		delete[] melds;
		delete[] possible_Pairs;
		delete[] hand;
		delete[] claimed;
	}
};