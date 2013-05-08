using UnityEngine;
using System.Collections;

public class DoorManager : MonoBehaviour 
{
    public float doorOpenTime = 3.0f;
    public AudioClip doorOpenSound;
    public AudioClip doorShutSound;

    private bool doorIsOpen = false;
    private float doorTimer = 0.0f;
    private GameObject currentDoor;


	// Use this for initialization
	void Start ()
    {
        doorTimer = 0.0f;
	}
	
	// Update is called once per frame
    void Update()
    {
        if (doorIsOpen)
        {
            doorTimer += Time.deltaTime;
        }

        if (doorTimer > doorOpenTime)
        {
            doorTimer = 0.0f;
            Door(false, doorShutSound, "doorshut");
        }
    }

    private void DoorCheck()
    {
        if (!doorIsOpen)
        {
            Door(true, doorOpenSound, "dooropen");
        }
    }
	
    private void Door(bool openCheck, AudioClip aClip, string animName)
    {
        doorIsOpen = openCheck;
        audio.PlayOneShot(aClip);
        transform.parent.animation.Play(animName);
    }


}
