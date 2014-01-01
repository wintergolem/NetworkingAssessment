
//INCLUDES
#include <list>
#include "RakNetTypes.h"

#ifndef _GAMECLASS_H_
#define _GAMESCLASS_H_

//GLOBAL VARIABLES
const int MAP_SIZE = 20;

//used to keep track of moves entered by players
class Move
{
public:
	bool updated;
	char moveType;
	char result; //d = dead, v = victory, s = successful move, m = marked i = invalid
	int x,y;

	void Update(char a_moveType, float a_x, float a_y)
	{
		moveType = a_moveType;
		x = a_x;
		y = a_y;
		updated = true;
	}

	void SetToNotUpdated()
	{
		updated = false;
		moveType = ' ';
		x = -1;
		y = -1;
	}

};

class Player
{
public:
	int locX, locY;//location
	Move move;
	int playerNumber;
	bool done; //via death or victory
	bool placed;
	RakNet::SystemAddress address;
	Player(int newPlayerNumber)
	{
		playerNumber = newPlayerNumber;
		move.updated = false;
		placed =false;
	}

	void Update()
	{

	}

	void SetDone() {done = true;}
};

class GameClass
{
public:

	bool DMSetExit;

	bool ready;
	std::list<Player> players;

	char map[MAP_SIZE][MAP_SIZE];

	GameClass()
	{
		ready = false;
		BuildMap();
		DMSetExit = false;
	}
	//move functions
	void AddPlayer(int newPlayerNumber, RakNet::SystemAddress a_address)
	{
		Player p = Player(newPlayerNumber);
		p.address = a_address;
		players.push_back(p);
	}
	int UpdatePlayers(int a_player, char a_moveType, int a_x, int a_y)
	{
		for(std::list<Player>::iterator it = players.begin(); it != players.end(); it++)
		{
			if(it->playerNumber == a_player)//find correct player
			{
				if(!DMSetExit)
				{
					if(it->playerNumber == 0)
					{
						it->move.Update(a_moveType, a_x, a_y);
						it->move.moveType = 'e';
						MarkMap(*it);
						it->move.updated = false;//should be handled by markmap, but just to be sure
						DMSetExit = true;
						return 1;
					}
				}
				if(it->move.updated) return 0; //previous move not calculated

				if(!it->placed && it->playerNumber != 0) it->move.Update('p', a_x, a_y);

				else it->move.Update(a_moveType, a_x, a_y);
				return 1;
				
			}
		}
		return 2;
	}
	void CheckReady()
	{
		ready = true;
		if(players.size() == 0 || !DMSetExit)
		{// if they are no players or the DM still needs to set exit, then game does not need to update
			ready = false;
			return;
		}
		for(std::list<Player>::iterator it = players.begin(); it != players.end(); it++)
		{
			if(!it->move.updated)
			{
				ready = false;
				return;
			}
		}
	}
	//map functions
	void BuildMap()
	{
		for(int i = 0; i < MAP_SIZE; ++i)
		{
			for(int k = 0; k < MAP_SIZE; ++k)
			{
				map[i][k] = '_';
			}
		}
	}
	void MarkMap(Player &p)
	{
		int result;
		switch(p.move.moveType)
		{
		case 'm': //move to
			result = CheckLoc(p.move.x, p.move.y);
			switch (result)
			{
			case 0: //empty move in
				//remove old marker from map
				map[ p.locX ][ p.locY ] = '_';
				//set move loc to current loc
				p.locX = p.move.x; p.locY = p.move.y;
				//place marker on map
				map[ p.locX ][ p.locY ] = 'P';
				p.move.result = 's';
				break;
			case 1: //bomb, time to die
				//remove old marker from map
				map[ p.locX ][ p.locY ] = '_';
				//remove bomb marker
				map[ p.move.x ][ p.move.y ] = '_';
				p.move.result = 'd';
				break;
			case 2://anothr player is in that space
				//for now, behave the same, later add a message telling the player they have company

				//remove old marker from map
				map[ p.locX ][ p.locY ] = '_';
				//set move loc to current loc
				p.locX = p.move.x; p.locY = p.move.y;
				//place marker on map
				map[ p.locX ][ p.locY ] = 'P';
				p.move.result = 'i';
				break;
			}
			break;

		case 'b':
			{
				int result2 = CheckLoc(p.move.x, p.move.y);
				switch (result2)
				{
				case 0:
					map[ p.move.x ][ p.move.y ] = 'B';
					break;
				case 1:
					//send message that they cant place a bomb on a player
					break;
				case 2:
					//send message that they cant place a bomb on a bomb
					break;
				}
				break;
			}
		case 'p':
			//place marker on map
			p.locX = p.move.x;
			p.locY = p.move.y;
				map[ p.locX ][ p.locY ] = 'P';
				p.move.result = 's';
				p.placed = true;
			break;
		case 'e': //setting exit
			map[p.move.x][p.move.y] = 'E';
			break;
		default:
			printf("Error - GameClass - MarkMap - Bad Case - Invalid moveType");
			break;
		}
		p.move.updated = false;
	}
	int CheckLoc(float a_x, float a_y) //returns: 0 = emtpy; 1 = bomb; 2 = another Player
	{
		switch (map[ int(a_x) ][ int(a_y) ] )
		{
		case '_':
			return 0;
		case 'B':
			return 1;
		case 'P':
			return 2;
		}
	}
	void PrintMap()
	{
		char screenMap[1000];
		ZeroMemory(screenMap,1000);
		int h = 0;
		for(int i =0; i < MAP_SIZE; i++)
		{
			for(int j =0; j < MAP_SIZE; j++)
			{
				screenMap[h] = map[i][j];
				h++;
			}
			screenMap[h] = '\n';
			h++;
		}
		//for(int i =0;i<sizeof(screenMap);i++)
			std::cout<<screenMap;
	}

	bool Update()
	{
		//check if ready ( all moves are updated
		CheckReady();
		if(ready)
		{
			//run through moves and update game
			for(std::list<Player>::iterator it = players.begin(); it != players.end(); it++)
			{
				MarkMap(*it);
			}
			//results of movement to client
			system("cls");
			PrintMap();
			return true;
		}
		return false;
	}

	
};

#endif