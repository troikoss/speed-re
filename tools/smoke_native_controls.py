#!/usr/bin/env python3
"""Retail Controls package routing/capture checks; native integration, not byte matching."""
import argparse
from pathlib import Path
import subprocess
from native_gui_test import add_render_arguments, render_environment
ROOT = Path(__file__).resolve().parents[1]
CASES = {
    'pad_button_secondary': ('CGRAH', ['pad-bound action=3 slot=1 code=3']),
    'pad_axis_capture': ('CGAJ', ['pad-bound action=3 slot=0 code=101']),
    'pad_capture_cancel': ('CGAB', ['capture=3', 'capture=0']),
    'capture_primary': ('CAK', ['capture=3', 'bound action=3 slot=0 key=Z', 'key-active=1 old-active=0']),
    'capture_secondary': ('CRAK', ['row=1 column=2', 'bound action=3 slot=1 key=Z']),
    'capture_cancel': ('CAB', ['capture=3', 'capture=0']),
    'mouse_primary': ('CMK', ['capture=3', 'bound action=3 slot=0 key=Z']),
    'scroll_last': ('C'+'D'*20, ['row=18 column=1 top=11']),
    'return_parent': ('CBRA', ['pause: showing UI_PauseOptionsMain.fng']),
    'player_controls_entry': ('RARRRA'+'D'*10+'A', ['pause: showing UI_Options_PC_Controller.fng']),
}
def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--demo', type=Path, default=ROOT / 'build/native/ug2demo')
    parser.add_argument('--game', type=Path, default=ROOT / 'Need for Speed Underground 2')
    add_render_arguments(parser)
    args = parser.parse_args()
    out = ROOT / 'build/analysis/native-controls';out.mkdir(parents=True,exist_ok=True)
    for name,(keys,expected) in CASES.items():
        image=out/(name+'.ppm')
        image.unlink(missing_ok=True)
        r=subprocess.run([str(args.demo),'--game',str(args.game),'--car','SKYLINE','--frames',str(80+30*len(keys)),
                          '--pause-at','20','--pause-keys',keys,'--screenshot',str(image)],
                         cwd=ROOT,env=render_environment(args,UG2_DEBUG_PAUSE='1'),capture_output=True,text=True,timeout=120)
        log=r.stdout+r.stderr;(out/(name+'.log')).write_text(log)
        assert r.returncode==0,(name,r.returncode)
        assert image.exists() and image.read_bytes().startswith(b'P6\n'),name
        assert 'unrecovered original code' not in log,name
        for text in expected: assert text in log,(name,text)
        print('PASS',name,flush=True)
    print('PASS',len(CASES),'native Controls scenarios')
if __name__=='__main__':main()
