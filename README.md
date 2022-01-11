# crqt-ng
crqt-ng - cross-platform open source e-book reader using crengine-ng. It is a fork of the [CoolReader](https://github.com/buggins/coolreader) project.

## Description
crqt-ng is an e-book reader.
In fact, it is a [Qt](https://www.qt.io/) frontend for the [crengine-ng](https://gitlab.com/coolreader-ng/crengine-ng) library.

Supported platforms: Windows, Linux, MacOS. Basically all platforms that are supported by crengine-ng, Qt and cmake.

Supported e-book formats: fb2, fb3 (incomplete), epub (non-DRM), doc, docx, odt, rtf, pdb, mobi (non-DRM), txt, html, chm, tcr.

Main functions:
* Ability to display 2 pages at the same time
* Displaying and Navigating Book Contents
* Text search
* Text selection
* Copy the selected text to the clipboard
* Word hyphenation using hyphenation dictionaries
* Most complete support for FB2 - styles, tables, footnotes at the bottom of the page
* Extensive font rendering capabilities: use of ligatures, kerning, hinting option selection, floating punctuation, simultaneous use of several fonts, including fallback fonts
* Detection of the main font incompatible with the language of the book
* Reading books directly from a ZIP archive
* TXT auto reformat, automatic encoding recognition
* Flexible styling with CSS files
* Background pictures, textures, or solid background

## Visuals
TODO: add screenshots...

## Installation
1. To install the program, make sure that all dependencies are installed: Qt framework and crengine-ng library.

   It is best to use your Linux distribution's package manager to install Qt. Otherwise, download the installation package from https://www.qt.io/. When installing using your package manager, remember to install the "-dev" variant of the package, for example for Ubuntu use the following command:
```
$ sudo apt install qtbase5-dev qttools5-dev
```

On windows msys2 and 'pacman' package manager can be used:
```
$ pacman -S mingw-w64-x86_64-qt5
```

To install crengine-ng follow the instructions https://gitlab.com/coolreader-ng/crengine-ng#installation

2. Compile the program using cmake, for example:
```
$ mkdir ../crqt-ng-build
$ cd ../crqt-ng-build
$ cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release ../crqt-ng
$ make -j10 VERBOSE=1
$ make install

```

## Contributing
TODO: write this guide...

## Authors and acknowledgment
The list of contributors can be found in the AUTHORS file.

This list of authors is obtained from git history using the script tools/getauthors/getauthos.sh.
The first significant item in the git log is 'Mon Nov 9 16:47:42 2009 +0300', but the project started around 2000, so this list can be incomplete.

## License
This project is released under the [GNU General Public License Version 2](https://opensource.org/licenses/GPL-2.0). See LICENSE file.
