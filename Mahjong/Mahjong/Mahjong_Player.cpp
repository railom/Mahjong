// A mahjong player.
// Author: Alex Lobl
// Date: 6/5/2015
// Version: 0.0.1 Alpha

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
	int player_Value;// Purely program related. Matches a player to her corresponding season and flower.
	int hand_Points; // The amount of points a player's hand is currently worth.
	int points;		 // A player has a number of points. This value can be positie or negative in some versions.
	bool riichi;	 // In Japanese mahjong, declaring a ready hand. Determines furiten status.
	bool furiten;	 // A player is furiten if she discards her wait with a ready hand and cannot win unless it is self-draw (riichi) or until her next turn.


	Player(string w, int x = 0){
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
	}

	//void determine_Seating(int roll = 1){}

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

	// A player can look at what she has openly claimed.
	void see_Claimed(){
		for (int i = 0; i < 8; i++){
			if (claimed[i].value > 0){
				cout << "Tile " << i + 1 << " in East's claimed: " << claimed[i].value
					<< " " << claimed[i].suit << endl;
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
};