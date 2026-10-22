// Export the recovered model of one address range as plain text, so the C
// layer can be generated from Python.
//
//   Dump.java <outdir> <start-hex> <end-hex>
//
// Writes
//   <outdir>/units.txt   one line per instruction AND per data unit inside the
//                        range, in address order:
//                          <addr-hex> <len> <bytes-hex> <kind> <text>
//                        kind is "I" for a listing instruction and "D" for a
//                        defined data unit (the text is the C expression Ghidra
//                        gives it: undefined4, s_"..." , etc.)
//   <outdir>/funcs.txt   one line per function entry: <addr-hex> <name>
//   <outdir>/decomp.txt  the decompiled C of every function, separated by
//                        "///// <addr-hex> <name>" lines.  Functions Ghidra
//                        does not know about but that have instructions are
//                        created on the fly and decompiled too.
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import java.io.*;

public class Dump extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();
        File out = new File(a[0]);
        out.mkdirs();
        long start = Long.parseLong(a[1], 16);
        long end = Long.parseLong(a[2], 16);
        Address lo = toAddr(start), hi = toAddr(end);
        Program p = currentProgram;
        Listing lst = p.getListing();
        Memory mem = p.getMemory();

        PrintWriter w = new PrintWriter(new BufferedWriter(new FileWriter(new File(out, "units.txt")), 1 << 20));
        int n = 0;
        for (CodeUnit cu : lst.getCodeUnits(lo, true)) {
            Address s = cu.getAddress();
            if (s.getOffset() >= end) break;
            int len = cu.getLength();
            byte[] b = new byte[len];
            try { mem.getBytes(s, b); } catch (MemoryAccessException e) { continue; }
            StringBuilder sb = new StringBuilder();
            for (byte x : b) sb.append(String.format("%02x", x));
            String txt;
            if (cu instanceof Instruction) {
                Instruction ins = (Instruction) cu;
                StringBuilder t = new StringBuilder(ins.getMnemonicString());
                int k = ins.getNumOperands();
                for (int i = 0; i < k; i++) {
                    t.append(i == 0 ? " " : ", ");
                    t.append(ins.getDefaultOperandRepresentation(i));
                }
                String fl = ins.getFlowType().toString();
                txt = t.toString().replace('\t', ' ');
            } else {
                txt = "DATA:" + cu.toString().replace('\t', ' ');
            }
            w.println(Long.toHexString(s.getOffset()) + " " + len + " " + sb + " "
                      + (cu instanceof Instruction ? "I" : "D") + " " + txt);
            if ((++n % 50000) == 0) println("units " + n);
        }
        w.close();
        println("units " + n);

        // function list, plus a function for every instruction the analysis left
        // uncovered: the C layer needs one entry per call target to avoid
        // splitting a thunk body, so create them here (the program is writable).
        PrintWriter fw = new PrintWriter(new FileWriter(new File(out, "funcs.txt")));
        int made = 0;
        for (Address s = lo; s.getOffset() < end; ) {
            Instruction ins = lst.getInstructionContaining(s);
            if (ins == null) { s = s.add(1); continue; }
            Function f = lst.getFunctionContaining(s);
            if (f == null && ins.getAddress().equals(s)) {
                try { createFunction(s, null); made++; } catch (Throwable e) { }
            }
            s = ins.getMaxAddress().add(1);
        }
        println("functions created " + made);
        for (Function f : p.getFunctionManager().getFunctions(lo, true)) {
            if (f.getEntryPoint().getOffset() >= end) break;
            fw.println(Long.toHexString(f.getEntryPoint().getOffset()) + " " + f.getName());
        }
        fw.close();

        DecompInterface di = new DecompInterface();
        DecompileOptions opt = new DecompileOptions();
        opt.setMaxPayloadMBytes(400);
        opt.setMaxInstructions(200000);
        di.setOptions(opt);
        di.toggleCCode(true);
        di.toggleSyntaxTree(false);
        di.setSimplificationStyle("decompile");
        if (!di.openProgram(p)) {
            println("decompiler open failed: " + di.getLastMessage());
            return;
        }
        PrintWriter dw = new PrintWriter(new BufferedWriter(new FileWriter(new File(out, "decomp.txt")), 1 << 20));
        int done = 0, failed = 0;
        for (Function f : p.getFunctionManager().getFunctions(lo, true)) {
            if (f.getEntryPoint().getOffset() >= end) break;
            if (f.isThunk()) continue;
            DecompileResults r = di.decompileFunction(f, 600, monitor);
            dw.println("///// " + Long.toHexString(f.getEntryPoint().getOffset()) + " " + f.getName());
            if (r.decompileCompleted()) {
                dw.println(r.getDecompiledFunction().getC());
            } else {
                dw.println("/* FAILED " + r.getErrorMessage() + " */");
                failed++;
            }
            if ((++done % 200) == 0) println("decompiled " + done + " failed " + failed);
        }
        dw.close();
        di.dispose();
        println("decompiled " + done + " failed " + failed);
    }
}
