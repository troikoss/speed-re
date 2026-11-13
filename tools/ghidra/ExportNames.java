import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.*;
import java.io.*;

public class ExportNames extends GhidraScript {
    @Override
    public void run() throws Exception {
        PrintWriter out = new PrintWriter(new FileWriter(getScriptArgs()[0]));
        SymbolIterator it = currentProgram.getSymbolTable().getAllSymbols(true);
        while (it.hasNext()) {
            Symbol s = it.next();
            if (s.getSource() == SourceType.DEFAULT) continue;
            String k = s.getSymbolType() == SymbolType.FUNCTION ? "F" : "D";
            if (s.getSymbolType() != SymbolType.FUNCTION && s.getSymbolType() != SymbolType.LABEL) continue;
            out.println(k + " " + s.getAddress() + " " + s.getName());
        }
        out.close();
    }
}
