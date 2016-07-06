package com.wxitsky.leon.smartlab.Adapters;

import android.net.Uri;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.avos.avoscloud.AVFile;
import com.avos.avoscloud.AVObject;
import com.facebook.drawee.view.SimpleDraweeView;
import com.wxitsky.leon.smartlab.R;

import java.text.SimpleDateFormat;
import java.util.List;

/**
 * Project Name:SmartLab
 * Created by Leon on 2016-4-3-0003 19:19.
 */
public class HistoryAdapter extends RecyclerView.Adapter<HistoryAdapter.ViewHolder> {
    private List<AVObject> avObjects;

    public HistoryAdapter(List<AVObject> his) {
        avObjects = his;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_history_recycler_view, parent, false);
        ViewHolder vh = new ViewHolder(view);
        return vh;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        holder.tv_student_id.setText(avObjects.get(position).getString("username"));
        SimpleDateFormat time = new SimpleDateFormat("yyyy/MM/dd  HH:mm:ss");
        holder.tv_time.setText(time.format(avObjects.get(position).getCreatedAt()));
        AVFile af =avObjects.get(position).getAVFile("pic");
        Uri u = Uri.parse(af.getUrl());
        holder.pic.setImageURI(u);
    }

    @Override
    public int getItemCount() {
        return avObjects.size();
    }

    public class ViewHolder extends RecyclerView.ViewHolder {
        public TextView tv_student_id;
        public TextView tv_time;
        SimpleDraweeView pic;

        public ViewHolder(View itemView) {
            super(itemView);
            tv_student_id = (TextView) itemView.findViewById(R.id.tv_item_list_student_id);
            tv_time = (TextView) itemView.findViewById(R.id.tv_item_list_time);
            pic = (SimpleDraweeView) itemView.findViewById(R.id.fresco_item_photo);

        }
    }
}
