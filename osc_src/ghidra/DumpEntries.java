// Write the list of function entry points Ghidra has so far (hex addresses),
// then exit - used to compute which BL targets are still missing.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import java.io.*;

public class DumpEntries extends GhidraScript {
    @Override
    public void run() throws Exception {
        String out = getScriptArgs()[0];
        PrintWriter w = new PrintWriter(new FileWriter(out));
        for (Function f : currentProgram.getFunctionManager().getFunctions(true))
            w.println(Long.toHexString(f.getEntryPoint().getOffset()));
        w.close();
        println("wrote entries");
    }
}
