import org.tosa.engine.core.Engine;
import org.tosa.engine.core.renderer.Window;
import org.tosa.engine.core.utils.Debug;

public class Laucnher {
    public static void main(String[] args) {
        Window window = new Window(800, 600, "test");
        Engine engine = new Engine(window);
        Debug.Log("Log");
        Debug.Warning("Warning");
        Debug.Error("Error");
        engine.start();
    }
}
