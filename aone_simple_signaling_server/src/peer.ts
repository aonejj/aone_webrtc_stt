
import { WebSocket, RawData  } from "ws";
import { kRegistMsg, kOfferMsg, kCandidateMsg } from "./types/signaling_msg_type";

export interface IPeerListener {
    onPeerClose(peer:Peer): void;
    onPeerRegist(peer:Peer, jobj:any): void;
    onPeerOffer(peer:Peer, jobj:any): void;
    onPeerCandidate(peer:Peer, jobj:any): void;
}

export class Peer {
    private readonly  _ws:WebSocket;
    private readonly  _id:string;
    private readonly  _listener: IPeerListener;
    private _rtc_node_id:string | null = null;

    constructor(ws:WebSocket, id:string, listener:IPeerListener) {
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

    setRTCNodeId(nodeId:string):void {
        this._rtc_node_id = nodeId;
    }

    getRTCNodeId():string | null {
        return this._rtc_node_id;
    }

    send(jobj:any):void {
        if(!this.isOpen()) {
            return;
        }

        try {
            const msg = JSON.stringify(jobj);
            this._ws.send(msg);
    //        console.log(`[Peer ${this._id}] Sent: ${msg}`);
        } catch (error) {
            console.log(`[Peer ${this._id}] Failed to stringify message for sending: `, jobj, error);
        }
    }

    close():void {
        if(!this.isOpen()) {
            return;
        }

        this._ws.close();
    }

    onMessage(data: RawData, isBinary:boolean):void {
        if(isBinary) {
            console.warn(`[Peer ${this._id}] Received binary data, ignoring.`);
            return;
        }

        const msg = data.toString();
        let jobj:any;

        try {
            jobj = JSON.parse(msg);
        } catch(error) {
            console.error(`[Peer ${this._id}] Failed to parse incoming message as JSON: ${msg}. Error: ${error}`);
            return;
        }

        console.log(`[Peer ${this._id}] Received ${jobj.cmd} command.`);

        switch(jobj.cmd) {
            case kRegistMsg:
                this._listener.onPeerRegist(this, jobj);
                break;

            case kOfferMsg:
                this._listener.onPeerOffer(this, jobj);
                break;
                
            case kCandidateMsg:
                this._listener.onPeerCandidate(this, jobj);
                break;
        }
    }

    onError(error:Error):void {

    }

    onClose(code:number, reason:Buffer): void {
        console.log(`Peer onClose`);
        this._listener.onPeerClose(this);
    }

    onPing(data:Buffer):void {

    }

    onPong(data:Buffer):void {

    }
}