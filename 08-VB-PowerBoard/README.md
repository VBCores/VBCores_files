# PowerBoard

## Overview
The VBCores PowerBoard is an advanced solution for powering up your hardware. It provides multiple power inputs, sophisticated protection features, power system status monitoring, and basic user IO.

Based on [VB32G4 controller](https://github.com/VBCores/VBCores_files/tree/main/01-VB-Core32G4) 




## Description

![VBCores Powerboard 30A](vb-powerboard-v1_2-pinout.png)
PDF version: [vb-powerboard-v1_2-pinout.pdf](vb-powerboard-v1_2-pinout.pdf)

### Power inputs and outputs
The Powerboard draws power from one of three parallel channels: two "main" high-current inputs and one low-current "auxiliary" channel.
All three channels feature reverse polarity protection. Only one channel can be enabled at a time.

The two main channels support hot-swapping and reverse-current capability, and can be connected to an external charger via the "charger" input. These channels can be prioritized independently of the voltage levels of connected batteries. If two batteries are connected, the priority battery will drain first, and the secondary battery will automatically engage once the priority battery is depleted.

The auxiliary channel is designed for low-power sources (up to 3A) to maintain system operation during main battery replacement. It does not permit reverse current flow. The auxiliary input has the highest priority and, when connected, automatically disables the main channels.

There are two output channels: Bus and PC Bus .

- The Bus includes an emergency stop button that overrides all powerboard software control signals as a safety measure.
- The PC Bus ensures continuous power to the onboard computer, allowing users to diagnose faulty software configurations that may cause unexpected behavior.

The Bus can only be enabled if the PC Bus is active. Disabling the PC Bus will also disable the Bus .


### **Powerboard Charging Notes**  
The Powerboard does not include a built-in battery charger. However, connected batteries can be charged using an external charger connected to the **"Charger" input**.  

⚠️ **Critical Safety Warning** ⚠️  
- **Only use an appropriate charger!**  
- **Voltage and current levels must strictly match the battery specifications!**  

### User IO
8 identical digital input-output channels can be used to drive LEDs and read switches. Each channel is equiped with source type DMOS and Schmitt trigger. PWM supported. Output voltage level is determined by solder jumper and can be either 5V or 12V. 

### Software
Out of the box software implements power and buzzer control only. For detailed description please refer to github repository. 


## Features
- Two 30A and one 3A inputs
- 3A charger input
- Hot swap
- Reverse polarity protection
- Reverse current capability
- Power source prioritization
- Undervoltage and overcurrent protection
- 8 user IO
- Power system state is reported to the computer via FDCAN bus

### Specs
- Up to 50V input voltage
- 30A continuous current, up to 50A peak
- FDCAN bitrate: up to 8 Mbit/s data rate
  
### Dimensions
- PCB: 61x51 mm
- Mount holes: 55x45 mm D2.5

### Schematic
![VBCore PowerBoard_V1_2 Schematic](vb-powerboard-v1_2-schematic.png)
PDF version: [vb-powerboard-v1_2-schematic.pdf](vb-powerboard-v1_2-schematic.pdf)


## Development Resources

### Main Functions of VB PowerBoard

- Battery protection from over-discharge and inrush current limitation
- Robot and battery protection during short circuits
- Robot protection when batteries are connected with reverse polarity
- Multiplexing three independent power sources into two power buses - computer power bus and power consumer bus
- Ability to connect external charger and charge "on the go" without disconnecting the computer bus
- Monitoring power system status and transmitting it via FDCAN bus
- Control of basic robot indicators - buzzer, LEDs, user buttons
- Emergency robot shutdown when Big Red Button is pressed


### Multiplexing

PowerBoard has three input power channels: two controlled ("2" and "3") and one uncontrolled ("1"). Controlled channels withstand continuous current up to 30A, uncontrolled - up to 3A. Both controlled channels have bidirectional conductivity and pass current to their battery if the power bus voltage exceeds the battery voltage (for example, if a motor connected to the robot rotates under external forces). All three channels can be simultaneously connected to three different power sources, with the following statements being true:

- Power buses can be simultaneously connected to only one of the channels
- If there is voltage on the uncontrolled channel, controlled channels are automatically disconnected. Their discharge and charge are impossible in this case
- If voltage is present on only one channel, it is connected automatically
- If voltage is applied only to two controlled channels, the board can select a priority channel regardless of their battery status. Priority channel selection is configured by board firmware or command from the onboard computer
- If the priority controlled channel is discharged but another controlled channel is charged - bus switching occurs automatically and seamlessly

Current from different channels is not summed - the maximum available current is determined by the capabilities of the selected channel.

### User Input/Output

PowerBoard is enabled by connecting the **Enable contact to GND**. If the connector is not closed, the total board consumption from all sources is approximately zero. A closed **Emergency** connector disables the robot's power bus at the hardware level regardless of the board program status. Designed for connecting the Big Red Button.

PowerBoard is equipped with two six-pin connectors for connecting RGB buttons and Buzzer. The outputs have a common anode, with voltage of 5V or 12V depending on the board jumper state.

:::info
**Cathodes are connected to ground through ULN2003, which is controlled by pins PC6, PC7, PC8, PC9 (connected to TIM3 channels). Inputs are connected to pins PB0, PB1, PB2, PB10. If built-in indication means are insufficient, you can use the integrated expansion port connected to I2C2 for communication with other boards.**
:::

### Firmware Modification

The program for PowerBoard is divided into two parts: service and user. The service part is processed in **TIM6** and **TIM16** timer interrupts. The first services the **ADC1** data filtering subroutine. The second calls the board power switch control subroutine.

**DO NOT MAKE CHANGES TO THESE CODE SECTIONS IF YOU DON'T UNDERSTAND WHAT YOU'RE DOING! Disruption of the service part of the program can lead to physical damage to the board, robot, and batteries in emergency situations like short circuits on the bus.**

The user part of the program is intended for managing indication, communication with the onboard computer. It is preferable to perform these tasks in the _uavcan_setup()_ and _uavcan_spin()_ functions from _main()_. The interrupt priorities of **TIM6, TIM7** and **TIM16** should be defined as follows (in descending order):

1. **TIM7**. Counts microseconds since program start.
2. **TIM6**. Updates and filters data from **ADC1**.
3. **TIM16**. Calls the board power switch control subroutine.

User interrupt priorities **MUST** be lower than those of the above timers.



### Photos
<p float="left">
<img src="vb-powerboard-v1_2-1.jpg" width="200">
<img src="vb-powerboard-v1_2-2.jpg" width="200">
</p>

### 3D model

STEP model: [vb-powerboard-v1_2.stp](vb-powerboard-v1_2.stp)
<br>
Texture top: [vb-powerboard-v1_2-texture-top.png](vb-powerboard-v1_2-texture-top.png)
<br>
Texture bottom: [vb-powerboard-v1_2-texture-bottom](vb-powerboard-v1_2-texture-bottom.png)

<p float="left">
<img src="vb-powerboard-v1_2-render-1.png" width="300">
<img src="vb-powerboard-v1_2-render-2.png" width="300">
</p>

### Use cases and other

Example of wiring Lanboo RGB buttons. Check carefully led voltage!

![VBCore PowerBoard_V1_2 Schematic](cases/lanboo-pinout.png)
PDF version: [cases/lanboo-pinout.pdf](cases/lanboo-pinout.pdf)





