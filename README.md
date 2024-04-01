# AMD AMA Plugin

## Introduction

The plugin provides an AVC, HEVC and AV1 encoder done by the hardware accelerator card AMD MA35D. It also includes:

* A CMake project file
* GitHub Actions workflows and repository actions

## Set Up

### Pre Requisites

In order to use the plugin it is necessary to have the following requirements on your PC:

* Linux Ubuntu 22.04 OS with Kernel version 5.15.0-92 generic
* At least one AMD MA35D hardware acceleration card
* AMD AMA 1.1.1 SDK installed, for intructions on installing the SDK follow the instructions from the following [link](https://amd.github.io/ama-sdk/v1.1.1/getting_started_on_prem.html)
* OBS Studio version 30.0 or greater installed
* If you want to build the plugin you will need additional requirements which are listed and explained in the following [link](https://github.com/obsproject/obs-studio/wiki/Build-Instructions-For-Linux) 

## Installation

The plugin can be installed using dpkg command once you have the plugin package. To obtain the package, download the package zip from the repo and unzip it or build the package following the instructions of section [Build Plugin](#build-plugin). To install the package run the following command:

`dpkg -i obs-amd-ama-plugin.deb`

After running command, AMD AMA Plugin features should be available when launching OBS.

## Build Plugin {#build-plugin}

To build the plugin run the following commands:

* `cd /.github/scripts`
* `./build-linux`

After running these two commands you will have a build folder named `build_x86_64`, particularly this folder will contain the shared object associated with the plugin `obs-amd-ama.so`. If you want to create a `.deb` package of the plugin you can run the following command in `.github` directory:

`cmake --build build_x86_64 --target package`

## Known Limitations

This plugin has the following limitations when streaming or recording:

* It is not possible for the user to select an encoding level, the encoding level selection is done by the plugin taking into account the characteristics of the stream or recording to be created.

* It is not possible for the user to select the amount of B frames for the encoding, the amount of B frames is selected by the plugin taking into account the characteristics of the stream or recording to be created.

* The only possible input pixel format for this plugin is I42O. If another pixel format is selected by the user, the plugin will convert it to I420 format. 

* It is not possible to change encoder settings while doing streaming. If the user needs to change encoding settings, it must stop the streaming, change the encoding settings and start again the stream.


