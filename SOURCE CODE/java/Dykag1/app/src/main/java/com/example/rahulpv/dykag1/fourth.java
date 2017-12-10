package com.example.rahulpv.dykag1;

import android.app.Activity;
import android.os.Bundle;
import android.text.Html;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by Rahulpv on 5/13/2016.
 */
public class fourth  extends Activity implements AdapterView.OnItemSelectedListener {
    Spinner spinnerloc;
    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {

    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {

    }

    TextView tet;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.fourth);
       spinnerloc=(Spinner)findViewById(R.id.spinnerloc);
        List<String> list = new ArrayList<String>();
        list.add("location");
        ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, list);
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinnerloc.setAdapter(dataAdapter);






    }
}
