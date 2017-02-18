package com.noctuasoftware.stellarium;

import android.app.Activity;
import android.util.DisplayMetrics;
import android.util.Log;
import android.os.Bundle;
import android.view.Display;

public class Stellarium
{
    private Activity m_activity;
    private static String TAG = "Stellarium";
    public static boolean canPause = false;

    public Stellarium(Activity activity) {
        m_activity = activity;
    }

    public void setCanPause(boolean value) {
        canPause = value;
    }

    public String getModel() {
        return android.os.Build.MANUFACTURER + ":" + android.os.Build.MODEL;
    }

    public float getScreenDensity() {
        DisplayMetrics metrics = new DisplayMetrics();
        m_activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
        return metrics.density * 160;
    }

    public int getRotation() {
        Display display = m_activity.getWindowManager().getDefaultDisplay();
        return display.getRotation();
    }
};

