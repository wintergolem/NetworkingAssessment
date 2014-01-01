using UnityEngine;
using System.Collections;

public class NetworkingVariables : MonoBehaviour{

	public Vector3 target;

	void OnSerializeNetworkView(BitStream stream, NetworkMessageInfo Info)
	{
		Vector3 syncPosition = Vector3.zero;
		if (stream.isWriting)
		{
			syncPosition = target;
			stream.Serialize(ref syncPosition);
		}
		else
		{
			stream.Serialize(ref syncPosition);
			target = syncPosition;
		}
	}
}
