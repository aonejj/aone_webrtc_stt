import { EventEmitter } from 'events';
import { WebSocket } from 'ws';
import { v4 as uuidv4 } from 'uuid';

import {
    S_WS_RET, 
    S_WS_STATE, 
    ISignalingWebsocketServerListener, 
    SignalingWebsocketServer     
} from './websocket_server';

import { Peer, IPeerListener } from './peer'
import { RTCNode, IRTCNodeListener } from './rtcnode'
import { kAddPeerMsg,
         kFail, kSuccess, 
         kRegistMsg, kRegistMsgRes,
         kRemovePeerMsg,
         kOfferMsg,
         kCandidateMsg,
         kAnswerMsg,
         kGoogleStunUrl,
         type CommandMessage, 
         ResponseMessage, 
         SDPMessage, 
         CandidateMessage, 
         IceServerInfo,
         IceStateMessage } from './types/signaling_msg_type'

const kCreatePeer:string = "kCreatePeer";
const kCreateNode:string = "kCreateNode";

export class SignalingServer extends EventEmitter implements ISignalingWebsocketServerListener, IPeerListener, IRTCNodeListener {

    private readonly _peers = new Map<string, Peer>;
    private readonly _nodes = new Map<string, RTCNode>;
    private readonly _node_to_peers = new Map<RTCNode, Peer[]>;

    private _ws_server:SignalingWebsocketServer|null = null;

    constructor() {
        super();

        this.on(kCreatePeer, this.onCreatePeer.bind(this));
        this.on(kCreateNode, this.onCreateNode.bind(this));
    }

    startServer(port:number):void {
        if(this._ws_server !== null) {
            console.log(`SignalingServer websocket server already running`);
            return;
        }
        console.log(`SignalingServer startServer`);
        this._ws_server = new SignalingWebsocketServer(this);
        this._ws_server.start(port);
    }

    stopServer(): void {
        if(this._ws_server === null) {
            console.log(`SignalingServer websocket server already stop`);
            return;
        }
        console.log(`SignalingServer stopServer`);
        this._ws_server.stop();
    }

    onWebsocketServerListening(port: number): void {
        console.log('SignalingServer onListening port: ', port);
    }

    onConnectPeer(ws: WebSocket): void {
        console.log(`onConnectPeer`);
        const uuid:string = uuidv4();
        let peer:Peer = new Peer(ws, uuid, this);
        this._peers.set(uuid, peer);
        this.emit(kCreatePeer, peer);    
    }

    onConnectNode(ws: WebSocket): void {
        console.log(`onConnectNode`);
        const uuid:string = uuidv4();
        let node:RTCNode = new RTCNode(ws, uuid, this);
        this._nodes.set(uuid, node);
        this.emit(kCreateNode, node);
    }

    onWebsocketServerClose(): void {
        console.log(`SignalingServer onWebsocketServerClose`);
        this._ws_server = null;
    }
    onWebSocketServerError(err: Error): void {
        console.log(`SignalingServer onWebSocketServerError`);
    }

    onPeerClose(peer:Peer): void {
        this._peers.delete(peer.getId());
        let node_id:string|null = peer.getRTCNodeId();
        if(node_id !== null) {
            let remove_peer_cmd:CommandMessage = {
                cmd:kRemovePeerMsg,
                id:peer.getId()
            };

            const node:RTCNode|null|undefined =  this._nodes.get(node_id);
            if(node !== null && node != undefined) {
                this._nodes.get(node_id)!.send(remove_peer_cmd);
                this.node_to_peer_remove(node, peer);
            }
        }
    }

    onPeerRegist(peer:Peer, jobj:any): void {
        const entry = this._nodes.entries().next().value;
        if(entry) {
            const [key, node] = entry;
            peer.setRTCNodeId(key);

            let add_peer_cmd:CommandMessage = {
                cmd:kAddPeerMsg,
                id: peer.getId()
            };
            node.send(add_peer_cmd);

            this.node_to_peer_add(node, peer);

            const stun_server:IceServerInfo = {
                url:kGoogleStunUrl
            };

            let regist_res_msg:ResponseMessage ={
                cmd: kRegistMsgRes,
                result:kSuccess,
                iceinfo:[stun_server]
            };
            peer.send(regist_res_msg);

        } else {
            let regist_res_msg:ResponseMessage ={
                cmd: kRegistMsgRes,
                result:kFail,
            };
            peer.send(regist_res_msg);             
        }
    }

