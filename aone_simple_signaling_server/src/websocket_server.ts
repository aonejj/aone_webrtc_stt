import { WebSocket, WebSocketServer } from "ws";
import type { IncomingMessage } from 'http'

export enum S_WS_RET {
    S_WS_OK,
    S_WS_E_NEW_WEBSOCKET_SERVER,
    S_WS_E_AREADY_START,
}

export enum S_WS_STATE {
    STATE_IDLE,
    STATE_RUN,
    STATE_ERROR,        
}

export interface ISignalingWebsocketServerListener {
    onWebsocketServerListening(port:number): void;
    onConnectPeer(ws:WebSocket): void;
    onConnectNode(ws:WebSocket): void;
    onWebsocketServerClose(): void;
    onWebSocketServerError(err:Error):void;
}

export class SignalingWebsocketServer {
    private _port: number = 0;
    private _listener: ISignalingWebsocketServerListener | null = null;
    private _wss: WebSocketServer | null = null;
    private _state:S_WS_STATE = S_WS_STATE.STATE_IDLE;

    constructor(listener:ISignalingWebsocketServerListener) {
        this._listener = listener;
    }

    public start(port: number): S_WS_RET {
        console.log('SignalingWebsocketServer start');
        if(this._state !== S_WS_STATE.STATE_IDLE) {
            return S_WS_RET.S_WS_E_AREADY_START;
        }

        this._port = port;
        let newWss: WebSocketServer | null = null;

        try {
            newWss = new WebSocketServer({port:this._port, host: '0.0.0.0'});
            newWss.on('connection', this.onConnection.bind(this));
            newWss.on('error', this.onError.bind(this));
            newWss.on('listening', this.onListening.bind(this));
            newWss.on('close', this.onClose.bind(this));
            this._wss = newWss;

            return S_WS_RET.S_WS_OK;
        } catch(error: any) {
            this._wss = null;
            return S_WS_RET.S_WS_E_NEW_WEBSOCKET_SERVER;
        }
    }

    public stop(): void {
        console.log('SignalingWebsocketServer stop');
        if(this._state !== S_WS_STATE.STATE_RUN) {
            return;
        }

        if(this._wss !== null) {
            this._wss.close();
        }
    }

    private onConnection(ws:WebSocket, request:IncomingMessage): void {
//        const ctype = request.headers['x-type'];
        if(request.url === '/peer') {
            if(this._listener !== null) {
                this._listener.onConnectPeer(ws);
            }
        } else if (request.url === '/node') {
            if(this._listener !== null) {
                this._listener.onConnectNode(ws);
            }
        } else {
            console.log(`[WebSocketServer] Unknown type or path connection from: ${request.url || 'N/A'}`);
            ws.close(1008, "Invalid connection path");
        }
    }

    private onError(error:Error): void {
        this._state = S_WS_STATE.STATE_ERROR;
        if(this._listener !== null) {
            this._listener.onWebSocketServerError(error);
        }
    }

    private onListening(): void {
        console.log('SignalingWebsocketServer onListening');
        this._state = S_WS_STATE.STATE_RUN;

        if(this._listener !== null) {
            this._listener.onWebsocketServerListening(this._port);
        }
    }

    private onClose(): void {
        console.log('SignalingWebsocketServer onClose');
        this._wss = null;
        this._state = S_WS_STATE.STATE_IDLE;        
        if(this._listener !== null) {
            this._listener.onWebsocketServerClose();
        }
    }
}