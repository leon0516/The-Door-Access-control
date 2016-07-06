package com.wxitsky.leon.smartlab.Fragments;


import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.avos.avoscloud.AVException;
import com.avos.avoscloud.AVObject;
import com.avos.avoscloud.AVQuery;
import com.avos.avoscloud.FindCallback;
import com.wxitsky.leon.smartlab.Adapters.HistoryAdapter;
import com.wxitsky.leon.smartlab.R;

import java.util.List;

/**
 * A simple {@link Fragment} subclass.
 */
public class HistoryRecordFragment extends Fragment {


    private RecyclerView mRecyclerView;
    private LinearLayoutManager mLayoutManager;
    private List<AVObject> avObjects;
    private HistoryAdapter mAdapter;

    public HistoryRecordFragment() {
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View root = inflater.inflate(R.layout.fragment_history_record, container, false);
        mRecyclerView = (RecyclerView) root.findViewById(R.id.history_recyclerview);
//创建默认的线性LayoutManager
        mLayoutManager = new LinearLayoutManager(container.getContext());
        mRecyclerView.setLayoutManager(mLayoutManager);
//创建并设置Adapter
        final AVQuery<AVObject> historyquery = new AVQuery<AVObject>("History");
        historyquery.limit(50);
        historyquery.orderByDescending("createdAt");
        historyquery.findInBackground(new FindCallback<AVObject>() {
            @Override
            public void done(List<AVObject> list, AVException e) {
                if (e == null) {
                    avObjects = list;
                    mAdapter = new HistoryAdapter(list);
                    mRecyclerView.setAdapter(mAdapter);
                    historyquery.clearCachedResult();
                }
            }
        });
        return root;
    }

}
