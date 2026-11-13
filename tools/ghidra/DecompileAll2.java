// Two passes: (1) decompile every function and commit its inferred parameters/convention to the
// program database; (2) decompile again so call sites see the callees' real signatures.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.pcode.*;
import ghidra.program.model.symbol.SourceType;
import java.io.*;
import java.nio.file.*;
import java.util.*;

public class DecompileAll2 extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();
        List<String> lines = Files.readAllLines(Paths.get(a[0]));
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        int n = 0;
        for (String ln : lines) {
            String[] p = ln.split(" ", 3);
            Function f = getFunctionAt(toAddr("0x" + p[0]));
            if (f == null) continue;
            try {
                DecompileResults r = di.decompileFunction(f, 10, monitor);
                if (r.decompileCompleted()) {
                    HighFunctionDBUtil.commitParamsToDatabase(r.getHighFunction(), true, HighFunctionDBUtil.ReturnCommitOption.COMMIT, SourceType.ANALYSIS);
                }
            } catch (Exception e) { }
            if (++n % 1000 == 0) println("pass1 " + n);
        }
        di.closeProgram();
        di = new DecompInterface();
        di.openProgram(currentProgram);
        PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(a[1])));
        n = 0;
        for (String ln : lines) {
            String[] p = ln.split(" ", 3);
            Function f = getFunctionAt(toAddr("0x" + p[0]));
            if (f == null) { continue; }
            DecompileResults r = di.decompileFunction(f, 20, monitor);
            String c = r.decompileCompleted() ? r.getDecompiledFunction().getC() : null;
            StringBuilder sb = new StringBuilder("{\"a\":\"" + p[0] + "\",");
            if (c == null) sb.append("\"err\":\"fail\"}");
            else sb.append("\"c\":\"").append(c.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n").replace("\r", "").replace("\t", "\\t")).append("\"}");
            out.println(sb);
            if (++n % 500 == 0) { out.flush(); println("pass2 " + n); }
        }
        out.close();
    }
}
