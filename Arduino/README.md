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



## H2
### H3
#### H4
##### H5
###### H6

Alternatively, for H1 and H2, an underline-ish style:

Alt-H1
======

Alt-H2
------

* item1
  - subitem1
  - subitem2
* item2
  1. subitem1
  2. subitem2
* item3


* http://www.cheminfo.org
* [I'm an inline-style link](https://www.google.com)


Inline `code` has `back-ticks around` it.


~~abc~~

_italic_ *italic*
__bold__ **bold**

---

First Header  | Second Header
------------- | -------------
Content Cell  | Content Cell
Content Cell  | Content Cell

| Left-Aligned  | Center Aligned  | Right Aligned |
| :------------ |:---------------:| -----:|
| col 3 is      | some wordy text | $1600 |
| col 2 is      | centered        |   $12 |
| zebra stripes | are neat        |    $1 |

___

```
var a=0;
for (var i=0; i<100; i++) {
  a+=i;
}
```
***

> Blockquotes are very handy in email to emulate reply text.
> This line is part of the same quote.
>> Blockquotes are very handy in email to emulate reply text.
>> This line is part of the same quote.





