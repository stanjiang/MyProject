using UnityEngine;
using System.Collections;

public class Rocket : MonoBehaviour
{

    public float moveSpeed = 500.0f;
    float blastSpeed;

	// Use this for initialization
	void Start ()
    {
	
	}
	
	// Update is called once per frame
	void Update ()
    {
        blastSpeed = moveSpeed * 2.0f;
        Vector3 newVelocity = new Vector3(0, 0, blastSpeed);
        rigidbody.velocity = newVelocity;
	}
}
