package org.tosa.engine.core;

import org.lwjgl.glfw.GLFWErrorCallback;
import static org.lwjgl.opengl.GL46C.*;
import static org.tosa.engine.core.utils.FileManagment.readFileFromResource;
import static org.tosa.engine.core.utils.MemoryManagment.*;
import org.tosa.engine.core.renderer.Shader;
import org.tosa.engine.core.renderer.ShaderManagment;
import org.tosa.engine.core.renderer.Window;

public class Engine {
    private Window MainWindow;
    public ShaderManagment shaderManagment;

    String fragment =
        "#version 410\n" +
                "\n" +
                "layout(location = 0) out vec4 out_colout;\n" +
                "\n" +
                "in vec3 position;\n" +
                "\n" +
                "void main()\n" +
                "{\n" +
                "    out_colour = vec4(1.0f, 0.0f, 1.0f, 1.0f)\n" +
                "}";

    String vertex =
        "#version 410\n" +
                "\n" +
                "layout(location = 0) in vec3 attrib_Position;\n" +
                "\n" +
                "out vec3 position;\n" +
                "\n" +
                "void main()\n" +
                "{\n" +
                "   position = attrib_Position;\n" +
                "   gl_Position = vec4(attrib_Position, 1.0f);\n" +
                "}";

    public Engine(Window MainWindow) {
        this.MainWindow = MainWindow;
    }

    public void start() {
        GLFWErrorCallback.createPrint(System.err).set();
        MainWindow.create();

        Shader VertexShader = new Shader(GL_VERTEX_SHADER, vertex);
        Shader FragmentShader = new Shader(GL_FRAGMENT_SHADER, fragment);

        shaderManagment = new ShaderManagment(VertexShader, FragmentShader);

        this.update();
    }

    public void update() {
        float[] verties = {
                0.5f,  0.5f, 0f, -0.5f, 0.5f, 0f,
                -0.5f, -0.5f, 0f, 0.5f, -0.5f, 0f
        };

        int[] indices = { 0, 1, 2, 0, 2, 3 };

        int vaoId = glGenVertexArrays();
        glBindVertexArray(vaoId);
        //IBO
        int iboId = glGenBuffers();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, putData(indices), GL_STATIC_DRAW);
        //VBO
        int vboId = glGenBuffers();
        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glBufferData(GL_ARRAY_BUFFER, putData(verties), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER,vaoId);
        //

        glBindVertexArray(vaoId);

        while(!MainWindow.isClose()) {
            MainWindow.update();
            //clear
            glClearColor(0, 0, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            //render
            glBindVertexArray(vaoId);
            shaderManagment.bind();
            glDrawElements(GL_TRIANGLES, indices.length, GL_UNSIGNED_INT, 0);
            shaderManagment.unbind();
            glBindVertexArray(vaoId);

        }

        MainWindow.destroy();
    }

    public Window getMainWindow() {
        return MainWindow;
    }
}