    onPeerOffer(peer:Peer, jobj:any): void {
        const node:RTCNode = this._nodes.get(peer.getRTCNodeId()!)!;
        const offer_msg:SDPMessage = {
            cmd: kOfferMsg,
            id: peer.getId(),
            sdp: jobj.sdp 
        };

        node.send(offer_msg);
    }   

    onPeerCandidate(peer:Peer, jobj:any):void {
        const node:RTCNode = this._nodes.get(peer.getRTCNodeId()!)!;
        const candidate_msg:CandidateMessage = {
            cmd: kCandidateMsg,
            id: peer.getId(),
            mline_index: jobj.mline_index,
            mid: jobj.mid,
            candidate: jobj.candidate
        };

        node.send(candidate_msg);
    }

    onRTCNodeRegist(node:RTCNode, jobj:any): void {
        console.log(`onRTCNodeRegist`);
        const stun_server:IceServerInfo = {
            url:kGoogleStunUrl
        };
        const node_id = node.getId();

        let regist_res_msg:ResponseMessage ={
            cmd: kRegistMsgRes,
            result:kSuccess,
            iceinfo:[stun_server],
            id:node_id
        };
        node.send(regist_res_msg);
    }

     onRTCNodeAddPeerRes(node:RTCNode, jobj:any):void {
        const peer_id = jobj.id;
        const result = jobj.result;
        if(result === "fail") {
            const peer = this._peers.get(peer_id);
            peer?.close();
            this.node_to_peer_remove(node, peer);
        }
     }

    onRTCNodeRemovePeerRes(node:RTCNode, jobj:any):void {

    }

    onRTCNodeAnswer(node:RTCNode, jobj:any):void {
        const peer_id = jobj.id;
        const peer = this._peers.get(peer_id);
        if(peer !== null && peer !== undefined) {
            let answer_msg:SDPMessage = {
                cmd:kAnswerMsg,
                sdp: jobj.sdp 
            }
            peer.send(answer_msg);
        }
    }

    onRTCNodeCandidate(node:RTCNode, jobj:any):void {
        const peer_id = jobj.id!;
        const peer = this._peers.get(peer_id);
        if(peer !== null && peer !== undefined) {
            let candidate_msg:CandidateMessage = {
                cmd:kCandidateMsg,
                mline_index: jobj.mline_index,
                mid: jobj.mid,
                candidate: jobj.candidate
            }
            peer.send(candidate_msg);
        }
    }

    onRTCNodeIceState(node:RTCNode, jobj:any):void {
        console.log(`[RTCNode ${node.getId()}] IceState ${jobj.state} desc ${jobj.desc}.`);
    }

    onRTCNodeClose(node:RTCNode): void {
        this._nodes.delete(node.getId());

        const peers =  this._node_to_peers.get(node);
        if(peers) {
            for(const peer of peers) {
                peer.close();
            }
        }

        this._node_to_peers.delete(node);
    }

    async onCreatePeer(peer:Peer): Promise<void> {
        await Promise.resolve();
        console.log(`onCreatePeer`);
        if(peer.isOpen() === false) {
            console.log(`onCreatePeer peer !isOpen`);
            this._peers.delete(peer.getId());
        }
    }

    async onCreateNode(node:RTCNode): Promise<void> {
        await Promise.resolve();

        console.log(`onCreateNode`); 

        if(node.isOpen() === false) {
            this._nodes.delete(node.getId());
        }
    }

    private node_to_peer_add(node:RTCNode, peer:Peer):void {
        if(!this._node_to_peers.has(node)) {
            this._node_to_peers.set(node, []);
        }
        this._node_to_peers.get(node)?.push(peer);
    }    

    private node_to_peer_remove(node:RTCNode, peer?:Peer):void {
        if (!peer) {
            return;
        }

        if(!this._node_to_peers.has(node)) {
            return;
        }
        const idx = this._node_to_peers.get(node)?.findIndex(item=>item === peer);
        if(idx !== -1 && idx != undefined) {
            this._node_to_peers.get(node)?.splice(idx, 1);
        }
    }
}