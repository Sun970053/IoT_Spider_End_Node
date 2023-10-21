package com.example.mk200_android_ver1;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;
import android.widget.Switch;

import com.google.android.material.switchmaterial.SwitchMaterial;

public class SettingsActivity extends AppCompatActivity {

    private SwitchMaterial switchOn, switchLight, switchMode;
    private RelativeLayout templateRobotRoute, templateRobotManualMode;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        switchOn = findViewById(R.id.switchOn);
        switchLight = findViewById(R.id.switchLight);
        switchMode = findViewById(R.id.switchMode);

        templateRobotRoute = findViewById(R.id.templateRobotRoute);
        templateRobotManualMode = findViewById(R.id.templateRobotManualMode);

        templateRobotRoute.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(SettingsActivity.this, SettingRobotRouteActivity.class);
                startActivity(intent);
            }
        });

        templateRobotManualMode.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(SettingsActivity.this, RobotManualModeActivity.class);
                startActivity(intent);
            }
        });
    }
}