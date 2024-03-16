import org.lwjgl.Version;
import org.tosa.engine.core.Engine;
import org.tosa.engine.core.render.Window;
import org.tosa.engine.core.utils.Debug;

public class Laucnher {
    public static void main(String[] args) {
        System.out.println(Version.getVersion());
        Window window = new Window(800, 600, "test");
        Engine engine = new Engine(window);
        Debug.Log("Log");
        Debug.Warning("Warning");
        Debug.Error("Error");
        engine.start();
    }
}
