package com.example.mk200_android_ver1;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    private TextView txtRobotStatusSpace, txtLightStatusSpace, txtActionModeSpace;
    private Button btnStatus, btnSettings, btnNews, btnDataReport, btnMap, btnShutDown;
    ActivityResultLauncher<Intent> activityResultLauncher = registerForActivityResult(
            new ActivityResultContracts.StartActivityForResult(),
            new ActivityResultCallback<ActivityResult>() {
                @Override
                public void onActivityResult(ActivityResult o) {
                    Log.d(TAG, "onActivityResult: ");

                    if (o.getResultCode() == 1) {
                        Intent intent = o.getData();

                        if (intent != null) {
                            String Data = intent.getStringExtra("SWITCH_SENDER");
                            txtRobotStatusSpace.setText(Data);
                        }
                    }
                    if (o.getResultCode() == 2) {
                        Intent intent = o.getData();

                        if (intent != null) {
                            String Data = intent.getStringExtra("LIGHT_SENDER");
                            txtLightStatusSpace.setText(Data);
                        }
                    }
                    if (o.getResultCode() == 3) {
                        Intent intent = o.getData();

                        if (intent != null) {
                            String Data = intent.getStringExtra("MODE_SENDER");
                            txtActionModeSpace.setText(Data);
                        }
                    }
                }
            }
    );

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        txtRobotStatusSpace = findViewById(R.id.txtRobotStatusSpace);
        txtLightStatusSpace = findViewById(R.id.txtLightStatusSpace);
        txtActionModeSpace = findViewById(R.id.txtActionModeSpace);

        btnStatus = findViewById(R.id.btnStatus);
        btnSettings = findViewById(R.id.btnSettings);
        btnNews = findViewById(R.id.btnNews);
        btnDataReport = findViewById(R.id.btnDataReport);
        btnMap = findViewById(R.id.btnMap);
        btnShutDown =findViewById(R.id.btnShutDown);

//        Intent receiverIntent = getIntent();
//        String switchValue = receiverIntent.getStringExtra("SWITCH_SENDER");
//        txtRobotStatusSpace.setText(switchValue);
//        String lightValue = receiverIntent.getStringExtra("LIGHT_SENDER");
//        txtLightStatusSpace.setText(lightValue);
//        String modeValue = receiverIntent.getStringExtra("MODE_SENDER");
//        txtActionModeSpace.setText(modeValue);

        btnStatus.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, StatusActivity.class);
                startActivity(intent);
            }
        });

        btnSettings.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, SettingsActivity.class);
                activityResultLauncher.launch(intent);
//                startActivity(intent);
            }
        });
    }
}