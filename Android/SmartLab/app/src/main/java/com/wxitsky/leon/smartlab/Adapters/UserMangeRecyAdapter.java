package com.wxitsky.leon.smartlab.Adapters;

import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.avos.avoscloud.AVUser;
import com.wxitsky.leon.smartlab.R;

import java.util.List;

/**
 * Project Name:SmartLab
 * Created by Leon on 2016-4-3-0003 19:19.
 */
public class UserMangeRecyAdapter extends RecyclerView.Adapter<UserMangeRecyAdapter.ViewHolder> {
    private List<AVUser> mUser;

    public UserMangeRecyAdapter(List<AVUser> user) {
        mUser = user;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_user_mange_recycler_view, parent, false);
        ViewHolder vh = new ViewHolder(view);
        return vh;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        holder.tv_mane.setText(mUser.get(position).getString("realName"));
        holder.tv_id.setText(mUser.get(position).getUsername());
        holder.tv_class.setText(mUser.get(position).getString("studentClassName"));
    }

    @Override
    public int getItemCount() {
        return mUser.size();
    }

    public class ViewHolder extends RecyclerView.ViewHolder {
        public TextView tv_mane;
        public TextView tv_id;
        public TextView tv_class;

        public ViewHolder(View itemView) {
            super(itemView);
            tv_class = (TextView) itemView.findViewById(R.id.tv_list_item_user_class);
            tv_id = (TextView) itemView.findViewById(R.id.tv_list_item_user_id);
            tv_mane = (TextView) itemView.findViewById(R.id.tv_list_item_user_name);

        }
    }
}
