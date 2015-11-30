# NAME

dinner - A lightweight web server

# SYNOPSIS

This is a lightweight web server based on epoll and a simple [EDA][ID_EDA]
(event-driven architecture).

[ID_EDA]: https://en.wikipedia.org/wiki/Event-driven_architecturei "Learn more about EDA"

# BUILD

```shell
git clone https://github.com/Arondight/dinner.git dinner
cd dinner
./build.sh
```

For a test, you can run `dinner` after build like:

```shell
./bin/dinner
```

Then visit `http://127.0.0.1:8080` in your web browser.

Read [systemd/README.txt][ID_SYSTEMD_README_TXT] for configure.

[ID_SYSTEMD_README_TXT]: systemd/README.txt "Read systemd/README.txt"

# DEPENDENCIES

1. [Linux](https://www.kernel.org) >= 2.6
+ [Glibc](https://www.gnu.org/software/libc) >= 2.3.2
+ [GCC](https://gcc.gnu.org) >= 3.3
+ [CMake](https://cmake.org) >= 2.6
+ [Perl][ID_Perl] and [CGI][ID_CGI] (both not necessary but recommended)

[ID_Perl]: https://www.perl.org
[ID_CGI]: https://metacpan.org/pod/distribution/CGI/lib/CGI.pod

# USAGE

Run `dinner -h`

# COPYRIGHT

Copyright (c) 2015 秦凡东 (Qin Fandong)

# LICENSE

Read [LICENSE][ID_LICENSE]

[ID_LICENSE]: LICENSE "Read LICENSE"
