from faster_whisper import WhisperModel

def download_base_model():
    model = WhisperModel("base", compute_type="int8", cpu_threads=4)
    print("download complete base model!!!")

if __name__ == "__main__":
    download_base_model()

