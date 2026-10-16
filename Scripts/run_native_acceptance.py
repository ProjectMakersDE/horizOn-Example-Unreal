#!/usr/bin/env python3
"""Run real Unreal automation against an isolated loopback fixture, never production."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import time
import uuid

ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser()
    engine=parser.add_mutually_exclusive_group(required=True)
    engine.add_argument('--editor',type=Path);engine.add_argument('--game',type=Path)
    parser.add_argument('--output',type=Path,required=True);parser.add_argument('--port',type=int,default=18814)
    parser.add_argument('--headless',action='store_true');parser.add_argument('--hello',action='store_true')
    parser.add_argument('--width',type=int,default=1280);parser.add_argument('--height',type=int,default=720)
    args=parser.parse_args();args.output=args.output.resolve();args.output.mkdir(parents=True,exist_ok=True)
    ready=args.output/'fixture.ready';result=args.output/'fixture.json';report=args.output/'report'
    if result.exists() or report.exists(): raise SystemExit('Use a fresh output directory to prevent stale evidence')
    user_dir=ROOT/'Saved'/('acceptance-'+uuid.uuid4().hex)
    test='Seagull.Acceptance.'+('HelloHorizon' if args.hello else 'GameAndPlayerValues')
    command=[str(args.editor or args.game)]
    if args.editor: command += [str(ROOT/'SeagullStorm.uproject'),'-game']
    if args.hello: command += ['/Game/Maps/MainMap?game=/Script/Engine.GameModeBase']
    command += ['-windowed',f'-ResX={args.width}',f'-ResY={args.height}',f'-SeagullFixture=http://127.0.0.1:{args.port}',f'-UserDir={user_dir}',f'-ExecCmds=Automation RunTests {test}','-TestExit=Automation Test Queue Empty',f'-ReportExportPath={report}','-unattended','-NoSplash','-NoEpicPortal','-nosound','-log']
    if args.headless: command += ['-nullrhi']
    captures=args.output/'screenshots';captures.mkdir(exist_ok=True)
    command += [f'-SeagullCaptureDir={captures}']
    process=None
    with (args.output/'fixture.log').open('w') as fixture_log,(args.output/'launcher.log').open('w') as engine_log:
        fixture=subprocess.Popen(['python3',str(ROOT/'Scripts/player_values_fixture.py'),'--port',str(args.port),'--ready',str(ready),'--result',str(result)],cwd=ROOT,stdout=fixture_log,stderr=subprocess.STDOUT)
        try:
            deadline=time.monotonic()+10
            while not ready.exists():
                if fixture.poll() is not None: raise RuntimeError('Fixture failed before readiness; existing ports are never killed')
                if time.monotonic()>deadline: raise TimeoutError('Fixture readiness')
                time.sleep(.1)
            process=subprocess.Popen(command,cwd=ROOT,stdout=engine_log,stderr=subprocess.STDOUT)
            code=process.wait(timeout=300)
            if not (report/'index.json').is_file(): raise RuntimeError('Native report absent; launcher exit alone is insufficient')
            summary=json.loads((report/'index.json').read_text(encoding='utf-8-sig'))
            if code or summary.get('failed',0) or not summary.get('tests') or any(t.get('state')!='Success' or t.get('errors') for t in summary['tests']): raise RuntimeError('Native automation failed')
        finally:
            for child in (process,fixture):
                if child and child.poll() is None:
                    child.terminate()
                    try: child.wait(timeout=15)
                    except subprocess.TimeoutExpired: child.kill();child.wait()
            ready.unlink(missing_ok=True)
            logs=user_dir/'Saved/Logs'
            if logs.is_dir(): shutil.copytree(logs,args.output/'engine-logs',dirs_exist_ok=True)
    proof=json.loads(result.read_text())
    if args.hello:
        assert proof['scores']==1 and proof['purchases']==0 and proof['adjustments']==0
        assert [c['route'] for c in proof['calls']]==['/actuator/health','/api/v1/app/user-management/signup','/api/v1/app/leaderboard/submit']
    else:
        assert proof['scores']==0 and proof['purchases']==3 and proof['adjustments']==1 and proof['gold']==93 and proof['energy']==9
    if not args.headless:
        names=['15-hello.png'] if args.hello else [f'{n:02}-' for n in range(1,15)]
        for name in names:
            matches=list(captures.glob(name if args.hello else name+'*.png'))
            if len(matches)!=1: raise RuntimeError('Native screenshot absent or ambiguous: '+name)
    print(json.dumps(dict(passed=True,test=test,report=str(report),fixtureRequests=len(proof['calls']),userDir=str(user_dir),headless=args.headless)))
if __name__=='__main__': main()
