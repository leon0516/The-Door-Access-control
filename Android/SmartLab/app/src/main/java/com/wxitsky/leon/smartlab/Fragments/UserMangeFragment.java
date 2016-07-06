package com.wxitsky.leon.smartlab.Fragments;


import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.avos.avoscloud.AVException;
import com.avos.avoscloud.AVQuery;
import com.avos.avoscloud.AVUser;
import com.avos.avoscloud.FindCallback;
import com.wxitsky.leon.smartlab.Adapters.UserMangeRecyAdapter;
import com.wxitsky.leon.smartlab.R;

import java.util.ArrayList;
import java.util.List;


/**
 * A simple {@link Fragment} subclass.
 */
public class UserMangeFragment extends Fragment {


    private RecyclerView mRecyclerView;
    private LinearLayoutManager mLayoutManager;
    private UserMangeRecyAdapter mAdapter;
    private List<AVUser> user;

    public UserMangeFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View root = inflater.inflate(R.layout.fragment_user_mange, container, false);
        user = new ArrayList<AVUser>();
        mRecyclerView = (RecyclerView) root.findViewById(R.id.my_recycler_view);
//创建默认的线性LayoutManager
        mLayoutManager = new LinearLayoutManager(container.getContext());
        mRecyclerView.setLayoutManager(mLayoutManager);
//创建并设置Adapter
        AVQuery<AVUser> usersquery = new AVQuery<AVUser>("_User");
        usersquery.findInBackground(new FindCallback<AVUser>() {
            @Override
            public void done(List<AVUser> list, AVException e) {
                if (e == null) {
                    user = list;
                    mAdapter = new UserMangeRecyAdapter(list);
                    mRecyclerView.setAdapter(mAdapter);
                }
            }
        });
        return root;
    }

}
