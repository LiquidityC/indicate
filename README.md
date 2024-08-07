# indicate

`indicate` is a simple graphical app with one job. It will open whatever
`image/png` content you have in clipboard. Allow you to draw red squares on
said image and put whatever you create back into the clipboard. The main purpose
is to be able to quickly take a screenshot. Highlight what's important and
paste it to whatever means of communication you're using at the moment.

## Installation

It's all `cmake`. Just setup the build and run `make install`.

Eg.

- `git clone --recurse-submodules https://github.com/LiquidityC/indicate`
- `cd indicate`
- `mkdir build`
- `cd build`
- `cmake ..`
- `make install`

## SDL3 as a submodule

Yes. SDL3 comes with clipboard support for any data type. SDL2 doesn't. SDL3
has yet to be released so we rely on the master branch of SDL3. That's just how
it has to be for now.

## SDL3_image as a submodule

For the same reason as above and to be able to handle more mime-types then `bmp`.

## Mime type?

`indicate` only supports the clipboard mime-type `image/png`. If your
screenshot tool of choice doesn't support this then you'll need to send me a
PR. I will not do it for you because I won't have time and I personally don't
need it. That does not mean that I'm not interested in seeing this software
improve. My time is limited is all. Please send PRs.

## Contribute?

Please do

## Why C and not C++?

Because C is a great language and it has everything this project needs. You
should learn C. It's great. See [Contribute?](#Contribute?)
