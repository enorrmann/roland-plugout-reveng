// Ghidra headless post-script: decompile functions to C.
//   analyzeHeadless <proj> <name> -process JX-3P.vst3 -noanalysis \
//       -postScript Decompile.java <outdir> <addr> [<addr> ...]
// Each function goes to <outdir>/<addr>.c (the function containing <addr>; created if
// Ghidra has none there).
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import java.io.*;

public class Decompile extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();
        File out = new File(a[0]);
        out.mkdirs();
        DecompInterface di = new DecompInterface();
        DecompileOptions opt = new DecompileOptions();
        opt.setMaxPayloadMBytes(200);
        opt.setMaxInstructions(200000);
        di.setOptions(opt);
        di.toggleCCode(true);
        di.toggleSyntaxTree(true);
        di.setSimplificationStyle("decompile");
        di.openProgram(currentProgram);
        for (int i = 1; i < a.length; i++) {
            Address ad = toAddr(a[i]);
            Function f = getFunctionAt(ad);
            if (f == null) f = getFunctionContaining(ad);
            if (f == null) {
                disassemble(ad);
                f = createFunction(ad, null);
            }
            if (f == null) { println("no function at " + a[i]); continue; }
            DecompileResults r = di.decompileFunction(f, 1800, monitor);
            String c = r.decompileCompleted() ? r.getDecompiledFunction().getC() : ("/* failed: " + r.getErrorMessage() + " */");
            try (PrintWriter w = new PrintWriter(new File(out, a[i] + ".c"))) { w.print(c); }
            println(a[i] + " " + f.getName() + " " + c.length() + " chars");
        }
        di.dispose();
    }
}
