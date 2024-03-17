package org.tosa.engine.core.renderer;

import org.lwjgl.BufferUtils;
import org.lwjgl.glfw.GLFW;
import org.lwjgl.glfw.GLFWVidMode;
import org.lwjgl.opengl.GL;
import org.lwjgl.opengl.GL11;
import org.lwjgl.system.MemoryStack;

import java.nio.IntBuffer;

public class Window {
    private int width,height;
    public IntBuffer bufferedWidth;
    public IntBuffer bufferedHeight;
    private GLFWVidMode videoMode;
    private String title;
    private long windowId;

    public Window(int wodth, int height, String title) {
        this.width = wodth;
        this.height = height;
        this.title = title;
    }

    public void create() {
        if(!GLFW.glfwInit())
            System.err.println("GLFW not inited!");

        this.windowId = GLFW.glfwCreateWindow(this.width, this.height, this.title, 0, 0);

        if(this.windowId == 0)
            System.err.println("Error init window!");

        try (MemoryStack mem = MemoryStack.stackPush()){
            this.bufferedWidth = BufferUtils.createIntBuffer(1);
            this.bufferedHeight = BufferUtils.createIntBuffer(1);
            this.videoMode = GLFW.glfwGetVideoMode(GLFW.glfwGetPrimaryMonitor());
            GLFW.glfwGetWindowSize(this.windowId, this.bufferedWidth, this.bufferedHeight);
        } catch (Exception e) {
            System.err.println(e);
        }

        GLFW.glfwSetWindowTitle(this.windowId, this.title);
        GLFW.glfwSetWindowSize(this.windowId, this.width, this.height);
        GLFW.glfwSetWindowAspectRatio(this.windowId, this.width, this.height);
        GLFW.glfwSetWindowPos(this.windowId,
                (this.videoMode.width() - bufferedWidth.get(0)) / 2,
                (this.videoMode.height() - bufferedHeight.get(0)) / 2);

        GLFW.glfwMakeContextCurrent(this.windowId);
        GL.createCapabilities();
        GL11.glViewport(0, 0, this.bufferedWidth.get(0), this.bufferedHeight.get(0));
    }

    public void update() {
        GLFW.glfwPollEvents();
        GLFW.glfwSwapBuffers(this.windowId);
    }

    public void destroy() {
        GLFW.glfwDestroyWindow(this.windowId);
    }

    public boolean isClose() {
        return GLFW.glfwWindowShouldClose(this.windowId);
    }

    public int getWodth() {
        return width;
    }

    public void setWodth(int wodth) {
        this.width = wodth;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public long getWindowId() {
        return windowId;
    }
}
