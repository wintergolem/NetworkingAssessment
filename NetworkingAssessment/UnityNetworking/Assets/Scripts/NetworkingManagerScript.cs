using UnityEngine;
using System.Collections;

public class NetworkingManagerScript : MonoBehaviour {

	string gameName = "DIETOOMO";

	bool refreshing = false;
	HostData [] hostData;

	float bX;
	float bY;
	float bW;
	float bH;

	public GameObject playerPrefab;
	public Transform spawnObject;

	// Use this for initialization
	void Start () {
		bX = Screen.width * 0.05f;
		bY = Screen.width * 0.05f;
		bW = Screen.width * 0.1f;
		bH = Screen.width * 0.1f;
	}
	
	// Update is called once per frame
	void Update () {
		if(refreshing)
		{
			if(MasterServer.PollHostList().Length > 0)
			{
				refreshing = false;
				hostData = MasterServer.PollHostList();
			}
		}
	}

	void SpawnPlayer()
	{
		Network.Instantiate(playerPrefab, spawnObject.position, Quaternion.identity, 0);
	}

	void StartServer()
	{
		print("starting");
		Network.InitializeServer(32, 25001, !Network.HavePublicAddress());
		MasterServer.RegisterHost(gameName, "Tutorial", "HI, DONT");
	}

	void OnServerInitialized()
	{
		SpawnPlayer();
	}

	void OnConnectedToServer()
	{
		SpawnPlayer();
	}

	void OnMasterServerEvent(MasterServerEvent mse)
	{
		if(mse == MasterServerEvent.RegistrationSucceeded)
		{
			Debug.Log("win");
		}
	}

	void RefreshHostList()
	{
		MasterServer.RequestHostList(gameName);
		refreshing = true;
	}
//GUI
	void OnGUI()
	{
		if(!Network.isClient && !Network.isServer)
		{
			if( GUI.Button(new Rect(bX, bY, bW, bH), "Start Server") )
			{
				StartServer();
			}

			if( GUI.Button(new Rect(bX, bY * 1.2f + bH, bW, bH), "Refresh Host") )
			{
				RefreshHostList();
			}
			if(hostData != null)
			{
				for( int i =0; i<hostData.Length; i++)
				{
					if(GUI.Button (new Rect(bX * 1.5f + bW, bY * 1.2f + (bH * i), bW*3, bH*0.5f), hostData[i].gameName) )
					{
						Network.Connect (hostData[i]);
					}
				}
			}
		}
	}
}
