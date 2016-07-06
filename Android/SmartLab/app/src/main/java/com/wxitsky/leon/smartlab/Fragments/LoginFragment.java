package com.wxitsky.leon.smartlab.Fragments;


import android.app.ProgressDialog;
import android.net.Uri;
import android.os.Bundle;
import android.support.design.widget.TextInputEditText;
import android.support.v4.app.Fragment;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Toast;

import com.avos.avoscloud.AVException;
import com.avos.avoscloud.AVUser;
import com.avos.avoscloud.LogInCallback;
import com.facebook.drawee.view.SimpleDraweeView;
import com.wxitsky.leon.smartlab.R;


/**
 * A simple {@link Fragment} subclass.
 */
public class LoginFragment extends Fragment {


    private Button btnLogin;
    private SimpleDraweeView loginUserPhoto;
    private TextInputEditText etUserPhone;
    private TextInputEditText etUserPassword;
    private String url;
    private ProgressDialog progressDialog;

    public LoginFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View root = inflater.inflate(R.layout.fragment_login, container, false);
        btnLogin = (Button) root.findViewById(R.id.btn_user_login);
        loginUserPhoto = (SimpleDraweeView) root.findViewById(R.id.login_user_photo);
        etUserPhone = (TextInputEditText) root.findViewById(R.id.et_login_user_name_id);
        etUserPassword = (TextInputEditText) root.findViewById(R.id.et_login_user_password);

        etUserPhone.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable s) {
                if (etUserPhone.length() == 10) {
                    url = "http://jwgl.wxit.edu.cn/images/xszp/" + etUserPhone.getText().toString().replace(" ", "") + ".jpg";
                    Uri uri = Uri.parse(url);
                    loginUserPhoto.setImageURI(uri);
                }
            }
        });
        btnLogin.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (etUserPhone.length() == 10 && etUserPassword.length() > 0) {
                    progressDialog = new ProgressDialog(getContext());
                    progressDialog.setMessage("正在登录中请稍等...");    //设置说明文字
                    progressDialog.setIndeterminate(true);    //设置进度条是否为不明确(来回旋转)
                    progressDialog.setCanceledOnTouchOutside(false);    //设置点击屏幕不消失
                    progressDialog.setCancelable(true);    //设置进度条是否可以按退回键取消
                    progressDialog.show();
                    AVUser user = new AVUser();
                    user.logInInBackground(etUserPhone.getText().toString(), etUserPassword.getText().toString(), new LogInCallback<AVUser>() {
                        @Override
                        public void done(AVUser avUser, AVException e) {
                            if (e == null) {
                                progressDialog.dismiss();
                                Toast.makeText(getContext(), "登录成功!", Toast.LENGTH_SHORT).show();

                            } else {
                                progressDialog.dismiss();
                                Toast.makeText(getContext(), "登录失败!", Toast.LENGTH_SHORT).show();
                            }
                        }
                    });
                } else {

                    Toast.makeText(getContext(), "输入有误请检查!", Toast.LENGTH_SHORT).show();
                }
            }
        });
        return root;
    }

}
