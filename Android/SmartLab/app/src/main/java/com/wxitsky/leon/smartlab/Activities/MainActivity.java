package com.wxitsky.leon.smartlab.Activities;

import android.app.PendingIntent;
import android.content.Intent;
import android.content.IntentFilter;
import android.nfc.NfcAdapter;
import android.nfc.tech.IsoDep;
import android.nfc.tech.MifareClassic;
import android.nfc.tech.MifareUltralight;
import android.nfc.tech.Ndef;
import android.nfc.tech.NfcA;
import android.nfc.tech.NfcB;
import android.nfc.tech.NfcF;
import android.nfc.tech.NfcV;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.design.widget.NavigationView;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

import com.avos.avoscloud.AVUser;
import com.facebook.drawee.view.SimpleDraweeView;
import com.wxitsky.leon.smartlab.Event.CardID;
import com.wxitsky.leon.smartlab.Event.LoginSuccessInfo;
import com.wxitsky.leon.smartlab.Fragments.HistoryRecordFragment;
import com.wxitsky.leon.smartlab.Fragments.MainFragment;
import com.wxitsky.leon.smartlab.Fragments.LoginFragment;
import com.wxitsky.leon.smartlab.Fragments.SmartConfigFragment;
import com.wxitsky.leon.smartlab.R;
import com.wxitsky.leon.smartlab.Fragments.RegisterUserFragment;
import com.wxitsky.leon.smartlab.Fragments.UserMangeFragment;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;


public class MainActivity extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener {

    private FragmentManager fragmentManager;
    private Boolean isAdmin = false;
    private Boolean isTeacher = false;
    SimpleDraweeView headimage;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(
                this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
        drawer.setDrawerListener(toggle);
        NavigationView navigationView = (NavigationView) findViewById(R.id.nav_view);
        toggle.syncState();
        AVUser user = AVUser.getCurrentUser();
        if (user != null) {
            String chaeckadmin = user.getString("isAdmin");
            String chaeckteacher = user.getString("isTeacher");
            if (chaeckadmin.equals("true")) {
                isAdmin = true;
            }
            if (chaeckteacher.equals("true"))
                isTeacher = true;
        }

        navigationView.inflateMenu(R.menu.activity_main_drawer_admin);
        navigationView.setNavigationItemSelectedListener(this);
        if (savedInstanceState == null) {
            fragmentManager = getSupportFragmentManager();
            fragmentManager.beginTransaction().add(R.id.mainframentactivity, new MainFragment()).commit();
            setTitle(R.string.nvr_menu_string_dashboard);
        }

    }

    @Override
    public void onBackPressed() {
        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else {
            super.onBackPressed();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }


    @SuppressWarnings("StatementWithEmptyBody")
    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        // Handle navigation view item clicks here.
        int id = item.getItemId();
        Fragment fragment = null;
        if (id == R.id.nav_camera) {
            fragment = new MainFragment();
        } else if (id == R.id.nav_gallery) {
            fragment = new RegisterUserFragment();
        } else if (id == R.id.nav_slideshow) {
            fragment = new LoginFragment();
        } else if (id == R.id.nav_manage) {
            fragment = new UserMangeFragment();
        } else if (id == R.id.nav_share) {
            fragment = new SmartConfigFragment();
        } else if (id == R.id.nav_send) {
            fragment = new HistoryRecordFragment();
        }
        fragmentManager.beginTransaction().replace(R.id.mainframentactivity, fragment).commit();
        setTitle(item.getTitle());
        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        drawer.closeDrawer(GravityCompat.START);
        return true;
    }

    private String ByteArrayToHexString(byte[] inarray) {
        int i, j, in;
        String[] hex = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};
        String out = "";

        for (j = 0; j < inarray.length; ++j) {
            in = (int) inarray[j] & 0xff;
            i = (in >> 4) & 0x0f;
            out += hex[i];
            i = in & 0x0f;
            out += hex[i];
        }
        return out;
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (isAdmin || isTeacher) {
            // creating pending intent:
            PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, new Intent(this, getClass()).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP), 0);
            // creating intent receiver for NFC events:
            IntentFilter filter = new IntentFilter();
            filter.addAction(NfcAdapter.ACTION_TAG_DISCOVERED);
            filter.addAction(NfcAdapter.ACTION_NDEF_DISCOVERED);
            filter.addAction(NfcAdapter.ACTION_TECH_DISCOVERED);
            // enabling foreground dispatch for getting intent from NFC event:
            NfcAdapter nfcAdapter = NfcAdapter.getDefaultAdapter(this);
            if (nfcAdapter != null) {
                nfcAdapter.enableForegroundDispatch(this, pendingIntent, new IntentFilter[]{filter}, this.techList);
            }
        }
    }

    // list of NFC technologies detected:
    private final String[][] techList = new String[][]{
            new String[]{
                    NfcA.class.getName(),
                    NfcB.class.getName(),
                    NfcF.class.getName(),
                    NfcV.class.getName(),
                    IsoDep.class.getName(),
                    MifareClassic.class.getName(),
                    MifareUltralight.class.getName(), Ndef.class.getName()
            }
    };

    @Override
    protected void onNewIntent(Intent intent) {
        if (isAdmin || isTeacher) {
            if (intent.getAction().equals(NfcAdapter.ACTION_TAG_DISCOVERED)) {
                Log.e("card ID", ByteArrayToHexString(intent.getByteArrayExtra(NfcAdapter.EXTRA_ID)));
                EventBus.getDefault().post(new CardID(ByteArrayToHexString(intent.getByteArrayExtra(NfcAdapter.EXTRA_ID))));
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (isAdmin || isTeacher) {
            // disabling foreground dispatch:
            NfcAdapter nfcAdapter = NfcAdapter.getDefaultAdapter(this);
            if (nfcAdapter != null)
                nfcAdapter.disableForegroundDispatch(this);
        }
    }

    @Subscribe
    public void onMessageEvent(LoginSuccessInfo loginSuccessInfo) {
        //TODO 添加登录成功事件处理!
    }

//    @Override
//    public void onStart() {
//        super.onStart();
//        EventBus.getDefault().register(this);
//    }
//
//    @Override
//    public void onStop() {
//        EventBus.getDefault().unregister(this);
//        super.onStop();
//    }
}
