import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.os.Looper;
import java.io.PrintWriter;
import java.util.List;
import android.util.Log;

public class HelperService{
    private static final String SOCKET_NAME = "apkm_service_socket";
    public static void main(String[] args) {
        try {
            // 1. Inicializa o ambiente de sistema
            if (Looper.getMainLooper() == null) Looper.prepareMainLooper();
            
            // Reflexão para o ActivityThread (já que é @hide)
            Class<?> atClass = Class.forName("android.app.ActivityThread");
            Object at = atClass.getMethod("systemMain").invoke(null);
            Context context = (Context) atClass.getMethod("getSystemContext").invoke(at);
            
            //PackageManage de uso global no serviço
            PackageManager pm = context.getPackageManager();

            System.out.println("APKM: Serviço iniciado e aguardando conexões...");

            // 2. Cria um Socket para ouvir o C++
            LocalServerSocket server = new LocalServerSocket(SOCKET_NAME);

            // Thread para não travar o Looper principal
            new Thread(() -> {
                while (true) {
                    try (LocalSocket client = server.accept();
                        java.io.BufferedReader in = new java.io.BufferedReader(
                            new java.io.InputStreamReader(client.getInputStream()));
                        java.io.PrintWriter out = new java.io.PrintWriter(client.getOutputStream(), true)) {

                        // Lê a entrada do C++ (bloqueia até receber uma linha)
                        String input = in.readLine();
                        
                        if (input != null) {
                            // Lógica de roteamento baseada na entrada
                            switch (input) {
                                case "GET_PACKAGES":
                                    out.println(buildPackageJson(pm));
                                    break;
                                    
                                case "GET_VERSION":
                                    out.println("{\"version\": \"1.0-service\"}");
                                    break;
                                    
                                case "PING":
                                    out.println("PONG");
                                    break;
                                case "getPackagesVersions":
                                    out.println(ApkmPckageManager.getPackagesVersions(pm));
                                    Log.i("APKM-SERVICE", "getPackagesVersions");
                                    break;
                                default:
                                    out.println("{\"error\": \"Unknown Command\"}");
                                    break;
                            }
                        }
                        client.close();
                        out.close();
                        in.close();
                    } catch (Exception e) {
                        // Log de erro interno para o logcat
                        android.util.Log.e("APKM_SERVICE", "Erro na conexão: " + e.getMessage());
                    }
                }
            }).start();
            Looper.loop();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static String buildPackageJson(PackageManager pm) {
        StringBuilder sb = new StringBuilder("{\"packages\":[");
        List<PackageInfo> packages = pm.getInstalledPackages(0);
        
        for (int i = 0; i < packages.size(); i++) {
            PackageInfo pkg = packages.get(i);
            // Agora o loadLabel FUNCIONA porque temos o contexto!
            String label = pkg.applicationInfo.loadLabel(pm).toString().replace("\"", "\\\"");
            
            sb.append("{")
              .append("\"id\":\"").append(pkg.packageName).append("\",")
              .append("\"name\":\"").append(label).append("\",")
              .append("\"vCode\":").append(pkg.getLongVersionCode())
              .append("}");
            
            if (i < packages.size() - 1) sb.append(",");
        }
        return sb.append("]}").toString();
    }
}