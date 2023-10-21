package com.example.mk200_android_ver1;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.RelativeLayout;
import android.widget.Switch;
import android.widget.Toast;

import com.google.android.material.materialswitch.MaterialSwitch;
import com.google.android.material.switchmaterial.SwitchMaterial;

public class SettingsActivity extends AppCompatActivity {

    private MaterialSwitch switchOn, switchLight, switchMode;
    private RelativeLayout templateRobotRoute, templateRobotManualMode;
    private SharedPreferences sharedPreferences;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        switchOn = findViewById(R.id.switchOn);
        switchLight = findViewById(R.id.switchLight);
        switchMode = findViewById(R.id.switchMode);

        templateRobotRoute = findViewById(R.id.templateRobotRoute);
        templateRobotManualMode = findViewById(R.id.templateRobotManualMode);

        sharedPreferences = getSharedPreferences("switchOn", MODE_PRIVATE);
        boolean isCheckedOn = sharedPreferences.getBoolean("switchOn", true);
        switchOn.setChecked(isCheckedOn);

        switchOn.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    Toast.makeText(SettingsActivity.this, "Robot Turn On", Toast.LENGTH_SHORT).show();
                    sharedPreferences.edit().putBoolean("switchOn", true).apply();
                } else {
                    Toast.makeText(SettingsActivity.this, "Robot Turn Off", Toast.LENGTH_SHORT).show();
                    sharedPreferences.edit().putBoolean("switchOn", false).apply();
                }
            }
        });

        switchLight.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    Toast.makeText(SettingsActivity.this, "Light Turn On", Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(SettingsActivity.this, "Light Turn Off", Toast.LENGTH_SHORT).show();
                }
            }
        });

        switchMode.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    Toast.makeText(SettingsActivity.this, "Power Saving Mode", Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(SettingsActivity.this, "Normal Mode", Toast.LENGTH_SHORT).show();
                }
            }
        });

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