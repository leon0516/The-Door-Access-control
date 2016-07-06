package com.wxitsky.leon.smartlab.Event;

/**
 * Project Name:SmartLab
 * Created by Leon on 2016-4-3-0003 17:56.
 */
public class LoginSuccessInfo {
    private boolean isAdmin;
    private boolean isTeacher;

    public boolean isAdmin() {
        return isAdmin;
    }

    public void setIsAdmin(boolean isAdmin) {
        this.isAdmin = isAdmin;
    }

    public boolean isTeacher() {
        return isTeacher;
    }

    public void setIsTeacher(boolean isTeacher) {
        this.isTeacher = isTeacher;
    }
}
