// Load the community struct header, apply community prototypes, commit signatures, then export C.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.app.cmd.function.ApplyFunctionSignatureCmd;
import ghidra.app.util.cparser.C.CParser;
import ghidra.app.util.parser.FunctionSignatureParser;
import ghidra.program.model.data.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.pcode.*;
import ghidra.program.model.symbol.SourceType;
import java.io.*;
import java.nio.file.*;
import java.util.*;

public class DecompileTyped extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs(); // funcs.txt header protos.tsv out.jsonl
        List<String> lines = Files.readAllLines(Paths.get(a[0]));
        DataTypeManager dtm = currentProgram.getDataTypeManager();
        String header = new String(Files.readAllBytes(Paths.get(a[1]))).replaceAll("(?m)^#.*$", "");
        try {
            CParser parser = new CParser(dtm, true, null);
            parser.parse(header);
            println("header parsed");
        } catch (Throwable t) { println("header parse failed: " + t); }
        int ok = 0, bad = 0;
        FunctionSignatureParser fsp = new FunctionSignatureParser(dtm, null);
        for (String ln : Files.readAllLines(Paths.get(a[2]))) {
            String[] p = ln.split("\t", 2);
            try {
                Address ad = toAddr("0x" + p[0]);
                Function f = getFunctionAt(ad);
                if (f == null) continue;
                String proto = p[1];
                String conv = null;
                java.util.regex.Matcher cm = java.util.regex.Pattern.compile("__(thiscall|stdcall|cdecl|fastcall)").matcher(proto);
                if (cm.find()) conv = "__" + cm.group(1);
                proto = proto.replaceAll("\\s*__(thiscall|stdcall|cdecl|fastcall)\\s*", " ");
                proto = proto.replaceAll("\\b(struct|union|enum)\\s+", "").trim().replaceAll(";$", "");
                FunctionDefinitionDataType def = fsp.parse(null, proto);
                if (conv != null) def.setCallingConvention(conv);
                ApplyFunctionSignatureCmd cmd = new ApplyFunctionSignatureCmd(ad, def, SourceType.USER_DEFINED, true, ghidra.app.cmd.function.FunctionRenameOption.NO_CHANGE);
                if (cmd.applyTo(currentProgram, monitor)) ok++; else bad++;
            } catch (Throwable t) { bad++; }
        }
        println("prototypes applied " + ok + " failed " + bad);
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        int n = 0;
        for (String ln : lines) {
            String[] p = ln.split(" ", 3);
            Function f = getFunctionAt(toAddr("0x" + p[0]));
            if (f == null) continue;
            try {
                DecompileResults r = di.decompileFunction(f, 10, monitor);
                if (r.decompileCompleted()) HighFunctionDBUtil.commitParamsToDatabase(r.getHighFunction(), true, HighFunctionDBUtil.ReturnCommitOption.COMMIT, SourceType.ANALYSIS);
            } catch (Throwable t) { }
            if (++n % 1000 == 0) println("pass1 " + n);
        }
        di.closeProgram();
        di = new DecompInterface();
        di.openProgram(currentProgram);
        PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(a[3])));
        n = 0;
        for (String ln : lines) {
            String[] p = ln.split(" ", 3);
            Function f = getFunctionAt(toAddr("0x" + p[0]));
            if (f == null) continue;
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
