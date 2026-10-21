// Create a function at every address of the list that is not already covered,
// retrying a few times as new functions may make earlier ones creatable.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import java.io.*;
import java.util.*;

public class Sweep extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        List<Long> offs = new ArrayList<>();
        BufferedReader r = new BufferedReader(new FileReader(args[0]));
        String line;
        while ((line = r.readLine()) != null) {
            line = line.trim();
            if (line.isEmpty() || line.startsWith("#")) continue;
            offs.add(Long.parseLong(line, 16));
        }
        r.close();
        println("entries " + offs.size());
        int made = 0, present = 0, faded = 0;
        for (int pass = 0; pass < 3; pass++) {
            int n = 0;
            for (long off : offs) {
                Address a = toAddr(off);
                if (getFunctionAt(a) != null) { present++; continue; }
                if (currentProgram.getMemory().getBlock(a) == null) { faded++; continue; }
                try {
                    if (getInstructionAt(a) == null) disassemble(a);
                    if (getFunctionAt(a) == null && getInstructionAt(a) != null) {
                        createFunction(a, null); made++; n++;
                    }
                } catch (Throwable e) { faded++; }
            }
            println("pass " + pass + " made " + n + " (total " + made + ")");
            if (n == 0) break;
        }
        println("Sweep done made=" + made + " present=" + present + " failed=" + faded);
    }
}
