package com.wxitsky.leon.smartlab.Fragments;


import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.avos.avoscloud.AVException;
import com.avos.avoscloud.AVObject;
import com.avos.avoscloud.AVQuery;
import com.avos.avoscloud.FindCallback;
import com.wxitsky.leon.smartlab.R;

import java.util.List;

/**
 * A simple {@link Fragment} subclass.
 */
public class MainFragment extends Fragment {

    private TextView tv_humidity;
    private TextView tv_temperature;

    public MainFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View root = inflater.inflate(R.layout.fragment_main, container, false);

        tv_humidity = (TextView) root.findViewById(R.id.tv_realtime_humidity);
        tv_temperature = (TextView) root.findViewById(R.id.tv_realtime_temperature);
        final AVQuery<AVObject> query = new AVQuery<AVObject>("EnvironmentInfo");
        query.setLimit(1);
        query.orderByDescending("createdAt");
        query.findInBackground(new FindCallback<AVObject>() {
            public void done(List<AVObject> avObjects, AVException e) {
                if (e == null) {
                    Log.d("成功", "查询到" + avObjects.size() + " 条符合条件的数据");
                    tv_humidity.setText(avObjects.get(0).getString("humidity"));
                    tv_temperature.setText(avObjects.get(0).getString("temperature"));
                    query.clearCachedResult();
                } else {
                    Log.d("失败", "查询错误: " + e.getMessage());
                }
            }
        });
        return root;
    }
}
