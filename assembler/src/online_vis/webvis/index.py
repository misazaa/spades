#Flask imports
import flask
from flask import Flask, session, request
from flask.ext.session import Session
#Aux imports
import re
from urllib import quote, unquote

from shellder import *

# General app settings
app = Flask(__name__)
SESSION_TYPE = "filesystem"
app.config.from_object(__name__)
Session(app)

def make_url(string):
    files = re.findall(r"(?:[\w\.]+\/)+(?:\w+\.\w+)", string)
    res = string
    for f in files:
        method = "render" if f.endswith(".dot") else "get"
        url = "<a href=\"%s?file=%s\">%s</a>" % (method, quote(f), f)
        res = res.replace(f, url)
    return res

def format_output(lines):
    return "<br/>".join(map(make_url, lines))

env_path = "../../../"
shellder = None

@app.route("/", methods=['GET'])
def index():
    global shellder
    if shellder is None:
        shellder = Shellder("/tmp/vis_in", "/tmp/vis_out", env_path)
        session["log"] = shellder.get_output()
    return flask.render_template("index.html", console=format_output(session["log"]))

@app.route("/logout", methods=['GET'])
def logout():
    global shellder
    if shellder is not None:
        shellder.close()
        shellder = None
        return "You have been logged out"
    return "No opened session"

@app.route("/command", methods=['POST'])
def command():
    global shellder
    result = shellder.send(request.form["command"]).get_output()
    session["log"].extend(result)
    return format_output(result)

@app.route("/get")
def get():
    file_path = env_path + unquote(request.args.get("file", ""))
    print("Getting", file_path)
    return flask.send_file(file_path, as_attachment=True, attachment_filename=path.basename(file_path))

@app.route("/render")
def render():
    file_path = unquote(request.args.get("file", ""))
    dirfile, _ = path.splitext(file_path)
    res_path = dirfile + ".png"
    result = open(res_path, "w")
    subprocess.call(["dot", "-Tpng", file_path], stdout=result)
    result.close()
    return flask.redirect("/get?file=" + res_path)

if __name__ == "__main__":
    app.debug = True
    app.secret_key = "somekey"
    app.run()
