package org.tosa.engine.core.renderer;

import static org.lwjgl.opengl.GL46.*;

public class Shader {
    public int id;
    private int type;
    private String source;

    public Shader(int type, String source) {
        this.type = type;
        this.source = source;
        createShader();
    }

    public void createShader() {
        id = glCreateShader(type);
        glShaderSource(id, source);
        glCompileShader(id);

        int isCompiled = 0;
        isCompiled = glGetShaderi(id, GL_COMPILE_STATUS);

        if(isCompiled == 0) {
            int maxLenght = 0;
            maxLenght = glGetShaderi(id, GL_INFO_LOG_LENGTH);
            String error = glGetShaderInfoLog(id, maxLenght);
            glDeleteShader(id);
            System.err.println("Error compile " + type + " shader: " + error);
            System.exit(-1);
        }
    }
}