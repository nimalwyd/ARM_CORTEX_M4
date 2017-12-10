package com.example.rahulpv.dykag1;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;


import android.content.Intent;
import android.os.Bundle;
import android.app.Activity;
import android.app.ListActivity;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.view.Display;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

public class third extends ListActivity {
    Button  btnPostYourEnquiry;
    ImageView imageview1;


    String[] players={"Mr. Fix It Tech LLC","Mr. Fix It Tech LLC","Mr. Fix It Tech LLC","Mr. Fix It Tech LLC","Mr. Fix It Tech LLC"  };
    int[] images={R.drawable.t,R.drawable.t,R.drawable.t,R.drawable.t,R.drawable.t};
    String datas[]={"INTERIOR","INTERIOR","INTERIOR","INTERIOR","INTERIOR","INTERIOR","INTERIOR",};
    String datas1[]={"FIT OUTS","FIT OUTS","FIT OUTS","FIT OUTS","FIT OUTS","FIT OUTS","FIT OUTS",};
    String datas2[]={"REPAIRS","REPAIRS","REPAIRS","REPAIRS","REPAIRS","REPAIRS","REPAIRS",};
    String datas3[]={"TECHNICAL","TECHNICAL","TECHNICAL","TECHNICAL","TECHNICAL","TECHNICAL","TECHNICAL",};
    String datas4[]={"ELECTRICAL","ELECTRICAL","ELECTRICAL","ELECTRICAL","ELECTRICAL","ELECTRICAL","ELECTRICAL",};
    int[] images1={R.drawable.border};


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        setContentView(R.layout.third);

        ListView list = getListView();
        View headerView = View.inflate(this, R.layout.ist_header, null);
        list.addHeaderView(headerView);
        View footerView = View.inflate(this, R.layout.footerview, null);
        list.addFooterView(footerView );



        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);


        //create and setup  adapter
        customadapterclass adapter=new customadapterclass(this, players, datas,datas1,datas2,datas3,datas4,images,images1);
        setListAdapter(adapter);


    }
/*
        listView=(ListView)findViewById(R.id.android:list);

        listView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view,int position, long id) {
                String yourData = temparr.get(position);
            }
        });*/
        /*list.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?>customAdapterView, View view,int position, long id) {

                Intent obj2=new Intent(third.this,fourth.class);
                startActivity(obj2);

            }
        });*/




        /*imageview1=(ImageView)findViewById(R.id.imageview1);
        imageview1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                Intent obj2=new Intent(third.this,fourth.class);
                startActivity(obj2);

            }
        });

*/








    /* listView=(ListView)findViewById(R.id.android_list);
        FrameLayout footerLayout = (FrameLayout) getLayoutInflater().inflate(R.layout.footerview,null);
        btnPostYourEnquiry = (Button) footerLayout.findViewById(R.id.btnGetMoreResults);

        listView.addFooterView(footerLayout);*/
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        // TODO Auto-generated method stub
        Toast.makeText(this, "this", Toast.LENGTH_LONG).show();


    }




}