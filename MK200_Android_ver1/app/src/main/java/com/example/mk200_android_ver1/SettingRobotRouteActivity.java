package com.example.mk200_android_ver1;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageManager;
import android.location.Address;
import android.location.Geocoder;
import android.location.Location;
import android.os.Bundle;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.location.FusedLocationProviderClient;
import com.google.android.gms.location.Granularity;
import com.google.android.gms.location.LocationCallback;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationResult;
import com.google.android.gms.location.LocationServices;
import com.google.android.gms.location.LocationSettingsRequest;
import com.google.android.gms.location.Priority;
import com.google.android.gms.location.SettingsClient;
import com.google.android.gms.tasks.OnSuccessListener;

import android.Manifest;
import android.widget.Toast;

import java.io.IOException;
import java.util.List;
import java.util.Locale;

public class SettingRobotRouteActivity extends AppCompatActivity {

    String TAG = "location ";
    private final static int REQUEST_CODE = 1;

    private FusedLocationProviderClient mFusedLocationClient;
    private LocationCallback mLocationCallback;
    private SettingsClient mSettingsClient;
    private LocationRequest mLocationRequest;
    private LocationSettingsRequest mLocationSettingsRequest;
    private Location lastLocation;
    Double d_lat, d_lon;
    String s_lat = "";
    String s_lon = "";
    String fetched_address = "";


    private Context context;
    private EditText editTxtLon1, editTxtLat1, editTxtLon2, editTxtLat2, editTxtLon3, editTxtLat3, editTxtLon4, editTxtLat4;
    private ImageButton imageBtnStop1, imageBtnStop2, imageBtnStop3, imageBtnStop4;
    private Button btnConfirm;

/*
    private FusedLocationProviderClient fusedLocationProviderClient;
    private final static int REQUEST_CODE = 100;
*/


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_setting_robot_route);

        editTxtLon1 = findViewById(R.id.editTxtLon1);
        editTxtLat1 = findViewById(R.id.editTxtLat1);

        editTxtLon2 = findViewById(R.id.editTxtLon2);
        editTxtLat2 = findViewById(R.id.editTxtLat2);

        editTxtLon3 = findViewById(R.id.editTxtLon3);
        editTxtLat3 = findViewById(R.id.editTxtLat3);

        editTxtLon4 = findViewById(R.id.editTxtLon4);
        editTxtLat4 = findViewById(R.id.editTxtLat4);

        imageBtnStop1 = findViewById(R.id.imageBtnStop1);
        imageBtnStop2 = findViewById(R.id.imageBtnStop2);
        imageBtnStop3 = findViewById(R.id.imageBtnStop3);
        imageBtnStop4  = findViewById(R.id.imageBtnStop4);
        
        btnConfirm = findViewById(R.id.btnConfirm);

        imageBtnStop1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                checkLocationPermission();
//                init();
            }
        });

        imageBtnStop2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                checkLocationPermission();
                init();
            }
        });

        imageBtnStop3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                checkLocationPermission();
                init();
            }
        });

        imageBtnStop4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                checkLocationPermission();
                init();
            }
        });
        
        btnConfirm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Toast.makeText(SettingRobotRouteActivity.this, "路徑設定完成", Toast.LENGTH_SHORT).show();
            }
        });
    }

    /**
     * step 1
     * Location permission code
     */
    private void checkLocationPermission() {

        Log.d(TAG, "inside check location");

        if (ContextCompat.checkSelfPermission(SettingRobotRouteActivity.this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {

            if (ActivityCompat.shouldShowRequestPermissionRationale(SettingRobotRouteActivity.this, Manifest.permission.ACCESS_FINE_LOCATION)) {
                ActivityCompat.requestPermissions(SettingRobotRouteActivity.this, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, REQUEST_CODE);
            } else {
                ActivityCompat.requestPermissions(SettingRobotRouteActivity.this, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, REQUEST_CODE);
            }
        }
    }

    /**
     * step 2
     * respond for request code
     */
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case REQUEST_CODE:

                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {

                    if (ContextCompat.checkSelfPermission(SettingRobotRouteActivity.this,
                            Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED) {

                        Toast.makeText(this, "Permission Granted...", Toast.LENGTH_SHORT).show();

                        /**
                         * code for after request for location is granted...
                         */
                        init();
                    }
                }
                return;
        }

    }

    /**
     * step 3
     */
    @SuppressLint("MissingPermission")
    private void startLocationUpdates() {

        mSettingsClient.checkLocationSettings(mLocationSettingsRequest)
                .addOnSuccessListener(locationSettingsResponse -> {
                    Log.d(TAG, "Location settings are OK");
                    mFusedLocationClient.requestLocationUpdates(mLocationRequest, mLocationCallback, Looper.myLooper());
                })
                .addOnFailureListener(e -> {
                    int statusCode = ((ApiException) e).getStatusCode();
                    Log.d(TAG, "inside error -> " + statusCode);
                });
    }

    /**
     * step 4
     */
    public void stopLocationUpdates() {

        mFusedLocationClient
                .removeLocationUpdates(mLocationCallback)
                .addOnCompleteListener(task -> {Log.d(TAG, "stop location updates");});
    }

    /**
     * step 5
     * method for setting text in each EditText
     */
    public void setStopPoint(EditText edtLat, EditText edtLon) {
        edtLat.setText(s_lat);
        edtLon.setText(s_lon);
    }

    /**
     * step 5
     * Receive location
     */
    private void receiveLocation(@NonNull LocationResult locationResult) {

        lastLocation = locationResult.getLastLocation();

        Log.d(TAG, "Latitude : " + lastLocation.getLatitude());
        Log.d(TAG, "Longitude : " + lastLocation.getLongitude());
        Log.d(TAG, "Altitude : " + lastLocation.getAltitude());

        s_lat = String.format(Locale.ROOT, "%.6f", lastLocation.getLatitude());
        s_lon = String.format(Locale.ROOT, "%.6f", lastLocation.getLongitude());

        d_lat = lastLocation.getLatitude();
        d_lon = lastLocation.getLongitude();

        editTxtLat1.setText(s_lat);
        editTxtLon1.setText(s_lon);

        /**
         * code to fetch address from lat lon
         */
        try {
            Geocoder geocoder = new Geocoder(this, Locale.getDefault());
            List<Address> addresses = geocoder.getFromLocation(d_lat, d_lon, 1);

            fetched_address = addresses.get(0).getAddressLine(0);

            Log.d(TAG, "" + fetched_address);

        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    /**
     * step 6
     */
    public void init() {

        mFusedLocationClient = LocationServices.getFusedLocationProviderClient(this);
        mSettingsClient = LocationServices.getSettingsClient(this);
        mLocationCallback = new LocationCallback() {
            @Override
            public void onLocationResult(@NonNull LocationResult locationResult) {
                super.onLocationResult(locationResult);
                receiveLocation(locationResult);
            }
        };

        mLocationRequest = new LocationRequest.Builder(Priority.PRIORITY_HIGH_ACCURACY, 5000)
                .setGranularity(Granularity.GRANULARITY_PERMISSION_LEVEL)
                .setMinUpdateIntervalMillis(500)
                .setMinUpdateDistanceMeters(1)
                .setWaitForAccurateLocation(true)
                .build();

        LocationSettingsRequest.Builder builder = new LocationSettingsRequest.Builder();
        builder.addLocationRequest(mLocationRequest);
        mLocationSettingsRequest = builder.build();
        startLocationUpdates();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        stopLocationUpdates();
    }

}