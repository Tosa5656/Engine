package org.tosa.engine.core.utils;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class FileManagment {
    public static String readFileFromResource(boolean AppendSlashes, boolean ReturnInOneLine, String filename) {
        boolean appendSlashes = AppendSlashes;
        boolean returnInOneLine = ReturnInOneLine;
        StringBuilder fileSource = new StringBuilder();
        try {
            InputStream in = Class.class.getResourceAsStream(filename);
            BufferedReader reader = new BufferedReader(new InputStreamReader(in));
            String line;
            while((line = reader.readLine()) != null) {
                fileSource.append(line);
                if(appendSlashes) fileSource.append("//");
                if(!returnInOneLine) fileSource.append("\n");
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        return fileSource.toString();
    }
}
