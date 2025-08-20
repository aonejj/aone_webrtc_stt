from faster_whisper import WhisperModel

def download_medium_model():
    model = WhisperModel("medium", compute_type="int8", cpu_threads=4)
    print("download complete medium model!!!")

if __name__ == "__main__":
    download_medium_model()

