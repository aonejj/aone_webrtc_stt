# aone_webrtc_stt (STT Pilot)

## Overview
This project demonstrates a **WebRTC-based voice-to-text pipeline**.  
A native C++ server receives audio via WebRTC, decodes Opus into PCM, applies noise suppression and VAD, and forwards it to a Python-based STT engine (`faster-whisper`).  
For this pilot, transcription results are printed to the console only.

---

## Open Source / Dependencies

### 1. RTCPeerConnection (C++17, Google WebRTC Native)
- **Static libraries:**  
  - absl (Google Abseil C++)  
  - BoringSSL (TLS/DTLS)  
  - usrsctp (SCTP over DTLS for DataChannel)  
- **Role:**  
  - Modified Google WebRTC Native for server-side PeerConnection  
  - Handles RTP/RTCP and media transport

### 2. RTCVoiceSDK (C++17)
- **Static libraries:**  
  - opus (Opus codec decoding)  
- **Role:**  
  - Abstracts PeerConnection complexity  
  - Performs Opus → PCM decoding  
  - Provides PCM callback to `RTCServerNode`  
  - Offers simple API: `connect()`, `start()`, `stop()`

### 3. RTCServerNode (C++17)
- **Static libraries:**  
  - websocket (signaling communication)  
  - jansson (JSON parsing)  
  - rnnoise (noise suppression + VAD)  
  - Android AOSP foundation (Handler, Thread, Time utilities)  
- **Shared libraries:**  
  - ZeroMQ (PCM data transport to Python STT server)  
- **Role:**  
  - Manages signaling integration and PeerConnection lifecycle  
  - Receives PCM callback from `RTCVoiceSDK`  
  - Applies noise suppression & VAD using RNNoise  
  - Sends PCM chunks to `aone_simple_stt_server`  
  - Future: return STT results to client

### 4. aone_simple_stt_server (Python)
- **Dependencies:** faster-whisper  
- **Role:**  
  - Receives PCM chunks from `RTCServerNode`  
  - Performs STT transcription  
  - Outputs results to console (for now)

---

## Architecture
Client (WebRTC)
│
▼
aone_simple_signaling_server (TypeScript)
│
▼
RTCServerNode (C++17)
│ manages
▼
RTCVoiceSDK (C++17)
│
▼
RTCPeerConnection (Google WebRTC Native, modified)
│
▼
PCM callback → RTCServerNode → RNNoise (noise suppression + VAD)
│
▼
aone_simple_stt_server (Python, faster-whisper)

## Data Flow
1. Client connects to `aone_simple_signaling_server` via WebSocket.  
2. `RTCServerNode` negotiates a WebRTC session with the client.  
3. `RTCPeerConnection` receives Opus audio packets.  
4. `RTCVoiceSDK` decodes Opus → PCM, provides callback to `RTCServerNode`.  
5. `RTCServerNode` applies RNNoise noise suppression and VAD.  
6. PCM chunks are forwarded to `aone_simple_stt_server`.  
7. STT server transcribes audio chunks using faster-whisper and prints results.

---

## Current Status
- ✅ End-to-end audio → text pipeline verified  
- ✅ STT output printed to console  
- ⬜ Future: return transcription results to client (via signaling or DataChannel)

