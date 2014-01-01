using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public enum MovementState {General, ToAttack, Still};

public class PlayerGamePlayScript : MonoBehaviour {

	public int id;

	public string m_name;

	public BaseStats stats;

	//death stats
	float timeDead;
	float timeWaitTilRespawn;

	//K / D stats
	int kills;
	int deaths;
	int assists;
	float timeTilHitExspires;

	//who has been hitting me?
	List<int> hitMeIds;

	//for networking purposes
	public Vector3 pos;

	//variables handled by ClickandMoveScript
	public bool attacking;
	public Vector3 target;
	public MovementState moveState;

	// Use this for initialization
	void Start () {
		hitMeIds = new List<int>();
	}
	
	// Update is called once per frame
	void Update () {
		if(networkView.isMine)
		{
			pos = transform.position;
		}
	}

	public void Init(int a_id, string a_name, BaseStats init)
	{
		id = a_id;
		m_name = a_name;
		stats = init;
	}


}
