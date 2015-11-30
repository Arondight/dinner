This directory contains init scripts for systemd.

CMake will ignore this directory. If systemd is avalible to your system,
you need to configure it manually.

First copy service file:
  sudo cp ./dinner.service /lib/systemd/system/

After edit /lib/systemd/system/dinner.service, run:
  sudo systemctl daemon-reload

Run command below to set autostart:
  sudo systemctl enable dinner

Run command below to start it now:
  sudo systemctl start dinner

