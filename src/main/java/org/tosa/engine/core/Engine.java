package org.tosa.engine.core;

import org.lwjgl.glfw.GLFWErrorCallback;
import org.tosa.engine.core.render.Window;
import org.tosa.engine.core.utils.Input;
import org.tosa.engine.core.utils.KeyCode;

public class Engine {
    private Window MainWindow;

    public Engine(Window MainWindow) {
        this.MainWindow = MainWindow;
    }

    public void start() {
        GLFWErrorCallback.createPrint(System.err).set();
        MainWindow.create();
        this.update();
    }

    public void update() {
        while(!MainWindow.isClose()) {
            if(Input.getKeyDown(MainWindow.getWindowId(), KeyCode.LeftAlt))
                System.out.println("B pressed");

            if(Input.getKeyUp(MainWindow.getWindowId(), KeyCode.B))
                System.out.println("B releassed");

            if(Input.getMouse(MainWindow.getWindowId(), KeyCode.MouseLeft))
                System.out.println("B releassed");

            MainWindow.update();
            //render
        }

        MainWindow.destroy();
    }

    public Window getMainWindow() {
        return MainWindow;
    }
}
