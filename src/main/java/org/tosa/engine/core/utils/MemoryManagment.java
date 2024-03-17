package org.tosa.engine.core.utils;

import java.nio.DoubleBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;

import static org.lwjgl.system.MemoryStack.*;

public class MemoryManagment {
    public static IntBuffer putData(int[] data) {
        return (IntBuffer) stackGet().mallocInt(4 * data.length).put(data).flip();
    }

    public static FloatBuffer putData(float[] data) {
        return (FloatBuffer) stackGet().mallocFloat(4 * data.length).put(data).flip();
    }

    public static DoubleBuffer putData(double[] data) {
        return (DoubleBuffer) stackGet().mallocDouble(8 * data.length).put(data).flip();
    }

    public static LongBuffer putData(long[] data) {
        return (LongBuffer) stackGet().mallocLong(8 * data.length).put(data).flip();
    }
}
