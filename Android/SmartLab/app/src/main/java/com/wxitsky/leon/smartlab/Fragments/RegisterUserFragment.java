package com.wxitsky.leon.smartlab.Fragments;

import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.Nullable;
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
import com.avos.avoscloud.AVFile;
import com.avos.avoscloud.AVUser;
import com.avos.avoscloud.SignUpCallback;
import com.facebook.common.executors.CallerThreadExecutor;
import com.facebook.common.references.CloseableReference;
import com.facebook.datasource.DataSource;
import com.facebook.drawee.backends.pipeline.Fresco;
import com.facebook.drawee.view.SimpleDraweeView;
import com.facebook.imagepipeline.core.ImagePipeline;
import com.facebook.imagepipeline.datasource.BaseBitmapDataSubscriber;
import com.facebook.imagepipeline.image.CloseableImage;
import com.facebook.imagepipeline.request.ImageRequest;
import com.facebook.imagepipeline.request.ImageRequestBuilder;
import com.wxitsky.leon.smartlab.Event.CardID;
import com.wxitsky.leon.smartlab.R;

import org.greenrobot.eventbus.Subscribe;

import java.io.ByteArrayOutputStream;


public class RegisterUserFragment extends Fragment {

    private Button btnUserRegister;
    private TextInputEditText etUserRealName;
    private TextInputEditText etUserID;
    private TextInputEditText etCardID;
    private TextInputEditText etClassName;
    private TextInputEditText etUserPhoneNo;
    private SimpleDraweeView frescoUserPhoto;
    private String url;
    private AVFile photo;

    public RegisterUserFragment() {
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View root = inflater.inflate(R.layout.fragment_register_user, container, false);
        btnUserRegister = (Button) root.findViewById(R.id.btn_user_register);
        etUserRealName = (TextInputEditText) root.findViewById(R.id.et_user_real_name);
        etUserID = (TextInputEditText) root.findViewById(R.id.et_user_name_id);
        etCardID = (TextInputEditText) root.findViewById(R.id.et_card_id);
        etClassName = (TextInputEditText) root.findViewById(R.id.et_class_name);
        etUserPhoneNo = (TextInputEditText) root.findViewById(R.id.et_user_phone_no);
        frescoUserPhoto = (SimpleDraweeView) root.findViewById(R.id.fresco_user_photo);
        frescoUserPhoto.setContentDescription("照片");
        etUserID.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                if (etUserID.length() == 10) {
                    url = "http://jwgl.wxit.edu.cn/images/xszp/" + etUserID.getText().toString().replace(" ", "") + ".jpg";
                    Uri uri = Uri.parse(url);
                    frescoUserPhoto.setImageURI(uri);
                }
            }
        });
        btnUserRegister.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (etUserID.length() != 10 || etClassName.length() < 1 || etUserRealName.length() < 1 || etUserPhoneNo.length() < 1) {
                    Toast.makeText(getActivity(), "输入有误请检查!", Toast.LENGTH_LONG).show();
                } else {
                    ImageRequest imageRequest = ImageRequestBuilder
                            .newBuilderWithSource(Uri.parse(url))
                            .setProgressiveRenderingEnabled(true)
                            .build();

                    ImagePipeline imagePipeline = Fresco.getImagePipeline();
                    DataSource<CloseableReference<CloseableImage>>
                            dataSource = imagePipeline.fetchDecodedImage(imageRequest, getContext());

                    dataSource.subscribe(new BaseBitmapDataSubscriber() {

                                             @Override
                                             public void onNewResultImpl(@Nullable Bitmap bitmap) {
                                                 photo = new AVFile(etUserID.getText().toString() + ".png", Bitmap2Bytes(bitmap));
                                             }

                                             @Override
                                             public void onFailureImpl(DataSource dataSource) {
                                             }
                                         },
                            CallerThreadExecutor.getInstance());
                    AVUser user = new AVUser();

                    user.setUsername(etUserID.getText().toString().trim());
                    user.setPassword("123456");
                    user.setMobilePhoneNumber(etUserPhoneNo.getText().toString().trim());
                    user.put("studentID", etUserID.getText().toString().trim());
                    user.put("cardID", etCardID.getText().toString().trim());
                    user.put("studentClassName", etClassName.getText().toString().trim());
                    user.put("realName", etUserRealName.getText().toString().trim());
                    user.put("isTeacher", "false");
                    user.put("isAdmin", "false");
                    user.put("photo", photo);
                    user.signUpInBackground(new SignUpCallback() {
                        public void done(AVException e) {
                            if (e == null) {
                                // successfully
                                Toast.makeText(getActivity(), "登记成功!", Toast.LENGTH_SHORT).show();
                            } else {
                                // failed
                                Toast.makeText(getActivity(), "登记失败请重试!", Toast.LENGTH_SHORT).show();
                            }
                        }
                    });
                }
            }
        });
        return root;
    }

    private byte[] Bitmap2Bytes(Bitmap bm) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        bm.compress(Bitmap.CompressFormat.PNG, 100, baos);
        return baos.toByteArray();
    }

    @Subscribe
    public void onMessageEvent(CardID cid) {
        etCardID.setText(cid.getCardid());
    }

    @Override
    public void onStart() {
        super.onStart();
        org.greenrobot.eventbus.EventBus.getDefault().register(this);
    }

    @Override
    public void onStop() {
        org.greenrobot.eventbus.EventBus.getDefault().unregister(this);
        super.onStop();
    }
}
