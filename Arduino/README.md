# Installation

## Cloning the project

You know with age we take some habits and it would be easier if you follow the same one ...

All our github projects are always in a folder called `git`
that is at the first level of the home directory. It is followed by the 
name of the github user/organisation and finaly the project.

So to install the project from bash:
```
mkdir -p ~/git/bioreactor/
cd ~/git/bioreactor/
git clone git@github.com:Bioreactor/Bioreactor_v4.git
```

The possiblity to clone the project will require that you have validate your public key
in github !

## Installing the Arduino IDE

Currently we are still using the Arduino IDE. Plase download the last 1.6 version
of the program from: https://www.arduino.cc/en/Main/Software

## Libraries

In this project we use many non standard libraries. By default the Arduino IDE
will look for them in `~/Arduino/libraries`

If you are using only our project it is easier to directly point this folder
to the libraries that are present in the github.

```
mkdir ~/Arduino/
ln -s ~/git/bioreactor/Bioreactor_v4/Arduino/libraries/ ~/Arduino/
```

## That's it

You should now be able to compile the project from the Arduino IDE.
