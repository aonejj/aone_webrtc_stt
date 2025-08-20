from faster_whisper import WhisperModel

def download_small_model():
    model = WhisperModel("small", compute_type="int8", cpu_threads=4)
    print("download complete small model!!!")

if __name__ == "__main__":
    download_small_model()

