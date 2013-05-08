using UnityEngine;
using System.Collections;

public class Inventory : MonoBehaviour
{
    public static int charge = 0;

    public AudioClip collectSound;
    public Texture2D[] hudCharge;
    public GUITexture chargeHudGUI;

    public Texture2D[] meterCharge;
    public Renderer meter;

	// Use this for initialization
	void Start ()
    {
        charge = 0;
        meter.material.mainTexture = meterCharge[0];
	}
	
	// Update is called once per frame
	void Update ()
    {
	
	}

    private void CellPickup()
    {
        HUDon();
        AudioSource.PlayClipAtPoint(collectSound, transform.position);
        ++charge;
        chargeHudGUI.texture = hudCharge[charge];
        meter.material.mainTexture = meterCharge[charge];
    }

    private void HUDon()
    {
        if (!chargeHudGUI.enabled)
        {
            chargeHudGUI.enabled = true;
        }
    }

}
