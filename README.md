# ESP-32s3-Mp3-Player

Goal -- I love music and wanted to build something I would actually use. This project exists to learn, have fun, and grow as an engineer by working through real hardware and software challenges from start to finish.



Description -- 
Building a portable MP3 player from scratch using an ESP32-S3 microcontroller. This project combines embedded systems, digital audio, and hardware integration — taking individual components and turning them into something that actually works.
The idea was simple: design a device that can read audio files from an SD card, decode them, and output clean sound through a headphone jack, all while displaying track information on a small TFT screen. Simple in concept, but there's a lot going on under the hood.
The audio chain runs from the ESP32-S3 over I2S to a PCM5102 DAC, then through a MAX97220 headphone amplifier — each stage chosen deliberately to keep the signal clean and the output quality high. The display and SD card both communicate over SPI, so managing multiple peripherals on shared buses is part of the challenge too.
The project follows a deliberate four phase development process. The first phase focuses entirely on component validation — each piece of hardware gets its own dedicated test sketch before anything is integrated together. The SD card, TFT display, DAC, and amplifier are all verified individually so that when something doesn't work, the cause is already narrowed down. The second phase brings those components together, confirming that the full hardware stack communicates correctly as a system. The third phase is where the actual player UI gets built, with confidence that everything underneath it is solid. By the time the interface is being designed, the hard problems are already solved.
Once the proof of concept is working, the project moves into its next stage. The breakout boards get replaced with a custom PCB designed around board level components, producing a cleaner and more compact result. Alongside that, a custom enclosure will be designed and 3D printed to house everything, and a LiPo battery with charging circuit will be added to make the device fully portable and self contained — a finished, usable product rather than just a development board on a breadboard.
This is as much a learning project as it is a technical one. Every decision, mistake, and milestone is documented here so the build process is transparent and reproducible from start to finish.
