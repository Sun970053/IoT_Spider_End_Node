package com.example.mk200_android_ver1;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class MainActivity extends AppCompatActivity {

    private Button btnStatus, btnSettings, btnNews, btnDataReport, btnMap, btnShutDown;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        btnStatus = findViewById(R.id.btnStatus);
        btnSettings = findViewById(R.id.btnSettings);
        btnNews = findViewById(R.id.btnNews);
        btnDataReport = findViewById(R.id.btnDataReport);
        btnMap = findViewById(R.id.btnMap);
        btnShutDown =findViewById(R.id.btnShutDown);

        btnSettings.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, SettingsActivity.class);
                startActivity(intent);
            }
        });
    }
}