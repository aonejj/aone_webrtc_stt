import numpy as np
import zmq
import os
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
import threading
from faster_whisper import WhisperModel
from scipy.io import wavfile
import wave

IPC_PATH = "/tmp/stt_server.ipc"
PCM_SAMPLE_RATE = 16000
PCM_SAMPLE_WIDTH = 2  # 16-bit PCM


#model = WhisperModel("tiny", compute_type="int8", cpu_threads=4)
#model = WhisperModel("base", compute_type="int8", cpu_threads=4)
#model = WhisperModel("small", compute_type="int8", cpu_threads=4)
model = WhisperModel("medium", compute_type="int8", cpu_threads=4)


client_pcm_buffers = defaultdict(bytearray)
client_locks = defaultdict(threading.Lock)


if os.path.exists(IPC_PATH):
    os.remove(IPC_PATH)

context = zmq.Context()
socket = context.socket(zmq.ROUTER)
socket.bind(f"ipc://{IPC_PATH}")
os.chmod(IPC_PATH, 0o666)

executor = ThreadPoolExecutor(max_workers=4)

file_idx = 1

def handle_event(identity, client_id, event_type, pcm_data):
    global file_idx
    with client_locks[client_id]:
        if event_type == "start":
            print(f"[{client_id}] START")
            client_pcm_buffers[client_id] = bytearray(pcm_data)

        elif event_type == "speeching":
            client_pcm_buffers[client_id].extend(pcm_data)

        elif event_type == "end":
            print(f"[{client_id}] END - STT 시작")
            buffer = client_pcm_buffers[client_id]
            if len(buffer) > 0:
                try:
                    print("stt transcribe buflen "+ str(len(buffer)))
                    wav_filename = f"output_{file_idx}.wav"
                    file_idx += 1

                    pcm_int16 = np.frombuffer(buffer, dtype=np.int16)

                    if True:
                        pcm_np = pcm_int16.astype(np.float32) / 32768.0
                        segments, _ = model.transcribe(pcm_np, language="ko", beam_size=5, patience=1.2)

                        if not segments:
                            print("인식된 segment 없음 (segments is empty)")
                        else:
                            for seg in segments:
                                print("🗣️", seg.text)

                        full_text = "".join([seg.text for seg in segments]).strip()
                        print(f"[{client_id}] ✅ [final] {full_text}")

                        if False:
                            socket.send(identity, zmq.SNDMORE)
                            socket.send(b"", zmq.SNDMORE)
                            socket.send_string("[final] " + full_text)

                except Exception as e:
                    print(f"[{client_id}] STT 오류: {e}")

            client_pcm_buffers[client_id] = bytearray()

print(f" STT 서버 실행 중 (IPC: {IPC_PATH})")

try:
    while True:
        frames = []

        while True:
            part = socket.recv()
            frames.append(part)
            if not socket.getsockopt(zmq.RCVMORE):
                break


        identity = frames[0]
        client_id = frames[1].decode("utf-8")
        event_type = frames[2].decode("utf-8")
        if event_type == "end":
            pcm_data = b""
        else:
            pcm_data = frames[3]

        executor.submit(handle_event, identity, client_id, event_type, pcm_data)

except KeyboardInterrupt:
    print("\n 서버 종료 중...")

finally:
    socket.close()
    context.term()
    if os.path.exists(IPC_PATH):
        os.remove(IPC_PATH)
