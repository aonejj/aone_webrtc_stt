package com.example.llm_client_app.signaling_transport;

import android.util.Log;

import java.util.concurrent.TimeUnit;

import androidx.annotation.Nullable;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;
import okhttp3.WebSocket;
import okhttp3.WebSocketListener;
import okio.ByteString;

public class SignalingTransportSession implements SignalingTransportInterface {

    private static final String TAG = "SignalingTransportSession";


    @Nullable
    private okhttp3.WebSocket webSocket = null;

    @Nullable
    private SignalingTransportListener listener;

    private final Object listenerLock = new Object();

    private SignalingTransportState state = SignalingTransportState.SIGNALING_TRANSPORT_INIT;

    public SignalingTransportSession() {

    }

    @Override
    public int openSignalingTransport(String url) {
        if(state != SignalingTransportState.SIGNALING_TRANSPORT_INIT &&
            state != SignalingTransportState.SIGNALING_TRANSPORT_CLOSED) {
            return -1;
        }
        synchronized (listenerLock) {
            state = SignalingTransportState.SIGNALING_TRANSPORT_CONNECTING;
        }

        OkHttpClient client = new OkHttpClient.Builder()
                .connectTimeout(10, TimeUnit.SECONDS)
                .readTimeout(10, TimeUnit.SECONDS)
                .writeTimeout(10, TimeUnit.SECONDS)
                .build();

        Request request = new Request.Builder().url(url).build();
        webSocket = client.newWebSocket(request, new WebSocketListener() {
            public void onOpen(WebSocket webSocket, Response response) {
                Log.d(TAG, "onOpen ");
                synchronized (listenerLock) {
                    state = SignalingTransportState.SIGNALING_TRANSPORT_CONNECTED;
                    if(listener != null) {
                        listener.onSignalingTransportOpen();
                    }
                }
            }

            public void onMessage(WebSocket webSocket, String text) {
                Log.d(TAG, "onMessage ");
                synchronized (listenerLock) {
                    if(listener != null) {
                        listener.onSignalingTransportMessage(text);
                    }
                }
            }

            public void onMessage(WebSocket webSocket, ByteString bytes) {
            }

            public void onClosing(WebSocket webSocket, int code, String reason) {
                Log.d(TAG, "onClosing ");
                synchronized (listenerLock) {
                    state = SignalingTransportState.SIGNALING_TRANSPORT_CLOSING;
                    if(listener != null) {
                        listener.onSignalingTransportClosing(code, reason);
                    }
                }
            }

            public void onClosed(WebSocket webSocket, int code, String reason) {
                Log.d(TAG, "onClosed ");
                synchronized (listenerLock) {
                    state = SignalingTransportState.SIGNALING_TRANSPORT_CLOSED;
                    if(listener != null) {
                        listener.onSignalingTransportClosed(code, reason);
                    }
                }
            }

            public void onFailure(WebSocket webSocket, Throwable t, Response response) {
                Log.d(TAG, "onFailure "+t.getMessage());
                synchronized (listenerLock) {
                    state = SignalingTransportState.SIGNALING_TRANSPORT_ERROR;
                    if(listener != null) {
                        listener.onSignalingTransportFailure(t.getMessage());
                    }
                }
            }
        });
        return 0;
    }

    @Override
    public int closeSignalingTransport() {
        if(webSocket != null) {
            webSocket.close(1000, "Normal Closure by client");
        }
        return 0;
    }

    @Override
    public int sendMessage(String msg) {
        if(webSocket != null) {
            webSocket.send(msg);
        }
        return 0;
    }

    @Override
    public SignalingTransportState getState() {
        return null;
    }

    @Override
    public void setSignalingTransportListener(SignalingTransportListener listener) {
        synchronized (listenerLock) {
            this.listener = listener;
        }
    }
}
