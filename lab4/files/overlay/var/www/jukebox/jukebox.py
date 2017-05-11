from flask import abort, Flask, redirect, render_template, request, url_for

import vlc
import os

from tornado.wsgi import WSGIContainer
from tornado.httpserver import HTTPServer
from tornado.ioloop import IOLoop

import RPi.GPIO as GPIO

app = Flask(__name__)

root_dir = "/music"

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

if __name__ == "__main__":
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(10, GPIO.IN)
    GPIO.add_event_detect(10, GPIO.FALLING, callback=jukebox.pause, bouncetime=200)
    GPIO.setup(22, GPIO.IN)
    GPIO.add_event_detect(22, GPIO.FALLING, callback=jukebox.skip, bouncetime=200)

    server = HTTPServer(WSGIContainer(app))
    server.listen(80)
    IOLoop.instance().start()

