using UnityEngine;
using System.Collections;

public class TriggerZone : MonoBehaviour 
{
    public AudioClip lockedSound;
    public Light doorLight;
    public GUIText textHints;


	// Use this for initialization
	void Start ()
    {
        textHints.text = "";
	}
	
	// Update is called once per frame
	void Update () {
	
	}

    void OnTriggerEnter(Collider col)
    {
        if (col.gameObject.tag == "Player")
        {
            if (Inventory.charge == 4)
            {
                transform.FindChild("door").SendMessage("DoorCheck");
                GameObject guiObj = GameObject.Find("PowerGUI");
                if (guiObj != null)
                {
                    Destroy(guiObj);
                    doorLight.color = Color.green;
                }
            }
            else
            {
                transform.FindChild("door").audio.PlayOneShot(lockedSound);
                col.gameObject.SendMessage("HUDon");
                textHints.SendMessage("ShowHint", "This door seems locked, maybe that generator needs power...");
            }
            
        }
    }

}
