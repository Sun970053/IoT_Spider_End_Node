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

    private static String SWITCH_PREFS = "switch_prefs";
    private static String SWITCH_ON = "switch_on";
    private static String LIGHT_ON = "switch_light";
    private static String SAVING_MODE_ON = "switch_mode";
    private MaterialSwitch switchOn, switchLight, switchMode;
    private RelativeLayout templateRobotRoute, templateRobotManualMode;
    private SharedPreferences pref;
    private SharedPreferences.Editor editor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        switchOn = findViewById(R.id.switchOn);
        switchLight = findViewById(R.id.switchLight);
        switchMode = findViewById(R.id.switchMode);

        templateRobotRoute = findViewById(R.id.templateRobotRoute);
        templateRobotManualMode = findViewById(R.id.templateRobotManualMode);

        pref = getSharedPreferences(SWITCH_PREFS, MODE_PRIVATE);
        editor = pref.edit();

        boolean isCheckedOn = pref.getBoolean(SWITCH_ON, true);
        switchOn.setChecked(isCheckedOn);
        boolean isCheckedLight = pref.getBoolean(LIGHT_ON, false);
        switchLight.setChecked(isCheckedLight);
        boolean isCheckedMode = pref.getBoolean(SAVING_MODE_ON, false);
        switchMode.setChecked(isCheckedMode);

        switchOn.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

                Intent switchOnIntent = new Intent();

                if (isChecked) {
                    Toast.makeText(SettingsActivity.this, "Robot Turn On", Toast.LENGTH_SHORT).show();
                    editor.putBoolean(SWITCH_ON, true).apply();
                    switchOnIntent.putExtra("SWITCH_SENDER", "開");
                } else {
                    Toast.makeText(SettingsActivity.this, "Robot Turn Off", Toast.LENGTH_SHORT).show();
                    editor.putBoolean(SWITCH_ON, false).apply();
                    switchOnIntent.putExtra("SWITCH_SENDER", "關");
                }
                setResult(1, switchOnIntent);
            }
        });

        switchLight.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

                Intent lightIntent = new Intent();

                if (isChecked) {
                    Toast.makeText(SettingsActivity.this, "Light Turn On", Toast.LENGTH_SHORT).show();
                    editor.putBoolean(LIGHT_ON, true).apply();
                    lightIntent.putExtra("LIGHT_SENDER", "開");
                } else {
                    Toast.makeText(SettingsActivity.this, "Light Turn Off", Toast.LENGTH_SHORT).show();
                    editor.putBoolean(LIGHT_ON, false).apply();
                    lightIntent.putExtra("LIGHT_SENDER", "關");
                }
                setResult(2, lightIntent);
            }
        });

        switchMode.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

                Intent modeIntent = new Intent();

                if (isChecked) {
                    Toast.makeText(SettingsActivity.this, "Power Saving Mode", Toast.LENGTH_SHORT).show();
                    editor.putBoolean(SAVING_MODE_ON, true).apply();
                    modeIntent.putExtra("MODE_SENDER", "省電");
                } else {
                    Toast.makeText(SettingsActivity.this, "Normal Mode", Toast.LENGTH_SHORT).show();
                    editor.putBoolean(SAVING_MODE_ON, false).apply();
                    modeIntent.putExtra("MODE_SENDER", "一般");
                }
                setResult(3, modeIntent);
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