from flask import abort, Flask, redirect, render_template, request, url_for
from werkzeug.utils import secure_filename

import atexit
import os
import vlc

import RPi.GPIO as GPIO

app = Flask(__name__)
root_dir = "/music"
volume_button_on = False

class JukeboxQueue:
    queue = []

    def request(self, filename):
        try:
            index = self.queue.index(filename)
            if index == 0:
                return
            item = self.queue.pop(index)
            self.queue.insert(index - 1, item)
        except ValueError:
            self.queue.append(filename)

    def __len__(self):
        return len(self.queue)

    def get_contents(self):
        return list(self.queue)

    def pop(self):
        return self.queue.pop(0)

class VlcJukebox:
    def __init__(self, queue):
        self.instance = vlc.Instance()
        self.player = self.instance.media_player_new()
        event_manager = self.player.event_manager()
        event_manager.event_attach(vlc.EventType.MediaPlayerEndReached, self.song_ended_callback)
        self.queue = queue
        self.queue_ended = True
        self.current = None

    def new_song(self):
        self.current = queue.pop()
        media = self.instance.media_new(self.current)
        self.player.set_media(media)
        self.player.play()
    
    def queue_updated(self):
        if len(self.queue) > 0 and self.queue_ended:
            self.queue_ended = False
            self.new_song()

    def song_ended_callback(self, event):
        if len(self.queue) > 0:
            self.player = self.instance.media_player_new()
            event_manager = self.player.event_manager()
            event_manager.event_attach(vlc.EventType.MediaPlayerEndReached, self.song_ended_callback)
            self.new_song()
        else:
            self.queue_ended = True
            self.current = None

    def pause(self):
        self.player.pause()

    def skip(self):
        if (self.player.is_playing()):
            self.player.stop()
            self.song_ended_callback(None)

    def play_button_icon(self):
        if self.player.is_playing():
            return "pause"
        else:
            return "play_arrow"

    def volume_down(self):
        volume = max(0, self.player.audio_get_volume() - 10)
        self.player.audio_set_volume(volume)

    def volume_up(self):
        volume = min(100, self.player.audio_get_volume() + 10)
        self.player.audio_set_volume(volume)

queue = JukeboxQueue()
jukebox = VlcJukebox(queue)

@app.route("/<path:folder>")
@app.route("/")
def view_tracks(folder=""):
    real_path = os.path.join(root_dir, folder)
    if not os.path.isdir(real_path):
        abort(404)
    root = None
    if (len(folder) > 0):
        root = os.path.abspath(os.path.join("/", folder, ".."))
    for (dirpath, dirnames, filenames) in os.walk(real_path):
        music_files = [filename for filename in filenames if is_mp3_file(filename)]
        return render_template("index.html", 
                folder=folder, 
                dirnames=dirnames, 
                filenames=music_files, 
                queue=[get_filename(path) for path in queue.get_contents()],
                root=root,
                current=get_filename(jukebox.current),
                icon=jukebox.play_button_icon())

def is_mp3_file(filename):
    return os.path.splitext(filename)[-1].casefold() == ".mp3"

def get_filename(path):
    if path is None:
        return None
    return os.path.splitext(os.path.basename(path))[0]

@app.route("/play/<path:filename>")
def play_file(filename):
    queue.request(os.path.join(root_dir, filename))
    jukebox.queue_updated()
    return redirect(request.referrer)

@app.route("/pause")
def pause():
    jukebox.pause()
    return redirect(request.referrer)

@app.route("/skip")
def skip():
    jukebox.skip()
    return redirect(request.referrer)

@app.route("/queue")
def update_queue():
    return render_template("queue.html",
                queue=[get_filename(path) for path in queue.get_contents()])

@app.route("/now")
def now_playing():
    return render_template("now.html",
            current=get_filename(jukebox.current))

@app.route("/volume/<direction>")
def volume_ctrl(direction):
    if direction == "up":
        jukebox.volume_up()
    elif direction == "down":
        jukebox.volume_down()
    else:
        abort(404)
    return redirect(request.referrer)

@app.route("/upload/<path:folder>", methods=["POST"])
def upload(folder):
    real_path = os.path.join(root_dir, folder)
    if not os.path.isdir(real_path):
        abort(404)
    if "track" not in request.files:
        return redirect(request.referrer)
    track = request.files["track"]
    if is_mp3_file(track.filename):
        filename = secure_filename(track.filename)
        track.save(os.path.join(real_path, filename))
        return redirect(request.referrer)

def pause_button_callback(channel):
    if volume_button_on:
        jukebox.volume_up()
    else:
        jukebox.pause()

def skip_button_callback(channel):
    if volume_button_on:
        jukebox.volume_down()
    else:
        jukebox.skip()

def volume_toggle_callback(channel):
    volume_button_on = not volume_button_on

if __name__ == "__main__":
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(10, GPIO.IN)
    GPIO.add_event_detect(10, GPIO.FALLING, callback=pause_button_callback, bouncetime=200)
    GPIO.setup(22, GPIO.IN)
    GPIO.add_event_detect(22, GPIO.FALLING, callback=skip_button_callback, bouncetime=200)
    GPIO.setup(27, GPIO.IN)
    GPIO.add_event_detect(27, GPIO.BOTH, callback=volume_toggle_callback, bouncetime=200)

    atexit.register(GPIO.cleanup)
    app.run(host='0.0.0.0', port=8000)

