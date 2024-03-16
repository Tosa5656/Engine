package org.tosa.engine.core.utils;

public class Debug {
    public static void Log(String message) {
        System.out.println(message);
    }

    public static void Warning(String message) {
        System.out.println(Colors.yellow + "Waning: " + message + Colors.reset);
    }

    public static void Error(String message) {
        System.out.println(Colors.red + "Error: " + message + Colors.reset);
    }
}
