from pydub import AudioSegment
import os

def MP32WAV(mp3_path, wav_path):
    """
    这是MP3文件转化成WAV文件的函数
    :param mp3_path: MP3文件的地址
    :param wav_path: WAV文件的地址
    """
    MP3_File = AudioSegment.from_mp3(file=mp3_path).set_frame_rate(32000)
    MP3_File.export(wav_path, format="wav", bitrate='32k')
# MP32WAV('F:/mp3/3.mp3','F:/mp3/3.wav')

def run_main():

    # MP3文件和WAV文件的地址
    path1 = 'F:\myesp\master\esp-idf\pro\s3_lvgl_v7\mp3'
    path2 = "F:\wav"
    paths = os.listdir(path1)
    mp3_paths = []
    # 获取mp3文件的相对地址
    for mp3_path in paths:
        mp3_paths.append(path1 + "/" + mp3_path)
    # print(mp3_paths)

    # 得到MP3文件对应的WAV文件的相对地址
    wav_paths = []
    for mp3_path in mp3_paths:
        print(mp3_path)
        wav_path = path2 + "/" + mp3_path[1:].split('.')[0].split('/')[-1] + '.wav'
        print(wav_path)
        wav_paths.append(wav_path)
    print(wav_paths)

    # 将MP3文件转化成WAV文件
    for (mp3_path, wav_path) in zip(mp3_paths, wav_paths):

        # print(mp3_path)
        # print(mp3_paths)
        MP32WAV(mp3_path, wav_path)

if __name__ == '__main__':
    run_main()