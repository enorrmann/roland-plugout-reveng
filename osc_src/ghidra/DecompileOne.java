import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileOne extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        DecompileOptions opt = new DecompileOptions();
        opt.setMaxPayloadMBytes(200);
        opt.setMaxInstructions(400000);
        di.setOptions(opt);
        di.toggleCCode(true);
        di.toggleSyntaxTree(true);
        di.openProgram(currentProgram);
        for (String s : getScriptArgs()) {
            Address a = toAddr(Long.parseLong(s, 16));
            Function f = getFunctionAt(a);
            if (f == null) { println("no fn " + s); continue; }
            DecompileResults r = di.decompileFunction(f, 120, monitor);
            println("===== " + s + " " + f.getName() + " =====");
            if (r.decompileCompleted()) println(r.getDecompiledFunction().getC());
            else println("/* failed " + r.getErrorMessage() + " */");
        }
        di.dispose();
    }
}
