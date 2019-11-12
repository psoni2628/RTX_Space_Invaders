# RTX_Space_Invaders
RTX Space Invaders was created based on the retro video game Space Invaders. The game was designed for the NXP ARM LPC 1768 microcontroller in C using multiple threads to manage the game's various properties such as score and position. Onboard peripherals were used to allow the user to interface with the game. The peripherals used were a joystick to control user position, a push button to fire the user's weapon, and LEDs to keep track of the user's score. To achieve the necessary concurrency on the single core ARM Cortex-M3 processor found on the LPC 1768 board, CMSIS-RTOS2 was used for thread management.

https://psoni2628.github.io/#space-invaders
