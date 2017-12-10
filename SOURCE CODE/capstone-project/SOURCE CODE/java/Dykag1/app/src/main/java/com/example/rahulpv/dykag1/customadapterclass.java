package com.example.rahulpv.dykag1;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class customadapterclass extends ArrayAdapter<String>{

    Context c;
    String[] players;
    String[] datas;
    String[] datas1;
    String[] datas2;
    String[] datas3;
    String[] datas4;
    int[] images;
    int[] images1;
    LayoutInflater inflater;






    public customadapterclass(Context context, String[] players,String[] datas ,String[] datas1,String[] datas2,String[] datas3,String[] datas4, int[] images, int[] images1) {
        super(context, R.layout.rowmodel,players);
        // TODO Auto-generated constructor stub


        this.c=context;
        this.players=players;
        this.images=images;
        this.images1=images1;
        this.datas=datas;
        this.datas1=datas1;
        this.datas2=datas2;
        this.datas3=datas3;
        this.datas4=datas4;

    }




    //this inner class holds views
    public class viewholder{

        TextView nametv;
        ImageView img;
        ImageView img1;
        TextView nametv1;
        TextView nametv2;
        TextView nametv3;
        TextView nametv4;
        TextView nametv5;

    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        // TODO Auto-generated method stub
        // check if view is null if so create it
        if(convertView==null){
            inflater=(LayoutInflater) c.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView=inflater.inflate(R.layout.rowmodel, null);

        }
        //otherwise
        viewholder holder =new viewholder();
        //initialize views
        holder.nametv=(TextView) convertView.findViewById(R.id.nametv);
        holder.nametv1=(TextView) convertView.findViewById(R.id.nametv1);
        holder.nametv2=(TextView) convertView.findViewById(R.id.nametv2);
        holder.nametv3=(TextView) convertView.findViewById(R.id.nametv3);
        holder.nametv4=(TextView) convertView.findViewById(R.id.nametv4);
        holder.nametv5=(TextView) convertView.findViewById(R.id.nametv5);
        holder.img=(ImageView) convertView.findViewById(R.id.imageview1);
        holder.img1=(ImageView) convertView.findViewById(R.id.imageview2);
        //assign them the data
        holder.nametv.setText(players[position]);
        holder.nametv1.setText(datas[position]);
        holder.nametv2.setText(datas1[position]);
        holder.nametv3.setText(datas2[position]);
        holder.nametv4.setText(datas3[position]);
        holder.nametv5.setText(datas4[position]);
        holder.img.setImageResource(images[position]);
        holder.img1.setImageResource(images1[0]);

        return convertView;
    }
}
