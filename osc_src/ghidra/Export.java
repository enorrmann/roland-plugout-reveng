// Export the recovered model of the program as two files:
//   <out>.asm   one line per code unit:  <addr-hex> <len> <bytes-hex> <instr>
//   <out>.meta  one line per symbol:     <addr-hex> <type> <name>
// Both are plain text so the reconstruction can be driven from Python.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.mem.*;
import java.io.*;

public class Export extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();
        String base = a[0];
        Memory mem = currentProgram.getMemory();
        Listing lst = currentProgram.getListing();
        // code / data dump
        PrintWriter w = new PrintWriter(new BufferedWriter(new FileWriter(base + ".asm")));
        long n = 0;
        for (CodeUnit cu : lst.getCodeUnits(true)) {
            Address s = cu.getAddress();
            int len = cu.getLength();
            byte[] b = new byte[len];
            try { mem.getBytes(s, b); } catch (MemoryAccessException e) { continue; }
            StringBuilder sb = new StringBuilder();
            for (byte x : b) sb.append(String.format("%02x", x));
            String txt = "";
            if (cu instanceof Instruction) txt = ((Instruction) cu).toString();
            else if (cu instanceof Data) txt = "DATA:" + ((Data) cu).getDataType().getName();
            w.println(Long.toHexString(s.getOffset()) + " " + len + " " + sb + " " + txt.replace('\t',' '));
            if ((++n % 100000) == 0) println("exported " + n);
        }
        w.close();
        // symbols
        PrintWriter m = new PrintWriter(new BufferedWriter(new FileWriter(base + ".meta")));
        SymbolTable st = currentProgram.getSymbolTable();
        for (Symbol s : st.getAllSymbols(false)) {
            if (s.isExternal() || s.isDynamic()) continue;
            m.println(Long.toHexString(s.getAddress().getOffset()) + " " + s.getSymbolType() + " " + s.getName());
        }
        m.close();
        println("Export done");
    }
}
