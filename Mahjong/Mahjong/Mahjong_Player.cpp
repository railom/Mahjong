// A mahjong player.
// Author: Alex Lobl
// Date: 6/3/2015
// Version: 0.0.1 Alpha

#include "Mahjong_Tile.cpp"

using namespace std;

struct Player{
	string wind;	 // A player has a seat wind which changes each round.
	Tile* hand;		 // A player has a hand of tiles that makes up the brunt of the game.
	int points;		 // A player has a number of points. This value can be positie or negative in some versions.

	Player(string w, int x = 0){
		wind = w;
		hand = new Tile[13];
		points = x;
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
};