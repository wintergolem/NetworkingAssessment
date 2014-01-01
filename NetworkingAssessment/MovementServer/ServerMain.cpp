// ----------------------------------------------------------------------
//Steven Hoover
//Server.cpp
// ----------------------------------------------------------------------

#include "MessageIdentifiers.h"

#include "RakPeerInterface.h"
#include "RakNetStatistics.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "RakSleep.h"
#include "PacketLogger.h"
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include "Kbhit.h"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include "Gets.h"
#include "BitStream.h"
#include <iostream>
#include "GameClass.h"

#pragma pack(push, 1)
struct custom
{
	unsigned char typeID;
	int clientID;
	char moveType;
	float x,y;
	//char message[500];
};
#pragma pack(pop)

#if LIBCAT_SECURITY==1
#include "SecureHandshake.h" // Include header for secure handshake
#endif

#if defined(_CONSOLE_2)
#include "Console2SampleIncludes.h"
#endif

// We copy this from Multiplayer.cpp to keep things all in one file
unsigned char GetPacketIdentifier(RakNet::Packet *p);

#ifdef _CONSOLE_2
_CONSOLE_2_SetSystemProcessParams
#endif

//networking variables
	// Pointers to the interfaces of our server and client.
	// Note we can easily have both in the same program
		RakNet::RakPeerInterface *server;
		RakNet::RakNetStatistics *rss;
		// Holds packets
	RakNet::Packet* p;

	// GetPacketIdentifier returns this
	unsigned char packetIdentifier;
	RakNet::SystemAddress clientID;
	
	// Holds user data
	char portstring[30];
	RakNet::SocketDescriptor socketDescriptors[2];
		DataStructures::List< RakNet::RakNetSocket2* > sockets;
			char message[2048];
			// Message now holds what we want to broadcast
		char message2[2048];



//game variables
		int totalPlayers = 0;
		GameClass game;

//prototypes
		void SendPlayerType();


custom* ConvertToCustom(RakNet::Packet * packet)
{
	custom *c = new custom();

	RakNet::BitStream bsIn(packet->data, packet->length, false); // The false is for efficiency so we don't make a copy of the passed data
	//bsIn.Read(*c);
	bsIn.Read(c->typeID);
	bsIn.Read(c->clientID);
	bsIn.Read(c->moveType);
	bsIn.Read(c->x);
	bsIn.Read(c->y);
	return c;
}

