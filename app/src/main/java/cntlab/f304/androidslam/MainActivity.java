package cntlab.f304.androidslam;

import android.Manifest;
import android.app.NativeActivity;
import android.content.pm.PackageManager;
import android.os.Bundle;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class MainActivity extends NativeActivity
{
    @Override
    protected void onCreate(Bundle state)
    {
        super.onCreate(state);

        int check = ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA);
        if(check == PackageManager.PERMISSION_DENIED)
        {
            String[] required_list = new String[] { Manifest.permission.CAMERA };
            ActivityCompat.requestPermissions(this, required_list, 5);
        }

        setContentView(R.layout.activity_main);
    }
}