using UnityEngine;
using System.Collections;



public class ClickAndMoveScript : MonoBehaviour {
	public NavMeshAgent navMeshAgent;
	public PlayerGamePlayScript playScript;
	public NetworkingVariables net;

	public Vector3 target;
	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		if(networkView.isMine)
		{
			CheckPos();
			if(Input.GetMouseButton(0) )
			{
				Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
				RaycastHit hit;
				if(Physics.Raycast( ray, out hit) )
				{
					if(hit.collider.tag == "ground")
					{
						net.target = hit.point;
						playScript.moveState = MovementState.General;
					}
					if(hit.collider.tag == "minion")
					{
						target = hit.collider.transform.position;
						playScript.moveState = MovementState.ToAttack;
					}
				}
			}

			switch (playScript.moveState)
			{
			case MovementState.General:
				if(navMeshAgent.destination != net.target)
				{
					navMeshAgent.SetDestination(net.target);
				}
				break;
			case MovementState.ToAttack: //move toward target until attackRange is met then attack
				if(Vector3.Distance(target, transform.position) < playScript.stats.attackRange)
				{
					playScript.attacking = true;
					playScript.moveState = MovementState.Still;
				}
				break;
			}
		}
		else{
			if(navMeshAgent.destination != net.target)
			{
				navMeshAgent.SetDestination(net.target);
			}
		}
	}

	public void CheckPos()
	{
//		if(Vector3.Distance( transform.position, playScript.pos) > 10)
//		{
//			navMeshAgent.Warp(playScript.pos);
//		}	
	}
}
