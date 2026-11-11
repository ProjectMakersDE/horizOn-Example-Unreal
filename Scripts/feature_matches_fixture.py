#!/usr/bin/env python3
"""Actual Lab Match dispatch against exact loopback HTTP; never a hosted backend."""
import argparse,json,subprocess,threading
from pathlib import Path
from http.server import BaseHTTPRequestHandler,ThreadingHTTPServer
from urllib.parse import urlsplit,parse_qs
ROOT=Path(__file__).resolve().parents[1]
U='00000000-0000-4000-8000-000000000001';V='00000000-0000-4000-8000-000000000002';M='aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa';T='2026-10-18T00:00:00.000Z'
def match(revision):
    result=dict(id=M,name='Seagulls',revision=revision,creatorId=U,capacity=2,visibility='private',strength=0,status='lobby'if revision==7 else'active'if revision==8 else'reporting',participantIds=[U,V],invitedIds=[],forfeitedIds=[],currentPlayerId=U if revision==8 else None,turnDeadline=T if revision==8 else None,createdAt=T,startedAt=None if revision==7 else T,finishedAt=T if revision>=9 else None,purgeAt=None,finalStateRevision=9 if revision>=9 else None,hasDisagreement=False,ranked=False,timeoutDue=False,turnOrder=[]if revision==7 else[U,V],turnSeconds=60,timeoutPolicy='skip',dataBase64=''if revision==7 else'AAE=',reportDeadline=T if revision>=9 else None,absoluteDeadline=T,rankedLeaderboardKey=None,ownRankedRunId=None,notificationPending=False,reports=[])
    if revision==10:result['reports']=[dict(userId=U,finalStateRevision=9,resultBase64='AAE=',winnerIds=[U,V],reportedAt=T)]
    return dict(match=result,pollAfterSeconds=120,serverTime=T)
CASES=[('GET','/actuator/health',None,dict(status='UP'),200,None),('POST','/api/v1/app/user-management/signin',dict(email='lab@example.invalid',password='synthetic-password',type='EMAIL'),dict(userId=U,displayName='Lab',email='lab@example.invalid',accessToken='synthetic-lab-session',type='EMAIL',authStatus='AUTHENTICATED'),200,None),('GET','/api/v1/app/matches/'+M,None,match(7),200,None),('POST','/api/v1/app/matches/'+M+'/start',dict(userId=U,turnOrder=[U,V]),match(8),200,'7'),('POST','/api/v1/app/matches/'+M+'/turn',dict(userId=U,dataBase64='AAE=',finish=True),match(9),200,'8'),('POST','/api/v1/app/matches/'+M+'/result',dict(userId=U,finalStateRevision=9,resultBase64='AAE=',winnerIds=[U,V]),dict(code='TEMPORARY'),503,None),('POST','/api/v1/app/matches/'+M+'/result',dict(userId=U,finalStateRevision=9,resultBase64='AAE=',winnerIds=[U,V]),dict(match(10),replayed=True),200,None)]
records=[];errors=[];original=None
class Handler(BaseHTTPRequestHandler):
    def log_message(self,*_):pass
    def do_GET(self):self.case()
    def do_POST(self):self.case()
    def case(self):
        global original
        try:
            verb,path,body,reply,status,revision=CASES[len(records)];parsed=urlsplit(self.path);raw=self.rfile.read(int(self.headers.get('Content-Length',0)))
            assert(verb,path)==(self.command,parsed.path),(len(records),self.command,parsed.path)
            assert(json.loads(raw)if raw else None)==body,(path,raw)
            assert self.headers.get('If-Match')==revision
            if parsed.path.startswith('/api/'):
                assert self.headers.get('X-API-Key')=='task1004-example-fixture'
                if not path.endswith('/signin'):assert self.headers.get('Authorization')=='Bearer synthetic-lab-session'
            if verb=='GET'and'/matches/'in path:assert parse_qs(parsed.query)==dict(userId=[U])
            if path.endswith('/result'):
                if original is None:original=raw
                else:assert raw==original,'Stored result replay changed bytes'
            records.append(dict(method=verb,path=path,body=body,revision=revision))
            data=json.dumps(reply,separators=(',',':')).encode();self.send_response(status);self.send_header('Content-Type','application/json');self.send_header('Content-Length',str(len(data)));self.end_headers();self.wfile.write(data)
        except Exception as error:errors.append(str(error));self.send_error(500)
def main():
    p=argparse.ArgumentParser();p.add_argument('--editor',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=False)
    report=a.output/'report'
    with ThreadingHTTPServer(('127.0.0.1',0),Handler)as server:
        thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start()
        command=[str(a.editor),str(ROOT/'SeagullStorm.uproject'),'/Game/Maps/MainMap?game=/Script/SeagullStorm.SeagullFeaturesHelloGameMode','-game','-nullrhi','-unattended','-NoSplash','-NoEpicPortal','-nosound','-SeagullFixture=http://127.0.0.1:'+str(server.server_port),'-UserDir='+str(a.output/'isolated-user'),'-ExecCmds=Automation RunTests Seagull.Acceptance.CurrentFeatureMatches','-TestExit=Automation Test Queue Empty','-ReportExportPath='+str(report),'-log']
        try:
            with(a.output/'launcher.log').open('w')as log:result=subprocess.run(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,timeout=120)
        finally:
            server.shutdown();(a.output/'http-observed.json').write_text(json.dumps(dict(requests=records,errors=errors),indent=2)+'\n')
    summary=json.loads((report/'index.json').read_text(encoding='utf-8-sig'))
    assert result.returncode==0 and summary['failed']==0 and len(summary['tests'])==1,'Actual Lab Match native acceptance failed'
    assert not errors and len(records)==len(CASES),(records,errors)
    (a.output/'result.json').write_text(json.dumps(dict(passed=True,requests=len(records),actualLabDispatch=True,completeTurnOrder=True,completeWinnerIds=True,uncertainStoredResultBytesIdentical=True),indent=2)+'\n')
    print('Actual Unreal Lab Match start/finish/result + recreated-screen original retry passed: seven exact HTTPs')
if __name__=='__main__':main()
