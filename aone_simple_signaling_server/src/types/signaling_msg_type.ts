// commands
export const kRegistMsg:string = "regist";
export const kRegistMsgRes:string = 'regist_res';

export const kAddPeerMsg:string = "add_peer";
export const kAddPeerMsgRes:string = "add_peer_res";

export const kRemovePeerMsg:string = "remove_peer"
export const kRemovePeerMsgRes:string = "remove_peer_res";

export const kOfferMsg:string = "offer";
export const kAnswerMsg:string = "answer";
export const kCandidateMsg:string = "candidate";

export const kIceStateMsg:string = "iceState"

// results
export const kSuccess:string = "success";
export const kFail:string = "fail";

export const kGoogleStunUrl:string = "stun:stun.l.google.com:19302";

export interface IceServerInfo {
    url:string;
    username?: string;
    credential?:string;
}

export interface BaseSignalingMessge {
    cmd:string;
}

export interface CommandMessage extends BaseSignalingMessge {
    cmd:string;
    id?:string;
}

export interface ResponseMessage extends BaseSignalingMessge {
    cmd:string;
    result:string;
    iceinfo?:IceServerInfo[];
    id?:string;
}

export interface SDPMessage extends BaseSignalingMessge {
    cmd:string;
    id?:string;
    sdp:string;
}

export interface CandidateMessage extends BaseSignalingMessge {
    cmd:string;
    id?:string;
    mline_index:number;
    mid:string;
    candidate:string;
}

export interface IceStateMessage extends BaseSignalingMessge {
    cmd:string;
    state:number;
    desc:string;
}