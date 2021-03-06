package io.boden.android;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.Bundle;
import androidx.fragment.app.FragmentActivity;
import android.view.MenuItem;

import java.lang.reflect.Field;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.EventListener;
import java.util.Vector;

/** An activity that is controlled by native code.
 *
 *  You can directly specify this activity in your AndroidManifest.xml.
 *
 *  The activity will load the native code library and initialize everything to allow
 *  the native code size to control the contents and behaviour of the activity.
 *
 *  In your manifest you can specify a meta value called "android.app.lib_name"
 *  to control the name of the native library that the activity will load. If no lib_name
 *  is given then the activity will try to load a lib called "main".
 *
 *
 * */
public class NativeRootActivity extends FragmentActivity
{
    public interface BackButtonListener extends EventListener {
        boolean backButtonPressed();
    }

    class BackButtonHandlers {
        final Vector<BackButtonListener> listener = new Vector<>();

        void add(BackButtonListener a) {
            listener.addElement(a);
        }

        void remove(BackButtonListener l) {
            listener.remove(l);
        }

        boolean handleBackButtonPressed() {
            for(int i=0; i < listener.size(); i++) {
                if ((listener.elementAt(i)).backButtonPressed()) {
                    return true;
                }
            }
            return false;
        }
    }

    /** Name of the meta variable that can be used in the manifest to specify the
     *  name of the native code library.
     *
     *  If this is not specified then "main" is used.
     */
    private static final String META_DATA_LIB_NAME = "io.boden.android.lib_name";

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        _rootActivity = this;
        _backButtonHandlers = new BackButtonHandlers();

        super.onCreate(savedInstanceState);

        loadMetaData();

        _libName = getMetaString(META_DATA_LIB_NAME, "main");

        NativeInit.baseInit(_libName);

        _rootView = new NativeRootView(this );
        setContentView(_rootView);

        nativeRegisterAppContext(this);

        NativeInit.launch( getIntent() );
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                onBackPressed();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    public int getResourceIdFromURI(String uri, String type) {
        try {
            URI aUri = new URI(uri);

            String path = aUri.getPath();

            path = path.replace('/', '_');
            path = path.replace('.', '_');

            path = path.substring(1); // Remove leading

            return getResourceIdFromString(path, type);
        } catch (URISyntaxException e) {
            return -1;
        }
    }

    private int getResourceIdFromString(String resName, String type) {
        try {
            Class<?> cls = Class.forName("io.boden.android." + _libName + ".R$" + type);

            Field idField = cls.getDeclaredField(resName);
            return idField.getInt(idField);
        } catch (Exception e) {
            e.printStackTrace();
            return -1;
        }
    }


    @Override
    public void onConfigurationChanged (Configuration newConfig) {
        _rootView.onConfigurationChanged(newConfig);

        super.onConfigurationChanged(newConfig);
    }

    @Override
    public void onBackPressed() {
        if(!_backButtonHandlers.handleBackButtonPressed()) {
            super.onBackPressed();
        }
    }

    public void addBackButtonListener(BackButtonListener listener) {
        _backButtonHandlers.add(listener);
    }

    public void removeBackButtonListener(BackButtonListener listener) {
        _backButtonHandlers.remove(listener);
    }

    public static NativeRootActivity getRootActivity() {
        return _rootActivity;
    }

    private void loadMetaData()
    {
        try
        {
            ActivityInfo activityInfo = getPackageManager().getActivityInfo(
                    getComponentName(), PackageManager.GET_ACTIVITIES | PackageManager.GET_META_DATA);

            _metaData = activityInfo.metaData;
        }
        catch(PackageManager.NameNotFoundException e)
        {
            // activity is not in the manifest. Treat this like empty meta data
            _metaData = null;
        }

        if(_metaData==null)
            _metaData = new Bundle();
    }

    private String getMetaString(String valueName, String defaultValue)
    {
        Object val = _metaData.get( valueName );
        if( val instanceof String)
            return (String)val;
        else
            return defaultValue;
    }

    private static native void nativeRegisterAppContext(Context ctxt);

    private NativeRootView _rootView;
    private Bundle _metaData;
    private String _libName;
    private static NativeRootActivity _rootActivity;

    private BackButtonHandlers _backButtonHandlers;
}