//networking functions
void Setup()
{
	server=RakNet::RakPeerInterface::GetInstance();
	server->SetIncomingPassword("Rumpelstiltskin", (int)strlen("Rumpelstiltskin"));
	server->SetTimeoutTime(30000,RakNet::UNASSIGNED_SYSTEM_ADDRESS);
//	RakNet::PacketLogger packetLogger;
//	server->AttachPlugin(&packetLogger);

#if LIBCAT_SECURITY==1
	cat::EasyHandshake handshake;
	char public_key[cat::EasyHandshake::PUBLIC_KEY_BYTES];
	char private_key[cat::EasyHandshake::PRIVATE_KEY_BYTES];
	handshake.GenerateServerKey(public_key, private_key);
	server->InitializeSecurity(public_key, private_key, false);
	FILE *fp = fopen("publicKey.dat","wb");
	fwrite(public_key,sizeof(public_key),1,fp);
	fclose(fp);
#endif

	
	// Record the first client that connects to us so we can pass it to the ping function
	clientID=RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	printf("This is a sample implementation of a text based chat server.\n");
	printf("Connect to the project 'Chat Example Client'.\n");
	printf("Difficulty: Beginner\n\n");

	// A server
	puts("Enter the server port to listen on");
	Gets(portstring,sizeof(portstring));
	if (portstring[0]==0)
		strcpy(portstring, "1234");
	
	puts("Starting server.");
	// Starting the server is very simple.  2 players allowed.
	// 0 means we don't care about a connectionValidationInteger, and false
	// for low priority threads
	// I am creating two socketDesciptors, to create two sockets. One using IPV6 and the other IPV4
	
	socketDescriptors[0].port=atoi(portstring);
	socketDescriptors[0].socketFamily=AF_INET; // Test out IPV4
	socketDescriptors[1].port=atoi(portstring);
	socketDescriptors[1].socketFamily=AF_INET6; // Test out IPV6
	bool b = server->Startup(4, socketDescriptors, 2 )==RakNet::RAKNET_STARTED;
	server->SetMaximumIncomingConnections(4);
	if (!b)
	{
		printf("Failed to start dual IPV4 and IPV6 ports. Trying IPV4 only.\n");

		// Try again, but leave out IPV6
		b = server->Startup(4, socketDescriptors, 1 )==RakNet::RAKNET_STARTED;
		if (!b)
		{
			puts("Server failed to start.  Terminating.");
			exit(1);
		}
	}
	server->SetOccasionalPing(true);
	server->SetUnreliableTimeout(1000);


	server->GetSockets(sockets);
	printf("Socket addresses used by RakNet:\n");
	for (unsigned int i=0; i < sockets.Size(); i++)
	{
		printf("%i. %s\n", i+1, sockets[i]->GetBoundAddress().ToString(true));
	}

	printf("\nMy IP addresses:\n");
	for (unsigned int i=0; i < server->GetNumberOfAddresses(); i++)
	{
		RakNet::SystemAddress sa = server->GetInternalID(RakNet::UNASSIGNED_SYSTEM_ADDRESS, i);
		printf("%i. %s (LAN=%i)\n", i+1, sa.ToString(false), sa.IsLANAddress());
	}

	printf("\nMy GUID is %s\n", server->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
	puts("'quit' to quit. 'stat' to show stats. 'ping' to ping.\n'pingip' to ping an ip address\n'ban' to ban an IP from connecting.\n'kick to kick the first connected player.\nType to talk.");

}

int ConsoleCommands()
{
	//0 = break
	//1 = continue
	//2 = nothing
	if (kbhit())
	{
		// Notice what is not here: something to keep our network running.  It's
		// fine to block on gets or anything we want
		// Because the network engine was painstakingly written using threads.
		Gets(message,sizeof(message));

		if (strcmp(message, "quit")==0)
		{
			puts("Quitting.");
			return 0;
		}

		if (strcmp(message, "stat")==0)
		{
			rss=server->GetStatistics(server->GetSystemAddressFromIndex(0));
			StatisticsToString(rss, message, 2);
			printf("%s", message);
			printf("Ping %i\n", server->GetAveragePing(server->GetSystemAddressFromIndex(0)));
	
			return 1;
		}

		if (strcmp(message, "ping")==0)
		{
			server->Ping(clientID);

			return 1;
		}

		if (strcmp(message, "pingip")==0)
		{
			printf("Enter IP: ");
			Gets(message,sizeof(message));
			printf("Enter port: ");
			Gets(portstring,sizeof(portstring));
			if (portstring[0]==0)
				strcpy(portstring, "1234");
			server->Ping(message, atoi(portstring), false);

			return 1;
		}

		if (strcmp(message, "kick")==0)
		{
			server->CloseConnection(clientID, true, 0);

			return 1;
		}

		if (strcmp(message, "getconnectionlist")==0)
		{
			RakNet::SystemAddress systems[10];
			unsigned short numConnections=10;
			server->GetConnectionList((RakNet::SystemAddress*) &systems, &numConnections);
			for (int i=0; i < numConnections; i++)
			{
				printf("%i. %s\n", i+1, systems[i].ToString(true));
			}
			return 1;
		}

		if (strcmp(message, "ban")==0)
		{
			printf("Enter IP to ban.  You can use * as a wildcard\n");
			Gets(message,sizeof(message));
			server->AddToBanList(message);
			printf("IP %s added to ban list.\n", message);

			return 1;
		}
		
		// Append Server: to the message so clients know that it ORIGINATED from the server
		// All messages to all clients come from the server either directly or by being
		// relayed from other clients
		message2[0]=0;
		strcpy(message2, "Server: ");
		strcat(message2, message);
	
		// message2 is the data to send
		// strlen(message2)+1 is to send the null terminator
		// HIGH_PRIORITY doesn't actually matter here because we don't use any other priority
		// RELIABLE_ORDERED means make sure the message arrives in the right order
		// We arbitrarily pick 0 for the ordering stream
		// RakNet::UNASSIGNED_SYSTEM_ADDRESS means don't exclude anyone from the broadcast
		// true means broadcast the message to everyone connected
		server->Send(message2, (const int) strlen(message2)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		return 2;

	}
	return 2;
}

void SendResultsToClients()
{
	unsigned short * NumberOfSystems = new unsigned short[10];
	RakNet::SystemAddress * remoteSystems = new RakNet::SystemAddress[8];
	server->GetConnectionList(remoteSystems, NumberOfSystems);

	for(std::list<Player>::iterator it = game.players.begin(); it != game.players.end(); it++)
	{
		switch(it->move.result)
		{
		case 's':
			strcpy(message, "successful");
			break;
		case 'd':
			strcpy(message, "dead");
			break;
		case 'i':
			strcpy(message, "invalid");
			break;
		}
		server->Send(message, (const int) strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, it->address, true);
	}

}

int HandlePackets()
{
	// This sleep keeps RakNet responsive
		RakSleep(30);

		int result = ConsoleCommands();
		if(result == 0 || result ==1) 
			return result;
		//result == 2, do nothing
	
		// Get a packet from either the server or the client

		for (p=server->Receive(); p; server->DeallocatePacket(p), p=server->Receive())
		{
			// We got a packet, get the identifier with our handy function
			packetIdentifier = GetPacketIdentifier(p);

			// Check if this is a network message packet
			switch (p->data[0])//switch (packetIdentifier)
			{
			case ID_DISCONNECTION_NOTIFICATION:
				// Connection lost normally
				printf("ID_DISCONNECTION_NOTIFICATION from %s\n", p->systemAddress.ToString(true));;
				break;


			case ID_NEW_INCOMING_CONNECTION:
				// Somebody connected.  We have their IP now
				//printf("ID_NEW_INCOMING_CONNECTION from %s with GUID %s\n", p->systemAddress.ToString(true), p->guid.ToString());
				printf("ID_NEW_INCOMING_CONNECTION \n");
				clientID=p->systemAddress; // Record the player ID of the client

				printf("Remote internal IDs:\n");
				for (int index=0; index < MAXIMUM_NUMBER_OF_INTERNAL_IDS; index++)
				{
					RakNet::SystemAddress internalId = server->GetInternalID(p->systemAddress, index);
					if (internalId!=RakNet::UNASSIGNED_SYSTEM_ADDRESS)
					{
						printf("%i. %s\n", index+1, internalId.ToString(true));
					}
				}
				SendPlayerType();
				strcpy(message, "change need");
				game.AddPlayer(totalPlayers, p->systemAddress);
				totalPlayers++; //first playr should be player zero, but now there is one player
				break;

			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
				break;

			case ID_CONNECTED_PING:
				break;
			case ID_UNCONNECTED_PING:
				printf("Ping from %s\n", p->systemAddress.ToString(true));
				break;

			case ID_CONNECTION_LOST:
				// Couldn't deliver a reliable packet - i.e. the other system was abnormally
				// terminated
				printf("ID_CONNECTION_LOST from %s\n", p->systemAddress.ToString(true));;
				break;

			case 232:
			case ID_USER_PACKET_ENUM:
				{
					custom *c = ConvertToCustom(p);
					
					game.UpdatePlayers(c->clientID, c->moveType, c->x, c->y);

						//wrong system address->  server->Send(message, (const int) strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, true);
					//server->Send(message, (const int) strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
					
					delete c;
					break;
				}
			default:
				// The server knows the static data of all clients, so we can prefix the message
				// With the name data
				printf("%s\n", p->data);

				// Relay the message.  We prefix the name for other clients.  This demonstrates
				// That messages can be changed on the server before being broadcast
				// Sending is the same as before
				sprintf(message, "%s", p->data);
				assert(message != "change need");
				assert(message == "change need");
				server->Send(message, (const int) strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, true);

				break;
			}
		}
		return 2;
}


//game specific functions
void SendPlayerType()
{
	switch(totalPlayers)
	{
	case 0:
		strcpy(message, "DM");
		
		break;
	default:
		ZeroMemory(message, sizeof(message) );
		message[0] = 'p';
		message[1] = 't';
		message[2] = char(totalPlayers);
		break;
	}
	server->Send(message, (const int) strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

int main()
{
	Setup();

	// Game Loop
	while (1)
	{

		int result = HandlePackets();
		if(result == 0) break;
		if(result ==1) continue;
		if(game.Update())
			SendResultsToClients();
	}

	server->Shutdown(300);
	// We're done with the network
	RakNet::RakPeerInterface::DestroyInstance(server);

	return 0;
}

// Copied from Multiplayer.cpp
// If the first byte is ID_TIMESTAMP, then we want the 5th byte
// Otherwise we want the 1st byte
unsigned char GetPacketIdentifier(RakNet::Packet *p)
{
	if (p==0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
		return (unsigned char) p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
	}
	else if ((unsigned char)p->data[0] == ID_USER_PACKET_ENUM)
	{
		return (unsigned char) p->data[sizeof(RakNet::MessageID)];
	}
	else
		return (unsigned char) p->data[0];
}
