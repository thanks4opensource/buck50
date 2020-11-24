buck50: Test and measurement firmware for "Blue Pill" STM32F103
===============================================================

**buck50** is open-source firmware that turns a "Blue Pill" STM32F103 development board (widely available for approx. US$1.50) into a multi-purpose test and measurement instrument, including:
* 8 channel, 6+ MHz logic analyzer
    * Approx. 5K sample buffer depth
    * Samples stored only at signal edges for efficient memory usage
    * Units may be ganged for increased number of channels
    * Complex triggering via user-defined state machine supporting combinations of sequential ("A then B then C") and logical-OR ("A or B or C") conditionals
    * Output to VCD and other file formats for export to waveform viewing software [<sup>1</sup>](#footnote_1)
* Live monitoring and logging of digital, analog, USART (sync/async), SPI (MOSI/MISO), and I2C (master/slave/TX/RX) data
* Simple dual-channel approx. 1 MHz digital storage oscilloscope, approx. 5K sample buffer depth (10K if single channel)
* 3 channel digital pulse train generator with user-defined frequency and per-channel duty cycle and polarity
* Bidirectional bridge/converter from USART/UART (async/synchro), SPI (master/slave), or I2C ... to USB ... to host terminal, UNIX socket, or UNIX pty device file
* 8-bit parallel output counter (binary or gray code)
* Host terminal ascii or binary input data to 8-bit parallel output
* Firmware written in a combination of C++ and ARM Thumb-2 assembly code, with several non-standard [hacks\^H\^H\^H\^H\^Htechniques](#hacks_techniques) of possible general interest to advanced software developers
* Python host driver program with comprehensive inline help system and usability features
* Coffee maker attachment sold separately, not included in base price [<sup>2</sup>](#footnote_2)


<br> <a name="contents"></a>
Contents
--------

* [License](#license)
* [Firmware installation](#firmware_installation)
* [Hardware connections](#hardware_connections)
* [buck50.py driver program](#buck50_py_driver_program)
    * [Python](#python)
    * [Where's the GUI?](#Wheres_the_gui)
    * [TL;DNR("Too Long, Did Not Read")](#TL_DNR)
    * [NLE;WTRSBNTMM(“Not Long Enough, Want To Read Some But Not Too Much More”)](#NLE_WTRSBNTMM)
    * [OK;TSIGMTW9Y("OK, This Sounds Interesting, Give Me The Whole 9 Yards")](#OK_TSIGMTW9Y)
        * [buck50.py startup and device connection](#buck50_py_startup_and_device_connection)
        * [review license](#review_license)
        * [help](#help)
        * [usage help](#usage_help)
        * [`configure` command](#configure_command)
        * [`logic` command](#logic_command)
        * [`oscope` command](#oscope_command)
        * [`dump` command](#dump_command)
        * [`trigger` command](#trigger_command)
        * [`monitor` command](#monitor_command)
        * [`lines` configuration](#lines_config)
        * [`adcN` configurations](#adcN_configs)
        * [`numbers` command](#numbers_command)([*](#non_intuitive_names))
        * [`gpio` command](#gpio_command)
        * [`usart` command](#usart_command)
        * [`spi` command](#spi_command)
        * [`i2c` command](#i2c_command)
        * [`ipc` configuration](#ipc_configuration)
        * [`pty` configuration](#pty_configuration)
        * [`socket` configuration](#socket_configuration)
        * [`pulse` command](#pulse_command)([*](#non_intuitive_names))
        * [`reset` command](#reset_command)
* [Waveform viewing software](#waveform_viewing_software)
    * [gnuplot](#gnuplot)
    * [gtkwave](#gtkwave)
    * [PulseView](#pulseview)
    * [Logic](#Logic)
* [Firmware design and implementation](#firmware_design_and_implementation)
    * [Port assignments](#port_assignments)
    * [Hacks\^H\^H\^H\^H\^Htechniques](#hacks_techniques)
        * [Infinite loop](#infinite_loop)
        * [`longjmp` from interrupt handler](#longjmp_from_interrupt_handler)
        * [Sparse timestamped samples](#sparse_timestamped_samples)
    * [Why so slow?](#why_so_slow)
* [Building from source](#building_from_source)
* [Current version](#current_version)
* [Future work](#future_work)
* [Footnotes](#footnotes)




<br> <a name="license"></a>
License
-------

buck50: Test and measurement firmware for "Blue Pill" STM32F103 development board

Copyright (C) 2019,2020 Mark R. Rubin aka "thanks4opensource"

This file is part of buck50.

The buck50 program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

The buck50 program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the [GNU General Public License](LICENSE.txt) along with the buck50 program.  If not, see <https://www.gnu.org/licenses/gpl.html>




<br> <a name="firmware_installation"></a>
Firmware installation
--------------------

buck50 is provided as ready-to-flash files in ELF ([buck50.elf](build/buck50.elf)), binary ([buck50.bin](build/buck50.bin)), and Intel hex ([buck50.hex](build/buck50.hex)) formats in addition to source code (see [Building from source](#building_from_source), below).

Instructions on how to flash code onto microcontrollers such as the STM32F103 are beyond the scope of this document. Briefly, the STM32F103 cannot be flashed via its USB port (except [<sup>3</sup>](#footnote_3)), so other techniques must be employed.

Probably the best method is via the "Blue Pill" SWD port (see [Hardware connections](#hardware_connections), below) using either a Black Magic Probe ([website](https://1bitsquared.com/products/black-magic-probe), [wiki](https://github.com/blacksphere/blackmagic/wiki)) or an ST-Link ([website](https://www.st.com/en/development-tools/st-link-v2.html)) or clone thereof ([Google search](https://www.google.com/search?q=st-link+clone)). Note that the former is used with GDB from the [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm) and the latter either with GDB or `stlink` ([GitHub repository](https://github.com/stlink-org/stlink)) among other software front ends.

Alternately, the "Blue Pill" can be flashed via its USART1 serial port (RX on port PA10, TX on PA9, see [below](#hardware_connections)). This typically requires a USB-to-serial converter 
([Google search](https://www.google.com/search?&q=usb-to-serial+adapter&oq=usb-to-serial+adapter)) (or, ironically, another already-flashed "buck50" [<sup>4</sup>](#footnote_4)) as modern host computers rarely include an RS-232 port. Instructions and software required for doing so can be found at the `stm32flash` SourceForge [website](https://sourceforge.net/projects/stm32flash/), this tutorial [web page](https://maker.pro/arduino/tutorial/how-to-program-the-stm32-blue-pill-with-arduino-ide), and/or a [GitHub wiki](https://github.com/palhartinger/HowTo-STM32F103C8T6/wiki/6_Programming-with-UART), among other resources. If using a serial port method remember to change the BOOT0 jumper (see directly [below](#hardware_connections)) back to its "0" position after flashing with it set to "1".




<br> <a name="hardware_connections"></a>
Hardware connections
--------------------

The following is a highly simplified, edited version of the incredibly useful image created by Rasmus Friis Kjeldsen [<sup>5</sup>](#footnote_5), available at <http://reblag.dk/stm32/> (direct link: <http://reblag.dk/wordpress/wp-content/uploads/2016/07/The-Generic-STM32F103-Pinout-Diagram.pdf>). The edited and  [original](images/The-Generic-STM32F103-Pinout-Diagram.pdf) versions are included in this repository under terms in compliance with the original's [license](https://creativecommons.org/licenses/by-sa/1.0/).

![](images/buck50_full.png "hardware connections")

**WARNING!** Incorrect hookups to the "Blue Pill" (or any other microcontroller's) ports may cause permanent damage. Examples: Connecting a push-pull output port directly to a positive voltage source or ground sink, or to another push-pull output on the same or different MCU set to the opposite polarity. Readers of this document and users of the firmware it describes assume all responsibility for their actions.

#### Pin descriptions

The buck50 firmware configures and uses the "Blue Pill" STM32F103 GPIO ports as follows. Note that many pins have multiple uses depending on which `buck50.py` host command is in effect, including changing roles to be either inputs, outputs, or bidirectional.

        Pin(s)    Type                              Purpose                             buck50.py commands
        ======    ==============================    =================================   ==================
        PB4-11    input   weak pull-down            digital input                       logic, monitor
        PB10,11   in/out  open-drain                I2C                                 i2c, monitor
        PA0-7     output  push-pull                 digital output                      numbers, gpio
        PA0-7     input   analog                    analog input                        oscope, monitor
        PA1-3     output  push-pull                 digital pulse outputs               pulse
        PA0       input   weak pull-up              usart CTS                           usart
        PA1       output  push-pull                 usart RTS                           usart
        PA2       output  push-pull                 usart TX                            usart
        PA3       input   weak pull-up              usart RX                            usart
        PA4       output  push-pull                 SPI select output (master)          spi, monitor
        PA4       input   floating                  SPI select input  (slave)           spi, monitor
        PA5       output  push-pull                 SPI clock (master)                  spi, monitor
        PA5       input   floating                  SPI clock (slave)                   spi, monitor
        PA6       input   floating/pull-up/-down    SPI MISO                            spi, monitor
        PA7       output  push-pull                 SPI MOSI                            spi, monitor
        PA8       output  push-pull                 USART clock (synchronous)           usart, monitor
        PA9       output  push-pull                 USART TX                            usart, monitor
        PA10      input   weak pullup               USART RX                            usart, monitor
        PB13      output  push-pull                 sampling indicator, sync            logic, oscope
        PB14      output  open-drain                external/ganged trigger             logic, oscope
        PB15      output  open-drain                external/ganged trigger ack         logic, oscope
        PC13      output  open-drain                status/fault/sampling/triggering    <all>, logic, oscope
        PA11      output  open-drain                USB negative (do not connect)       <all>
        PA12      output  open-drain                USB positive (do not connect)       <all>

See [logic](#logic_command), [monitor](#monitor_command), [numbers](#numbers_command), [spi](#spi_command), and [usart](#usart_command), below.

##### Notes:
* See [buck50.py driver program](#buck50_py_driver_program), below, for command usage.
* PA0-7 are multipurpose pins, as per above.
* PC13 connected to on-board user LED
    * On: Firmware running and connected to USB
    * Off: Triggering and/or sampling
    * Blinking 2 Hz: `reset blink` command (used to identify physical device)
    * Blinking 4 Hz: Firmware error (requires hardware reboot and host software reconnect [<sup>6</sup>](#footnote_6)).
* PB13 outputs approx. 2.15 Hz 50% duty cycle square wave during digital sampling. Can be connected to an external LED (with appropriate current-limiting resistor) or other external monitoring as sampling indicator, and/or connected to ganged buck50s as additional sync signal for post-processing alignment of captured data.
* PB14 and PB15 can be used to synchronize triggering across multiple buck50 "Blue Pill"s. Connect all PB14 pins to an open-drain bus line (with pull-up resistor), and similarly for PB15. See `buck50.py`'s `help reset ganged=`, `help reset ext-trig`, and `help trigger`, and [logic command](#logic_command) and [reset command](#reset_command), below.




<br> <a name="buck50_py_driver_program"></a>
buck50.py driver program 
------------------------

<a name="python"></a>
### Python

[buck50.py](build/buck50.py) is (obviously) written in Python. Python3 ([<sup>7</sup>](#footnote_7)) (3.6 to be precise). It was developed under Linux and should be portable any Linux distribution or other UNIX-like operating system. External library and file system assumptions (`/dev/ttyACMn`, `/dev/pts/NN`) may require modification for use under macOS. It is unlikely that `buck50.py` will function in a Microsoft Windows&#174; environment without fairly extensive code modifications.


<br><a name="Wheres_the_gui"></a>
### Where's the GUI? 
(Note to the impatient: If reading sequentially, skip ahead to [TL;DNR ("Too Long, Did Not Read")](#TL_DNR), below.)

Good question. The original intent for buck50 was to integrate the host-based driver into an existing open source waveform viewer program. This was abandoned due to limitations in the ones I could find (see [Waveform viewing software](#waveform_viewing_software), below), and also as the scope of buck50 grew beyond it's original beginnings as a logical analyzer and evolved  into the many-headed Hydra it eventually became. (Its capabilities no longer fit into the basic design of a logic analyzer program.)

In addition, many of those capabilities are inherently text based (monitoring, SPI and USART bridges, etc). Sure, any decent GUI library provides a scrolling text widget, but a text-based program inherently leverages the features of the many excellent Linux terminal emulator programs available. As an analogy, consider the difference between the typical on-screen GUI calculator program that mimics a handheld device with only one numeric display line compared to a full featured text-based program like `bc` (or, for that matter, Python).

The `buck50.py` interface was expressly written for fast and easy usage in an active test and measurement environment. Think of it as an ugly-looking GUI with many, many "hot key" shortcuts. (Just like the GUI calculators: What's faster? Typing numbers on a keyboard or mousing around clicking virtual GUI buttons? Also [<sup>8</sup>](#footnote_8).) Try `buck50.py`, and if dissatisfied but still want to use the firmware, feel free to write a GUI driver (respect the [GPL license](#license) above). A wrapper layer around the `buck50.py` module would be fairly easy to implement.

<a name="viewer_integration"></a>
All the above notwithstanding, `buck50.py` does attempt to integrate nicely with external waveform viewing software (see [below](#waveform_viewing_software)). If `gnuplot` is installed on the host system, the code will by default to automatically dump captured digital or analog data to a temporary file and load that (see [gnuplot](#gnuplot), below) with the appropriate commands [<sup>9</sup>](#footnote_9). Other viewers may be manually configured/enabled as well.



<br> <a name="TL_DNR"></a>
### TL;DNR ("Too Long, Did Not Read") 

The buck50 firmware and `buck50.py` driver implement many complex capabilities. The interface is thus necessarily complex but has been designed to allow simple default usage when fine-grained control is not needed.

##### Examples:

Output 4Hz incrementing binary counter on output ports PA0 through PB77:

        $1.50: numbers
        <ENTER> to halt ...   

Monitor input ports PB4 through PB11:

        $1.50: monitor 
        <ENTER> to end
          0.000  00111001   ... ... blu grn yel ... ... brn 
          0.250  00111010   ... ... blu grn yel ... red ... 
          0.500  00111011   ... ... blu grn yel ... red brn 
          0.750  00111100   ... ... blu grn yel org ... ... 
          1.000  00111101   ... ... blu grn yel org ... brn 
          1.250  00111110   ... ... blu grn yel org red ... 
          1.500  00111111   ... ... blu grn yel org red brn 
          1.750  01000000   ... vio ... ... ... ... ... ... 
          2.000  01000001   ... vio ... ... ... ... ... brn 
          2.250  01000010   ... vio ... ... ... ... red ... 
          2.500  01000011   ... vio ... ... ... ... red brn 
          2.750  01000100   ... vio ... ... ... org ... ... 
          3.000  01000101   ... vio ... ... ... org ... brn 
          3.250  01000110   ... vio ... ... ... org red ... 

Logic analyzer, capture first 32 signal level changes on ports PB4 through PB11:

        $1.50: logic edges=32
        logic: edges=32
        Waiting for sampling finish (<ENTER> to abort) ...  
        Triggered at state #0: 32 samples (6.26MHz) in 0.00 seconds. Stopped by number of samples. 

The captured sample data will be automatically uploaded and an external viewer program launched if configured (see [above](#viewer_integration)). Otherwise (or in addition), data can be manually uploaded and saved, for example:

        $1.50: dump digital-frmt=vcd file=capture
        logic samples: 0...31 of 32 @ 6.26MHz ... saving to file capture.vcd

Logic analyzer, as above, but trigger on rising edge of input port PB8

        $1.50: trigger 0=xxx0xxxx-1-0  # wait for PB8 low 
        $1.50: trigger 1=xxx1xxxx-0-1  # wait for PB8 high
        $1.50: logic edges=32
        logic: edges=32
        Waiting for sampling finish (<ENTER> to abort) ...  
        Triggered at state #1: 32 samples (6.26MHz) in 5.18 seconds. Stopped by number of samples. 
        logic samples: 0...31 of 32 @ 6.26MHz ... saving to file /tmp/temp.csv

<a name="usart_bridge"></a>
USART bridge:

        $1.50: usart
        <ENTER> to input, then input ascii text with optional "\n", "\t", and "\x<two hex digits>" escape sequences ... 
        received data 
        more received data 
        -- <ENTER> key pressed here --
        "END" to finish: data to send
        yet more received data 
        -- <ENTER> key pressed here --
        "END" to finish: END
        flushing USB CDC-ACM pipe for 1 second (<ENTER> to abort) ...

or alternately:

        $1.50: usart ipc external pty enable 
        ipc: external
        pty: enable
        pty /dev/pts/41 ready
        I/O active, press <ENTER> to exit ...

and then run an external terminal application (`screen`, `miniterm`), or more simply:

        $ cat /dev/pts/41 & cat > /dev/pts/41

I2C bridge (slave @ address 31, initialize register 0x5b with 1, register 0x2a with 0x39, then read 6 byte wide register 0x33 twice):

        $1.50: i2c master 
        i2c: master
        Input format: <slave address> <rx size> <tx data <2hex|3dec|1ascii>) ...> [; ...] 
        "0 0" to end: 0x1f 0 5b 01 ; 0x1f 0 2a 39
        stat= OK tx=2  rx=0  : 
        stat= OK tx=2  rx=0  : 
        "0 0" to end: 0x1f 6 33
        stat= OK tx=1  rx=6  : 255-ff-. 215-d7-. 254-fe-. 044-2c-, 003-03-. 208-d0-.
        "0 0" to end: 0x1f 6 33
        stat= OK tx=1  rx=6  : 255-ff-. 212-d4-. 254-fe-. 043-2b-+ 003-03-. 212-d4-.
        "0 0" to end: 0 0

Continuously read register after above I2C address 31 device initialization:

        $1.50: monitor pb4-11=disabled rate=1s i2c master addr=31 tx-data=33 rx-size=6 
        i2c: master
        monitor: pb4-11=disabled
        monitor: rate=1Hz=1s
        i2c: addr=31
        i2c: tx-data=33
        i2c: rx-size=6
        <ENTER> to end
          0.001   i:ff.d9.fe.2e.03.d5
          1.002   i:ff.d5.fe.2c.03.d4
          2.003   i:ff.d7.fe.2f.03.d5
          3.003   i:ff.d1.fe.28.03.cf
          4.004   i:ff.d2.fe.31.03.d4
          5.005   i:ff.d9.fe.2a.03.dd
          6.006   i:ff.d5.fe.2c.03.d9
          7.007   i:ff.d2.fe.27.03.d0
          8.008   i:ff.cd.fe.2a.03.da
          ...

Output 1kHz square waves on port PA1 through PA3, rising edges at 250, 500, and 750 ms respectively in total period:

        $1.50: pulse enable

Capture analog values from inputs PA4 and PA5, triggering on positive slope at 1.65V on channel PA4

        $1.50: oscope trgr-chnl=4 scnd-chnl=5 trigger=0:3.3:0.15:0.01
        oscope: trgr-chnl=4
        oscope: scnd-chnl=5
        Waiting for sampling finish (<ENTER> to abort) ...  
        3257 samples, 2 channels (PA4,PA5) at 239.5+12.5@12MHz->47.6kHz in 3.83s wall clock time. Stopped by number of samples. 
        Samples 0...3256 (3257 total)  2 channels (PA4,PA5) per sample at 239.5+12.5@12MHz->47.6kHz 



<br> <a name="NLE_WTRSBNTMM"></a>
### NLE;WTRSBNTMM (“Not Long Enough, Want To Read Some But Not Too Much More”)

After reading the above, an reasonable reaction might be: *You want me to type all those complicated commands? How is that "fast and easy" usage?*

A valid criticism ... but there is hope. That's why you, the sophisticated user, chose to read this section. ;) Remember all that stuff that got printed when the program started?

        buck50.py 0.9.2
        Copyright 2020 Mark R. Rubin aka "thanks4opensource"
        This is free software with ABSOLUTELY NO WARRANTY.
        For details type "warranty" or "help warranty".
        Type "using" for program usage.
        Type "help" for commands, configurations, parameters, values.
        ...

Well, after reading the full license (you did type "warranty" or "help warranty", didn't you?), those with inquisitive minds might try:

        $1.50: using
        Input:
          - Type "<command> ..." followed by ENTER key
          - Abbreviate any command, configuration, parameter, or value to its unique prefix
            Example:
                m d=i r=4k a w=2 spe=6
              same as:
                monitor duration=infinite rate=4kHz adc weight=2 speed=600kHz=1.66667μs
          - "#" in line: rest of line ignored
        Completion:
          key(s)        result:
          ------        -------
          <TAB>         Completes partially typed command, configuration, parameter,
                        or value (if unique prefix), else ...
          <TAB><TAB>    ... sounds terminal bell and second <TAB> prints list
                        of possible completions
        ...

The `using` help natters on for quite a while (violating this section's "Not Too Much More” promise) with examples, information on input editing, history, and more. But the two important points are:

1. Abbreviate anything, and/or ...
2. ... use `TAB` key completion.

#2 is the "FTW" secret. If you take away nothing else from reading this document, remember:

* PRESS THE `TAB` KEY!!! ([<sup>10</sup>](#footnote_10))

That, along with the the program's built-in help system, largely make this entire `README.md` redundant. The help system is hierarchical, starting with general information:

        $1.50: help
        Help:
        Commands, Configurations, Actions, Parameters, Values:
            <command> [[<action>] [<parameter>=<value>] ...] [<configuration> [<action>]
                      [<parameter>=<value>] ...] ... [# comment ...]
        Syntax:
          symbol                explanation
          ------                -----------
          <command>             command name
          [...]                 optional
          [<action>]            optional action name
          [<parameter>=<value>] optional parameter=value names
          <configuration>       configuration name
          ...                   optional repeats
          [# comment ...]       text from "#" to end of line ignored
        Verbose explanation:
          - Program executes Commands
          - Commands use Parameter=Value settings
          - Parameter=Value settings are grouped into Configurations
          - Configurations may have optional Actions
          - Parameters have default Values
          - Parameters retain their Values until changed by new Parameter=Value setting,
            "configure load", or program exit/restart.
          - Commands have a primary Configuration, with same name as the Command
          - Commands may have one or more secondary Configurations
          - Primary configuration is initial current Configuration
          - Configuration remains current until changed by explicit new Configuration
          - Only current Configuration's Parameters=Values can be set
          - Configurations may be used by multiple Commands
          - Only one Action allowed per Configuration per Command
        ...

and continuing on with examples and hints on how to dive deeper into the help hierarchy.

Beyond that, experimenting with the commands and their configurations and parameters (and the `TAB` key!) should suffice to get started. Or continue with reading the next section ...




<br> <a name="OK_TSIGMTW9Y"></a>
### OK;TSIGMTW9Y ("OK, This Sounds Interesting, Give Me The Whole 9 Yards")

Kudos to those who ended up (or skipped directly to) here. A brief review [<sup>11</sup>](#footnote_11), particularly for the latter, of the `buck50.py` program's usage features ...

First, any input can be fully typed, or abbreviated to its unique prefix. Each of the following sets of lines produces an identical result:

        $1.50 h
        $1.50 he
        $1.50 help

        $1.50 m
        $1.50 mon
        $1.50 monitor

        $1.50: n o=p r=1k m=b
        $1.50: num op=pu ra=1kh mo=bin
        $1.50: numbers open-pull=push-pull rate=1kHz mode=binary 

Note that all commands echo any changed parameters, both as a reminder and more importantly to confirm the expansion of any abbreviations:

        $1.50: n o=p r=1k m=b
        numbers: open-pull=push-pull
        numbers: rate=1kHz=1ms
        numbers: mode=binary

        $1.50: num op=pu ra=1kh mo=bin
        numbers: open-pull=push-pull
        numbers: rate=1kHz=1ms
        numbers: mode=binary

        $1.50: numbers open-pull=push-pull rate=1kHz mode=binary 
        numbers: open-pull=push-pull
        numbers: rate=1kHz=1ms
        numbers: mode=binary


<a name="tab_key_redux"></a>
Second, the keyboard `TAB` key completes any unambiguous partially-typed entry, possibly up a common prefix of multiple valid choices. It sounds the system "bell" otherwise, and when pressed a second time presents a list of valid choices (if any). `TAB` can be used with blank space to give a full list of valid entries in the current context.

        $1.50: s<TAB>
        $1.50: spi <TAB><TAB>
        ascii-num=  endian=     mode=       pull=       snoop=      xmit-only=
        baud=       idle=       nss-time=   rate=       socket      
        busy-wait=  ipc         nss=        rx-wait=    speed=      
        disable     master      phase=      select=     spi         
        end=        miso=       pty         slave       tx-data=    
        $1.50: spi b<TAB><TAB>
        baud=       busy-wait=  
        $1.50: spi ba<TAB>
        $1.50: spi baud=
        1.125MHz    2.25MHz     36MHz       562.5KHz    
        18MHz       281.25KHz   4.5MHz      9MHz        
        $1.50: spi baud=2<TAB><TAB>
        2.25MHz     281.25KHz   
        $1.50: spi baud=28<TAB>
        $1.50: spi baud=281.25KHz 

The painstaking example above is more complicated and time-consuming to go through than actually using the `TAB` key in the `buck50.py` itself. Just keep pressing `TAB` and follow the suggestions. 

As described in the [previous section](#NLE_WTRSBNTMM), `buck50.py` features an extensive build-in help system. The following is a (largely depth-first) guided tour through it, with occasional brief additional comments.

First, however, the program needs to be started from a UNIX shell ...


<a name="buck50_py_startup_and_device_connection"></a>
#### buck50.py startup and device connection

Commandline arguments:

        $ ./buck50.py --help
        usage: buck50.py [-h] [-v] [-q] [-f [FILE]] [-x]
                         [--halt {monitor,gpio,usart,spi,i2c,numbers}]
                         [acm]

        positional arguments:
          acm                   CDC/ACM device (with or without "/dev" or "/dev/tty"
                                prefix) (default: /dev/ttyACM0)

        optional arguments:
          -h, --help            show this help message and exit
          -v, --version         Print program/protocol version and exit (default:
                                False)
          -q, --quiet           Don't print startup info (default: False)
          -f [FILE], --file [FILE]
                                Config file to load at startup (see "help configure
                                load" (default: None)
          -x, --no-auto-viewer  Disable automatic, pre-"--file" search for waveform
                                viewing software and setting of "logic autodump=",
                                "oscope autodump=", and "dump auto-digital= auto-
                                analog= viewer-csv= viewer-vcd= digital-frmt= analog-
                                frmt=" parameters. See "help xxx ..." for those
                                commands/configurations/parameters. (default: False)
          --halt {monitor,gpio,usart,spi,i2c,numbers}
                                Experimental. Reset firmware if buck50.py exit while
                                command in progress. Must be done before buck50.py
                                without "--halt", and must specify correct command
                                name, else firmware/hardware reset and/or power cycle,
                                and USB re-enumeration required. (default: none)


Normal startup:

        $ buck50.py
        buck50.py 0.9.2
        Copyright 2020 Mark R. Rubin aka "thanks4opensource"
        This is free software with ABSOLUTELY NO WARRANTY.
        For details type "warranty" or "help warranty".
        Type "using" for program usage.
        Type "help" for commands, configurations, parameters, values.
        Connecting to buck50 device (press CTRL-C to abort ...  )
        Firmware identity match: 0xea017af5
        Firmware version  match: 0.9.2
        Device serial number:    123456789abcdef987654321
        Found /usr/bin/pulseview and /usr/bin/gnuplot. Have now set: 
        configure logic autodump=enabled oscope autodump=enabled dump output=file auto-digital=enabled auto-analog=enabled digital-frmt=csv viewer-csv=gnuplot viewer-vcd=pulseview


Note that the correct `/dev` CDC/ACM USB special file corresponding to the STM32F103 "Blue Pill" device running the buck50 firmware must given as a commandline argument if it is not the default `/dev/ttyACM0`. On Linux systems the default should be correct if no other CDC/ACM USB devices are connected (or the buck50 device was connected first, before the others).

If not, the correct `/dev` file is reported when the "Blue Pill" is connected via USB. The utilities `dmesg`, `journalctl`, and others will show output similar to:

        usb m-n.p.q: new full-speed USB device number N using xhci_hcd
        usb m-n.p.q: New USB device found, idVendor=0483, idProduct=5740
        usb m-n.p.q: New USB device strings: Mfr=1, Product=2, SerialNumber=3
        usb m-n.p.q: Product: STM32 Virtual COM Port
        usb m-n.p.q: Manufacturer: STMicroelectronics
        usb m-n.p.q: SerialNumber: 1234567890abcdef01234567
        cdc_acm m-n.p.q:1.0: ttyACMn: USB ACM device

where "m", "n", "p", "q", "N", the `SerialNumber`, and the "n" in `ttyACMn` are arbitrary values.

If the "Blue Pill" is already plugged in and more than one `/devttyACMn` file exists, various Linux utilities can be used to determine which is correct. For example:

        $ for NAME in /dev/ttyACM* ; do
        > echo $NAME
        > udevadm info --name=$NAME | egrep '0483|5740|ID_SERIAL_SHORT'
        > done

Each `/dev/tttyACMn` corresponding to a buck50 "Blue Pill" will output lines of the form:

	/dev/ttyACMn
        E: ID_MODEL_ID=5740
        E: ID_SERIAL_SHORT=1234567890abcdef01234567
        E: ID_VENDOR_ID=0483

while non-"Blue Pill"/non-buck50 devices will not have the `E: ID_MODEL_ID=5740` and `ID_VENDOR_ID=0483` lines.

If multiple buck50 "Blue Pill"s are connected (which can be very useful [<sup>12</sup>](#footnote_12)) (type `help reset ganged=` for a usage example), once again the easiest way to tell which `/dev/ttyACMn` file corresponds to which is to plug them into USB sequentially and watch the `dmesg`, etc. output.

After multiple `buck50.py` programs are connected to the multiple buck50 devices via the appropriate `/dev/ttyACM0` files, which one is which can be determined by the serial numbers reported at program startup. The serial numbers can matched to physical hardware by noting the `dmesg`, etc. output, or by the `udevadm` command described above.

Finally (and easier), the `reset blink` command will blink the user LED of the connected "Blue Pill" device for identification purposes.

On macOS the device special files should be of the form `/dev/tty.usbmodemWXYZ...` for some value of "WXYZ..." [<sup>13</sup>](#footnote_13). As noted above, it is unlikely that the `buck50.py` program will function in a Microsoft Windows&#174; environment without significant revisions.



<a name="review_license"></a>
#### review license

Review the license:

        $1.50: warranty 

            This program is free software; you can redistribute it and/or modify
            it under the terms of the GNU General Public License as published by
            the Free Software Foundation; either version 3 of the License , or
            (at your option) any later version.

            This program is distributed in the hope that it will be useful,
            but WITHOUT ANY WARRANTY; without even the implied warranty of
            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
            GNU General Public License for more details.

            You should have received a copy of the GNU General Public License
            along with this program. If not, write to

               The Free Software Foundation, Inc.
               51 Franklin Street, Fifth Floor
               Boston, MA 02110-1335  USA


<a name="help"></a>
#### help

        $1.50: help
        Help:
        Commands, Configurations, Actions, Parameters, Values:
            <command> [[<action>] [<parameter>=<value>] ...] [<configuration> [<action>]
                      [<parameter>=<value>] ...] ... [# comment ...]
        Syntax:
          symbol                explanation
          ------                -----------
          <command>             command name
          [...]                 optional
          [<action>]            optional action name
          [<parameter>=<value>] optional parameter=value names
          <configuration>       configuration name
          ...                   optional repeats
          [# comment ...]       text from "#" to end of line ignored
        Verbose explanation:
          - Program executes Commands
          - Commands use Parameter=Value settings
          - Parameter=Value settings are grouped into Configurations
          - Configurations may have optional Actions
          - Parameters have default Values
          - Parameters retain their Values until changed by new Parameter=Value setting,
            "configure load", or program exit/restart.
          - Commands have a primary Configuration, with same name as the Command
          - Commands may have one or more secondary Configurations
          - Primary configuration is initial current Configuration
          - Configuration remains current until changed by explicit new Configuration
          - Only current Configuration's Parameters=Values can be set
          - Configurations may be used by multiple Commands
          - Only one Action allowed per Configuration per Command
        Examples:
          logic
            - Command with no Parameter=Value settings, Actions, or secondary
              Configurations
          usart baud=19.2kHz
            - Command with Parameter=Value setting in primary Configuration
          pulse time=125ms mode1=rise time1=15%
            - Command with multiple Parameters=Values settings in primary Configuration
          monitor lines 6=clock 7=data
            - Command with multiple Parameter=Value settings in secondary Configuration
          reset halt
            - Command with Action in primary Configuration
          monitor spi master
            - Command with Action in secondary Configuration
          monitor rate=10Hz pb4-11=disabled adc0 enable
                  s/h+adc=239.5+12.5@12MHz->47.6kHz spi slave miso=push-pull monitor
                  file=monitored.txt output=both
            - Command with multiple Parameters=Values settings in primary Configuration,
              two secondary Configurations each with a Parameter=Value setting and an
              Action, then explicit primary Configuration with two more Parameter=Value
              settings
        Commands:
            configure, trigger, logic, oscope, dump, monitor, pulse, gpio, usart, spi,
            i2c, numbers, reset, warranty, help, using, quit
        Configurations:
            configure, monitor, pulse, numbers, gpio, usart, spi, i2c, adc0, adc1, adc2,
            adc3, adc4, adc5, adc6, adc7, logic, oscope, dump, lines, reset, socket,
            pty, ipc
        Time/Frequency Errors:
          - Many time and/or frequency parameters have limited precision due to their
            STM32F103 hardware implementations (32 bit register values, etc). Arbitrary
            Parameter=Values are rounded to the nearest achievable value and the error
            difference reported. Examples:
          31.125μs(error:-5.5ns)
            - time is 5.5ns shorter than requested
          3.13043MHz=319.444ns(error:-28.8452kHz,+2.91663ns)
            - frequency is 28.8452kHz lower, time 2.91663ns longer, than requested
          monitor
            - Command with no Parameters=Values settings, Actions, or Configurations.
              All current Parameter=Value settings in all Configurations remain in
              effect.
        More help:
            help using
            help <command>
            help <command> <parameter>
            help <command> <configuration>
            help <command> <configuration> <parameter>

Commands, configurations, actions, parameters, values: What could be simpler? ;) Hopefully further help, below, experimenting using the program (and, as always [<sup>11</sup>](#footnote_11), using the [tab key](#tab_key_redux)) will clarify.


<a name="usage_help"></a>
#### usage help

        $1.50: help using
        Help for command "using":
        Input:
          - Type "<command> ..." followed by ENTER key
          - Abbreviate any command, configuration, parameter, or value to its unique
            prefix
            Example:
                m d=i r=4k a w=2 spe=6
              same as:
                monitor duration=infinite rate=4kHz adc weight=2 speed=600kHz=1.66667μs
          - "#" in line: rest of line ignored
        Completion:
          key(s)        result:
          ------        -------
          <TAB>         Completes partially typed command, configuration, parameter,
                        or value (if unique prefix), else ...
          <TAB><TAB>    ... sounds terminal bell and second <TAB> prints list
                        of possible completions
          Example:
              n<TAB> i<TAB>3 o<TAB>p<TAB> m<TAB><TAB>g<TAB> l<TAB><TAB>o<TAB>16
            completes:
              numbers increment=3 open-pull=push-pull mode=gray low=16
            (note could use abbreviations instead):
              n i=3 o=p m=g lo=16
          Example:
              usart p<TAB><TAB>
            prints list:
              parity=  phase=   ports=   pty
            then:
              usart pa<TAB>
            completes:
              usart parity=
          Example (note space after "usart", before double <TAB> press):
              spi <TAB><TAB>
            prints list:
              ascii-num=  endian=     mode=       pull=       snoop=      xmit-only=
              baud=       idle=       nss-time=   rate=       socket
              busy-wait=  ipc         nss=        rx-wait=    speed=
              disable     master      phase=      select=     spi
              end=        miso=       pty         slave       tx-data=
            then:
              spi x<TAB>
            completes:
              spi xmit-only=
            and:
              spi xmit-only=<TAB><TAB>
            prints list:
              disabled   enabled                                                        
            and finally:
              spi xmit-only=e<TAB>
            results in:
              spi xmit-only=enabled
        Editing, history:
          (only basics listed, web search "GNU readline" for more)
            key                         result:
            -------------------------   -------
            <LEFT-ARROW>  or <CTRL+b>   cursor left
            <RIGHT-ARROW> or <CTRL+f>   cursor right
            <BACKSPACE>   or <DELETE>   delete character
            <HOME>        or <CTRL+a>   cursor to start of line
            <END>         or <CTRL+f>   cursor to end of line
            <UP-ARROW>    or <CTRL+p>   previous input line
            <DOWN-ARROW>  or <CTRL+n>   next input line
                             <CTRL+r>   search input history
                             <CTRL+g>   clear input line
        Alternate "?" help syntax:
          "?" anywhere in line same as "help <line>" (line truncated at "?")
          "?" can immediately follow command/configuration/parameter/value or be
            separated by space
          Examples ("?", " ?", and " ? " always equivalent):
            "?" syntax                                  same as:
            ----------                                  --------
            usart?                                      help usart
            usart ?                                     help usart
            numbers rate?                               help numbers rate
            numbers rate=?                              help numbers rate=
            numbers rate=2Hz?                           help numbers rate=2Hz
            monitor adc3?                               help monitor adc3
            configure usart baud?                       help configure usart baud
            configure pulse time=1kHz spi?              help configure spi
            spi mode=? endian=lsb                       help spi mode=
            monitor duration=? adc weight=3             help monitor duration
            configure gpio end=XX rate=? pulse time=1s  help gpio rate
        More help:
          Type "help" for general help on commands, configurations, actions, and
            parameters

There it is: Abbreviations, the full `TAB` key magic, the rest of the line editing capabilities, and a quicker way to get help.  Commands and other elements in the following examples are fully typed, but remember that abbreviations or `TAB` completion can always be used instead. 

Once more, it cannot be overemphasized:

* PRESS THE `TAB` KEY OFTEN!!! [<sup>14</sup>](#footnote_14)


See [Future work](#future_work), below, for small problem with "?" alternate help syntax implementation.

<a name="non_intuitive_names"></a>
Also note that the non-intuitive command names (`gpio` instead of `parallel`, `numbers` instead of `counter`) were chosen in an attempt to make commands (and other strings) unique in their first letter (or first few) for easy `TAB` completion.

Continuing the help system tour ... 

<a name="configure_command"></a>
#### `configure` command

The `configure` command displays, changes, saves, and or loads any parameters in any or all configurations. 

        $1.50: help configure
        Help for command "configure":
        View, change, load, and/or save settings
          - View any or all configurations' parameters:
          - Save or load all configurations' parameters to/from disk file
          - Adjust all time output values (trim hardware CPU clock)
        Command usage:
             configure [<parameter>=<value> ...] [<action>] [<configuration>
               <parameter>=<value ...] [<action>]
        Configurations (primary="configure"):
            configure, lines, numbers, gpio, usart, adc0, adc1, adc2, adc3, adc4, adc5,
            adc6, adc7, spi, i2c, pulse, monitor, logic, oscope, dump, reset, socket,
            pty, ipc
        Type "help configure <configuration>" for configuration description,
            parameters, and actions
        Type "help configure <configuration> <parameter>" for parameter description

##### `configure` configuration

As with all commands, `configure` has its own default configuration, also named "configure" [<sup>15</sup>](#footnote_15), with a set of parameters and actions:

        $1.50: help configure configure 
        Help for configuration "configure" (e.g. "configure configure"):
        Load and/or save all configuration parameters to file
          - Will append ".b50" to file name if missing
          - Supports <TAB> completion of filenames
        Command/configuration usage:
           configure [<parameter>=<value> ...] [<action>]
        Actions:
            save            Save all configurations/parameters to file, name...
            load            Load all configurations/parameters to file, name...
            adjust          Adjust reported times by "trim" parameter value
        Parameters:
            file=           Name of ".b50" file to load or save configuratio...
            trim=           Trim all reported times by factor of X/Y where X...
        Configuration "configure" used by commands:
            configure
        Type "help configure" for command description
        Type "help configure configure <parameter>" for parameter description
        Type "help configure configure <action>" for action description

##### `configure` parameters and actions

Configuration help (for any configuration) truncates long explanations. To see the full text:

        $1.50: help configure save
        Help for action "save" (e.g. "configure configure save"):
        - Save all configurations/parameters to file, name specified with"file="
          parameter
        Type "help configure configure" for list of configure configuration actions
        Type "help configure" for command description

        $1.50: help configure load
        Help for action "load" (e.g. "configure configure load"):
        - Load all configurations/parameters to file, name specified with"file="
          parameter
        Type "help configure configure" for list of configure configuration actions
        Type "help configure" for command description

        $1.50: help configure adjust
        Help for action "adjust" (e.g. "configure configure adjust"):
        - Adjust reported times by "trim" parameter value
        Type "help configure configure" for list of configure configuration actions
        Type "help configure" for command description

        $1.50: help configure file=
        Help for parameter "file=" (e.g. "configure configure file="):
        - Name of ".b50" file to load or save configurations and parameters. ".b50"
          suffix will be appended if missing. See "load" and "save" actions.
        Current value:
        Valid values:  <any valid file or path name>
        Type "help configure configure" for list of configure configuration parameters
        Type "help configure" for command description

        $1.50: help configure trim=
        Help for parameter "trim=" (e.g. "configure configure trim="):
        - Trim all reported times by factor of X/Y where X is currently reported time
          and Y is desired. Use "adjust" action to effect.
        Current value: 1.00000/1.00000
        Valid values:  <X/Y with X and Y floating point numbers>
        Type "help configure configure" for list of configure configuration parameters
        Type "help configure" for command description

Customization of `buck50.py` can be done by changing any of the various commands' configuration parameters and saving with `configure file=<filename> save` [<sup>16</sup>](#footnote_16). The customizations can manually loaded with `configure file=<filename> load`, or automatically at `buck50.py` startup with the `--file=` commandline option.

<a name="reset_default"></a>
Note that `buck50.py` has no explicit "reset defaults" command or action. Again, this can be easily implemented via `configure file=<filename> save` before any configuration changes (and without commandline `--file=`) and reloaded as desired.

Note that the `configure` command is useful not only for changing its own configuration parameters, but also for changing other command's configurations' parameters without executing those commands. Examples:

        $1.50: configure trim=1.00/0.97
        configure: trim=1.00000/0.970000
        configure:
                file : default.b50
                trim : 1.00000/0.970000

        $1.50: configure monitor duration=90s
        monitor: duration=1.5m
        monitor:
                rate : 4Hz=250ms
            duration : 1.5m
              pb4-11 : enabled
                file :
              output : both
              printf : %7.3f

        $1.50: configure lines 4=NSS 5=SCK 6=MISO 7=MOSI spi slave miso=push-pull tx-data=12.34.56
        spi: slave
        lines: 4=NSS
        lines: 5=SCK
        lines: 6=MISO
        lines: 7=MOSI
        spi: miso=push-pull
        spi: tx-data=12.34.56
        lines:
                   4 : NSS
                   5 : SCK
                   6 : MISO
                   7 : MOSI
                   8 : grn
                   9 : blu
                  10 : vio
                  11 : gry
        spi:
                mode : slave
           xmit-only : disabled
               snoop : disabled
              select : software
                baud : 281.25KHz
              endian : msb
                idle : low
               phase : 1st
                miso : push-pull
                pull : floating
               speed : 2MHz
                 nss : floating
             tx-data : 12.34.56
                rate : unlimited
           busy-wait : 1ms
             rx-wait : 10ms
            nss-time : zero
           ascii-num : numeric
                 end : END

Moving on to the actual functional, results--producing commands and their configurations ...



<a name="logic_command"></a>
#### `logic` command
        $1.50: help logic
        Help for command "logic":
        Digital logic analyzer.
        - capture samples of logic levels on ports PB4(lsb) through PB11(msb)
        - samples recorded only if level changes on any port
        - samples also recorded every 0.233016889 seconds regardless if any change
        - sampling starts when trigger condition met (see "help trigger")
        - sampling continues until first of:
          . user interrupt (<ENTER> key)
          . duration elapsed ("logic duration=")
          . number of samples ("logic edges=") (incl. extra samples @ 233ms)
          . memory full
        - see "help logic mode" for sampling speed
        - see "help trigger" for triggering
        - see "help reset ganged=" and "help reset ext-trig" for external triggering and
            multiple device synchronization
        - on-board user LED off at start of triggering, back on when sampling finished
        - port PB13 toggles at c. 4.29 Hz during sampling, can monitored by external
            device, drive LED at 3mA max, and/or aid post-processing of multiple device
            captures
        - Use "dump" command to upload, print, save, and/or view samples.
        Command usage:
             logic [<parameter>=<value> ...] [<action>]
        Configuration: logic
        Type "help logic logic" for configuration description, parameters, and actions
        Type "help logic <parameter>" for parameter description


##### `logic` configuration

        $1.50: help logic logic
        Help for configuration "logic" (e.g. "logic logic"):
        Digital (logic analyzer) capture parameters.
        Command/configuration usage:
           logic [<parameter>=<value> ...]
        Parameters:
            mode=           Sampling mode...
            duration=       Sampling time limit
            edges=          Maximum number of digital samples (including ext...
            code-mem=       Sampling code memory bank. "ram" faster, "flash"...
            autodump=       Automatically (CAUTION!) do "dump" command after...
        Configuration "logic" used by commands:
            configure
            logic
        Type "help logic" for command description
        Type "help logic <parameter>" for parameter description

##### `logic` parameters

<a name="logic_mode"></a>

        $1.50: help logic mode=
        Help for parameter "mode=" (e.g. "logic logic mode="):
        - Sampling mode
          - digital digital ports PB4(lsb)...PB11(msb)
          - all clock counts @ 72MHz main CPU)
          - see https://github.com/thanks4opensource/buck50/#why_so_slow
          - if "code-mem=flash":
            6.26MHz     : sequence    5@11+1@14 CPU clocks/sample (72*6/69 = 6.26MHz)
            irregular   : alternating   11 + 14  "    "      "    (72*2/25 = 5.76MHz)
            uniform     : constant           15  "    "      "    (72  /15 = 4.80MHz)
            4MHz        : constant           18  "    "      "    (72  /18 = 4.00MHz)
          - if "code-mem=ram":
            6.26MHz     : sequence    5@13+1@17 CPU clocks/sample (72*6/69 = 5.27MHz)
            irregular   : alternating   13 + 17  "    "      "    (72*2/30 = 4.80MHz)
            uniform     : constant           17  "    "      "    (72  /17 = 4.24MHz)
            4MHz        : constant           18  "    "      "    (72  /18 = 3.43MHz)
        Current value: 6.26MHz
        Valid values:  "6.26MHz", "irregular", "uniform", or "4MHz"
        Type "help logic logic" for list of logic configuration parameters
        Type "help logic" for command description

Limitations of the digital sampling speeds is a long and unhappy topic. See [Why so slow?](#why_so_slow), below.

<a name="logic_duration"></a>

        $1.50: help logic duration=
        Help for parameter "duration=" (e.g. "logic logic duration="):
        - Sampling time limit
        Current value: infinite
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", or "ns" suffix in range [910.222μs ... 59.6514s], or
                       "infinite"
        Type "help logic logic" for list of logic configuration parameters
        Type "help logic" for command description

Note the funky `our` abbreviation for "hour" in the mandatory units suffix for time values. Units are required for all time and time-or-frequency parameters (see [below](#time_freq_param) for one of many examples of the latter). The rationale is that `hour` is very rarely used while `Hz` is very common -- implementing `our` instead of `hour` avoids having to explicitly type "hz" vs "ho" to disambiguate between the two in abbreviations and `TAB` completion. [<sup>17</sup>](#footnote_17)

        $1.50: help logic edges=
        Help for parameter "edges=" (e.g. "logic logic edges="):
        - Maximum number of digital samples (including extra timing samples at c. 4Hz)
        Current value: unlimited
        Valid values:  "unlimited" or integer in range [0 ... 65535]
        Type "help logic logic" for list of logic configuration parameters
        Type "help logic" for command description

        $1.50: help logic code-mem=
        Help for parameter "code-mem=" (e.g. "logic logic code-mem="):
        - Sampling code memory bank. See "help logic logic" and
          https://github.com/thanks4opensource/buck50/#flash_vs_ram
        Current value: flash
        Valid values:  "ram" or "flash"
        Type "help logic logic" for list of logic configuration parameters
        Type "help logic" for command description

<a name="flash_vs_ram"></a>
Sampling code can be run either directly from flash memory, or copied to RAM and executed there. Current tests show flash being faster despite generally accepted wisdom to the contrary, so there is little reason to use the `code-mem=ram` option (it also slightly reduces available sample memory). See [below](#flash_faster) for more details.

        $1.50: help logic autodump=
        Help for parameter "autodump=" (e.g. "logic logic autodump="):
        - Automatically (CAUTION!) do "dump" command after completion. See "CAUTION!" in
          "help dump auto-digital."
        Current value: enabled
        Valid values:  "enabled" or "disabled"
        Type "help logic logic" for list of logic configuration parameters
        Type "help logic" for command description


<a name="oscope_command"></a>
#### `oscope` command

The `oscope` command implements a primitive two channel digital storage oscilloscope. Bandwidth, sampling rate, and particularly sample storage depth are limited.

        $1.50: help oscope
        Help for command "oscope":
        Primitive DSO (digital storage oscilloscope)
        - samples one or two analog channels chosen from ports PB0...PB7
        - primary/trigger channel: "trgr-chnl="
        - second channel: "scnd-chnl=", (set to "none" for single channel operation)
        - triggered at specified level (with hysteresis) and slope
          . see "help oscope trigger=" and "help oscope slope="
          . hysteresis implemented due to ADC conversion noise (otherwise slowly rising
            or falling signals will trigger near level regardless of "slope=" setting)
            (as noise dithers around level)
          . trigger when signal rises above level after first falling below level minus
            hysteresis if "slope=positive"
          . trigger when signal falls below level after first rising above level plus
            hysteresis if "slope=negative"
          . use "slope=disabled" to trigger immediately without regard to level
        - samples recorded continuously at rate specified by "s/h+adc="
        - sampling continues number specified by "samples=" parameter
          . triggering and/or sampling interruptible via <ENTER> key
        - on-board user LED off, and output port PB13 toggles at c. 4 Hz, during
            triggering and sampling
        - Use "dump" command to upload, print, save, and/or view samples.
        - See "help reset ganged=" and "help reset ext-trig" for external/sync'd
            triggering.
        Command usage:
             oscope [<parameter>=<value> ...] [<action>] [<configuration>
               <parameter>=<value ...] [<action>]
        Configurations (primary="oscope"):
            oscope, adc0, adc1, adc2, adc3, adc4, adc5, adc6, adc7
        Type "help oscope <configuration>" for configuration description,  parameters,
            and actions
        Type "help oscope <configuration> <parameter>" for parameter description

##### `oscope` configuration

        $1.50: help oscope oscope 
        Help for configuration "oscope" (e.g. "oscope oscope"):
        Analog DSO (digital storage oscilloscope) parameters
        Command/configuration usage:
           oscope [<parameter>=<value> ...]
        Parameters:
            samples=        Maximum number of single or dual analog samples
            trgr-chnl=      Port number PA<x> (x=0...7) of trigger/first/sin...
            scnd-chnl=      Port number PA<x> (x=0...7) of second channel, o...
            trigger=        Triggering scaling, level, and hysteresis. Level...
            slope=          Analog triggering slope (or "disabled" for immed...
            s/h+adc=        Sampling rate, sample and hold time plus analog ...
            time-scale=     Scale factor for time values in "dump" command o...
            printf=         C format string for time values in "dump" comman...
            autodump=       Automatically do "dump" command after completion
        Configuration "oscope" used by commands:
            configure
            oscope
            dump
        Type "help oscope" for command description
        Type "help oscope oscope <parameter>" for parameter description

##### `oscope` parameters

        $1.50: help oscope samples=
        Help for parameter "samples=" (e.g. "oscope oscope samples="):
        - Maximum number of single or dual analog samples
        Current value: unlimited
        Valid values:  "unlimited" or integer in range [0 ... 65535]
        Type "help oscope oscope" for list of oscope configuration parameters
        Type "help oscope" for command description

        $1.50: help oscope trgr-chnl=
        Help for parameter "trgr-chnl=" (e.g. "oscope oscope trgr-chnl="):
        - Port number PA<x> (x=0...7) of trigger/first/single channel
        Current value: 0
        Valid values:  decimal or hex integer in range [0 ... 7]
        Type "help oscope oscope" for list of oscope configuration parameters
        Type "help oscope" for command description

        $1.50: help oscope scnd-chnl=
        Help for parameter "scnd-chnl=" (e.g. "oscope oscope scnd-chnl="):
        - Port number PA<x> (x=0...7) of second channel, or "none" for single channel
          sampling
        Current value: none
        Valid values:  "none" or integer in range [0 ... 15]
        Type "help oscope oscope" for list of oscope configuration parameters
        Type "help oscope" for command description

        $1.50: help oscope trigger=
        Help for parameter "trigger=" (e.g. "oscope oscope trigger="):
        - Triggering scaling, level, and hysteresis. Level and hysteresis in scaling
          (low-to-high) units/range.
        Current value: 0:3.3:1.65:0.05
        Valid values:  <four ":"-separated floating point values>
                       (<low>:<high>:<level>:<hysteresis>)
        Type "help oscope oscope" for list of oscope configuration parameters
        Type "help oscope" for command description

As noted in `help oscope` above, hysteresis is implemented due ADC input and conversion noise. If `slope=positive`, the signal must fall to `level - hysteresis` and then rise back above `level` to trigger. If `slope=negative`,  above `level + hysteresis` then below `level`. Without hysteresis or with it set too low, slowly changing signals will trigger immediately as they approach `level` regardless of `slope` (as noise sends the measurement above and below the `level` value). 

Level and hysteresis are expressed in `trigger=` low-to-high units/range.

        $1.50: help oscope slope=
        Help for parameter "slope=" (e.g. "oscope oscope slope="):
        - Analog triggering slope (or "disabled" for immediate start of sampling)
        Current value: positive
        Valid values:  "disabled", "positive", or "negative"
        Type "help oscope oscope" for list of oscope configuration parameters
        Type "help oscope" for command description

        $1.50: help oscope s/h+adc=
        Help for parameter "s/h+adc=" (e.g. "oscope oscope s/h+adc="):
        - Sampling rate, sample and hold time plus analog conversion time. Limited set
          of values supported by hardware.
        Current value: 239.5+12.5@12MHz->47.6kHz
        Valid values:  "1.5+12.5@12MHz->857kHz", "7.5+12.5@12MHz->600kHz",
                       "13.5+12.5@12MHz->462kHz", "28.5+12.5@12MHz->293kHz",
                       "41.5+12.5@12MHz->222kHz", "55.5+12.5@12MHz->176kHz",
                       "71.5+12.5@12MHz->143kHz", or "239.5+12.5@12MHz->47.6kHz"
        Type "help oscope oscope" for list of oscope configuration parameters
        Type "help oscope" for command description

        $1.50: help oscope time-scale=
        Help for parameter "time-scale=" (e.g. "oscope oscope time-scale="):
        - Scale factor for time values in "dump" command output
        Current value: 1000000.0
        Valid values:  <any floating point number>
        Type "help oscope oscope" for list of oscope configuration parameters
        Type "help oscope" for command description

        $1.50: help oscope printf=
        Help for parameter "printf=" (e.g. "oscope oscope printf="):
        - C format string for time values in "dump" command
        Current value: %10.2f μs
        Valid values:  <"printf"-style format string with <= 64 total characters for use
                       with single float argument. Must have single "%" followed by
                       valid printf format character. May contain arbitrary additional
                       text before and/or after printf format, including optional escape
                       sequences in the form \C character, \xMN hexadecimal, \ABC octal,
                       and/ or \uABCD unicode escapes) (use "\x20" or "\040" for space)
                       (use "%%" for single literal non-format "%" character). Examples:
                       "%.2f", "%10.4e", "%g seconds", "control: %6.4f \x03bcV",
                       "%8.3f%%"
        Type "help oscope oscope" for list of oscope configuration parameters
        Type "help oscope" for command description

        $1.50: help oscope autodump=
        Help for parameter "autodump=" (e.g. "oscope oscope autodump="):
        - Automatically do "dump" command after completion
        Current value: enabled
        Valid values:  "enabled" or "disabled"
        Type "help oscope oscope" for list of oscope configuration parameters
        Type "help oscope" for command description


<a name="dump_command"></a>
#### `dump` command

The `dump` command is used for uploading, printing, and exporting captured samples.

        $1.50: help dump
        Help for command "dump":
        Upload most recent captured samples (from "logic" or "oscope" commands) to
            terminal and/or file.
        Digital ("logic") samples:
        - Initial line, range of samples. Example:
          logic samples: 0...31 @ of 32 (max 4842)
        - One line per sample. Example:
            23  0xd3  11010011   gry vio ... grn ... ... red brn        622  8.63889μs
          \--/  \--/  \------/   \-----------------------------/   \------/  \-------/
           |     |       |           |                                |          |
           |     |       |           +-- port name ("configure        |          |
           |     |       |               names <bit>=<name>") if      |          |
           |     |       |               port high else "..."         |          |
           |     |       +-- state of ports PB11(msb) to PB4(lsb)     |          |
           |     +-- state of ports PB11(msb) to PB4(lsb)             |          |
           +-- sample number                                          |          |
                                        number of 72MHz ticks since --+          |
                                        previous sample                          |
                                                            elapsed time since --+
                                                            previous sample
        Analog ("oscope") samples:
        - Initial line, range of samples. Example:
          Samples 0...59 of 60 (max 4842)   2 channels (PA0,PA1) per sample at
          239.5+12.5@12MHz->47.6kHz
        - One line per one- or two-channel  samples. Example:
          37      1554.00 μs    PA0 1.631V   PA1 77.9°F
          \--/    \----------/  \--------/   \--------/
           |           |             |            |
           |           |             |            +-- second channel value
           |           |             +-- trigger channel value
           |           +-- sample time
           +-- sample number
        Command usage:
             dump [<parameter>=<value> ...] [<action>] [<configuration>
               <parameter>=<value ...] [<action>]
        Configurations (primary="dump"):
            dump, lines, oscope, adc0, adc1, adc2, adc3, adc4, adc5, adc6, adc7
        Type "help dump <configuration>" for configuration description,  parameters, and
            actions
        Type "help dump <configuration> <parameter>" for parameter description

##### `dump` configuration

The `dump` configuration:

        $1.50: help dump dump
        Help for configuration "dump" (e.g. "dump dump"):
        Dump samples to terminal and/or file.
        Command/configuration usage:
           dump [<parameter>=<value> ...]
        Parameters:
            begin=          First sample number
            count=          Number of samples to upload, starting with "begi...
            file=           File to write samples to
            digital-frmt=   Digital dump file format
            analog-frmt=    Analog dump file format
            actives=        List of "logic" ports to dump to file (does not ...
            tick-units=     Time units per tick for digital "vcd" format. No...
            per-tick=       Counts per tick for digital "vcd" format. If usi...
            output=         Dump output destination
            viewer-csv=     External viewer program for CSV files
            viewer-vcd=     External viewer program for VCD files
            other-csv=      External viewer program for CSV files when "view...
            other-vcd=      External viewer program for VCD files when "view...
            auto-digital=   Automatically (CAUTION!) run external viewer pro...
            auto-analog=    Automatically run external viewer program. See "...
            warn-pulseview= Warn if estimated "digital-frmt=vcd" memory usag...
            linewidth=      analog gnuplot line width
            pointtype=      analog gnuplot point type
            pointsize=      analog gnuplot point size (0 for no points)
        Configuration "dump" used by commands:
            configure
            dump
        Type "help dump" for command description
        Type "help dump dump <parameter>" for parameter description

##### `dump` parameters

        $1.50: help dump begin=
        Help for parameter "begin=" (e.g. "dump dump begin="):
        - First sample number
        Current value: 0
        Valid values:  decimal or hex integer in range [0 ... 65535]
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump count=
        Help for parameter "count=" (e.g. "dump dump count="):
        - Number of samples starting with "begin="
        Current value: unlimited
        Valid values:  "unlimited" or integer in range [1 ... 65535]
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump file=
        Help for parameter "file=" (e.g. "dump dump file="):
        - File to write samples to
        Current value:
        Valid values:  <any valid file or path name>
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

<a name="dump_digital_frmt"></a>

        $1.50: help dump digital-frmt=
        Help for parameter "digital-frmt=" (e.g. "dump dump digital-frmt="):
        - Digital dump file format
        Current value: vcd
        Valid values:  "csv" or "vcd"
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump analog-frmt=
        Help for parameter "analog-frmt=" (e.g. "dump dump analog-frmt="):
        - Analog dump file format
        Current value: csv
        Valid values:  csv
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump actives=
        Help for parameter "actives=" (e.g. "dump dump actives="):
        - List of "logic" ports to dump to file (does not affect terminal or "oscope"
          output)
        Current value: 4.5.6.7.8.9.10.11
        Valid values:  1 to 8 "."-separated integers in range 4...11
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

<a name="pulseview_parameters"></a>

        $1.50: help dump tick-units=
        Help for parameter "tick-units=" (e.g. "dump dump tick-units="):
        - Time units per tick for digital "vcd" format. Note only "us", "ns", and "fs"
          valid for `pulseview`. See "help dump per-tick"
        Current value: ns
        Valid values:  "s", "ms", "us", "ns", "ps", "fs", "as", "zs", or "ys"
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump per-tick=
        Help for parameter "per-tick=" (e.g. "dump dump per-tick="):
        - Counts per tick for digital "vcd" format. If using "pulseview", set this
          parameter and "tick-units" as large as possible without exceeding the shortest
          anticipated time period between samples. See
          https://github.com/thanks4opensource/buck50/#pulseview
        Current value: 125
        Valid values:  decimal or hex integer in range [1 ... 1000000000000000]
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

See [buck50.py VCD/pulseview parameters](#pulseview_buck50), below, for discussion regarding `per-tick` parameter.

        $1.50: help dump output=
        Help for parameter "output=" (e.g. "dump dump output="):
        - Dump output destination
        Current value: file
        Valid values:  "terminal" or "file"
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump viewer-csv=
        Help for parameter "viewer-csv=" (e.g. "dump dump viewer-csv="):
        - External viewer program for CSV files
        Current value: gnuplot
        Valid values:  "gnuplot", "pulseview", "gtkwave", or "other"
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

<a name="dump_viewer_vcd"></a>

        $1.50: help dump viewer-vcd=
        Help for parameter "viewer-vcd=" (e.g. "dump dump viewer-vcd="):
        - External viewer program for VCD files
        Current value: pulseview
        Valid values:  "gnuplot", "pulseview", "gtkwave", or "other"
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump other-csv=
        Help for parameter "other-csv=" (e.g. "dump dump other-csv="):
        - External viewer program for CSV files when "viewer-vcd=other". May contain
          "%s" character for filename. Use \040 for spaces between commandline arguments
        Current value:
        Valid values:  <any string <= 1024 chars including optional \C character, \xMN
                       hexadecimal, \ABC octal, and/ or \uABCD unicode escapes) (use
                       "\x20" or "\040" for space)>
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump other-vcd=
        Help for parameter "other-vcd=" (e.g. "dump dump other-vcd="):
        - External viewer program for VCD files when "viewer-vcd=other". May contain
          "%s" character for filename. Use \040 for spaces between commandline arguments
        Current value:
        Valid values:  <any string <= 1024 chars including optional \C character, \xMN
                       hexadecimal, \ABC octal, and/ or \uABCD unicode escapes) (use
                       "\x20" or "\040" for space)>
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

<a name="dump_auto"></a>

        $1.50: help dump auto-digital=
        Help for parameter "auto-digital=" (e.g. "dump dump auto-digital="):
        - Automatically (CAUTION!) run external viewer program. See "viewer_vcd=" and
          "viewer_csv=" parameters. CAUTION! If "viewer_vcd=pulseview", pathological
          "logic" sampling captures may cause almost unlimited host memory consumption
          and possible system lockup. See
          https://github.com/thanks4opensource/buck50/#pulseview
        Current value: enabled
        Valid values:  "enabled" or "disabled"
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump auto-analog=
        Help for parameter "auto-analog=" (e.g. "dump dump auto-analog="):
        - Automatically run external viewer program. See "viewer_vcd=" and "viewer_csv="
          parameters.
        Current value: enabled
        Valid values:  "enabled" or "disabled"
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump warn-pulseview=
        Help for parameter "warn-pulseview=" (e.g. "dump dump warn-pulseview="):
        - Warn if estimated "digital-frmt=vcd" memory usage exceeds value and might
          trigger host system slowdown or lockup when file loaded into `pulseview`. See
          "dump auto-digital=" and
          https://github.com/thanks4opensource/buck50/#pulseview
        Current value: 100000000.0
        Valid values:  <any floating point number>
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump linewidth=
        Help for parameter "linewidth=" (e.g. "dump dump linewidth="):
        - analog gnuplot line width
        Current value: 0.0
        Valid values:  <any floating point number>
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump pointtype=
        Help for parameter "pointtype=" (e.g. "dump dump pointtype="):
        - analog gnuplot point type
        Current value: none
        Valid values:  "none", "plus", "x", "asterisk", "square-open", "square-filled",
                       "circle-open", "circle-filled", "delta-open", "delta-filled",
                       "nabla-open", "nabla-filled", "diamond-open", "diamond-filled",
                       "arrow-open", or "arrow-filled"
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description

        $1.50: help dump pointsize=
        Help for parameter "pointsize=" (e.g. "dump dump pointsize="):
        - analog gnuplot point size (0 for no points)
        Current value: 0.0
        Valid values:  <any floating point number>
        Type "help dump dump" for list of dump configuration parameters
        Type "help dump" for command description


<a name="trigger_command"></a>
#### `trigger` command

The `trigger` command allows viewing and modification of the triggering state machine used by the `logic` command.

Far more than a [GUI](#Wheres_the_gui),  `buck50.py` needs a custom language and compiler for defining its trigger state machines. The state machine mechanism supports very sophisticated triggering scenarios, but defining the states is error prone and somewhat non-intuitive. Particularly difficult is implementing logical-OR ("A or B or C") conditions. Hopefully the following description and examples will help:

        $1.50: help trigger
        Help for command "trigger":
        Digital capture/sampling triggering.
        Usage:
            trigger                                         list trigger states
            trigger <n>=<abcdefgh>-<pass>-<fail> [text]     define trigger state
            trigger check                                   check trigger states
            trigger delete n[-m] [p[-q] ...                 delete trigger state(s)
            trigger undo                                    undo last define or delete
          where:
            state number        0...255
            a,b,c,d,e,f,g,h     state bits:
                                  X = don't care     alternate chars: pPUu.*
                                  0 = port low       alternate chars: oO
                                  1 = port high      alternate chars: iI
            pass                state number, 0...255
            fail                state number, 0...255
            text                optional comments
          Examples:
            trigger  0=xxxx101x-1-0
            trigger 17=11001111-23-15
        Trigger state machine pseudocode:
          set state to 0
          loop while not triggered:
              if connected to other units and PB14 external trigger is low:
                  triggering complete, exit loop, start sample capture
              else:
                  set input to ports PB4...PB11 (lsb...msb)
                  set begin_state to state
              loop while checking input:
                  if input matches state's bits (masked by "don't care" bits):
                      if state's "pass" state is 0:
                          triggering complete, exit both loops, start sample capture
                      else:
                          set state to state's "pass" state
                          exit inner "checking input" loop, continue outer loop
                  else:
                      set state to state's "fail" state
                      if state is same as begin_state:
                          exit inner "checking trigger" loop, continue outer loop
                      else:
                          continue inner "checking trigger" loop
        Triggers:
          - at least one trigger state, state #0, required
          - default state machine: one trigger, "0=xxxxxxxx-0-0
          - trigger state numbers arbitrary in range [0 ... 255]
          - state numbers need not be contiguous (slight download speed advantage if so)
          - "pass" and "fail" states must point to existing/defined trigger states      
          - chain of "pass" states back to state 0 (triggering complete) required
          - logical "OR" implemented by chains of "pass" states looping back first in
            chain or ending with "xxxxxxxx-X-Y" all "don't care" state
          - warnings for above and other requirements reported by "trigger check" and at
            start of "logic" command
          - cannot trigger on implicitly clocked asynchronous (UART) serial data, only
            synchronous/clocked (USART, SPI, etc)
          - see "help logic mode" for sampling speed after triggering
          - note triggering speed is slower than sampling speed, especially if multiple
            logical "OR" states
          - note minor additional slowdown if multiple units connected (see "help reset
            ganged=")
        Triggers examples:
          Trigger immediately (default):
            t 0=xxxxxxxx-0-0 any input matches, go to state 0, triggered
          Trigger on bit pattern:
            t 0=x1x101x0-0-0 trigger if pattern matches, else continue checking input
          Trigger on rising edge on port 9
            t 0=xx0xxxxx-1-0 wait until port PA9 is low
            t 1=xx1xxxxx-0-1 trigger when PA9 goes high
          Trigger on one bit pattern followed by second bit pattern
            t 0=xxxx01xx-1-0 go to state 1 if pattern matches, else recheck
            t 1=111xxxxx-0-1 trigger if new pattern matches, else recheck
          Trigger on one bit pattern followed immediately by second bit pattern
            t 0=xxxxxxxx-1-0 go to wait for state 1 match
            t 1=xxxx01xx-2-1 go to state 2 if pattern matches, else recheck
            t 2=111xxxxx-0-3 trigger if new pattern matches, else ...
            t 3=xxxxxxxx-1-1 ... recheck first pattern
          Trigger on serial 1-0-1 bitstream on PA8, clocked on rising edge on PA6
            t 0=xxxxxxxx-1-0 start state machine
            t 1=xxx0xxxx-2-1 wait until clock line PA8 low
            t 2=xxx1xxxx-3-2 wait until clock line PA8 high
            t 3=xxx1x1xx-4-0 continue if clocked PA6 bit is 1, else restart
            t 4=xxx0xxxx-5-4 wait until clock line PA8 low
            t 5=xxx1xxxx-6-5 wait until clock line PA8 high
            t 6=xxx1x0xx-7-0 continue if clocked PA6 bit is 0, else restart
            t 7=xxx0xxxx-8-7 wait until clock line PA8 low
            t 8=xxx1xxxx-9-8 wait until clock line PA8 high
            t 9=xxx1x1xx-0-0 trigger if clocked PA6 bit is 1, else restart
          Trigger on binary 3, 4, or 10 on ports PA4(lsb) through PA7(msb)
            t 0=xxxxxxxx-1-0 go to check for 3, 4, or 10
            t 1=xxxx0011-0-2 trigger if binary 3,  else check if binary 4
            t 2=xxxx0100-0-3 trigger if binary 4,  else check if binary 10
            t 3=xxxx1010-0-1 trigger if binary 10, else end 3, 4, or 10 check
          Trigger on 7-bit ascii "u", "$", or 'Z", followed immediately by "~", on
            PA4(lsb) through PA10, all data clocked on falling edge on PA11
            t 0=xxxxxxxx-1-0 start state machine                                        
            t 1=1xxxxxxx-2-1 wait for clock high
            t 2=0xxxxxxx-3-2 wait for clock low
            t 3=01110101-7-4 check if "u", else check if "$"
            t 4=00100100-7-5 check if "$", else check if "Z"
            t 5=01011010-7-6 check if "Z", else end "u" or "$" or "Z" check
            t 6=xxxxxxxx-1-7 end  "u" or "$" or "Z" check
            t 7=1xxxxxxx-8-7 wait for clock high
            t 8=0xxxxxxx-9-8 wait for clock low
            t 9=01111110-0-0 trigger if "~", else restart from beginning
          Never trigger (useful if only external triggering desired, see below):
            t 0=xxxxxxxx-1-0 start state machine
            t 1=00000000-2-1 go to state 2 on success
            t 2=11111111-1-2 go to state 1 on success
        External/sync'd triggering:
          - See "help reset ganged=" and "help reset ext-trig"
        Command usage:
             trigger [<parameter>=<value> ...] [<action>]
        Configuration: triggers
        Type "help trigger triggers" for configuration description, parameters, and
            actions
        Type "help trigger <parameter>" for parameter description
        Type "help trigger <action>" for action description

Regarding `trigger check`, I'll admit my "graph theory fu" is not quite at the ninja [<sup>18</sup>](#footnote_18) level I'd like it to be. The attempt is to warn about trigger state machines that cannot trigger regardless of the inputs fed to them. Such a state machine will cause the buck50 firmware on the "Blue Pill" to pause indefinitely, although because ...

1. the `logic` command does an automatic check before starting and issues the warning if necessary, and ...
2. triggering can always be interrupted by user input (`ENTER` key), and ...
3. if the `logic duration=` parameter is not `infinite`, triggering will timeout and abort automatically

... bad state machines are not a serious problem.  Also, in certain circumstances it may be useful to purposely define such a state machine, as per the "external trigger only" example in the help text. If anything, `trigger check` is likely overly restrictive in the warnings it issues.

##### `trigger` configuration and actions

        $1.50: help trigger triggers 
        Help for configuration "triggers" (e.g. "trigger triggers"):
        Command usage:
            trigger <state number> <abcdefgh>-<pass state>-<fail state> [text]
        where:
            state number      0...255
            a,b,c,d,e,f,g,h   X = don't care   alternate chars: pPUu.*
                              0 = port low     alternate chars: oO
                              1 = port high    alternate chars: iI
            pass state        0...255
            fail state        0...255
            text              optional comments
        Type "help trigger"
        Command/configuration usage:
           trigger triggers [<parameter>=<value> ...] [<action>]
        Actions:
            check           Check triggers for conflicts
            delete          Delete one or more triggers: n[-m] [p[-q] ...
            undo            Undo last trigger command change or delete
        Type "help trigger" for command description

`TAB` completion supports re-editing existing trigger states, and modifying them into new ones without excessive typing ([<sup>19</sup>](#footnote_19)). The "alternate characters" are supported in case they make entering "1", "0", and "x" bits easier (adjacent keys on keyboard).

Moving past triggering, sampling, and dumping ...


<a name="monitor_command"></a>
#### `monitor` command

        $1.50: help monitor
        Help for command "monitor":
        Monitor and/or log digital, analog, spi, and/or usart inputs
        Continue until "duration=<time>" (if not "infinite") elapsed, or user interrupt
            via <ENTER> key.
        Prints text line with monitored data to terminal ("output=terminal"), file
            ("file=<filename> output=file) or both ("file=<filename> output=both")
        Lines printed if rate limiting ("rate=<time>") time elapsed and any of the
            following conditions met:
          digital:("pb4-11=enabled")
            - level change on any digital port PB4 through PB11
          analog: ("adcN enable")
            - analog voltage change on enabled analog configs "adc<N>" for N in 0...7
            - values filtered by "adcN exponent=", "weight=", and "hysteresis="
            parameters
              via formula:
                filtered_voltage =   filtered_voltage * (pow(2,exponent) - weight)
                                   + current_voltage  * (                  weight)
              but only if:
                abs(filtered_voltage - previous_printed_voltage) > hysteresis
            - values printed using "adcN scale-hyst=" scaling, with "name=" name, and
            "printf=" printf formula including optional additional text
            - Example:
              Port PA4 connected to analog temperature sensor with 0 to 3.3V output
            corresponding to -20 to 160 degrees Fahrenheit:
              monitor adc4 enable scale-hyst=-20:160:0.5 exponent=3 weight=5
            name=outdoors printf=%5.1f\x20F
              If scaled current value filtered with previous value using ratio of 5/8 to
            3/8 is 82 and previous printed value was less than 81.5 or greater than 82.5
            will print:
              Xoutdoors: 82.0 F
              (see below for "x" error status character, normally blank space)
          usart: ("usart enable")
            - "usart synchro=disabled": if RX data received (note also "usart
            tx-data=<byte>" sent on TX port each "rate=<time>" period)
            - "usart synchro=enabled": if read RX data read on port different than
            previous
          spi:
            "mode=master": Send "tx-data=" on MOSI once per "rate=" period. Print
            received MISO data if different than previous printed
            "mode=slave": Print received MOSI data (having sent "tx-data=" on MISO
            one-to-one with MOSI bytes)
          i2c:
            "mode=master": Send "tx-data=" on per "rate=" period. Print received slave
            TX  data if different than previous printed
            "mode=slave": Print received master data. Also sends "tx-data=" (if any), as
            slave-tx-to-master.
        Prints one or more fields depending on enabled configurations/parameters:
          all:
              004.491                                                                   
              \-----/  seconds since command start ("monitor printf=<format code>")
          digital: ("pb4-11=enabled") state of digital ports PB4 to PB11. Example:
              10111011   gry ... blu grn yel ... red brn
              \------/   \-----------------------------/
                 |           |
                 |           +-- port name ("configure names <bit>=<name>")
                 |               if port high else "..."
                 +-- state of ports, PB11(msb) to PB4(lsb)
          analog: ("adcN enable") voltage on analog port PA<N>. Example:
            (see above for more details)
               PA2:1.963V
              |||||||||||
              |||||++++++-- "scale-hyst=" scaled value, "printf=" formula
              ||||+-------- ascii ":" character
              |+++--------  "name=" name
              +-----------  error code, normally blank (see below)
          usart: ("usart enable=enabled"), RX data on port PA10
               u:pr.rs...
              |||||||||||
              |||++++++++-- RX data bytes, length of "spi tx-data=",
              |||             two hexadecimal digits per byte, separated by "."
              ||+---------- ascii ":" character
              |+----------- ascii "u" character
              +------------ error code, normally blank (see below)
          spi:
               s:pr.rs...
              |||||||||||
              |||++++++++-- MISO or MOSI data bytes, length of "spi tx-data=",
              |||             two hexadecimal digits per byte, separated by "."
              ||+---------- ascii ":" character
              |+----------- ascii "s" character
              +------------ error code, normally blank (see below)
          i2c:
               mi:pr.rs...
              ||||||||||||
              ||||++++++++-- received data bytes (master or slave),
              ||||             two hexadecimal digits per byte, separated by "."
              |||+---------- ascii ":" character
              ||+----------- ascii "i" character
              |+------------ master:ascii "m"  slave:"0"/"1"/"2"==gen-call/oar1/oar2
              +------------- error code, normally blank (see below)
          all:
              (<ENTER> to abort) ...
          error codes:
              " "   no error
              "E"   receive data register empty (usart, spi)
              "B"   peripheral busy
              "O"   receive data overrun
              "T"   send or receive data hardware timeout                               
              "F"   generic error
              "L"   line break (usart)
              "N"   noise (usart)
              "P"   parity (usart)
              "U"   framing (usart)
              "S"   start (i2c)
              "A"   address (i2c)
              "N"   nack (i2c)
              "X"   missing BTF ("byte transfer finished") flag (i2c)
              "R"   missing RXNE ("receive register not empty") flag (i2c)
              "P"   missing STOP (i2c)
        Command usage:
             monitor [<parameter>=<value> ...] [<action>] [<configuration>
               <parameter>=<value ...] [<action>]
        Configurations (primary="monitor"):
            monitor, lines, usart, spi, i2c, adc0, adc1, adc2, adc3, adc4, adc5, adc6,
            adc7
        Type "help monitor <configuration>" for configuration description,  parameters,
            and actions
        Type "help monitor <configuration> <parameter>" for parameter description

`monitor` is probably the most complex and configurable of all the `buck50.py`  commands. Many different options. Its various output formats should be fairly self-explanatory in actual use. The `spi` and `usart` configuration parameters will be described below in the sections on the [spi](#spi_command) and [usart](#usart_command) commands.

See [lines configuration](#lines_config), below, for info on the `lines` configuration, also used by the `monitor` command.

##### `monitor` configuration

        $1.50: help monitor monitor 
        Help for configuration "monitor" (e.g. "monitor monitor"):
        Command usage:
            monitor [<parameter>=<value> ...]
        Parameters:
            rate=           Max update frequency
            duration=       Halt after time elapsed
            pb4-11=         Monitor/log digital inputs
            file=           Log file name
            output=         Log to file ("file" or "both") only if "file=" i...
            printf=         Print format for time (seconds). Single C-style ...
        Configuration "monitor" used by commands:
            configure
            monitor
        Type "help monitor" for command description
        Type "help monitor monitor <parameter>" for parameter description

##### `monitor` parameters

        $1.50: help monitor rate=
        Help for parameter "rate=" (e.g. "monitor monitor rate="):
        - Max update frequency
        Current value: "4Hz=250ms"
        Valid values:  floating point number with "y", "d", "h", "min", "s", "ms",
                        "us", "μs", "ns", "Hz", "kHz", "MHz", or "GHz" suffix in range
                       [72MHz=13.8889ns ... 0.1249nHz=253.881y], or "unlimited"
        Type "help monitor monitor" for list of monitor configuration parameters
        Type "help monitor" for command description

        $1.50: help monitor duration=
        Help for parameter "duration=" (e.g. "monitor monitor duration="):
        - Halt after time elapsed
        Current value: "infinite"
        Valid values:  floating point number with "y", "d", "h", "min", "s", "ms",
                        "us", "μs", or "ns" suffix in range [13.8889ns ... 253.881y], or
                       "infinite"
        Type "help monitor monitor" for list of monitor configuration parameters
        Type "help monitor" for command description

The `pb4-11=` parameter allows disabling monitoring of those digital input lines if not needed which can be useful to reduce output clutter if `analog`, `usart`, and/or `spi` monitoring is enabled in their respective configurations.

        $1.50: help monitor pb4-11=
        Help for parameter "pb4-11=" (e.g. "monitor monitor pb4-11="):
        - Monitor/log digital inputs
        Current value: "enabled"
        Valid values:  "enabled" or "disabled"
        Type "help monitor monitor" for list of monitor configuration parameters
        Type "help monitor" for command description

More `monitor` parameters:

        $1.50: help monitor file=
        Help for parameter "file=" (e.g. "monitor monitor file="):
        - Log file name
        Current value: ""
        Valid values:  <any valid file or path name>
        Type "help monitor monitor" for list of monitor configuration parameters
        Type "help monitor" for command description

        $1.50: help monitor output=
        Help for parameter "output=" (e.g. "monitor monitor output="):
        - Log to file ("file" or "both") only if "file=" is also set)
        Current value: "both"
        Valid values:  "terminal", "file", or "both"
        Type "help monitor monitor" for list of monitor configuration parameters
        Type "help monitor" for command description

        $1.50: help monitor printf=
        Help for parameter "printf=" (e.g. "monitor monitor printf="):
        - C format string for time (seconds).
        Current value: %7.3f
        Valid values:  <"printf"-style format string with <= 64 total characters for use
                       with single float argument. Must have single "%" followed by
                       valid printf format character. May contain arbitrary additonal
                       text before and/or after printf format, including optional escape
                       sequences in the form \C character, \xMN hexadecimal, \ABC octal,
                       and/ or \uABCD unicode escapes) (use "\x20" or "\040" for space)
                       (use "%%" for single literal non-format "%" character). Examples:
                       "%.2f", "%10.4e", "%g seconds", "control: %6.4f \x03bcV",
                       "%8.3f%%"
        Type "help monitor monitor" for list of monitor configuration parameters
        Type "help monitor" for command description



<a name="lines_config"></a>
### `lines` configuration

The `lines` configuration parameters are used by `dump` and `monitor` commands to associate default or user chosen names with the PB4 through PB11 digital input ports. The names are also output to the terminal by `monitor` and `dump`, and included in some `dump` output file formats for use when viewing with external [Waveform viewing software](#waveform_viewing_software).

As with all configurations/parameters, they can be viewed and changed using `configure`, so help for their configuration can be accessed via that command:

        $1.50: help configure lines lines
        Help for configuration "lines" (e.g. "configure lines"):
        Names for digital input ports PB4 through PB11.
        Command usage:
            configure lines [<parameter>=<value> ...]
        Parameters:
            4=              Symbolic name for input pin PB4
            5=              Symbolic name for input pin PB5
            6=              Symbolic name for input pin PB6
            7=              Symbolic name for input pin PB7
            8=              Symbolic name for input pin PB8
            9=              Symbolic name for input pin PB9
            10=             Symbolic name for input pin PB10
            11=             Symbolic name for input pin PB11
        Configuration "lines" used by commands:
            configure
            dump
            monitor
        Type "help configure" for command description
        Type "help configure lines <parameter>" for parameter description

#### `lines` parameters:

        $1.50: help configure lines 4=
        Help for parameter "4=" (e.g. "configure lines 4="):
        - Symbolic name for input pin PB4
        Current value: "brn"
        Valid values:  <any string <= 16 chars, \xMN hex escapes allowed>
        Type "help configure lines" for list of names configuration parameters
        Type "help configure" for command description

and similarly for `5=` though `11=`.

The default names are:

        $1.50: configure lines
        lines:
                   4 : brn
                   5 : red
                   6 : org
                   7 : yel
                   8 : grn
                   9 : blu
                  10 : vio
                  11 : gry

Apologies that this omits "black" from the standard resistor code color-to-numeral mapping, but being a simple-minded software engineer, "black" irrevocably means "ground" in my mind (my buck50 "Blue Pills" are color-coded as above). The resistor color codes [<sup>20</sup>](#footnote_20) are not as firmly burned into my brain as they are for EEs, and I'll use as a further excuse the fact that the buck50 PB ports also don't start at "0" (see [Port assignments](#port_assignments), below).

In any case, the names can be modified to suit (likely with signal names instead of colors -- "clock", "data", etc) and saved/reloaded via `configure save file=<filename>` and `configure save ...`). Or the `buck50.py` source code can be easily modified (open-source software and all that).


<a name="adcN_configs"></a>
### `adcN` configurations

The `adc0` through `adc7` configurations store parameters for ADC conversion on input ports PA0 through PA7, respectively. Several commands rely on some or all of these parameters.

Help for the `adc0` configuration is listed below (`adc1` through `adc7` are analogous):

        $1.50: help configure adc0
        Help for configuration "adc0" (e.g. "configure adc0"):
        Analog-to-digital input on port PA0.
        Filtering/scaling:
          - Filtering for "monitor" command (computed by firmware on STM32F103):
              new_filtered_value =   prev_filtered_value * (pow(2,exponent) - weight)
                                   + current_adc_value   * (                  weight)
            but then not reported unless:
              abs(new_filtered_value - prev_reported_value) > hysteresis
          - Scaling for "monitor" and "dump" commands (computed by software on host):
              normalized = value / float(max_12_bit_adc_value)
              scaled     = (1.0 - normalized) * lowscale + normalized * highscale
        Command/configuration usage:
           configure adc0 [<parameter>=<value> ...] [<action>]
        Actions:
            enable          Set parameter "active=enabled"
            disable         Set parameter "active=disabled"
        Parameters:
            active=         For "monitor" command
            s/h+adc=        Sampling rate, sample and hold time plus analog ...
            scale-hyst=     Scaling units/range for "monitor" and "dump" com...
            exponent=       Filter width. See "analog" section in "help moni...
            weight=         Filter width. See "analog" section in "help moni...
            name=           Port/channel name for "monitor", "oscope", and "...
            printf=         C format string to print scaled value. For "moni...
        Configuration "adc0" used by commands:
            configure
            oscope
            dump
            monitor
        Type "help configure" for command description
        Type "help configure adc0 <parameter>" for parameter description
        Type "help configure adc0 <action>" for action description

##### `adcN` parameters and actions

        $1.50: help configure adc0 enable
        Help for action "enable" (e.g. "configure adc0 enable"):
        - Set parameter "active=enabled"
        Type "help configure adc0" for list of adc0 configuration actions
        Type "help configure" for command description

        $1.50: help configure adc0 disable
        Help for action "disable" (e.g. "configure adc0 disable"):
        - Set parameter "active=disabled"
        Type "help configure adc0" for list of adc0 configuration actions
        Type "help configure" for command description

        $1.50: help configure adc0 active=
        Help for parameter "active=" (e.g. "configure adc0 active="):
        - For "monitor" command
        Current value: disabled
        Valid values:  "enabled" or "disabled"
        Type "help configure adc0" for list of adc0 configuration parameters
        Type "help configure" for command description

        $1.50: help configure adc0 s/h+adc=
        Help for parameter "s/h+adc=" (e.g. "configure adc0 s/h+adc="):
        - Sampling rate, sample and hold time plus analog conversion time. Limited set
          of values supported by STM32F103.
        Current value: 239.5+12.5@12MHz->47.6kHz
        Valid values:  "1.5+12.5@12MHz->857kHz", "7.5+12.5@12MHz->600kHz",
                       "13.5+12.5@12MHz->462kHz", "28.5+12.5@12MHz->293kHz",
                       "41.5+12.5@12MHz->222kHz", "55.5+12.5@12MHz->176kHz",
                       "71.5+12.5@12MHz->143kHz", or "239.5+12.5@12MHz->47.6kHz"
        Type "help configure adc0" for list of adc0 configuration parameters
        Type "help configure" for command description

        $1.50: help configure adc0 scale-hyst=
        Help for parameter "scale-hyst=" (e.g. "configure adc0 scale-hyst="):
        - Scaling units/range for "monitor" and "dump" commands, and hysteresis value
          for "monitor"
        Current value: 0:3.3:0
        Valid values:  <three ":"-separated floating point values>
                       (<low>:<high>:<hysteresis>)
        Type "help configure adc0" for list of adc0 configuration parameters
        Type "help configure" for command description

        $1.50: help configure adc0 exponent=
        Help for parameter "exponent=" (e.g. "configure adc0 exponent="):
        - Filter width. See "analog" section in "help monitor".
        Current value: 1
        Valid values:  decimal or hex integer in range [0 ... 8]
        Type "help configure adc0" for list of adc0 configuration parameters
        Type "help configure" for command description

        $1.50: help configure adc0 weight=
        Help for parameter "weight=" (e.g. "configure adc0 weight="):
        - Filter width. See "analog" section in "help monitor".
        Current value: 1
        Valid values:  decimal or hex integer in range [0 ... 255]
        Type "help configure adc0" for list of adc0 configuration parameters
        Type "help configure" for command description

        $1.50: help configure adc0 name=
        Help for parameter "name=" (e.g. "configure adc0 name="):
        - Port/channel name for "monitor", "oscope", and "dump" commands' output.
        Current value: PA0
        Valid values:  <any string <= 16 chars including optional \C character, \xMN
                       hexadecimal, \ABC octal, and/ or \uABCD unicode escapes) (use
                       "\x20" or "\040" for space)>
        Type "help configure adc0" for list of adc0 configuration parameters
        Type "help configure" for command description

        $1.50: help configure adc0 printf=
        Help for parameter "printf=" (e.g. "configure adc0 printf="):
        - C format string to print scaled value. For "monitor" and "oscope" commands
        Current value: %5.3fV
        Valid values:  <"printf"-style format string with <= 32 total characters for use
                       with single float argument. Must have single "%" followed by
                       valid printf format character. May contain arbitrary additonal
                       text before and/or after printf format, including optional escape
                       sequences in the form \C character, \xMN hexadecimal, \ABC octal,
                       and/ or \uABCD unicode escapes) (use "\x20" or "\040" for space)
                       (use "%%" for single literal non-format "%" character). Examples:
                       "%.2f", "%10.4e", "%g seconds", "control: %6.4f \x03bcV",
                       "%8.3f%%"
        Type "help configure adc0" for list of adc0 configuration parameters
        Type "help configure" for command description



<a name="numbers_command"></a>
### `numbers` command

The poorly-name `numbers` command (see [above](#non_intuitive_names), should be "counter") outputs sequential values on the "Blue Pill" GPIOB port lines:

        $1.50: help numbers
        Help for command "numbers":
        Output binary values on ports PB-4 (lsb) to PB-11 (msb).
        - Configurable rate, or unlimited (maximum firmware speed)
        - Incrementing/decrementing values
        - High/low limits
        - Binary or gray code
        Command usage:
             numbers [<parameter>=<value> ...] [<action>]
        Configuration: numbers
        Type "help numbers numbers" for configuration description, parameters, and
            actions
        Type "help numbers <parameter>" for parameter description

##### `numbers` configuration

        $1.50: help numbers numbers
        Help for configuration "numbers" (e.g. "numbers numbers"):
        "limited="   duration used if "rate=unlimited"
        "unlimited=" duration otherwise
        Command/configuration usage:
           numbers [<parameter>=<value> ...]
        Parameters:
            mode=           Numeric sequence encoding
            increment=      Change between successive values
            low=            Low value of low-to-high values range
            high=           High value of low-to-high values range
            rate=           Frequency/speed if not "rate= unlimited". See "T...
            gpio-speed=     Hardware slew rate
            open-pull=      Hardware port mode
            limited=        Command duration if "rate=limited"
            unlimited=      Command duration if "rate=unlimited"
        Configuration "numbers" used by commands:
            configure
            numbers
        Type "help numbers" for command description
        Type "help numbers <parameter>" for parameter description

##### `numbers` parameters

        $1.50: help numbers mode=
        Help for parameter "mode=" (e.g. "numbers numbers mode="):
        - Numeric sequence encoding
        Current value: binary
        Valid values:  "binary" or "gray"
        Type "help numbers numbers" for list of numbers configuration parameters
        Type "help numbers" for command description

        $1.50: help numbers increment=
        Help for parameter "increment=" (e.g. "numbers numbers increment="):
        - Change between successive values
        Current value: 1
        Valid values:  decimal or hex integer in range [-127 ... 127]
        Type "help numbers numbers" for list of numbers configuration parameters
        Type "help numbers" for command description

        $1.50: help numbers low=
        Help for parameter "low=" (e.g. "numbers numbers low="):
        - Low value of low-to-high values range
        Current value: 0
        Valid values:  decimal or hex integer in range [0 ... 255]
        Type "help numbers numbers" for list of numbers configuration parameters
        Type "help numbers" for command description

        $1.50: help numbers high=
        Help for parameter "high=" (e.g. "numbers numbers high="):
        - High value of low-to-high values range
        Current value: 255
        Valid values:  decimal or hex integer in range [0 ... 255]
        Type "help numbers numbers" for list of numbers configuration parameters
        Type "help numbers" for command description

<a name="time_freq_param"></a>

        $1.50: help numbers rate=
        Help for parameter "rate=" (e.g. "numbers numbers rate="):
        - Frequency/speed if not "rate= unlimited". See "Time/Frequency Errors" section
          in "help". Additional rate inaccuracies due to firmware speed limitations
          at rates greater than approx. 100kHz.
        Current value: 4Hz=250ms
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", "ns", "Hz", "kHz", "MHz", or "GHz" suffix in range
                       [72MHz=13.8889ns ... 16.7638milliHz=59.6523s], or "unlimited"
        Type "help numbers numbers" for list of numbers configuration parameters
        Type "help numbers" for command description

        $1.50: help numbers gpio-speed=
        Help for parameter "gpio-speed=" (e.g. "numbers numbers gpio-speed="):
        - Hardware slew rate
        Current value: 2MHz
        Valid values:  "2MHz", "10MHz", or "50MHz"
        Type "help numbers numbers" for list of numbers configuration parameters
        Type "help numbers" for command description

        $1.50: help numbers open-pull=
        Help for parameter "open-pull=" (e.g. "numbers numbers open-pull="):
        - Hardware port mode
        Current value: push-pull
        Valid values:  "push-pull" or "open-drain"
        Type "help numbers numbers" for list of numbers configuration parameters
        Type "help numbers" for command description

        $1.50: help numbers limited=
        Help for parameter "limited=" (e.g. "numbers numbers limited="):
        - Command duration if "rate=limited"
        Current value: infinite
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", or "ns" suffix in range [13.8889ns ... 8124.2y], or
                       "infinite"
        Type "help numbers numbers" for list of numbers configuration parameters
        Type "help numbers" for command description

        $1.50: help numbers unlimited=
        Help for parameter "unlimited=" (e.g. "numbers numbers unlimited="):
        - Command duration if "rate=unlimited"
        Current value: infinite
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", or "ns" suffix in range [910.222μs ... 59.6514s], or
                       "infinite"
        Type "help numbers numbers" for list of numbers configuration parameters
        Type "help numbers" for command description


<a name="gpio_command"></a>
#### `gpio` command

        $1.50: help gpio
        Help for command "gpio":
        Output 8-bit parallel data on port PA4(lsb) through PA11(msb).
        - Input line of ascii or numeric data ("gpio ascii-num=") then <ENTER> key to
            send. See data entry instructions at command start.
        - Line editing and history supported, type "help help" for more info.
        - Enter "999" if "ascii-num=numeric" or end string ("end=" parameter) if
            "ascii-num=ascii", followed by <ENTER> key to exit command.
        Command usage:
             gpio [<parameter>=<value> ...] [<action>] [<configuration>
               <parameter>=<value ...] [<action>]
        Configurations (primary="gpio"):
            gpio, ipc
        Type "help gpio <configuration>" for configuration description,  parameters, and
            actions
        Type "help gpio <configuration> <parameter>" for parameter description

##### `gpio` configuration

        $1.50: help gpio gpio 
        Help for configuration "gpio" (e.g. "gpio gpio"):
        Configuration for Interactive 8-bit parallel data (ports PA4(lsb) through PA11(msb).
        Command/configuration usage:
           gpio [<parameter>=<value> ...]
        Parameters:
            speed=          Output port slew rate
            open-pull=      Output port mode
            rate=           Byte output data rate. See "Time/Frequency Error...
            ascii-num=      Terminal input/output mode
            end=            Character sequence to exit command in "ascii-num...
        Configuration "gpio" used by commands:
            configure
            gpio
        Type "help gpio" for command description
        Type "help gpio gpio <parameter>" for parameter description

##### `gpio` parameters

        $1.50: help gpio speed=
        Help for parameter "speed=" (e.g. "gpio gpio speed="):
        - Output port slew rate
        Current value: 2MHz
        Valid values:  "2MHz", "10MHz", or "50MHz"
        Type "help gpio gpio" for list of gpio configuration parameters
        Type "help gpio" for command description

        $1.50: help gpio open-pull=
        Help for parameter "open-pull=" (e.g. "gpio gpio open-pull="):
        - Output port mode
        Current value: push-pull
        Valid values:  "push-pull" or "open-drain"
        Type "help gpio gpio" for list of gpio configuration parameters
        Type "help gpio" for command description

        $1.50: help  gpio rate=
        Help for parameter "rate=" (e.g. "gpio gpio rate="):
        - Byte output data rate. See "Time/Frequency Errors" section in "help".
        Current value: 10Hz=100ms
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", "ns", "Hz", "kHz", "MHz", or "GHz" suffix in range
                       [72MHz=13.8889ns ... 16.7638milliHz=59.6523s], or "unlimited"
        Type "help gpio gpio" for list of gpio configuration parameters
        Type "help gpio" for command description

        $1.50: help gpio ascii-num=
        Help for parameter "ascii-num=" (e.g. "gpio gpio ascii-num="):
        - Terminal input/output mode
        Current value: numeric
        Valid values:  "ascii" or "numeric"
        Type "help gpio gpio" for list of gpio configuration parameters
        Type "help gpio" for command description

        $1.50: help gpio end=
        Help for parameter "end=" (e.g. "gpio gpio end="):
        - Character sequence to exit command in "ascii-num=ascii" mode
        Current value: END
        Valid values:  <any string <= 16 chars including optional \C character, \xMN
                       hexadecimal, \ABC octal, and/ or \uABCD unicode escapes) (use
                       "\x20" or "\040" for space)>
        Type "help gpio gpio" for list of gpio configuration parameters
        Type "help gpio" for command description


<a name="usart_command"></a>
#### `usart` command

The `usart` command implements a bidirectional USB-to-serial converter. Input and output on the `buck50.py` host side can be directed to/from either the `buck50.py` terminal (see example usage [above](#usart_bridge)) or to UNIX pseudoterminals (`/dev/pts/NN`) and/or UNIX sockets.

<a name="usart_synchro_explanation"></a>
Note that if synchronous mode is enabled (see [`usart synchro=`](#usart_synchro_param), below), both connected devices  must be in synchronous mode with the buck50 "Blue Pill" being the "master" (STM32F1xx chips do not support USART synchronous slave mode). Bytes are only received on the master device's RX port in sync with those sent on TX --- it is impossible for the master to receive without sending, or the slave to send without receiving. This is similar to the SPI protocol (see [`spi` command](#spi_command), below), and in fact compatible with it if `usart datalen=8bits` and `parity=none` (defaults both) are set.

Note also that when used as a usart-to-USB-to-UNIX-pseudoterminal bridge (see [`ipc`](#ipc_configuration) and [`pty`](#pty_configuration) configurations, below), baud, parity, datalength, and stop bits, etc. must be set via the `buck50.py` interface. They cannot be set via `stty`, `ioctl`, `termios`, etc. APIs on the `/dev/pts/XX` device.

        $1.50: help usart
        Help for command "usart":
        Transmit and/or receive UART/USART serial data.
          - Asynchronous/UART ("synchro=disabled") or synchronous/USART
            ("synchro=disabled" modes
          - Uses one of two USART peripherals and sets of I/O ports:
            "usart ports=pa8-10": TX=PA9  RX=PA10, CLK=PA8
            "usart ports=0-30pa": TX=PA2  RX=PA3   CTS=PA0  RTS=PA1
          - Synchronous clock output on port PA8 ("master" mode)
          - Hardware does not support synchronous "slave" mode (clock input)
          - Synchronous "master" mode with "datalen=8bits", "parity=none" can compatibly
            connect to SPI slave device
          - TX and RX data to and from terminal, or socket and/or pty (see "help usart
            ipc term-ptysock=", "help usart socket", and "help usart pty")
          - Usart xmit and/or recv data to socket and/or pty if either active, else to
            terminal
        Terminal I/O, asynchronous mode:
          - If "recv=enabled" will print asynchronous recv data to terminal.
          - <ENTER> key to pause recv data and enter line of ascii data and/or <\xXY\>
            hexadecimal values, <ENTER> key to xmit.
        Terminal I/O, synchronous mode:
          - "master" mode: enter line of ascii data and/or <\xXY\> hexadecimal values,
            <ENTER> key to xmit
          - Will display recv data bytes (same number as xmitted) and prompt for next
            line of xmit data
        Terminal I/O, both modes:
          - Input line of ascii or numeric data ("gpio ascii-num=") then <ENTER> key to
            send. See data entry instructions at command start.
          - Line editing and history supported, type "help help" for more info.
          - Enter "999" if "ascii-num=numeric" or end string ("end=" parameter) if
            "ascii-num=ascii", followed by <ENTER> key to exit command.
        Command usage:
             usart [<parameter>=<value> ...] [<action>] [<configuration>
               <parameter>=<value ...] [<action>]
        Configurations (primary="usart"):
            usart, ipc, pty, socket
        Type "help usart <configuration>" for configuration description,  parameters,
            and actions
        Type "help usart <configuration> <parameter>" for parameter description


##### `usart` configuration

        $1.50: help usart usart
        Help for configuration "usart" (e.g. "usart usart"):
        Configuration for async/sync (UART/USART) serial peripheral on ports PA9(TX),
            PA10(RX) and PA8(clock) (if "ports=pa8-10") or ports PA2(TX), PA3(RX),
            PA1(RTS) and PA0(CTS) (if "ports=pa0-3")
          - Note "datalen=7bits" with "parity=none", or "datalen=9bits" with other than
            "parity=none," not supported.
        Command/configuration usage:
           usart [<parameter>=<value> ...] [<action>]
        Actions:
            enable          Set parameter "active=enabled"
            disable         Set parameter "active=disabled"
        Parameters:
            active=         Used by monitor command
            xmit=           Transmit RX data on port PA9
            recv=           Receive RX data on port PA10
            ports=          Select one of two hardware USARTs (by PAx-y port...
            baud=           Baud rate for both xmit and recv if "synchro=ena...
            datalen=        Number of data bits. See "help usart usart" for ...
            stoplen=        Number of stop bits
            parity=         Parity. See "help usart usart" for restrictions ...
            synchro=        Async(UART)(no clock) or synchronous(USART) mode
            idle=           Clock output level at idle in "synchro=enabled" ...
            phase=          Clock edge to sample bit at in "synchro=enabled"...
            lastclok=       Last clock pulse in "synchro=enabled" mode
            gpio=           Ports output slew rate
            rate=           Byte output data rate. See "Time/Frequency Error...
            tx-timeout=     Wait for hardware TX ready. See "Time/Frequency ...
            rx-wait=        Wait for hardware RX ready. See "Time/Frequency ...
            tx-data=        Byte to xmit in "monitor" command if "synchro=en...
            rx-len=
            cts=            CTS handshaking on port PA0 (only supported for ...
            rts=            RTS handshaking on port PA1 (only supported for ...
            ascii-num=      Interactive text input/output mode. See input fo...
            end=            Character sequence to exit command in "ascii-num...
            snoop=          Print socket traffic to terminal when "socket_co...
        Configuration "usart" used by commands:
            configure
            monitor
            usart
        Type "help usart" for command description
        Type "help usart usart <parameter>" for parameter description
        Type "help usart usart <action>" for action description

##### `usart` parameters and actions

        $1.50: help usart enable
        Help for action "enable" (e.g. "usart usart enable"):
        - Set parameter "active=enabled"
        Type "help usart usart" for list of usart configuration actions
        Type "help usart" for command description

        $1.50: help usart disable
        Help for action "disable" (e.g. "usart usart disable"):
        - Set parameter "active=disabled"
        Type "help usart usart" for list of usart configuration actions
        Type "help usart" for command description

        $1.50: help usart active=
        Help for parameter "active=" (e.g. "usart usart active="):
        - Used by monitor command
        Current value: disabled
        Valid values:  "enabled" or "disabled"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart xmit=
        Help for parameter "xmit=" (e.g. "usart usart xmit="):
        - Transmit RX data on port PA9
        Current value: enabled
        Valid values:  "enabled" or "disabled"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart recv=
        Help for parameter "recv=" (e.g. "usart usart recv="):
        - Receive RX data on port PA10
        Current value: enabled
        Valid values:  "enabled" or "disabled"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart ports=
        Help for parameter "ports=" (e.g. "usart usart ports="):
        - Select one of two hardware USARTs (by PAx-y port numbers)
        Current value: pa8-10
        Valid values:  "pa8-10" or "pa0-3"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart baud=
        Help for parameter "baud=" (e.g. "usart usart baud="):
        - Baud rate for both xmit and recv if "synchro=enabled", only xmit if
          "disabled". See "Time/Frequency Errors" section in "help".
        Current value: 9.6kHz=104.167μs
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", "ns", "Hz", "kHz", "MHz", or "GHz" suffix in range
                       [4.5MHz=222.222ns ... 1.09865kHz=910.208μs]
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart datalen=
        Help for parameter "datalen=" (e.g. "usart usart datalen="):
        - Number of data bits. See "help usart usart" for restrictions with "parity=".
        Current value: 8bits
        Valid values:  "7bits", "8bits", or "9bits"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart stoplen=
        Help for parameter "stoplen=" (e.g. "usart usart stoplen="):
        - Number of stop bits
        Current value: 1bit
        Valid values:  "1bit", "2bits", "0.5bits", or "1.5bits"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart parity=
        Help for parameter "parity=" (e.g. "usart usart parity="):
        - Parity. See "help usart usart" for restrictions with "datalen=".
        Current value: none
        Valid values:  "none", "even", or "odd"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

<a name="usart_synchro_param"></a>
`usart synchro=`, see [above](#usart_synchro_explanation)

        $1.50: help usart synchro=
        Help for parameter "synchro=" (e.g. "usart usart synchro="):
        - Async(UART)(no clock) or synchronous(USART) mode
        Current value: disabled
        Valid values:  "enabled" or "disabled"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart idle=
        Help for parameter "idle=" (e.g. "usart usart idle="):
        - Clock output level at idle in "synchro=enabled" mode
        Current value: low
        Valid values:  "low" or "high"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart phase=
        Help for parameter "phase=" (e.g. "usart usart phase="):
        - Clock edge to sample bit at in "synchro=enabled" mode
        Current value: 1st
        Valid values:  "1st" or "2nd"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart lastclok=
        Help for parameter "lastclok=" (e.g. "usart usart lastclok="):
        - Last clock pulse in "synchro=enabled" mode
        Current value: enabled
        Valid values:  "enabled" or "disabled"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart gpio=
        Help for parameter "gpio=" (e.g. "usart usart gpio="):
        - Ports output slew rate
        Current value: 2MHz
        Valid values:  "2MHz", "10MHz", or "50MHz"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart rate=
        Help for parameter "rate=" (e.g. "usart usart rate="):
        - Byte output data rateSee "Time/Frequency Errors" section in "help".
        Current value: unlimited
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", "ns", "Hz", "kHz", "MHz", or "GHz" suffix in range
                       [72MHz=13.8889ns ... 16.7638milliHz=59.6523s], or "unlimited"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart tx-timeout=
        Help for parameter "tx-timeout=" (e.g. "usart usart tx-timeout="):
        - Wait for hardware TX ready. See "Time/Frequency Errors" section in "help
          help".
        Current value: 10ms
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", or "ns" suffix in range [13.8889ns ... 59.6523s], or "none"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart rx-wait=
        Help for parameter "rx-wait=" (e.g. "usart usart rx-wait="):
        - Wait for hardware RX ready. See "Time/Frequency Errors" section in "help
          help".
        Current value: 10ms
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", or "ns" suffix in range [13.8889ns ... 59.6523s], or "none"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart tx-data=
        Help for parameter "tx-data=" (e.g. "usart usart tx-data="):
        - Byte to xmit in "monitor" command if "synchro=enabled"
        Current value: 0
        Valid values:  decimal or hex integer in range [0 ... 255]
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart rx-len=
        Help for parameter "rx-len=" (e.g. "usart usart rx-len="):
        - (no help available for "usart usart rx-len")
        Current value: 62
        Valid values:  decimal or hex integer in range [1 ... 62]
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart cts=
        Help for parameter "cts=" (e.g. "usart usart cts="):
        - CTS handshaking on port PA0 (only supported for "usart=pa0-3"
        Current value: disabled
        Valid values:  "enabled" or "disabled"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart rts=
        Help for parameter "rts=" (e.g. "usart usart rts="):
        - RTS handshaking on port PA1 (only supported for "usart=pa0-3"
        Current value: disabled
        Valid values:  "enabled" or "disabled"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart ascii-num=
        Help for parameter "ascii-num=" (e.g. "usart usart ascii-num="):
        - Interactive text input/output mode. See input format instructions printed at
          command start.
        Current value: ascii
        Valid values:  "ascii" or "numeric"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart end=
        Help for parameter "end=" (e.g. "usart usart end="):
        - Character sequence to exit command in "ascii-num=ascii" mode
        Current value: END
        Valid values:  <any string <= 16 chars including optional \C character, \xMN
                       hexadecimal, \ABC octal, and/ or \uABCD unicode escapes) (use
                       "\x20" or "\040" for space)>
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description

        $1.50: help usart snoop=
        Help for parameter "snoop=" (e.g. "usart usart snoop="):
        - Print socket traffic to terminal when "socket_config[active]=enabled" and/or
          "pty_config[active]=enabled"
        Current value: disabled
        Valid values:  "disabled", "hexadecimal", or "ascii"
        Type "help usart usart" for list of usart configuration parameters
        Type "help usart" for command description



<a name="spi_command"></a>
#### `spi` command

Similar to [`usart` command](#usart_command), above,  the `spi` command implements a bidirectional USB-to-SPI converter. The same host-side I/O capabilities apply.

A full description of the SPI protocol is outside the scope of this document, but similar restrictions as in the USART synchro discussion [above](#usart_synchro_explanation) apply to data send/recv in SPI master vs. slave configurations.

        $1.50: help spi
        Help for command "spi":
        Send SPI MOSI and/or receive MISO data on ports PA7 and PA6.
          - SPI master or slave mode
          - SPI clock on port PA5 ( output if master, input if slave)
          - SPI select ("NSS") on port PA4 (see "help spi select=" and "help spi nss=")
          - MOSI and MISO data to and from terminal, or socket and/or pty (see "help spi
            ipc term-ptysock=", "help spi socket", and "help spi pty")
        Terminal I/O, master mode:
          - Enter line of numeric or ascii data (format as per instructions printed at
            command start, mode set with "ascii-num=" parameter), then <ENTER> key to
            send on MOSI.
          - Will send data on MOSI and print received MISO data (same number of bytes as
            sent) to terminal (unless "xmit-only=enabled") and prompt for next line of
            MOSI data
          - As per SPI protocol, will receive MISO data regardless of whether slave
            device connected (defaults to zero)
        Terminal I/O, slave mode:
          - Prints received MOSI data, format as per "ascii-num=" parameter.
          - <ENTER> key to pause, then enter MISO data (format as per instructions
            printed at command start, mode set with "ascii-num=" parameter), then<ENTER>
            to queue MISO data and return to MOSI data printing
          - MISO data will be sent one-to-one with subsequent received MOSI bytes
          - If MISO data queue empty (e.g. at command start) default "tx-data=" (number
            of bytes and byte values) sent on MISO
          - MOSI bytes printed in groups, size determined by lesser of number of queued
            or default MISO bytes and MOSI bytes received (unless "rx-wait=" time
            elapsed)
        Terminal I/O, both modes:
          - Line editing and history supported, type "help help" for more info.
          - Enter "999" if "ascii-num=numeric" or end string ("end=" parameter) if
            "ascii-num=ascii", followed by <ENTER> key to exit command.
        pty/socket I/O:
          - See "help spi ipc term-ptysock=", "help spi pty" and "help spi socket"
        pty/socket I/O, master mode:
          - Data received from pty and/or socket sent via MOSI. Same number of bytes
            received on MISO sent to pty and/or socket(s)
        pty/socket I/O, slave mode:
          - Data received from pty and/or socket enqueued for sent via MISO, one-to-one
            with received MOSI data
          - Data received from MOSI sent to pty and/or socket(s)
        Command usage:
             spi [<parameter>=<value> ...] [<action>] [<configuration>
               <parameter>=<value ...] [<action>]
        Configurations (primary="spi"):
            spi, ipc, pty, socket
        Type "help spi <configuration>" for configuration description,  parameters, and
            actions                                                                     
        Type "help spi <configuration> <parameter>" for parameter description

##### `spi` configuration

        $1.50: help spi spi
        Help for configuration "spi" (e.g. "spi spi"):
        Configuration for SPI serial peripheral on ports PA4(NSS), PA5(clock/SCK),
            PA6(MISO) and PA7(MOSI)
        Slave select:
          Slave ("mode=slave"):
            - "select=software"  slave enabled/active
            - "select=hardware"  slave controlled by level on NSS port (port PA4)
          Master ("mode=master") NSS pin (port PA4):
            - "nss=floating"     inactive/hi-Z
            - "nss=low"          set low (any connected slave selected)
            - "nss-active"       set high at idle, goes low "nss-time=<value>" before
                                 first clock (port PA5) edge and stays low until
                                 goes high "nss-time=<value>" after MOSI data finished
        Command/configuration usage:
           spi [<parameter>=<value> ...] [<action>]
        Actions:
            disable         Set parameter "active=disabled"
            master          Set parameter "active=master"
            slave           Set parameter "active=slave"
        Parameters:
            mode=           SPI mode, or "disabled"
            xmit-only=      Ignore MISO data in "spi" command, "mode=master"
            snoop=          Print socket traffic to terminal when "socket_co...
            select=         See "help spi spi"
            baud=           One of fixed set of supported baud rates
            endian=         Bit order for MOSI and MISO data
            idle=           Clock port PA5 level at idle, "mode=master"
            phase=          Clock edge at which data bit sampled
            miso=           Slave ("mode=slave"): MISO port PA6 hardware mod...
            pull=           Master ("mode=master"): MISO (port PA6). Use "pu...
            speed=          Output ports (PA7=MOSI, PA5=clock) slew rate
            nss=            See "help spi spi"
            tx-data=        NEED UPDATE: "mode=slave" MISO data byte for "mo...
            rate=           MOSI data output rate. See "Time/Frequency Error...
            busy-wait=      Wait time for hardware TX not ready. See "Time/F...
            rx-wait=        Delay for concatenating MISO data into single me...
            nss-time=       See "help spi spi". See "Time/Frequency Errors" ...
            ascii-num=      Interactive text input/output mode. See input fo...
            end=            Character sequence to exit command in "ascii-num...
        Configuration "spi" used by commands:
            configure
            monitor
            spi
        Type "help spi" for command description
        Type "help spi spi <parameter>" for parameter description
        Type "help spi spi <action>" for action description

##### `spi` parameters
        $1.50: help spi mode=
        Help for parameter "mode=" (e.g. "spi spi mode="):
        - SPI mode, or "disabled"
        Current value: disabled
        Valid values:  "disabled", "master", or "slave"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi xmit-only=
        Help for parameter "xmit-only=" (e.g. "spi spi xmit-only="):
        - Ignore MISO data in "spi" command, "mode=master"
        Current value: disabled
        Valid values:  "enabled" or "disabled"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi snoop=
        Help for parameter "snoop=" (e.g. "spi spi snoop="):
        - Print socket traffic to terminal when "socket_config[active]=enabled" and/or
          "pty_config[active]=enabled"
        Current value: disabled
        Valid values:  "disabled", "hexadecimal", or "ascii"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi select=
        Help for parameter "select=" (e.g. "spi spi select="):
        - See "help spi spi"
        Current value: software
        Valid values:  "software" or "hardware"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi baud=
        Help for parameter "baud=" (e.g. "spi spi baud="):
        - One of fixed set of supported baud rates
        Current value: 281.25KHz
        Valid values:  "36MHz", "18MHz", "9MHz", "4.5MHz", "2.25MHz", "1.125MHz",
                       "562.5KHz", or "281.25KHz"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi endian=
        Help for parameter "endian=" (e.g. "spi spi endian="):
        - Bit order for MOSI and MISO data
        Current value: msb
        Valid values:  "msb" or "lsb"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi idle=
        Help for parameter "idle=" (e.g. "spi spi idle="):
        - Clock port PA5 level at idle, "mode=master"
        Current value: low
        Valid values:  "low" or "high"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi phase=
        Help for parameter "phase=" (e.g. "spi spi phase="):
        - Clock edge at which data bit sampled
        Current value: 1st
        Valid values:  "1st" or "2nd"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi miso=
        Help for parameter "miso=" (e.g. "spi spi miso="):
        - Slave ("mode=slave"): MISO port PA6 hardware mode. Set "open-drain" with
          external pull-up if multiple hardware slaves, else "push-pull".
        Current value: open-drain
        Valid values:  "push-pull" or "open-drain"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi pull=
        Help for parameter "pull=" (e.g. "spi spi pull="):
        - Master ("mode=master"): MISO (port PA6). Use "pull=floating" if single
          connected slave MISO is push-pull or external pull-up on MISO line, else
          "pull=up". Do not use "pull=down".
        Current value: floating
        Valid values:  "floating", "up", or "down"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi speed=
        Help for parameter "speed=" (e.g. "spi spi speed="):
        - Output ports (PA7=MOSI, PA5=clock) slew rate
        Current value: 2MHz
        Valid values:  "2MHz", "10MHz", or "50MHz"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi nss=
        Help for parameter "nss=" (e.g. "spi spi nss="):
        - See "help spi spi"
        Current value: floating
        Valid values:  "floating", "low", or "active"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi tx-data=
        Help for parameter "tx-data=" (e.g. "spi spi tx-data="):
        - NEED UPDATE: "mode=slave" MISO data byte for "monitor" command, and for "spi"
          command if interactive input empty/exhausted
        Current value: 00
        Valid values:  1 to 32 "."-separated values, each 2 hexadecimal, 3 decimal, or 1
                       ascii character(s)
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi rate=
        Help for parameter "rate=" (e.g. "spi spi rate="):
        - MOSI data output rate. See "Time/Frequency Errors" section in "help".
        Current value: unlimited
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", "ns", "Hz", "kHz", "MHz", or "GHz" suffix in range
                       [72MHz=13.8889ns ... 16.7638milliHz=59.6523s], or "unlimited"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi busy-wait=
        Help for parameter "busy-wait=" (e.g. "spi spi busy-wait="):
        - Wait time for hardware TX not ready. See "Time/Frequency Errors" section in
          "help".
        Current value: 1ms
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", or "ns" suffix in range [13.8889ns ... 59.6523s], or
                       "infinite"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi rx-wait=
        Help for parameter "rx-wait=" (e.g. "spi spi rx-wait="):
        - Delay for concatenating MISO data into single message to host, "spi" command.
          See "Time/Frequency Errors" section in "help".
        Current value: 10ms
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", or "ns" suffix in range [13.8889ns ... 59.6523s], or "none"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi nss-time=
        Help for parameter "nss-time=" (e.g. "spi spi nss-time="):
        - See "help spi spi". See "Time/Frequency Errors" section in "help".
        Current value: zero
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", or "ns" suffix in range [13.8889ns ... 59.6523s], or "zero"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi ascii-num=
        Help for parameter "ascii-num=" (e.g. "spi spi ascii-num="):
        - Interactive text input/output mode. See input format instructions printed at
          command start.
        Current value: numeric
        Valid values:  "ascii" or "numeric"
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description

        $1.50: help spi end=
        Help for parameter "end=" (e.g. "spi spi end="):
        - Character sequence to exit command in "ascii-num=ascii" mode
        Current value: END
        Valid values:  <any string <= 16 chars including optional \C character, \xMN
                       hexadecimal, \ABC octal, and/ or \uABCD unicode escapes) (use
                       "\x20" or "\040" for space)>
        Type "help spi spi" for list of spi configuration parameters
        Type "help spi" for command description



<a name="i2c_command"></a>
#### `i2c` command

I2C semantics are even more complex than SPI. Again, a full tutorial on the subject is outside the scope of this document, and the `help` below assumes general knowledge of the I2C protocols.

        $1.50: help i2c
        Help for command "i2c":
        Send and/or receive I2C data
          - I2C master or slave mode
          - I2C clock (SCL) on port PB10, data (SDA) on port PB11
          - Master send to, and optionally receives data from, I2C address
          - Slave receive data from master at one or more I2C addresses, and optionally
            send TX data to master.
          - I2C data to and from terminal (only), input and output in hexadecimal,
            decimal, and/or ascii values
        Master ("i2c mode=master")
          - Enter lines with slave address, size of RX data to receive from slave, and
            optional data bytes to TX to slave, finished with <ENTER> key
          - Data bytes formatted as space separated entries, each as either 2
            hexadecimal digits, 3 decimal digits, or one ascii character
          - Multiple "<address> <rx-size> [data ...]" commands separated by ";" allowed
          - Prints peripheral status, number of bytes sent and received, and received
            byte values (if any) in "ddd-hh-a" format ("ddd" 3 decimal digits, "hh" two
            hexadecimal digits, "a" one ascii character or "." if unprintable)
          - Example:
            "0 0" to end: 8 2 097 61 a
            stat= OK tx=3  rx=2  : 077-4d-M 078-4e-N
          - Enter "0 0" to exit command
          - See help text printed at command start
        Slave ("i2c mode=slave")
          - Prints data received from master with status, size of data, which address
            was matched ("oar1=", "oar2=", "gen-call=") and size of TX data sent to
            master (if any requested by master)
          - Received data bytes printed in "ddd-hh-a" format ("ddd" 3 decimal digits,
            "hh" two hexadecimal digits, "a" one ascii character or "." if unprintable)
          - Examples:
            stat= OK tx=2  rx=3 @A1 : 097-61-a 097-61-a 097-61-a
            stat= OK tx=1  rx=4 @A2 : 100-64-d 101-65-e 102-66-f 103-67-g
            stat= OK tx=0  rx=1 @GC : 120-78-x
          - Note: TX slave-to-master not allowed for "gen-call" address 0
          - Press <ENTER> key to set maximum number of data bytes to receive from master
            (any additional will be discarded), and optional data bytes to queue to be
            sent to master (format: space-separated triple decimal or double hexadecimal
            digits, or single ascii characters), finished with <ENTER> key
          - If no slave-to-master data enqueued, will cyclically send "i2c tx-data="
            bytes for any number of slave TX bytes requested by master
          - Enter "0" to exit command
          - See help texts printed at command start and at <ENTER> key
        Both modes:
          - Line editing and history supported, type "help help" for more info.
          - Note I2C multi-master, 10-bit addresses, SMBus mode, etc. not supported.
        Command usage:
             i2c [<parameter>=<value> ...] [<action>]
        Configuration: i2c
        Type "help i2c i2c" for configuration description, parameters, and actions
        Type "help i2c <parameter>" for parameter description                           
        Type "help i2c <action>" for action description

##### `i2c` configuration

        $1.50: help i2c i2c
        Help for configuration "i2c" (e.g. "i2c i2c"):
        Configuration for I2C serial peripheral on ports PB10(clock/SCL) and
            PB11(data/SDA)
        Command/configuration usage:
           i2c [<parameter>=<value> ...] [<action>]
        Actions:
            disable         Set parameter "active=disabled"
            master          Set parameter "active=master"
            slave           Set parameter "active=slave"
        Parameters:
            mode=           I2C mode, or "disabled"
            flavor=         I2C protocol mode
            fast-duty=      Duty cycle for I2C clock in "flavor=fast" mode
            freq=           I2C clock frequency (only used for "mode=master"...
            addr=           Master TX address for "monitor" command
            gen-call=       Slave accepts "general call" address (0) in addi...
            oar1=           Slave address
            oar2=           Optional second slave address, or "disabled"
            tx-data=        Data for master TX in "monitor" command, or slav...
            rx-size=        Maximum size of slave RX data (any additonal dis...
            timeout=        Timeout value to abort firmware-to-hardware comm...
            gpio=           I/O port output slew rate, PB11(SDA) (master or ...
        Configuration "i2c" used by commands:
            configure
            monitor
            i2c
        Type "help i2c" for command description
        Type "help i2c <parameter>" for parameter description
        Type "help i2c <action>" for action description

##### `i2c` parameters

        $1.50: help i2c mode=
        Help for parameter "mode=" (e.g. "i2c i2c mode="):
        - I2C mode, or "disabled"
        Current value: disabled
        Valid values:  "disabled", "master", or "slave"
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description

        $1.50: help i2c flavor=
        Help for parameter "flavor=" (e.g. "i2c i2c flavor="):
        - I2C protocol mode
        Current value: standard
        Valid values:  "standard" or "fast"
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description

        $1.50: help i2c fast-duty=
        Help for parameter "fast-duty=" (e.g. "i2c i2c fast-duty="):
        - Duty cycle for I2C clock in "flavor=fast" mode
        Current value: 2:1
        Valid values:  "2:1" or "16:9"
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description

        $1.50: help i2c freq=
        Help for parameter "freq=" (e.g. "i2c i2c freq="):
        - I2C clock frequency (only used for "mode=master". See "Time/Frequency Errors"
          section in "help".
        Current value: 100kHz=10μs
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", "ns", "Hz", "kHz", "MHz", or "GHz" suffix in range
                       [1MHz=1μs ... 5kHz=200μs]
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description

        $1.50: help i2c addr=
        Help for parameter "addr=" (e.g. "i2c i2c addr="):
        - Master TX address for "monitor" command
        Current value: 0
        Valid values:  decimal or hex integer in range [0 ... 127]
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description

        $1.50: help i2c gen-call=
        Help for parameter "gen-call=" (e.g. "i2c i2c gen-call="):
        - Slave accepts "general call" address (0) in addition to "oar1=" and "oar2="
        Current value: disabled
        Valid values:  "enabled" or "disabled"
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description

        $1.50: help i2c oar1=
        Help for parameter "oar1=" (e.g. "i2c i2c oar1="):
        - Slave address
        Current value: 8
        Valid values:  decimal or hex integer in range [8 ... 119]
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description

        $1.50: help i2c oar2=
        Help for parameter "oar2=" (e.g. "i2c i2c oar2="):
        - Optional second slave address, or "disabled"
        Current value: disabled
        Valid values:  "disabled" or integer in range [8 ... 119]
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description

        $1.50: help i2c tx-data=
        Help for parameter "tx-data=" (e.g. "i2c i2c tx-data="):
        - Data for master TX in "monitor" command, or slave TX in "i2c" command if no
          data enqueued interactively in "i2c" command
        Current value: 00
        Valid values:  1 to 16 "."-separated values, each 2 hexadecimal, 3 decimal, or 1
                       ascii character(s)
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description

        $1.50: help i2c rx-size=
        Help for parameter "rx-size=" (e.g. "i2c i2c rx-size="):
        - Maximum size of slave RX data (any additonal discarded) in "monitor" command
        Current value: 0
        Valid values:  decimal or hex integer in range [0 ... 60]
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description

        $1.50: help i2c timeout=
        Help for parameter "timeout=" (e.g. "i2c i2c timeout="):
        - Timeout value to abort firmware-to-hardware commands if no response. See
          "Time/Frequency Errors" section in "help".
        Current value: 100ms
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", or "ns" suffix in range [13.8889ns ... 59.6523s], or
                       "infinite"
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description

        $1.50: help i2c gpio=
        Help for parameter "gpio=" (e.g. "i2c i2c gpio="):
        - I/O port output slew rate, PB11(SDA) (master or slave) and PB10(SCL) for
          master
        Current value: 2MHz
        Valid values:  "2MHz", "10MHz", or "50MHz"
        Type "help i2c i2c" for list of i2c configuration parameters
        Type "help i2c" for command description



<a name="pulse_command"></a>
##### `pulse` command
The poorly-name `pulse` command (see [above](#non_intuitive_names), should be "pwm" or "waveform") outputs a one, two, or three phase digital pulse train on ports PA1 through PA3.

Note (as per `help pulse`, below) that as opposed to all other `buck50.py` commands which run until completion/exited/interrupted, the pulse waveform outputs continue indefinitely until `pulse disable` or a different command which uses the same ports is executed. Using this capability a single buck50 "Blue Pill" can drive external systems while simultaneously observing them (`monitor`, `logic`, `oscope`, `usart`, `spi`, etc commands), or, alternately, do self-test experiments by connecting the `pulse` output ports to the various commands' input ports.

        $1.50: help pulse
        Help for command "pulse":
        Output digital pulse waveforms on ports PA1, PA2, and/or PA3.
        - Waveform output continues, even after program exit, until explicitly halted
            with "pulse enable=disabled", or "gpio", "numbers", or "usart ports=pa0-3"
            commands (which use same ports/pins).
        - "time1", "time2", and "time3" specify event times less than or equal to master
            "time" period
        - At event times "time1"/"time2"/"time3" ports PA1/PA2/PA3 respectively change
            level in various ways controlled by "mode1", "mode2", and "mode3"
        - At end of master "time" period ports PA1/PA2/PA3 return to default states
        Command usage:
             pulse [<parameter>=<value> ...] [<action>]
        Configuration: pulse
        Type "help pulse pulse" for configuration description, parameters, and actions
        Type "help pulse <parameter>" for parameter description
        Type "help pulse <action>" for action description

##### `pulse` configuration

        $1.50: help pulse pulse
        Help for configuration "pulse" (e.g. "pulse pulse"):
        Parameters "mode1", "mode2", "mode3" control output levels on PA1/PA2/PA3 ports
            at "time1", "time2", "time3", all within master "time" period.
        Command/configuration usage:
           pulse [<parameter>=<value> ...] [<action>]
        Actions:
            enable          Set parameter "active=enabled"
            disable         Set parameter "active=disabled"
        Parameters:
            active=         If "enabled" runs (even after  program exit) unt...
            time=           Master period/frequency for time1, time2, and ti...
            time1=          Time for "mode1" event type. If  "%"  suffix is ...
            time2=          See "help pulse time1"
            time3=          See "help pulse time1"
            mode1=          ort PA1 change at "time1":...
            mode2=          See "help pulse mode1"
            mode3=          See "help pulse mode1"
            gpio1=          Disable ("hi-z") or enable "push-pull" or "open-...
            gpio2=          See "help pulse gpio1"
            gpio3=          See "help pulse gpio1"
            speed1=         Port PA1 slew rate
            speed2=         See "help pulse speed1"
            speed3=         See "help pulse speed1"
        Configuration "pulse" used by commands:
            configure
            pulse
        Type "help pulse" for command description
        Type "help pulse <parameter>" for parameter description
        Type "help pulse <action>" for action description

##### `pulse` parameters

        $1.50: help pulse active=
        Help for parameter "active=" (e.g. "pulse pulse active="):
        - If "enabled" runs (even after  program exit) until halted with "disabled" or
          by "gpio", "numbers" or "usart ports=pa0-3" command.
        Current value: disabled
        Valid values:  "enabled" or "disabled"
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse time=
        Help for parameter "time=" (e.g. "pulse pulse time="):
        - Master period/frequency for time1, time2, and time3. See "Time/Frequency
          Errors" section in "help".
        Current value: 1kHz=1ms
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", "ns", "Hz", "kHz", "MHz", or "GHz" suffix in range
                       [36MHz=27.7778ns ... 16.7638milliHz=59.6523s]
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse time1=
        Help for parameter "time1=" (e.g. "pulse pulse time1="):
        - Time for "mode1" event type. If  "%"  suffix is percent of master "time"
          parameter. If explicit time/freq must be less than or equal to "time" (greater
          than or equal to frequency), and will be clamped if "time" changes. See
          "Time/Frequency Errors" section in "help".
        Current value: 25%=250.028μs(error:-6.94444ns)
        Valid values:  floating point number with "%", "s", "ms", "us", "μs", "ns",
                       "Hz", "kHz", "MHz", or "GHz" suffix in range [0ps ... 18.2042hr]
                       or <xxx>% (xxx floating point number in range [0 ... 100])
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse time2=
        Help for parameter "time2=" (e.g. "pulse pulse time2="):
        - See "help pulse time1"
        Current value: 50%=500.028μs(error:-13.8889ns)
        Valid values:  floating point number with "%", "s", "ms", "us", "μs", "ns",
                       "Hz", "kHz", "MHz", or "GHz" suffix in range [0ps ... 18.2042hr]
                       or <xxx>% (xxx floating point number in range [0 ... 100])
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse time3=
        Help for parameter "time3=" (e.g. "pulse pulse time3="):
        - See "help pulse time1"
        Current value: 75%=750μs(error:+6.94444ns)
        Valid values:  floating point number with "%", "s", "ms", "us", "μs", "ns",
                       "Hz", "kHz", "MHz", or "GHz" suffix in range [0ps ... 18.2042hr]
                       or <xxx>% (xxx floating point number in range [0 ... 100])
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse mode1=
        Help for parameter "mode1=" (e.g. "pulse pulse mode1="):
        - Port PA1 change at "time1":
          frozen : no change
              up : set high
            down : set low
          toggle : set high if low, or set low if high (no change at master "time")
            high : continuously high
             low : continuously low
            fall : high-to-low (return high at master "time")
            rise : low-to-high (return low at master "time")
        Current value: rise
        Valid values:  "frozen", "up", "down", "toggle", "low", "high", "fall", or
                       "rise"
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse mode2=
        Help for parameter "mode2=" (e.g. "pulse pulse mode2="):
        - See "help pulse mode1"
        Current value: rise
        Valid values:  "frozen", "up", "down", "toggle", "low", "high", "fall", or
                       "rise"
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse mode3=
        Help for parameter "mode3=" (e.g. "pulse pulse mode3="):
        - See "help pulse mode1"
        Current value: rise
        Valid values:  "frozen", "up", "down", "toggle", "low", "high", "fall", or
                       "rise"
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse gpio1=
        Help for parameter "gpio1=" (e.g. "pulse pulse gpio1="):
        - Disable ("hi-z") or enable "push-pull" or "open-drain") on port PA1
        Current value: push-pull
        Valid values:  "push-pull", "open-drain", or "hi-z"
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse gpio2=
        Help for parameter "gpio2=" (e.g. "pulse pulse gpio2="):
        - See "help pulse gpio1"
        Current value: push-pull
        Valid values:  "push-pull", "open-drain", or "hi-z"
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse gpio3=
        Help for parameter "gpio3=" (e.g. "pulse pulse gpio3="):
        - See "help pulse gpio1"
        Current value: push-pull
        Valid values:  "push-pull", "open-drain", or "hi-z"
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse speed1=
        Help for parameter "speed1=" (e.g. "pulse pulse speed1="):
        - Port PA1 slew rate
        Current value: 2MHz
        Valid values:  "2MHz", "10MHz", or "50MHz"
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse speed2=
        Help for parameter "speed2=" (e.g. "pulse pulse speed2="):
        - See "help pulse speed1"
        Current value: 2MHz
        Valid values:  "2MHz", "10MHz", or "50MHz"
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description

        $1.50: help pulse speed3=
        Help for parameter "speed3=" (e.g. "pulse pulse speed3="):
        - See "help pulse speed1"
        Current value: 2MHz
        Valid values:  "2MHz", "10MHz", or "50MHz"
        Type "help pulse pulse" for list of pulse configuration parameters
        Type "help pulse" for command description



<a name="ipc_configuration"></a>
### `ipc` configuration

The `ipc` configuration controls whether serial protocol (`usart`, `spi`, `i2c`) I/O is directed to the `buck50.py` terminal or to external UNIX devices.

        $1.50: help configure ipc
        Help for configuration "ipc" (e.g. "configure ipc"):
        Enable/disable IPC (pty and/or socket) I/O
          - Also see "help <command> pty" and "help <command> socket"
        Command/configuration usage:
           configure ipc [<parameter>=<value> ...] [<action>]
        Actions:
            terminal        Set parameter "i/o=terminal"
            external        Set parameter "i/o=external"
        Parameters:
            i/o=            I/O to terminal or to pty and/or socket (set "pt...
            flush=          Delay time to flush pending received device-to-h...
        Configuration "ipc" used by commands:
            configure
            gpio
            usart
            spi
        Type "help configure" for command description
        Type "help configure ipc <parameter>" for parameter description
        Type "help configure ipc <action>" for action description

##### `ipc` parameters and actions

        $1.50: help configure ipc terminal 
        Help for action "terminal" (e.g. "configure ipc terminal"):
        - Set parameter "i/o=terminal"
        Type "help configure ipc" for list of ipc configuration actions
        Type "help configure" for command description

        $1.50: help configure ipc terminal external 
        Help for action "external" (e.g. "configure ipc external"):
        - Set parameter "i/o=external"
        Type "help configure ipc" for list of ipc configuration actions
        Type "help configure" for command description

        $1.50: help configure ipc terminal i/o=
        Help for parameter "i/o=" (e.g. "configure ipc i/o="):
        - I/O to terminal or to pty and/or socket (set "pty enable" and/or "socket
          enable")
        Current value: terminal
        Valid values:  "terminal" or "external"
        Type "help configure ipc" for list of ipc configuration parameters
        Type "help configure" for command description

        $1.50: help configure ipc terminal flush=
        Help for parameter "flush=" (e.g. "configure ipc flush="):
        - Delay time to flush pending received device-to-host data at command exit
        Current value: 1s
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                       "us", "μs", or "ns" suffix in range [1s ... 1min]
        Type "help configure ipc" for list of ipc configuration parameters
        Type "help configure" for command description



<a name="pty_configuration"></a>
### `pty` configuration

        $1.50: help configure pty
        Help for configuration "pty" (e.g. "configure pty"):
        Connect USB I/O to /dev/pty/... special device file.
          - Connect external program to /dev/pts/N ("N" printed to terminal at startup)
          - Example: `miniterm /dev/pts/32`
          - Pty stty/termios values not passed through to usart -- use usart config
            parameters (see "help usart usart") to set baud, parity, etc.
          - Must also set "ipc external"
          - Set "snoop=enabled" parameter to view xmit/recv data
        Command/configuration usage:
           configure pty [<parameter>=<value> ...] [<action>]
        Actions:
            enable          Set parameter "active=enabled"
            disable         Set parameter "active=disabled"
        Parameters:
            active=         Create, and I/O to, /dev/pts/N
            snoop=          Print socket traffic to terminal
        Configuration "pty" used by commands:
            configure
            usart
            spi
        Type "help configure" for command description
        Type "help configure pty <parameter>" for parameter description
        Type "help configure pty <action>" for action description

##### `pty` parameters and actions

        $1.50: help configure pty enable
        Help for action "enable" (e.g. "configure pty enable"):
        - Set parameter "active=enabled"
        Type "help configure pty" for list of pty configuration actions
        Type "help configure" for command description

        $1.50: help configure pty disable
        Help for action "disable" (e.g. "configure pty disable"):
        - Set parameter "active=disabled"
        Type "help configure pty" for list of pty configuration actions
        Type "help configure" for command description

        $1.50: help configure pty active=
        Help for parameter "active=" (e.g. "configure pty active="):
        - Create, and I/O to, /dev/pts/N
        Current value: disabled
        Valid values:  "enabled" or "disabled"
        Type "help configure pty" for list of pty configuration parameters
        Type "help configure" for command description

        $1.50: help configure pty snoop=
        Help for parameter "snoop=" (e.g. "configure pty snoop="):
        - Print socket traffic to terminal
        Current value: disabled
        Valid values:  "disabled", "hexadecimal", or "ascii"
        Type "help configure pty" for list of pty configuration parameters
        Type "help configure" for command description



<a name="socket_configuration"></a>
### `socket` configuration

        $1.50: help configure socket
        Help for configuration "socket" (e.g. "configure socket"):
        Connect USB I/O to Unix socket.
          - Socket is server, supports connections from multiple external clients
          - Set "host" and "port" parameters
          - Connect via external client program, e.g. `telnet localhost 1024`
          - Must also set "ipc external"
          - Set "snoop" parameter to view xmit/recv data
        Command/configuration usage:
           configure socket [<parameter>=<value> ...] [<action>]
        Actions:
            enable          Set parameter "active=enabled"
            disable         Set parameter "active=disabled"
        Parameters:
            active=         Enable/disable I/O to socket
            host=           Server socket hostname
            port=           Server socket port number
            snoop=          Print socket traffic to terminal
        Configuration "socket" used by commands:
            configure
            usart
            spi
        Type "help configure" for command description
        Type "help configure socket <parameter>" for parameter description
        Type "help configure socket <action>" for action description

##### `socket` parameters and actions

        $1.50: help configure socket enable
        Help for action "enable" (e.g. "configure socket enable"):
        - Set parameter "active=enabled"
        Type "help configure socket" for list of socket configuration actions
        Type "help configure" for command description

        $1.50: help configure socket disable 
        Help for action "disable" (e.g. "configure socket disable"):
        - Set parameter "active=disabled"
        Type "help configure socket" for list of socket configuration actions
        Type "help configure" for command description

        $1.50: help configure socket active=
        Help for parameter "active=" (e.g. "configure socket active="):
        - Enable/disable I/O to socket
        Current value: disabled
        Valid values:  "enabled" or "disabled"
        Type "help configure socket" for list of socket configuration parameters
        Type "help configure" for command description

        $1.50: help configure socket host=
        Help for parameter "host=" (e.g. "configure socket host="):
        - Server socket hostname
        Current value: localhost
        Valid values:  <any string <= 63 chars including optional \C character, \xMN
                       hexadecimal, \ABC octal, and/ or \uABCD unicode escapes) (use
                       "\x20" or "\040" for space)>
        Type "help configure socket" for list of socket configuration parameters
        Type "help configure" for command description

        $1.50: help configure socket port=
        Help for parameter "port=" (e.g. "configure socket port="):
        - Server socket port number
        Current value: 1024
        Valid values:  decimal or hex integer in range [1024 ... 65535]
        Type "help configure socket" for list of socket configuration parameters
        Type "help configure" for command description

        $1.50: help configure socket snoop=
        Help for parameter "snoop=" (e.g. "configure socket snoop="):
        - Print socket traffic to terminal
        Current value: disabled
        Valid values:  "disabled", "hexadecimal", or "ascii"
        Type "help configure socket" for list of socket configuration parameters
        Type "help configure" for command description



<a name="reset_command"></a>
#### `reset` command

The `reset` command implements a set of varied secondary actions. With the exception of the `reset ext-trig` action and its associated `reset ganged=` parameter, none of them should be necessary during normal buck50 operation. `ext-trig` is important for "ganging" multiple buck50s together -- see `help reset ext-trig` and `help reset ganged=`, below.

Most of the other actions are designed for recovery in the unlikely [<sup>21</sup>](#footnote_21) event of a firmware fault. The somewhat redundant ability to disconnect a running `buck50.py` process from one buck50 "Blue Pill" and reconnect to another is also supported. The `reset serialnum` and `reset blink` actions may be useful in identifying multiple simultaneously-connected buck50 "Blue Pill" boards -- see  [buck50.py startup and device connection](#buck50_py_startup_and_device_connection), above.

Changing the default ST-recommended `reset wait=` and ` pre=` parameters with the `reset flash` action is not advisable, but included is case the user wishes to live dangerously.

See [above](#reset_default) for why there is no `reset defaults` command.

        $1.50: help reset
        Help for command "reset":
        Reset various host, device, and USB subsystems:
          - Enabling or disabling multiple connected devices external triggering and
            sampling synchronization ("ganged=", "ext-trig")
          - USB disconnect, (re-)connect, or port/driver change ("connect", "usb=")
          - Stalled USB communications recovery ("flush")
          - Firmware halt and/or reset ("halt", "init")
          - Change flash memory (firmware code) wait states and/or enable/disable
            prefetch queue ("flash" with "wait=" and/or "pre=")
        Performing "halt", "flush", and/or "init" required only in case of
            firmware/software bug or USB communication failure.
        Similarly, "connect" with or without new "usb=/dev/<...>" for error or to
            connect to different port/device.
        Command usage:
             reset [<parameter>=<value> ...] [<action>]
        Configuration: reset
        Type "help reset reset" for configuration description, parameters, and actions
        Type "help reset <parameter>" for parameter description
        Type "help reset <action>" for action description

##### `reset` configuration
        $1.50: help reset reset
        Help for configuration "reset" (e.g. "reset reset"):
        - Set any/all of "halt=", "flush=", "reconnect=", and/or "init="  to "enabled"
        - Will execute in above order
        - All reset to "disabled" after command completion.
        Command/configuration usage:
           reset [<parameter>=<value> ...] [<action>]
        Actions:
            ext-trig        Activates or de-activates external trigger sync ...
            flash           Set flash memory wait states and prefetch buffer...
            halt            Send halt command to firmware
            flush           Flush USB data pipe (/dev/tty/ACM0)
            init            Send initialization commands to firmware
            connect         Disconnect/re-connect to same or different USB p...
            serialnum       Get device serial number
            blink           Blink user LED to identify device
        Parameters:
            ganged=         Synchronized digital ("logic") and/or analog ("o...
            wait=           Flash memory wait states. Stability unlikely if ...
            pre=            Flash memory prefetch buffer. Performance impact...
            usb=            USB port and device driver
            timeout=        Timeout value for reset USB operations (can inte...
        Configuration "reset" used by commands:
            configure
            reset
        Type "help reset" for command description
        Type "help reset <parameter>" for parameter description
        Type "help reset <action>" for action description

##### `reset` actions and parameters

        $1.50: help reset ext-trig 
        Help for action "ext-trig" (e.g. "reset ext-trig"):
        - Activates or de-activates external trigger sync after setting "ganged="
          parameter ("enabled" or "disabled"). See "help reset ganged=". Also resets all
          PAx and PBx ports to default.
        Type "help reset reset" for list of reset configuration actions
        Type "help reset" for command description

        $1.50: help reset flash 
        Help for action "flash" (e.g. "reset flash"):
        - Set flash memory wait states and prefetch buffering. Must be after "wait=" and
          "pre=" parameters.
        Type "help reset reset" for list of reset configuration actions
        Type "help reset" for command description

        $1.50: help reset halt
        Help for action "halt" (e.g. "reset halt"):
        - Send halt command to firmware
        Type "help reset reset" for list of reset configuration actions
        Type "help reset" for command description

        $1.50: help reset flush
        Help for action "flush" (e.g. "reset flush"):
        - Flush USB data pipe (/dev/tty/ACM0)
        Type "help reset reset" for list of reset configuration actions
        Type "help reset" for command description

        $1.50: help reset init
        Help for action "init" (e.g. "reset init"):
        - Send initialization commands to firmware
        Type "help reset reset" for list of reset configuration actions
        Type "help reset" for command description

        $1.50: help reset connect
        Help for action "connect" (e.g. "reset connect"):
        - Disconnect/re-connect to same or different USB port / device driver (first set
          "usb" parameter if changing)
        Type "help reset reset" for list of reset configuration actions
        Type "help reset" for command description

        $1.50: help reset serialnum 
        Help for action "serialnum" (e.g. "reset serialnum"):
        - Get device serial number
        Type "help reset reset" for list of reset configuration actions
        Type "help reset" for command description

        $1.50: help reset blink
        Help for action "blink" (e.g. "reset blink"):
        - Blink user LED to identify device
        Type "help reset reset" for list of reset configuration actions
        Type "help reset" for command description

        $1.50: help reset ganged=
        Help for parameter "ganged=" (e.g. "reset reset ganged="):
        - Synchronized digital ("logic") and/or analog ("oscope") command triggering
          across multiple devices. Requires two open-drain bus lines, each with pull-up
          resistor to Vdd. Connect all devices' PB14 ports to one open-drain bus, and
          all PB15 ports to other. After enabling with "ganged=enabled" must be
          activated via "reset ext-trig" action before "logic" or "oscope" commands.
          Triggering on any device will trigger all.
        Current value: disabled
        Valid values:  "enabled" or "disabled"
        Type "help reset reset" for list of reset configuration parameters
        Type "help reset" for command description

        $1.50: help reset wait=
        Help for parameter "wait=" (e.g. "reset reset wait="):
        - Flash memory wait states. Stability unlikely if less than default value of 2.
          Use "flash" action to set.
        Current value: 2
        Valid values:  decimal or hex integer in range [0 ... 2]
        Type "help reset reset" for list of reset configuration parameters
        Type "help reset" for command description

        $1.50: help reset pre=
        Help for parameter "pre=" (e.g. "reset reset pre="):
        - Flash memory prefetch buffer. Performance impacted if not enabled. Use "flash"
          action to set
        Current value: enabled
        Valid values:  "enabled" or "disabled"
        Type "help reset reset" for list of reset configuration parameters
        Type "help reset" for command description

        $1.50: help reset usb=
        Help for parameter "usb=" (e.g. "reset reset usb="):
        - USB port and device driver
        Current value: /dev/tty/ACM0
        Valid values:  <any valid file or path name> (no overwrite warning if filename
                       contains "tmp", "temp", or "scratch")
        Type "help reset reset" for list of reset configuration parameters
        Type "help reset" for command description

        $1.50: help reset timeout=
        Help for parameter "timeout=" (e.g. "reset reset timeout="):
        - Timeout value for reset USB operations (can interrupt by <ENTER> key
        Current value: 1s
        Valid values:  floating point number with "y", "d", "our", "min", "s", "ms",
                        "us", "μs", or "ns" suffix in range [0ps ... 1.82795e+10y], or
                       "infinite"
        Type "help reset reset" for list of reset configuration parameters
        Type "help reset" for command description



<br> <a name="waveform_viewing_software"></a>
Waveform viewing software
-------------------------

There are several open-source digital and analog waveform viewing programs that can be used to display digital and analog sample data captured by `buck50.py`. Each has its pros and cons, but unfortunately none is ideal. In somewhat reverse order of usefulness ...

#### gnuplot <a name="gnuplot"></a>

![](images/gnuplot.png "gnuplot")

As per [above](#viewer_integration), if `gnuplot` ([gnuplot website](http://www.gnuplot.info/)) is installed on the `buck50.py` host system, it is used as the default viewer for both digital and analog captures. The captures are automatically dumped (see `dump auto-digital` and `dump auto-analog` [above](#dump_auto)) to a temporary CSV file and `gnuplot` invoked with the requisite [<sup>9</sup>](#footnote_9) commands to view it.

The intent is to provide a quick "sanity check" overview of the capture. `gnuplot`'s interactive measurement and analysis tools are limited [<sup>22</sup>](#footnote_22), and it certainly has no protocol decoding capabilities. But both `pulseview` and `gtkwave` are slower to start up, and function poorly when repeatedly invoked with commandline-specified data files. It is much more practical to start them independently, use their menus to load `buck50.py` VCD output, and "reload" when those output files are changed. See `dump digital-frmt=` and `dump viewer-vcd=`, [above](#dump_digital_frmt)/[above](#dump_viewer_vcd).


#### gtkwave <a name="gtkwave"></a>
![](images/gtkwave.png "gtkwave")

`gtkwave` looks to be a highly capable program with many features, but somewhat aimed at different use-cases (VLSI simulation?) than what `buck50.py` requires. It's also a classic example of non-intuitive graphical interface with non-useful default behavior. When a file (maybe just VCD files?) is loaded, nothing is displayed. The above image is the result of much post-load setup: Click "top", then when the initially undisplayed list of captured signals appears select them all, and then "Append" [<sup>23</sup>](#footnote_23). And if color-coded traces are desired, each signal name must be individually right-clicked, then "Color Format" in the pulldown menu, and a color chosen. Simple and obvious, right? Did the designers ever consider displaying everything by default with the ability to delete unwanted elements?

It also has no signal decoding capabilities [<sup>24</sup>](#footnote_24), again likely because that falls outside the program's intended use-cases.

#### PulseView <a name="pulseview"></a>
![](images/pulseview.png "pulseview")

The sigrok [website](https://sigrok.org/) "PulseView" software is among the best programs, open-source or commercial, I have ever used. I prefer its design, interface, usability, extensibility, and range of available binary and protocol decoders to even Saleae's "Logic" software, [below](#Logic).

Unfortunately, `pulseview` has one well-known, almost fatal flaw (along with several minor ones, see [<sup>25</sup>](#footnote_25)), particularly for use with "buck50" captures: It allocates memory on a per-tick/per-signal basis, regardless of whether there is any change in any signal at that tick's time.

I believe this is the basis for the oft-cited complaint against `pulseview` : That it's slow. I don't believe this is caused by any failing in the code's algorithms or their implementations, but simply due to memory swapping. For example, do **NOT** load the following example VCD file into `pulseview` (at least with the default VCD import settings):

        $timescale 1 ns $end
        $scope module top $end
        $var wire 1 z blk $end
        $upscope $end
        $enddefinitions $end
        #0
        0z
        #9999999999
        1z
        #10000000000
        0z

That's a 1ns pulse at the end of 10 seconds of inactivity. On my system it consumes approximately 10 gigabytes of memory. On systems with less than that amount of RAM (and with disk swap enabled) I've witnessed real, non-example captured data with  similar periods of inactivity lock up the host in infinite swap activity requiring a hardware reboot to recover [<sup>26</sup>](#footnote_26).

Yes, there's the "compress idle periods" option of the VCD importer. (Is there any way to set that from the `pulseview` commandline when loading a file at startup?) It's a useful hack to avoid the "fatality" of the problem, but it's a hack nonetheless -- it requires *a priori* knowledge of the maximum length of actual important null periods, and it destroys real information about the interval between events (compress idle periods above 1ms, and is it 1ms or 10 minutes between two UART characters?)

The saddest part is that a casual examination of the source code for `pulseview` and its `libsigrok` library show them to be well designed and structured pieces of software engineering. In particular, `pulseview` is written in C++, and it seems that, for example, `class Segment` could be derived from, with an overloaded `get_raw_sample()` method that handled sparse, memory-efficient, non-consecutive samples. 

I might get desperate and try to implement this in `pulseview` myself. But there's an obvious learning curve to be breached, and I've already devoted far too much time developing "buck50" itself, so I'm hoping that a sigrok developer will someday address the issue (which has been been publicly requested by others). FWIW, the offending code in `libsigrok/src/input/vcd.c` is:

        /* Generate samples from prev_timestamp up to timestamp - 1. */
        count = timestamp - inc->prev_timestamp;
        sr_spew("Got a new timestamp, feeding %zu samples", count);
        add_samples(in, count, FALSE);
        inc->prev_timestamp = timestamp;
        inc->data_after_timestamp = FALSE;
        continue;

##### `buck50.py` VCD/pulseview parameters <a name="pulseview_buck50"></a>
The "buck50" firmware timestamps its sparse samples with 1/72MHz precision [<sup>27</sup>](#footnote_27). Ideally, its VCD output files would specify their `$timescale` header with this value, but not only does the VCD format not support frequency-based  nor floating point period timescales, but loading such a file into `pulseview` would almost certainly cause excessive memory consumption, possibly even with "compress idle periods" set. This is the purpose of `buck50.py`'s `dump tick-units=` and `dump per-tick=` parameters. See the `buck50.py` help system, [above](#pulseview_parameters), regarding the topic.



#### Logic (honorable mention) <a name="Logic"></a>
![](images/logic.png "logic")

The `Logic` software by Saleae ([company website](https://www.saleae.com/), [software download](https://www.saleae.com/downloads/)) is not open-source software but is distributed for use free of charge ([license](https://support.saleae.com/faq/general-faq/license)). Much appreciation to Saleae for doing so.

The following comments are strictly my own (possible mis-)understandings and opinions:

In at least their original products, Saleae leveraged the ability of the Cypress Semiconductor FX2 USB microcontroller to transfer data acquired on GPIO port inputs to USB at very high speed (full USB2.0 bandwidth?). This is in contrast to traditional logic analyzers (also "buck50") which buffer data in memory and transfer it to a host computer (or even more traditionally use a built-in display) for viewing. The advantage of Saleae approach is that it provides effectively infinitely long captures as long as the analyzer-to-host communication bandwidth and host processing/storage speeds are not exceeded.

Consequently, the basic use of the `Logic` software is to drive the Saleae hardware and capture its output in real time. The software can save and reload captured data, but only in Saleae's own proprietary, undocumented format. The realtime communication protocol is publicly known (unsure if this is with Saleae's permission or not) and in fact PulseView can be used with a realtime-attached Saleae analyzer.

The bottom line to all this is that `Logic` is not usable for viewing "buck50"-captured data. I would prefer to spend any development effort improving PulseView's failings as described [above](#pulseview) instead of simulating the Saleae protocol for non-realtime viewing. In addition, doing the latter would likely fall into the same "sparse data" hole that PulseView has, requiring the generation of the non-existent static/non-changing samples only to have them elided by `Logic` (which does handle long idle periods without problems).

FWIW, note that the data in the above image was captured using an older, discontinued, but genuine Saleae product which was on temporary loan to me by its owner. Using it (my first hands-on experience with a logic analyzer although I was certainly aware of them and their capabilities) along with lack of funds to purchase one for permanent use was in some ways the impetus for creating "buck50". Note also that the Saleae products (and similarly other commercially-available logic analyzers) "kick ass" on buck50's capabilities [<sup>28</sup>](#footnote_28).




<br> <a name="firmware_design_and_implementation"></a>
Firmware design and implementation
----------------------------------

<a name="port_assignments"></a>
### Port assignments 

Trying to optimally map peripherals to I/O pins on ST microcontrollers is very much like solving a multi-dimensional jigsaw puzzle. This is especially true with the primitive "alternate function" mapping on the STM32F1 series MCUs (which ST improved slightly in their subsequent chips by inverting the many-to-one, function-to-pin API into a pin-to-function scheme). I've complained vociferously about this in my other GitHub repositories, citing the huge advantages in capabilities and ease of use of NXP's "switch matrix" crossbar approach (but note that NXP has their own brain damage, such as the lack of software SPI slave select which forces wasting two pins to achieve the same result).

An early version of buck50 used ports PA0 through PA7 for its digital sampling and monitoring lines. That was much cleaner from a conceptual (ports 0-7 mapped to bits 0-7) and hardware (PA0 through PA7 are contiguous on the "Blue Pill" board) standpoint, but had the fatal flaw that reading GPIOA also brought in the `USB-` and `USB+` bits from pins PA11 and PA12 respectively. As those are changing at random with USB activity (including activity not directed at the buck50 "Blue Pill" in use) it required masking the bits out with an extra instruction before the `crnt==prev` test in the hyper-time-critical sampling loop (see [Hacks\^H\^H\^H\^H\^Htechniques](#hacks_techniques), below). [<sup>29</sup>](#footnote_29)

Additionally, as buck50 grew from its beginnings as a pure logic analyzer into the multi-function mess\^H\^H\^H\^Hsuper tool it is now, switching over to the PB ports allowed better utilization of the additional functions (SPI, USART, etc.) Note that PB2 is not brought out as a usable pin on the "Blue Pill" boards and it was again going to be inefficient if the port bits were not contiguous. That left a choice between PB3-PB10 vs PB4-PB11, and it was felt that as long as the pins were not going to be contiguous on the boards it was better to split them 6-and-2 instead of 7-and-1 with the lone PB10 on the opposite side of the board from the others.

That's the story, anyway. If I was laying out the board I'd do it differently (and use a more modern/powerful chip), but then it wouldn't cost the US$1.50 which is a large part of the [<sup>30</sup>](#footnote_30) charm of this project.



<a name="hacks_techniques"></a>
### Hacks\^H\^H\^H\^H\^Htechniques 

As mentioned, the buck50 firmware uses several somewhat unusual programming idioms to achieve its time-critical sampling performance given the limitations of the STM32F103 platform. Among them are ...

##### Infinite loop <a name="infinite_loop"></a>

The first hack [<sup>31</sup>](#footnote_31) is the innermost sampling code loop, which in its simplest form consists of:

        uniform:
        ldr     r3, [r0, #8]
        ldr     r5, [r1, #8]
        orr.w   r5, r5, r3, lsl #20
        cmp     r3, r4
        it      ne
        strne.w r5, [r2], #4
        mov     r4, r3
        b.n     uniform

Note that this is written in ARM Thumb assembly, which isn't strictly necessary for this part of the code but avoids fighting GCC's limited control of register allocation when using embedded assembly from C/C++. That control is, in turn, required for other parts of the overall architecture, to be described shortly.

The fine details of the code, which is essentially a translation of the C-like pseudocode:

        while (true) {
            crnt = gpiob->idr;           // read GPIO port B
            time = systick->val;         // read current time, lower 24 bits
            smpl = time | (crnt << 20);  // GPIO bits 4 thru 11 or'd with time
            if (crnt == prev)
                *samples++ = crnt;
            prev = crnt;
        }

are not as important as its obvious faults:

<a name="loop_slowdowns"></a>
1. It's an infinite loop, without exit.
2. No counter (or end pointer) for number of samples.
3. No check for elapsed maximum sampling duration.
4. No check for user interrupt.

All of these are intentional in order to make the loop as fast as possible. Incrementing and checking a counter (or comparing `samples` to an end address), checking a timer's count or a timeout bit, and/or checking the USB communication layer's state or message interrupt flag would all add extra cycles.

(Note that "buck50" includes optional variants of the assembly code (see `logic mode=`, [above](#logic_mode)) that eliminate the `prev=current` copy by "ping-ponging" between two registers,  and/or partially unroll the loop to amortize the time-costly branch instruction and pipleline stall it causes, but the principles remain the same.)

So how, you the hypothetical curious reader ask, does this ever work? The scheme was part of the original "buck50" conceptual design from its very beginning. As per the `buck50.py` help system [above](#logic_command):

        $1.50: help logic
        ...
        - sampling continues until first of:
          . user interrupt (<ENTER> key)
          . duration elapsed ("logic duration=")
          . number of samples ("logic edges=") (incl. extra samples @ 233ms)
          . memory full

<a name="following_hacks"></a>
This is implemented via the following hacks:

1. RAM sections are laid out in a non-standard `.ld` configuration. From low to high it's `.data` (initialized data), then `.bss` (uninitialized data), then the  `.stack`, and finally sample memory from there until the end of RAM. (Usually the stack is at the top of RAM.)
2. Samples are stored starting at `end_of_ram - number_of_samples`.
3. When the code attempts to write a sample past the end of RAM, the CPU generates a fault interrupt which is trapped.
4. If a finite sampling time is specified (`logic duration=`, [above](#logic_duration)), a timer is started with that time period which then generates an interrupt, also trapped, when it expires.
5. The USB interrupt handler checks a "sampling in progress" flag, and if true does what the fault and timer handlers always do, which is ...


##### `longjmp` from interrupt handler <a name="longjmp_from_interrupt_handler"></a>

The memory fault and timer interrupt handlers always, and the USB handler if sampling in progress, set a few global variables and then do a C-style `longjmp` to the top of `main()`'s main loop. The variables are read and handled, an appropriate USB message is sent to the `buck50.py` host program, and the main loop continues waiting for another host command. Standard `setjmp`/`longjmp` methodology.

This again was part of the basic "buck50" design from the beginning. Alternately, in addition to the other time-consuming code designs described [above](#loop_slowdowns), the interrupt handlers could set a flag (either a C/C++ variable or, for faster testing, directly in a register) and the sampling loop could check that. Either way (more so with the variable) would add precious cycles and slow down the sampling rate.

But there was a fly in the ointment. A fair amount of work went into initial "buck50" development, with unthinking confidence in the ancient C longjmp mechanism before it was discovered that the `longjmp` distributed in the [GNU Arm Embedded Toolchain's](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm) C libraries doesn't work.

Or to be more accurate [<sup>32</sup>](#footnote_32), it doesn't work if called from an ARM interrupt handler. Or more accurately still, it *does* work when called from  an ARM interrupt handler -- but it only works once. Or maybe more, but that's impossible to determine, because ... if called from an interrupt handler, the standard library's `longjmp` leaves the the ARM interrupt system in an illegal state (the corresponding interrupt number's bit is set in the NVIC IABR register and cannot be cleared by software) and no further interrupts handlers will ever be executed again until after a hardware reboot.

Much pain, confusion, hair-pulling and teeth-gnashing was involved in discovering these facts. Once known there was more of the same, with many fine hours spent poring over the detailed and informative but complex and confusing *ARM v7-M Architecture Reference Manual*. What eventually resulted from this work was a hack-of-all-hacks: Code to manipulate the bizarre state of the call stack inside an interrupt handler, which inserts the `longjmp` address in place of the normal return address so that upon arriving there more custom cleanup code can restore the registers and stack to their original `setjmp` conditions. See `.macro irq_handler_exit` in [buck50_asm.s](build/src/buck50_asm.s) and the `// setjmp` and `longjmp_return:` sections in `main()` in [buck50.cxx](build/src/buck50.cxx) for the gory details.

Side note: ARM seems to pride its interrupt handling architecture for both  its speed and efficiency, and  the fact that any normal C function can be used as an interrupt handler without special compiler support to insert special entry setup and exit teardown code. To accomplish this, the interrupt hardware saves all CPU registers on the stack before invoking the handler -- "buck50"'s crazy return address manipulation and  the failure of the standard library's `longjmp` is due to the CPU's hacking the normal link register as part of the process. I personally would happily trade the ability to use normal functions as interrupt handlers for a faster interrupt entry (*not* saving all the registers, thus requiring the handler to save any it will use, or all if it is not a leaf function and is going to call others with unknown register usage) and, as a side benefit, allowing standard `longjmp` to work from an interrupt handler.

Any thoughts, ideas, insights, corrections, or proposed improvements to the above explanation or the code it describes would be welcome.


##### Sparse timestamped samples <a name="sparse_timestamped_samples"></a>

One more hack, not as radical as the ones above ...

As per the infinite loop code/pseudocode [above](#infinite_loop) and other documentation, the buck50 firmware stores logic analyzer samples only when one or more signal lines change level. If this was not the case,  storing samples at 6 MHz into less than 5K of sample RAM would result in an unusable c. 1 ms maximum sampling time.

The samples are timestamped with the value of the 24-bit ARM "systick" timer running at full 72 MHz core clock speed [<sup>33</sup>](#footnote_33). This presents a problem as the 24-bit counter rolls over every 0x1000000/72e6==0.233 seconds -- if no signal changes within that period, the elapsed time between samples can only be computed as N*0.233+T where "T" is the delta between the systick values and N is an unknowable integer.

The solution is to internally connect one of the STM peripheral timers to an output port and have it toggle faster than 1.0/0.233 Hz. This introduces a fictitious level change causing at least one sample to be stored within the systick rollover period, which makes "N" uniquely 1 and the delta time between samples deterministic. The extra c. 4 Hz sample memory consumption is minimal, and extra timer port bit is shifted out in the same instruction that combines the systick value with the 8 input ports, i.e. at with no time penalty [<sup>34</sup>](#footnote_34).






<a name="why_so_slow"></a>
### Why so slow? 

With all the above assembly code and custom implementations, why is "buck50" still limited to an approximately 6.25MHz sampling rate?

Good question, and one that I'd appreciate any help with in finding a conclusive answer. Once again, the original conceptual design was based on an estimate that 8MHz would be achievable with register ping-ponging and a 4 times unrolled loop.

<a name="loop_unroll"></a>
This was to be implemented with the four "unrolls" looking something like:

        6884            ldr     r4, [r0, #8]
        688d            ldr     r5, [r1, #8]
        ea45 5504       orr.w   r5, r5, r4, lsl #20
        429c            cmp     r4, r3
        bf18            it      ne
        f842 5b04       strne.w r5, [r2], #4

(with alternate unrolls swapping `r3` and `r4`) and a single amortized branch back to the beginning. (Disassembled binary instructions included for reference in discussion [below](#flash_faster).)

*The ARM Cortex-M3 Processor (Revision: r2p1) Technical Reference Manual*  supposedly gives the number of cycles required for every instruction. There is some vagueness in its specifications ("An IT instruction can be folded onto a preceding 16-bit Thumb instruction, enabling execution in zero cycles."), but basically most instructions are documented to take 1 cycle each, with loads and stores taking 2, and branches taking "1+P" where "P" is "The number of cycles required for a pipeline refill. This ranges from 1 to 3 depending on the
alignment and width of the target instruction, and whether the processor manages to speculate the address early."

So given the above, and assuming the `it` instruction takes zero cycles due to the preceding 16-bit Thumb `cmp` instruction (`cmp` with two "low" `r0` to `r7` registers can fit into the 16-bit Thumb format), that's 8 cycles for the loop. Add 4 cycles for the amortized-over-4-unrolls branch -- the likely worst case scenario of 1+3 given the distance of the branch almost certainly causing a full pipeline flush -- and that's 9 total which at a 72MHz clock speed is 8MHz per sample. And that's without the benefit of the *TRM*'s claim of "Neighboring load and store single instructions can pipeline their address and data phases. This enables these instructions to complete in a single execution cycle." ("single instructions" in this context means load and store a single register, as opposed the the `ldm`, `stm`, `push`, `pop`, etc instructions which load or store multiple registers at once.)

Unfortunately, the *TRM* is work of complete fiction. I've done extensive testing, both inside buck50 and with standalone code, and every instruction takes more cycles than is documented. If nothing else, there is a difference between the same assembly opcode depending on whether its arguments  (high-vs-low registers, addressing modes) require  16- and 32-bit instruction encoding -- a difference never mentioned in the *TRM*. Also, branches typically take 5 cycles, not 4.

Note that the Cortex-M3 core in the STM32F103 series MCUs is a relatively simple CPU. It has a 3-stage pipeline (source: *Cortex-M3 Devices Generic User Guide, © 2010*), and limited branch prediction ("speculatively prefetches from branch target addresses", ibid).

One possible explanation I've seen online is that the cycle counts listed in the *TRM* are for CPU execution only and do not include additional time required for memory accesses and other issues related to the non-ARM logic that ST and other licensees add to their chips. Firstly, if true, doesn't that make the TRM documentation fairly useless, especially if ST doesn't publish their own amended specs with the full numbers? Second, ST *does* document specs on their chip's internal buses (AHB, APB1, APB2, etc). But the "buck50" code is accessing GPIOB which is on the APB2 bus operating at full 72 MHz speed, and I believe that the `systick` timer is part of the core or directly connected to it and also should be accessible without slowdown.

<a name="flash_faster"></a>
Finally, the biggest mystery is this: Despite every piece of documentation and online forum/blog/etc advice I've seen stating that to run at the fastest possible speed code should execute from RAM, I found (to my great surprise) that "buck50" runs faster when executed directly in flash memory(!!). This is of course with flash prefetch enabled, but according to the *RM0008 Reference manual STM32F101xx, STM32F102xx, STM32F103xx, STM32F105xx and STM32F107xx advanced Arm®-based 32-bit MCUs* the prefetch buffer is only a "Read interface with prefetch buffer (2x64-bit words)", so two of the 16-byte loop unrolls [above](#loop_unroll) will fit, but no more and that doesn't include the branch instruction. Nevertheless, all of the `logic mode=` variants, regardless of loop size, run faster in flash. Note also that standalone experiments show a speedup if flash wait states are reduced from the recommended value of 2 (with a 72MHz main clock) but doing so is of course ill-advised.

In any case, the `logic code-mem=ram` option was left in `buck50.py` for purposes of experimentation. Note that only small, sampling mode specific,  amounts of executable code are copied from flash to RAM (dynamically, when the `logic` command is run) so reduction of available sample memory is minimal.  Any interesting reports comparing `=ram` to `=flash` would be greatly appreciated.

One more thing: Given the insanely low "street price" of the "Blue Pill" boards (far lower than even large quantity pricing for the STM32F103xb chip alone from reputable U.S. distributors, much less the added cost of the crystal, USB connector, LEDs, reset switch, passives, the PC board itself and assembly) there exists the possibility that these are clone/bootleg/counterfeit chips. (There is much information on the subject available online.) Regardless, my amateur speculation on the topic is that even if counterfeit and not up to ST's quality standards, that would result in analog/silicon-level problems such as the chips not running stably at their full rated speed. It seems likely that a fake chip would use stolen VHDL IP or IC masks, compared to the cloner generating a design from scratch using API documentation but getting it "wrong" resulting in the fake design requiring more cycles per instruction than the genuine article. 

Pardon my rancor on the subject, but I spent an inordinate amount of time trying to uncover these facts. I may have never embarked on this project had I known them in advance -- 8MHz sampling was an original design goal that I had to abandon because of them. Once again, any definitive information, insights, corrections or other advice on the topic would be welcome.




<br> <a name="building_from_source"></a>
Building from source 
--------------------

Source code for the buck50 firmware and its associated dependencies is included in the directory tree rooted at [build](build).

The top-level directory contains ready-to-flash pre-built binaries [buck50.elf](build/buck50.elf), [buck50.bin](build/buck50.bin), and [buck50.hex](build/buck50.hex) (see [Firmware installation](#firmware_installation), above), a [Makefile](build/Makefile) for building them from source, and the [buck50.py](build/buck50.py) host driver program.

The [buck50.cxx](build/src/buck50.cxx) implementation was written using the 
[regbits](https://github.com/thanks4opensource/regbits), [regbits_stm](https://github.com/thanks4opensource/regbits_stm), and the [<sup>35</sup>](#footnote_35) [papoon_usb](https://github.com/thanks4opensource/papoon_usb) libraries. All are included here in the [build/include/thanks4opensource](build/include/thanks4opensource) and [build/util/stm32f10_12357xx](build/util/stm32f10_12357xx) subdirectories to avoid the need for linked or separate repository downloading. Additional dependencies are provided in those and other subdirectories.

The pre-made binaries have been built with the [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm), release "9.3.1 20200408". Note that building  with this compiler suite requires GCC options of `-std=c++17` [<sup>36</sup>](#footnote_36) and `-O1` or higher (the [Makefile](build/Makefile) and pre-built binaries are `-O2`). Also, the `Makefile` requires a `GCC_ARM_ROOT` environment variable pointing to the toolchain's installation directory.

Building with other toolchains will likely require option and possibly source code changes, and the user may want to replace the [stm32f103_flash_init.c](build/init/stm32f103_flash_init.c) startup code and [low_stack_flash.ld](build/src/low_stack_flash.ld) with a more platform-appropriate implementations. (See [above](#following_hacks) and the `low_stack_flash.ld` source concerning the required memory layout.)

Reports about successes or failures building buck50 in different environments are welcome.




<br> <a name="current_version"></a>
Current version
---------------

        $ egrep 'VERSION.*{' build/src/buck50.cxx 
                                VERSION            [] = {0, 9, 2},

        $ ./buck50.py --version
        buck50.py 0.9.2
        Copyright 2020 Mark R. Rubin aka "thanks4opensource"




<br> <a name="future_work"></a>
Future work
-----------

* Find GNU Readline python module with working `set_pre_input_hook()` so `?` alternate help syntax can immediately retype line without requiring user to press the `UP` key.
* Timing-based triggering for UART async
* Option to use ports PA0-PA3 instead of or in addition to PA4 for SPI master
NSS to support enabling/disabling multiple SPI slaves in `spi` command.
* Add "strobed" mode to `gpio` command. Dedicate one PA0-7 line for strobe, set data on remaining ports and pulse strobe high/low (configurable, and with configurable timing). Implement similar option for `pb4-11` input in `monitor` command.
* Coffee maker attachment in planning stages. No ETA for completion.


<br> <a name="footnotes"></a>
Footnotes
---------
1. <a name="footnote_1"></a> See [Waveform viewing software](#waveform_viewing_software), above, for why this is not integrated into buck50.
2. <a name="footnote_2"></a> See [Future work](#future_work), above
3. <a name="footnote_3"></a> Actually the STM32F103 *can* be flashed over USB, but doing so requires installing a custom bootloader. See e.g. <https://medium.com/@paramaggarwal/programming-an-stm32f103-board-using-usb-port-blue-pill-953cec0dbc86> and/or <https://github.com/Serasidis/STM32_HID_Bootloader>, among others. But note that installing the bootloader in turn requires a means of flashing via USB, so it's turtles all the way down.
4. <a name="footnote_4"></a> The buck50 firmware and `buck50.py` host driver program can flash firmware onto a second "Blue Pill" STM32F103 MCU via their respective serial ports (see [Hardware connections](#hardware_connections), above, and "help usart pty" in the `buck50.py` program), but ... those damn turtles again.
5. <a name="footnote_5"></a> Mr. Kjeldsen calls his work "a nice little stm32f103 pinout diagram". That's a massive understatement. His one simple,  beautiful, functional, and incredibly useful image almost completely eliminates the need for futile searches through over a thousand pages of ST's incompetent and/or obfuscated official documentation ... all of it  scattered across multiple manuals, datasheets, and application notes. It is this author's opinion that ST should fire their entire worldwide documentation staff (or, more properly, the managers thereof) and redirect their salaries to Kjeldsen and other open source creators who do the job much more competently.
6. <a name="footnote_6"></a> Never observed in the author's testing, but there's always something.
7. <a name="footnote_7"></a> I was dragged kicking and screaming away from Python2, but after a decade --- and the dropping of Python2 support --- it was time. The distinction between `byte` and `string`, and the disaster that is Unicode remains regrettable.
8. <a name="footnote_8"></a> My favorite: Software-based test instruments with GUIs simulating 7 segment LED displays. A 1080p or even 4K resolution monitor capable of displaying beautifully readable typefaces instead being used to emulate primitive 50 year old technology. Designers didn't use MAN1 LEDs because they were easy to read -- they used them because that was the best thing available at the time. And don't get me started on Nixie tubes, with their rearmost cathodes obscured by the ones further to the front (shudder). But to each his own. If it gives you a warm fuzzy feeling to look at a poor onscreen rendering of the front panel of some piece of test equipment you used years ago, go for it. "It's so retro!!!" Whatever. Peace out.
9. <a name="footnote_9"></a> And very verbose and cryptic commands they are, but the author of buck50.py probably shouldn't be throwing stones. That said, `gnuplot` doesn't have command completion, and its hierarchical help system is, IMHO, lacking compared to `buck50.py`'s. YMMV, and it also should be noted that `gnuplot`'s capabilities are more extensive and complex in comparison.
10. <a name="footnote_10"></a> "Use the `TAB` key, Luke!"
11. <a name="footnote_11"></a> i.e. beating a dead horse
12. <a name="footnote_12"></a> And at $1.50 each, affordable on most budgets.
13. <a name="footnote_13"></a> As per online research; not tested by the author.
14. <a name="footnote_14"></a> ... and do so with great vigor
15. <a name="footnote_15"></a> The Department Of Redundancy Department
16. <a name="footnote_16"></a> Or modify the xxx_config dictionaries in the `buck50.py` code. Open source FTW.
17. <a name="footnote_17"></a> Just another "software with text interfaces *can* be efficient" usability feature.
18. <a name="footnote_18"></a> mixed metaphor
19. <a name="footnote_19"></a> Without too much excessive typing. Trigger state machine entry is admittedly still a pain.
20. <a name="footnote_20"></a> Isn't the mnemonic something like "Bad Boys Respect Our Young Girls Because Violet Got the Winning first place trophy at the national MMA championships and will kick their asses if they get out of line"?
21. <a name="footnote_21"></a> Wishful thinking.
22. <a name="footnote_22"></a> No "snapping", for example. But `gnuplot` wasn't designed to be a waveform viewer.
23. <a name="footnote_23"></a> It took me much experimentation to figure all that out. But I've already documented my anti-GUI/pro-text prejudices.
24. <a name="footnote_24"></a> Or I just haven't been able to find them, given the opaque interface and my inability to use GUI programs.
25. <a name="footnote_25"></a> Including the GUI's requirement to import VCD files explicitly instead of via automatic detection of their type from the "Open..." pulldown menu (but automatic detection *is* implemented for commandline filename arguments). But more importantly: The colors. What's wrong with bright, saturated graphic traces on a black background? The red and green, high/low spans are ugly, and their pastel backgrounds indistinguishable from each other. Please, sigrok project, provide an option to do it the "right" way.
26. <a name="footnote_26"></a> Don't get me started on the fact that Linux processes in "D" state are unkillable, or the default Linux memory allocation policy that never returns a NULL pointer indicating failure even if the requested size is larger than all available memory.
27. <a name="footnote_27"></a> The accuracy of those timestamps is open to question, although the "Blue Pill" STM32F103 is driven by a crystal oscillator so it should be fairly reasonable. Also see `buck50.py`'s `configure adjust trim=` command.
28. <a name="footnote_28"></a> But can the Saleae products function as usart/spi/i2c serial bridges, or output PWM or 8 bit parallel signals? Point buck50 ;)
29. <a name="footnote_29"></a> Either that or accepting significant extra wasted sample memory consumption.
30. <a name="footnote_30"></a> ... possibly the only part
31. <a name="footnote_31"></a> I'll drop ancient Usenet \^H pun. After 5 uses it's getting stale, if in fact it wasn't at or even before the very first.
32. <a name="footnote_32"></a> The GNU Project's flagship GCC software wouldn't have a non-working `longjmp`, would it?
33. <a name="footnote_33"></a> The systick time can optionally run at 1/8 core clock speed, but that would reduce the precision/granularity of the timestamps.
34. <a name="footnote_34"></a> If test experiments and ARM instruction cycle timings are to be believed.
35. <a name="footnote_35"></a> curiously named
36. <a name="footnote_36"></a> This is required for proper compilation of some aspects of the [regbits](https://github.com/thanks4opensource/regbits) library. Only `-std=c++11` was needed for the release 7 and 8 versions. The full reason for this is unknown.
