from flask import abort, Flask, redirect, render_template, request, url_for
from werkzeug.utils import secure_filename

import atexit
import os
import mpd

import threading
import time
import gpio

app = Flask(__name__)
root_dir = "/media/dachb/My ZEN"
volume_button_on = False
running = True

class MpcJukebox:
    song_ids = {}

    def __init__(self):
        self.client = mpd.MPDClient()
        self.client.connect("127.0.0.1", 6600)
        self.client.update()

    def play(self, filename):
        if self.client.status()["state"] != "play":
            self.song_ids = {}
            self.client.clear()
        if filename in self.song_ids:
            self.client.moveid(self.song_ids[filename], -1)
        else:
            song_id = self.client.addid(filename)
            self.song_ids[filename] = song_id
        self.client.play()

    def pause(self):
        self.client.pause()

    def skip(self):
        self.client.next()

    def volume_up(self):
        volume = min(int(self.client.status()["volume"]) + 10, 100)
        self.client.setvol(volume)

    def volume_down(self):
        volume = max(int(self.client.status()["volume"]) - 10, 0)
        self.client.setvol(volume)

    def close(self):
        self.client.close()

    def get_queue(self):
        status = self.client.status()
        if "song" in status:
            current_song = str(int(self.client.status()["song"]) + 1)
            return self.client.playlistinfo(current_song + ":")
        else:
            return []

    def play_icon(self):
        state = self.client.status()["state"]
        if state == "play":
            return "pause"
        else:
            return "play_arrow"

    def current_song(self):
        status = self.client.status()
        if "song" in status:
            current_song = self.client.status()["song"]
            return self.client.playlistinfo(current_song)[0]
        else:
            return None

jukebox = MpcJukebox()

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
                queue=jukebox.get_queue(),
                root=root,
                current=jukebox.current_song(),
                icon=jukebox.play_icon())

def is_mp3_file(filename):
    return os.path.splitext(filename)[-1].casefold() == ".mp3"

def get_filename(path):
    if path is None:
        return None
    return os.path.splitext(os.path.basename(path))[0]

@app.route("/play//<path:filename>/")
@app.route("/play/<path:directory>/<path:filename>/")
def play_file(filename, directory=""):
    fullname = os.path.join(directory, filename)
    jukebox.play(fullname)
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
                queue=jukebox.get_queue())

@app.route("/now")
def now_playing():
    return render_template("now.html",
            current=jukebox.current_song())

@app.route("/volume/<direction>")
def volume_ctrl(direction):
    if direction == "up":
        jukebox.volume_up()
    elif direction == "down":
        jukebox.volume_down()
    else:
        abort(404)
    return redirect(request.referrer)

@app.route("/upload/", methods=["POST"])
@app.route("/upload/<path:folder>", methods=["POST"])
def upload(folder = ""):
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

@app.route("/create_dir/", methods=["POST"])
@app.route("/create_dir/<path:folder>", methods=["POST"])
def create_dir(folder = ""):
    if "dir_name" not in request.form:
        return redirect(request.referrer)
    name = request.form.get("dir_name")
    fullpath = os.path.join(root_dir, folder, name)
    if not os.path.isdir(fullpath):
        os.mkdir(fullpath)
    return redirect(request.referrer)

def pause_button_callback():
    if volume_button_on:
        jukebox.volume_up()
    else:
        jukebox.pause()

def skip_button_callback():
    if volume_button_on:
        jukebox.volume_down()
    else:
        jukebox.skip()

def volume_toggle_callback():
    global volume_button_on
    volume_button_on = not volume_button_on

class GPIOPin:
    previous_state = True

    def __init__(self, pin, edge, callback):
        self.pin = pin
        self.edge = edge
        self.callback = callback

    def read(self):
        state = gpio.read(self.pin)
        falling = False
        rising = False
        if self.previous_state and not state:
            falling = True
        if not self.previous_state and state:
            rising = True
        if self.edge == "RISING":
            if rising:
                self.callback()
        elif self.edge == "FALLING":
            if falling:
                self.callback()
        else:
            if rising or falling:
                self.callback()
        self.previous_state = state

def cleanup():
    for pin in [10, 22, 27]:
        gpio.cleanup(pin)

def read_pins(pins, polltime):
    global running
    while running:
        for pin in pins:
            pin.read()
        time.sleep(polltime)

if __name__ == "__main__":
    pin_no = [10, 22, 27]                                
    for pin in pin_no:
        gpio.setup(pin, gpio.IN)

    def cleanup():                                         
        for pin in pin_no:                         
            gpio.cleanup(pin)

    pins = []
    pins.append(GPIOPin(10, "FALLING", pause_button_callback))
    pins.append(GPIOPin(22, "FALLING", skip_button_callback))
    pins.append(GPIOPin(27, "BOTH", volume_toggle_callback))

    thread = threading.Thread(target = lambda: read_pins(pins, 0.1))
    thread.daemon = True
    thread.start()

    atexit.register(gpio.cleanup)
    atexit.register(jukebox.close)
    app.run(host='0.0.0.0', port=8000)
