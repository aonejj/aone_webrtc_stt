from faster_whisper import WhisperModel

def download_tiny_model():
    model = WhisperModel("tiny", compute_type="int8", cpu_threads=4)
    print("download complete tiny model!!!")

if __name__ == "__main__":
    download_tiny_model()

