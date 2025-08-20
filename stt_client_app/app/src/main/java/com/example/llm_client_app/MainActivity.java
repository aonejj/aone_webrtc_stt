package com.example.llm_client_app;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import okio.ByteString;

import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.example.llm_client_app.signaling_transport.SignalingTransportListener;
import com.example.llm_client_app.signaling_transport.SignalingTransportSession;

import org.json.JSONException;
import org.json.JSONObject;

import org.webrtc.MediaStream;
import org.webrtc.PeerConnection;
import org.webrtc.PeerConnectionFactory;
import org.webrtc.MediaConstraints;
import org.webrtc.AudioSource;
import org.webrtc.AudioTrack;
import org.webrtc.IceCandidate;
import org.webrtc.SessionDescription;
import org.webrtc.SdpObserver;
import org.webrtc.RtpReceiver;
import org.webrtc.RtpTransceiver;
import org.webrtc.DataChannel;

import java.util.ArrayList;


public class MainActivity extends AppCompatActivity {
    private static final String TAG = "xxx_MainActivity";

    private EditText urlInputField;
    private Button connectButton;
    private Button webrtcConnectButton;

    private SignalingTransportSession signalingTransportSession;

    private HandlerThread workerThread;
    private Handler workerHandler;
    private Looper workerLooper;

    private boolean _connect_req = true;

    private static final int SIGNAING_WORKER_MESSAGE_ON_CONNECT = 1000;
    private static final int SIGNAING_WORKER_MESSAGE_ON_CLOSING = 1001;
    private static final int SIGNAING_WORKER_MESSAGE_ON_CLOSED = 1002;
    private static final int SIGNAING_WORKER_MESSAGE_ON_ERROR = 1003;
    private static final int SIGNAING_WORKER_MESSAGE_ON_MESSAGE = 1004;

    private static final int SIGNAING_WORKER_MESSAGE_CLOSE = 1005;

    //////////////////////////////////////////////////////
    // WebRTC
    private PeerConnectionFactory peerConnectionFactory;
    private PeerConnection peerConnection;
//    private MediaStream mediaStream;
    private AudioTrack localAudioTrack;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        urlInputField = findViewById(R.id.url_input_field);
        connectButton = findViewById(R.id.connect_button);
        webrtcConnectButton = findViewById(R.id.webrtc_connect_button);

