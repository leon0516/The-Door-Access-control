package com.wxitsky.leon.smartlab.Application;

import android.app.Application;

import com.avos.avoscloud.AVOSCloud;
import com.facebook.drawee.backends.pipeline.Fresco;


/**
 * Project Name:SmartLab
 * Created by Leon on 2016-3-28-0028 19:44.
 */
public class SmartLab extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        AVOSCloud.initialize(this, "YI5qI01Wuzqbtz4tnXtXthMU-gzGzoHsz", "0k72vg1QeqSaB4fT0viGm119");
        Fresco.initialize(this);
    }
}
