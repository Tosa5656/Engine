package org.tosa.engine.core.utils;

import org.lwjgl.glfw.GLFW;
import org.lwjgl.glfw.GLFWCursorEnterCallbackI;
import org.lwjgl.glfw.GLFWCursorPosCallbackI;

public class Input {
    private static boolean isPressed = false;
    private static boolean wasIsPressed = false;

    private static boolean isMousePressed = false;
    private static boolean wasMouseIsPressed;


    public static boolean getKey(long window, int key) {
        return GLFW.glfwGetKey(window, key) == GLFW.GLFW_TRUE;
    }

    public static boolean getKeyDown(long window, int key) {
        if(!isPressed && GLFW.glfwGetKey(window, key) == GLFW.GLFW_TRUE) {
            isPressed = true;
            return true;
        } else if(GLFW.glfwGetKey(window, key) == GLFW.GLFW_FALSE) {
            isPressed = false;
        }
        return false;
    }

    public static boolean getKeyUp(long window, int key) {
        boolean isKeyPressed = GLFW.glfwGetKey(window, key) == GLFW.GLFW_TRUE;

        if (!isKeyPressed && wasIsPressed) {
            wasIsPressed = isKeyPressed;
            return true;
        } else {
            wasIsPressed = isKeyPressed;
            return false;
        }
    }

    //Mause
    public static boolean getMouse(long window, int key) {
        return GLFW.glfwGetMouseButton(window, key) == GLFW.GLFW_TRUE;
    }

}