        if (checkSelfPermission(android.Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{android.Manifest.permission.RECORD_AUDIO}, 1001);
        }


        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(_connect_req) {
                    String url = urlInputField.getText().toString();
                    if (url.isEmpty()) {
                        Toast.makeText(MainActivity.this, "URL을 입력해주세요.", Toast.LENGTH_SHORT).show();
                    } else {
                        // TODO: 여기에 WebSocket 접속 로직을 구현하세요.
                        connectButton.setEnabled(false);
                        signalingTransportSession.openSignalingTransport(url);
                    }
                } else {
                    workerHandler.sendMessage(workerHandler.obtainMessage(SIGNAING_WORKER_MESSAGE_CLOSE));
                }
            }
        });

        // Set up click listener for the "WebRTC 연결" (WebRTC Connect) button
        webrtcConnectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // TODO: 여기에 WebRTC 연결 로직을 구현하세요.
                Toast.makeText(MainActivity.this, "WebRTC 연결 시도", Toast.LENGTH_SHORT).show();
            }
        });

        // Optional: Set a default URL for convenience during development
        urlInputField.setText("ws://192.168.0.22:8080/peer"); // Or your preferred default URL

        // Initialize SignalingTransportSession
        signalingTransportSession = new SignalingTransportSession();

        // Set the listener for SignalingTransportSession
        signalingTransportSession.setSignalingTransportListener(new SignalingTransportListener() {
            @Override
            public void onSignalingTransportOpen() {
                Log.d(TAG, "Signaling Transport: Connection Opened");
                Message msg = workerHandler.obtainMessage(SIGNAING_WORKER_MESSAGE_ON_CONNECT);
                workerHandler.sendMessage(msg);
            }

            @Override
            public void onSignalingTransportMessage(String message) {
                Log.d(TAG, "Signaling Transport: Message Received: " + message);
                Message msg = workerHandler.obtainMessage(SIGNAING_WORKER_MESSAGE_ON_MESSAGE, message);
                workerHandler.sendMessage(msg);
            }

            @Override
            public void onSignalingTransportMessage(ByteString bytes) {

            }

            @Override
            public void onSignalingTransportClosing(int code, String reason) {
                Log.d(TAG, "Signaling Transport: Connection Closing. Code: " + code + ", Reason: " + reason);
                Message msg = workerHandler.obtainMessage(SIGNAING_WORKER_MESSAGE_ON_CLOSING);
                workerHandler.sendMessage(msg);
            }

            @Override
            public void onSignalingTransportClosed(int code, String reason) {
                Log.d(TAG, "Signaling Transport: Connection Closed. Code: " + code + ", Reason: " + reason);
                Message msg = workerHandler.obtainMessage(SIGNAING_WORKER_MESSAGE_ON_CLOSED);
                workerHandler.sendMessage(msg);
            }

            @Override
            public void onSignalingTransportFailure(String error) {
                Log.e(TAG, "Signaling Transport: Connection Failure: " + error);
                Message msg = workerHandler.obtainMessage(SIGNAING_WORKER_MESSAGE_ON_ERROR);
                workerHandler.sendMessage(msg);
            }
        });

        workerThread = new HandlerThread("WorkerThreadForSignalingProcessing");
        workerThread.start();
        workerLooper = workerThread.getLooper();

        workerHandler = new Handler(workerLooper) {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case SIGNAING_WORKER_MESSAGE_ON_CONNECT:
                    {
                        _regist_req();
                        break;
                    }
                    case SIGNAING_WORKER_MESSAGE_ON_CLOSING:
                    {
                        _on_closing();
                        break;
                    }
                    case SIGNAING_WORKER_MESSAGE_ON_CLOSED:
                    {
                        _on_closed();
                        break;
                    }
                    case SIGNAING_WORKER_MESSAGE_ON_ERROR:
                    {
                        _on_error();
                        break;
                    }
                    case SIGNAING_WORKER_MESSAGE_ON_MESSAGE:
                    {
                        _on_message((String)msg.obj);
                        break;
                    }
                    case SIGNAING_WORKER_MESSAGE_CLOSE:
                    {
                        signalingTransportSession.closeSignalingTransport();
                    }
                }
            }
        };

        PeerConnectionFactory.initialize(
                PeerConnectionFactory.InitializationOptions.builder(this)
                        .setFieldTrials("WebRTC-Audio-Agc3/Enabled/"
                        + "WebRTC-Audio-Aec3/Enabled/"
                        + "WebRTC-Audio-NoiseCancellation/Enabled/")
                        .createInitializationOptions()
        );

        peerConnectionFactory = PeerConnectionFactory.builder().createPeerConnectionFactory();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (workerThread != null) {
            workerThread.quitSafely();
            try {
                workerThread.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                Log.e(TAG, "Worker thread join interrupted", e);
            }
        }
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 1001) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                Log.d(TAG, "RECORD_AUDIO permission granted");
            } else {
                Log.e(TAG, "RECORD_AUDIO permission denied");
                Toast.makeText(this, "마이크 권한이 필요합니다", Toast.LENGTH_SHORT).show();
            }
        }
    }



    private void _regist_req() {
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("cmd", "regist"); // "cmd": "regist" 추가
            String jsonString = jsonObject.toString(); // JSON 객체를 문자열로 변환

            // SignalingTransportSession을 통해 메시지 전송
            int result = signalingTransportSession.sendMessage(jsonString);

            if (result != 0) {
                Log.e(TAG, "Failed to send JSON. Result code: " + result);
            }

        } catch (JSONException e) {
            Log.e(TAG, "Failed to create JSON object: " + e.getMessage());
        }
    }

    private void _on_closing() {

    }

    private void _on_closed() {
        _connect_req = true;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                connectButton.setText("connect");
                connectButton.setEnabled(true);
            }
        });
    }

    private void _on_error() {
        _connect_req = true;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                connectButton.setText("connect");
                connectButton.setEnabled(true);
            }
        });
    }

    private void _on_message(String msg) {
        try {
            JSONObject jsonObject = new JSONObject(msg);
            String cmd = jsonObject.getString("cmd");
            switch(cmd) {
                case "regist_res":
                {
                    _on_regist_res(jsonObject);
                    break;
                }
                case "answer":
                {
                    _on_answer_res(jsonObject);
                    break;
                }
                case "candidate":
                {
                    _on_ice_candidate(jsonObject);
                    break;
                }
            }

        } catch (JSONException e) {
            throw new RuntimeException(e);
        }
    }

    private void _on_regist_res(JSONObject jobj) {
        try {
            _connect_req = false;
            String result = jobj.getString("result");

            if(result.equals("success")) {
                Log.d(TAG, "regist success");
                runOnUiThread(() -> {
                    _connect_req = false;
                    connectButton.setText("disconnect");
                    connectButton.setEnabled(true);
                    startWebRTCConnection();
                });
            } else {
                Log.d(TAG, "regist fail");
                workerHandler.sendMessage(workerHandler.obtainMessage(SIGNAING_WORKER_MESSAGE_CLOSE));
            }

        } catch (JSONException e) {
            throw new RuntimeException(e);
        }
    }

    private void _on_answer_res(JSONObject jobj) {
        try {
            String sdp = jobj.getString("sdp");
            SessionDescription answer = new SessionDescription(SessionDescription.Type.ANSWER, sdp);
            peerConnection.setRemoteDescription(new SdpObserver() {
                @Override
                public void onSetSuccess() {
                    Log.d(TAG, "Remote SDP set success");
                }
                @Override
                public void onSetFailure(String s) {
                    Log.e(TAG, "Remote SDP set failure: " + s);
                }
                @Override
                public void onCreateSuccess(SessionDescription sdp) {}
                @Override
                public void onCreateFailure(String s) {}
            }, answer);
        } catch (JSONException e) {
            throw new RuntimeException(e);
        }

    }

    private void _on_ice_candidate(JSONObject jobj) {
        try {
            String sdpMid = jobj.getString("mid");
            int sdpMLineIndex = jobj.getInt("mline_index");
            String candidate = jobj.getString("candidate");
            IceCandidate iceCandidate = new IceCandidate(sdpMid, sdpMLineIndex, candidate);
            peerConnection.addIceCandidate(iceCandidate);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private void startWebRTCConnection() {
        Log.d(TAG, "startWebRTCConnection");
        PeerConnection.RTCConfiguration rtcConfig = new PeerConnection.RTCConfiguration(new ArrayList<>());
        rtcConfig.sdpSemantics = PeerConnection.SdpSemantics.UNIFIED_PLAN;

        peerConnection = peerConnectionFactory.createPeerConnection(rtcConfig, new PeerConnection.Observer() {
            @Override
            public void onSignalingChange(PeerConnection.SignalingState signalingState) {
                Log.d(TAG, "kimi SignalingState " + signalingState);
            }

            @Override
            public void onIceConnectionChange(PeerConnection.IceConnectionState iceConnectionState) {
                Log.d(TAG, "kimi IceConnectionState " + iceConnectionState);
            }

            @Override
            public void onIceConnectionReceivingChange(boolean b) {

            }

            @Override
            public void onIceGatheringChange(PeerConnection.IceGatheringState iceGatheringState) {

            }

            @Override
            public void onIceCandidate(IceCandidate iceCandidate) {
                try {
                    JSONObject json = new JSONObject();
                    json.put("cmd", "candidate");
                    json.put("mid", iceCandidate.sdpMid);
                    json.put("mline_index", iceCandidate.sdpMLineIndex);
                    json.put("candidate", iceCandidate.sdp);
                    signalingTransportSession.sendMessage(json.toString());
                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }

            @Override
            public void onIceCandidatesRemoved(IceCandidate[] iceCandidates) {

            }

            @Override
            public void onAddStream(MediaStream mediaStream) {

            }

            @Override
            public void onRemoveStream(MediaStream mediaStream) {

            }

            @Override
            public void onDataChannel(DataChannel dataChannel) {

            }

            @Override
            public void onRenegotiationNeeded() {

            }
        });


        AudioSource audioSource = peerConnectionFactory.createAudioSource(new MediaConstraints());
        localAudioTrack = peerConnectionFactory.createAudioTrack("audioTrack", audioSource);
        peerConnection.addTrack(localAudioTrack);

        MediaConstraints offerConstraints = new MediaConstraints();
        offerConstraints.mandatory.add(new MediaConstraints.KeyValuePair("OfferToReceiveAudio", "true"));
        offerConstraints.mandatory.add(new MediaConstraints.KeyValuePair("OfferToReceiveVideo", "false"));

        peerConnection.createOffer(new SdpObserver() {
            @Override
            public void onCreateSuccess(SessionDescription sessionDescription) {

//                String sdpDescription = sessionDescription.description.replace("a=sendrecv", "a=sendonly");
//                SessionDescription modifiedSdp = new SessionDescription(sessionDescription.type, sdpDescription);

                peerConnection.setLocalDescription(new SdpObserver() {
                    @Override
                    public void onSetSuccess() {
                        try {
                            JSONObject json = new JSONObject();
                            json.put("cmd", "offer");
                            json.put("sdp", sessionDescription.description);
                            signalingTransportSession.sendMessage(json.toString());
                        } catch (JSONException e) {
                            e.printStackTrace();
                        }
                    }
                    @Override
                    public void onSetFailure(String s) { }
                    @Override
                    public void onCreateSuccess(SessionDescription sdp) { }
                    @Override
                    public void onCreateFailure(String s) { }
                }, sessionDescription);
            }

            @Override
            public void onSetSuccess() { }
            @Override
            public void onSetFailure(String s) { }
            @Override
            public void onCreateFailure(String s) {
                Log.e(TAG, "Offer creation failed: " + s);
            }
        }, offerConstraints);
    }
}