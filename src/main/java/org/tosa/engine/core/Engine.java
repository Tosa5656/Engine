package org.tosa.engine.core;

import org.lwjgl.glfw.GLFWErrorCallback;
import org.tosa.engine.core.render.Window;

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
        while(!this.MainWindow.isClose()) {
            this.MainWindow.update();
            //render
        }

        this.MainWindow.destroy();
    }

    public Window getMainWindow() {
        return MainWindow;
    }
}
