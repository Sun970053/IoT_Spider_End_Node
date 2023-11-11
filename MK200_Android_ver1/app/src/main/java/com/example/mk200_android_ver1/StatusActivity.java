package com.example.mk200_android_ver1;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.widget.TextView;

public class StatusActivity extends AppCompatActivity {

    TextView txtRobotStatusSpace, txtLightStatusSpace, txtActionModeSpace;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_status);

        txtRobotStatusSpace = findViewById(R.id.txtRobotStatusSpace);
        txtLightStatusSpace = findViewById(R.id.txtLightStatusSpace);
        txtActionModeSpace = findViewById(R.id.txtActionModeSpace);

        Intent receiverIntent = getIntent();
        String switchValue = receiverIntent.getStringExtra("SWITCH_SENDER");
        txtRobotStatusSpace.setText(switchValue);
        String lightValue = receiverIntent.getStringExtra("LIGHT_SENDER");
        txtLightStatusSpace.setText(lightValue);
        String modeValue = receiverIntent.getStringExtra("MODE_SENDER");
        txtActionModeSpace.setText(modeValue);

    }
}