package com.wxitsky.leon.smartlab.Fragments;


import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.design.widget.TextInputEditText;
import android.support.v4.app.Fragment;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Switch;
import android.widget.Toast;

import com.wxitsky.leon.smartlab.EspWifiAdminSimple;
import com.wxitsky.leon.smartlab.EsptouchTask;
import com.wxitsky.leon.smartlab.IEsptouchListener;
import com.wxitsky.leon.smartlab.IEsptouchResult;
import com.wxitsky.leon.smartlab.IEsptouchTask;
import com.wxitsky.leon.smartlab.R;
import com.wxitsky.leon.smartlab.task.__IEsptouchTask;

import java.util.List;

/**
 * A simple {@link Fragment} subclass.
 */
public class SmartConfigFragment extends Fragment {

    private static final String TAG = "SmartConfigFragment";
    private TextInputEditText mTvApSsid;
    private TextInputEditText mEdtApPassword;
    private Button mBtnConfirm;
    private Switch mSwitchIsSsidHidden;
    private EspWifiAdminSimple mWifiAdmin;

    public SmartConfigFragment() {
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // 初始化界面
        View root = inflater.inflate(R.layout.fragment_smart_config, container, false);
        mWifiAdmin = new EspWifiAdminSimple(getContext());
        mTvApSsid = (TextInputEditText) root.findViewById(R.id.tvApSssidConnected);
        mEdtApPassword = (TextInputEditText) root.findViewById(R.id.edtApPassword);
        mBtnConfirm = (Button) root.findViewById(R.id.btnConfirm);
        mSwitchIsSsidHidden = (Switch) root.findViewById(R.id.switchIsSsidHidden);
        mBtnConfirm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String apSsid = mTvApSsid.getText().toString();
                String apPassword = mEdtApPassword.getText().toString();
                String apBssid = mWifiAdmin.getWifiConnectedBssid();
                Boolean isSsidHidden = mSwitchIsSsidHidden.isChecked();
                String isSsidHiddenStr = "NO";
                String taskResultCountStr = "1";
                if (isSsidHidden) {
                    isSsidHiddenStr = "YES";
                }
                if (__IEsptouchTask.DEBUG) {
                    Log.d(TAG, "mBtnConfirm is clicked, mEdtApSsid = " + apSsid
                            + ", " + " mEdtApPassword = " + apPassword);
                }
                new EsptouchAsyncTask3().execute(apSsid, apBssid, apPassword,
                        isSsidHiddenStr, taskResultCountStr);
            }
        });
        return root;
    }


    @Override
    public void onResume() {
        super.onResume();
        // 显示已经连接的SSID
        String apSsid = mWifiAdmin.getWifiConnectedSsid();
        if (apSsid != null) {
            mTvApSsid.setText(apSsid);
        } else {
            mTvApSsid.setText("");
        }
        // 检查是否连接上WIFI
        boolean isApSsidEmpty = TextUtils.isEmpty(apSsid);
        mBtnConfirm.setEnabled(!isApSsidEmpty);
    }

    private void onEsptoucResultAddedPerform(final IEsptouchResult result) {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                String text = "设备已成功连上WIFI!";
                Toast.makeText(getContext(), text,
                        Toast.LENGTH_LONG).show();
            }
        });
    }

    private IEsptouchListener myListener = new IEsptouchListener() {

        @Override
        public void onEsptouchResultAdded(final IEsptouchResult result) {
            onEsptoucResultAddedPerform(result);
        }
    };

    private class EsptouchAsyncTask3 extends AsyncTask<String, Void, List<IEsptouchResult>> {

        private ProgressDialog mProgressDialog;

        private IEsptouchTask mEsptouchTask;
        private final Object mLock = new Object();

        @Override
        protected void onPreExecute() {
            mProgressDialog = new ProgressDialog(getContext());
            mProgressDialog
                    .setMessage("请稍等! 配置中...");
            mProgressDialog.setCanceledOnTouchOutside(false);
            mProgressDialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
                @Override
                public void onCancel(DialogInterface dialog) {
                    synchronized (mLock) {
                        if (__IEsptouchTask.DEBUG) {
                            Log.i(TAG, "progress dialog is canceled");
                        }
                        if (mEsptouchTask != null) {
                            mEsptouchTask.interrupt();
                        }
                    }
                }
            });
            mProgressDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    "Waiting...", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                        }
                    });
            mProgressDialog.show();
            mProgressDialog.getButton(DialogInterface.BUTTON_POSITIVE)
                    .setEnabled(false);
        }

        @Override
        protected List<IEsptouchResult> doInBackground(String... params) {
            int taskResultCount = -1;
            synchronized (mLock) {
                String apSsid = params[0];
                String apBssid = params[1];
                String apPassword = params[2];
                String isSsidHiddenStr = params[3];
                String taskResultCountStr = params[4];
                boolean isSsidHidden = false;
                if (isSsidHiddenStr.equals("YES")) {
                    isSsidHidden = true;
                }
                taskResultCount = Integer.parseInt(taskResultCountStr);
                mEsptouchTask = new EsptouchTask(apSsid, apBssid, apPassword,
                        isSsidHidden, getContext());
                mEsptouchTask.setEsptouchListener(myListener);
            }
            return mEsptouchTask.executeForResults(taskResultCount);
        }

        @Override
        protected void onPostExecute(List<IEsptouchResult> result) {
            mProgressDialog.getButton(DialogInterface.BUTTON_POSITIVE)
                    .setEnabled(true);
            mProgressDialog.getButton(DialogInterface.BUTTON_POSITIVE).setText(
                    "确认");
            IEsptouchResult firstResult = result.get(0);
            // 检查任务是否取消
            if (!firstResult.isCancelled()) {
                int count = 0;
                final int maxDisplayCount = 5;
                if (firstResult.isSuc()) {
                    StringBuilder sb = new StringBuilder();
                    for (IEsptouchResult resultInList : result) {
                        sb.append("配置成功!\n\n" + "设备IP地址是:").append(resultInList.getInetAddress()
                                .getHostAddress());
                        count++;
                        if (count >= maxDisplayCount) {
                            break;
                        }
                    }
                    if (count < result.size()) {
                        sb.append("\nthere's ").append(result.size() - count).append(" more result(s) without showing\n");
                    }
                    mProgressDialog.setMessage(sb.toString());
                } else {
                    mProgressDialog.setMessage("配置失败!");
                }
            }
        }
    }
}
