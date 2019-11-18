#!/usr/bin/python3

from pathlib import Path
import shutil
import shlex
import subprocess

def main():
    compose_dest = Path("/etc/compose")
    compose_dest.mkdir(exist_ok=True)

    while True:

        print("which compose file to install?\n")
        updir = Path("../")
        for ymlfile in updir.iterdir():
            if str(ymlfile).endswith("yml"):
                print(ymlfile.name)

        print()
        resp = input()
        ymlfile = updir/resp
        if ymlfile.exists():
            break

        print("Bad yml file! Please choose from above list")


    print(
        "Copying {} to {}".format(
            str(ymlfile),
            str(compose_dest/Path("docker-compose.yml"))))
    shutil.copy(str(ymlfile), str(compose_dest/Path("docker-compose.yml")))

    with open("webclient-compose.service", 'r') as f:
        sfile = f.read()

    print(sfile)
    sfile = sfile.format(www_dir=str((updir).absolute().parents[1]))
    with open("/tmp/webclient-compose.service", 'w') as tmp:
        tmp.write(sfile)

    shutil.copy("/tmp/webclient-compose.service",
                "/etc/systemd/system/webclient-compose.service")

    cmd = shlex.split("systemctl daemon-reload")

    process = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    print(stdout)
    print(stderr)

    cmd = shlex.split("systemctl enable webclient-compose.service")

    process = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)

    stdout, stderr = process.communicate()
    print(stdout)
    print(stderr)


main()
