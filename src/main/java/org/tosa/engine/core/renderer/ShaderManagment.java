package org.tosa.engine.core.renderer;

import static org.lwjgl.opengl.GL20.GL_INFO_LOG_LENGTH;
import static org.lwjgl.opengl.GL20.glDeleteShader;
import static org.lwjgl.opengl.GL46C.*;

public class ShaderManagment {
    private int programId;
    private Shader vertex;
    private Shader fragment;

    public ShaderManagment(Shader vertexShader, Shader fragmentShader) {
        vertex = vertexShader;
        fragment = fragmentShader;
    }

    public void compile() {
        programId = glCreateProgram();

        glAttachShader(programId, vertex.id);
        glAttachShader(programId, fragment.id);

        glLinkProgram(programId);

        int isLinked = 0;
        isLinked = glGetProgrami(programId, GL_LINK_STATUS);

        if(isLinked == 0) {
            int maxLenght = 0;
            maxLenght = glGetProgrami(programId, GL_INFO_LOG_LENGTH);
            String error = glGetProgramInfoLog(programId, maxLenght);

            glDetachShader(programId, vertex.id);
            glDetachShader(programId, fragment.id);

            glDeleteShader(vertex.id);
            glDeleteShader(fragment.id);

            glDeleteProgram(programId);
            System.err.println("Error link shaders: " + error);
        }

        glDetachShader(programId, vertex.id);
        glDetachShader(programId, fragment.id);
    }

    public void bind() {
        glUseProgram(programId);
    }

    public void unbind() {
        glUseProgram(0);
    }
}
