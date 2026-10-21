// Count what the analysis covered: functions, instructions, data.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;

public class Count extends GhidraScript {
    @Override
    public void run() throws Exception {
        Listing lst = currentProgram.getListing();
        int nfn = currentProgram.getFunctionManager().getFunctionCount();
        int nins = 0, ndata = 0, nundef = 0;
        long insBytes = 0;
        for (Instruction i : lst.getInstructions(true)) { nins++; insBytes += i.getLength(); }
        for (Data d : lst.getDefinedData(true)) ndata++;
        int ncode = 0;
        for (CodeUnit cu : lst.getCodeUnits(true)) ncode++;
        println("functions=" + nfn + " instructions=" + nins + " insBytes=" + insBytes
                + " data=" + ndata + " codeunits=" + ncode);
        // symbols already present
        int nsym = 0;
        for (Symbol s : currentProgram.getSymbolTable().getAllSymbols(true)) nsym++;
        println("symbols=" + nsym);
    }
}
