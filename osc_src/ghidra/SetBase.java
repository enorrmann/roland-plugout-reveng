// Fix the image base of the raw import (the BinaryLoader option alone leaves
// it at 0, which makes the ARM analyzer see an unmapped address space).
import ghidra.app.script.GhidraScript;

public class SetBase extends GhidraScript {
    @Override
    public void run() throws Exception {
        println("base before " + currentProgram.getImageBase());
        currentProgram.setImageBase(toAddr(0x60000000L), true);
        println("base after " + currentProgram.getImageBase());
    }
}
