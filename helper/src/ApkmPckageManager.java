
import java.lang.reflect.Method;
import java.util.List;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.util.Log;

public class ApkmPckageManager {
    static final int FLAG_SYSTEM = 1 << 0; // 0x00000001
    static final int FLAG_UPDATED_SYSTEM_APP = 1 << 7; // 0x00000080
    public static String packagesFile = "/data/system/packages.xml";
    public static String getPackagesVersions(PackageManager pmanager){
        StringBuilder sbrest = new StringBuilder();
        List<PackageInfo> packages = pmanager.getInstalledPackages(0);
        boolean first = true;
        for (int i = 0; i < packages.size(); i++) {
            PackageInfo pkg = packages.get(i);
            boolean isSystem = (pkg.applicationInfo.flags & FLAG_SYSTEM) != 0;
            boolean isUpdatedSystem = (pkg.applicationInfo.flags & FLAG_UPDATED_SYSTEM_APP) != 0;
            if (!isSystem || isUpdatedSystem){
                String label = "\""+pkg.applicationInfo.loadLabel(pmanager).toString()+"\"";
                if (!first){
                    sbrest.append("\n");
                }
                sbrest.append("{")
                    .append("\"package\":").append(pkg.packageName != null ? "\""+pkg.packageName+"\"" : "unknown").append(",")
                    .append("\"appName\":").append(label != null ? label : "unknown").append(",")
                    .append("\"vCode\":").append(pkg.getLongVersionCode()).append(",")
                    .append("\"vName\":\"").append(pkg.versionName != null ? pkg.versionName : "unknown").append("\"");
                sbrest.append("}");
                first = false;
            }
        }
        return sbrest.toString();
    }
}
