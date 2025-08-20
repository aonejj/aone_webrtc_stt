package com.example.llm_client_app.signaling_transport;

public interface SignalingTransportInterface {
    enum SignalingTransportState {
        SIGNALING_TRANSPORT_INIT,
        SIGNALING_TRANSPORT_CONNECTING,
        SIGNALING_TRANSPORT_CONNECTED,
        SIGNALING_TRANSPORT_CLOSING,
        SIGNALING_TRANSPORT_CLOSED,
        SIGNALING_TRANSPORT_ERROR,
    }

    public int openSignalingTransport(String url);
    public int closeSignalingTransport();

    public int sendMessage(String msg);

    public SignalingTransportState getState();

    public void setSignalingTransportListener(SignalingTransportListener listener);
}
