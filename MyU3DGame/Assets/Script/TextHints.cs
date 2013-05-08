using UnityEngine;
using System.Collections;

public class TextHints : MonoBehaviour
{
    private float timer = 0.0f;

	// Use this for initialization
	void Start ()
    {
	
	}
	
	// Update is called once per frame
	void Update ()
    {
        if (guiText.enabled)
        {
            timer += Time.deltaTime;
            if (timer >= 4)
            {
                guiText.enabled = false;
                timer = 0.0f;
            }
        }
	
	}

    private void ShowHint(string msg)
    {
        guiText.text = msg;
        if (!guiText.enabled)
        {
            guiText.enabled = true;
        }
    }
}
