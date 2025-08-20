package com.example.llm_client_app.signaling_transport;

import okio.ByteString;
public interface SignalingTransportListener {
    public void onSignalingTransportOpen();
    public void onSignalingTransportMessage(String message);
    public void onSignalingTransportMessage(ByteString bytes);
    public void onSignalingTransportClosing(int code, String reason);
    public void onSignalingTransportClosed(int code, String reason);
    public void onSignalingTransportFailure(String reason);
}
