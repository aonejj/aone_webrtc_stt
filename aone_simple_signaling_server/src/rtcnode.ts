
import { WebSocket, RawData  } from "ws";
import { kAddPeerMsgRes, kRegistMsg,
         kRemovePeerMsgRes, kAnswerMsg,
         kCandidateMsg, kIceStateMsg } from "./types/signaling_msg_type";

export interface IRTCNodeListener {
    onRTCNodeClose(node:RTCNode): void;
    onRTCNodeRegist(node:RTCNode, jobj:any): void;
    onRTCNodeAddPeerRes(node:RTCNode, jobj:any):void;
    onRTCNodeRemovePeerRes(node:RTCNode, jobj:any):void;
    onRTCNodeAnswer(node:RTCNode, jobj:any):void;
    onRTCNodeCandidate(node:RTCNode, jobj:any):void;
    onRTCNodeIceState(node:RTCNode, jobj:any):void;
}

export class RTCNode {
    private readonly  _ws:WebSocket;
    private readonly  _id:string;
    private readonly  _listener:IRTCNodeListener;

    constructor(ws:WebSocket, id:string, listener:IRTCNodeListener) {
        this._ws = ws;
        this._id = id;
        this._listener = listener;

        this._ws.on('message', this.onMessage.bind(this));
        this._ws.on('error', this.onError.bind(this));
        this._ws.on('close', this.onClose.bind(this));
        this._ws.on('ping', this.onPing.bind(this));
        this._ws.on('pong', this.onPong.bind(this));
    }

    getId():string {
        return this._id;
    }

    isOpen():boolean {
        if(this._ws.readyState === WebSocket.OPEN) {
            return true;
        }

        return false;
    }
    
    send(jobj:any):void {
        if(!this.isOpen()) {
            return;
        }

        try {
            const msg = JSON.stringify(jobj);
            this._ws.send(msg);
//            console.log(`[RTCNode ${this._id}] Sent: ${msg}`);
        } catch (error) {
            console.log(`[RTCNode ${this._id}] Failed to stringify message for sending: `, jobj, error);
        }
    }

    onMessage(data: RawData, isBinary:boolean):void {
        if(isBinary) {
            console.warn(`[RTCNode ${this._id}] Received binary data, ignoring.`);
            return;
        }

        const msg = data.toString();
        let jobj:any;

        try {
            jobj = JSON.parse(msg);
        } catch(error) {
            console.error(`[RTCNode ${this._id}] Failed to parse incoming message as JSON: ${msg}. Error: ${error}`);
            return;
        }

        switch(jobj.cmd) {
            case kRegistMsg:
                console.log(`[RTCNode ${this._id}] Received ${jobj.cmd} command.`);
                this._listener.onRTCNodeRegist(this, jobj);
                break;

            case kAddPeerMsgRes:
                console.log(`[RTCNode ${this._id}] Received  ${jobj.cmd} command  result ${jobj.result}.`);
                this._listener.onRTCNodeAddPeerRes(this, jobj);
                break;
            case kRemovePeerMsgRes:
                console.log(`[RTCNode ${this._id}] Received  ${jobj.cmd} command  result ${jobj.result}.`);
                this._listener.onRTCNodeRemovePeerRes(this, jobj);
                break;
            case kAnswerMsg:
                this._listener.onRTCNodeAnswer(this, jobj);
                break;
            case kCandidateMsg:
                this._listener.onRTCNodeCandidate(this, jobj);
                break;
            case kIceStateMsg:
                this._listener.onRTCNodeIceState(this, jobj);
                break;
        }
    }

    onError(error:Error):void {

    }

    onClose(code:number, reason:Buffer): void {
        console.log(`RTCNode onClose`);
        this._listener.onRTCNodeClose(this);
    }

    onPing(data:Buffer):void {

    }

    onPong(data:Buffer):void {

    }
}
