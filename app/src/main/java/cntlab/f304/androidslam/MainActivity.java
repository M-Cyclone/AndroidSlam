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

        String[] check_list = new String[]
        {
                Manifest.permission.CAMERA,
                //Manifest.permission.MANAGE_EXTERNAL_STORAGE,
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE,
        };

        for (String s : check_list)
        {
            int check = ContextCompat.checkSelfPermission(this, s);
            if (check == PackageManager.PERMISSION_DENIED)
            {
                String[] required_list = new String[]{s};
                ActivityCompat.requestPermissions(this, required_list, 5);
            }
        }


        setContentView(R.layout.activity_main);
    }
}