import argparse
import yt_dlp

parser = argparse.ArgumentParser()

parser.add_argument("-f", "--format", type=str, required=True)
parser.add_argument("-c", "--cookies", type=str, required=True)
parser.add_argument("-u", "--url_suffix", type=str, required=True)
parser.add_argument("-o", "--output", type=str, required=True)

args = parser.parse_args()

def download_video(url, cookie_file, output_file):
    ydl_opts = {
        'format': 'bestvideo[ext=mp4]+bestaudio[ext=m4a]/best[ext=mp4]',
        'outtmpl': output_file,         
        'cookiefile': cookie_file,             
        'quiet': True,
        'no_warnings': True,
    }
    
    with yt_dlp.YoutubeDL(ydl_opts) as ydl:
        ydl.download([url])

def download_audio(url, cookie_file, output_file):
    ydl_opts = {
        'format': 'bestaudio[ext=m4a]/best',             
        'outtmpl': f"{output_file}.mp3",         
        'cookiefile': cookie_file,             
        'quiet': True,
        'no_warnings': True,
        'extractaudio': True,                 
        'audioformat': 'mp3',                 
        'audioquality': 192,
    }
    
    with yt_dlp.YoutubeDL(ydl_opts) as ydl:
        ydl.download([url])

url = f"https://www.youtube.com{args.url_suffix}"  
cookie_file = args.cookies
output_file = args.output

if args.format == "MP3":
	download_audio(url, cookie_file, output_file)  
elif args.format == "MP4":
	download_video(url, cookie_file, output_file)  
